#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h> // For fabsf

#define WAH_IMPLEMENTATION
#include "wah.h"
#include "wah_testutils.c"

static const char *wasm_binary_spec = "wasm \
    types {[ \
        fn [] [i64], \
        fn [i64] [], \
        fn [] [f32], \
        fn [f32] [], \
    ]} \
    funcs {[ 0, 1, 2, 3 ]} \
    globals {[ \
        i64 mut i64.const 200 end, \
        f32 mut f32.const 1.5f32 end, \
    ]} \
    code {[ \
        {[] global.get 0 end}, \
        {[] local.get 0 global.set 0 end}, \
        {[] global.get 1 end}, \
        {[] local.get 0 global.set 1 end}, \
    ]}";

static const char *wasm_binary_type_mismatch_spec = "wasm \
    types {[]} \
    globals {[ i64 mut f32.const 1.5f32 end ]}";

void test_i64_global(wah_exec_context_t* exec_ctx, wah_module_t* module) {
    wah_value_t result;
    wah_error_t err;

    printf("Testing i64 global...\n");
    // Get initial value
    err = wah_call(exec_ctx, module, 0, NULL, 0, &result);
    assert(err == WAH_OK);
    printf("Initial i64 value: %lld\n", (long long)result.i64);
    assert(result.i64 == 200);

    // Set new value
    wah_value_t params_i64[1];
    params_i64[0].i64 = -5000;
    err = wah_call(exec_ctx, module, 1, params_i64, 1, NULL);
    assert(err == WAH_OK);

    // Get new value
    err = wah_call(exec_ctx, module, 0, NULL, 0, &result);
    assert(err == WAH_OK);
    printf("New i64 value: %lld\n", (long long)result.i64);
    assert(result.i64 == -5000);
}

void test_f32_global(wah_exec_context_t* exec_ctx, wah_module_t* module) {
    wah_value_t result;
    wah_error_t err;

    printf("Testing f32 global...\n");
    // Get initial value
    err = wah_call(exec_ctx, module, 2, NULL, 0, &result);
    assert(err == WAH_OK);
    printf("Initial f32 value: %f\n", result.f32);
    assert(fabsf(result.f32 - 1.5f) < 1e-6);

    // Set new value
    wah_value_t params_f32[1];
    params_f32[0].f32 = 9.99f;
    err = wah_call(exec_ctx, module, 3, params_f32, 1, NULL);
    assert(err == WAH_OK);

    // Get new value
    err = wah_call(exec_ctx, module, 2, NULL, 0, &result);
    assert(err == WAH_OK);
    printf("New f32 value: %f\n", result.f32);
    assert(fabsf(result.f32 - 9.99f) < 1e-6);
}

void test_global_type_mismatch(void) {
    wah_module_t module_mismatch;
    wah_error_t err_mismatch;

    printf("\n--- Running Global Type Mismatch Test ---\n");

    // Parse the module - this should fail with WAH_ERROR_TYPE_MISMATCH
    err_mismatch = wah_parse_module_from_spec(&module_mismatch, wasm_binary_type_mismatch_spec);

    printf("Expected error: %s, Actual error: %s\n", wah_strerror(WAH_ERROR_VALIDATION_FAILED), wah_strerror(err_mismatch));
    assert(err_mismatch == WAH_ERROR_VALIDATION_FAILED);

    printf("--- Global Type Mismatch Test Passed (as expected failure) ---\n");
}

int main(void) {
    wah_module_t module;
    wah_exec_context_t exec_ctx;
    wah_error_t err;

    printf("--- Running Globals Test ---\n");

    // Parse the module
    err = wah_parse_module_from_spec(&module, wasm_binary_spec);
    if (err != WAH_OK) {
        printf("Failed to parse module: %s\n", wah_strerror(err));
        assert(err == WAH_OK);
    }

    // Create execution context
    err = wah_exec_context_create(&exec_ctx, &module);
    assert(err == WAH_OK);

    test_i64_global(&exec_ctx, &module);
    test_f32_global(&exec_ctx, &module);

    // Cleanup for regular globals test
    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);

    printf("--- Globals Test Passed ---\n");

    test_global_type_mismatch();

    return 0;
}
