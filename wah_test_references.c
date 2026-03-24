#define WAH_IMPLEMENTATION
#include "wah.h"
#include "wah_testutils.c"
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

int main() {
    test_ref_null_funcref();
    test_ref_null_externref();
    test_ref_func();
    test_ref_is_null_funcref_null();
    test_ref_is_null_funcref_nonnull();
    test_ref_is_null_externref_null();
    test_ref_func_invalid_index();

    printf("\nAll Reference Types tests passed!\n");
    return 0;
}