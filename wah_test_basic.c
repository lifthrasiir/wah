#define WAH_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include "wah.h"
#include "wah_testutils.c"

int main() {
    wah_module_t module;
    wah_exec_context_t ctx;

    printf("Testing Valid Module (simple_add_wasm)...\n");
    const char *simple_add_wasm = "wasm \
        types {[ fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] local.get 0 local.get 1 i32.add end } ]}";
    assert_ok(wah_parse_module_from_spec(&module, simple_add_wasm));
    assert_ok(wah_exec_context_create(&ctx, &module));

    printf("  Function max stack depth: %u\n", module.code_bodies[0].max_stack_depth);

    uint32_t func_idx = 0;
    wah_value_t params[2];
    wah_value_t result;

    params[0].i32 = 10;
    params[1].i32 = 20;

    printf("  Interpreting function %u with params %d and %d...\n", func_idx, params[0].i32, params[1].i32);
    assert_ok(wah_call(&ctx, func_idx, params, 2, &result));
    assert_eq_i32(result.i32, 30);

    params[0].i32 = 5;
    params[1].i32 = 7;
    printf("  Interpreting function %u with params %d and %d...\n", func_idx, params[0].i32, params[1].i32);
    assert_ok(wah_call(&ctx, func_idx, params, 2, &result));
    assert_eq_i32(result.i32, 12);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);

    printf("Testing Invalid Module (invalid_local_get_wasm)...\n");
    const char *invalid_local_get_wasm = "wasm \
        types {[ fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] local.get 2 local.get 1 i32.add end } ]}";
    assert_err(wah_parse_module_from_spec(&module, invalid_local_get_wasm), WAH_ERROR_VALIDATION_FAILED);

    printf("Testing Stack Underflow Module (stack_underflow_wasm)...\n");
    const char *stack_underflow_wasm = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] i32.add end } ]}";
    assert_err(wah_parse_module_from_spec(&module, stack_underflow_wasm), WAH_ERROR_VALIDATION_FAILED);

    return 0;
}
