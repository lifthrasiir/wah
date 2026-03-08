#define WAH_IMPLEMENTATION
#include "wah.h"
#include "wah_testutils.c"
#include <stdio.h>
#include <assert.h>

// Test just a block with no branches: block { i32.const 42 } end
static const char *simple_block_wasm_spec = "wasm \
    types {[ fn [] [i32] ]} \
    funcs {[ 0 ]} \
    code {[ {[] block i32 \
        i32.const 42 \
    end end } ]}";

// Based on working simple_block_wasm, create a simple if
static const char *simple_if_wasm_spec = "wasm \
    types {[ fn [] [i32] ]} \
    funcs {[ 0 ]} \
    code {[ {[] i32.const 1 if i32 \
        i32.const 42 \
    else \
        i32.const 99 \
    end end } ]}";

// Test a simple if-else: if (param == 42) return 1; else return 0;
static const char *if_else_wasm_spec = "wasm \
    types {[ fn [i32] [i32] ]} \
    funcs {[ 0 ]} \
    code {[ {[] local.get 0 i32.const 42 i32.eq if i32 \
        i32.const 1 \
    else \
        i32.const 0 \
    end end } ]}";

// Test a simple loop: for(i=0; i<param; i++) sum+=i; return sum;
static const char *loop_wasm_spec = "wasm \
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

static const char *unreachable_i32_add_underflow_fail_wasm_spec = "wasm \
    types {[ fn [] [i32] ]} \
    funcs {[ 0 ]} \
    code {[ {[] unreachable i32.add end } ]}";

// This should pass validation because i32.add expects i32s, and i32.const pushes an i32.
static const char *unreachable_i32_i32_add_pass_wasm_spec = "wasm \
    types {[ fn [] [i32] ]} \
    funcs {[ 0 ]} \
    code {[ {[] unreachable i32.const 0 i32.add end } ]}";

// This should fail validation because 'br 0' branches to a block that expects an i32 result,
// but the stack is empty when 'br 0' is executed.
static const char *br_empty_stack_fail_wasm_spec = "wasm \
    types {[ fn [] [i32] ]} \
    funcs {[ 0 ]} \
    code {[ {[] block i32 \
        br 0 \
    end end } ]}";

// This should pass validation because 'br 0' branches to a block that expects an i32 result,
// and an i32 is on the stack when 'br 0' is executed.
static const char *br_correct_stack_pass_wasm_spec = "wasm \
    types {[ fn [] [i32] ]} \
    funcs {[ 0 ]} \
    code {[ {[] block i32 \
        i32.const 42 \
        br 0 \
    end end } ]}";

// This should fail validation because the function expects an i32 result,
// but an i64 is on the stack when 'return' is executed.
static const char *return_i64_fail_wasm_spec = "wasm \
    types {[ fn [] [i32] ]} \
    funcs {[ 0 ]} \
    code {[ {[] i64.const 0 return end } ]}";

// This should pass validation because the function expects an i32 result,
// and an i32 is on the stack when 'return' is executed.
static const char *return_i32_pass_wasm_spec = "wasm \
    types {[ fn [] [i32] ]} \
    funcs {[ 0 ]} \
    code {[ {[] i32.const 42 return end } ]}";

