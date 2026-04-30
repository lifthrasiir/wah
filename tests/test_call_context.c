// Test for call context

#include "../wah.h"
#include "common.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

// --- Host functions for overlap tests (Tests 5-7) ---
// These deliberately write results FIRST, then read params, to verify overlap safety.

// (i32) -> (i32): write result first, then read param
static void host_write_result_then_read_param(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    // Write result first (potentially corrupting param[0] if they overlap)
    wah_result_i32(ctx, 0, 0xDEAD);
    // Now read param -- must still be the original value
    int32_t p = wah_param_i32(ctx, 0);
    // Overwrite result with the correct answer based on the (still intact) param
    wah_result_i32(ctx, 0, p * 3);
}

// (i32, i32) -> (i32): write result first, then read both params
static void host_write_result_then_read_two_params(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    // Write result first
    wah_result_i32(ctx, 0, 0xDEAD);
    // Read both params -- must still be intact
    int32_t a = wah_param_i32(ctx, 0);
    int32_t b = wah_param_i32(ctx, 1);
    wah_result_i32(ctx, 0, a + b);
}

// (i32, i32, i32) -> (i32): write result first, then read all three params
static void host_write_result_then_read_three_params(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    // Write result first
    wah_result_i32(ctx, 0, 0xDEAD);
    // Read all params
    int32_t a = wah_param_i32(ctx, 0);
    int32_t b = wah_param_i32(ctx, 1);
    int32_t c = wah_param_i32(ctx, 2);
    wah_result_i32(ctx, 0, a + b + c);
}

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
    assert_eq_f32(f32_val, 3.14f, 1e-5f);
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
        assert_eq_f32(results[2].f32, 1.5f, 1e-5f);
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

    // Test 5: write result THEN read param -- (i32) -> (i32), overlap at index 0
    printf("Test 5: wah_result then wah_param, (i32)->(i32)\n");
    {
        // Host writes result[0] first, then reads param[0] -- they'd overlap in the old code
        static const char *wasm_spec = "wasm \
            types {[ fn [i32] [i32] ]} \
            imports {[ {'env'} {'f'} fn# 0 ]} \
            funcs {[ ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ ]}";

        wah_module_t wasm_mod = {0}, env_mod = {0};
        wah_exec_context_t ctx = {0};

        assert_ok(wah_new_module(&env_mod, NULL));
        assert_ok(wah_module_export_func(&env_mod, "f", "(i32) -> i32",
            host_write_result_then_read_param, NULL, NULL));

        assert_ok(wah_parse_module_from_spec(&wasm_mod, wasm_spec));
        assert_ok(wah_exec_context_create(&ctx, &wasm_mod, NULL));
        assert_ok(wah_link_module(&ctx, "env", &env_mod));
        assert_ok(wah_instantiate(&ctx));

        wah_export_desc_t entry;
        assert_ok(wah_module_export_by_name(&wasm_mod, "f", &entry));

        // param=7 -> host writes result=0xDEAD, reads param (must be 7), sets result=7*3=21
        wah_value_t param = { .i32 = 7 };
        wah_value_t result = {0};
        assert_ok(wah_call(&ctx, entry.index, &param, 1, &result));
        assert_eq_i32(result.i32, 21);

        wah_exec_context_destroy(&ctx);
        wah_free_module(&wasm_mod);
        wah_free_module(&env_mod);
    }

    // Test 6: write result THEN read params -- (i32, i32) -> (i32), result overlaps param[0]
    printf("Test 6: wah_result then wah_param, (i32,i32)->(i32)\n");
    {
        static const char *wasm_spec = "wasm \
            types {[ fn [i32, i32] [i32] ]} \
            imports {[ {'env'} {'f'} fn# 0 ]} \
            funcs {[ ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ ]}";

        wah_module_t wasm_mod = {0}, env_mod = {0};
        wah_exec_context_t ctx = {0};

        assert_ok(wah_new_module(&env_mod, NULL));
        assert_ok(wah_module_export_func(&env_mod, "f", "(i32, i32) -> i32",
            host_write_result_then_read_two_params, NULL, NULL));

        assert_ok(wah_parse_module_from_spec(&wasm_mod, wasm_spec));
        assert_ok(wah_exec_context_create(&ctx, &wasm_mod, NULL));
        assert_ok(wah_link_module(&ctx, "env", &env_mod));
        assert_ok(wah_instantiate(&ctx));

        wah_export_desc_t entry;
        assert_ok(wah_module_export_by_name(&wasm_mod, "f", &entry));

        // params=(10,20) -> host writes result, then reads a=10,b=20 -> result=30
        wah_value_t params[2] = {{ .i32 = 10 }, { .i32 = 20 }};
        wah_value_t result = {0};
        assert_ok(wah_call(&ctx, entry.index, params, 2, &result));
        assert_eq_i32(result.i32, 30);

        wah_exec_context_destroy(&ctx);
        wah_free_module(&wasm_mod);
        wah_free_module(&env_mod);
    }

    // Test 7: write result THEN read params -- (i32, i32, i32) -> (i32)
    printf("Test 7: wah_result then wah_param, (i32,i32,i32)->(i32)\n");
    {
        static const char *wasm_spec = "wasm \
            types {[ fn [i32, i32, i32] [i32] ]} \
            imports {[ {'env'} {'f'} fn# 0 ]} \
            funcs {[ ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ ]}";

        wah_module_t wasm_mod = {0}, env_mod = {0};
        wah_exec_context_t ctx = {0};

        assert_ok(wah_new_module(&env_mod, NULL));
        assert_ok(wah_module_export_func(&env_mod, "f", "(i32, i32, i32) -> i32",
            host_write_result_then_read_three_params, NULL, NULL));

        assert_ok(wah_parse_module_from_spec(&wasm_mod, wasm_spec));
        assert_ok(wah_exec_context_create(&ctx, &wasm_mod, NULL));
        assert_ok(wah_link_module(&ctx, "env", &env_mod));
        assert_ok(wah_instantiate(&ctx));

        wah_export_desc_t entry;
        assert_ok(wah_module_export_by_name(&wasm_mod, "f", &entry));

        // params=(10,20,30) -> host writes result, then reads all three -> result=60
        wah_value_t params[3] = {{ .i32 = 10 }, { .i32 = 20 }, { .i32 = 30 }};
        wah_value_t result = {0};
        assert_ok(wah_call(&ctx, entry.index, params, 3, &result));
        assert_eq_i32(result.i32, 60);

        wah_exec_context_destroy(&ctx);
        wah_free_module(&wasm_mod);
        wah_free_module(&env_mod);
    }

    printf("All tests passed!\n");
    return 0;
}
