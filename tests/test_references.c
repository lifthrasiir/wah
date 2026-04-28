#include "../wah.h"
#include "wah_impl.h"
#include "common.h"
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

static int host_externref_identity_called = 0;
static void host_externref_identity(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    host_externref_identity_called = 1;
    ctx->results[0].ref = ctx->params[0].ref;
}

static void host_externref_is_null(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    wah_return_i32(ctx, ctx->params[0].ref == NULL ? 1 : 0);
}

static void host_return_externref(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    ctx->results[0].ref = (void *)(uintptr_t)0xDEADBEEF;
}

static void test_externref_host_roundtrip() {
    printf("Running test_externref_host_roundtrip...\n");

    wah_module_t host_mod = {0};
    assert_ok(wah_new_module(&host_mod));
    assert_ok(wah_module_export_func(&host_mod, "identity", "(externref) -> externref",
        host_externref_identity, NULL, NULL));
    assert_ok(wah_module_export_func(&host_mod, "isNull", "(externref) -> i32",
        host_externref_is_null, NULL, NULL));
    assert_ok(wah_module_export_func(&host_mod, "make", "() -> externref",
        host_return_externref, NULL, NULL));

    const char *spec = "wasm \
        types {[ \
            fn [externref] [externref], \
            fn [externref] [i32], \
            fn [] [externref], \
            fn [] [i32] \
        ]} \
        imports {[ \
            {'host'} {'identity'} fn# 0, \
            {'host'} {'isNull'} fn# 1, \
            {'host'} {'make'} fn# 2 \
        ]} \
        funcs {[ 3, 3 ]} \
        code {[ \
            {[] call 2 call 0 call 1 end }, \
            {[] ref.null externref call 0 call 1 end } \
        ]}";

    wah_module_t wasm_mod = {0};
    assert_ok(wah_parse_module_from_spec(&wasm_mod, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &wasm_mod));
    assert_ok(wah_link_module(&ctx, "host", &host_mod));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;

    host_externref_identity_called = 0;
    assert_ok(wah_call(&ctx, 3, NULL, 0, &result));
    assert_true(host_externref_identity_called);
    assert_eq_i32(result.i32, 0);

    host_externref_identity_called = 0;
    assert_ok(wah_call(&ctx, 4, NULL, 0, &result));
    assert_true(host_externref_identity_called);
    assert_eq_i32(result.i32, 1);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&wasm_mod);
    wah_free_module(&host_mod);
}

static void test_externref_table_host_boundary() {
    printf("Running test_externref_table_host_boundary...\n");

    wah_module_t host_mod = {0};
    assert_ok(wah_new_module(&host_mod));
    assert_ok(wah_module_export_func(&host_mod, "make", "() -> externref",
        host_return_externref, NULL, NULL));
    assert_ok(wah_module_export_func(&host_mod, "isNull", "(externref) -> i32",
        host_externref_is_null, NULL, NULL));

    const char *spec = "wasm \
        types {[ \
            fn [] [externref], \
            fn [externref] [i32], \
            fn [] [i32] \
        ]} \
        imports {[ \
            {'host'} {'make'} fn# 0, \
            {'host'} {'isNull'} fn# 1 \
        ]} \
        funcs {[ 2 ]} \
        tables {[ externref limits.i32/2 1 2 ]} \
        code {[ \
            {[] \
                i32.const 0 call 0 table.set 0 \
                i32.const 0 table.get 0 call 1 \
            end } \
        ]}";

    wah_module_t wasm_mod = {0};
    assert_ok(wah_parse_module_from_spec(&wasm_mod, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &wasm_mod));
    assert_ok(wah_link_module(&ctx, "host", &host_mod));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 2, NULL, 0, &result));
    assert_eq_i32(result.i32, 0);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&wasm_mod);
    wah_free_module(&host_mod);
}

static void test_externref_global_host_boundary() {
    printf("Running test_externref_global_host_boundary...\n");

    wah_module_t host_mod = {0};
    assert_ok(wah_new_module(&host_mod));
    assert_ok(wah_module_export_func(&host_mod, "make", "() -> externref",
        host_return_externref, NULL, NULL));
    assert_ok(wah_module_export_func(&host_mod, "isNull", "(externref) -> i32",
        host_externref_is_null, NULL, NULL));

    const char *spec = "wasm \
        types {[ \
            fn [] [externref], \
            fn [externref] [i32], \
            fn [] [i32] \
        ]} \
        imports {[ \
            {'host'} {'make'} fn# 0, \
            {'host'} {'isNull'} fn# 1 \
        ]} \
        funcs {[ 2 ]} \
        globals {[ externref mut ref.null externref end ]} \
        code {[ \
            {[] \
                call 0 global.set 0 \
                global.get 0 call 1 \
            end } \
        ]}";

    wah_module_t wasm_mod = {0};
    assert_ok(wah_parse_module_from_spec(&wasm_mod, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &wasm_mod));
    assert_ok(wah_link_module(&ctx, "host", &host_mod));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 2, NULL, 0, &result));
    assert_eq_i32(result.i32, 0);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&wasm_mod);
    wah_free_module(&host_mod);
}

