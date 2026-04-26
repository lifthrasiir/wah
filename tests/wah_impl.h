// Debug/test wrapper API for accessing wah.h internals without WAH_IMPLEMENTATION.
// Implemented in wah_impl.c (which defines WAH_IMPLEMENTATION).

#ifndef WAH_IMPL_H
#define WAH_IMPL_H

#include "../wah.h"

// --- Constants as functions ---
uint32_t wah_debug_wasm_page_size(void);
wah_type_t wah_debug_type_host_function(void);
uint32_t wah_debug_no_supertype(void);
uint32_t wah_debug_entry_kind_function(void);
uint32_t wah_debug_entry_kind_table(void);
uint32_t wah_debug_entry_kind_memory(void);
uint32_t wah_debug_entry_kind_global(void);

// --- Entry ID helpers ---
wah_entry_id_t wah_debug_make_entry_id(uint32_t kind, uint32_t index);
uint32_t wah_debug_get_entry_kind(wah_entry_id_t id);
uint32_t wah_debug_get_entry_index(wah_entry_id_t id);

// --- Memory accessors ---
uint8_t *wah_debug_memory_data(const wah_exec_context_t *ctx, uint32_t mem_idx);
uint64_t wah_debug_memory_size(const wah_exec_context_t *ctx, uint32_t mem_idx);
wah_error_t wah_debug_memory_type(const wah_module_t *mod, uint32_t idx,
    wah_type_t *out_addr_type, uint64_t *out_min_pages, uint64_t *out_max_pages);

// --- Table accessors ---
wah_error_t wah_debug_table_type(const wah_module_t *mod, uint32_t idx,
    wah_type_t *out_elem_type, uint64_t *out_min_elements, uint64_t *out_max_elements);
uint32_t wah_debug_table_size(const wah_exec_context_t *ctx, uint32_t tbl_idx);

// --- Global accessors ---
wah_error_t wah_debug_global_def(const wah_module_t *mod, uint32_t idx,
    wah_type_t *out_type, bool *out_is_mutable);
wah_value_t wah_debug_global_value(const wah_exec_context_t *ctx,
    const wah_module_t *mod, uint32_t global_idx);

// --- Func type accessors ---
wah_error_t wah_debug_func_type(const wah_module_t *mod, uint32_t type_idx,
    uint32_t *out_param_count, const wah_type_t **out_param_types,
    const wah_type_flags_t **out_param_type_flags,
    uint32_t *out_result_count, const wah_type_t **out_result_types,
    const wah_type_flags_t **out_result_type_flags);

// --- Type def accessors ---
uint32_t wah_debug_type_def_supertype(const wah_module_t *mod, uint32_t type_idx);

// --- Element segment accessors ---
wah_error_t wah_debug_element_segment(const wah_module_t *mod, uint32_t idx,
    wah_type_t *out_elem_type, wah_type_flags_t *out_elem_type_flags);

// --- Import global info ---
bool wah_debug_is_mutable_import_global(const wah_module_t *mod, uint32_t idx);

// --- Module export table (programmatic table export creation) ---
wah_error_t wah_debug_module_export_table(wah_module_t *mod, const char *name,
    wah_type_t elem_type, wah_type_flags_t elem_type_flags,
    wah_type_t addr_type, uint64_t min_elements, uint64_t max_elements);

// --- Func ref range check (for spectest) ---
bool wah_debug_is_func_ref_in_ctx(const wah_exec_context_t *ctx, const void *ref);
bool wah_debug_is_func_ref_in_module(const wah_module_t *mod, const void *ref);

// --- Linked module accessors ---
const wah_module_t *wah_debug_linked_module(const wah_exec_context_t *ctx, uint32_t i);
const wah_exec_context_t *wah_debug_linked_ctx(const wah_exec_context_t *ctx, uint32_t i);

// --- Linked module patching (for spectest realloc fixups) ---
void wah_debug_set_linked_module(wah_exec_context_t *ctx, uint32_t i, const wah_module_t *mod);
void wah_debug_set_linked_ctx(wah_exec_context_t *ctx, uint32_t i, wah_exec_context_t *lctx);

// --- Platform feature detection ---
void wah_debug_print_platform_features(void);

// --- parse_func_spec ---
wah_error_t wah_debug_parse_func_spec(const char *types,
    size_t *out_nparams, wah_type_t **out_param_types,
    size_t *out_nresults, wah_type_t **out_result_types);

#endif
