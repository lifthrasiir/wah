#define WAH_IMPLEMENTATION
#include "wah.h"
#include "wah_testutils.c"
#include <stdio.h>
#include <assert.h>

static int test_ref_null_funcref() {
    printf("Running test_ref_null_funcref...\n");

    wah_module_t module;
    wah_error_t err = wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [] [funcref] ]} \
        funcs {[ 0 ]} \
        exports {[ {'test_func'} fn# 0 ]} \
        code {[ {[] ref.null funcref end } ]}");
    if (err != WAH_OK) {
        printf("  - FAILED: Could not parse module: %s\n", wah_strerror(err));
        return 1;
    }

    wah_exec_context_t exec_ctx;
    wah_value_t result;

    err = wah_exec_context_create(&exec_ctx, &module);
    if (err != WAH_OK) {
        printf("  - FAILED: Could not create execution context: %s\n", wah_strerror(err));
        return 1;
    }

    err = wah_call(&exec_ctx, 0, NULL, 0, &result);
    if (err != WAH_OK) {
        printf("  - FAILED: Could not call function: %s\n", wah_strerror(err));
        wah_exec_context_destroy(&exec_ctx);
        return 1;
    }

    if (result.ref != NULL) {
        printf("  - FAILED: Expected funcref to be NULL, got %p\n", result.ref);
        wah_exec_context_destroy(&exec_ctx);
        wah_free_module(&module);
        return 1;
    }

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
    printf("  - PASSED: ref.null funcref returned NULL\n");
    return 0;
}

static int test_ref_null_externref() {
    printf("Running test_ref_null_externref...\n");

    wah_module_t module;
    wah_error_t err = wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [] [externref] ]} \
        funcs {[ 0 ]} \
        exports {[ {'test_extern'} fn# 0 ]} \
        code {[ {[] ref.null externref end } ]}");
    if (err != WAH_OK) {
        printf("  - FAILED: Could not parse module: %s\n", wah_strerror(err));
        return 1;
    }

    wah_exec_context_t exec_ctx;
    wah_value_t result;

    err = wah_exec_context_create(&exec_ctx, &module);
    if (err != WAH_OK) {
        printf("  - FAILED: Could not create execution context: %s\n", wah_strerror(err));
        return 1;
    }

    err = wah_call(&exec_ctx, 0, NULL, 0, &result);
    if (err != WAH_OK) {
        printf("  - FAILED: Could not call function: %s\n", wah_strerror(err));
        wah_exec_context_destroy(&exec_ctx);
        return 1;
    }

    if (result.ref != NULL) {
        printf("  - FAILED: Expected externref to be NULL, got %p\n", result.ref);
        wah_exec_context_destroy(&exec_ctx);
        wah_free_module(&module);
        return 1;
    }

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
    printf("  - PASSED: ref.null externref returned NULL\n");
    return 0;
}

static int test_ref_func() {
    printf("Running test_ref_func...\n");

    wah_module_t module;
    wah_error_t err = wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [] [funcref] ]} \
        funcs {[ 0 ]} \
        exports {[ {'test_ref_func'} fn# 0 ]} \
        code {[ {[] ref.func 0 end } ]}");
    if (err != WAH_OK) {
        printf("  - FAILED: Could not parse module: %s\n", wah_strerror(err));
        return 1;
    }

    wah_exec_context_t exec_ctx;
    wah_value_t result;

    err = wah_exec_context_create(&exec_ctx, &module);
    if (err != WAH_OK) {
        printf("  - FAILED: Could not create execution context: %s\n", wah_strerror(err));
        return 1;
    }

    err = wah_call(&exec_ctx, 0, NULL, 0, &result);
    if (err != WAH_OK) {
        printf("  - FAILED: Could not call function: %s\n", wah_strerror(err));
        wah_exec_context_destroy(&exec_ctx);
        return 1;
    }

    if (result.ref == NULL) {
        printf("  - FAILED: Expected funcref to be non-NULL, got NULL\n");
        wah_exec_context_destroy(&exec_ctx);
        wah_free_module(&module);
        return 1;
    }

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
    printf("  - PASSED: ref.func 0 returned valid function reference\n");
    return 0;
}

static int test_ref_is_null_funcref_null() {
    printf("Running test_ref_is_null_funcref_null...\n");

    wah_module_t module;
    wah_error_t err = wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        exports {[ {'test_ref_is_null'} fn# 0 ]} \
        code {[ {[] ref.null funcref ref.is_null end } ]}");
    if (err != WAH_OK) {
        printf("  - FAILED: Could not parse module: %s\n", wah_strerror(err));
        return 1;
    }

    wah_exec_context_t exec_ctx;
    wah_value_t result;

    err = wah_exec_context_create(&exec_ctx, &module);
    if (err != WAH_OK) {
        printf("  - FAILED: Could not create execution context: %s\n", wah_strerror(err));
        return 1;
    }

    err = wah_call(&exec_ctx, 0, NULL, 0, &result);
    if (err != WAH_OK) {
        printf("  - FAILED: Could not call function: %s\n", wah_strerror(err));
        wah_exec_context_destroy(&exec_ctx);
        return 1;
    }

    if (result.i32 != 1) {
        printf("  - FAILED: Expected ref.is_null to return 1 (true), got %d\n", result.i32);
        wah_exec_context_destroy(&exec_ctx);
        wah_free_module(&module);
        return 1;
    }

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
    printf("  - PASSED: ref.is_null returned 1 for null funcref\n");
    return 0;
}

static int test_ref_is_null_funcref_nonnull() {
    printf("Running test_ref_is_null_funcref_nonnull...\n");

    wah_module_t module;
    wah_error_t err = wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        exports {[ {'test_ref_is_null'} fn# 0 ]} \
        code {[ {[] ref.func 0 ref.is_null end } ]}");
    if (err != WAH_OK) {
        printf("  - FAILED: Could not parse module: %s\n", wah_strerror(err));
        return 1;
    }

    wah_exec_context_t exec_ctx;
    wah_value_t result;

    err = wah_exec_context_create(&exec_ctx, &module);
    if (err != WAH_OK) {
        printf("  - FAILED: Could not create execution context: %s\n", wah_strerror(err));
        return 1;
    }

    err = wah_call(&exec_ctx, 0, NULL, 0, &result);
    if (err != WAH_OK) {
        printf("  - FAILED: Could not call function: %s\n", wah_strerror(err));
        wah_exec_context_destroy(&exec_ctx);
        return 1;
    }

    if (result.i32 != 0) {
        printf("  - FAILED: Expected ref.is_null to return 0 (false), got %d\n", result.i32);
        wah_exec_context_destroy(&exec_ctx);
        wah_free_module(&module);
        return 1;
    }

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
    printf("  - PASSED: ref.is_null returned 0 for non-null funcref\n");
    return 0;
}

static int test_ref_is_null_externref_null() {
    printf("Running test_ref_is_null_externref_null...\n");

    wah_module_t module;
    wah_error_t err = wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        exports {[ {'test_ref_is_null'} fn# 0 ]} \
        code {[ {[] ref.null externref ref.is_null end } ]}");
    if (err != WAH_OK) {
        printf("  - FAILED: Could not parse module: %s\n", wah_strerror(err));
        return 1;
    }

    wah_exec_context_t exec_ctx;
    wah_value_t result;

    err = wah_exec_context_create(&exec_ctx, &module);
    if (err != WAH_OK) {
        printf("  - FAILED: Could not create execution context: %s\n", wah_strerror(err));
        return 1;
    }

    err = wah_call(&exec_ctx, 0, NULL, 0, &result);
    if (err != WAH_OK) {
        printf("  - FAILED: Could not call function: %s\n", wah_strerror(err));
        wah_exec_context_destroy(&exec_ctx);
        return 1;
    }

    if (result.i32 != 1) {
        printf("  - FAILED: Expected ref.is_null to return 1 (true), got %d\n", result.i32);
        wah_exec_context_destroy(&exec_ctx);
        wah_free_module(&module);
        return 1;
    }

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
    printf("  - PASSED: ref.is_null returned 1 for null externref\n");
    return 0;
}

// This tests that ref.func with an invalid function index fails validation.
// This ensures the assertion "validation should have verified function index" holds.
static int test_ref_func_invalid_index() {
    printf("Running test_ref_func_invalid_index...\n");

    wah_module_t module;
    wah_error_t err = wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [] [funcref] ]} \
        funcs {[ 0 ]} \
        exports {[ {'test_invalid'} fn# 0 ]} \
        code {[ {[] ref.func 99 end } ]}");  // Function index 99 doesn't exist

    if (err != WAH_ERROR_VALIDATION_FAILED) {
        printf("  - FAILED: Expected WAH_ERROR_VALIDATION_FAILED for invalid function index, got: %s\n", wah_strerror(err));
        if (err == WAH_OK) {
            wah_free_module(&module);
        }
        return 1;
    }

    printf("  - PASSED: ref.func with invalid index correctly rejected\n");
    return 0;
}

int main() {
    printf("Testing WebAssembly Reference Types...\n\n");

    int failed = 0;

    failed |= test_ref_null_funcref();
    failed |= test_ref_null_externref();
    failed |= test_ref_func();
    failed |= test_ref_is_null_funcref_null();
    failed |= test_ref_is_null_funcref_nonnull();
    failed |= test_ref_is_null_externref_null();
    failed |= test_ref_func_invalid_index();

    if (failed) {
        printf("\nSome tests failed.\n");
        return 1;
    } else {
        printf("\nAll Reference Types tests passed!\n");
        return 0;
    }
}