static void test_externref_ref_test() {
    printf("Running test_externref_ref_test...\n");

    // non-null externref, ref.test externref -> 1
    {
        wah_module_t host_mod = {0};
        assert_ok(wah_new_module(&host_mod));
        assert_ok(wah_module_export_func(&host_mod, "make", "() -> externref",
            host_return_externref, NULL, NULL));

        const char *spec = "wasm \
            types {[ fn [] [externref], fn [] [i32] ]} \
            imports {[ {'host'} {'make'} fn# 0 ]} \
            funcs {[ 1 ]} \
            code {[ {[] call 0 ref.test externref end } ]}";
        wah_module_t wasm_mod = {0};
        assert_ok(wah_parse_module_from_spec(&wasm_mod, spec));
        wah_exec_context_t ctx = {0};
        assert_ok(wah_exec_context_create(&ctx, &wasm_mod));
        assert_ok(wah_link_module(&ctx, "host", &host_mod));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t result;
        assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
        assert_eq_i32(result.i32, 1);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&wasm_mod);
        wah_free_module(&host_mod);
    }

    // null externref, ref.test externref -> 0
    {
        const char *spec = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0 ]} \
            code {[ {[] ref.null externref ref.test externref end } ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t result;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
        assert_eq_i32(result.i32, 0);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    // null externref, ref.test.null externref -> 1
    {
        const char *spec = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0 ]} \
            code {[ {[] ref.null externref ref.test.null externref end } ]}";
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
}

static void test_externref_ref_cast() {
    printf("Running test_externref_ref_cast...\n");

    // non-null externref, ref.cast externref -> should succeed
    {
        wah_module_t host_mod = {0};
        assert_ok(wah_new_module(&host_mod));
        assert_ok(wah_module_export_func(&host_mod, "make", "() -> externref",
            host_return_externref, NULL, NULL));
        assert_ok(wah_module_export_func(&host_mod, "isNull", "(externref) -> i32",
            host_externref_is_null, NULL, NULL));

        const char *spec = "wasm \
            types {[ fn [] [externref], fn [externref] [i32], fn [] [i32] ]} \
            imports {[ {'host'} {'make'} fn# 0, {'host'} {'isNull'} fn# 1 ]} \
            funcs {[ 2 ]} \
            code {[ {[] call 0 ref.cast externref call 1 end } ]}";
        wah_module_t wasm_mod = {0};
        assert_ok(wah_parse_module_from_spec(&wasm_mod, spec));
        wah_exec_context_t ctx = {0};
        assert_ok(wah_exec_context_create(&ctx, &wasm_mod));
        assert_ok(wah_link_module(&ctx, "host", &host_mod));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t result;
        assert_ok(wah_call(&ctx, 2, NULL, 0, &result));
        assert_eq_i32(result.i32, 0);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&wasm_mod);
        wah_free_module(&host_mod);
    }

    // null externref, ref.cast externref -> should trap
    {
        const char *spec = "wasm \
            types {[ fn [] [externref] ]} \
            funcs {[ 0 ]} \
            code {[ {[] ref.null externref ref.cast externref end } ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t result;
        assert_err(wah_call(&ctx, 0, NULL, 0, &result), WAH_ERROR_TRAP);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    // br_on_cast.null.null with externref (extern > noextern hierarchy)
    {
        wah_module_t host_mod = {0};
        assert_ok(wah_new_module(&host_mod));
        assert_ok(wah_module_export_func(&host_mod, "make", "() -> externref",
            host_return_externref, NULL, NULL));

        const char *spec = "wasm \
            types {[ fn [] [externref], fn [] [i32] ]} \
            imports {[ {'host'} {'make'} fn# 0 ]} \
            funcs {[ 1 ]} \
            code {[ {[] \
                block externref \
                    call 0 \
                    br_on_cast.null.null 0 externref externref \
                    drop \
                    ref.null externref \
                end \
                ref.is_null \
                i32.const 1 i32.xor \
            end } ]}";
        wah_module_t wasm_mod = {0};
        assert_ok(wah_parse_module_from_spec(&wasm_mod, spec));
        wah_exec_context_t ctx = {0};
        assert_ok(wah_exec_context_create(&ctx, &wasm_mod));
        assert_ok(wah_link_module(&ctx, "host", &host_mod));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t result;
        assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
        assert_eq_i32(result.i32, 1);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&wasm_mod);
        wah_free_module(&host_mod);
    }
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
    assert_ok(wah_module_export_func(&env_mod, "return42", "() -> i32", host_return42, NULL, NULL));

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

