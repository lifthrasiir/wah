// Regression tests verifying that wah_module_t is read-only after parse and
// can be safely shared across multiple wah_exec_context_t instances. These
// were added when migrating per-instance state (function fn_ctx, segment
// dropped state, data.drop free) off of wah_module_t.
//
// We need WAH_IMPLEMENTATION so we can directly inspect module internals
// (functions[], element_segments[], data_segments[]) and verify they don't
// mutate during instance ops.

#define WAH_IMPLEMENTATION
#include "../wah.h"
#include "common.h"
#include <stdio.h>
#include <string.h>

// Snapshot a few module fields that the runtime previously mutated so we can
// assert they did not change after instance ops.
typedef struct {
    uint32_t local_function_count;
    uint32_t functions_local_idx[16];
    uint32_t functions_global_idx[16];
    const struct wah_module_s *functions_fn_module[16];
    uint32_t elem_num_elems[16];
    uint32_t data_data_len[16];
    const uint8_t *data_data_ptr[16];
} module_snapshot_t;

static void snapshot_module(module_snapshot_t *snap, const wah_module_t *mod) {
    memset(snap, 0, sizeof(*snap));
    snap->local_function_count = mod->local_function_count;
    uint32_t fn = mod->local_function_count < 16 ? mod->local_function_count : 16;
    for (uint32_t i = 0; i < fn; i++) {
        snap->functions_local_idx[i] = mod->functions[i].local_idx;
        snap->functions_global_idx[i] = mod->functions[i].global_idx;
        snap->functions_fn_module[i] = mod->functions[i].fn_module;
    }
    uint32_t es = mod->element_segment_count < 16 ? mod->element_segment_count : 16;
    for (uint32_t i = 0; i < es; i++) {
        snap->elem_num_elems[i] = mod->element_segments[i].num_elems;
    }
    uint32_t ds = mod->data_segment_count < 16 ? mod->data_segment_count : 16;
    for (uint32_t i = 0; i < ds; i++) {
        snap->data_data_len[i] = mod->data_segments[i].data_len;
        snap->data_data_ptr[i] = mod->data_segments[i].data;
    }
}

static void assert_module_unchanged(const module_snapshot_t *snap, const wah_module_t *mod, const char *where) {
    if (mod->local_function_count != snap->local_function_count) {
        fprintf(stderr, "%s: local_function_count changed\n", where);
        exit(1);
    }
    uint32_t fn = mod->local_function_count < 16 ? mod->local_function_count : 16;
    for (uint32_t i = 0; i < fn; i++) {
        if (mod->functions[i].local_idx != snap->functions_local_idx[i] ||
            mod->functions[i].global_idx != snap->functions_global_idx[i] ||
            mod->functions[i].fn_module != snap->functions_fn_module[i]) {
            fprintf(stderr, "%s: module->functions[%u] mutated\n", where, i);
            exit(1);
        }
    }
    uint32_t es = mod->element_segment_count < 16 ? mod->element_segment_count : 16;
    for (uint32_t i = 0; i < es; i++) {
        if (mod->element_segments[i].num_elems != snap->elem_num_elems[i]) {
            fprintf(stderr, "%s: module->element_segments[%u].num_elems mutated\n", where, i);
            exit(1);
        }
    }
    uint32_t ds = mod->data_segment_count < 16 ? mod->data_segment_count : 16;
    for (uint32_t i = 0; i < ds; i++) {
        if (mod->data_segments[i].data_len != snap->data_data_len[i] ||
            mod->data_segments[i].data != snap->data_data_ptr[i]) {
            fprintf(stderr, "%s: module->data_segments[%u] mutated\n", where, i);
            exit(1);
        }
    }
}

