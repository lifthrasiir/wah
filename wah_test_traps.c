#define WAH_IMPLEMENTATION
#include "wah.h"
#include <stdio.h>
#include <stdint.h>
#include "wah_testutils.c"

int main() {
    wah_module_t module;
    wah_exec_context_t ctx;
    wah_error_t err;
    wah_value_t params[2], result;

    printf("=== Testing Division by Zero Traps ===\n");

    // Test signed division by zero
    printf("\n1. Testing i32.div_s with division by zero:\n");
    err = wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] local.get 0 local.get 1 i32.div_s end } ]}");
    if (err != WAH_OK) {
        printf("Failed to parse div_by_zero module: %s\n", wah_strerror(err));
        return 1;
    }
    // Create context
    err = wah_exec_context_create(&ctx, &module);
    if (err != WAH_OK) {
        fprintf(stderr, "Error creating execution context for div_by_zero test: %s\n", wah_strerror(err));
        wah_free_module(&module);
        return 1;
    }

    params[0].i32 = 42;
    params[1].i32 = 0; // Division by zero
    err = wah_call(&ctx, &module, 0, params, 2, &result);
    if (err == WAH_ERROR_TRAP) {
        printf("- Correctly trapped on division by zero (%s)\n", wah_strerror(err));
    } else {
        printf("x Expected trap but got error: %s\n", wah_strerror(err));
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
        return 1;
    }
    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);

    // Test signed integer overflow
    printf("\n2. Testing i32.div_s with signed integer overflow:\n");
    err = wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] local.get 0 local.get 1 i32.div_s end } ]}");
    if (err != WAH_OK) {
        printf("Failed to parse signed_overflow module: %s\n", wah_strerror(err));
        return 1;
    }
    // Create context
    err = wah_exec_context_create(&ctx, &module);
    if (err != WAH_OK) {
        fprintf(stderr, "Error creating execution context for signed_overflow test: %s\n", wah_strerror(err));
        wah_free_module(&module);
        return 1;
    }

    params[0].i32 = INT32_MIN;
    params[1].i32 = -1; // This causes overflow: INT_MIN / -1 = +2^31 (unrepresentable)
    err = wah_call(&ctx, &module, 0, params, 2, &result);
    if (err == WAH_ERROR_TRAP) {
        printf("- Correctly trapped on signed integer overflow (%s)\n", wah_strerror(err));
    } else {
        printf("x Expected trap but got error: %s\n", wah_strerror(err));
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
        return 1;
    }
    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);

    // Test unsigned division by zero
    printf("\n3. Testing i32.div_u with division by zero:\n");
    err = wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] local.get 0 local.get 1 i32.div_u end } ]}");
    if (err != WAH_OK) {
        printf("Failed to parse div_u_by_zero module: %s\n", wah_strerror(err));
        return 1;
    }
    // Create context
    err = wah_exec_context_create(&ctx, &module);
    if (err != WAH_OK) {
        fprintf(stderr, "Error creating execution context for div_u_by_zero test: %s\n", wah_strerror(err));
        wah_free_module(&module);
        return 1;
    }

    params[0].i32 = 100;
    params[1].i32 = 0; // Division by zero
    err = wah_call(&ctx, &module, 0, params, 2, &result);
    if (err == WAH_ERROR_TRAP) {
        printf("- Correctly trapped on unsigned division by zero (%s)\n", wah_strerror(err));
    } else {
        printf("x Expected trap but got error: %s\n", wah_strerror(err));
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
        return 1;
    }
    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);

    // Test remainder by zero
    printf("\n4. Testing i32.rem_s with division by zero:\n");
    err = wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] local.get 0 local.get 1 i32.rem_s end } ]}");
    if (err != WAH_OK) {
        printf("Failed to parse rem_by_zero module: %s\n", wah_strerror(err));
        return 1;
    }
    // Create context
    err = wah_exec_context_create(&ctx, &module);
    if (err != WAH_OK) {
        fprintf(stderr, "Error creating execution context for rem_by_zero test: %s\n", wah_strerror(err));
        wah_free_module(&module);
        return 1;
    }

    params[0].i32 = 7;
    params[1].i32 = 0; // Division by zero
    err = wah_call(&ctx, &module, 0, params, 2, &result);
    if (err == WAH_ERROR_TRAP) {
        printf("- Correctly trapped on remainder by zero (%s)\n", wah_strerror(err));
    } else {
        printf("x Expected trap but got error: %s\n", wah_strerror(err));
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
        return 1;
    }
    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);

    // Test that valid operations still work
    printf("\n5. Testing that valid operations still work:\n");
    err = wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] local.get 0 local.get 1 i32.div_s end } ]}");
    if (err != WAH_OK) {
        printf("Failed to parse module for valid test: %s\n", wah_strerror(err));
        return 1;
    }
    // Create context
    err = wah_exec_context_create(&ctx, &module);
    if (err != WAH_OK) {
        fprintf(stderr, "Error creating execution context for valid test: %s\n", wah_strerror(err));
        wah_free_module(&module);
        return 1;
    }

    params[0].i32 = 20;
    params[1].i32 = 4; // Valid division: 20 / 4 = 5
    err = wah_call(&ctx, &module, 0, params, 2, &result);
    if (err == WAH_OK) {
        printf("- Valid division works: 20 / 4 = %d\n", result.i32);
    } else {
        printf("x Valid division failed with error: %s\n", wah_strerror(err));
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
        return 1;
    }
    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);

    // Test that INT_MIN % -1 = 0 (doesn't trap per spec)
    printf("\n6. Testing i32.rem_s with INT_MIN %% -1 (should return 0, not trap):\n");
    err = wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] local.get 0 local.get 1 i32.rem_s end } ]}");
    if (err != WAH_OK) {
        printf("Failed to parse rem module: %s\n", wah_strerror(err));
        return 1;
    }
    // Create context
    err = wah_exec_context_create(&ctx, &module);
    if (err != WAH_OK) {
        fprintf(stderr, "Error creating execution context for INT_MIN %% -1 test: %s\n", wah_strerror(err));
        wah_free_module(&module);
        return 1;
    }

    params[0].i32 = INT32_MIN;
    params[1].i32 = -1; // This should return 0, not trap
    err = wah_call(&ctx, &module, 0, params, 2, &result);
    if (err == WAH_OK && result.i32 == 0) {
        printf("- INT_MIN %% -1 correctly returns 0 (no trap)\n");
    } else {
        printf("x INT_MIN %% -1 failed: %s, result: %d\n", wah_strerror(err), result.i32);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
        return 1;
    }
    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);

    printf("\n=== All trap tests passed! ===\n");
    return 0;
}