static void test_typed_ref_decoding() {
    printf("Running test_typed_ref_decoding (non-null ref func param)...\n");
    {
        // Function type: (ref func) -> i32
        // type.ref encodes as 0x64 followed by heap type
        const char *spec = "wasm \
            types {[ fn [type.ref.func] [i32] ]} \
            funcs {[ 0 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[] local.get 0 ref.is_null end } ]}";
        wah_module_t module;
        // Should parse and validate: param is (ref func) non-nullable
        assert_ok(wah_parse_module_from_spec(&module, spec));
        // Verify the parsed type has non-nullable flag
        assert(module.type_count == 1);
        { uint32_t pc, rc; const wah_type_t *pt, *rt;
          assert_ok(wah_debug_func_type(&module, 0, &pc, &pt, &rc, &rt));
          assert(pc == 1); assert(pt[0] == WAH_TYPE_FUNC); assert(!WAH_TYPE_IS_NULLABLE(pt[0]));
          assert(rc == 1); assert(rt[0] == WAH_TYPE_I32); }
        wah_free_module(&module);
    }

    printf("Running test_typed_ref_decoding (ref null func param)...\n");
    {
        // Function type: (ref null func) -> i32
        // type.ref.null (0x63) followed by heap type byte for func (0x70)
        const char *spec = "wasm \
            types {[ fn [type.ref.null.func] [i32] ]} \
            funcs {[ 0 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[] local.get 0 ref.is_null end } ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));
        assert(module.type_count == 1);
        { uint32_t pc; const wah_type_t *pt;
          assert_ok(wah_debug_func_type(&module, 0, &pc, &pt, NULL, NULL));
          assert(pc == 1); assert(pt[0] == WAH_TYPE_FUNCREF); assert(WAH_TYPE_IS_NULLABLE(pt[0])); }
        wah_free_module(&module);
    }

    printf("Running test_typed_ref_decoding (ref null anyref result)...\n");
    {
        // Function type: () -> (ref null any) -- same as anyref shorthand
        const char *spec = "wasm \
            types {[ fn [] [type.ref.null.any] ]} \
            funcs {[ 0 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[] ref.null anyref end } ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));
        { uint32_t rc; const wah_type_t *rt;
          assert_ok(wah_debug_func_type(&module, 0, NULL, NULL, &rc, &rt));
          assert(rc == 1); assert(rt[0] == WAH_TYPE_ANYREF); assert(WAH_TYPE_IS_NULLABLE(rt[0])); }
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t result;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
        assert_null(result.ref);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Running test_typed_ref_decoding (non-null ref in local)...\n");
    {
        // Local declared as (ref null extern) via full encoding
        const char *spec = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[type.ref.null.extern] ref.null externref local.set 0 local.get 0 ref.is_null end } ]}";
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

    printf("Running test_typed_ref_decoding (concrete typeidx ref)...\n");
    {
        // Function type with (ref null $0) where $0 is a function type
        // type.ref.null followed by type index 0 (LEB128)
        const char *spec = "wasm \
            types {[ fn [] [type.ref.null 0] ]} \
            funcs {[ 0 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[] ref.null nullfuncref end } ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));
        { uint32_t rc; const wah_type_t *rt;
          assert_ok(wah_debug_func_type(&module, 0, NULL, NULL, &rc, &rt));
          assert(rc == 1); assert(WAH_TYIDX(rt[0]) == 0); assert(WAH_TYPE_IS_NULLABLE(rt[0])); }
        wah_free_module(&module);
    }
}