// Active element segment + active data segment + funcref-via-table dispatch.
// All three must work in a SECOND instance of the same module.
static void test_active_segments_in_both_instances() {
    printf("Running test_active_segments_in_both_instances...\n");

    // - active data segment writes 0xAA at memory[0]
    // - active element segment puts func 0 at table[0]
    // - func 0 returns 42; func 1 = read mem[0]; func 2 = call_indirect(0)
    const char *spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0, 0, 0 ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        memories {[ limits.i32/1 1 ]} \
        exports {[ {'read_byte'} fn# 1, {'call0'} fn# 2 ]} \
        elements {[ elem.active.table#0 i32.const 0 end [0] ]} \
        code {[ \
            {[] i32.const 42 end}, \
            {[] i32.const 0 i32.load8_u align=1 0 end}, \
            {[] i32.const 0 call_indirect 0 0 end} \
        ]} \
        data {[ data.active.table#0 i32.const 0 end {%'aa'} ]}";

    wah_module_t mod = {0};
    assert_ok(wah_parse_module_from_spec(&mod, spec));

    module_snapshot_t snap;
    snapshot_module(&snap, &mod);

    wah_export_desc_t e_read, e_call;
    assert_ok(wah_export_by_name(&mod, "read_byte", &e_read));
    assert_ok(wah_export_by_name(&mod, "call0", &e_call));

    wah_value_t r;
    wah_exec_context_t a = {0};
    assert_ok(wah_new_exec_context(&a, &mod, NULL));
    assert_ok(wah_instantiate(&a));
    assert_ok(wah_call(&a, (uint32_t)e_read.index, NULL, 0, &r));
    assert_eq_i32(r.i32 & 0xFF, 0xAA);
    assert_ok(wah_call(&a, (uint32_t)e_call.index, NULL, 0, &r));
    assert_eq_i32(r.i32, 42);

    assert_module_unchanged(&snap, &mod, "after first instance");

    wah_exec_context_t b = {0};
    assert_ok(wah_new_exec_context(&b, &mod, NULL));
    assert_ok(wah_instantiate(&b));
    assert_ok(wah_call(&b, (uint32_t)e_read.index, NULL, 0, &r));
    assert_eq_i32(r.i32 & 0xFF, 0xAA);
    assert_ok(wah_call(&b, (uint32_t)e_call.index, NULL, 0, &r));
    assert_eq_i32(r.i32, 42);

    assert_module_unchanged(&snap, &mod, "after second instance");

    wah_free_exec_context(&a);
    wah_free_exec_context(&b);
    wah_free_module(&mod);
}

// data.drop in instance A must not affect instance B.
static void test_data_drop_isolation() {
    printf("Running test_data_drop_isolation...\n");

    // Passive data segment "ABC". `init_byte` does memory.init(size=1) then
    // returns mem[0]. `drop` runs data.drop. After drop, init_byte must trap.
    const char *spec = "wasm \
        types {[ fn [] [], fn [] [i32] ]} \
        funcs {[ 1, 0 ]} \
        memories {[ limits.i32/1 1 ]} \
        exports {[ \
            {'init_byte'} fn# 0, \
            {'drop'} fn# 1 \
        ]} \
        datacount { 1 } \
        code {[ \
            {[] i32.const 0 i32.const 0 i32.const 1 memory.init 0 0 \
                i32.const 0 i32.load8_u align=1 0 end}, \
            {[] data.drop 0 end} \
        ]} \
        data {[ data.passive {%'414243'} ]}";

    wah_module_t mod = {0};
    assert_ok(wah_parse_module_from_spec(&mod, spec));

    module_snapshot_t snap;
    snapshot_module(&snap, &mod);

    wah_export_desc_t e_init, e_drop;
    assert_ok(wah_export_by_name(&mod, "init_byte", &e_init));
    assert_ok(wah_export_by_name(&mod, "drop", &e_drop));

    wah_exec_context_t a = {0}, b = {0};
    assert_ok(wah_new_exec_context(&a, &mod, NULL));
    assert_ok(wah_instantiate(&a));
    assert_ok(wah_new_exec_context(&b, &mod, NULL));
    assert_ok(wah_instantiate(&b));

    wah_value_t r;
    assert_ok(wah_call(&a, (uint32_t)e_init.index, NULL, 0, &r));
    assert_eq_i32(r.i32 & 0xFF, 'A');
    assert_ok(wah_call(&b, (uint32_t)e_init.index, NULL, 0, &r));
    assert_eq_i32(r.i32 & 0xFF, 'A');

    assert_ok(wah_call(&a, (uint32_t)e_drop.index, NULL, 0, NULL));

    assert_err(wah_call(&a, (uint32_t)e_init.index, NULL, 0, &r), WAH_ERROR_TRAP);
    assert_ok(wah_call(&b, (uint32_t)e_init.index, NULL, 0, &r));
    assert_eq_i32(r.i32 & 0xFF, 'A');

    assert_module_unchanged(&snap, &mod, "after data.drop in A");

    // Fresh instance after A's drop must still see the segment.
    wah_exec_context_t c = {0};
    assert_ok(wah_new_exec_context(&c, &mod, NULL));
    assert_ok(wah_instantiate(&c));
    assert_ok(wah_call(&c, (uint32_t)e_init.index, NULL, 0, &r));
    assert_eq_i32(r.i32 & 0xFF, 'A');

    assert_module_unchanged(&snap, &mod, "after fresh instantiate post-drop");

    wah_free_exec_context(&a);
    wah_free_exec_context(&b);
    wah_free_exec_context(&c);
    wah_free_module(&mod);
}

// elem.drop in instance A must not affect instance B.
static void test_elem_drop_isolation() {
    printf("Running test_elem_drop_isolation...\n");

    // Passive element segment with func 0 (returns 7). `call_via_init` does
    // table.init then call_indirect. `drop` runs elem.drop.
    const char *spec = "wasm \
        types {[ fn [] [i32], fn [] [] ]} \
        funcs {[ 0, 0, 1 ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        exports {[ {'call_via_init'} fn# 1, {'drop'} fn# 2 ]} \
        elements {[ elem.passive elem.funcref [0] ]} \
        code {[ \
            {[] i32.const 7 end}, \
            {[] i32.const 0 i32.const 0 i32.const 1 table.init 0 0 \
                i32.const 0 call_indirect 0 0 end}, \
            {[] elem.drop 0 end} \
        ]}";

    wah_module_t mod = {0};
    assert_ok(wah_parse_module_from_spec(&mod, spec));

    module_snapshot_t snap;
    snapshot_module(&snap, &mod);

    wah_export_desc_t e_init, e_drop;
    assert_ok(wah_export_by_name(&mod, "call_via_init", &e_init));
    assert_ok(wah_export_by_name(&mod, "drop", &e_drop));

    wah_exec_context_t a = {0}, b = {0};
    assert_ok(wah_new_exec_context(&a, &mod, NULL));
    assert_ok(wah_instantiate(&a));
    assert_ok(wah_new_exec_context(&b, &mod, NULL));
    assert_ok(wah_instantiate(&b));

    wah_value_t r;
    assert_ok(wah_call(&a, (uint32_t)e_init.index, NULL, 0, &r));
    assert_eq_i32(r.i32, 7);
    assert_ok(wah_call(&b, (uint32_t)e_init.index, NULL, 0, &r));
    assert_eq_i32(r.i32, 7);

    assert_ok(wah_call(&a, (uint32_t)e_drop.index, NULL, 0, NULL));

    assert_err(wah_call(&a, (uint32_t)e_init.index, NULL, 0, &r), WAH_ERROR_TRAP);
    assert_ok(wah_call(&b, (uint32_t)e_init.index, NULL, 0, &r));
    assert_eq_i32(r.i32, 7);

    assert_module_unchanged(&snap, &mod, "after elem.drop in A");

    wah_free_exec_context(&a);
    wah_free_exec_context(&b);
    wah_free_module(&mod);
}

// Two instances dispatching funcref-via-table from the same module, interleaved.
static void test_funcref_dispatch_two_instances() {
    printf("Running test_funcref_dispatch_two_instances...\n");

    const char *spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0, 0 ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        exports {[ {'do_call'} fn# 1 ]} \
        elements {[ elem.active.table#0 i32.const 0 end [0] ]} \
        code {[ \
            {[] i32.const 123 end}, \
            {[] i32.const 0 call_indirect 0 0 end} \
        ]}";

    wah_module_t mod = {0};
    assert_ok(wah_parse_module_from_spec(&mod, spec));

    module_snapshot_t snap;
    snapshot_module(&snap, &mod);

    wah_export_desc_t e_do;
    assert_ok(wah_export_by_name(&mod, "do_call", &e_do));

    wah_exec_context_t a = {0}, b = {0};
    assert_ok(wah_new_exec_context(&a, &mod, NULL));
    assert_ok(wah_instantiate(&a));
    assert_ok(wah_new_exec_context(&b, &mod, NULL));
    assert_ok(wah_instantiate(&b));

    wah_value_t r;
    for (int i = 0; i < 8; i++) {
        wah_exec_context_t *ctx = (i & 1) ? &b : &a;
        assert_ok(wah_call(ctx, (uint32_t)e_do.index, NULL, 0, &r));
        assert_eq_i32(r.i32, 123);
    }

    assert_module_unchanged(&snap, &mod, "after interleaved funcref calls");

    wah_free_exec_context(&a);
    wah_free_exec_context(&b);
    wah_free_module(&mod);
}

int main() {
    printf("Testing wah_module_t reuse across instances...\n\n");
    test_active_segments_in_both_instances();
    test_data_drop_isolation();
    test_elem_drop_isolation();
    test_funcref_dispatch_two_instances();
    printf("\nAll module-reuse tests passed!\n");
    return 0;
}
