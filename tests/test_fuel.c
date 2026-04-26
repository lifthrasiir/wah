#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../wah.h"
#include "common.h"

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
    assert_ok(wah_exec_context_create(&ctx, &mod));

    wah_value_t params[2] = { {.i32 = 10}, {.i32 = 20} };
    wah_value_t result;

    wah_set_fuel(&ctx, 100);
    assert_ok(wah_call(&ctx, 0, params, 2, &result));
    assert_eq_i32(result.i32, 30);
    int64_t remaining = wah_get_fuel(&ctx);
    printf("  fuel remaining after 4-instr function with fuel=100: %lld\n", (long long)remaining);
    assert_eq_i64(remaining, 96);

    wah_set_fuel(&ctx, 4);
    assert_ok(wah_call(&ctx, 0, params, 2, &result));
    assert_eq_i32(result.i32, 30);
    assert_eq_i64(wah_get_fuel(&ctx), 0);

    wah_set_fuel(&ctx, 3);
    assert_err(wah_call(&ctx, 0, params, 2, &result), WAH_STATUS_FUEL_EXHAUSTED);

    wah_exec_context_destroy(&ctx);
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
    assert_ok(wah_exec_context_create(&ctx, &mod));

    wah_set_fuel(&ctx, 0);
    assert_err(wah_call(&ctx, 0, NULL, 0, NULL), WAH_STATUS_FUEL_EXHAUSTED);

    wah_set_fuel(&ctx, 1);
    assert_ok(wah_call(&ctx, 0, NULL, 0, NULL));
    assert_eq_i64(wah_get_fuel(&ctx), 0);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_no_metering_when_disabled(void) {
    printf("Testing no metering when fuel_metering is disabled...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [i32, i32] [i32]]} funcs {[0]} \
        code {[{[] local.get 0 local.get 1 i32.add end}]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod));

    wah_value_t params[2] = { {.i32 = 5}, {.i32 = 7} };
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, params, 2, &result));
    assert_eq_i32(result.i32, 12);

    wah_exec_context_destroy(&ctx);
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
    assert_ok(wah_exec_context_create(&ctx, &mod));

    wah_value_t params[1] = { {.i32 = 3} };
    wah_value_t result;

    wah_set_fuel(&ctx, 10000);
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 3);
    int64_t consumed = 10000 - wah_get_fuel(&ctx);
    printf("  fuel consumed for loop(3): %lld\n", (long long)consumed);

    params[0].i32 = 3;
    wah_set_fuel(&ctx, consumed / 2);
    assert_err(wah_call(&ctx, 0, params, 1, &result), WAH_STATUS_FUEL_EXHAUSTED);

    wah_exec_context_destroy(&ctx);
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
    assert_ok(wah_exec_context_create(&ctx, &mod));

    wah_value_t params[1];
    wah_value_t result;

    params[0].i32 = 1;
    wah_set_fuel(&ctx, 10000);
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 42);
    int64_t consumed_then = 10000 - wah_get_fuel(&ctx);

    params[0].i32 = 0;
    wah_set_fuel(&ctx, 10000);
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 99);
    int64_t consumed_else = 10000 - wah_get_fuel(&ctx);

    printf("  fuel consumed (then): %lld, (else): %lld\n",
           (long long)consumed_then, (long long)consumed_else);

    wah_exec_context_destroy(&ctx);
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
    assert_ok(wah_exec_context_create(&ctx, &mod));

    wah_value_t params[2] = { {.i32 = 3}, {.i32 = 7} };
    wah_value_t result;

    wah_set_fuel(&ctx, 10000);
    assert_ok(wah_call(&ctx, 0, params, 2, &result));
    assert_eq_i32(result.i32, 10);
    int64_t fuel_add = 10000 - wah_get_fuel(&ctx);

    wah_set_fuel(&ctx, 10000);
    assert_ok(wah_call(&ctx, 1, params, 2, &result));
    assert_eq_i32(result.i32, 10);
    int64_t fuel_caller = 10000 - wah_get_fuel(&ctx);

    printf("  fuel for add: %lld, fuel for caller: %lld\n",
           (long long)fuel_add, (long long)fuel_caller);
    assert_true(fuel_caller > fuel_add);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_max_fuel(void) {
    printf("Testing INT64_MAX fuel (no overflow)...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32] [i32]]} funcs {[0]} \
        code {[{[] local.get 0 local.get 1 i32.add end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod));

    wah_value_t params[2] = { {.i32 = 1}, {.i32 = 2} };
    wah_value_t result;

    wah_set_fuel(&ctx, INT64_MAX);
    assert_ok(wah_call(&ctx, 0, params, 2, &result));
    assert_eq_i32(result.i32, 3);
    assert_true(wah_get_fuel(&ctx) > 0);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_deterministic_fuel(void) {
    printf("Testing deterministic fuel (same fuel = same result)...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32] [i32]]} funcs {[0]} \
        code {[{[] local.get 0 local.get 1 i32.add end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod));

    wah_value_t params[2] = { {.i32 = 10}, {.i32 = 20} };
    wah_value_t result;

    wah_set_fuel(&ctx, 100);
    assert_ok(wah_call(&ctx, 0, params, 2, &result));
    int64_t fuel1 = wah_get_fuel(&ctx);

    wah_set_fuel(&ctx, 100);
    assert_ok(wah_call(&ctx, 0, params, 2, &result));
    int64_t fuel2 = wah_get_fuel(&ctx);

    assert_eq_i64(fuel1, fuel2);

    wah_exec_context_destroy(&ctx);
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
    assert_ok(wah_exec_context_create(&ctx, &mod));

    wah_value_t params[2] = { {.i32 = 10}, {.i32 = 20} };
    wah_value_t result;

    // Full execution needs 6 fuel
    wah_set_fuel(&ctx, 6);
    assert_ok(wah_call(&ctx, 0, params, 2, &result));
    assert_eq_i32(result.i32, 31);
    assert_eq_i64(wah_get_fuel(&ctx), 0);

    // With fuel=3: METER chunk cost > 3, enters slow path.
    // Slow path should execute 3 TICKs then exhaust on the 4th.
    wah_set_fuel(&ctx, 3);
    assert_err(wah_call(&ctx, 0, params, 2, &result), WAH_STATUS_FUEL_EXHAUSTED);
    // Fuel should be exactly -1 (3 TICKs succeeded: 3->2->1->0, 4th TICK: 0->-1 < 0)
    assert_eq_i64(wah_get_fuel(&ctx), -1);

    // With fuel=1: should execute exactly 1 instruction in slow path
    wah_set_fuel(&ctx, 1);
    assert_err(wah_call(&ctx, 0, params, 2, &result), WAH_STATUS_FUEL_EXHAUSTED);
    assert_eq_i64(wah_get_fuel(&ctx), -1);

    // With fuel=5: one short of completion
    wah_set_fuel(&ctx, 5);
    assert_err(wah_call(&ctx, 0, params, 2, &result), WAH_STATUS_FUEL_EXHAUSTED);
    assert_eq_i64(wah_get_fuel(&ctx), -1);

    wah_exec_context_destroy(&ctx);
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
    assert_ok(wah_exec_context_create(&ctx, &mod));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;

    // Full execution: all 5 steps complete, global=5
    wah_set_fuel(&ctx, 10000);
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 5);

    // Reset global to 0
    wah_set_fuel(&ctx, 10000);
    assert_ok(wah_call(&ctx, 2, NULL, 0, NULL));

    // Now call stepper with limited fuel.
    // Each "step" is: global.get(1) + i32.const(1) + i32.add(1) + global.set(1) = 4 instr.
    // After 5 steps: global.get(1) + end(1) = 2 more. Total = 22 instructions.
    // Count the full cost first:
    wah_set_fuel(&ctx, 10000);
    assert_ok(wah_call(&ctx, 2, NULL, 0, NULL)); // reset
    wah_set_fuel(&ctx, 10000);
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
        wah_set_fuel(&ctx, 10000); \
        assert_ok(wah_call(&ctx, 2, NULL, 0, NULL)); \
        wah_set_fuel(&ctx, (fuel_val)); \
        assert_err(wah_call(&ctx, 0, NULL, 0, &result), WAH_STATUS_FUEL_EXHAUSTED); \
        wah_set_fuel(&ctx, 10000); \
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

    wah_exec_context_destroy(&ctx);
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

    printf("\n=== All fuel tests passed ===\n");
    return 0;
}
