#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../wah.h"
#include "wah_impl.h"
#include "common.h"

void wah_test_table_indirect_call() {
    printf("Running wah_test_table_indirect_call...\n");

    wah_module_t module;
    const char *indirect_call_spec = "wasm \
        types {[ fn [i32] [i32] ]} \
        funcs {[ 0, 0, 0, 0 ]} \
        tables {[ funcref limits.i32/1 2 ]} \
        exports {[ \
            {'add_one'} fn# 0, \
            {'sub_one'} fn# 1, \
            {'call_indirect_add'} fn# 2, \
            {'call_indirect_sub'} fn# 3, \
        ]} \
        elements {[ elem.active.table#0 i32.const 0 end [0, 1] ]} \
        code {[ \
            {[] local.get 0 i32.const 1 i32.add end}, \
            {[] local.get 0 i32.const 1 i32.sub end}, \
            {[] local.get 0 i32.const 0 call_indirect 0 0 end}, \
            {[] local.get 0 i32.const 1 call_indirect 0 0 end}, \
        ]}";
    assert_ok(wah_parse_module_from_spec(&module, indirect_call_spec));

    wah_exec_context_t exec_ctx;
    assert_ok(wah_exec_context_create(&exec_ctx, &module, NULL));

    // Find the exported function indices by name
    wah_export_desc_t entry;
    assert_ok(wah_module_export_by_name(&module, "call_indirect_add", &entry));
    uint32_t call_indirect_add_func_idx = (uint32_t)entry.index;

    assert_ok(wah_module_export_by_name(&module, "call_indirect_sub", &entry));
    uint32_t call_indirect_sub_func_idx = (uint32_t)entry.index;

    // Test call_indirect_add (calls add_one indirectly)
    // The first wah_call call should automatically call wah_instantiate.
    wah_value_t params_add[1] = {{.i32 = 10}};
    wah_value_t result_add;
    assert_ok(wah_call(&exec_ctx, call_indirect_add_func_idx, params_add, 1, &result_add));
    assert(result_add.i32 == 11);

    // Test call_indirect_sub (calls sub_one indirectly)
    wah_value_t params_sub[1] = {{.i32 = 10}};
    wah_value_t result_sub;
    assert_ok(wah_call(&exec_ctx, call_indirect_sub_func_idx, params_sub, 1, &result_sub));
    assert(result_sub.i32 == 9);

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
}

