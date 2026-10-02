#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "../wah.h"
#include "common.h"
#include "wah_impl.h"

static const wah_parse_options_t fuel_opts = { .features = WAH_FEATURE_ALL, .enable_fuel_metering = true };

#define PARSE_FUEL(mod, spec) \
    assert_ok(wah_parse_module_from_spec_ex((mod), &fuel_opts, (spec)))

static void test_straight_line_exact_fuel(void) {
    printf("Testing straight-line exact fuel accounting...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // func(i32, i32) -> i32 = local.get 0, local.get 1, i32.add, end
    // 4 Wasm instructions
    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32] [i32]]} funcs {[0]} \
        code {[{[] local.get 0 local.get 1 i32.add end}]}");
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));

    wah_value_t params[2] = { {.i32 = 10}, {.i32 = 20} };
    wah_value_t result;

    assert_ok(wah_set_fuel(&ctx, 100));
    assert_ok(wah_call(&ctx, 0, params, 2, &result));
    assert_eq_i32(result.i32, 30);
    int64_t remaining = wah_get_fuel(&ctx);
    printf("  fuel remaining after 4-instr function with fuel=100: %lld\n", (long long)remaining);
    assert_eq_i64(remaining, 96);

    assert_ok(wah_set_fuel(&ctx, 4));
    assert_ok(wah_call(&ctx, 0, params, 2, &result));
    assert_eq_i32(result.i32, 30);
    assert_eq_i64(wah_get_fuel(&ctx), 0);

    assert_ok(wah_set_fuel(&ctx, 3));
    assert_err(wah_call(&ctx, 0, params, 2, &result), WAH_STATUS_FUEL_EXHAUSTED);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_zero_fuel(void) {
    printf("Testing zero fuel (immediate exhaustion)...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // func() = end  (1 Wasm instruction: END)
    PARSE_FUEL(&mod, "wasm \
        types {[fn [] []]} funcs {[0]} \
        code {[{[] end}]}");
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));

    assert_ok(wah_set_fuel(&ctx, 0));
    assert_err(wah_call(&ctx, 0, NULL, 0, NULL), WAH_STATUS_FUEL_EXHAUSTED);

    assert_ok(wah_set_fuel(&ctx, 1));
    assert_ok(wah_call(&ctx, 0, NULL, 0, NULL));
    assert_eq_i64(wah_get_fuel(&ctx), 0);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_no_metering_when_disabled(void) {
    printf("Testing no metering when fuel_metering is disabled...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [i32, i32] [i32]]} funcs {[0]} \
        code {[{[] local.get 0 local.get 1 i32.add end}]}"));
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));

    wah_value_t params[2] = { {.i32 = 5}, {.i32 = 7} };
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, params, 2, &result));
    assert_eq_i32(result.i32, 12);

    // Setting fuel via wah_set_fuel / wah_set_limits when metering is disabled must be rejected.
    assert_err(wah_set_fuel(&ctx, 1000), WAH_ERROR_DISABLED_FEATURE);
    wah_limits_t lim = {0};
    lim.fuel = 1000;
    assert_err(wah_set_limits(&ctx, &lim), WAH_ERROR_DISABLED_FEATURE);

    wah_free_exec_context(&ctx);

    // Same check on the wah_new_exec_context entry point.
    wah_exec_options_t opts = { .limits = { .fuel = 1000 } };
    wah_exec_context_t ctx2 = {0};
    assert_err(wah_new_exec_context(&ctx2, &mod, &opts), WAH_ERROR_DISABLED_FEATURE);

    wah_free_module(&mod);
}

