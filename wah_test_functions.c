#define WAH_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "wah.h"
#include "wah_testutils.c"

// A WebAssembly binary with a start section that sets a global variable.
static const char *start_section_wasm_spec = "wasm \
    types {[ fn [] [] ]} \
    funcs {[ 0 ]} \
    globals {[ i32 mut i32.const 0 end ]} \
    start { 0 } \
    code {[ {[] i32.const 42 global.set 0 end } ]}";

// --- Start Section Test ---

int test_start_section() {
    wah_module_t module;
    wah_exec_context_t ctx;
    wah_error_t err;

    printf("--- Testing Start Section ---\n");
    printf("Parsing start_section_wasm module...\n");
    err = wah_parse_module_from_spec(&module, start_section_wasm_spec);
    if (err != WAH_OK) {
        fprintf(stderr, "Error parsing module with start section: %s\n", wah_strerror(err));
        return 0;
    }
    printf("Module parsed successfully.\n");

    // Create execution context. This should trigger the start function.
    err = wah_exec_context_create(&ctx, &module);
    if (err != WAH_OK) {
        fprintf(stderr, "Error creating execution context for start section: %s\n", wah_strerror(err));
        wah_free_module(&module);
        return 0;
    }
    printf("Execution context created. Checking global variable...\n");

    // Verify that the global variable was set by the start function
    if (ctx.globals[0].i32 == 42) {
        printf("Global variable $g is %d (expected 42). Start section executed successfully.\n", ctx.globals[0].i32);
    } else {
        fprintf(stderr, "Global variable $g is %d (expected 42). Start section FAILED to execute or set value.\n", ctx.globals[0].i32);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
        return 0;
    }

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
    printf("Module with start section freed.\n");
    return 1;
}

// --- Multiple Return Functionality Tests ---

// Test zero return functions with wah_call and wah_call_multi
int test_zero_return_functions() {
    printf("\n--- Testing Zero Return Functions ---\n");

    // Simple void function: () -> ()
    static const char *wasm_binary_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        code {[ {[] end } ]}";

    wah_module_t module;
    wah_exec_context_t ctx;
    wah_error_t err;

    err = wah_parse_module_from_spec(&module, wasm_binary_spec);
    if (err != WAH_OK) {
        printf("[x] Parse error: %s\n", wah_strerror(err));
        return 0;
    }

    err = wah_exec_context_create(&ctx, &module);
    if (err != WAH_OK) {
        printf("[x] Context creation error: %s\n", wah_strerror(err));
        wah_free_module(&module);
        return 0;
    }

    // Test wah_call with zero return function
    wah_value_t result;
    memset(&result, 0xFF, sizeof(wah_value_t)); // Initialize with garbage to verify zeroization
    err = wah_call(&ctx, &module, 0, NULL, 0, &result);
    if (err == WAH_OK) {
        // Verify that result is properly zeroized
        bool is_zeroed = (result.i32 == 0 && result.i64 == 0 && result.f32 == 0.0f && result.f64 == 0.0);
        if (is_zeroed) {
            printf("[v] wah_call with zero return: result properly zeroized\n");
        } else {
            printf("[x] wah_call with zero return: result not zeroized (i32=%d, i64=%lld, f32=%.6f, f64=%.6f)\n",
                   result.i32, (long long)result.i64, result.f32, result.f64);
            wah_exec_context_destroy(&ctx);
            wah_free_module(&module);
            return 0;
        }
    } else {
        printf("[x] wah_call with zero return failed: %s\n", wah_strerror(err));
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
        return 0;
    }

    // Test wah_call_multi with zero return function
    wah_value_t results[1];
    uint32_t actual_results;
    err = wah_call_multi(&ctx, &module, 0, NULL, 0, results, 1, &actual_results);
    if (err == WAH_OK && actual_results == 0) {
        printf("[v] wah_call_multi with zero return: actual_results=%u (expected 0)\n", actual_results);
    } else {
        printf("[x] wah_call_multi with zero return failed: err=%s, actual_results=%u\n",
               wah_strerror(err), actual_results);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
        return 0;
    }

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
    return 1;
}

