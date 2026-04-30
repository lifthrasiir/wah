#define WAH_IMPLEMENTATION
#include "../wah.h"
#include "wah_impl.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

uint32_t wah_debug_wasm_page_size(void) { return WAH_WASM_PAGE_SIZE; }

uint8_t *wah_debug_memory_data(const wah_exec_context_t *ctx, uint32_t mem_idx) {
    return ctx->memories[mem_idx].data;
}
uint64_t wah_debug_memory_size(const wah_exec_context_t *ctx, uint32_t mem_idx) {
    return ctx->memories[mem_idx].size;
}

wah_error_t wah_debug_replace_data_segment_fill(wah_module_t *mod,
    uint32_t data_idx, uint32_t size, uint8_t value)
{
    if (!mod || data_idx >= mod->data_segment_count) return WAH_ERROR_NOT_FOUND;
    uint8_t *data = (uint8_t *)malloc(size);
    if (!data) return WAH_ERROR_OUT_OF_MEMORY;
    memset(data, value, size);
    free((void *)mod->data_segments[data_idx].data);
    mod->data_segments[data_idx].data = data;
    mod->data_segments[data_idx].data_len = size;
    return WAH_OK;
}

uint64_t wah_debug_table_size(const wah_exec_context_t *ctx, uint32_t tbl_idx) {
    return ctx->tables[tbl_idx].size;
}

wah_value_t wah_debug_global_value(const wah_exec_context_t *ctx,
    const wah_module_t *mod, uint32_t global_idx)
{
    if (mod && global_idx < mod->import_global_count &&
        mod->global_imports[global_idx].is_mutable) {
        return *(wah_value_t *)ctx->globals[global_idx].ref;
    }
    return ctx->globals[global_idx];
}

wah_error_t wah_debug_element_segment(const wah_module_t *mod, uint32_t idx,
    wah_type_t *out_elem_type)
{
    if (idx >= mod->element_segment_count) return WAH_ERROR_NOT_FOUND;
    if (out_elem_type) *out_elem_type = mod->element_segments[idx].elem_type;
    return WAH_OK;
}

wah_error_t wah_debug_module_export_table(wah_module_t *mod, const char *name,
    wah_type_t elem_type,
    wah_type_t addr_type, uint64_t min_elements, uint64_t max_elements)
{
    wah_alloc_t alloc_storage = wah_resolve_alloc(&mod->alloc);
    const wah_alloc_t *alloc = &alloc_storage;
    void *new_ptr = mod->tables;
    WAH_CHECK(wah_realloc(alloc, mod->table_count + 1, sizeof(*mod->tables), &new_ptr));
    mod->tables = (wah_table_type_t *)new_ptr;
    mod->tables[mod->table_count] = (wah_table_type_t){0};
    mod->tables[mod->table_count].elem_type = elem_type;
    mod->tables[mod->table_count].addr_type = addr_type;
    mod->tables[mod->table_count].min_elements = min_elements;
    mod->tables[mod->table_count].max_elements = max_elements;

    new_ptr = mod->exports;
    WAH_CHECK(wah_realloc(alloc, mod->export_count + 1, sizeof(*mod->exports), &new_ptr));
    mod->exports = (wah_export_t *)new_ptr;
    char *name_copy = wah_strdup(name, alloc);
    if (!name_copy) return WAH_ERROR_OUT_OF_MEMORY;
    mod->exports[mod->export_count].name = name_copy;
    mod->exports[mod->export_count].name_len = strlen(name);
    mod->exports[mod->export_count].kind = 1;
    mod->exports[mod->export_count].index = mod->table_count;
    mod->export_count++;
    mod->exports_cap = mod->export_count;
    mod->table_count++;
    return WAH_OK;
}

static int is_func_ref_in_range(const void *base, uint32_t count, size_t elem_size, const void *ref) {
    uintptr_t start, end, ptr;
    if (!base || count == 0) return 0;
    start = (uintptr_t)base;
    end = start + (uintptr_t)count * elem_size;
    ptr = (uintptr_t)ref;
    return ptr >= start && ptr < end;
}

bool wah_debug_is_func_ref_in_ctx(const wah_exec_context_t *ctx, const void *ref) {
    if (!ctx || !ref) return false;
    if (is_func_ref_in_range(ctx->function_table, ctx->function_table_count,
                             sizeof(wah_function_t), ref))
        return true;
    return false;
}