int main() {
    test_typed_ref_decoding();
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

    // --- Subtype matching tests ---

    printf("Running test_subtype_structref_in_anyref_local...\n");
    {
        // structref value stored in anyref local (structref <: eqref <: anyref)
        // requires ref.cast to recover concrete type before struct.get/set
        const char *spec = "wasm \
            types {[ struct [i32 mut], fn [] [i32] ]} \
            funcs {[ 1 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[1 anyref] \
                struct.new_default 0 local.set 0 \
                local.get 0 ref.cast.null 0 i32.const 77 struct.set 0 0 \
                local.get 0 ref.cast.null 0 struct.get 0 0 \
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
        // requires ref.cast to recover concrete type before struct.get
        const char *spec = "wasm \
            types {[ struct [i32 mut], fn [] [i32] ]} \
            funcs {[ 1 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[1 eqref] \
                struct.new_default 0 local.set 0 \
                local.get 0 ref.cast.null 0 struct.get 0 0 \
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

    printf("Running test_subtype_concrete_struct_in_structref_local...\n");
    {
        // type 0: struct { mut i32 }  (concrete struct)
        // concrete struct ref stored in concrete-typed local, accessed via struct.get
        const char *spec = "wasm \
            types {[ struct [i32 mut], fn [] [i32] ]} \
            funcs {[ 1 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[1 type.ref.null 0] \
                i32.const 55 struct.new 0 local.set 0 \
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
        assert_eq_i32(result.i32, 55);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Running test_subtype_concrete_struct_in_eqref_local...\n");
    {
        // concrete struct ref stored in eqref local (struct <: structref <: eqref)
        // requires ref.cast to recover concrete type before struct.get
        const char *spec = "wasm \
            types {[ struct [i32 mut], fn [] [i32] ]} \
            funcs {[ 1 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[1 eqref] \
                i32.const 88 struct.new 0 local.set 0 \
                local.get 0 ref.cast.null 0 struct.get 0 0 \
                end } ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_gc_start(&ctx));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t result;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
        assert_eq_i32(result.i32, 88);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Running test_subtype_concrete_struct_in_anyref_local...\n");
    {
        // concrete struct ref stored in anyref local
        // requires ref.cast to recover concrete type before struct.get
        const char *spec = "wasm \
            types {[ struct [i32 mut], fn [] [i32] ]} \
            funcs {[ 1 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[1 anyref] \
                i32.const 33 struct.new 0 local.set 0 \
                local.get 0 ref.cast.null 0 struct.get 0 0 \
                end } ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_gc_start(&ctx));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t result;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
        assert_eq_i32(result.i32, 33);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Running test_subtype_declared_supertype...\n");
    {
        // type 0: struct { mut i32 }         (parent)
        // type 1: sub 0 struct { mut i32, mut i32 }  (child extends parent)
        // type 2: func [] -> [i32]
        // child struct ref stored in parent-typed local via structref
        // sub.final [0] means: final sub type, 1 supertype count, supertype index 0
        const char *spec = "wasm \
            types {[ sub [] struct [i32 mut], sub.final [0] struct [i32 mut, i32 mut], fn [] [i32] ]} \
            funcs {[ 2 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[1 type.ref.null 1] \
                i32.const 10 i32.const 20 struct.new 1 local.set 0 \
                local.get 0 struct.get 1 0 \
                local.get 0 struct.get 1 1 \
                i32.add \
                end } ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));

        // Verify subtype relationship was parsed
        assert(wah_debug_type_def_supertype(&module, 1) == 0);
        assert(wah_debug_type_def_supertype(&module, 0) == wah_debug_no_supertype());

        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_gc_start(&ctx));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t result;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
        assert_eq_i32(result.i32, 30);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Running test_subtype_ref_test_concrete...\n");
    {
        // type 0: struct { mut i32 }
        // type 1: sub 0 struct { mut i32 }  (child extends parent, same layout)
        // type 2: func [] -> [i32]
        // type 3: func [] -> [i32]
        const char *spec = "wasm \
            types {[ sub [] struct [i32 mut], sub.final [0] struct [i32 mut], fn [] [i32] ]} \
            funcs {[ 2, 2 ]} \
            exports {[ {'test_parent'} fn# 0, {'test_child'} fn# 1 ]} \
            code {[ \
                {[] i32.const 1 struct.new 1 ref.test.null 0 end }, \
                {[] i32.const 1 struct.new 1 ref.test.null 1 end } \
            ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_gc_start(&ctx));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t result;

        // child instance tested against parent type -> 1 (child <: parent)
        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
        assert_eq_i32(result.i32, 1);

        // child instance tested against child type -> 1
        assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
        assert_eq_i32(result.i32, 1);

        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Running test_subtype_ref_test_negative...\n");
    {
        // type 0: struct { mut i32 }
        // type 1: struct { mut i64 }  (not a subtype of 0)
        // type 2: func [] -> [i32]
        // ref.test: type 1 instance against type 0 -> 0
        const char *spec = "wasm \
            types {[ struct [i32 mut], struct [i64 mut], fn [] [i32] ]} \
            funcs {[ 2 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[] i64.const 99 struct.new 1 ref.test.null 0 end } ]}";
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

    // --- ref.eq, ref.as_non_null, br_on_null, br_on_non_null ---

    printf("Running test_ref_eq_same...\n");
    {
        // two struct.new_default produce different objects -> ref.eq = 0
        // same object -> ref.eq = 1
        const char *spec = "wasm \
            types {[ struct [i32 mut], fn [] [i32], fn [] [i32] ]} \
            funcs {[ 1, 2 ]} \
            exports {[ {'diff'} fn# 0, {'same'} fn# 1 ]} \
            code {[ \
                {[] struct.new_default 0 struct.new_default 0 ref.eq end }, \
                {[1 eqref] struct.new_default 0 local.set 0 local.get 0 local.get 0 ref.eq end } \
            ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_gc_start(&ctx));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t result;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
        assert_eq_i32(result.i32, 0);
        assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
        assert_eq_i32(result.i32, 1);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Running test_ref_eq_null...\n");
    {
        // null == null -> 1
        const char *spec = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[] ref.null eqref ref.null eqref ref.eq end } ]}";
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

    printf("Running test_ref_as_non_null...\n");
    {
        // ref.as_non_null on non-null -> passes through
        const char *spec = "wasm \
            types {[ struct [i32 mut], fn [] [i32] ]} \
            funcs {[ 1 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[] i32.const 42 struct.new 0 ref.as_non_null struct.get 0 0 end } ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_gc_start(&ctx));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t result;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
        assert_eq_i32(result.i32, 42);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Running test_ref_as_non_null_trap...\n");
    {
        // ref.as_non_null on null -> trap
        const char *spec = "wasm \
            types {[ struct [i32 mut], fn [] [i32] ]} \
            funcs {[ 1 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[] ref.null 0 ref.as_non_null struct.get 0 0 end } ]}";
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

    printf("Running test_br_on_null...\n");
    {
        // br_on_null: if null, branch to block end (return 1), else fall through (return 0)
        const char *spec = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[] \
                block void ref.null funcref br_on_null 0 drop i32.const 0 br 1 end i32.const 1 end \
            } ]}";
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

    printf("Running test_br_on_non_null...\n");
    {
        // br_on_non_null: push non-null ref, br_on_non_null branches, drop ref, return 1
        // Encode block type as funcref result via raw bytes
        uint8_t wasm[] = {
            0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, // header
            0x01, 0x05, 0x01, 0x60, 0x00, 0x01, 0x7f,       // type section: () -> (i32)
            0x03, 0x02, 0x01, 0x00,                           // func section: 1 func, type 0
            0x07, 0x05, 0x01, 0x01, 0x66, 0x00, 0x00,        // export "f" func 0
            0x0a, 0x10, 0x01, 0x0e, 0x00,                    // code section, 1 body, 14 bytes, 0 locals
            0x02, 0x70,                                        // block (result funcref)
            0xd2, 0x00,                                        // ref.func 0
            0xd6, 0x00,                                        // br_on_non_null 0
            0xd0, 0x70,                                        // ref.null funcref
            0x0b,                                              // end block
            0x1a,                                              // drop
            0x41, 0x01,                                        // i32.const 1
            0x0b,                                              // end func
        };
        wah_module_t module;
        assert_ok(wah_parse_module(wasm, sizeof(wasm), &module));
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t result;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
        assert_eq_i32(result.i32, 1);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Running test_type_flags_ref_func_non_nullable_to_local...\n");
    {
        // ref.func produces non-nullable; storing into nullable local succeeds
        const char *spec = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[1 funcref] ref.func 0 local.set 0 local.get 0 ref.is_null end } ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &module));
        wah_value_t result;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
        assert_eq_i32(result.i32, 0);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Running test_type_flags_ref_as_non_null_on_ref_func...\n");
    {
        const char *spec = "wasm \
            types {[ fn [] [funcref] ]} \
            funcs {[ 0 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[] ref.func 0 ref.as_non_null end } ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &module));
        wah_value_t result;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
        assert_not_null(result.ref);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Running test_type_flags_ref_as_non_null_traps_on_null...\n");
    {
        const char *spec = "wasm \
            types {[ fn [] [funcref] ]} \
            funcs {[ 0 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[] ref.null funcref ref.as_non_null end } ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &module));
        wah_value_t result;
        assert_err(wah_call(&ctx, 0, NULL, 0, &result), WAH_ERROR_TRAP);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Running test_type_flags_struct_new_non_nullable...\n");
    {
        // struct.new produces non-nullable; ref.as_non_null should not trap
        const char *spec = "wasm \
            types {[ struct [i32 mut], fn [] [i32] ]} \
            funcs {[ 1 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[] i32.const 42 struct.new 0 ref.as_non_null struct.get 0 0 end } ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_gc_start(&ctx));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t result;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
        assert_eq_i32(result.i32, 42);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Running test_type_flags_array_new_non_nullable...\n");
    {
        const char *spec = "wasm \
            types {[ array i32 mut, fn [] [i32] ]} \
            funcs {[ 1 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[] i32.const 99 i32.const 3 array.new 0 ref.as_non_null i32.const 0 array.get 0 end } ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_gc_start(&ctx));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t result;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
        assert_eq_i32(result.i32, 99);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Running test_type_flags_global_get_nullable...\n");
    {
        const char *spec = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0 ]} \
            globals {[ funcref mut ref.null funcref end ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[] global.get 0 ref.is_null end } ]}";
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

    // --- Typed-reference semantics: concrete types flow end-to-end ---

    printf("Running test_struct_new_pushes_concrete_type...\n");
    {
        // struct.new pushes concrete typeidx which struct.get accepts directly
        const char *spec = "wasm \
            types {[ struct [i32 mut], fn [] [i32] ]} \
            funcs {[ 1 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[] i32.const 42 struct.new 0 struct.get 0 0 end } ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_gc_start(&ctx));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t result;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
        assert_eq_i32(result.i32, 42);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Running test_array_new_pushes_concrete_type...\n");
    {
        // array.new pushes concrete typeidx which array.get accepts directly
        const char *spec = "wasm \
            types {[ array i32 mut, fn [] [i32] ]} \
            funcs {[ 1 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[] i32.const 7 i32.const 3 array.new 0 i32.const 1 array.get 0 end } ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_gc_start(&ctx));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t result;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
        assert_eq_i32(result.i32, 7);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Running test_call_result_carries_flags...\n");
    {
        // func 0 returns (ref 0) [non-nullable]; struct.get should accept it
        const char *spec = "wasm \
            types {[ struct [i32 mut], fn [] [type.ref 0], fn [] [i32] ]} \
            funcs {[ 1, 2 ]} \
            exports {[ {'f'} fn# 1 ]} \
            code {[ \
                {[] i32.const 55 struct.new 0 end }, \
                {[] call 0 struct.get 0 0 end } \
            ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_gc_start(&ctx));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t result;
        assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
        assert_eq_i32(result.i32, 55);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Running test_ref_cast_produces_concrete_type...\n");
    {
        // ref.cast from eqref to concrete type 0, then struct.get
        const char *spec = "wasm \
            types {[ struct [i32 mut], fn [] [i32] ]} \
            funcs {[ 1 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[1 eqref] \
                i32.const 33 struct.new 0 local.set 0 \
                local.get 0 ref.cast.null 0 struct.get 0 0 \
                end } ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_gc_start(&ctx));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t result;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
        assert_eq_i32(result.i32, 33);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Running test_struct_get_rejects_abstract_structref...\n");
    {
        // struct.get should reject abstract structref (must have concrete type)
        const char *spec = "wasm \
            types {[ struct [i32 mut], fn [] [i32] ]} \
            funcs {[ 1 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[1 structref] \
                struct.new_default 0 local.set 0 \
                local.get 0 struct.get 0 0 \
                end } ]}";
        wah_module_t module;
        assert_err(wah_parse_module_from_spec(&module, spec), WAH_ERROR_VALIDATION_FAILED);
        wah_free_module(&module);
    }

    printf("Running test_br_on_null_refines_to_non_nullable...\n");
    {
        // br_on_null refines nullable ref to non-nullable; struct.get should accept it.
        // br_on_null $l : [t* (ref null ht)] -> [t* (ref ht)]
        // label 0 has result type [i32], so we need an i32 on stack before the ref.
        const char *spec = "wasm \
            types {[ struct [i32 mut], fn [type.ref.null 0] [i32] ]} \
            funcs {[ 1 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[] \
                block [] \
                local.get 0 \
                br_on_null 0 \
                struct.get 0 0 \
                return \
                end \
                i32.const 0 \
                end } ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_gc_start(&ctx));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t args[1] = {{.ref = NULL}};
        wah_value_t result;
        // null input: br_on_null takes the branch, returns default i32 0
        assert_ok(wah_call(&ctx, 0, args, 1, &result));
        assert_eq_i32(result.i32, 0);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    // --- Type flags threading through validation paths ---

    printf("Running test_call_rejects_nullable_for_non_null_param...\n");
    {
        // func 0 takes (ref 0) [non-nullable], passing nullable ref should fail
        const char *spec = "wasm \
            types {[ struct [i32 mut], fn [type.ref 0] [i32], fn [] [i32] ]} \
            funcs {[ 1, 2 ]} \
            exports {[ {'f'} fn# 1 ]} \
            code {[ \
                {[] local.get 0 struct.get 0 0 end }, \
                {[1 type.ref.null 0] struct.new_default 0 local.set 0 local.get 0 call 0 end } \
            ]}";
        wah_module_t module;
        assert_err(wah_parse_module_from_spec(&module, spec), WAH_ERROR_VALIDATION_FAILED);
        wah_free_module(&module);
    }

    printf("Running test_call_accepts_non_null_for_non_null_param...\n");
    {
        // func 0 takes (ref 0) [non-nullable], passing non-null struct.new result should succeed
        const char *spec = "wasm \
            types {[ struct [i32 mut], fn [type.ref 0] [i32], fn [] [i32] ]} \
            funcs {[ 1, 2 ]} \
            exports {[ {'f'} fn# 1 ]} \
            code {[ \
                {[] local.get 0 struct.get 0 0 end }, \
                {[] i32.const 42 struct.new 0 call 0 end } \
            ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_gc_start(&ctx));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t result;
        assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
        assert_eq_i32(result.i32, 42);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Running test_block_rejects_nullable_for_non_null_param...\n");
    {
        // block expects non-nullable param, nullable on stack should fail
        const char *spec = "wasm \
            types {[ struct [i32 mut], fn [type.ref 0] [i32], fn [] [i32] ]} \
            funcs {[ 2 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[1 type.ref.null 0] \
                struct.new_default 0 local.set 0 \
                local.get 0 \
                block 1 struct.get 0 0 end \
                end } ]}";
        wah_module_t module;
        assert_err(wah_parse_module_from_spec(&module, spec), WAH_ERROR_VALIDATION_FAILED);
        wah_free_module(&module);
    }

    printf("Running test_block_accepts_non_null_for_non_null_param...\n");
    {
        // block expects non-nullable param, non-null struct.new result should succeed
        const char *spec = "wasm \
            types {[ struct [i32 mut], fn [type.ref 0] [i32], fn [] [i32] ]} \
            funcs {[ 2 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[] \
                i32.const 7 struct.new 0 \
                block 1 struct.get 0 0 end \
                end } ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_gc_start(&ctx));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t result;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
        assert_eq_i32(result.i32, 7);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Running test_return_rejects_nullable_for_non_null_result...\n");
    {
        // function result is non-nullable (ref 0), returning nullable should fail
        const char *spec = "wasm \
            types {[ struct [i32 mut], fn [] [type.ref 0] ]} \
            funcs {[ 1 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[1 type.ref.null 0] struct.new_default 0 local.set 0 local.get 0 end } ]}";
        wah_module_t module;
        assert_err(wah_parse_module_from_spec(&module, spec), WAH_ERROR_VALIDATION_FAILED);
        wah_free_module(&module);
    }

    printf("Running test_return_accepts_non_null_for_non_null_result...\n");
    {
        // function result is non-nullable (ref 0), returning struct.new should succeed
        const char *spec = "wasm \
            types {[ struct [i32 mut], fn [] [type.ref 0] ]} \
            funcs {[ 1 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[] i32.const 1 struct.new 0 end } ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_gc_start(&ctx));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t result;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
        assert_not_null(result.ref);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Running test_br_rejects_nullable_for_non_null_block_result...\n");
    {
        // block result is non-nullable, br with nullable should fail
        const char *spec = "wasm \
            types {[ struct [i32 mut], fn [] [type.ref 0], fn [] [i32] ]} \
            funcs {[ 2 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[1 type.ref.null 0] \
                struct.new_default 0 local.set 0 \
                block 1 local.get 0 br 0 end \
                struct.get 0 0 end } ]}";
        wah_module_t module;
        assert_err(wah_parse_module_from_spec(&module, spec), WAH_ERROR_VALIDATION_FAILED);
        wah_free_module(&module);
    }

    // --- Typed-reference semantics: element/table/call_indirect ---

    printf("Running test_elem_segment_externref...\n");
    {
        // Element segment with externref type (mode 5 = passive expr)
        // Should parse successfully since we no longer restrict to funcref only
        const char *spec = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0 ]} \
            tables {[ externref limits.i32/1 1 ]} \
            exports {[ {'f'} fn# 0 ]} \
            elements {[ elem.passive.expr externref [ref.null externref end] ]} \
            code {[ {[] i32.const 1 end } ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));
        { wah_type_t et;
          assert_ok(wah_debug_element_segment(&module, 0, &et));
          assert_eq_i32(et, WAH_TYPE_EXTERNREF); assert_true(WAH_TYPE_IS_NULLABLE(et)); }
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_instantiate(&ctx));
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Running test_elem_segment_type_mismatch_rejected...\n");
    {
        // Active element with externref into a funcref table should fail validation
        const char *spec = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0 ]} \
            tables {[ funcref limits.i32/1 1 ]} \
            exports {[ {'f'} fn# 0 ]} \
            elements {[ elem.active.expr.table# 0 i32.const 0 end externref [ref.null externref end] ]} \
            code {[ {[] i32.const 1 end } ]}";
        wah_module_t module;
        assert_err(wah_parse_module_from_spec(&module, spec), WAH_ERROR_VALIDATION_FAILED);
        wah_free_module(&module);
    }

    printf("Running test_elem_funcref_into_funcref_table...\n");
    {
        // Active element with funcref into funcref table (same type) should succeed
        const char *spec = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0 ]} \
            tables {[ funcref limits.i32/1 1 ]} \
            exports {[ {'f'} fn# 0 ]} \
            elements {[ elem.active.expr.table# 0 i32.const 0 end funcref [ref.func 0 end] ]} \
            code {[ {[] i32.const 42 end } ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t result;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
        assert_eq_i32(result.i32, 42);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Running test_table_init_elem_type_subtype...\n");
    {
        // table.init: element segment with nullfuncref (subtype of funcref) into funcref table
        // nullfuncref <: funcref, so this should be accepted
        const char *spec = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0 ]} \
            tables {[ funcref limits.i32/1 1 ]} \
            exports {[ {'f'} fn# 0 ]} \
            elements {[ elem.passive.expr nullfuncref [ref.null nullfuncref end] ]} \
            code {[ {[] \
                i32.const 0 i32.const 0 i32.const 1 table.init 0 0 \
                i32.const 1 end } ]}";
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

    printf("Running test_table_init_type_mismatch_rejected...\n");
    {
        // table.init: externref element into funcref table should fail validation
        const char *spec = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0 ]} \
            tables {[ funcref limits.i32/1 1 ]} \
            exports {[ {'f'} fn# 0 ]} \
            elements {[ elem.passive.expr externref [ref.null externref end] ]} \
            code {[ {[] \
                i32.const 0 i32.const 0 i32.const 1 table.init 0 0 \
                i32.const 1 end } ]}";
        wah_module_t module;
        assert_err(wah_parse_module_from_spec(&module, spec), WAH_ERROR_VALIDATION_FAILED);
        wah_free_module(&module);
    }

    printf("Running test_table_copy_subtype...\n");
    {
        // table.copy from a nullfuncref table to a funcref table
        // nullfuncref <: funcref, so validation should accept
        const char *spec = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0 ]} \
            tables {[ funcref limits.i32/1 1, nullfuncref limits.i32/1 1 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[] \
                i32.const 0 i32.const 0 i32.const 1 table.copy 0 1 \
                i32.const 1 end } ]}";
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

    printf("Running test_table_copy_type_mismatch_rejected...\n");
    {
        // table.copy from externref table to funcref table should fail
        const char *spec = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0 ]} \
            tables {[ funcref limits.i32/1 1, externref limits.i32/1 1 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[] \
                i32.const 0 i32.const 0 i32.const 1 table.copy 0 1 \
                i32.const 1 end } ]}";
        wah_module_t module;
        assert_err(wah_parse_module_from_spec(&module, spec), WAH_ERROR_VALIDATION_FAILED);
        wah_free_module(&module);
    }

    printf("Running test_call_indirect_concrete_functype_table...\n");
    {
        // Table with concrete function type (ref null $0), call_indirect through it
        // type 0: fn [i32] -> [i32]  (concrete type for the table)
        // Element segment uses (ref null $0) to match the table's element type
        const char *spec = "wasm \
            types {[ fn [i32] [i32] ]} \
            funcs {[ 0, 0 ]} \
            tables {[ type.ref.null 0 limits.i32/1 1 ]} \
            exports {[ {'run'} fn# 1 ]} \
            elements {[ elem.active.expr.table# 0 i32.const 0 end type.ref.null 0 [ref.func 0 end] ]} \
            code {[ \
                {[] local.get 0 i32.const 1 i32.add end}, \
                {[] local.get 0 i32.const 0 call_indirect 0 0 end} \
            ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t param = {.i32 = 10};
        wah_value_t result;
        assert_ok(wah_call(&ctx, 1, &param, 1, &result));
        assert_eq_i32(result.i32, 11);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Running test_global_concrete_funcref_type...\n");
    {
        // Global with concrete function type (ref null $0) initialized via ref.func
        // This tests that prefuncref conversion works for non-funcref function reference types
        const char *spec = "wasm \
            types {[ fn [i32] [i32] ]} \
            funcs {[ 0, 0 ]} \
            tables {[ funcref limits.i32/1 1 ]} \
            globals {[ type.ref.null 0 mut ref.func 0 end ]} \
            exports {[ {'run'} fn# 1 ]} \
            code {[ \
                {[] local.get 0 i32.const 5 i32.add end}, \
                {[] i32.const 0 global.get 0 table.set 0 \
                    local.get 0 i32.const 0 call_indirect 0 0 end} \
            ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t param = {.i32 = 7};
        wah_value_t result;
        assert_ok(wah_call(&ctx, 1, &param, 1, &result));
        assert_eq_i32(result.i32, 12);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Running test_declarative_elem_externref...\n");
    {
        // Declarative element with externref (mode 7 = declarative expr)
        const char *spec = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0 ]} \
            exports {[ {'f'} fn# 0 ]} \
            elements {[ elem.declarative.expr externref [ref.null externref end] ]} \
            code {[ {[] i32.const 1 end } ]}";
        wah_module_t module;
        assert_ok(wah_parse_module_from_spec(&module, spec));
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_instantiate(&ctx));
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    // --- Host-boundary behavior for externref ---

    test_externref_host_roundtrip();
    test_externref_table_host_boundary();
    test_externref_global_host_boundary();

    test_externref_ref_test();
    test_externref_ref_cast();

    printf("\nAll Reference Types tests passed!\n");
    return 0;
}