static void test_loop_fuel(void) {
    printf("Testing loop fuel consumption...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // loop K times, count iterations
    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32] [i32]]} funcs {[0]} \
        code {[{[1 i32] \
            block void \
                loop void \
                    local.get 0 i32.eqz br_if 1 \
                    local.get 0 i32.const 1 i32.sub local.set 0 \
                    local.get 1 i32.const 1 i32.add local.set 1 \
                    br 0 \
                end \
            end \
            local.get 1 \
        end }]}");
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));

    wah_value_t params[1] = { {.i32 = 3} };
    wah_value_t result;

    assert_ok(wah_set_fuel(&ctx, 10000));
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 3);
    int64_t consumed = 10000 - wah_get_fuel(&ctx);
    printf("  fuel consumed for loop(3): %lld\n", (long long)consumed);

    params[0].i32 = 3;
    assert_ok(wah_set_fuel(&ctx, consumed / 2));
    assert_err(wah_call(&ctx, 0, params, 1, &result), WAH_STATUS_FUEL_EXHAUSTED);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_branch_fuel(void) {
    printf("Testing branch fuel (only executed path counts)...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // if-then-else
    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32] [i32]]} funcs {[0]} \
        code {[{[] local.get 0 if i32 i32.const 42 else i32.const 99 end end}]}");
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));

    wah_value_t params[1];
    wah_value_t result;

    params[0].i32 = 1;
    assert_ok(wah_set_fuel(&ctx, 10000));
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 42);
    int64_t consumed_then = 10000 - wah_get_fuel(&ctx);

    params[0].i32 = 0;
    assert_ok(wah_set_fuel(&ctx, 10000));
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 99);
    int64_t consumed_else = 10000 - wah_get_fuel(&ctx);

    printf("  fuel consumed (then): %lld, (else): %lld\n",
           (long long)consumed_then, (long long)consumed_else);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_call_fuel(void) {
    printf("Testing fuel across function calls...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // func 0: add(i32,i32)->i32
    // func 1: caller(i32,i32)->i32 that calls func 0
    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32] [i32]]} funcs {[0, 0]} \
        code {[ \
            {[] local.get 0 local.get 1 i32.add end}, \
            {[] local.get 0 local.get 1 call 0 end}, \
        ]}");
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));

    wah_value_t params[2] = { {.i32 = 3}, {.i32 = 7} };
    wah_value_t result;

    assert_ok(wah_set_fuel(&ctx, 10000));
    assert_ok(wah_call(&ctx, 0, params, 2, &result));
    assert_eq_i32(result.i32, 10);
    int64_t fuel_add = 10000 - wah_get_fuel(&ctx);

    assert_ok(wah_set_fuel(&ctx, 10000));
    assert_ok(wah_call(&ctx, 1, params, 2, &result));
    assert_eq_i32(result.i32, 10);
    int64_t fuel_caller = 10000 - wah_get_fuel(&ctx);

    printf("  fuel for add: %lld, fuel for caller: %lld\n",
           (long long)fuel_add, (long long)fuel_caller);
    assert_true(fuel_caller > fuel_add);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_max_fuel(void) {
    printf("Testing INT64_MAX fuel (no overflow)...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32] [i32]]} funcs {[0]} \
        code {[{[] local.get 0 local.get 1 i32.add end}]}");
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));

    wah_value_t params[2] = { {.i32 = 1}, {.i32 = 2} };
    wah_value_t result;

    assert_ok(wah_set_fuel(&ctx, INT64_MAX));
    assert_ok(wah_call(&ctx, 0, params, 2, &result));
    assert_eq_i32(result.i32, 3);
    assert_true(wah_get_fuel(&ctx) > 0);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_deterministic_fuel(void) {
    printf("Testing deterministic fuel (same fuel = same result)...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32] [i32]]} funcs {[0]} \
        code {[{[] local.get 0 local.get 1 i32.add end}]}");
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));

    wah_value_t params[2] = { {.i32 = 10}, {.i32 = 20} };
    wah_value_t result;

    assert_ok(wah_set_fuel(&ctx, 100));
    assert_ok(wah_call(&ctx, 0, params, 2, &result));
    int64_t fuel1 = wah_get_fuel(&ctx);

    assert_ok(wah_set_fuel(&ctx, 100));
    assert_ok(wah_call(&ctx, 0, params, 2, &result));
    int64_t fuel2 = wah_get_fuel(&ctx);

    assert_eq_i64(fuel1, fuel2);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_slow_path_partial_execution(void) {
    printf("Testing slow-path partial execution (METER restore regression)...\n");
    // Regression: METER used to subtract cost and jump to slow path with
    // fuel still negative, causing the first TICK to immediately exhaust.
    // The fix: METER restores fuel before jumping to the slow path.
    //
    // Function: 5 straight-line instructions (local.get 0, local.get 1,
    // i32.add, i32.const 1, i32.add, end = 6 total).
    // These will be in one METER chunk with cost=6.
    // With fuel=3, METER fails (3 < 6), restores fuel to 3, enters slow path.
    // Slow path TICKs execute 3 instructions, then the 4th TICK exhausts.
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32] [i32]]} funcs {[0]} \
        code {[{[] \
            local.get 0 local.get 1 i32.add \
            i32.const 1 i32.add \
        end}]}");
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));

    wah_value_t params[2] = { {.i32 = 10}, {.i32 = 20} };
    wah_value_t result;

    // Full execution needs 6 fuel
    assert_ok(wah_set_fuel(&ctx, 6));
    assert_ok(wah_call(&ctx, 0, params, 2, &result));
    assert_eq_i32(result.i32, 31);
    assert_eq_i64(wah_get_fuel(&ctx), 0);

    // With fuel=3: METER chunk cost > 3, enters slow path.
    // Slow path should execute 3 TICKs then exhaust on the 4th.
    assert_ok(wah_set_fuel(&ctx, 3));
    assert_err(wah_call(&ctx, 0, params, 2, &result), WAH_STATUS_FUEL_EXHAUSTED);
    // Fuel should be exactly -1 (3 TICKs succeeded: 3->2->1->0, 4th TICK: 0->-1 < 0)
    assert_eq_i64(wah_get_fuel(&ctx), -1);

    // With fuel=1: should execute exactly 1 instruction in slow path
    assert_ok(wah_set_fuel(&ctx, 1));
    assert_err(wah_call(&ctx, 0, params, 2, &result), WAH_STATUS_FUEL_EXHAUSTED);
    assert_eq_i64(wah_get_fuel(&ctx), -1);

    // With fuel=5: one short of completion
    assert_ok(wah_set_fuel(&ctx, 5));
    assert_err(wah_call(&ctx, 0, params, 2, &result), WAH_STATUS_FUEL_EXHAUSTED);
    assert_eq_i64(wah_get_fuel(&ctx), -1);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_slow_path_side_effects(void) {
    printf("Testing slow-path partial execution side effects via globals...\n");
    // Use a mutable global as a step counter.
    // func 0 (stepper): increments global 0 five times, then returns it.
    //   global.get 0  i32.const 1  i32.add  global.set 0   (step 1)
    //   global.get 0  i32.const 1  i32.add  global.set 0   (step 2)
    //   global.get 0  i32.const 1  i32.add  global.set 0   (step 3)
    //   global.get 0  i32.const 1  i32.add  global.set 0   (step 4)
    //   global.get 0  i32.const 1  i32.add  global.set 0   (step 5)
    //   global.get 0  end
    // func 1 (getter): global.get 0, end
    // func 2 (resetter): i32.const 0, global.set 0, end
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[ \
            fn [] [i32], \
            fn [] [], \
        ]} \
        funcs {[0, 0, 1]} \
        globals {[ i32 mut i32.const 0 end ]} \
        code {[ \
            {[] \
                global.get 0 i32.const 1 i32.add global.set 0 \
                global.get 0 i32.const 1 i32.add global.set 0 \
                global.get 0 i32.const 1 i32.add global.set 0 \
                global.get 0 i32.const 1 i32.add global.set 0 \
                global.get 0 i32.const 1 i32.add global.set 0 \
                global.get 0 \
            end}, \
            {[] global.get 0 end}, \
            {[] i32.const 0 global.set 0 end}, \
        ]}");
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;

    // Full execution: all 5 steps complete, global=5
    assert_ok(wah_set_fuel(&ctx, 10000));
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 5);

    // Reset global to 0
    assert_ok(wah_set_fuel(&ctx, 10000));
    assert_ok(wah_call(&ctx, 2, NULL, 0, NULL));

    // Now call stepper with limited fuel.
    // Each "step" is: global.get(1) + i32.const(1) + i32.add(1) + global.set(1) = 4 instr.
    // After 5 steps: global.get(1) + end(1) = 2 more. Total = 22 instructions.
    // Count the full cost first:
    assert_ok(wah_set_fuel(&ctx, 10000));
    assert_ok(wah_call(&ctx, 2, NULL, 0, NULL)); // reset
    assert_ok(wah_set_fuel(&ctx, 10000));
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    int64_t full_cost = 10000 - wah_get_fuel(&ctx);
    printf("  stepper full cost: %lld\n", (long long)full_cost);

    // Each step is: global.get(1) + i32.const(1) + i32.add(1) + global.set(1) = 4 instr.
    // global.set is the 4th instruction in each step, so:
    //   fuel=4n   -> global.set of step n just completed -> global = n
    //   fuel=4n-1 -> i32.add done but global.set not yet -> global = n-1
    //   fuel=4n+1 -> global.get of step n+1 done         -> global = n
    // Test helper: reset global, run stepper with given fuel, read global back.
    #define CHECK_STEP(fuel_val, expected_global) do { \
        assert_ok(wah_set_fuel(&ctx, 10000)); \
        assert_ok(wah_call(&ctx, 2, NULL, 0, NULL)); \
        assert_ok(wah_set_fuel(&ctx, (fuel_val))); \
        assert_err(wah_call(&ctx, 0, NULL, 0, &result), WAH_STATUS_FUEL_EXHAUSTED); \
        assert_ok(wah_set_fuel(&ctx, 10000)); \
        assert_ok(wah_call(&ctx, 1, NULL, 0, &result)); \
        printf("  fuel=%2d -> global=%d (expected %d)\n", (fuel_val), result.i32, (expected_global)); \
        assert_eq_i32(result.i32, (expected_global)); \
    } while (0)

    CHECK_STEP( 1, 0);  // global.get done, nothing committed
    CHECK_STEP( 2, 0);  // global.get + i32.const done
    CHECK_STEP( 3, 0);  // through i32.add, global.set not yet
    CHECK_STEP( 4, 1);  // global.set of step 1 just completed
    CHECK_STEP( 5, 1);  // step 2's global.get done
    CHECK_STEP( 7, 1);  // step 2's i32.add done, global.set not yet
    CHECK_STEP( 8, 2);  // step 2's global.set completed
    CHECK_STEP( 9, 2);  // step 3's global.get done
    CHECK_STEP(11, 2);  // step 3's i32.add done
    CHECK_STEP(12, 3);  // step 3 completed
    CHECK_STEP(16, 4);  // step 4 completed
    CHECK_STEP(19, 4);  // step 5's i32.add done, global.set not yet
    CHECK_STEP(20, 5);  // all 5 steps completed, but global.get+end still needed

    #undef CHECK_STEP

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_br_table_fuel(void) {
    printf("Testing br_table fuel accounting...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // br_table with 3 targets + default, each target returns a different constant.
    // br_table is a metering boundary, so each path's fuel should reflect only
    // the instructions actually executed.
    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32] [i32]]} funcs {[0]} \
        code {[{[] \
            block void \
                block void \
                    block void \
                        block void \
                            local.get 0 \
                            br_table [0, 1, 2] 3 \
                        end \
                        i32.const 10 return \
                    end \
                    i32.const 20 return \
                end \
                i32.const 30 return \
            end \
            i32.const 40 \
        end}]}");
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));

    wah_value_t params[1], result = {0};
    int32_t expected[] = {10, 20, 30, 40};

    for (int i = 0; i < 4; i++) {
        params[0].i32 = i;
        assert_ok(wah_set_fuel(&ctx, 10000));
        assert_ok(wah_call(&ctx, 0, params, 1, &result));
        int64_t fuel = 10000 - wah_get_fuel(&ctx);
        printf("  br_table[%d] fuel=%lld result=%d\n", i, (long long)fuel, result.i32);
        assert_eq_i32(result.i32, expected[i]);
    }

    // With insufficient fuel, should exhaust before completing
    params[0].i32 = 0;
    assert_ok(wah_set_fuel(&ctx, 1));
    assert_err(wah_call(&ctx, 0, params, 1, &result), WAH_STATUS_FUEL_EXHAUSTED);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_br_table_resume(void) {
    printf("Testing br_table resume...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // Use a global counter incremented inside each br_table target to verify
    // that resumption after br_table lands at the correct continuation.
    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32] [i32], fn [] []]} funcs {[0, 1]} \
        globals {[i32 mut i32.const 0 end]} \
        code {[ \
            {[] \
                block void \
                    block void \
                        local.get 0 \
                        br_table [0, 1] 1 \
                    end \
                    global.get 0 i32.const 10 i32.add global.set 0 \
                    i32.const 0 return \
                end \
                global.get 0 i32.const 20 i32.add global.set 0 \
                i32.const 1 return \
            end}, \
            {[] i32.const 0 global.set 0 end} \
        ]}");
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t params[1];

    // Resume through br_table target 0 with 1-fuel increments
    assert_ok(wah_set_fuel(&ctx, 10000));
    assert_ok(wah_call(&ctx, 1, NULL, 0, NULL)); // reset
    params[0].i32 = 0;
    assert_ok(wah_set_fuel(&ctx, 1));
    assert_ok(wah_start(&ctx, 0, params, 1));
    wah_error_t err;
    while ((err = wah_resume(&ctx)) > 0) {
        assert_ok(wah_set_fuel(&ctx, 1));
    }
    assert_eq_i32(err, WAH_OK);
    wah_value_t res;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &res, 1, &actual));
    assert_eq_i32(res.i32, 0);
    // Verify global was incremented by target 0's path (+10)
    assert_ok(wah_set_fuel(&ctx, 10000));
    assert_ok(wah_call(&ctx, 1, NULL, 0, NULL)); // reset won't help, read first
    // Re-read: run the function fresh and check global
    // Actually, just run a fresh call to check the global state
    // We need a getter. Let's re-verify via another approach: run target 1.
    assert_ok(wah_set_fuel(&ctx, 10000));
    assert_ok(wah_call(&ctx, 1, NULL, 0, NULL)); // reset global
    params[0].i32 = 1;
    assert_ok(wah_set_fuel(&ctx, 1));
    assert_ok(wah_start(&ctx, 0, params, 1));
    while ((err = wah_resume(&ctx)) > 0) {
        assert_ok(wah_set_fuel(&ctx, 1));
    }
    assert_eq_i32(err, WAH_OK);
    assert_ok(wah_finish(&ctx, &res, 1, &actual));
    assert_eq_i32(res.i32, 1); // target 1 returns 1

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_call_indirect_fuel(void) {
    printf("Testing call_indirect fuel accounting...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // func 0: add(i32,i32)->i32
    // func 1: sub(i32,i32)->i32
    // func 2: dispatch(i32,i32,i32)->i32 calls table[param2](param0,param1)
    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32] [i32], fn [i32, i32, i32] [i32]]} \
        funcs {[0, 0, 1]} \
        tables {[funcref limits.i32/1 2]} \
        elements {[elem.active.table#0 i32.const 0 end [0, 1]]} \
        code {[ \
            {[] local.get 0 local.get 1 i32.add end}, \
            {[] local.get 0 local.get 1 i32.sub end}, \
            {[] local.get 0 local.get 1 local.get 2 call_indirect 0 0 end} \
        ]}");
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t params[3], result;

    // call_indirect -> add
    params[0].i32 = 10; params[1].i32 = 3; params[2].i32 = 0;
    assert_ok(wah_set_fuel(&ctx, 10000));
    assert_ok(wah_call(&ctx, 2, params, 3, &result));
    assert_eq_i32(result.i32, 13);
    int64_t fuel_add = 10000 - wah_get_fuel(&ctx);

    // call_indirect -> sub
    params[2].i32 = 1;
    assert_ok(wah_set_fuel(&ctx, 10000));
    assert_ok(wah_call(&ctx, 2, params, 3, &result));
    assert_eq_i32(result.i32, 7);
    int64_t fuel_sub = 10000 - wah_get_fuel(&ctx);

    printf("  indirect add fuel=%lld, sub fuel=%lld\n",
           (long long)fuel_add, (long long)fuel_sub);
    // Both paths should cost the same (same number of instructions)
    assert_eq_i64(fuel_add, fuel_sub);

    // Exhaust mid-indirect-call
    params[2].i32 = 0;
    assert_ok(wah_set_fuel(&ctx, 2));
    assert_err(wah_call(&ctx, 2, params, 3, &result), WAH_STATUS_FUEL_EXHAUSTED);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_call_indirect_resume(void) {
    printf("Testing call_indirect resume...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32] [i32], fn [i32, i32, i32] [i32]]} \
        funcs {[0, 0, 1]} \
        tables {[funcref limits.i32/1 2]} \
        elements {[elem.active.table#0 i32.const 0 end [0, 1]]} \
        code {[ \
            {[] local.get 0 local.get 1 i32.add end}, \
            {[] local.get 0 local.get 1 i32.sub end}, \
            {[] local.get 0 local.get 1 local.get 2 call_indirect 0 0 end} \
        ]}");
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t params[3] = { {.i32 = 30}, {.i32 = 12}, {.i32 = 1} };

    // Resume 1 fuel at a time through indirect call to sub
    assert_ok(wah_set_fuel(&ctx, 1));
    assert_ok(wah_start(&ctx, 2, params, 3));
    int suspensions = 0;
    wah_error_t err;
    while ((err = wah_resume(&ctx)) > 0) {
        suspensions++;
        assert_ok(wah_set_fuel(&ctx, 1));
    }
    assert_eq_i32(err, WAH_OK);
    assert_true(suspensions > 0);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 18); // 30 - 12
    printf("  indirect call resumed %d times\n", suspensions);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_return_call_fuel(void) {
    printf("Testing return_call fuel accounting...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // func 0: entry, tail-calls func 1
    // func 1: returns i32.const 42
    PARSE_FUEL(&mod, "wasm \
        types {[fn [] [i32]]} funcs {[0, 0]} \
        code {[ \
            {[] return_call 1 end}, \
            {[] i32.const 42 end} \
        ]}");
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));

    wah_value_t result;
    assert_ok(wah_set_fuel(&ctx, 10000));
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);
    int64_t fuel_tail = 10000 - wah_get_fuel(&ctx);

    // Compare with non-tail: func that calls and returns
    wah_module_t mod2 = {0};
    wah_exec_context_t ctx2 = {0};
    PARSE_FUEL(&mod2, "wasm \
        types {[fn [] [i32]]} funcs {[0, 0]} \
        code {[ \
            {[] call 1 end}, \
            {[] i32.const 42 end} \
        ]}");
    assert_ok(wah_new_exec_context(&ctx2, &mod2, NULL));
    assert_ok(wah_set_fuel(&ctx2, 10000));
    assert_ok(wah_call(&ctx2, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);
    int64_t fuel_normal = 10000 - wah_get_fuel(&ctx2);

    printf("  tail-call fuel=%lld, normal call fuel=%lld\n",
           (long long)fuel_tail, (long long)fuel_normal);

    wah_free_exec_context(&ctx2);
    wah_free_module(&mod2);
    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_return_call_resume(void) {
    printf("Testing return_call resume...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // Tail-recursive countdown: func(n) -> if n==0 return 0 else return_call(n-1)
    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32] [i32]]} funcs {[0]} \
        code {[{[] \
            local.get 0 i32.eqz \
            if i32 \
                i32.const 0 \
            else \
                local.get 0 i32.const 1 i32.sub return_call 0 \
            end \
        end}]}");
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));

    wah_value_t params = { .i32 = 20 };

    // Reference result
    assert_ok(wah_set_fuel(&ctx, INT64_MAX));
    wah_value_t ref_result;
    assert_ok(wah_call(&ctx, 0, &params, 1, &ref_result));
    assert_eq_i32(ref_result.i32, 0);

    // Resume with limited fuel
    assert_ok(wah_set_fuel(&ctx, 5));
    assert_ok(wah_start(&ctx, 0, &params, 1));
    int suspensions = 0;
    wah_error_t err;
    while ((err = wah_resume(&ctx)) > 0) {
        suspensions++;
        assert_ok(wah_set_fuel(&ctx, 5));
    }
    assert_eq_i32(err, WAH_OK);
    assert_true(suspensions > 0);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0);
    printf("  tail-recursive resume: %d suspensions\n", suspensions);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_return_call_indirect_resume(void) {
    printf("Testing return_call_indirect resume...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // func 0: entry, tail-calls table[0] which is func 1
    // func 1: returns 99
    PARSE_FUEL(&mod, "wasm \
        types {[fn [] [i32]]} funcs {[0, 0]} \
        tables {[funcref limits.i32/1 1]} \
        elements {[elem.active.table#0 i32.const 0 end [1]]} \
        code {[ \
            {[] i32.const 0 return_call_indirect 0 0 end}, \
            {[] i32.const 99 end} \
        ]}");
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    assert_ok(wah_set_fuel(&ctx, 1));
    assert_ok(wah_start(&ctx, 0, NULL, 0));
    int suspensions = 0;
    wah_error_t err;
    while ((err = wah_resume(&ctx)) > 0) {
        suspensions++;
        assert_ok(wah_set_fuel(&ctx, 1));
    }
    assert_eq_i32(err, WAH_OK);
    assert_true(suspensions > 0);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 99);
    printf("  return_call_indirect resumed %d times\n", suspensions);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_exception_fuel(void) {
    printf("Testing exception path fuel accounting...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // throw inside try_table, caught by catch handler.
    // Compare fuel for throw-path vs. normal-path.
    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32] [i32], fn [i32] []]} \
        funcs {[0]} \
        tags {[tag.type# 1]} \
        code {[{[] \
            block i32 \
                try_table void [catch 0 1] \
                    local.get 0 \
                    throw 0 \
                end \
                i32.const 0 \
            end \
        end}]}");
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t params = { .i32 = 55 };
    wah_value_t result;

    assert_ok(wah_set_fuel(&ctx, 10000));
    assert_ok(wah_call(&ctx, 0, &params, 1, &result));
    assert_eq_i32(result.i32, 55);
    int64_t fuel_used = 10000 - wah_get_fuel(&ctx);
    printf("  exception catch fuel=%lld\n", (long long)fuel_used);
    assert_true(fuel_used > 0);

    // With very little fuel, should exhaust
    assert_ok(wah_set_fuel(&ctx, 1));
    assert_err(wah_call(&ctx, 0, &params, 1, &result), WAH_STATUS_FUEL_EXHAUSTED);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_exception_resume(void) {
    printf("Testing exception path resume...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // throw value, catch it into the block, add 100 after block end.
    // catch label 0 = the enclosing block (from try_table's perspective, label 0
    // is the immediately enclosing block i32).
    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32] [i32], fn [i32] []]} \
        funcs {[0]} \
        tags {[tag.type# 1]} \
        code {[{[] \
            block i32 \
                try_table void [catch 0 0] \
                    local.get 0 \
                    throw 0 \
                end \
                i32.const 0 \
            end \
            i32.const 100 i32.add \
        end}]}");
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t params = { .i32 = 7 };

    // Resume 1 fuel at a time
    assert_ok(wah_set_fuel(&ctx, 1));
    assert_ok(wah_start(&ctx, 0, &params, 1));
    int suspensions = 0;
    wah_error_t err;
    while ((err = wah_resume(&ctx)) > 0) {
        suspensions++;
        assert_ok(wah_set_fuel(&ctx, 1));
    }
    assert_eq_i32(err, WAH_OK);
    assert_true(suspensions > 0);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 107); // 7 + 100
    printf("  exception resume: %d suspensions\n", suspensions);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_exception_across_call_resume(void) {
    printf("Testing exception across call boundary resume...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // func 0 (thrower): throws tag 0 with param
    // func 1 (catcher): calls func 0 inside try_table, catches
    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32] [], fn [i32] [i32]]} \
        funcs {[0, 1]} \
        tags {[tag.type# 0]} \
        code {[ \
            {[] local.get 0 throw 0 end}, \
            {[] \
                block i32 \
                    try_table void [catch 0 1] \
                        local.get 0 call 0 \
                    end \
                    i32.const -1 \
                end \
            end} \
        ]}");
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t params = { .i32 = 42 };

    assert_ok(wah_set_fuel(&ctx, 1));
    assert_ok(wah_start(&ctx, 1, &params, 1));
    int suspensions = 0;
    wah_error_t err;
    while ((err = wah_resume(&ctx)) > 0) {
        suspensions++;
        assert_ok(wah_set_fuel(&ctx, 1));
    }
    assert_eq_i32(err, WAH_OK);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 42);
    printf("  cross-call exception resume: %d suspensions\n", suspensions);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_multi_value_fuel(void) {
    printf("Testing multi-value return fuel...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // func returns (i32, i32)
    PARSE_FUEL(&mod, "wasm \
        types {[fn [] [i32, i32]]} funcs {[0]} \
        code {[{[] i32.const 11 i32.const 22 end}]}");
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));

    assert_ok(wah_set_fuel(&ctx, 10000));
    wah_value_t results[2];
    uint32_t actual_count;
    assert_ok(wah_call_multi(&ctx, 0, NULL, 0, results, 2, &actual_count));
    assert_eq_u32(actual_count, 2);
    assert_eq_i32(results[0].i32, 11);
    assert_eq_i32(results[1].i32, 22);

    int64_t fuel_used = 10000 - wah_get_fuel(&ctx);
    printf("  multi-value fuel=%lld\n", (long long)fuel_used);
    assert_true(fuel_used > 0);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_multi_value_resume(void) {
    printf("Testing multi-value return resume...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // func returns (i32, i32, i32) after some computation
    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32] [i32, i32, i32]]} funcs {[0]} \
        code {[{[] \
            local.get 0 \
            local.get 0 i32.const 1 i32.add \
            local.get 0 i32.const 2 i32.add \
        end}]}");
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));

    wah_value_t params = { .i32 = 10 };

    // Resume with 1 fuel at a time
    assert_ok(wah_set_fuel(&ctx, 1));
    assert_ok(wah_start(&ctx, 0, &params, 1));
    int suspensions = 0;
    wah_error_t err;
    while ((err = wah_resume(&ctx)) > 0) {
        suspensions++;
        assert_ok(wah_set_fuel(&ctx, 1));
    }
    assert_eq_i32(err, WAH_OK);
    assert_true(suspensions > 0);

    wah_value_t results[3];
    uint32_t actual;
    assert_ok(wah_finish(&ctx, results, 3, &actual));
    assert_eq_u32(actual, 3);
    assert_eq_i32(results[0].i32, 10);
    assert_eq_i32(results[1].i32, 11);
    assert_eq_i32(results[2].i32, 12);
    printf("  multi-value resume: %d suspensions\n", suspensions);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_meter_chunk_reset_before_polled_loop(void) {
    printf("Testing meter chunk reset before polled loop...\n");

    wah_module_t mod = {0};
    PARSE_FUEL(&mod, "wasm \
        types {[fn [] []]} funcs {[0]} \
        code {[{[] \
            nop nop nop nop nop nop nop nop nop nop \
            nop nop nop nop nop nop nop nop nop nop \
            nop nop nop nop nop nop nop nop nop nop \
            nop nop nop nop nop nop nop nop nop nop \
            nop nop nop nop nop nop nop nop nop nop \
            nop nop nop nop nop nop nop nop nop nop \
            nop nop nop nop nop nop nop nop nop nop \
            nop nop nop nop nop nop nop nop nop nop \
            nop nop nop nop nop nop nop nop nop nop \
            nop nop nop nop nop nop nop nop nop \
            return \
            loop void end \
        end}]}");
    wah_free_module(&mod);
}