static void test_simple_block() {
    printf("Testing simple block...\n");

    wah_module_t module;
    wah_error_t err = wah_parse_module_from_spec(&module, simple_block_wasm_spec);
    assert(err == WAH_OK);
    printf("  - Simple block parsed successfully\n");

    wah_exec_context_t ctx;
    err = wah_exec_context_create(&ctx, &module);
    assert(err == WAH_OK);

    wah_value_t result;
    err = wah_call(&ctx, &module, 0, NULL, 0, &result);
    assert(err == WAH_OK);
    assert(result.i32 == 42);
    printf("  - Simple block executed, result: %d\n", result.i32);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_simple_if_const() {
    printf("Testing simple if (constant)...\n");

    wah_module_t module;
    wah_error_t err = wah_parse_module_from_spec(&module, simple_if_wasm_spec);
    assert(err == WAH_OK);
    printf("  - Simple if parsed successfully\n");

    wah_exec_context_t ctx;
    err = wah_exec_context_create(&ctx, &module);
    assert(err == WAH_OK);

    wah_value_t result;
    err = wah_call(&ctx, &module, 0, NULL, 0, &result);
    assert(err == WAH_OK);
    assert(result.i32 == 42);
    printf("  - Execution succeeded, result: %d\n", result.i32);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_if_else() {
    printf("Testing if-else (parameter)...\n");

    wah_module_t module;
    wah_error_t err = wah_parse_module_from_spec(&module, if_else_wasm_spec);
    assert(err == WAH_OK);

    wah_exec_context_t ctx;
    err = wah_exec_context_create(&ctx, &module);
    assert(err == WAH_OK);

    // Test if branch (param == 42)
    wah_value_t params[1] = {{.i32 = 42}};
    wah_value_t result;
    err = wah_call(&ctx, &module, 0, params, 1, &result);
    assert(err == WAH_OK);
    assert(result.i32 == 1);
    printf("  - If branch works (42 -> 1)\n");

    // Test else branch (param != 42)
    params[0].i32 = 99;
    err = wah_call(&ctx, &module, 0, params, 1, &result);
    assert(err == WAH_OK);
    assert(result.i32 == 0);
    printf("  - Else branch works (99 -> 0)\n");

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_loop() {
    printf("Testing loop control flow...\n");

    wah_module_t module;
    wah_error_t err = wah_parse_module_from_spec(&module, loop_wasm_spec);
    if (err != WAH_OK) {
        printf("  ERROR: Failed to parse loop module: %s\n", wah_strerror(err));
        assert(false);
        return;
    }

    wah_exec_context_t ctx;
    err = wah_exec_context_create(&ctx, &module);
    assert(err == WAH_OK);

    // Test loop: sum of 0..4 = 0+1+2+3 = 6
    wah_value_t params[1] = {{.i32 = 4}};
    wah_value_t result;
    err = wah_call(&ctx, &module, 0, params, 1, &result);
    if (err != WAH_OK) {
        printf("  ERROR: Loop function call failed: %s\n", wah_strerror(err));
        assert(false);
    } else if (result.i32 != 6) {
        printf("  ERROR: Expected 6, got %d\n", result.i32);
        assert(false);
    } else {
        printf("  - Loop works (sum 0..3 = 6)\n");
    }

    // Test empty loop
    params[0].i32 = 0;
    err = wah_call(&ctx, &module, 0, params, 1, &result);
    assert(err == WAH_OK);
    assert(result.i32 == 0);
    printf("  - Empty loop works (sum 0.. = 0)\n");

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// block $a end block $b br $a end (should fail validation)
static const char *br_to_outer_block_fail_wasm_spec = "wasm \
    types {[ fn [] [] ]} \
    funcs {[ 0 ]} \
    code {[ {[] \
        block void end \
        block void br 1 end \
    end } ]}";

// block $a end block $b br $b end (should pass validation)
static const char *br_to_current_block_pass_wasm_spec = "wasm \
    types {[ fn [] [] ]} \
    funcs {[ 0 ]} \
    code {[ {[] \
        block void end \
        block void br 0 end \
    end } ]}";

static const char *br_if_pass_wasm_spec = "wasm \
    types {[ fn [i32] [i32] ]} \
    funcs {[ 0 ]} \
    code {[ {[] block i32 \
        i32.const 10 \
        local.get 0 \
        br_if 0 \
        i32.const 20 \
    end end } ]}";

static const char *br_if_fail_wasm_spec = "wasm \
    types {[ fn [i64] [i32] ]} \
    funcs {[ 0 ]} \
    code {[ {[] block i32 \
        i32.const 10 \
        local.get 0 \
        br_if 0 \
        i32.const 20 \
    end end } ]}";

static void test_validation_unreachable_br_return() {
    printf("Testing validation for unreachable, br, and return...\n");
    wah_module_t module;
    wah_error_t err;

    // --- Failure Case 1: unreachable i32.add (stack underflow) ---
    err = wah_parse_module_from_spec(&module, unreachable_i32_add_underflow_fail_wasm_spec);
    assert(err == WAH_OK);
    printf("  - Unreachable i32.add (fail) - Validation passed as expected (unreachable context)\n");
    wah_free_module(&module); // Free module even if parsing failed, to clean up any allocated parts

    // --- Success Case 1: unreachable i32.const 0 i32.add (expects i32, got i32) ---
    err = wah_parse_module_from_spec(&module, unreachable_i32_i32_add_pass_wasm_spec);
    assert(err == WAH_OK);
    printf("  - Unreachable i32.const 0 i32.add (pass) - Validation passed as expected\n");
    wah_free_module(&module);

    // --- Failure Case 2: br 0 with empty stack for i32 result ---
    err = wah_parse_module_from_spec(&module, br_empty_stack_fail_wasm_spec);
    assert(err == WAH_ERROR_VALIDATION_FAILED);
    printf("  - br 0 with empty stack for i32 result (fail) - Validation failed as expected\n");
    wah_free_module(&module);

    // --- Success Case 2: br 0 with i32 on stack for i32 result ---
    err = wah_parse_module_from_spec(&module, br_correct_stack_pass_wasm_spec);
    assert(err == WAH_OK);
    printf("  - br 0 with i32 on stack for i32 result (pass) - Validation passed as expected\n");
    wah_free_module(&module);

    // --- Failure Case 3: return with i64 on stack for i32 result ---
    err = wah_parse_module_from_spec(&module, return_i64_fail_wasm_spec);
    assert(err == WAH_ERROR_VALIDATION_FAILED);
    printf("  - return with i64 on stack for i32 result (fail) - Validation failed as expected\n");
    wah_free_module(&module);

    // --- Success Case 3: return with i32 on stack for i32 result ---
    err = wah_parse_module_from_spec(&module, return_i32_pass_wasm_spec);
    assert(err == WAH_OK);
    printf("  - return with i32 on stack for i32 result (pass) - Validation passed as expected\n");
    wah_free_module(&module);

    // --- Failure Case 4: br to an outer block that is no longer on the control stack ---
    err = wah_parse_module_from_spec(&module, br_to_outer_block_fail_wasm_spec);
    assert(err == WAH_ERROR_VALIDATION_FAILED);
    printf("  - br to outer block (fail) - Validation failed as expected\n");
    wah_free_module(&module);

    // --- Success Case 4: br to current block ---
    err = wah_parse_module_from_spec(&module, br_to_current_block_pass_wasm_spec);
    assert(err == WAH_OK);
    printf("  - br to current block (pass) - Validation passed as expected\n");
    wah_free_module(&module);
}

static void test_br_if_validation() {
    printf("Testing br_if validation...\n");
    wah_module_t module;
    wah_error_t err;

    // --- Success Case: br_if with correct i32 condition and i32 result ---
    err = wah_parse_module_from_spec(&module, br_if_pass_wasm_spec);
    if (err != WAH_OK) {
        printf("ERROR: br_if_pass_wasm validation failed with error: %s (%d)\n", wah_strerror(err), err);
    }
    assert(err == WAH_OK);
    printf("  - br_if with i32 condition and i32 result (pass) - Validation passed as expected\n");
    wah_free_module(&module);

    // --- Failure Case: br_if with i64 condition (expects i32) ---
    err = wah_parse_module_from_spec(&module, br_if_fail_wasm_spec);
    assert(err == WAH_ERROR_VALIDATION_FAILED);
    printf("  - br_if with i64 condition (fail) - Validation failed as expected\n");
    wah_free_module(&module);
}

static void test_br_table() {
    printf("Testing br_table...\n");
    wah_module_t module;
    wah_error_t err = wah_parse_module_from_spec(&module, "wasm \
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
        end } ]}");
    if (err != WAH_OK) {
        printf("  ERROR: Failed to parse br_table module: %s\n", wah_strerror(err));
        assert(false);
    }
    assert(err == WAH_OK);
    printf("  - br_table module parsed successfully\n");

    wah_exec_context_t ctx;
    err = wah_exec_context_create(&ctx, &module);
    assert(err == WAH_OK);

    wah_value_t params[1];
    wah_value_t result;

    // Test case 0
    params[0].i32 = 0;
    err = wah_call(&ctx, &module, 0, params, 1, &result);
    assert(err == WAH_OK);
    assert(result.i32 == 10);
    printf("  - Case 0 -> 10 PASSED\n");

    // Test case 1
    params[0].i32 = 1;
    err = wah_call(&ctx, &module, 0, params, 1, &result);
    assert(err == WAH_OK);
    assert(result.i32 == 20);
    printf("  - Case 1 -> 20 PASSED\n");

    // Test case 2
    params[0].i32 = 2;
    err = wah_call(&ctx, &module, 0, params, 1, &result);
    assert(err == WAH_OK);
    assert(result.i32 == 30);
    printf("  - Case 2 -> 30 PASSED\n");

    // Test case 3 (default)
    params[0].i32 = 3;
    err = wah_call(&ctx, &module, 0, params, 1, &result);
    assert(err == WAH_OK);
    assert(result.i32 == 40);
    printf("  - Case 3 (default) -> 40 PASSED\n");

    // Test case 4 (default, out of bounds)
    params[0].i32 = 4;
    err = wah_call(&ctx, &module, 0, params, 1, &result);
    assert(err == WAH_OK);
    assert(result.i32 == 40);
    printf("  - Case 4 (default) -> 40 PASSED\n");

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// Test for br_table type consistency
static void test_br_table_type_consistency() {
    printf("Testing br_table type consistency...\n");

    // Valid case: all targets have the same result type (i32)
    wah_module_t module_valid;
    wah_error_t err_valid = wah_parse_module_from_spec(&module_valid, "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            block i32 \
                block i32 \
                    i32.const 0 \
                    i32.const 0 \
                    br_table [0, 1] 0 \
                end \
                i32.const 1 \
            end \
            i32.const 2 \
        end } ]}");
    assert(err_valid == WAH_OK);
    wah_free_module(&module_valid);

    // Invalid case: targets have different result types
    wah_module_t module_invalid;
    wah_error_t err_invalid = wah_parse_module_from_spec(&module_invalid, "wasm \
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
        end } ]}");
    assert(err_invalid == WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module_invalid); // Should still free even if parsing fails
}

static const char *block_type_with_params_pass_wasm_spec = "wasm \
    types {[ fn [i32] [i32] ]} \
    funcs {[ 0 ]} \
    code {[ {[] \
        local.get 0 \
        block 0 \
            i32.const 1 \
            i32.add \
        end \
    end } ]}";

static void test_block_type_with_params_pass() {
    printf("Testing block type with parameters (should pass validation and execute correctly)...\n");
    wah_module_t module;
    wah_error_t err = wah_parse_module_from_spec(&module, block_type_with_params_pass_wasm_spec);
    assert(err == WAH_OK);
    printf("  - Block type with parameters parsed successfully\n");
    wah_exec_context_t ctx;
    err = wah_exec_context_create(&ctx, &module);
    assert(err == WAH_OK);
    wah_value_t params[1] = {{.i32 = 10}};
    wah_value_t result;
    err = wah_call(&ctx, &module, 0, params, 1, &result);
    assert(err == WAH_OK);
    assert(result.i32 == 11);
    printf("  - Block type with parameters executed, result: %d\n", result.i32);
    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// Since unreachable's type can be assumed to be `() -> (i32, i32)`, this should pass the validation.
static const char *unreachable_drop_br_underflow_fail_wasm_spec = "wasm \
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

static void test_unreachable_drop_br_underflow_fail() {
    printf("Testing unreachable drop and br underflow...\n");
    wah_module_t module;
    wah_error_t err = wah_parse_module_from_spec(&module, unreachable_drop_br_underflow_fail_wasm_spec);
    assert(err == WAH_OK);
    printf("  - Unreachable drop and br underflow (pass) - Validation passed as expected\n");
    wah_free_module(&module);
}

// This should fail validation because the if expects an i32 on the stack, but unreachable leaves the stack in a bottom state.
// The current bug is that it fails validation, but it should pass.
static const char *unreachable_if_fail_wasm_spec = "wasm \
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

static void test_unreachable_if_validation() {
    printf("Testing unreachable followed by if validation...\n");
    wah_module_t module;
    wah_error_t err;

    err = wah_parse_module_from_spec(&module, unreachable_if_fail_wasm_spec);
    assert(err == WAH_OK);
    printf("  - Unreachable followed by if (pass) - Validation passed as expected\n");
    wah_free_module(&module);
}

// --- Multi-Value Control Flow Tests ---

// Test block with multiple results: (i32, i32) -> (i32, i32)
void test_block_multi_result() {
    printf("Testing block with multiple results...\n");

    wah_module_t module;
    wah_error_t err = wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [i32, i32] [i32, i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            block void \
                local.get 0 \
                local.get 1 \
            end \
        end } ]}");
    if (err == WAH_OK) {
        printf("[v] Multi-value block parsed successfully\n");
        wah_free_module(&module);
    } else {
        printf("[i] Multi-value block not yet supported: %s\n", wah_strerror(err));
    }
}

// Test if-else with multiple results
void test_if_else_multi_result() {
    printf("Testing if-else with multiple results...\n");

    wah_module_t module;
    wah_error_t err = wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [i32] [i32, i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            if void \
                local.get 0 \
                i32.const 1 \
                i32.const 2 \
            else \
                i32.const 3 \
                i32.const 4 \
            end \
        end } ]}");
    if (err == WAH_OK) {
        printf("[v] Multi-value if-else parsed successfully\n");
        wah_free_module(&module);
    } else {
        printf("[i] Multi-value if-else not yet supported: %s\n", wah_strerror(err));
    }
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

    printf("=== Control Flow Tests Complete ===\n");
    return 0;
}
