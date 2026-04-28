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
    wah_table_type_t *new_tables = (wah_table_type_t *)realloc(
        mod->tables, (mod->table_count + 1) * sizeof(*mod->tables));
    if (!new_tables) return WAH_ERROR_OUT_OF_MEMORY;
    mod->tables = new_tables;
    mod->tables[mod->table_count] = (wah_table_type_t){0};
    mod->tables[mod->table_count].elem_type = elem_type;
    mod->tables[mod->table_count].addr_type = addr_type;
    mod->tables[mod->table_count].min_elements = min_elements;
    mod->tables[mod->table_count].max_elements = max_elements;

    wah_export_t *new_exports = (wah_export_t *)realloc(
        mod->exports, (mod->export_count + 1) * sizeof(*mod->exports));
    if (!new_exports) return WAH_ERROR_OUT_OF_MEMORY;
    mod->exports = new_exports;
    char *name_copy = strdup(name);
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
    WAH_CHECK_GOTO(wah_type_spec_parse_func(&p, &ft, !has_fn), cleanup);
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
    wah_type_spec_parser_t p;
    wah_func_type_t ft = {0};
    wah_type_def_t td = { .kind = WAH_COMP_FUNC, .is_final = true, .supertype = WAH_NO_SUPERTYPE };

    WAH_ENSURE(mod && out && spec, WAH_ERROR_MISUSE);
    *out = (wah_debug_type_spec_t){0};
    p = (wah_type_spec_parser_t){ .cur = spec, .module = mod, .args = args, .allow_placeholders = true };

    if (wah_type_spec_take_kw(&p, "fn")) {
        p.cur = spec;
        td.kind = WAH_COMP_FUNC;
        WAH_CHECK_GOTO(wah_type_spec_parse_func(&p, &ft, false), cleanup);
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
        WAH_CHECK_GOTO(wah_type_spec_parse_struct(&p, &td), cleanup);
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
        WAH_CHECK_GOTO(wah_type_spec_parse_array(&p, &td), cleanup);
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
    wah_type_spec_parser_t p;
    wah_func_type_t ft = {0};

    WAH_ENSURE(mod && out && spec, WAH_ERROR_MISUSE);
    *out = (wah_debug_type_spec_t){0};
    p = (wah_type_spec_parser_t){ .cur = spec, .module = mod, .allow_placeholders = false };
    bool has_fn = wah_type_spec_take_kw(&p, "fn");
    p.cur = spec;
    WAH_CHECK_GOTO(wah_type_spec_parse_func(&p, &ft, !has_fn), cleanup);
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
