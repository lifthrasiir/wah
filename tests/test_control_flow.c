#define WAH_IMPLEMENTATION
#include "../wah.h"
#include "common.c"
#include <stdio.h>
#include <assert.h>

static void test_simple_block() {
    printf("Testing simple block...\n");

    // Test just a block with no branches: block { i32.const 42 } end
    const char *simple_block_wasm_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] block i32 \
            i32.const 42 \
        end end } ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, simple_block_wasm_spec));

    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_simple_if_const() {
    printf("Testing simple if (constant)...\n");

    // Based on working simple_block_wasm, create a simple if
    const char *simple_if_wasm_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] i32.const 1 if i32 \
            i32.const 42 \
        else \
            i32.const 99 \
        end end } ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, simple_if_wasm_spec));

    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_if_else() {
    printf("Testing if-else (parameter)...\n");

    // Test a simple if-else: if (param == 42) return 1; else return 0;
    const char *if_else_wasm_spec = "wasm \
        types {[ fn [i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] local.get 0 i32.const 42 i32.eq if i32 \
            i32.const 1 \
        else \
            i32.const 0 \
        end end } ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, if_else_wasm_spec));

    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module));

    // Test if branch (param == 42)
    wah_value_t params[1] = {{.i32 = 42}};
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 1);

    // Test else branch (param != 42)
    params[0].i32 = 99;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 0);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_loop() {
    printf("Testing loop control flow...\n");

    // Test a simple loop: for(i=0; i<param; i++) sum+=i; return sum;
    const char *loop_wasm_spec = "wasm \
        types {[ fn [i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[2 i32] \
            i32.const 0 local.set 1 \
            i32.const 0 local.set 2 \
            block void \
                loop void \
                    local.get 2 \
                    local.get 0 \
                    i32.ge_s \
                    br_if 1 \
                    local.get 1 \
                    local.get 2 \
                    i32.add \
                    local.set 1 \
                    local.get 2 \
                    i32.const 1 \
                    i32.add \
                    local.set 2 \
                    br 0 \
                end \
            end \
            local.get 1 \
        end } ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, loop_wasm_spec));

    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module));

    // Test loop: sum of 0..4 = 0+1+2+3 = 6
    wah_value_t params[1] = {{.i32 = 4}};
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 6);

    // Test empty loop
    params[0].i32 = 0;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 0);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_validation_unreachable_br_return() {
    printf("Testing validation for unreachable, br, and return...\n");
    wah_module_t module;

    // --- Failure Case 1: unreachable i32.add (stack underflow) ---
    const char *unreachable_i32_add_underflow_fail_wasm_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] unreachable i32.add end } ]}";
    assert_ok(wah_parse_module_from_spec(&module, unreachable_i32_add_underflow_fail_wasm_spec));
    wah_free_module(&module); // Free module even if parsing failed, to clean up any allocated parts

    // --- Success Case 1: unreachable i32.const 0 i32.add (expects i32, got i32) ---
    // This should pass validation because i32.add expects i32s, and i32.const pushes an i32.
    const char *unreachable_i32_i32_add_pass_wasm_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] unreachable i32.const 0 i32.add end } ]}";
    assert_ok(wah_parse_module_from_spec(&module, unreachable_i32_i32_add_pass_wasm_spec));
    wah_free_module(&module);

    // --- Failure Case 2: br 0 with empty stack for i32 result ---
    // This should fail validation because 'br 0' branches to a block that expects an i32 result,
    // but the stack is empty when 'br 0' is executed.
    const char *br_empty_stack_fail_wasm_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] block i32 \
            br 0 \
        end end } ]}";
    assert_err(wah_parse_module_from_spec(&module, br_empty_stack_fail_wasm_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);

    // --- Success Case 2: br 0 with i32 on stack for i32 result ---
    // This should pass validation because 'br 0' branches to a block that expects an i32 result,
    // and an i32 is on the stack when 'br 0' is executed.
    const char *br_correct_stack_pass_wasm_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] block i32 \
            i32.const 42 \
            br 0 \
        end end } ]}";
    assert_ok(wah_parse_module_from_spec(&module, br_correct_stack_pass_wasm_spec));
    wah_free_module(&module);

    // --- Failure Case 3: return with i64 on stack for i32 result ---
    // This should fail validation because the function expects an i32 result,
    // but an i64 is on the stack when 'return' is executed.
    const char *return_i64_fail_wasm_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] i64.const 0 return end } ]}";
    assert_err(wah_parse_module_from_spec(&module, return_i64_fail_wasm_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);

    // --- Success Case 3: return with i32 on stack for i32 result ---
    // This should pass validation because the function expects an i32 result,
    // and an i32 is on the stack when 'return' is executed.
    const char *return_i32_pass_wasm_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] i32.const 42 return end } ]}";
    assert_ok(wah_parse_module_from_spec(&module, return_i32_pass_wasm_spec));
    wah_free_module(&module);

    // --- Failure Case 4: br to a label index beyond all enclosing blocks and the function frame ---
    const char *br_to_outer_block_fail_wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            block void end \
            block void br 2 end \
        end } ]}";
    assert_err(wah_parse_module_from_spec(&module, br_to_outer_block_fail_wasm_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);

    // --- Success Case 4: br to current block ---
    const char *br_to_current_block_pass_wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            block void end \
            block void br 0 end \
        end } ]}";
    assert_ok(wah_parse_module_from_spec(&module, br_to_current_block_pass_wasm_spec));
    wah_free_module(&module);
}