bool wah_debug_is_func_ref_in_module(const wah_module_t *mod, const void *ref) {
    if (!mod || !ref) return false;
    return is_func_ref_in_range(mod->functions, mod->local_function_count,
                                sizeof(wah_function_t), ref) != 0;
}

bool wah_debug_type_is_exnref(const wah_module_t *mod, wah_type_t type) {
    if (!mod || !WAH_TYPE_IS_REF(type)) return false;
    return wah_type_is_subtype(type, WAH_TYPE_EXNREF, mod);
}

void wah_debug_free_exnref(wah_exec_context_t *ctx, void *ref) {
    wah_exception_free(ctx, (wah_exception_t *)ref);
}

const wah_module_t *wah_debug_linked_module(const wah_exec_context_t *ctx, uint32_t i) {
    if (i >= ctx->linked_module_count) return NULL;
    return ctx->linked_modules[i].module;
}

const wah_exec_context_t *wah_debug_linked_ctx(const wah_exec_context_t *ctx, uint32_t i) {
    if (i >= ctx->linked_module_count) return NULL;
    return ctx->linked_modules[i].ctx;
}

void wah_debug_set_linked_module(wah_exec_context_t *ctx, uint32_t i, const wah_module_t *mod) {
    if (i < ctx->linked_module_count) ctx->linked_modules[i].module = mod;
}
void wah_debug_set_linked_ctx(wah_exec_context_t *ctx, uint32_t i, wah_exec_context_t *lctx) {
    if (i < ctx->linked_module_count) ctx->linked_modules[i].ctx = lctx;
}

static int ptr_in_range(const void *p, const void *base, size_t byte_size) {
    const char *pc = (const char *)p;
    return pc >= (const char *)base && pc < (const char *)base + byte_size;
}

static wah_exec_context_t *relocate_ctx_ptr(wah_exec_context_t *p, void *old_base, size_t byte_size, ptrdiff_t delta) {
    if (!p || !ptr_in_range(p, old_base, byte_size)) return p;
    return (wah_exec_context_t *)((char *)p + delta);
}

void wah_debug_relocate_exec_refs(wah_exec_context_t *ctx, void *old_base, size_t byte_size, ptrdiff_t delta) {
    if (!ctx || !old_base || byte_size == 0 || delta == 0) return;
    for (uint32_t i = 0; i < ctx->linked_module_count; i++) {
        ctx->linked_modules[i].ctx = relocate_ctx_ptr(ctx->linked_modules[i].ctx, old_base, byte_size, delta);
    }
    for (uint32_t i = 0; i < ctx->function_table_count; i++) {
        if (!ctx->function_table[i].is_host) {
            ctx->function_table[i].fn_ctx = relocate_ctx_ptr(ctx->function_table[i].fn_ctx, old_base, byte_size, delta);
        }
    }
    for (uint32_t i = 0; i < ctx->table_count; i++) {
        ctx->tables[i].import_ctx = relocate_ctx_ptr(ctx->tables[i].import_ctx, old_base, byte_size, delta);
    }
    for (uint32_t i = 0; i < ctx->memory_count; i++) {
        ctx->memories[i].import_ctx = relocate_ctx_ptr(ctx->memories[i].import_ctx, old_base, byte_size, delta);
    }
}

wah_error_t wah_debug_repr_info(const wah_module_t *mod, wah_type_t type,
    wah_debug_repr_info_t *out_info, wah_debug_repr_field_t *out_fields, uint32_t max_fields)
{
    if (!mod || !out_info || type < 0) return WAH_ERROR_MISUSE;
    uint32_t typeidx = WAH_TYIDX(type);
    if (typeidx >= mod->type_count || !mod->typeidx_to_repr) return WAH_ERROR_NOT_FOUND;
    wah_repr_t repr_id = mod->typeidx_to_repr[typeidx];
    if (repr_id < 0 || (uint32_t)repr_id >= mod->repr_count || !mod->repr_infos) return WAH_ERROR_NOT_FOUND;
    const wah_repr_info_t *info = mod->repr_infos[repr_id];
    if (!info) return WAH_ERROR_NOT_FOUND;
    *out_info = (wah_debug_repr_info_t){
        .type = (uint32_t)info->type,
        .typeidx = info->typeidx,
        .size = info->size,
        .count = info->count,
    };
    if (out_fields) {
        uint32_t n = info->count < max_fields ? info->count : max_fields;
        for (uint32_t i = 0; i < n; ++i) {
            out_fields[i] = (wah_debug_repr_field_t){
                .offset = info->fields[i].offset,
                .repr_id = info->fields[i].repr_id,
            };
        }
    }
    return WAH_OK;
}

