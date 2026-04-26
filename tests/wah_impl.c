#define WAH_IMPLEMENTATION
#include "../wah.h"
#include "wah_impl.h"
#include <string.h>
#include <stdlib.h>

uint32_t wah_debug_wasm_page_size(void) { return WAH_WASM_PAGE_SIZE; }
wah_type_t wah_debug_type_host_function(void) { return WAH_TYPE_HOST_FUNCTION; }
uint32_t wah_debug_no_supertype(void) { return WAH_NO_SUPERTYPE; }
uint32_t wah_debug_entry_kind_function(void) { return WAH_ENTRY_KIND_FUNCTION; }
uint32_t wah_debug_entry_kind_table(void) { return WAH_ENTRY_KIND_TABLE; }
uint32_t wah_debug_entry_kind_memory(void) { return WAH_ENTRY_KIND_MEMORY; }
uint32_t wah_debug_entry_kind_global(void) { return WAH_ENTRY_KIND_GLOBAL; }

wah_entry_id_t wah_debug_make_entry_id(uint32_t kind, uint32_t index) {
    return WAH_MAKE_ENTRY_ID(kind, index);
}
uint32_t wah_debug_get_entry_kind(wah_entry_id_t id) { return WAH_GET_ENTRY_KIND(id); }
uint32_t wah_debug_get_entry_index(wah_entry_id_t id) { return WAH_GET_ENTRY_INDEX(id); }

uint8_t *wah_debug_memory_data(const wah_exec_context_t *ctx, uint32_t mem_idx) {
    return ctx->memories[mem_idx].data;
}
uint64_t wah_debug_memory_size(const wah_exec_context_t *ctx, uint32_t mem_idx) {
    return ctx->memories[mem_idx].size;
}
wah_error_t wah_debug_memory_type(const wah_module_t *mod, uint32_t idx,
    wah_type_t *out_addr_type, uint64_t *out_min_pages, uint64_t *out_max_pages)
{
    if (idx >= mod->memory_count + mod->import_memory_count) return WAH_ERROR_NOT_FOUND;
    if (out_addr_type) *out_addr_type = mod->memories[idx].addr_type;
    if (out_min_pages) *out_min_pages = mod->memories[idx].min_pages;
    if (out_max_pages) *out_max_pages = mod->memories[idx].max_pages;
    return WAH_OK;
}

wah_error_t wah_debug_table_type(const wah_module_t *mod, uint32_t idx,
    wah_type_t *out_elem_type, uint64_t *out_min_elements, uint64_t *out_max_elements)
{
    if (idx >= mod->table_count + mod->import_table_count) return WAH_ERROR_NOT_FOUND;
    if (out_elem_type) *out_elem_type = mod->tables[idx].elem_type;
    if (out_min_elements) *out_min_elements = mod->tables[idx].min_elements;
    if (out_max_elements) *out_max_elements = mod->tables[idx].max_elements;
    return WAH_OK;
}
uint32_t wah_debug_table_size(const wah_exec_context_t *ctx, uint32_t tbl_idx) {
    return ctx->tables[tbl_idx].size;
}

wah_error_t wah_debug_global_def(const wah_module_t *mod, uint32_t idx,
    wah_type_t *out_type, bool *out_is_mutable)
{
    if (idx >= mod->global_count) return WAH_ERROR_NOT_FOUND;
    if (out_type) *out_type = mod->globals[idx].type;
    if (out_is_mutable) *out_is_mutable = mod->globals[idx].is_mutable;
    return WAH_OK;
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

wah_error_t wah_debug_func_type(const wah_module_t *mod, uint32_t type_idx,
    uint32_t *out_param_count, const wah_type_t **out_param_types,
    const wah_type_flags_t **out_param_type_flags,
    uint32_t *out_result_count, const wah_type_t **out_result_types,
    const wah_type_flags_t **out_result_type_flags)
{
    if (type_idx >= mod->type_count) return WAH_ERROR_NOT_FOUND;
    if (out_param_count) *out_param_count = mod->types[type_idx].param_count;
    if (out_param_types) *out_param_types = mod->types[type_idx].param_types;
    if (out_param_type_flags) *out_param_type_flags = mod->types[type_idx].param_type_flags;
    if (out_result_count) *out_result_count = mod->types[type_idx].result_count;
    if (out_result_types) *out_result_types = mod->types[type_idx].result_types;
    if (out_result_type_flags) *out_result_type_flags = mod->types[type_idx].result_type_flags;
    return WAH_OK;
}

uint32_t wah_debug_type_def_supertype(const wah_module_t *mod, uint32_t type_idx) {
    if (!mod->type_defs || type_idx >= mod->type_count) return UINT32_MAX;
    return mod->type_defs[type_idx].supertype;
}

wah_error_t wah_debug_element_segment(const wah_module_t *mod, uint32_t idx,
    wah_type_t *out_elem_type, wah_type_flags_t *out_elem_type_flags)
{
    if (idx >= mod->element_segment_count) return WAH_ERROR_NOT_FOUND;
    if (out_elem_type) *out_elem_type = mod->element_segments[idx].elem_type;
    if (out_elem_type_flags) *out_elem_type_flags = mod->element_segments[idx].elem_type_flags;
    return WAH_OK;
}

bool wah_debug_is_mutable_import_global(const wah_module_t *mod, uint32_t idx) {
    if (idx >= mod->import_global_count) return false;
    return mod->global_imports[idx].is_mutable;
}

wah_error_t wah_debug_module_export_table(wah_module_t *mod, const char *name,
    wah_type_t elem_type, wah_type_flags_t elem_type_flags,
    wah_type_t addr_type, uint64_t min_elements, uint64_t max_elements)
{
    wah_table_type_t *new_tables = (wah_table_type_t *)realloc(
        mod->tables, (mod->table_count + 1) * sizeof(*mod->tables));
    if (!new_tables) return WAH_ERROR_OUT_OF_MEMORY;
    mod->tables = new_tables;
    mod->tables[mod->table_count] = (wah_table_type_t){0};
    mod->tables[mod->table_count].elem_type = elem_type;
    mod->tables[mod->table_count].elem_type_flags = elem_type_flags;
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

wah_error_t wah_debug_parse_func_spec(const char *types,
    size_t *out_nparams, wah_type_t **out_param_types,
    size_t *out_nresults, wah_type_t **out_result_types)
{
    return wah_parse_func_spec(types, out_nparams, out_param_types, out_nresults, out_result_types);
}
