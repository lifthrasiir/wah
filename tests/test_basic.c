#include <stdio.h>
#include <stdlib.h>
#include "../wah.h"
#include "common.h"

int main() {
    wah_module_t module;
    wah_exec_context_t ctx;

    printf("Testing Valid Module (simple_add_wasm)...\n");
    const char *simple_add_wasm = "wasm \
        types {[ fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] local.get 0 local.get 1 i32.add end } ]}";
    assert_ok(wah_parse_module_from_spec(&module, simple_add_wasm));
    assert_ok(wah_new_exec_context(&ctx, &module, NULL));

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

    wah_free_exec_context(&ctx);
    wah_free_module(&module);

    printf("Testing wah_call_by_name...\n");
    {
        const char *spec = "wasm \
            types {[ fn [i32, i32] [i32] ]} \
            funcs {[ 0 ]} \
            memories {[limits.i32/1 1]} \
            exports {[ {'add'} fn# 0, {'mem'} mem# 0 ]} \
            code {[ {[] local.get 0 local.get 1 i32.add end } ]}";

        wah_module_t mod;
        assert_ok(wah_parse_module_from_spec(&mod, spec));

        wah_exec_context_t ectx;
        assert_ok(wah_new_exec_context(&ectx, &mod, NULL));

        wah_value_t p[2] = {{.i32 = 3}, {.i32 = 4}};
        wah_value_t r;
        assert_ok(wah_call_by_name(&ectx, "add", p, 2, &r));
        assert_eq_i32(r.i32, 7);

        // non-existent name
        assert_err(wah_call_by_name(&ectx, "nonexistent", p, 2, &r), WAH_ERROR_NOT_FOUND);

        // non-function export
        assert_err(wah_call_by_name(&ectx, "mem", p, 2, &r), WAH_ERROR_NOT_FOUND);

        wah_free_exec_context(&ectx);
        wah_free_module(&mod);
    }

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