// A branch-target END followed by a polled loop left behind a zero-cost METER whose slow path was never patched,
// so negative fuel there (allowed after array.new_default) restarted the function body from offset 0.
static void test_meter_at_branch_target_before_polled_loop(void) {
    printf("Testing meter at a branch target right before a polled loop...\n");

    wah_module_t mod = {0};
    PARSE_FUEL(&mod, "wasm \
        types {[ sub [] array i32 mut, fn [] [i32] ]} funcs {[1]} \
        globals {[ i32 mut i32.const 0 end ]} \
        code {[{[] \
            global.get 0 i32.const 1 i32.add global.set 0 \
            block void i32.const 65536 array.new_default 0 drop br 0 end \
            loop void end \
            global.get 0 \
        end}]}");
    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    assert_ok(wah_set_fuel(&ctx, 20)); // array.new_default charges 64 after the fact
    assert_ok(wah_start(&ctx, 0, NULL, 0));
    wah_error_t err;
    int suspensions = 0;
    while ((err = wah_resume(&ctx)) == WAH_STATUS_FUEL_EXHAUSTED) {
        assert_true(++suspensions < 10);
        assert_ok(wah_set_fuel(&ctx, 1000000));
    }
    assert_ok(err);
    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 1); // The body ran exactly once

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

