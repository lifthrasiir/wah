#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../wah.h"
#include "common.h"

static const wah_parse_options_t fuel_opts = { .features = WAH_FEATURE_ALL, .enable_fuel_metering = true };

#define PARSE_FUEL(mod, spec) \
    assert_ok(wah_parse_module_from_spec_ex((mod), &fuel_opts, (spec)))

static void test_resume_straight_line(void) {
    printf("Testing straight-line resume...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // func(i32, i32) -> i32 = local.get 0, local.get 1, i32.add, end
    // costs 4 fuel
    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32] [i32]]} funcs {[0]} \
        code {[{[] local.get 0 local.get 1 i32.add end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod));

    wah_value_t params[2] = { {.i32 = 10}, {.i32 = 20} };

    // Not enough fuel: should suspend
    wah_set_fuel(&ctx, 2);
    assert_ok(wah_start(&ctx, 0, params, 2));
    assert_true(wah_is_suspended(&ctx));
    assert_eq_i32((int32_t)wah_exec_state(&ctx), WAH_EXEC_SUSPENDED);

    wah_error_t err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_STATUS_FUEL_EXHAUSTED);
    assert_true(wah_is_suspended(&ctx));

    // Refuel and resume to completion
    wah_set_fuel(&ctx, 100);
    err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_OK);
    assert_eq_i32((int32_t)wah_exec_state(&ctx), WAH_EXEC_FINISHED);

    wah_value_t result;
    uint32_t actual_results;
    assert_ok(wah_finish(&ctx, &result, 1, &actual_results));
    assert_eq_i32(result.i32, 30);
    assert_eq_u32(actual_results, 1);
    assert_eq_i32((int32_t)wah_exec_state(&ctx), WAH_EXEC_READY);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_resume_loop(void) {
    printf("Testing loop resume...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // Loop counting: func(i32) -> i32 = sum 1..n
    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32] [i32]]} funcs {[0]} \
        code {[{[1 i32] \
            block void \
                loop void \
                    local.get 0 i32.eqz br_if 1 \
                    local.get 1 local.get 0 i32.add local.set 1 \
                    local.get 0 i32.const 1 i32.sub local.set 0 \
                    br 0 \
                end \
            end \
            local.get 1 \
        end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod));

    // First, compute with unlimited fuel for reference
    wah_value_t params = { .i32 = 5 };
    wah_value_t ref_result;
    wah_set_fuel(&ctx, INT64_MAX);
    assert_ok(wah_call(&ctx, 0, &params, 1, &ref_result));
    assert_eq_i32(ref_result.i32, 15); // 1+2+3+4+5 = 15

    // Now resume with limited fuel
    wah_set_fuel(&ctx, 5);
    assert_ok(wah_start(&ctx, 0, &params, 1));

    int suspensions = 0;
    wah_error_t err;
    while ((err = wah_resume(&ctx)) > 0) {
        assert_eq_i32(err, WAH_STATUS_FUEL_EXHAUSTED);
        suspensions++;
        wah_set_fuel(&ctx, 5);
    }
    assert_eq_i32(err, WAH_OK);
    assert_true(suspensions > 0);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 15); // Same result as unlimited

    printf("  resumed %d times\n", suspensions);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_resume_multiple_suspensions(void) {
    printf("Testing multiple suspensions...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // Simple function that costs 4 fuel
    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32] [i32]]} funcs {[0]} \
        code {[{[] local.get 0 local.get 1 i32.add end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod));

    wah_value_t params[2] = { {.i32 = 3}, {.i32 = 7} };

    // Give 1 fuel at a time
    wah_set_fuel(&ctx, 1);
    assert_ok(wah_start(&ctx, 0, params, 2));

    int suspensions = 0;
    wah_error_t err;
    while ((err = wah_resume(&ctx)) > 0) {
        suspensions++;
        wah_set_fuel(&ctx, 1);
    }
    assert_eq_i32(err, WAH_OK);
    assert_true(suspensions >= 3);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 10);

    printf("  suspended %d times with 1-fuel increments\n", suspensions);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_resume_nested_calls(void) {
    printf("Testing nested call resume...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // func 0: add(i32,i32)->i32 = local.get 0, local.get 1, i32.add, end
    // func 1: caller(i32,i32)->i32 = local.get 0, local.get 1, call 0, end
    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32] [i32]]} funcs {[0, 0]} \
        code {[ \
            {[] local.get 0 local.get 1 i32.add end}, \
            {[] local.get 0 local.get 1 call 0 end}, \
        ]}");
    assert_ok(wah_exec_context_create(&ctx, &mod));

    wah_value_t params[2] = { {.i32 = 11}, {.i32 = 22} };

    // Very limited fuel to force suspension inside the nested call
    wah_set_fuel(&ctx, 3);
    assert_ok(wah_start(&ctx, 1, params, 2));

    int suspensions = 0;
    wah_error_t err;
    while ((err = wah_resume(&ctx)) > 0) {
        suspensions++;
        wah_set_fuel(&ctx, 3);
    }
    assert_eq_i32(err, WAH_OK);
    assert_true(suspensions > 0);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 33);

    printf("  nested call resumed %d times\n", suspensions);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_resume_yield_via_interrupt(void) {
    printf("Testing yield via interrupt...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // A loop that would run 1000 iterations; interrupt mid-way
    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32] [i32]]} funcs {[0]} \
        code {[{[1 i32] \
            block void \
                loop void \
                    local.get 0 i32.eqz br_if 1 \
                    local.get 1 local.get 0 i32.add local.set 1 \
                    local.get 0 i32.const 1 i32.sub local.set 0 \
                    br 0 \
                end \
            end \
            local.get 1 \
        end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod));

    wah_value_t params = { .i32 = 10 };

    // Give enough fuel but request an interrupt
    wah_set_fuel(&ctx, INT64_MAX);
    assert_ok(wah_start(&ctx, 0, &params, 1));

    wah_request_interrupt(&ctx);
    wah_error_t err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_STATUS_YIELDED);
    assert_true(wah_is_suspended(&ctx));

    // Resume to completion (no more interrupts)
    err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_OK);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 55); // 1+2+...+10

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_cancel_and_reuse(void) {
    printf("Testing cancel and reuse...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32] [i32]]} funcs {[0]} \
        code {[{[] local.get 0 local.get 1 i32.add end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod));

    wah_value_t params[2] = { {.i32 = 5}, {.i32 = 6} };

    // Start with insufficient fuel, then cancel
    wah_set_fuel(&ctx, 1);
    assert_ok(wah_start(&ctx, 0, params, 2));
    wah_error_t err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_STATUS_FUEL_EXHAUSTED);
    assert_true(wah_is_suspended(&ctx));

    wah_cancel(&ctx);
    assert_eq_i32((int32_t)wah_exec_state(&ctx), WAH_EXEC_READY);
    assert_false(wah_is_suspended(&ctx));

    // Regular wah_call should work after cancel
    wah_set_fuel(&ctx, 100);
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, params, 2, &result));
    assert_eq_i32(result.i32, 11);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_destroy_while_suspended(void) {
    printf("Testing destroy while suspended...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32] [i32]]} funcs {[0]} \
        code {[{[] local.get 0 local.get 1 i32.add end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod));

    wah_value_t params[2] = { {.i32 = 1}, {.i32 = 2} };
    wah_set_fuel(&ctx, 1);
    assert_ok(wah_start(&ctx, 0, params, 2));
    wah_resume(&ctx); // fuel exhausted

    // Should not crash
    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_misuse_start_while_suspended(void) {
    printf("Testing misuse: start while suspended...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32] [i32]]} funcs {[0]} \
        code {[{[] local.get 0 local.get 1 i32.add end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod));

    wah_value_t params[2] = { {.i32 = 1}, {.i32 = 2} };
    wah_set_fuel(&ctx, 1);
    assert_ok(wah_start(&ctx, 0, params, 2));
    wah_resume(&ctx);
    assert_true(wah_is_suspended(&ctx));

    assert_err(wah_start(&ctx, 0, params, 2), WAH_ERROR_MISUSE);

    wah_cancel(&ctx);
    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_misuse_resume_while_ready(void) {
    printf("Testing misuse: resume while ready...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [] []]} funcs {[0]} \
        code {[{[] end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod));

    assert_err(wah_resume(&ctx), WAH_ERROR_MISUSE);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_misuse_finish_while_suspended(void) {
    printf("Testing misuse: finish while suspended...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32] [i32]]} funcs {[0]} \
        code {[{[] local.get 0 local.get 1 i32.add end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod));

    wah_value_t params[2] = { {.i32 = 1}, {.i32 = 2} };
    wah_set_fuel(&ctx, 1);
    assert_ok(wah_start(&ctx, 0, params, 2));
    wah_resume(&ctx);
    assert_true(wah_is_suspended(&ctx));

    wah_value_t result;
    uint32_t actual;
    assert_err(wah_finish(&ctx, &result, 1, &actual), WAH_ERROR_MISUSE);

    wah_cancel(&ctx);
    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_wah_call_backward_compat(void) {
    printf("Testing wah_call backward compatibility...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32] [i32]]} funcs {[0]} \
        code {[{[] local.get 0 local.get 1 i32.add end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod));

    wah_value_t params[2] = { {.i32 = 3}, {.i32 = 7} };
    wah_value_t result;

    // wah_call with insufficient fuel returns status and context is READY
    wah_set_fuel(&ctx, 2);
    assert_err(wah_call(&ctx, 0, params, 2, &result), WAH_STATUS_FUEL_EXHAUSTED);
    assert_eq_i32((int32_t)wah_exec_state(&ctx), WAH_EXEC_READY);

    // Can call again normally
    wah_set_fuel(&ctx, 100);
    assert_ok(wah_call(&ctx, 0, params, 2, &result));
    assert_eq_i32(result.i32, 10);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_finish_void_function(void) {
    printf("Testing finish on void function...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [] []]} funcs {[0]} \
        code {[{[] end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod));

    wah_set_fuel(&ctx, 100);
    assert_ok(wah_start(&ctx, 0, NULL, 0));
    wah_error_t err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_OK);

    uint32_t actual;
    assert_ok(wah_finish(&ctx, NULL, 0, &actual));
    assert_eq_u32(actual, 0);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_cancel_ready_noop(void) {
    printf("Testing cancel on ready context (no-op)...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [] []]} funcs {[0]} \
        code {[{[] end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod));

    // Should not crash
    wah_cancel(&ctx);
    assert_eq_i32((int32_t)wah_exec_state(&ctx), WAH_EXEC_READY);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

int main(void) {
    test_resume_straight_line();
    test_resume_loop();
    test_resume_multiple_suspensions();
    test_resume_nested_calls();
    test_resume_yield_via_interrupt();
    test_cancel_and_reuse();
    test_destroy_while_suspended();
    test_misuse_start_while_suspended();
    test_misuse_resume_while_ready();
    test_misuse_finish_while_suspended();
    test_wah_call_backward_compat();
    test_finish_void_function();
    test_cancel_ready_noop();

    printf("\n=== All resume tests passed ===\n");
    return 0;
}
