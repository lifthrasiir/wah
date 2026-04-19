#include <stdio.h>
#include <string.h>
#include <assert.h>

#define WAH_IMPLEMENTATION
#include "../wah.h"
#include "common.c"

// Test 1: Simple const expression (i32.const)
void test_simple_const(void) {
    fprintf(stderr, "Testing simple const expression...\n");
    wah_module_t module;

    const char *wasm_simple_const_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        globals {[ i32 mut i32.const 42 end ]} \
        code {[ {[] global.get 0 end} ]}";
    assert_ok(wah_parse_module_from_spec(&module, wasm_simple_const_spec));

    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module));

    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// Test 2: Const expression with binary operation (i32.add)
void test_binary_op(void) {
    printf("Testing binary operation in const expression...\n");
    wah_module_t module;

    const char *wasm_binary_op_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        globals {[ \
            i32 mut i32.const 10 end, \
            i32 mut i32.const 32 end, \
            i32 mut i32.const 50 end, \
        ]} \
        code {[ {[] global.get 2 end} ]}";
    assert_ok(wah_parse_module_from_spec(&module, wasm_binary_op_spec));

    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module));

    assert_ok(wah_instantiate(&ctx));

    // Check that all globals are initialized correctly
    assert_eq_i32(ctx.globals[0].i32, 10);
    assert_eq_i32(ctx.globals[1].i32, 32);
    assert_eq_i32(ctx.globals[2].i32, 50);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// Test 3: Const expression referencing earlier global (global.get)
void test_global_get(void) {
    printf("Testing global.get in const expression...\n");
    wah_module_t module;

    const char *wasm_global_get_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        globals {[ \
            i32 mut i32.const 100 end, \
            i32 mut global.get 0 end, \
        ]} \
        code {[ {[] global.get 1 end} ]}";
    assert_ok(wah_parse_module_from_spec(&module, wasm_global_get_spec));

    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module));

    assert_ok(wah_instantiate(&ctx));

    // global[1] should have the value of global[0]
    assert_eq_i32(ctx.globals[0].i32, 100);
    assert_eq_i32(ctx.globals[1].i32, 100);

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 100);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// Test 4: Complex const expression (i32.add with global.get)
void test_complex_const_expr(void) {
    printf("Testing complex const expression...\n");
    wah_module_t module;

    const char *wasm_complex_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        globals {[ \
            i32 mut i32.const 10 end, \
            i32 mut i32.const 32 end, \
            i32 mut global.get 0 global.get 1 i32.add end, \
        ]} \
        code {[ {[] global.get 2 end} ]}";
    assert_ok(wah_parse_module_from_spec(&module, wasm_complex_spec));

    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module));

    assert_ok(wah_instantiate(&ctx));

    // global[2] should be global[0] + global[1] = 10 + 32 = 42
    assert_eq_i32(ctx.globals[0].i32, 10);
    assert_eq_i32(ctx.globals[1].i32, 32);
    assert_eq_i32(ctx.globals[2].i32, 42);

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// Test 5: Invalid const expression (control flow not allowed)
void test_invalid_control_flow(void) {
    printf("Testing invalid const expression (control flow)...\n");
    wah_module_t module;
    const char *wasm_invalid_control_flow_spec = "wasm \
        types {[]} \
        globals {[ i32 mut block end i32.const 42 end end ]}";
    assert_err(wah_parse_module_from_spec(&module, wasm_invalid_control_flow_spec), WAH_ERROR_VALIDATION_FAILED);
}

// Test 6: Invalid const expression (type mismatch)
void test_invalid_type(void) {
    printf("Testing invalid const expression (type mismatch)...\n");
    wah_module_t module;
    const char *wasm_invalid_type_spec = "wasm \
        types {[]} \
        globals {[ i64 mut i32.const 42 end ]}";
    assert_err(wah_parse_module_from_spec(&module, wasm_invalid_type_spec), WAH_ERROR_VALIDATION_FAILED);
}

// Test 7: Invalid const expression (referencing later global)
void test_invalid_forward_ref(void) {
    printf("Testing invalid const expression (forward reference)...\n");
    wah_module_t module;
    const char *wasm_invalid_forward_ref_spec = "wasm \
        types {[]} \
        globals {[ \
            i32 mut global.get 1 end, \
            i32 mut i32.const 42 end, \
        ]}";
    assert_err(wah_parse_module_from_spec(&module, wasm_invalid_forward_ref_spec), WAH_ERROR_VALIDATION_FAILED);
}

int main(void) {
    // Run each test separately
    test_simple_const();
    test_binary_op();
    test_global_get();
    test_complex_const_expr();
    test_invalid_control_flow();
    test_invalid_type();
    test_invalid_forward_ref();

    printf("\n--- All Const Expression Tests Passed ---\n");
    fflush(stdout);
    return 0;
}