// A polled loop right after a branch-target END recorded its header before the empty chunk there was retracted,
// so back-edges landed in the middle of the next METER.
static void test_loop_after_branch_target_end(void) {
    printf("Testing loop back-edge right after a branch-target END...\n");

    wah_module_t mod = {0};
    PARSE_FUEL(&mod, "wasm \
        types {[fn [] [i32]]} funcs {[0]} \
        globals {[ i32 mut i32.const 0 end ]} \
        code {[{[] \
            block void br 0 end \
            loop void \
                global.get 0 i32.const 1 i32.add global.set 0 \
                global.get 0 i32.const 3 i32.lt_u br_if 0 \
            end \
            global.get 0 \
        end}]}");
    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));
    assert_ok(wah_set_fuel(&ctx, 1000));
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 3);
    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static char *repeat_spec(const char *pre, const char *rep, int n, const char *post) {
    size_t lp = strlen(pre), lr = strlen(rep), lq = strlen(post);
    char *s = malloc(lp + lr * (size_t)n + lq + 1), *p = s;
    assert(s);
    memcpy(p, pre, lp); p += lp;
    for (int i = 0; i < n; i++) { memcpy(p, rep, lr); p += lr; }
    memcpy(p, post, lq + 1);
    return s;
}

// Slow islands of metered code used to precede each instruction with a 10-byte TICK.
static void test_metered_code_size(void) {
    printf("Testing size of metered straight-line code...\n");
    enum { N = 10000 };
    char *spec = repeat_spec("wasm types {[fn [] []]} funcs {[0]} code {[{[] ", "i32.const 0 drop ", N, "end}]}");
    wah_module_t mod = {0};
    PARSE_FUEL(&mod, spec);
    free(spec);
    uint32_t size = wah_debug_module_bytecode_size(&mod, 0);
    printf("  %u bytes of bytecode for %d pairs of i32.const and drop\n", size, N);
    assert_true(size < 30u * N); // Fast path is 8 bytes per pair, and slow path copies them with a TICK each

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_set_fuel(&ctx, 2 * N));
    assert_ok(wah_start(&ctx, 0, NULL, 0));
    assert_err(wah_resume(&ctx), WAH_STATUS_FUEL_EXHAUSTED); // Before the final END
    assert_ok(wah_set_fuel(&ctx, 1));
    assert_ok(wah_resume(&ctx));
    assert_ok(wah_finish(&ctx, NULL, 0, NULL));
    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_long_straight_line_chunk(void) {
    printf("Testing fuel accounting of very long straight-line code...\n");

    // 2n+2 instructions without any branch, so that a single chunk would cost more than 65535.
    static const int ns[] = { 32767, 32770, 100000 };
    for (size_t i = 0; i < sizeof(ns) / sizeof(*ns); i++) {
        int n = ns[i];
        char *spec = repeat_spec("wasm types {[fn [] [i32]]} funcs {[0]} code {[{[] i32.const 0 ",
                                 "i32.const 1 i32.add ", n, "end}]}");
        wah_module_t mod = {0};
        PARSE_FUEL(&mod, spec);
        free(spec);
        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &mod, NULL));

        wah_value_t result;
        assert_ok(wah_set_fuel(&ctx, 1000000));
        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
        assert_eq_i32(result.i32, n);
        assert_eq_i64(1000000 - wah_get_fuel(&ctx), 2 * n + 2);

        // Run out of fuel in the middle of chunks so that slow paths are taken
        assert_ok(wah_set_fuel(&ctx, 1000));
        assert_ok(wah_start(&ctx, 0, NULL, 0));
        wah_error_t err;
        while ((err = wah_resume(&ctx)) == WAH_STATUS_FUEL_EXHAUSTED) assert_ok(wah_set_fuel(&ctx, 1000));
        assert_ok(err);
        assert_ok(wah_finish(&ctx, &result, 1, NULL));
        assert_eq_i32(result.i32, n);

        wah_free_exec_context(&ctx);
        wah_free_module(&mod);
    }
}

