// Test for call context

#define WAH_IMPLEMENTATION
#include "wah.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

// Test host function that uses all param accessors
void test_params(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;

    printf("  Testing param accessors...\n");

    // Test all param types
    int32_t i32_val = wah_param_i32(ctx, 0);
    int64_t i64_val = wah_param_i64(ctx, 1);
    float f32_val = wah_param_f32(ctx, 2);
    double f64_val = wah_param_f64(ctx, 3);

    printf("    i32: %d\n", i32_val);
    printf("    i64: %lld\n", (long long)i64_val);
    printf("    f32: %f\n", f32_val);
    printf("    f64: %lf\n", f64_val);

    // Verify values
    if (i32_val != 42) {
        printf("    FAIL: expected i32=42, got %d\n", i32_val);
        wah_trap(ctx, WAH_ERROR_TRAP);
        return;
    }
    if (i64_val != 1234567890123LL) {
        printf("    FAIL: expected i64=1234567890123, got %lld\n", (long long)i64_val);
        wah_trap(ctx, WAH_ERROR_TRAP);
        return;
    }
    if (f32_val != 3.14f) {
        printf("    FAIL: expected f32=3.14, got %f\n", f32_val);
        wah_trap(ctx, WAH_ERROR_TRAP);
        return;
    }
    if (f64_val != 2.71828) {
        printf("    FAIL: expected f64=2.71828, got %lf\n", f64_val);
        wah_trap(ctx, WAH_ERROR_TRAP);
        return;
    }

    printf("    PASS\n");
}

// Test host function that uses result accessors
void test_results(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;

    printf("  Testing result accessors...\n");

    // Set all result types
    wah_result_i32(ctx, 0, 100);
    wah_result_i64(ctx, 1, 2000000000000LL);
    wah_result_f32(ctx, 2, 1.5f);
    wah_result_f64(ctx, 3, 2.5);

    printf("    Results set\n");
    printf("    PASS\n");
}

// Test host function that uses return macros
void test_returns(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;

    printf("  Testing return macros...\n");

    // Test single return macros - each call uses index 0, so they overwrite
    // We'll just test the last one (f64)
    wah_return_f64(ctx, 3.14);

    printf("    Returns set\n");
    printf("    PASS\n");
}

// Test host function that traps
void test_trap_func(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;

    printf("  Testing trap...\n");

    // Set some values first
    wah_result_i32(ctx, 0, 999);

    // Then trap
    wah_trap(ctx, WAH_ERROR_TRAP);

    printf("    PASS\n");
}

int main() {
    printf("Testing call context...\n\n");

    // Test 1: Param accessors
    printf("Test 1: Param accessors\n");
    {
        wah_call_context_t ctx = {0};

        // Set up params
        int32_t i32_val = 42;
        int64_t i64_val = 1234567890123LL;
        float f32_val = 3.14f;
        double f64_val = 2.71828;

        wah_value_t params[4] = {
            {.i32 = i32_val},
            {.i64 = i64_val},
            {.f32 = f32_val},
            {.f64 = f64_val}
        };

        wah_type_t param_types[4] = {
            WAH_TYPE_I32,
            WAH_TYPE_I64,
            WAH_TYPE_F32,
            WAH_TYPE_F64
        };

        ctx.params = params;
        ctx.nparams = 4;
        ctx.param_types = param_types;

        test_params(&ctx, NULL);

        if (ctx.trap_reason != WAH_OK) {
            printf("FAIL: Test 1 trapped with %s\n", wah_strerror(ctx.trap_reason));
            return 1;
        }
    }
    printf("\n");

    // Test 2: Result accessors
    printf("Test 2: Result accessors\n");
    {
        wah_call_context_t ctx = {0};
        wah_value_t results[4];
        memset(results, 0, sizeof(results));

        wah_type_t result_types[4] = {
            WAH_TYPE_I32,
            WAH_TYPE_I64,
            WAH_TYPE_F32,
            WAH_TYPE_F64
        };

        ctx.results = results;
        ctx.nresults = 4;
        ctx.result_types = result_types;

        test_results(&ctx, NULL);

        // Verify results
        if (results[0].i32 != 100) {
            printf("FAIL: expected result[0].i32=100, got %d\n", results[0].i32);
            return 1;
        }
        if (results[1].i64 != 2000000000000LL) {
            printf("FAIL: expected result[1].i64=2000000000000, got %lld\n", (long long)results[1].i64);
            return 1;
        }
        if (results[2].f32 != 1.5f) {
            printf("FAIL: expected result[2].f32=1.5, got %f\n", results[2].f32);
            return 1;
        }
        if (results[3].f64 != 2.5) {
            printf("FAIL: expected result[3].f64=2.5, got %lf\n", results[3].f64);
            return 1;
        }
    }
    printf("\n");

    // Test 3: Return macros
    printf("Test 3: Return macros\n");
    {
        wah_call_context_t ctx = {0};
        wah_value_t results[1];
        memset(results, 0, sizeof(results));

        wah_type_t result_types[1] = { WAH_TYPE_F64 };

        ctx.results = results;
        ctx.nresults = 1;
        ctx.result_types = result_types;

        test_returns(&ctx, NULL);

        // Verify results
        if (results[0].f64 != 3.14) {
            printf("FAIL: expected result[0].f64=3.14, got %lf\n", results[0].f64);
            return 1;
        }
    }
    printf("\n");

    // Test 4: Trap
    printf("Test 4: Trap\n");
    {
        wah_call_context_t ctx = {0};
        wah_value_t results[1];
        memset(results, 0, sizeof(results));

        wah_type_t result_types[1] = { WAH_TYPE_I32 };

        ctx.results = results;
        ctx.nresults = 1;
        ctx.result_types = result_types;

        test_trap_func(&ctx, NULL);

        if (ctx.trap_reason != WAH_ERROR_TRAP) {
            printf("FAIL: expected trap, got %s\n", wah_strerror(ctx.trap_reason));
            return 1;
        }

        // results[0] is not guaranteed to be preserved after a trap, but in our implementation it is
    }
    printf("\n");

    printf("All tests passed!\n");
    return 0;
}