static void test_br_if_validation() {
    printf("Testing br_if validation...\n");
    wah_module_t module;

    // --- Success Case: br_if with correct i32 condition and i32 result ---
    const char *br_if_pass_wasm_spec = "wasm \
        types {[ fn [i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] block i32 \
            i32.const 10 \
            local.get 0 \
            br_if 0 \
            drop \
            i32.const 20 \
        end end } ]}";
    assert_ok(wah_parse_module_from_spec(&module, br_if_pass_wasm_spec));
    wah_free_module(&module);

    // --- Failure Case: br_if with i64 condition (expects i32) ---
    const char *br_if_fail_wasm_spec = "wasm \
        types {[ fn [i64] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] block i32 \
            i32.const 10 \
            local.get 0 \
            br_if 0 \
            i32.const 20 \
        end end } ]}";
    assert_err(wah_parse_module_from_spec(&module, br_if_fail_wasm_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);
}

static void test_br_table() {
    printf("Testing br_table...\n");
    wah_module_t module;

    const char *br_table_wasm_spec = "wasm \
        types {[ fn [i32] [i32] ]} \
        funcs {[ 0 ]} \
        exports {[ {'br_table_func'} fn# 0 ]} \
        code {[ {[] \
            block void \
                block void \
                    block void \
                        block void \
                            local.get 0 \
                            br_table [0, 1, 2] 3 \
                        end \
                        i32.const 10 return \
                    end \
                    i32.const 20 return \
                end \
                i32.const 30 return \
            end \
            i32.const 40 \
        end } ]}";

    assert_ok(wah_parse_module_from_spec(&module, br_table_wasm_spec));

    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module));

    wah_value_t params[1];
    wah_value_t result;

    // Test case 0
    params[0].i32 = 0;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 10);

    // Test case 1
    params[0].i32 = 1;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 20);

    // Test case 2
    params[0].i32 = 2;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 30);

    // Test case 3 (default)
    params[0].i32 = 3;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 40);

    // Test case 4 (default, out of bounds)
    params[0].i32 = 4;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 40);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// Test for br_table type consistency
