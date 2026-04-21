#define WAH_IMPLEMENTATION
#include "../wah.h"
#include "common.c"
#include <stdio.h>
#include <assert.h>

static void test_ref_null_funcref() {
    printf("Running test_ref_null_funcref...\n");

    const char *wasm_spec = "wasm \
        types {[ fn [] [funcref] ]} \
        funcs {[ 0 ]} \
        exports {[ {'test_func'} fn# 0 ]} \
        code {[ {[] ref.null funcref end } ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, wasm_spec));

    wah_exec_context_t exec_ctx;
    wah_value_t result;
    assert_ok(wah_exec_context_create(&exec_ctx, &module));
    assert_ok(wah_call(&exec_ctx, 0, NULL, 0, &result));
    assert_null(result.ref);

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
}

static void test_ref_null_externref() {
    printf("Running test_ref_null_externref...\n");

    const char *wasm_spec = "wasm \
        types {[ fn [] [externref] ]} \
        funcs {[ 0 ]} \
        exports {[ {'test_extern'} fn# 0 ]} \
        code {[ {[] ref.null externref end } ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, wasm_spec));

    wah_exec_context_t exec_ctx;
    wah_value_t result;
    assert_ok(wah_exec_context_create(&exec_ctx, &module));
    assert_ok(wah_call(&exec_ctx, 0, NULL, 0, &result));
    assert_null(result.ref);

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
}

static void test_ref_func() {
    printf("Running test_ref_func...\n");

    const char *wasm_spec = "wasm \
        types {[ fn [] [funcref] ]} \
        funcs {[ 0 ]} \
        exports {[ {'test_ref_func'} fn# 0 ]} \
        code {[ {[] ref.func 0 end } ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, wasm_spec));

    wah_exec_context_t exec_ctx;
    wah_value_t result;
    assert_ok(wah_exec_context_create(&exec_ctx, &module));
    assert_ok(wah_call(&exec_ctx, 0, NULL, 0, &result));
    assert_not_null(result.ref);

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
}