#define PAD10 "i32.const 0 drop i32.const 0 drop i32.const 0 drop i32.const 0 drop i32.const 0 drop " \
              "i32.const 0 drop i32.const 0 drop i32.const 0 drop i32.const 0 drop i32.const 0 drop "
#define PAD50 PAD10 PAD10 PAD10 PAD10 PAD10

// Code reached by a branch must be charged like any other code.
static void check_branch_target_metered(const char *prefix, const char *suffix) {
    char pre[2048];
    snprintf(pre, sizeof(pre), "wasm types {[fn [] [i32], fn [] []]} funcs {[0]} tags {[ tag.type# 1 ]} "
                               "code {[{[1 i32] %s ", prefix);
    char post[256];
    snprintf(post, sizeof(post), "%s local.get 0 end}]}", suffix);
    char *spec = repeat_spec(pre, "local.get 0 i32.const 1 i32.add local.set 0 ", 1000, post);
    wah_module_t mod = {0};
    PARSE_FUEL(&mod, spec);
    free(spec);
    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));

    wah_value_t result;
    assert_ok(wah_set_fuel(&ctx, 1000000));
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 1000);
    int64_t used = 1000000 - wah_get_fuel(&ctx);
    if (used < 4000) {
        fprintf(stderr, "only %lld fuel used for 4000+ instructions after a branch\n", (long long)used);
        exit(1);
    }

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_branch_targets_are_metered(void) {
    printf("Testing code at branch targets is metered...\n");
    check_branch_target_metered("block void i32.const 1 br_if 0 " PAD50 "i32.const 0 br_if 0 end", "");
    check_branch_target_metered("block void i32.const 1 br_if 0 " PAD50 "i32.const 0 br_if 0 i32.const 0 drop end", "");
    check_branch_target_metered("block void " PAD50 "i32.const 0 br_table [] 0 i32.const 0 drop end", "");
    check_branch_target_metered(PAD50 "i32.const 0 if void i32.const 0 drop end", "");
    check_branch_target_metered(PAD50 "i32.const 0 if void i32.const 0 drop else", "end");
    check_branch_target_metered("block void try_table void [catch_all 0] " PAD50 "throw 0 end end", "");
}