void wah_test_table_size() {
    printf("Running wah_test_table_size...\n");

    wah_module_t module;
    const char *table_size_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 10, funcref limits.i32/1 20 ]} \
        exports {[ {'get_table_size'} fn# 0 ]} \
        code {[ \
            {[] table.size 0 i32.const 7 i32.mul table.size 1 i32.add end}, \
        ]}";
    assert_ok(wah_parse_module_from_spec(&module, table_size_spec));

    wah_exec_context_t exec_ctx;
    assert_ok(wah_exec_context_create(&exec_ctx, &module, NULL));

    wah_export_desc_t entry;
    assert_ok(wah_module_export_by_name(&module, "get_table_size", &entry));
    uint32_t func_idx = (uint32_t)entry.index;

    wah_value_t result;
    assert_ok(wah_call(&exec_ctx, func_idx, NULL, 0, &result));
    assert_eq_i32(result.i32, 90); // 10 * 7 + 20 = 90

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
}

void wah_test_table_grow() {
    printf("Running wah_test_table_grow...\n");

    wah_module_t module;
    const char *table_grow_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 10 ]} \
        exports {[ {'grow_table'} fn# 0 ]} \
        code {[ \
            {[] ref.null funcref i32.const 1 table.grow 0 end}, \
        ]}";
    assert_ok(wah_parse_module_from_spec(&module, table_grow_spec));

    wah_exec_context_t exec_ctx;
    assert_ok(wah_exec_context_create(&exec_ctx, &module, NULL));

    wah_export_desc_t entry;
    assert_ok(wah_module_export_by_name(&module, "grow_table", &entry));
    uint32_t func_idx = (uint32_t)entry.index;

    wah_value_t result;
    assert_ok(wah_call(&exec_ctx, func_idx, NULL, 0, &result));
    assert_eq_i32(result.i32, 10);  // Old size was 10

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
}

void wah_test_table_fill() {
    printf("Running wah_test_table_fill...\n");

    wah_module_t module;
    const char *table_fill_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 10, funcref limits.i32/1 5 ]} \
        exports {[ {'test_fill'} fn# 0 ]} \
        code {[ \
            {[] \
                i32.const 0 ref.func 0 i32.const 1 table.fill 0 \
                i32.const 0 table.get 0 ref.is_null i32.const 100 i32.mul \
                i32.const 1 table.get 0 ref.is_null i32.const 10 i32.mul \
                i32.const 0 table.get 1 ref.is_null i32.add i32.add \
            end} \
        ]}";
    assert_ok(wah_parse_module_from_spec(&module, table_fill_spec));

    wah_exec_context_t exec_ctx;
    assert_ok(wah_exec_context_create(&exec_ctx, &module, NULL));

    wah_export_desc_t entry;
    assert_ok(wah_module_export_by_name(&module, "test_fill", &entry));
    uint32_t func_idx = (uint32_t)entry.index;

    wah_value_t result;
    assert_ok(wah_call(&exec_ctx, func_idx, NULL, 0, &result));
    // Table 0 should have ref.func 0 at index 0, and the rest should be null. Table 1 should be unaffected and all null.
    assert_eq_i32(result.i32, 11);

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
}

void wah_test_table_copy() {
    printf("Running wah_test_table_copy...\n");

    // Test table.copy within the same table (overlapping regions)
    wah_module_t module;
    const char *table_copy_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 10, funcref limits.i32/1 10 ]} \
        exports {[ {'test_copy'} fn# 0 ]} \
        elements {[ elem.passive elem.funcref [0] ]} \
        code {[ \
            {[] \
                i32.const 0 i32.const 0 i32.const 1 table.init 0 0 \
                i32.const 5 i32.const 0 i32.const 1 table.copy 1 0 \
                i32.const 5 table.get 0 ref.is_null i32.const 10 i32.mul \
                i32.const 5 table.get 1 ref.is_null i32.add \
            end}, \
        ]}";
    assert_ok(wah_parse_module_from_spec(&module, table_copy_spec));

    wah_exec_context_t exec_ctx;
    assert_ok(wah_exec_context_create(&exec_ctx, &module, NULL));

    wah_export_desc_t entry;
    assert_ok(wah_module_export_by_name(&module, "test_copy", &entry));
    uint32_t func_idx = (uint32_t)entry.index;

    wah_value_t result;
    assert_ok(wah_call(&exec_ctx, func_idx, NULL, 0, &result));
    assert_eq_i32(result.i32, 10); // After copying, index 5 of table 1 should have the function reference, so not null

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
}

void wah_test_table_init() {
    printf("Running wah_test_table_init...\n");

    wah_module_t module;
    const char *table_init_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 10, funcref limits.i32/1 10 ]} \
        exports {[ {'test_init'} fn# 0 ]} \
        elements {[ elem.passive elem.funcref [0], elem.passive elem.funcref [0, 0] ]} \
        code {[ \
            {[] \
                i32.const 0 i32.const 0 i32.const 1 table.init 0 1 \
                i32.const 0 i32.const 0 i32.const 2 table.init 1 0 \
                i32.const 0 table.get 0 ref.is_null i32.const 1000 i32.mul \
                i32.const 1 table.get 0 ref.is_null i32.const 100 i32.mul \
                i32.const 0 table.get 1 ref.is_null i32.const 10 i32.mul \
                i32.const 1 table.get 1 ref.is_null i32.add i32.add i32.add \
            end}, \
        ]}";
    assert_ok(wah_parse_module_from_spec(&module, table_init_spec));

    wah_exec_context_t exec_ctx;
    assert_ok(wah_exec_context_create(&exec_ctx, &module, NULL));

    wah_export_desc_t entry;
    assert_ok(wah_module_export_by_name(&module, "test_init", &entry));
    uint32_t func_idx = (uint32_t)entry.index;

    wah_value_t result;
    assert_ok(wah_call(&exec_ctx, func_idx, NULL, 0, &result));
    assert_eq_i32(result.i32, 1); // Only index 1 of table 1 is uninitialized

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
}

void wah_test_elem_drop() {
    printf("Running wah_test_table_elem_drop...\n");

    wah_module_t module;
    const char *elem_drop_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 10 ]} \
        exports {[ {'test_drop'} fn# 0 ]} \
        elements {[ elem.passive elem.funcref [0] ]} \
        code {[ \
            {[] \
                elem.drop 0 \
                i32.const 0 i32.const 0 i32.const 1 table.init 0 0 \
                i32.const 0 call_indirect 0 0 \
            end}, \
        ]}";
    assert_ok(wah_parse_module_from_spec(&module, elem_drop_spec));

    wah_exec_context_t exec_ctx;
    assert_ok(wah_exec_context_create(&exec_ctx, &module, NULL));

    wah_export_desc_t entry;
    assert_ok(wah_module_export_by_name(&module, "test_drop", &entry));
    uint32_t func_idx = (uint32_t)entry.index;

    wah_value_t result;
    assert_err(wah_call(&exec_ctx, func_idx, NULL, 0, &result), WAH_ERROR_TRAP); // Should trap because element segment was dropped

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
}

void wah_test_elem_active_table1() {
    printf("Running wah_test_elem_active_table1...\n");

    // Test active element segment targeting table#1 (not table#0)
    wah_module_t module;
    const char *elem_active_table1_spec = "wasm \
        types {[ fn [i32] [i32] ]} \
        funcs {[ 0, 0, 0 ]} \
        tables {[ funcref limits.i32/1 5, funcref limits.i32/1 5 ]} \
        exports {[ {'test_call'} fn# 2 ]} \
        elements {[ elem.active.table# 1 i32.const 0 end elem.funcref [0, 1] ]} \
        code {[ \
            {[] local.get 0 i32.const 1 call_indirect 0 1 end}, \
            {[] local.get 0 i32.const 1 i32.sub end}, \
            {[] local.get 0 i32.const 1 i32.add end}, \
        ]}";
    assert_ok(wah_parse_module_from_spec(&module, elem_active_table1_spec));

    wah_exec_context_t exec_ctx;
    assert_ok(wah_exec_context_create(&exec_ctx, &module, NULL));

    wah_export_desc_t entry;
    assert_ok(wah_module_export_by_name(&module, "test_call", &entry));
    uint32_t func_idx = (uint32_t)entry.index;

    // Test calling function from table#1
    wah_value_t params[1] = {{.i32 = 10}};
    wah_value_t result;
    assert_ok(wah_call(&exec_ctx, func_idx, params, 1, &result));
    assert_eq_i32(result.i32, 11); // Should call add_one (func 0) from table#1

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
}

static void host_add_one(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    wah_return_i32(ctx, wah_param_i32(ctx, 0) + 1);
}

static void host_sub_one(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    wah_return_i32(ctx, wah_param_i32(ctx, 0) - 1);
}

void wah_test_elem_import_index_drift() {
    printf("Running wah_test_elem_import_index_drift...\n");

    // Test that function indices in element segments account for imports
    // Import 2 functions, so local functions start at index 2
    wah_module_t module;
    const char *elem_import_index_drift_spec = "wasm \
        types {[ fn [i32] [i32] ]} \
        imports {[ \
            {'mod'} {'add_one'} fn# 0, \
            {'mod'} {'sub_one'} fn# 0, \
        ]} \
        funcs {[ 0, 0, 0 ]} \
        tables {[ funcref limits.i32/1 5 ]} \
        exports {[ {'test_call'} fn# 4 ]} \
        elements {[ elem.active.table#0 i32.const 0 end [2, 3, 4] ]} \
        code {[ \
            {[] local.get 0 i32.const 2 call_indirect 0 0 end}, \
            {[] local.get 0 i32.const 1 i32.add end}, \
            {[] local.get 0 i32.const 2 i32.mul end}, \
        ]}";
    assert_ok(wah_parse_module_from_spec(&module, elem_import_index_drift_spec));

    // Create host module for linkage
    wah_module_t host_mod;
    assert_ok(wah_new_module(&host_mod, NULL));

    assert_ok(wah_module_export_func(&host_mod, "add_one", "(i32) -> i32", host_add_one, NULL, NULL));
    assert_ok(wah_module_export_func(&host_mod, "sub_one", "(i32) -> i32", host_sub_one, NULL, NULL));

    wah_exec_context_t exec_ctx;
    assert_ok(wah_exec_context_create(&exec_ctx, &module, NULL));
    assert_ok(wah_link_module(&exec_ctx, "mod", &host_mod));
    assert_ok(wah_instantiate(&exec_ctx));

    wah_export_desc_t entry;
    assert_ok(wah_module_export_by_name(&module, "test_call", &entry));
    uint32_t func_idx = (uint32_t)entry.index;

    // Test calling local function (index 4, which is the second local function after 2 imports)
    wah_value_t params[1] = {{.i32 = 10}};
    wah_value_t result;
    assert_ok(wah_call(&exec_ctx, func_idx, params, 1, &result));
    assert_eq_i32(result.i32, 20); // Should call multiply_by_2 (func 4) which does x*2

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
    wah_free_module(&host_mod);
}

void wah_test_elem_declarative() {
    printf("Running wah_test_elem_declarative...\n");

    // Test declarative element segments (for reference but not initialization)
    // Declarative elements don't initialize any table but declare function references
    wah_module_t module;
    const char *elem_declarative_spec = "wasm \
        types {[ fn [i32] [i32] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 5 ]} \
        exports {[ {'test_fn'} fn# 0 ]} \
        elements {[ elem.declarative elem.funcref [0] ]} \
        code {[ \
            {[] local.get 0 i32.const 1 i32.add end}, \
        ]}";
    assert_ok(wah_parse_module_from_spec(&module, elem_declarative_spec));

    wah_exec_context_t exec_ctx;
    assert_ok(wah_exec_context_create(&exec_ctx, &module, NULL));

    wah_export_desc_t entry;
    assert_ok(wah_module_export_by_name(&module, "test_fn", &entry));
    uint32_t func_idx = (uint32_t)entry.index;

    // Test that the function works correctly
    wah_value_t params[1] = {{.i32 = 10}};
    wah_value_t result;
    assert_ok(wah_call(&exec_ctx, func_idx, params, 1, &result));
    assert_eq_i32(result.i32, 11);

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
}

void wah_test_elem_expr_style() {
    printf("Running wah_test_elem_expr_style...\n");

    // Test expression-style element segments (using ref.func instead of function indices)
    wah_module_t module;
    const char *elem_expr_spec = "wasm \
        types {[ fn [i32] [i32] ]} \
        funcs {[ 0, 0, 0, 0 ]} \
        tables {[ funcref limits.i32/1 5 ]} \
        exports {[ {'call_first'} fn# 2, {'call_second'} fn# 3 ]} \
        elements {[ elem.active.expr.table#0 i32.const 0 end [ref.func 0 end, ref.func 1 end] ]} \
        code {[ \
            {[] local.get 0 i32.const 1 i32.add end}, \
            {[] local.get 0 i32.const 2 i32.add end}, \
            {[] local.get 0 i32.const 0 call_indirect 0 0 end}, \
            {[] local.get 0 i32.const 1 call_indirect 0 0 end}, \
        ]}";
    assert_ok(wah_parse_module_from_spec(&module, elem_expr_spec));

    wah_exec_context_t exec_ctx;
    assert_ok(wah_exec_context_create(&exec_ctx, &module, NULL));

    wah_export_desc_t entry;
    assert_ok(wah_module_export_by_name(&module, "call_first", &entry));
    uint32_t first_func_idx = (uint32_t)entry.index;

    assert_ok(wah_module_export_by_name(&module, "call_second", &entry));
    uint32_t second_func_idx = (uint32_t)entry.index;

    // Test calling first function via table
    wah_value_t params[1] = {{.i32 = 10}};
    wah_value_t result;
    assert_ok(wah_call(&exec_ctx, first_func_idx, params, 1, &result));
    assert_eq_i32(result.i32, 11); // add_one

    // Test calling second function via table
    assert_ok(wah_call(&exec_ctx, second_func_idx, params, 1, &result));
    assert_eq_i32(result.i32, 12); // add_two

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
}

void wah_test_elem_multiple_segments() {
    printf("Running wah_test_elem_multiple_segments...\n");

    // Test multiple element segments in the same module
    wah_module_t module;
    const char *elem_multiple_segments_spec = "wasm \
        types {[ fn [i32] [i32] ]} \
        funcs {[ 0, 0, 0, 0, 0 ]} \
        tables {[ funcref limits.i32/1 10 ]} \
        exports {[ {'test_all'} fn# 4 ]} \
        elements {[ \
            elem.active.table#0 i32.const 0 end [0, 1], \
            elem.active.table#0 i32.const 2 end [2, 3], \
        ]} \
        code {[ \
            {[] local.get 0 i32.const 1 i32.add end}, \
            {[] local.get 0 i32.const 2 i32.add end}, \
            {[] local.get 0 i32.const 3 i32.add end}, \
            {[] local.get 0 i32.const 4 i32.add end}, \
            {[] \
                local.get 0 \
                i32.const 0 call_indirect 0 0 \
                local.get 0 \
                i32.const 1 call_indirect 0 0 \
                i32.add \
                local.get 0 \
                i32.const 2 call_indirect 0 0 \
                i32.add \
                local.get 0 \
                i32.const 3 call_indirect 0 0 \
                i32.add \
            end}, \
        ]}";
    assert_ok(wah_parse_module_from_spec(&module, elem_multiple_segments_spec));

    wah_exec_context_t exec_ctx;
    assert_ok(wah_exec_context_create(&exec_ctx, &module, NULL));

    wah_export_desc_t entry;
    assert_ok(wah_module_export_by_name(&module, "test_all", &entry));
    uint32_t func_idx = (uint32_t)entry.index;

    // Test calling all functions from table
    wah_value_t params[1] = {{.i32 = 0}};
    wah_value_t result;
    assert_ok(wah_call(&exec_ctx, func_idx, params, 1, &result));
    // Should call: add_one(0)+add_two(add_one(0))+add_three(add_two(add_one(0)))
    // = 1 + (1+2) + ((1+2)+3) = 1 + 3 + 6 = 10
    assert_eq_i32(result.i32, 10);

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
}

static void host_imported_fn(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    wah_return_i32(ctx, wah_param_i32(ctx, 0) + 100);
}

void wah_test_elem_passive_with_imports() {
    printf("Running wah_test_elem_passive_with_imports...\n");

    // Test passive element segments with imports causing index drift
    wah_module_t module;
    const char *elem_passive_with_imports_spec = "wasm \
        types {[ fn [i32] [i32] ]} \
        imports {[ {'mod'} {'imported_fn'} fn# 0 ]} \
        funcs {[ 0, 0 ]} \
        tables {[ funcref limits.i32/1 5 ]} \
        exports {[ {'test_init'} fn# 1 ]} \
        elements {[ elem.passive elem.funcref [1, 2] ]} \
        code {[ \
            {[] local.get 0 i32.const 1 i32.add end}, \
            {[] \
                i32.const 2 i32.const 0 i32.const 0 table.init 0 0 \
                local.get 0 \
                i32.const 0 call_indirect 0 0 \
            end}, \
        ]}";
    assert_ok(wah_parse_module_from_spec(&module, elem_passive_with_imports_spec));

    // Create host module for linkage
    wah_module_t host_mod;
    assert_ok(wah_new_module(&host_mod, NULL));

    assert_ok(wah_module_export_func(&host_mod, "imported_fn", "(i32) -> i32", host_imported_fn, NULL, NULL));

    wah_exec_context_t exec_ctx;
    assert_ok(wah_exec_context_create(&exec_ctx, &module, NULL));
    assert_ok(wah_link_module(&exec_ctx, "mod", &host_mod));
    assert_ok(wah_instantiate(&exec_ctx));

    wah_export_desc_t entry;
    assert_ok(wah_module_export_by_name(&module, "test_init", &entry));
    uint32_t func_idx = (uint32_t)entry.index;

    // Test that passive element with index 1 refers to first local function (after import)
    wah_value_t params[1] = {{.i32 = 10}};
    wah_value_t result;
    assert_ok(wah_call(&exec_ctx, func_idx, params, 1, &result));
    assert_eq_i32(result.i32, 11); // Should call local function (add_one)

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
    wah_free_module(&host_mod);
}

void wah_test_table_grow_isolated() {
    // Regression test: table.grow must not affect a second exec context
    // sharing the same module (bug: TABLE_GROW wrote back to module->tables[].min_elements)
    printf("Running wah_test_table_grow_isolated...\n");

    wah_module_t module;
    const char *spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0, 0 ]} \
        tables {[ funcref limits.i32/1 5 ]} \
        exports {[ {'grow'} fn# 0, {'size'} fn# 1 ]} \
        code {[ \
            {[] ref.null funcref i32.const 1 table.grow 0 end}, \
            {[] table.size 0 end}, \
        ]}";
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx1, ctx2;
    assert_ok(wah_exec_context_create(&ctx1, &module, NULL));
    assert_ok(wah_exec_context_create(&ctx2, &module, NULL));

    wah_export_desc_t grow_entry, size_entry;
    assert_ok(wah_module_export_by_name(&module, "grow", &grow_entry));
    assert_ok(wah_module_export_by_name(&module, "size", &size_entry));
    uint32_t grow_idx = (uint32_t)grow_entry.index;
    uint32_t size_idx = (uint32_t)size_entry.index;

    wah_value_t result;

    // Grow ctx1's table from 5 to 6; returns old size (5)
    assert_ok(wah_call(&ctx1, grow_idx, NULL, 0, &result));
    assert_eq_i32(result.i32, 5);

    // ctx2's table must still be size 5 (isolated from ctx1's grow)
    assert_ok(wah_call(&ctx2, size_idx, NULL, 0, &result));
    assert_eq_i32(result.i32, 5);

    // ctx1's table is now size 6
    assert_ok(wah_call(&ctx1, size_idx, NULL, 0, &result));
    assert_eq_i32(result.i32, 6);

    wah_exec_context_destroy(&ctx1);
    wah_exec_context_destroy(&ctx2);
    wah_free_module(&module);
}

void wah_test_table_no_max_is_unbounded() {
    // Regression: limits.i32/1 (flags=0x00) must set max_elements=UINT32_MAX, not min_elements.
    printf("Running wah_test_table_no_max_is_unbounded...\n");

    wah_module_t module;
    const char *spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 3 ]} \
        exports {[ {'grow'} fn# 0 ]} \
        code {[ \
            {[] ref.null funcref i32.const 5 table.grow 0 end}, \
        ]}";
    assert_ok(wah_parse_module_from_spec(&module, spec));
    { wah_table_desc_t td; assert_ok(wah_module_table(&module, 0, &td));
      assert_eq_u32((uint32_t)td.min_elements, 3); assert_eq_u32((uint32_t)td.max_elements, UINT32_MAX); }

    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));

    // Growing by 5 from min=3 yields old_size=3; must succeed (no max limit)
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 3);  // returns old size
    assert_eq_u64(wah_debug_table_size(&ctx, 0), 8);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