static void test_br_table_type_consistency() {
    printf("Testing br_table type consistency...\n");

    // Valid case: all targets have the same result type (i32)
    const char *br_table_valid_wasm_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            block i32 \
                block i32 \
                    i32.const 0 \
                    i32.const 0 \
                    br_table [0, 1] 0 \
                end \
            end \
        end } ]}";
    wah_module_t module_valid;
    assert_ok(wah_parse_module_from_spec(&module_valid, br_table_valid_wasm_spec));
    wah_free_module(&module_valid);

    // Invalid case: targets have different result types
    const char *br_table_invalid_wasm_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            block i32 \
                block i64 \
                    i32.const 0 \
                    br_table [0, 1] 0 \
                end \
                i32.const 1 \
            end \
            i32.const 2 \
        end } ]}";
    wah_module_t module_invalid;
    assert_err(wah_parse_module_from_spec(&module_invalid, br_table_invalid_wasm_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module_invalid); // Should still free even if parsing fails
}

static void test_block_type_with_params_pass() {
    printf("Testing block type with parameters (should pass validation and execute correctly)...\n");
    const char *block_type_with_params_pass_wasm_spec = "wasm \
        types {[ fn [i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            local.get 0 \
            block 0 \
                i32.const 1 \
                i32.add \
            end \
        end } ]}";
    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, block_type_with_params_pass_wasm_spec));
    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module));
    wah_value_t params[1] = {{.i32 = 10}};
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 11);
    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_unreachable_drop_br_underflow_fail() {
    printf("Testing unreachable drop and br underflow...\n");

    // Since unreachable's type can be assumed to be `() -> (i32, i32)`, this should pass the validation.
    const char *unreachable_drop_br_underflow_fail_wasm_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            block i32 \
                i32.const 0 \
                unreachable \
                drop \
                br 0 \
            end \
        end } ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, unreachable_drop_br_underflow_fail_wasm_spec));
    wah_free_module(&module);
}

static void test_unreachable_if_validation() {
    printf("Testing unreachable followed by if validation...\n");
    wah_module_t module;

    // This should fail validation because the if expects an i32 on the stack, but unreachable leaves the stack in a bottom state.
    // The current bug is that it fails validation, but it should pass.
    const char *unreachable_if_fail_wasm_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        exports {[ {'test'} fn# 0 ]} \
        code {[ {[] \
            unreachable \
            if i32 \
                i32.const 1 \
            else \
                i32.const 0 \
            end \
        end } ]}";

    assert_ok(wah_parse_module_from_spec(&module, unreachable_if_fail_wasm_spec));
    wah_free_module(&module);
}

// --- Multi-Value Control Flow Tests ---

// Test block with multiple results using a type-indexed block type ([] -> [i32, i32])
void test_block_multi_result() {
    printf("Testing block with multiple results...\n");

    wah_module_t module;
    // type 0: function [i32,i32]->[i32,i32], type 1: block []->[i32,i32]
    assert_ok(wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [i32, i32] [i32, i32], fn [] [i32, i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            block 1 \
                local.get 0 \
                local.get 1 \
            end \
        end } ]}"));

    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module));

    wah_value_t params[2] = {{.i32 = 7}, {.i32 = 13}};
    wah_value_t results[2];
    uint32_t cnt;
    assert_ok(wah_call_multi(&ctx, 0, params, 2, results, 2, &cnt));
    assert_eq_u32(cnt, 2);
    assert_eq_i32(results[0].i32, 7);
    assert_eq_i32(results[1].i32, 13);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// Test if-else with multiple results using a type-indexed block type ([] -> [i32, i32])
void test_if_else_multi_result() {
    printf("Testing if-else with multiple results...\n");

    wah_module_t module;
    // type 0: function [i32]->[i32,i32], type 1: block []->[i32,i32]
    assert_ok(wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [i32] [i32, i32], fn [] [i32, i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            local.get 0 \
            if 1 \
                i32.const 1 \
                i32.const 2 \
            else \
                i32.const 3 \
                i32.const 4 \
            end \
        end } ]}"));

    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module));

    wah_value_t param;
    wah_value_t results[2];
    uint32_t cnt;

    param.i32 = 1;
    assert_ok(wah_call_multi(&ctx, 0, &param, 1, results, 2, &cnt));
    assert_eq_u32(cnt, 2);
    assert_eq_i32(results[0].i32, 1);
    assert_eq_i32(results[1].i32, 2);

    param.i32 = 0;
    assert_ok(wah_call_multi(&ctx, 0, &param, 1, results, 2, &cnt));
    assert_eq_u32(cnt, 2);
    assert_eq_i32(results[0].i32, 3);
    assert_eq_i32(results[1].i32, 4);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// Block type validation error tests
