// Test for call context

#define WAH_IMPLEMENTATION
#include "wah.h"
#include "wah_testutils.c"
#include <stdio.h>
#include <string.h>
#include <assert.h>

// Test host function that uses all param accessors
void test_params(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;

    printf("  Testing param accessors...");

    // Test all param types
    int32_t i32_val = wah_param_i32(ctx, 0);
    int64_t i64_val = wah_param_i64(ctx, 1);
    float f32_val = wah_param_f32(ctx, 2);
    double f64_val = wah_param_f64(ctx, 3);

    // Verify values
    assert_eq_i32(i32_val, 42);
    assert_eq_i64(i64_val, 1234567890123LL);
    assert_eq_f32(f32_val, 3.14f, 1e-5);
    assert_eq_f64(f64_val, 2.71828, 1e-5);

    printf(" PASS\n");
}

// Test host function that uses result accessors
void test_results(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;

    printf("  Testing result accessors...");

    // Set all result types
    wah_result_i32(ctx, 0, 100);
    wah_result_i64(ctx, 1, 2000000000000LL);
    wah_result_f32(ctx, 2, 1.5f);
    wah_result_f64(ctx, 3, 2.5);

    printf(" PASS\n");
}

// Test host function that uses return macros
void test_returns(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;

    printf("  Testing return macros...");

    // Test single return macros - each call uses index 0, so they overwrite
    // We'll just test the last one (f64)
    wah_return_f64(ctx, 3.14);

    printf(" PASS\n");
}

// Test host function that traps
void test_trap_func(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;

    printf("  Testing trap...");

    // Set some values first
    wah_result_i32(ctx, 0, 999);

    // Then trap
    wah_trap(ctx, WAH_ERROR_TRAP);

    printf(" PASS\n");
}

int main() {
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

        assert_err(ctx.trap_reason, WAH_OK);
    }

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

        assert_eq_i32(results[0].i32, 100);
        assert_eq_i64(results[1].i64, 2000000000000LL);
        assert_eq_f32(results[2].f32, 1.5f, 1e-5);
        assert_eq_f64(results[3].f64, 2.5, 1e-5);
    }

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
        assert_eq_f64(results[0].f64, 3.14, 1e-5);
    }

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

        assert_err(ctx.trap_reason, WAH_ERROR_TRAP);

        // results[0] is not guaranteed to be preserved after a trap, but in our implementation it is
    }

    printf("All tests passed!\n");
    return 0;
}
