// Debug/test wrapper API for accessing wah.h internals without WAH_IMPLEMENTATION.
// Implemented in wah_impl.c (which defines WAH_IMPLEMENTATION).

#ifndef WAH_IMPL_H
#define WAH_IMPL_H

#include "../wah.h"

// --- Constants as functions ---
uint32_t wah_debug_wasm_page_size(void);

// --- Memory accessors ---
uint8_t *wah_debug_memory_data(const wah_exec_context_t *ctx, uint32_t mem_idx);
uint64_t wah_debug_memory_size(const wah_exec_context_t *ctx, uint32_t mem_idx);
wah_error_t wah_debug_replace_data_segment_fill(wah_module_t *mod,
    uint32_t data_idx, uint32_t size, uint8_t value);

// --- Table accessors ---
uint64_t wah_debug_table_size(const wah_exec_context_t *ctx, uint32_t tbl_idx);

// --- Global accessors ---
wah_value_t wah_debug_global_value(const wah_exec_context_t *ctx,
    const wah_module_t *mod, uint32_t global_idx);

// --- Element segment accessors ---
wah_error_t wah_debug_element_segment(const wah_module_t *mod, uint32_t idx,
    wah_type_t *out_elem_type);

// --- Module export table (programmatic table export creation) ---
wah_error_t wah_debug_module_export_table(wah_module_t *mod, const char *name,
    wah_type_t elem_type,
    wah_type_t addr_type, uint64_t min_elements, uint64_t max_elements);

// --- Func ref range check (for spectest) ---
bool wah_debug_is_func_ref_in_ctx(const wah_exec_context_t *ctx, const void *ref);
bool wah_debug_is_func_ref_in_module(const wah_module_t *mod, const void *ref);
bool wah_debug_type_is_exnref(const wah_module_t *mod, wah_type_t type);
void wah_debug_free_exnref(wah_exec_context_t *ctx, void *ref);

// --- Linked module accessors ---
const wah_module_t *wah_debug_linked_module(const wah_exec_context_t *ctx, uint32_t i);
const wah_exec_context_t *wah_debug_linked_ctx(const wah_exec_context_t *ctx, uint32_t i);

// --- Linked module patching (for spectest realloc fixups) ---
void wah_debug_set_linked_module(wah_exec_context_t *ctx, uint32_t i, const wah_module_t *mod);
void wah_debug_set_linked_ctx(wah_exec_context_t *ctx, uint32_t i, wah_exec_context_t *lctx);
void wah_debug_relocate_exec_refs(wah_exec_context_t *ctx, void *old_base, size_t byte_size, ptrdiff_t delta);

typedef struct {
    uint32_t offset;
    int32_t repr_id;
} wah_debug_repr_field_t;

typedef struct {
    uint32_t type;
    uint32_t typeidx;
    uint32_t size;
    uint32_t count;
} wah_debug_repr_info_t;

wah_error_t wah_debug_repr_info(const wah_module_t *mod, wah_type_t type,
    wah_debug_repr_info_t *out_info, wah_debug_repr_field_t *out_fields, uint32_t max_fields);

// --- Platform feature detection ---
void wah_debug_print_platform_features(void);

// --- parse_func_spec ---
wah_error_t wah_debug_parse_func_spec(const char *types,
    size_t *out_nparams, wah_type_t **out_param_types,
    size_t *out_nresults, wah_type_t **out_result_types);

typedef enum {
    WAH_DEBUG_TYPE_SPEC_FUNC = 1,
    WAH_DEBUG_TYPE_SPEC_STRUCT = 2,
    WAH_DEBUG_TYPE_SPEC_ARRAY = 3,
} wah_debug_type_spec_kind_t;

typedef struct {
    wah_debug_type_spec_kind_t kind;
    uint32_t param_count;
    wah_type_t *param_types;
    uint32_t result_count;
    wah_type_t *result_types;
    uint32_t field_count;
    wah_type_t *field_types;
    bool *field_mutables;
} wah_debug_type_spec_t;

// --- type spec parser ---
wah_error_t wah_debug_parse_define_type_spec(wah_module_t *mod, wah_debug_type_spec_t *out, const char *spec, ...);
wah_error_t wah_debug_parse_export_func_type_spec(wah_module_t *mod, wah_debug_type_spec_t *out, const char *spec);
void wah_debug_free_type_spec(wah_debug_type_spec_t *spec);

#endif
