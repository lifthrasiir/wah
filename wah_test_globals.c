#include <stdio.h>
#include <string.h>
#include <math.h> // For fabsf

#define WAH_IMPLEMENTATION
#include "wah.h"
#include "wah_testutils.c"

void test_i64_global(wah_exec_context_t* exec_ctx) {
    wah_value_t result;

    printf("Testing i64 global...\n");
    // Get initial value
    assert_ok(wah_call(exec_ctx, 0, NULL, 0, &result));
    assert_eq_i64(result.i64, 200);

    // Set new value
    wah_value_t params_i64[1];
    params_i64[0].i64 = -5000;
    assert_ok(wah_call(exec_ctx, 1, params_i64, 1, NULL));

    // Get new value
    assert_ok(wah_call(exec_ctx, 0, NULL, 0, &result));
    assert_eq_i64(result.i64, -5000);
}

void test_f32_global(wah_exec_context_t* exec_ctx) {
    wah_value_t result;

    printf("Testing f32 global...\n");
    // Get initial value
    assert_ok(wah_call(exec_ctx, 2, NULL, 0, &result));
    assert_eq_f32(result.f32, 1.5f, 1e-6);

    // Set new value
    wah_value_t params_f32[1];
    params_f32[0].f32 = 9.99f;
    assert_ok(wah_call(exec_ctx, 3, params_f32, 1, NULL));

    // Get new value
    assert_ok(wah_call(exec_ctx, 2, NULL, 0, &result));
    assert_eq_f32(result.f32, 9.99f, 1e-6);
}

void test_global_type_mismatch(void) {
    wah_module_t module_mismatch;

    printf("\n--- Running Global Type Mismatch Test ---\n");

    // Parse the module - this should fail with WAH_ERROR_TYPE_MISMATCH
    const char *mismatch_wasm = "wasm \
        types {[]} \
        globals {[ i64 mut f32.const 1.5f32 end ]}";
    assert_err(wah_parse_module_from_spec(&module_mismatch, mismatch_wasm), WAH_ERROR_VALIDATION_FAILED);

    printf("--- Global Type Mismatch Test Passed (as expected failure) ---\n");
}

int main(void) {
    wah_module_t module;
    wah_exec_context_t exec_ctx;

    printf("--- Running Globals Test ---\n");

    // Parse the module
    const char *globals_wasm = "wasm \
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
    assert_ok(wah_parse_module_from_spec(&module, globals_wasm));

    // Create execution context
    assert_ok(wah_exec_context_create(&exec_ctx, &module));

    test_i64_global(&exec_ctx);
    test_f32_global(&exec_ctx);

    // Cleanup for regular globals test
    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);

    printf("--- Globals Test Passed ---\n");

    test_global_type_mismatch();

    return 0;
}