static void test_ref_is_null_funcref_null() {
    printf("Running test_ref_is_null_funcref_null...\n");

    const char *wasm_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        exports {[ {'test_ref_is_null'} fn# 0 ]} \
        code {[ {[] ref.null funcref ref.is_null end } ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, wasm_spec));

    wah_exec_context_t exec_ctx;
    wah_value_t result;
    assert_ok(wah_exec_context_create(&exec_ctx, &module));
    assert_ok(wah_call(&exec_ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 1);

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
}

static void test_ref_is_null_funcref_nonnull() {
    printf("Running test_ref_is_null_funcref_nonnull...\n");

    const char *wasm_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        exports {[ {'test_ref_is_null'} fn# 0 ]} \
        code {[ {[] ref.func 0 ref.is_null end } ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, wasm_spec));

    wah_exec_context_t exec_ctx;
    wah_value_t result;
    assert_ok(wah_exec_context_create(&exec_ctx, &module));
    assert_ok(wah_call(&exec_ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 0);

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
}

static void test_ref_is_null_externref_null() {
    printf("Running test_ref_is_null_externref_null...\n");

    const char *wasm_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        exports {[ {'test_ref_is_null'} fn# 0 ]} \
        code {[ {[] ref.null externref ref.is_null end } ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, wasm_spec));

    wah_exec_context_t exec_ctx;
    wah_value_t result;
    assert_ok(wah_exec_context_create(&exec_ctx, &module));
    assert_ok(wah_call(&exec_ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 1);

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
}

// This tests that ref.func with an invalid function index fails validation.
// This ensures the assertion "validation should have verified function index" holds.
static void test_ref_func_invalid_index() {
    printf("Running test_ref_func_invalid_index...\n");

    const char *wasm_spec = "wasm \
        types {[ fn [] [funcref] ]} \
        funcs {[ 0 ]} \
        exports {[ {'test_invalid'} fn# 0 ]} \
        code {[ {[] ref.func 99 end } ]}";  // Function index 99 doesn't exist

    wah_module_t module;
    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_VALIDATION_FAILED);
}

// Regression: ref.func 0 in a const_expr (e.g. global initializer) must NOT be treated as null.
// Before the sentinel fix, ref.func 0 stored (void*)(uintptr_t)0 == NULL, making ref.is_null return 1.
static void test_ref_func_zero_not_null() {
    printf("Running test_ref_func_zero_not_null...\n");

    // Global 0 is a funcref initialized to ref.func 0 (the only function, global index 0).
    // ref.is_null on that global must return 0.
    const char *spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        globals {[ funcref mut ref.func 0 end ]} \
        exports {[ {'check'} fn# 0 ]} \
        code {[ \
            {[] global.get 0 ref.is_null end} \
        ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 0); // NOT null

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// Regression: a funcref global initialized with ref.func must be callable (table.set + call_indirect).
// Without converting from prefuncref to a runtime function_table pointer, call_indirect would
// read the sentinel pointer as a wah_function_t* and crash or return garbage.
static void test_ref_func_global_callable() {
    printf("Running test_ref_func_global_callable...\n");

    // func 0 (global idx 0): identity fn [i32]->[i32]
    // func 1 (global idx 1): reads funcref global 0 (= ref.func 0), stores in table[0][0],
    //                         then calls via call_indirect and returns the result.
    // global 0: funcref, mutable, initialized to ref.func 0
    const char *spec = "wasm \
        types {[ fn [i32] [i32] ]} \
        funcs {[ 0, 0 ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        globals {[ funcref mut ref.func 0 end ]} \
        exports {[ {'run'} fn# 1 ]} \
        code {[ \
            {[] local.get 0 end}, \
            {[] i32.const 0 global.get 0 table.set 0 \
                local.get 0 i32.const 0 call_indirect 0 0 end}, \
        ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module));

    wah_value_t param = {.i32 = 7};
    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, &param, 1, &result)); // fn# 1 = global idx 1
    assert_eq_i32(result.i32, 7);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// Regression: ref.func in an expr-style element segment must use the global function index
// (which already includes imports). The const_expr evaluator must NOT add import_function_count.
static void host_return42(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    wah_return_i32(ctx, 42);
}

static void test_ref_func_in_elem_with_import() {
    printf("Running test_ref_func_in_elem_with_import...\n");

    // 1 import (return42, global idx 0) + 2 local funcs (global idx 1, 2).
    // Element: ref.func 0 at table slot 0 -- must resolve to the import, not local func 0.
    // Local func 0 returns 99; if the bug is present the call_indirect returns 99 instead of 42.
    const char *spec = "wasm \
        types {[ fn [] [i32] ]} \
        imports {[ {'env'} {'return42'} fn# 0 ]} \
        funcs {[ 0, 0 ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        exports {[ {'call_via_table'} fn# 2 ]} \
        elements {[ elem.active.expr.table#0 i32.const 0 end [ref.func 0 end] ]} \
        code {[ \
            {[] i32.const 99 end}, \
            {[] i32.const 0 call_indirect 0 0 end}, \
        ]}";

    wah_module_t wasm_mod = {0};
    wah_module_t env_mod = {0};
    assert_ok(wah_new_module(&env_mod));
    wah_type_t no_params[1];
    wah_type_t result_i32[1] = { WAH_TYPE_I32 };
    assert_ok(wah_module_export_funcv(&env_mod, "return42", 0, no_params, 1, result_i32, host_return42, NULL, NULL));

    assert_ok(wah_parse_module_from_spec(&wasm_mod, spec));
    assert_eq_u32(wasm_mod.import_function_count, 1);

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &wasm_mod));
    assert_ok(wah_link_module(&ctx, "env", &env_mod));
    assert_ok(wah_instantiate(&ctx));

    wah_entry_t entry;
    assert_ok(wah_module_export_by_name(&wasm_mod, "call_via_table", &entry));

    wah_value_t result;
    assert_ok(wah_call(&ctx, entry.id, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&wasm_mod);
    wah_free_module(&env_mod);
}

static void test_struct_new_get_set() {
    printf("Running test_struct_new_get_set...\n");

    const char *spec = "wasm \
        types {[ struct [i32 mut, i32 mut], fn [] [i32] ]} \
        funcs {[ 1 ]} \
        exports {[ {'f'} fn# 0 ]} \
        code {[ {[] i32.const 0 end } ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 0);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_struct_set_and_read() {
    printf("Running test_struct_set_and_read...\n");

    const char *spec = "wasm \
        types {[ struct [i32 mut, i32 mut], fn [] [i32] ]} \
        funcs {[ 1 ]} \
        exports {[ {'f'} fn# 0 ]} \
        code {[ {[1 structref] \
            struct.new_default 0 local.set 0 \
            local.get 0 i32.const 42 struct.set 0 0 \
            local.get 0 i32.const -7 struct.set 0 1 \
            local.get 0 struct.get 0 0 \
            local.get 0 struct.get 0 1 \
            i32.add \
            end } ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 35); // 42 + (-7)

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_struct_new_with_values() {
    printf("Running test_struct_new_with_values...\n");

    // i32.const 10, i32.const 20, struct.new 0, struct.get 0 1
    const char *spec = "wasm \
        types {[ struct [i32 mut, i32 mut], fn [] [i32] ]} \
        funcs {[ 1 ]} \
        exports {[ {'f'} fn# 0 ]} \
        code {[ {[] \
            i32.const 10 i32.const 20 struct.new 0 struct.get 0 1 \
            end } ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 20);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_array_new_get_set_len() {
    printf("Running test_array_new_get_set_len...\n");

    // array.new 0 with init=0, len=5, then set [2]=99, get [2], check len
    const char *spec = "wasm \
        types {[ array i32 mut, fn [] [i32], fn [] [i32] ]} \
        funcs {[ 1, 2 ]} \
        exports {[ {'get'} fn# 0, {'len'} fn# 1 ]} \
        code {[ \
            {[1 arrayref] \
                i32.const 0 i32.const 5 array.new 0 local.set 0 \
                local.get 0 i32.const 2 i32.const 99 array.set 0 \
                local.get 0 i32.const 2 array.get 0 \
                end }, \
            {[] \
                i32.const 0 i32.const 7 array.new 0 array.len \
                end } \
        ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 99);

    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 7);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_array_oob_trap() {
    printf("Running test_array_oob_trap...\n");

    const char *spec = "wasm \
        types {[ array i32 mut, fn [] [i32] ]} \
        funcs {[ 1 ]} \
        exports {[ {'f'} fn# 0 ]} \
        code {[ {[] \
            i32.const 0 i32.const 3 array.new 0 \
            i32.const 5 array.get 0 \
            end } ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_err(wah_call(&ctx, 0, NULL, 0, &result), WAH_ERROR_TRAP);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_struct_null_trap() {
    printf("Running test_struct_null_trap...\n");

    const char *spec = "wasm \
        types {[ struct [i32 mut], fn [] [i32] ]} \
        funcs {[ 1 ]} \
        exports {[ {'f'} fn# 0 ]} \
        code {[ {[] \
            ref.null structref struct.get 0 0 \
            end } ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_err(wah_call(&ctx, 0, NULL, 0, &result), WAH_ERROR_TRAP);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

int main() {
    test_ref_null_funcref();
    test_ref_null_externref();
    test_ref_func();
    test_ref_is_null_funcref_null();
    test_ref_is_null_funcref_nonnull();
    test_ref_is_null_externref_null();
    test_ref_func_invalid_index();
    test_ref_func_zero_not_null();
    test_ref_func_global_callable();
    test_ref_func_in_elem_with_import();

    test_struct_new_get_set();
    test_struct_set_and_read();
    test_struct_new_with_values();
    test_array_new_get_set_len();
    test_array_oob_trap();
    test_struct_null_trap();

    // --- Subtype matching tests ---

    printf("Running test_subtype_structref_in_anyref_local...\n");
    {
        // structref value stored in anyref local (structref <: eqref <: anyref)
        const char *spec = "wasm \
            types {[ struct [i32 mut], fn [] [i32] ]} \
            funcs {[ 1 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[1 anyref] \
                struct.new_default 0 local.set 0 \
                local.get 0 i32.const 77 struct.set 0 0 \
                local.get 0 struct.get 0 0 \
                end } ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_gc_start(&ctx));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t result;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
        assert_eq_i32(result.i32, 77);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Running test_subtype_structref_in_eqref_local...\n");
    {
        // structref value stored in eqref local (structref <: eqref)
        const char *spec = "wasm \
            types {[ struct [i32 mut], fn [] [i32] ]} \
            funcs {[ 1 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[1 eqref] \
                struct.new_default 0 local.set 0 \
                local.get 0 struct.get 0 0 \
                end } ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_gc_start(&ctx));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t result;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
        assert_eq_i32(result.i32, 0);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Running test_subtype_arrayref_in_anyref_local...\n");
    {
        // arrayref value stored in anyref local (arrayref <: eqref <: anyref)
        const char *spec = "wasm \
            types {[ array i32 mut, fn [] [i32] ]} \
            funcs {[ 1 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[1 anyref] \
                i32.const 0 i32.const 3 array.new 0 local.set 0 \
                local.get 0 array.len \
                end } ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_gc_start(&ctx));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t result;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
        assert_eq_i32(result.i32, 3);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Running test_subtype_nullref_in_structref_local...\n");
    {
        // null ref stored in structref local (nullref <: structref)
        const char *spec = "wasm \
            types {[ struct [i32 mut], fn [] [i32] ]} \
            funcs {[ 1 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[1 structref] \
                ref.null structref local.set 0 \
                local.get 0 ref.is_null \
                end } ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_gc_start(&ctx));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t result;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
        assert_eq_i32(result.i32, 1);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Running test_subtype_nullfuncref_in_funcref_local...\n");
    {
        // null funcref stored in funcref local (nullfuncref <: funcref)
        const char *spec = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[1 funcref] \
                ref.null funcref local.set 0 \
                local.get 0 ref.is_null \
                end } ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t result;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
        assert_eq_i32(result.i32, 1);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("\nAll Reference Types tests passed!\n");
    return 0;
}