void wah_debug_print_platform_features(void) {
#if defined(WAH_X86_64) && ((WAH_COMPILED_FEATURES) & WAH_FEATURE_SIMD)
    wah_x86_64_features_t f = wah_x86_64_features();
    printf("CPU features: x86-64 sse2");
    if (f.ssse3) printf(" ssse3");
    if (f.sse41) printf(" sse4.1");
    if (f.sse42) printf(" sse4.2");
    if (f.avx2) printf(" avx2");
    if (f.avx512f_vl) printf(" avx512f+vl");
    if (f.avx512dq_vl) printf(" avx512dq+vl");
    if (f.avx512bw_vl) printf(" avx512bw+vl");
    if (f.avx512bitalg_vl) printf(" avx512bitalg+vl");
    printf("\n");
#elif defined(WAH_AARCH64)
    printf("CPU features: aarch64 neon\n");
#else
    printf("CPU features: portable\n");
#endif
}

wah_error_t wah_debug_parse_func_spec(const char *types,
    size_t *out_nparams, wah_type_t **out_param_types,
    size_t *out_nresults, wah_type_t **out_result_types)
{
    wah_error_t err;
    wah_alloc_t alloc = wah_resolve_alloc(NULL);
    wah_type_spec_parser_t p;
    wah_func_type_t ft = {0};
    WAH_ENSURE(types && out_nparams && out_param_types && out_nresults && out_result_types, WAH_ERROR_MISUSE);
    *out_nparams = 0;
    *out_param_types = NULL;
    *out_nresults = 0;
    *out_result_types = NULL;
    p = (wah_type_spec_parser_t){ .cur = types, .module = NULL, .allow_placeholders = false };
    bool has_fn = wah_type_spec_take_kw(&p, "fn");
    p.cur = types;
    WAH_CHECK_GOTO(wah_type_spec_parse_func(&p, &ft, !has_fn, &alloc), cleanup);
    wah_type_spec_skip_ws(&p);
    WAH_ENSURE_GOTO(*p.cur == '\0', WAH_ERROR_BAD_SPEC, cleanup);
    *out_nparams = ft.param_count;
    *out_param_types = ft.param_types;
    *out_nresults = ft.result_count;
    *out_result_types = ft.result_types;
    return WAH_OK;
cleanup:
    free(ft.param_types);
    free(ft.result_types);
    return err == WAH_OK ? WAH_ERROR_BAD_SPEC : err;
}

void wah_debug_free_type_spec(wah_debug_type_spec_t *spec) {
    if (!spec) return;
    free(spec->param_types);
    free(spec->result_types);
    free(spec->field_types);
    free(spec->field_mutables);
    *spec = (wah_debug_type_spec_t){0};
}

