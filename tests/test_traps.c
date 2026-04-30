#include "../wah.h"
#include <stdio.h>
#include <stdint.h>
#include "common.h"

int main() {
    wah_module_t module;
    wah_exec_context_t ctx;
    wah_value_t params[2], result;

    printf("=== Testing Division by Zero Traps ===\n");

    // Test signed division by zero
    printf("1. Testing i32.div_s with division by zero:\n");
    const char *div_by_zero_spec = "wasm \
        types {[ fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] local.get 0 local.get 1 i32.div_s end } ]}";
    assert_ok(wah_parse_module_from_spec(&module, div_by_zero_spec));
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
    params[0].i32 = 42;
    params[1].i32 = 0; // Division by zero
    assert_err(wah_call(&ctx, 0, params, 2, &result), WAH_ERROR_TRAP);
    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);

    // Test signed integer overflow
    printf("2. Testing i32.div_s with signed integer overflow:\n");
    const char *signed_overflow_spec = "wasm \
        types {[ fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] local.get 0 local.get 1 i32.div_s end } ]}";
    assert_ok(wah_parse_module_from_spec(&module, signed_overflow_spec));
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
    params[0].i32 = INT32_MIN;
    params[1].i32 = -1; // This causes overflow: INT_MIN / -1 = +2^31 (unrepresentable)
    assert_err(wah_call(&ctx, 0, params, 2, &result), WAH_ERROR_TRAP);
    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);

    // Test unsigned division by zero
    printf("3. Testing i32.div_u with division by zero:\n");
    const char *div_u_by_zero_spec = "wasm \
        types {[ fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] local.get 0 local.get 1 i32.div_u end } ]}";
    assert_ok(wah_parse_module_from_spec(&module, div_u_by_zero_spec));
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
    params[0].i32 = 100;
    params[1].i32 = 0; // Division by zero
    assert_err(wah_call(&ctx, 0, params, 2, &result), WAH_ERROR_TRAP);
    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);

    // Test remainder by zero
    printf("4. Testing i32.rem_s with division by zero:\n");
    const char *rem_by_zero_spec = "wasm \
        types {[ fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] local.get 0 local.get 1 i32.rem_s end } ]}";
    assert_ok(wah_parse_module_from_spec(&module, rem_by_zero_spec));
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
    params[0].i32 = 7;
    params[1].i32 = 0; // Division by zero
    assert_err(wah_call(&ctx, 0, params, 2, &result), WAH_ERROR_TRAP);
    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);

    // Test that valid operations still work
    printf("5. Testing that valid operations still work:\n");
    const char *valid_div_spec = "wasm \
        types {[ fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] local.get 0 local.get 1 i32.div_s end } ]}";
    assert_ok(wah_parse_module_from_spec(&module, valid_div_spec));
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
    params[0].i32 = 20;
    params[1].i32 = 4; // Valid division: 20 / 4 = 5
    assert_ok(wah_call(&ctx, 0, params, 2, &result));
    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);

    // Test that INT_MIN % -1 = 0 (doesn't trap per spec)
    printf("6. Testing i32.rem_s with INT_MIN %% -1 (should return 0, not trap):\n");
    const char *int_min_rem_spec = "wasm \
        types {[ fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] local.get 0 local.get 1 i32.rem_s end } ]}";
    assert_ok(wah_parse_module_from_spec(&module, int_min_rem_spec));
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
    params[0].i32 = INT32_MIN;
    params[1].i32 = -1; // This should return 0, not trap
    assert_ok(wah_call(&ctx, 0, params, 2, &result));
    assert_eq_i32(result.i32, 0);
    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);

    // 296dd8d: Trap correctly recovers the stack pointer.
    printf("7. Testing trap recovery (296dd8d):\n");
    {
        const char *trap_spec = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0 ]} \
            code {[ {[] unreachable end } ]}";
        const char *ok_spec = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0 ]} \
            code {[ {[] i32.const 42 end } ]}";

        assert_ok(wah_parse_module_from_spec(&module, trap_spec));
        assert_ok(wah_exec_context_create(&ctx, &module, NULL));

        // First call traps
        assert_err(wah_call(&ctx, 0, NULL, 0, &result), WAH_ERROR_TRAP);

        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);

        // Second call on fresh context must succeed without stale stack
        assert_ok(wah_parse_module_from_spec(&module, ok_spec));
        assert_ok(wah_exec_context_create(&ctx, &module, NULL));

        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
        assert_eq_i32(result.i32, 42);

        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    // Also test: trap then re-call on SAME context
    printf("8. Testing trap then re-call on same context (296dd8d):\n");
    {
        // Two functions: func 0 traps (div by zero), func 1 returns 99
        const char *spec = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0, 0 ]} \
            code {[ \
                {[] i32.const 1 i32.const 0 i32.div_s end }, \
                {[] i32.const 99 end } \
            ]}";

        assert_ok(wah_parse_module_from_spec(&module, spec));
        assert_ok(wah_exec_context_create(&ctx, &module, NULL));

        // Func 0 traps
        assert_err(wah_call(&ctx, 0, NULL, 0, &result), WAH_ERROR_TRAP);

        // Func 1 must still work - no stale operand stack
        assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
        assert_eq_i32(result.i32, 99);

        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("\n=== All trap tests passed! ===\n");
    return 0;
}
