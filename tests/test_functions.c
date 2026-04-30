#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../wah.h"
#include "common.h"

// --- Start Section Test ---

void test_start_section() {
    wah_module_t module;
    wah_exec_context_t ctx;

    // A WebAssembly binary with a start section that sets a global variable.
    const char *start_section_wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        globals {[ i32 mut i32.const 0 end ]} \
        start { 0 } \
        code {[ {[] i32.const 42 global.set 0 end } ]}";

    printf("Testing Start Section...\n");
    assert_ok(wah_parse_module_from_spec(&module, start_section_wasm_spec));

    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
    // Start function runs during instantiation (after globals/imports/elements are ready).
    assert_ok(wah_instantiate(&ctx));

    // Verify that the global variable was set by the start function
    assert_eq_i32(ctx.globals[0].i32, 42);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// --- Multiple Return Functionality Tests ---

// Test zero return functions with wah_call and wah_call_multi
void test_zero_return_functions() {
    printf("Testing Zero Return Functions...\n");

    // Simple void function: () -> ()
    static const char *wasm_binary_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        code {[ {[] end } ]}";

    wah_module_t module;
    wah_exec_context_t ctx;

    assert_ok(wah_parse_module_from_spec(&module, wasm_binary_spec));
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));

    // Test wah_call with zero return function
    wah_value_t result;
    memset(&result, 0xFF, sizeof(wah_value_t)); // Initialize with garbage to verify zeroization
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));

    // Verify that result is properly zeroized
    assert_eq_i32(result.i32, 0);
    assert_eq_i64(result.i64, 0);
    assert_eq_f32(result.f32, 0.0f, 0);
    assert_eq_f64(result.f64, 0.0, 0);

    // Test wah_call_multi with zero return function
    wah_value_t results[1];
    uint32_t actual_results;
    assert_ok(wah_call_multi(&ctx, 0, NULL, 0, results, 1, &actual_results));
    assert_eq_u32(actual_results, 0);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// Test single return functions (baseline)
void test_single_return_functions() {
    printf("Testing Single Return Functions...\n");

    // Function that returns i32 constant 42
    static const char *wasm_binary_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] i32.const 42 end } ]}";

    wah_module_t module;
    wah_exec_context_t ctx;

    assert_ok(wah_parse_module_from_spec(&module, wasm_binary_spec));
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));

    // Test wah_call with single return function
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);

    // Test wah_call_multi with single return function
    wah_value_t results[1];
    uint32_t actual_results;
    assert_ok(wah_call_multi(&ctx, 0, NULL, 0, results, 1, &actual_results));
    assert_eq_u32(actual_results, 1);
    assert_eq_i32(results[0].i32, 42);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// Test using existing globals module to verify wah_call_multi works
void test_multiple_return_with_existing_functions() {
    printf("Testing Multiple Return API with Existing Functions...\n");

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

    assert_ok(wah_parse_module_from_spec(&module, wasm_binary_spec));
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));

    // Test function 0: get_i64 with wah_call_multi (should return WAH_OK)
    wah_value_t results[1];
    uint32_t actual_results;
    assert_ok(wah_call_multi(&ctx, 0, NULL, 0, results, 1, &actual_results));

    // Test function 2: get_f32 with wah_call_multi
    wah_value_t f32_results[1];
    uint32_t f32_actual_results;
    assert_ok(wah_call_multi(&ctx, 2, NULL, 0, f32_results, 1, &f32_actual_results));
    assert_eq_u32(f32_actual_results, 1);
    assert_eq_f32(f32_results[0].f32, 1.5f, 0);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// Test buffer validation with wah_call_multi
void test_multi_return_buffer_validation() {
    printf("Testing Multi-Return Buffer Validation...\n");

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

    assert_ok(wah_parse_module_from_spec(&module, wasm_binary_spec));
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));

    // Test with max_results=0: function executes, actual_results reports true count
    wah_value_t results[1];
    uint32_t actual_results;
    assert_ok(wah_call_multi(&ctx, 0, NULL, 0, results, 0, &actual_results));
    assert_eq_u32(actual_results, 1);
    
    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

int main() {
    test_start_section();
    test_zero_return_functions();
    test_single_return_functions();
    test_multiple_return_with_existing_functions();
    test_multi_return_buffer_validation();

    printf("\nAll tests passed!\n");
    return 0;
}