void wah_test_table_set_nonnull_rejects_null() {
    printf("Running wah_test_table_set_nonnull_rejects_null...\n");

    wah_module_t module;
    const char *spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        tables {[ type.ref.func limits.i32/1 1 ]} \
        exports {[ {'f'} fn# 0 ]} \
        code {[ {[] i32.const 0 ref.null funcref table.set 0 end} ]}";
    assert_err(wah_parse_module_from_spec(&module, spec), WAH_ERROR_VALIDATION_FAILED);
}

void wah_test_table_set_nonnull_accepts_ref_func() {
    printf("Running wah_test_table_set_nonnull_accepts_ref_func...\n");

    wah_module_t module;
    const char *spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        tables {[ table.w/init type.ref.func limits.i32/1 1 ref.func 0 end ]} \
        exports {[ {'f'} fn# 0 ]} \
        elements {[ elem.declarative elem.funcref [0] ]} \
        code {[ {[] i32.const 0 ref.func 0 table.set 0 end} ]}";
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_free_module(&module);
}

void wah_test_table_grow_nonnull_rejects_null() {
    printf("Running wah_test_table_grow_nonnull_rejects_null...\n");

    wah_module_t module;
    const char *spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        tables {[ type.ref.func limits.i32/1 1 ]} \
        exports {[ {'f'} fn# 0 ]} \
        code {[ {[] ref.null funcref i32.const 1 table.grow 0 end} ]}";
    assert_err(wah_parse_module_from_spec(&module, spec), WAH_ERROR_VALIDATION_FAILED);
}

void wah_test_table_fill_nonnull_rejects_null() {
    printf("Running wah_test_table_fill_nonnull_rejects_null...\n");

    wah_module_t module;
    const char *spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        tables {[ type.ref.func limits.i32/1 1 ]} \
        exports {[ {'f'} fn# 0 ]} \
        code {[ {[] i32.const 0 ref.null funcref i32.const 1 table.fill 0 end} ]}";
    assert_err(wah_parse_module_from_spec(&module, spec), WAH_ERROR_VALIDATION_FAILED);
}

void wah_test_table_nullable_accepts_nonnull() {
    printf("Running wah_test_table_nullable_accepts_nonnull...\n");

    wah_module_t module;
    const char *spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        exports {[ {'f'} fn# 0 ]} \
        elements {[ elem.declarative elem.funcref [0] ]} \
        code {[ {[] \
            i32.const 0 ref.func 0 table.set 0 \
            ref.func 0 i32.const 1 table.grow 0 \
        end} ]}";
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_free_module(&module);
}

void wah_test_table_copy_nullable_to_nonnull_rejected() {
    printf("Running wah_test_table_copy_nullable_to_nonnull_rejected...\n");

    wah_module_t module;
    const char *spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        tables {[ type.ref.func limits.i32/1 1, funcref limits.i32/1 1 ]} \
        exports {[ {'f'} fn# 0 ]} \
        code {[ {[] i32.const 0 i32.const 0 i32.const 1 table.copy 0 1 end} ]}";
    assert_err(wah_parse_module_from_spec(&module, spec), WAH_ERROR_VALIDATION_FAILED);
}

void wah_test_table_copy_nonnull_to_nullable_accepted() {
    printf("Running wah_test_table_copy_nonnull_to_nullable_accepted...\n");

    wah_module_t module;
    const char *spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 1, table.w/init type.ref.func limits.i32/1 1 ref.func 0 end ]} \
        exports {[ {'f'} fn# 0 ]} \
        elements {[ elem.declarative elem.funcref [0] ]} \
        code {[ {[] i32.const 0 i32.const 0 i32.const 1 table.copy 0 1 end} ]}";
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_free_module(&module);
}

void wah_test_table_init_nullable_elem_to_nonnull_table_rejected() {
    printf("Running wah_test_table_init_nullable_elem_to_nonnull_table_rejected...\n");

    wah_module_t module;
    const char *spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        tables {[ type.ref.func limits.i32/1 1 ]} \
        exports {[ {'f'} fn# 0 ]} \
        elements {[ elem.passive.expr funcref [ref.func 0 end] ]} \
        code {[ {[] i32.const 0 i32.const 0 i32.const 1 table.init 0 0 end} ]}";
    assert_err(wah_parse_module_from_spec(&module, spec), WAH_ERROR_VALIDATION_FAILED);
}

void wah_test_table_init_nonnull_elem_to_nullable_table_accepted() {
    printf("Running wah_test_table_init_nonnull_elem_to_nullable_table_accepted...\n");

    wah_module_t module;
    const char *spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        exports {[ {'f'} fn# 0 ]} \
        elements {[ elem.passive.expr type.ref.func [ref.func 0 end] ]} \
        code {[ {[] i32.const 0 i32.const 0 i32.const 1 table.init 0 0 end} ]}";
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_free_module(&module);
}

void wah_test_table_init_src_range_overflow_traps() {
    printf("Running wah_test_table_init_src_range_overflow_traps...\n");

    wah_module_t module;
    const char *spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 10 ]} \
        exports {[ {'f'} fn# 0 ]} \
        elements {[ elem.passive elem.funcref [0] ]} \
        code {[ {[] i32.const 0 i32.const -1 i32.const 2 table.init 0 0 end} ]}";
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));

    wah_export_desc_t entry;
    assert_ok(wah_module_export_by_name(&module, "f", &entry));
    assert_err(wah_call(&ctx, (uint32_t)entry.index, NULL, 0, NULL), WAH_ERROR_TRAP);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

void wah_test_active_elem_nullable_into_nonnull_table_rejected() {
    printf("Running wah_test_active_elem_nullable_into_nonnull_table_rejected...\n");

    wah_module_t module;
    const char *spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        tables {[ type.ref.func limits.i32/1 1 ]} \
        exports {[ {'f'} fn# 0 ]} \
        elements {[ elem.active.expr.table# 0 i32.const 0 end funcref [ref.func 0 end] ]} \
        code {[ {[] end} ]}";
    assert_err(wah_parse_module_from_spec(&module, spec), WAH_ERROR_VALIDATION_FAILED);
}

void wah_test_active_elem_nonnull_into_nonnull_table_accepted() {
    printf("Running wah_test_active_elem_nonnull_into_nonnull_table_accepted...\n");

    wah_module_t module;
    const char *spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        tables {[ table.w/init type.ref.func limits.i32/1 1 ref.func 0 end ]} \
        exports {[ {'f'} fn# 0 ]} \
        elements {[ elem.active.expr.table# 0 i32.const 0 end type.ref.func [ref.func 0 end] ]} \
        code {[ {[] end} ]}";
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_free_module(&module);
}

// c36d249: Parse 0x40 table encoding (tables with init expressions).
void wah_test_table_init_expr() {
    printf("Running wah_test_table_init_expr...\n");

    // Table with init expr (ref.func 0): all slots should have func 0.
    // func 0 returns 42, func 1 does call_indirect on table[0].
    const char *spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0, 0 ]} \
        tables {[ table.w/init funcref limits.i32/1 2 ref.func 0 end ]} \
        elements {[ elem.declarative.expr funcref [ ref.func 0 end ] ]} \
        code {[ \
            {[] i32.const 42 end }, \
            {[] i32.const 1 call_indirect 0 0 end } \
        ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
    assert_ok(wah_instantiate(&ctx));

    // call_indirect on table[1] should find func 0 (from init expr)
    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// b4a8325: Drop active element segments after instantiation per spec.
void wah_test_active_elem_dropped_after_init() {
    printf("Running wah_test_active_elem_dropped_after_init...\n");

    // Active elem segment populates table during instantiation.
    // After instantiation, table.init from that segment (with size > 0) should trap.
    const char *spec = "wasm \
        types {[ fn [] [], fn [] [i32] ]} \
        funcs {[ 0, 1 ]} \
        tables {[ funcref limits.i32/1 2 ]} \
        elements {[ elem.active.table#0 i32.const 0 end [ 0 ] ]} \
        code {[ \
            {[] \
                i32.const 1 \
                i32.const 0 \
                i32.const 1 \
                table.init 0 0 \
            end }, \
            {[] i32.const 42 end } \
        ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
    assert_ok(wah_instantiate(&ctx));

    // table.init with size 1 from dropped segment should trap
    assert_err(wah_call(&ctx, 0, NULL, 0, NULL), WAH_ERROR_TRAP);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// 28ffe76: Out-of-bound condition with active element segments must trap.
void wah_test_active_elem_oob_trap() {
    printf("Running wah_test_active_elem_oob_trap...\n");

    // Table has 1 slot, but active element tries to write 2 elements at offset 0
    const char *spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0, 0 ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        elements {[ elem.active.table#0 i32.const 0 end [ 0, 1 ] ]} \
        code {[ {[] end }, {[] end } ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
    assert_err(wah_instantiate(&ctx), WAH_ERROR_TRAP);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);

    // Positive control: 1 element in 1-slot table at offset 0 is fine
    const char *good_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        elements {[ elem.active.table#0 i32.const 0 end [ 0 ] ]} \
        code {[ {[] end } ]}";

    wah_module_t good = {0};
    assert_ok(wah_parse_module_from_spec(&good, good_spec));
    wah_exec_context_t ctx2 = {0};
    assert_ok(wah_exec_context_create(&ctx2, &good, NULL));
    assert_ok(wah_instantiate(&ctx2));
    wah_exec_context_destroy(&ctx2);
    wah_free_module(&good);
}

// 621bae8: Reject tables with non-defaultable element type lacking init expression.
void wah_test_nonnull_table_no_init() {
    printf("Running wah_test_nonnull_table_no_init...\n");

    // Negative: non-nullable (ref func) table without init expression
    const char *bad_spec = "wasm \
        types {[ fn [] [] ]} \
        tables {[ type.ref.func limits.i32/1 1 ]}";
    wah_module_t bad = {0};
    assert_err(wah_parse_module_from_spec(&bad, bad_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad);

    // Positive: non-nullable (ref func) table WITH init expression (0x40 encoding)
    const char *good_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        tables {[ table.w/init type.ref.func limits.i32/1 1 ref.func 0 end ]} \
        exports {[ {'f'} fn# 0 ]} \
        code {[ {[] end} ]}";
    wah_module_t good = {0};
    assert_ok(wah_parse_module_from_spec(&good, good_spec));
    wah_free_module(&good);

    // Positive: nullable funcref table without init is fine
    const char *nullable_spec = "wasm \
        types {[ fn [] [] ]} \
        tables {[ funcref limits.i32/1 1 ]}";
    wah_module_t nullable = {0};
    assert_ok(wah_parse_module_from_spec(&nullable, nullable_spec));
    wah_free_module(&nullable);
}

int main() {
    wah_test_table_indirect_call();
    wah_test_table_size();
    wah_test_table_grow();
    wah_test_table_fill();
    wah_test_table_copy();
    wah_test_table_init();
    wah_test_elem_drop();
    wah_test_elem_active_table1();
    wah_test_elem_import_index_drift();
    wah_test_elem_declarative();
    wah_test_elem_expr_style();
    wah_test_elem_multiple_segments();
    wah_test_elem_passive_with_imports();
    wah_test_table_grow_isolated();
    wah_test_table_no_max_is_unbounded();
    wah_test_table_set_nonnull_rejects_null();
    wah_test_table_set_nonnull_accepts_ref_func();
    wah_test_table_grow_nonnull_rejects_null();
    wah_test_table_fill_nonnull_rejects_null();
    wah_test_table_nullable_accepts_nonnull();
    wah_test_table_copy_nullable_to_nonnull_rejected();
    wah_test_table_copy_nonnull_to_nullable_accepted();
    wah_test_table_init_nullable_elem_to_nonnull_table_rejected();
    wah_test_table_init_nonnull_elem_to_nullable_table_accepted();
    wah_test_table_init_src_range_overflow_traps();
    wah_test_active_elem_nullable_into_nonnull_table_rejected();
    wah_test_active_elem_nonnull_into_nonnull_table_accepted();
    wah_test_nonnull_table_no_init();
    wah_test_active_elem_oob_trap();
    wah_test_active_elem_dropped_after_init();
    wah_test_table_init_expr();
    return 0;
}