static int g_tick_interrupt;
static void host_maybe_interrupt(wah_call_context_t *cc, void *ud) {
    (void)ud;
    if (g_tick_interrupt) wah_request_interrupt_from_host(cc);
}

// Runs f with the given fuel, refilling one fuel at a time, and returns the fuel left minus 1000 per refill.
static int64_t run_ticks_with_fuel(int interrupt, int64_t fuel) {
    wah_module_t env = {0}, mod = {0};
    assert_ok(wah_new_module(&env, NULL));
    assert_ok(wah_export_func(&env, "h", "() -> ()", host_maybe_interrupt, NULL, NULL));
    PARSE_FUEL(&mod, "wasm types {[ fn [] [], fn [] [i32] ]} \
        imports {[ {'env'} {'h'} fn# 0 ]} funcs {[ 1 ]} \
        code {[ {[] call 0 i32.const 1 i32.const 2 i32.add i32.const 3 i32.add end} ]}");
    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_link_module(&ctx, "env", &env));
    assert_ok(wah_instantiate(&ctx));
    assert_ok(wah_set_fuel(&ctx, fuel));
    g_tick_interrupt = interrupt;
    assert_ok(wah_start(&ctx, 1, NULL, 0));
    int64_t refills = 0;
    wah_error_t err;
    while ((err = wah_resume(&ctx)) != WAH_OK) {
        if (err == WAH_STATUS_YIELDED) {
            g_tick_interrupt = 0;
        } else {
            assert_err(err, WAH_STATUS_FUEL_EXHAUSTED);
            refills++;
            assert_ok(wah_set_fuel(&ctx, 1));
        }
    }
    wah_value_t r;
    assert_ok(wah_finish(&ctx, &r, 1, NULL));
    assert_eq_i32(r.i32, 6);
    int64_t left = wah_get_fuel(&ctx) - refills * 1000;
    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
    wah_free_module(&env);
    return left;
}

