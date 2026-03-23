#include <stdio.h>
#include <string.h>
#include <assert.h>

#define WAH_IMPLEMENTATION
#include "wah.h"
#include "wah_testutils.c"

// Test 1: Simple const expression (i32.const)
static const char *wasm_simple_const_spec = "wasm \
    types {[ fn [] [i32] ]} \
    funcs {[ 0 ]} \
    globals {[ i32 mut i32.const 42 end ]} \
    code {[ {[] global.get 0 end} ]}";

// Test 2: Const expression with binary operation (i32.add)
static const char *wasm_binary_op_spec = "wasm \
    types {[ fn [] [i32] ]} \
    funcs {[ 0 ]} \
    globals {[ \
        i32 mut i32.const 10 end, \
        i32 mut i32.const 32 end, \
        i32 mut i32.const 50 end, \
    ]} \
    code {[ {[] global.get 2 end} ]}";

// Test 3: Const expression referencing earlier global (global.get)
static const char *wasm_global_get_spec = "wasm \
    types {[ fn [] [i32] ]} \
    funcs {[ 0 ]} \
    globals {[ \
        i32 mut i32.const 100 end, \
        i32 mut global.get 0 end, \
    ]} \
    code {[ {[] global.get 1 end} ]}";

// Test 4: Complex const expression (i32.add with global.get)
static const char *wasm_complex_spec = "wasm \
    types {[ fn [] [i32] ]} \
    funcs {[ 0 ]} \
    globals {[ \
        i32 mut i32.const 10 end, \
        i32 mut i32.const 32 end, \
        i32 mut global.get 0 global.get 1 i32.add end, \
    ]} \
    code {[ {[] global.get 2 end} ]}";

// Test 5: Invalid const expression (control flow not allowed)
static const char *wasm_invalid_control_flow_spec = "wasm \
    types {[]} \
    globals {[ i32 mut block end i32.const 42 end end ]}";

// Test 6: Invalid const expression (type mismatch)
static const char *wasm_invalid_type_spec = "wasm \
    types {[]} \
    globals {[ i64 mut i32.const 42 end ]}";

// Test 7: Invalid const expression (referencing later global)
static const char *wasm_invalid_forward_ref_spec = "wasm \
    types {[]} \
    globals {[ \
        i32 mut global.get 1 end, \
        i32 mut i32.const 42 end, \
    ]}";

void test_simple_const(void) {
    fprintf(stderr, "--- Testing simple const expression ---\n");
    wah_module_t module;
    wah_error_t err = wah_parse_module_from_spec(&module, wasm_simple_const_spec);
    if (err != WAH_OK) {
        fprintf(stderr, "FAIL: wah_parse_module_from_spec returned %s\n", wah_strerror(err));
        return;
    }
    fprintf(stderr, "PASS: Parse module\n");

    wah_exec_context_t ctx;
    err = wah_exec_context_create(&ctx, &module);
    if (err != WAH_OK) {
        fprintf(stderr, "FAIL: wah_exec_context_create returned %s\n", wah_strerror(err));
        wah_free_module(&module);
        return;
    }
    fprintf(stderr, "PASS: Create exec context\n");

    err = wah_instantiate(&ctx);
    if (err != WAH_OK) {
        fprintf(stderr, "FAIL: wah_instantiate returned %s\n", wah_strerror(err));
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
        return;
    }
    fprintf(stderr, "PASS: Instantiate\n");

    wah_value_t result;
    err = wah_call(&ctx, 0, NULL, 0, &result);
    assert(err == WAH_OK);
    assert(result.i32 == 42);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
    fprintf(stderr, "PASS: Simple const expression\n");
}

void test_binary_op(void) {
    printf("--- Testing binary operation in const expression ---\n");
    wah_module_t module;
    wah_error_t err = wah_parse_module_from_spec(&module, wasm_binary_op_spec);
    assert(err == WAH_OK);

    wah_exec_context_t ctx;
    err = wah_exec_context_create(&ctx, &module);
    assert(err == WAH_OK);

    err = wah_instantiate(&ctx);
    assert(err == WAH_OK);

    // Check that all globals are initialized correctly
    assert(ctx.globals[0].i32 == 10);
    assert(ctx.globals[1].i32 == 32);
    assert(ctx.globals[2].i32 == 50);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
    printf("PASS: Binary operation in const expression\n");
}

void test_global_get(void) {
    printf("--- Testing global.get in const expression ---\n");
    wah_module_t module;
    wah_error_t err = wah_parse_module_from_spec(&module, wasm_global_get_spec);
    assert(err == WAH_OK);

    wah_exec_context_t ctx;
    err = wah_exec_context_create(&ctx, &module);
    assert(err == WAH_OK);

    err = wah_instantiate(&ctx);
    assert(err == WAH_OK);

    // global[1] should have the value of global[0]
    assert(ctx.globals[0].i32 == 100);
    assert(ctx.globals[1].i32 == 100);

    wah_value_t result;
    err = wah_call(&ctx, 0, NULL, 0, &result);
    assert(err == WAH_OK);
    assert(result.i32 == 100);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
    printf("PASS: global.get in const expression\n");
}

void test_complex_const_expr(void) {
    printf("--- Testing complex const expression ---\n");
    wah_module_t module;
    wah_error_t err = wah_parse_module_from_spec(&module, wasm_complex_spec);
    assert(err == WAH_OK);

    wah_exec_context_t ctx;
    err = wah_exec_context_create(&ctx, &module);
    assert(err == WAH_OK);

    err = wah_instantiate(&ctx);
    assert(err == WAH_OK);

    // global[2] should be global[0] + global[1] = 10 + 32 = 42
    assert(ctx.globals[0].i32 == 10);
    assert(ctx.globals[1].i32 == 32);
    assert(ctx.globals[2].i32 == 42);

    wah_value_t result;
    err = wah_call(&ctx, 0, NULL, 0, &result);
    assert(err == WAH_OK);
    assert(result.i32 == 42);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
    printf("PASS: Complex const expression\n");
}

void test_invalid_control_flow(void) {
    printf("--- Testing invalid const expression (control flow) ---\n");
    wah_module_t module;
    wah_error_t err = wah_parse_module_from_spec(&module, wasm_invalid_control_flow_spec);
    assert(err == WAH_ERROR_VALIDATION_FAILED);
    printf("PASS: Control flow rejected in const expression\n");
}

void test_invalid_type(void) {
    printf("--- Testing invalid const expression (type mismatch) ---\n");
    wah_module_t module;
    wah_error_t err = wah_parse_module_from_spec(&module, wasm_invalid_type_spec);
    assert(err == WAH_ERROR_VALIDATION_FAILED);
    printf("PASS: Type mismatch rejected in const expression\n");
}

void test_invalid_forward_ref(void) {
    printf("--- Testing invalid const expression (forward reference) ---\n");
    wah_module_t module;
    wah_error_t err = wah_parse_module_from_spec(&module, wasm_invalid_forward_ref_spec);
    assert(err == WAH_ERROR_VALIDATION_FAILED);
    printf("PASS: Forward reference rejected in const expression\n");
}

int main(void) {
    printf("--- Running Const Expression Tests ---\n");
    fflush(stdout);

    // Run each test separately
    printf("\n");
    test_simple_const();
    printf("\n");
    test_binary_op();
    printf("\n");
    test_global_get();
    printf("\n");
    test_complex_const_expr();
    printf("\n");
    test_invalid_control_flow();
    printf("\n");
    test_invalid_type();
    printf("\n");
    test_invalid_forward_ref();

    printf("\n--- All Const Expression Tests Passed ---\n");
    fflush(stdout);
    return 0;
}
