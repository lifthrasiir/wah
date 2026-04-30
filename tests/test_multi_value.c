#include <stdio.h>
#include <stdlib.h>
#include "../wah.h"
#include "common.h"

// --- Test 1: Function returning two i32 values ---
static void test_multi_return_i32_pair() {
    printf("Testing function returning two i32 values...\n");

    const char *spec = "wasm \
        types {[ fn [] [i32, i32] ]} \
        funcs {[ 0 ]} \
        exports {[ {'test'} fn# 0 ]} \
        code {[ {[] \
            i32.const 42 \
            i32.const 99 \
        end } ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));

    wah_value_t results[2];
    uint32_t actual_count;
    assert_ok(wah_call_multi(&ctx, 0, NULL, 0, results, 2, &actual_count));
    assert_eq_u32(actual_count, 2);
    assert_eq_i32(results[0].i32, 42);
    assert_eq_i32(results[1].i32, 99);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// --- Test 2: Function returning three values ---
static void test_multi_return_three_values() {
    printf("Testing function returning three i32 values...\n");

    const char *spec = "wasm \
        types {[ fn [i32, i32, i32] [i32, i32, i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            local.get 0 \
            local.get 1 \
            local.get 2 \
        end } ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));

    wah_value_t params[3];
    params[0].i32 = 10;
    params[1].i32 = 20;
    params[2].i32 = 30;

    wah_value_t results[3];
    uint32_t actual_count;
    assert_ok(wah_call_multi(&ctx, 0, params, 3, results, 3, &actual_count));
    assert_eq_u32(actual_count, 3);
    assert_eq_i32(results[0].i32, 10);
    assert_eq_i32(results[1].i32, 20);
    assert_eq_i32(results[2].i32, 30);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// --- Test 3: Function returning mixed types (i32, f64) ---
static void test_multi_return_mixed_types() {
    printf("Testing function returning mixed types (i32, f64)...\n");

    const char *spec = "wasm \
        types {[ fn [i32, f64] [i32, f64] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            local.get 0 \
            local.get 1 \
        end } ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));

    wah_value_t params[2];
    params[0].i32 = 42;
    params[1].f64 = 3.14;

    wah_value_t results[2];
    uint32_t actual_count;
    assert_ok(wah_call_multi(&ctx, 0, params, 2, results, 2, &actual_count));
    assert_eq_u32(actual_count, 2);
    assert_eq_i32(results[0].i32, 42);
    assert_eq_f64(results[1].f64, 3.14, 0.0001);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// --- Test 4: Function with explicit RETURN returning multiple values ---
static void test_multi_return_with_explicit_return() {
    printf("Testing function with explicit return of multiple values...\n");

    const char *spec = "wasm \
        types {[ fn [] [i32, i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            i32.const 100 \
            i32.const 200 \
            return \
        end } ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));

    wah_value_t results[2];
    uint32_t actual_count;
    assert_ok(wah_call_multi(&ctx, 0, NULL, 0, results, 2, &actual_count));
    assert_eq_u32(actual_count, 2);
    assert_eq_i32(results[0].i32, 100);
    assert_eq_i32(results[1].i32, 200);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// --- Test 5: Function calling another function that returns multiple values ---
static void test_call_function_with_multi_return() {
    printf("Testing calling a function that returns multiple values...\n");

    // Function 0: () -> (i32, i32) -- returns (5, 6)
    // Function 1: () -> (i32) -- calls func 0, adds results
    const char *spec = "wasm \
        types {[ \
            fn [] [i32, i32], \
            fn [] [i32] \
        ]} \
        funcs {[ 0, 1 ]} \
        code {[ \
            {[] \
                i32.const 5 \
                i32.const 6 \
            end}, \
            {[] \
                call 0 \
                i32.add \
            end} \
        ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));

    // Test function 0 directly via wah_call_multi
    wah_value_t results[2];
    uint32_t actual_count;
    assert_ok(wah_call_multi(&ctx, 0, NULL, 0, results, 2, &actual_count));
    assert_eq_u32(actual_count, 2);
    assert_eq_i32(results[0].i32, 5);
    assert_eq_i32(results[1].i32, 6);

    // Test function 1 which calls function 0
    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    // func 1: calls func 0, gets (5, 6), adds them = 11
    assert_eq_i32(result.i32, 11);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// --- Test 6: Function with void block returning multiple values at function level ---
static void test_void_block_with_multi_return_function() {
    printf("Testing void block inside multi-return function...\n");

    const char *spec = "wasm \
        types {[ fn [i32, i32] [i32, i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            block void \
                local.get 0 \
                local.get 1 \
            end \
        end } ]}";

    // void block type cannot leave values on stack -- validation must fail
    wah_module_t module;
    assert_err(wah_parse_module_from_spec(&module, spec), WAH_ERROR_VALIDATION_FAILED);
}

// --- Test 7: Function with if-else returning multiple values at function level ---
static void test_if_else_with_multi_return_function() {
    printf("Testing if-else inside multi-return function...\n");

    const char *spec = "wasm \
        types {[ fn [i32] [i32, i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            local.get 0 \
            if void \
                i32.const 1 \
                i32.const 2 \
            else \
                i32.const 3 \
                i32.const 4 \
            end \
        end } ]}";

    // void if/else type cannot leave values on stack -- validation must fail
    wah_module_t module;
    assert_err(wah_parse_module_from_spec(&module, spec), WAH_ERROR_VALIDATION_FAILED);
}

// --- Test 8: Function returning four values ---
static void test_multi_return_four_values() {
    printf("Testing function returning four values...\n");

    const char *spec = "wasm \
        types {[ fn [] [i32, i32, i32, i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            i32.const 1 \
            i32.const 2 \
            i32.const 3 \
            i32.const 4 \
        end } ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));

    wah_value_t results[4];
    uint32_t actual_count;
    assert_ok(wah_call_multi(&ctx, 0, NULL, 0, results, 4, &actual_count));
    assert_eq_u32(actual_count, 4);
    assert_eq_i32(results[0].i32, 1);
    assert_eq_i32(results[1].i32, 2);
    assert_eq_i32(results[2].i32, 3);
    assert_eq_i32(results[3].i32, 4);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// --- Test 9: wah_call returns WAH_ERROR_MULTI_RETURN for multi-result functions ---
static void test_wah_call_multi_return_error() {
    printf("Testing wah_call returns multi-return error...\n");

    const char *spec = "wasm \
        types {[ fn [] [i32, i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            i32.const 10 \
            i32.const 20 \
        end } ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));

    wah_value_t result;
    assert_err(wah_call(&ctx, 0, NULL, 0, &result), WAH_ERROR_MULTI_RETURN);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// --- Test 9b: wah_call does not execute multi-result functions ---
static void test_wah_call_multi_return_no_execution() {
    printf("Testing wah_call does not execute multi-return function...\n");

    // Function writes to a mutable global then returns two values.
    // If the global stays 0, the function body was never entered.
    const char *spec = "wasm \
        types {[ fn [] [i32, i32] ]} \
        funcs {[ 0 ]} \
        globals {[ i32 mut i32.const 0 end ]} \
        code {[ {[] \
            i32.const 42 global.set 0 \
            i32.const 10 \
            i32.const 20 \
        end } ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_err(wah_call(&ctx, 0, NULL, 0, &result), WAH_ERROR_MULTI_RETURN);

    // Global must still be 0 -- the function was never executed
    assert_eq_i32(ctx.globals[0].i32, 0);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// --- Test 10: Multi-return with locals ---
static void test_multi_return_with_locals() {
    printf("Testing multi-return with local variables...\n");

    // Function: (i32) -> (i32, i32)
    // Computes (x, x*2)
    const char *spec = "wasm \
        types {[ fn [i32] [i32, i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            local.get 0 \
            local.get 0 \
            i32.const 2 \
            i32.mul \
        end } ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));

    wah_value_t param;
    param.i32 = 21;

    wah_value_t results[2];
    uint32_t actual_count;
    assert_ok(wah_call_multi(&ctx, 0, &param, 1, results, 2, &actual_count));
    assert_eq_u32(actual_count, 2);
    assert_eq_i32(results[0].i32, 21);
    assert_eq_i32(results[1].i32, 42);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

int main() {
    printf("=== Multi-Value Return Tests ===\n\n");

    test_multi_return_i32_pair();
    test_multi_return_three_values();
    test_multi_return_mixed_types();
    test_multi_return_with_explicit_return();
    test_call_function_with_multi_return();
    test_void_block_with_multi_return_function();
    test_if_else_with_multi_return_function();
    test_multi_return_four_values();
    test_wah_call_multi_return_error();
    test_wah_call_multi_return_no_execution();
    test_multi_return_with_locals();

    printf("\n=== All Multi-Value Return Tests Passed ===\n");
    return 0;
}
