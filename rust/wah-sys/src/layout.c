// Sizes and offsets of the public types, against which the Rust declarations are tested.
#include <stddef.h>
#include "wah.h"

#define WAH_SYS_LAYOUT(T) sizeof(T), WAH_ALIGNOF(T)

const size_t wah_sys_layout[] = {
    WAH_SYS_LAYOUT(wah_module_t),
    WAH_SYS_LAYOUT(wah_exec_context_t),
    WAH_SYS_LAYOUT(wah_value_t),
    WAH_SYS_LAYOUT(wah_v128_t),
    WAH_SYS_LAYOUT(wah_alloc_t),
    WAH_SYS_LAYOUT(wah_type_desc_t),
    offsetof(wah_type_desc_t, field_mutables),
    WAH_SYS_LAYOUT(wah_func_desc_t),
    offsetof(wah_func_desc_t, result_types),
    WAH_SYS_LAYOUT(wah_global_desc_t),
    WAH_SYS_LAYOUT(wah_memory_desc_t),
    WAH_SYS_LAYOUT(wah_table_desc_t),
    WAH_SYS_LAYOUT(wah_tag_desc_t),
    WAH_SYS_LAYOUT(wah_extern_desc_t),
    WAH_SYS_LAYOUT(wah_import_desc_t),
    offsetof(wah_import_desc_t, name_len),
    offsetof(wah_import_desc_t, u),
    WAH_SYS_LAYOUT(wah_export_desc_t),
    offsetof(wah_export_desc_t, u),
    WAH_SYS_LAYOUT(wah_parse_options_t),
    offsetof(wah_parse_options_t, disable_jit),
    WAH_SYS_LAYOUT(wah_limits_t),
    offsetof(wah_limits_t, no_memory_bytes),
    WAH_SYS_LAYOUT(wah_exec_options_t),
    offsetof(wah_exec_options_t, memory_creator), offsetof(wah_exec_options_t, userdata),
    offsetof(wah_exec_options_t, table_limiter),
    WAH_SYS_LAYOUT(wah_memory_request_t),
    offsetof(wah_memory_request_t, is_64), offsetof(wah_memory_request_t, reserve_bytes),
    WAH_SYS_LAYOUT(wah_host_memory_t),
    offsetof(wah_host_memory_t, guarded),
    WAH_SYS_LAYOUT(wah_memory_creator_t),
    WAH_SYS_LAYOUT(wah_limiter_t),
    offsetof(wah_limiter_t, release), offsetof(wah_limiter_t, userdata),
    WAH_SYS_LAYOUT(wah_gc_heap_stats_t),
    offsetof(wah_gc_heap_stats_t, total_polls),
    sizeof(wah_error_t), sizeof(wah_exec_state_t), sizeof(wah_comp_type_kind_t),
    sizeof(wah_fault_handling_t), sizeof(wah_gc_phase_t),
};
const size_t wah_sys_layout_len = sizeof(wah_sys_layout) / sizeof(wah_sys_layout[0]);