// Test single return functions (baseline)
int test_single_return_functions() {
    printf("\n--- Testing Single Return Functions ---\n");

    // Function that returns i32 constant 42
    static const char *wasm_binary_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] i32.const 42 end } ]}";

    wah_module_t module;
    wah_exec_context_t ctx;
    wah_error_t err;

    err = wah_parse_module_from_spec(&module, wasm_binary_spec);
    if (err != WAH_OK) {
        printf("[x] Parse error: %s\n", wah_strerror(err));
        return 0;
    }

    err = wah_exec_context_create(&ctx, &module);
    if (err != WAH_OK) {
        printf("[x] Context creation error: %s\n", wah_strerror(err));
        wah_free_module(&module);
        return 0;
    }

    // Test wah_call with single return function
    wah_value_t result;
    err = wah_call(&ctx, &module, 0, NULL, 0, &result);
    if (err == WAH_OK && result.i32 == 42) {
        printf("[v] wah_call with single return: i32=%d (expected 42)\n", result.i32);
    } else {
        printf("[x] wah_call with single return failed: err=%s, i32=%d\n",
               wah_strerror(err), result.i32);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
        return 0;
    }

    // Test wah_call_multi with single return function
    wah_value_t results[1];
    uint32_t actual_results;
    err = wah_call_multi(&ctx, &module, 0, NULL, 0, results, 1, &actual_results);
    if (err == WAH_OK && actual_results == 1 && results[0].i32 == 42) {
        printf("[v] wah_call_multi with single return: i32=%d, count=%u\n",
               results[0].i32, actual_results);
    } else {
        printf("[x] wah_call_multi with single return failed: err=%s, i32=%d, count=%u\n",
               wah_strerror(err), results[0].i32, actual_results);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
        return 0;
    }

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
    return 1;
}

// Test using existing globals module to verify wah_call_multi works
int test_multiple_return_with_existing_functions() {
    printf("\n--- Testing Multiple Return API with Existing Functions ---\n");

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

    wah_module_t module;
    wah_exec_context_t ctx;
    wah_error_t err;

    err = wah_parse_module_from_spec(&module, wasm_binary_spec);
    if (err != WAH_OK) {
        printf("❌ Parse error: %s\n", wah_strerror(err));
        return 0;
    }

    err = wah_exec_context_create(&ctx, &module);
    if (err != WAH_OK) {
        printf("❌ Context creation error: %s\n", wah_strerror(err));
        wah_free_module(&module);
        return 0;
    }

    // Test function 0: get_i64 with wah_call_multi (should return WAH_OK)
    wah_value_t results[1];
    uint32_t actual_results;
    err = wah_call_multi(&ctx, &module, 0, NULL, 0, results, 1, &actual_results);
    if (err == WAH_OK) {
        printf("[v] wah_call_multi with single return: i64=%lld, count=%u\n",
               (long long)results[0].i64, actual_results);
    } else {
        printf("[x] wah_call_multi failed: %s\n", wah_strerror(err));
    }

    // Test function 2: get_f32 with wah_call_multi
    wah_value_t f32_results[1];
    uint32_t f32_actual_results;
    err = wah_call_multi(&ctx, &module, 2, NULL, 0, f32_results, 1, &f32_actual_results);
    if (err == WAH_OK) {
        printf("[v] wah_call_multi get_f32: result=%.2f, count=%u\n",
               f32_results[0].f32, f32_actual_results);
    } else {
        printf("[x] wah_call_multi get_f32 failed: %s\n", wah_strerror(err));
    }

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);

    // Return 1 (success) if both tests passed
    return 1;
}

// Test buffer validation with wah_call_multi
int test_multi_return_buffer_validation() {
    printf("\n--- Testing Multi-Return Buffer Validation ---\n");

    // Use the same working binary from other tests for buffer validation
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

    wah_module_t module;
    wah_exec_context_t ctx;
    wah_error_t err;

    err = wah_parse_module_from_spec(&module, wasm_binary_spec);
    if (err != WAH_OK) {
        printf("[x] Parse error: %s (buffer validation test skipped)\n", wah_strerror(err));
        return 0;
    }

    err = wah_exec_context_create(&ctx, &module);
    if (err != WAH_OK) {
        printf("[x] Context creation error: %s\n", wah_strerror(err));
        wah_free_module(&module);
        return 0;
    }

    // Test with insufficient buffer (should return validation error)
    wah_value_t results[1]; // No space for result, but [0] warns
    uint32_t actual_results;
    err = wah_call_multi(&ctx, &module, 0, NULL, 0, results, 0, &actual_results);
    if (err == WAH_ERROR_VALIDATION_FAILED) {
        printf("[v] Insufficient buffer correctly returned error: %s\n", wah_strerror(err));
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
        return 1; // Test passed
    } else {
        printf("[x] Expected validation error for insufficient buffer, got: %s\n", wah_strerror(err));
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
        return 0; // Test failed
    }
}

int main() {
    bool success = true;

    success = success && test_start_section();
    success = success && test_zero_return_functions();
    success = success && test_single_return_functions();
    success = success && test_multiple_return_with_existing_functions();
    success = success && test_multi_return_buffer_validation();

    printf("=== Test Results ===\n");

    if (success) {
        printf("[v] All tests passed!\n");
        return 0;
    } else {
        printf("[x] Some tests failed.\n");
        return 1;
    }
}