static void test_block_type_validation_errors() {
    printf("Testing block type validation errors...\n");
    wah_module_t module;

    // block i32 but i64 is on the stack at end
    assert_err(wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [] [i32] ]} funcs {[ 0 ]} \
        code {[ {[] block i32 i64.const 0 end end } ]}"),
        WAH_ERROR_VALIDATION_FAILED);

    // block i32 but nothing on the stack at end
    assert_err(wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [] [i32] ]} funcs {[ 0 ]} \
        code {[ {[] block i32 end end } ]}"),
        WAH_ERROR_VALIDATION_FAILED);

    // block with type index: params type mismatch (i64 on stack, block expects i32)
    assert_err(wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [] [i32], fn [i32] [i32] ]} funcs {[ 0 ]} \
        code {[ {[] i64.const 0 block 1 end end } ]}"),
        WAH_ERROR_VALIDATION_FAILED);

    // block with out-of-range type index
    assert_err(wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [] [] ]} funcs {[ 0 ]} \
        code {[ {[] block 99 end end } ]}"),
        WAH_ERROR_VALIDATION_FAILED);

    // if: true branch returns i32, else branch returns i64
    assert_err(wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [] [i32] ]} funcs {[ 0 ]} \
        code {[ {[] i32.const 0 if i32 i32.const 1 else i64.const 2 end end } ]}"),
        WAH_ERROR_VALIDATION_FAILED);

    // if with result but no else branch (if without else must have void type)
    assert_err(wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [] [i32] ]} funcs {[ 0 ]} \
        code {[ {[] i32.const 0 if i32 i32.const 1 end end } ]}"),
        WAH_ERROR_VALIDATION_FAILED);
}

// This tests that an ELSE without a matching IF fails validation.
// This ensures the assertion "validation should have verified ELSE is inside IF" holds.
static void test_else_without_if() {
    printf("Testing ELSE without matching IF (should fail validation)...\n");
    wah_module_t module;

    const char *else_without_if_wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        code {[ {[] else end } ]}";

    // Case 1: ELSE at the start of a function (no IF)
    assert_err(wah_parse_module_from_spec(&module, else_without_if_wasm_spec), WAH_ERROR_VALIDATION_FAILED);
}

// This tests that BR with an invalid relative depth fails validation.
// This ensures the assertion "validation should have verified relative depth" holds.
static void test_br_invalid_relative_depth() {
    printf("Testing BR with invalid relative depth (should fail validation)...\n");
    wah_module_t module;

    const char *br_invalid_relative_depth_wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        code {[ {[] block void end br 1 end } ]}";

    // Case 1: br 1 when only one block is on the stack
    assert_err(wah_parse_module_from_spec(&module, br_invalid_relative_depth_wasm_spec), WAH_ERROR_VALIDATION_FAILED);
}

int main() {
    printf("=== Control Flow Tests ===\n");
    test_simple_block();
    test_simple_if_const();
    test_if_else();
    test_loop();
    test_validation_unreachable_br_return();
    test_br_if_validation();
    test_br_table();
    test_br_table_type_consistency();
    test_block_type_with_params_pass();
    test_unreachable_drop_br_underflow_fail();
    test_unreachable_if_validation();

    printf("\n=== Multi-Value Control Flow Tests ===\n");
    test_block_multi_result();
    test_if_else_multi_result();

    printf("\n=== Block Type Validation Error Tests ===\n");
    test_block_type_validation_errors();

    printf("\n=== Validation Assertion Tests ===\n");
    test_else_without_if();
    test_br_invalid_relative_depth();

    printf("\n=== Control Flow Tests Complete ===\n");
    return 0;
}