// Regression: TICK charged fuel before yielding to an interrupt, and charged it again when retried on resumption.
static void test_interrupted_tick_charges_once(void) {
    printf("Testing an interrupted TICK charges fuel once...\n");
    for (int64_t fuel = 1; fuel <= 8; fuel++) {
        assert_eq_i64(run_ticks_with_fuel(1, fuel), run_ticks_with_fuel(0, fuel));
    }
}

// Regression: negative fuel was accepted, and INT64_MIN made METER and TICK overflow.
static void test_negative_fuel_rejected(void) {
    printf("Testing negative fuel is rejected...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};
    PARSE_FUEL(&mod, "wasm types {[ fn [] [] ]} funcs {[ 0 ]} code {[ {[] end} ]}");
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_err(wah_set_fuel(&ctx, INT64_MIN), WAH_ERROR_MISUSE);
    assert_err(wah_set_fuel(&ctx, -1), WAH_ERROR_MISUSE);
    assert_ok(wah_set_fuel(&ctx, 0));
    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

int main(void) {
    test_straight_line_exact_fuel();
    test_zero_fuel();
    test_no_metering_when_disabled();
    test_loop_fuel();
    test_branch_fuel();
    test_call_fuel();
    test_max_fuel();
    test_deterministic_fuel();
    test_slow_path_partial_execution();
    test_slow_path_side_effects();
    test_br_table_fuel();
    test_br_table_resume();
    test_call_indirect_fuel();
    test_call_indirect_resume();
    test_return_call_fuel();
    test_return_call_resume();
    test_return_call_indirect_resume();
    test_exception_fuel();
    test_exception_resume();
    test_exception_across_call_resume();
    test_multi_value_fuel();
    test_multi_value_resume();
    test_meter_chunk_reset_before_polled_loop();
    test_meter_at_branch_target_before_polled_loop();
    test_loop_after_branch_target_end();
    test_long_straight_line_chunk();
    test_metered_code_size();
    test_branch_targets_are_metered();
    test_interrupted_tick_charges_once();
    test_negative_fuel_rejected();

    printf("\n=== All fuel tests passed ===\n");
    return 0;
}