static wah_error_t wah_debug_parse_define_type_specv(wah_module_t *mod, wah_debug_type_spec_t *out,
                                                     const char *spec, va_list *args) {
    wah_error_t err;
    wah_alloc_t alloc = wah_resolve_alloc(NULL);
    wah_type_spec_parser_t p;
    wah_func_type_t ft = {0};
    wah_type_def_t td = { .kind = WAH_COMP_FUNC, .is_final = true, .supertype = WAH_NO_SUPERTYPE };

    WAH_ENSURE(mod && out && spec, WAH_ERROR_MISUSE);
    *out = (wah_debug_type_spec_t){0};
    p = (wah_type_spec_parser_t){ .cur = spec, .module = mod, .args = args, .allow_placeholders = true };

    if (wah_type_spec_take_kw(&p, "fn")) {
        p.cur = spec;
        td.kind = WAH_COMP_FUNC;
        WAH_CHECK_GOTO(wah_type_spec_parse_func(&p, &ft, false, &alloc), cleanup);
        out->kind = WAH_DEBUG_TYPE_SPEC_FUNC;
        out->param_count = ft.param_count;
        out->param_types = ft.param_types;
        out->result_count = ft.result_count;
        out->result_types = ft.result_types;
        ft = (wah_func_type_t){0};
    } else if (wah_type_spec_take_kw(&p, "struct")) {
#if ((WAH_COMPILED_FEATURES) & WAH_FEATURE_GC)
        p.cur = spec;
        td.kind = WAH_COMP_STRUCT;
        WAH_CHECK_GOTO(wah_type_spec_parse_struct(&p, &td, &alloc), cleanup);
        out->kind = WAH_DEBUG_TYPE_SPEC_STRUCT;
        out->field_count = td.field_count;
        out->field_types = td.field_types;
        out->field_mutables = td.field_mutables;
        td.field_count = 0;
        td.field_types = NULL;
        td.field_mutables = NULL;
#else
        err = WAH_ERROR_DISABLED_FEATURE; goto cleanup;
#endif
    } else if (wah_type_spec_take_kw(&p, "array")) {
#if ((WAH_COMPILED_FEATURES) & WAH_FEATURE_GC)
        p.cur = spec;
        td.kind = WAH_COMP_ARRAY;
        WAH_CHECK_GOTO(wah_type_spec_parse_array(&p, &td, &alloc), cleanup);
        out->kind = WAH_DEBUG_TYPE_SPEC_ARRAY;
        out->field_count = td.field_count;
        out->field_types = td.field_types;
        out->field_mutables = td.field_mutables;
        td.field_count = 0;
        td.field_types = NULL;
        td.field_mutables = NULL;
#else
        err = WAH_ERROR_DISABLED_FEATURE; goto cleanup;
#endif
    } else {
        err = WAH_ERROR_BAD_SPEC;
        goto cleanup;
    }

    wah_type_spec_skip_ws(&p);
    WAH_ENSURE_GOTO(*p.cur == '\0', WAH_ERROR_BAD_SPEC, cleanup);
    return WAH_OK;

cleanup:
    free(ft.param_types);
    free(ft.result_types);
    free(td.field_types);
    free(td.field_mutables);
    wah_debug_free_type_spec(out);
    return err == WAH_OK ? WAH_ERROR_BAD_SPEC : err;
}

wah_error_t wah_debug_parse_define_type_spec(wah_module_t *mod, wah_debug_type_spec_t *out, const char *spec, ...) {
    wah_error_t err;
    va_list args;
    va_start(args, spec);
    err = wah_debug_parse_define_type_specv(mod, out, spec, &args);
    va_end(args);
    return err;
}

wah_error_t wah_debug_parse_export_func_type_spec(wah_module_t *mod, wah_debug_type_spec_t *out, const char *spec) {
    wah_error_t err;
    wah_alloc_t alloc = wah_resolve_alloc(NULL);
    wah_type_spec_parser_t p;
    wah_func_type_t ft = {0};

    WAH_ENSURE(mod && out && spec, WAH_ERROR_MISUSE);
    *out = (wah_debug_type_spec_t){0};
    p = (wah_type_spec_parser_t){ .cur = spec, .module = mod, .allow_placeholders = false };
    bool has_fn = wah_type_spec_take_kw(&p, "fn");
    p.cur = spec;
    WAH_CHECK_GOTO(wah_type_spec_parse_func(&p, &ft, !has_fn, &alloc), cleanup);
    wah_type_spec_skip_ws(&p);
    WAH_ENSURE_GOTO(*p.cur == '\0', WAH_ERROR_BAD_SPEC, cleanup);

    out->kind = WAH_DEBUG_TYPE_SPEC_FUNC;
    out->param_count = ft.param_count;
    out->param_types = ft.param_types;
    out->result_count = ft.result_count;
    out->result_types = ft.result_types;
    ft = (wah_func_type_t){0};
    return WAH_OK;

cleanup:
    free(ft.param_types);
    free(ft.result_types);
    wah_debug_free_type_spec(out);
    return err == WAH_OK ? WAH_ERROR_BAD_SPEC : err;
}

void wah_debug_gc_step(wah_exec_context_t *ctx) {
    wah_gc_step(ctx);
}
