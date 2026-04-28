#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../wah.h"
#include "common.h"
#include "wah_impl.h"

static const wah_parse_options_t fuel_opts = { .features = WAH_FEATURE_ALL, .enable_fuel_metering = true };

static void host_request_interrupt(wah_call_context_t *call, void *userdata) {
    (void)userdata;
    wah_request_interrupt(call->exec);
}

static void replace_data_segment_with_fill(wah_module_t *mod, uint32_t data_idx, uint32_t size, uint8_t value) {
    assert_ok(wah_debug_replace_data_segment_fill(mod, data_idx, size, value));
}

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

static void test_resume_br_table(void) {
    printf("Testing resume through br_table...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // br_table dispatches to different blocks, each pushes a different result.
    // Resuming with 1 fuel at a time must produce the same result as unlimited fuel.
    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32] [i32]]} funcs {[0]} \
        code {[{[] \
            block void \
                block void \
                    block void \
                        local.get 0 \
                        br_table [0, 1, 2] 2 \
                    end \
                    i32.const 100 return \
                end \
                i32.const 200 return \
            end \
            i32.const 300 \
        end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod));

    int32_t expected[] = {100, 200, 300, 300};
    for (int i = 0; i < 4; i++) {
        wah_value_t params = { .i32 = i };
        wah_set_fuel(&ctx, 1);
        assert_ok(wah_start(&ctx, 0, &params, 1));
        wah_error_t err;
        while ((err = wah_resume(&ctx)) > 0) {
            wah_set_fuel(&ctx, 1);
        }
        assert_eq_i32(err, WAH_OK);
        wah_value_t result;
        uint32_t actual;
        assert_ok(wah_finish(&ctx, &result, 1, &actual));
        assert_eq_i32(result.i32, expected[i]);
    }

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_resume_tail_call_deep(void) {
    printf("Testing resume through deep tail-call recursion...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // Tail-recursive sum: sum(n, acc) -> if n==0 return acc else return_call sum(n-1, acc+n)
    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32] [i32]]} funcs {[0]} \
        code {[{[] \
            local.get 0 i32.eqz \
            if i32 \
                local.get 1 \
            else \
                local.get 0 i32.const 1 i32.sub \
                local.get 1 local.get 0 i32.add \
                return_call 0 \
            end \
        end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod));

    wah_value_t params[2] = { {.i32 = 100}, {.i32 = 0} };

    // Reference result with unlimited fuel
    wah_set_fuel(&ctx, INT64_MAX);
    wah_value_t ref;
    assert_ok(wah_call(&ctx, 0, params, 2, &ref));
    assert_eq_i32(ref.i32, 5050); // 1+2+...+100

    // Resume with small fuel chunks
    wah_set_fuel(&ctx, 3);
    assert_ok(wah_start(&ctx, 0, params, 2));
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
    assert_eq_i32(result.i32, 5050);
    printf("  deep tail-call: %d suspensions\n", suspensions);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_resume_return_call_indirect(void) {
    printf("Testing resume through return_call_indirect...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // func 0: entry, dispatches to table[param] via return_call_indirect
    // func 1: returns 111
    // func 2: returns 222
    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32] [i32], fn [] [i32]]} \
        funcs {[0, 1, 1]} \
        tables {[funcref limits.i32/1 2]} \
        elements {[elem.active.table#0 i32.const 0 end [1, 2]]} \
        code {[ \
            {[] local.get 0 return_call_indirect 1 0 end}, \
            {[] i32.const 111 end}, \
            {[] i32.const 222 end} \
        ]}");
    assert_ok(wah_exec_context_create(&ctx, &mod));
    assert_ok(wah_instantiate(&ctx));

    for (int target = 0; target < 2; target++) {
        wah_value_t params = { .i32 = target };
        wah_set_fuel(&ctx, 1);
        assert_ok(wah_start(&ctx, 0, &params, 1));
        wah_error_t err;
        while ((err = wah_resume(&ctx)) > 0) {
            wah_set_fuel(&ctx, 1);
        }
        assert_eq_i32(err, WAH_OK);
        wah_value_t result;
        uint32_t actual;
        assert_ok(wah_finish(&ctx, &result, 1, &actual));
        assert_eq_i32(result.i32, target == 0 ? 111 : 222);
    }

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_resume_exception_catch(void) {
    printf("Testing resume through exception catch...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // Throw inside try_table, catch delivers value, add 1000, return.
    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32] [i32], fn [i32] []]} \
        funcs {[0]} \
        tags {[tag.type# 1]} \
        code {[{[] \
            block i32 \
                try_table void [catch 0 0] \
                    local.get 0 throw 0 \
                end \
                i32.const -1 \
            end \
            i32.const 1000 i32.add \
        end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t params = { .i32 = 77 };

    wah_set_fuel(&ctx, 1);
    assert_ok(wah_start(&ctx, 0, &params, 1));
    int suspensions = 0;
    wah_error_t err;
    while ((err = wah_resume(&ctx)) > 0) {
        suspensions++;
        wah_set_fuel(&ctx, 1);
    }
    assert_eq_i32(err, WAH_OK);
    assert_true(suspensions > 0);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 1077); // 77 + 1000
    printf("  exception catch resume: %d suspensions\n", suspensions);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_resume_multi_value(void) {
    printf("Testing resume with multi-value results...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32] [i32, i32, i32]]} funcs {[0]} \
        code {[{[] \
            local.get 0 local.get 1 i32.add \
            local.get 0 local.get 1 i32.sub \
            local.get 0 local.get 1 i32.mul \
        end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod));

    wah_value_t params[2] = { {.i32 = 7}, {.i32 = 3} };

    wah_set_fuel(&ctx, 1);
    assert_ok(wah_start(&ctx, 0, params, 2));
    int suspensions = 0;
    wah_error_t err;
    while ((err = wah_resume(&ctx)) > 0) {
        suspensions++;
        wah_set_fuel(&ctx, 1);
    }
    assert_eq_i32(err, WAH_OK);
    assert_true(suspensions > 0);

    wah_value_t results[3];
    uint32_t actual;
    assert_ok(wah_finish(&ctx, results, 3, &actual));
    assert_eq_u32(actual, 3);
    assert_eq_i32(results[0].i32, 10);  // 7+3
    assert_eq_i32(results[1].i32, 4);   // 7-3
    assert_eq_i32(results[2].i32, 21);  // 7*3
    printf("  multi-value resume: %d suspensions\n", suspensions);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_resume_gc_funcref_on_stack(void) {
    printf("Testing GC safety: funcref on stack during suspension...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // func 0: push ref.func 0 onto stack, do some i32 work, then test the ref.
    // The funcref must survive GC cycles during suspension.
    PARSE_FUEL(&mod, "wasm \
        types {[fn [] [i32]]} funcs {[0]} \
        exports {[{'f'} fn# 0]} \
        code {[{[1 funcref] \
            ref.func 0 local.set 0 \
            i32.const 1 i32.const 2 i32.add drop \
            i32.const 3 i32.const 4 i32.add drop \
            local.get 0 ref.is_null \
        end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod));

    wah_set_fuel(&ctx, 1);
    assert_ok(wah_start(&ctx, 0, NULL, 0));
    int suspensions = 0;
    wah_error_t err;
    while ((err = wah_resume(&ctx)) > 0) {
        suspensions++;
        wah_gc_step(&ctx);
        assert_true(wah_gc_verify_heap(&ctx));
        wah_set_fuel(&ctx, 1);
    }
    assert_eq_i32(err, WAH_OK);
    assert_true(suspensions > 0);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0); // ref.func 0 is not null
    printf("  gc funcref on stack: %d suspensions, heap valid\n", suspensions);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_resume_gc_ref_across_calls(void) {
    printf("Testing GC safety: funcref across nested calls during suspension...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // func 0: creates a funcref, calls func 1, then tests the ref.
    // func 1: does some work (burns fuel) so suspension happens while ref is live in caller's frame.
    PARSE_FUEL(&mod, "wasm \
        types {[fn [] [i32], fn [] []]} funcs {[0, 1]} \
        exports {[{'f'} fn# 0]} \
        code {[ \
            {[1 funcref] \
                ref.func 0 local.set 0 \
                call 1 \
                local.get 0 ref.is_null \
            end}, \
            {[] \
                i32.const 1 i32.const 2 i32.add drop \
                i32.const 3 i32.const 4 i32.add drop \
                i32.const 5 i32.const 6 i32.add drop \
            end} \
        ]}");
    assert_ok(wah_exec_context_create(&ctx, &mod));

    wah_set_fuel(&ctx, 2);
    assert_ok(wah_start(&ctx, 0, NULL, 0));
    int suspensions = 0;
    wah_error_t err;
    while ((err = wah_resume(&ctx)) > 0) {
        suspensions++;
        wah_gc_step(&ctx);
        assert_true(wah_gc_verify_heap(&ctx));
        wah_set_fuel(&ctx, 2);
    }
    assert_eq_i32(err, WAH_OK);
    assert_true(suspensions > 0);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0); // funcref should still be valid (not null)
    printf("  gc ref across calls: %d suspensions\n", suspensions);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_cancel_with_live_refs(void) {
    printf("Testing cancel with live funcref on stack...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [] [i32]]} funcs {[0]} \
        exports {[{'f'} fn# 0]} \
        code {[{[1 funcref] \
            ref.func 0 local.set 0 \
            i32.const 1 i32.const 2 i32.add drop \
            i32.const 3 i32.const 4 i32.add drop \
            local.get 0 ref.is_null \
        end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod));

    wah_set_fuel(&ctx, 2);
    assert_ok(wah_start(&ctx, 0, NULL, 0));
    wah_error_t err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_STATUS_FUEL_EXHAUSTED);
    assert_true(wah_is_suspended(&ctx));

    // Cancel while funcref is live on the value stack
    wah_cancel(&ctx);
    assert_eq_i32((int32_t)wah_exec_state(&ctx), WAH_EXEC_READY);

    // GC and verify heap after cancel
    wah_gc_step(&ctx);
    assert_true(wah_gc_verify_heap(&ctx));

    // Context should be reusable
    wah_set_fuel(&ctx, 100);
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 0);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_destroy_with_live_refs(void) {
    printf("Testing destroy while suspended with live refs...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [] [i32]]} funcs {[0]} \
        exports {[{'f'} fn# 0]} \
        code {[{[1 funcref] \
            ref.func 0 local.set 0 \
            i32.const 1 i32.const 2 i32.add drop \
            local.get 0 ref.is_null \
        end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod));

    wah_set_fuel(&ctx, 2);
    assert_ok(wah_start(&ctx, 0, NULL, 0));
    wah_resume(&ctx); // fuel exhausted, funcref live in locals
    assert_true(wah_is_suspended(&ctx));

    // Destroy with live refs should not crash or leak
    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_cancel_with_pending_exception(void) {
    printf("Testing cancel with pending exception...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // func 0: calls func 1 which throws. func 0 has a try_table to catch it,
    // but the throw + unwind path requires fuel. If we exhaust mid-unwind or
    // just before the throw, we get suspended with exception state live.
    // Here we set up a scenario where we suspend during the computation
    // leading up to the throw.
    // func 0 (catcher): calls func 1 inside try_table, catch_all returns -1.
    // func 1 (thrower): does work then throws.
    // tag type is fn [] [] (no params).
    PARSE_FUEL(&mod, "wasm \
        types {[fn [] [i32], fn [] []]} \
        funcs {[0, 1]} \
        tags {[tag.type# 1]} \
        code {[ \
            {[] \
                block void \
                    try_table void [catch_all 0] \
                        call 1 \
                    end \
                end \
                i32.const -1 \
            end}, \
            {[] \
                i32.const 1 i32.const 2 i32.add drop \
                i32.const 3 i32.const 4 i32.add drop \
                throw 0 \
            end} \
        ]}");
    assert_ok(wah_exec_context_create(&ctx, &mod));
    assert_ok(wah_instantiate(&ctx));

    // Suspend inside func 1 before the throw executes
    wah_set_fuel(&ctx, 3);
    assert_ok(wah_start(&ctx, 0, NULL, 0));
    wah_error_t err = wah_resume(&ctx);
    // Might be suspended or completed depending on fuel accounting
    if (err > 0) {
        assert_true(wah_is_suspended(&ctx));
        // Cancel while potentially mid-exception-setup
        wah_cancel(&ctx);
        assert_eq_i32((int32_t)wah_exec_state(&ctx), WAH_EXEC_READY);
    }

    // Verify context is clean and reusable
    wah_set_fuel(&ctx, 10000);
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, -1);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_resume_gc_ref_global(void) {
    printf("Testing GC safety: funcref global during suspension...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // Mutable funcref global. Function stores ref.func into it, does work, reads it back.
    PARSE_FUEL(&mod, "wasm \
        types {[fn [] [i32]]} funcs {[0]} \
        globals {[funcref mut ref.null funcref end]} \
        exports {[{'f'} fn# 0]} \
        code {[{[] \
            ref.func 0 global.set 0 \
            i32.const 1 i32.const 2 i32.add drop \
            i32.const 3 i32.const 4 i32.add drop \
            global.get 0 ref.is_null \
        end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod));
    assert_ok(wah_instantiate(&ctx));

    wah_set_fuel(&ctx, 1);
    assert_ok(wah_start(&ctx, 0, NULL, 0));
    int suspensions = 0;
    wah_error_t err;
    while ((err = wah_resume(&ctx)) > 0) {
        suspensions++;
        wah_gc_step(&ctx);
        assert_true(wah_gc_verify_heap(&ctx));
        wah_set_fuel(&ctx, 1);
    }
    assert_eq_i32(err, WAH_OK);
    assert_true(suspensions > 0);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0); // global funcref is not null
    printf("  gc ref global: %d suspensions\n", suspensions);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_poll_yield_without_fuel(void) {
    printf("Testing POLL yield without fuel metering...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // No fuel metering -- just a loop that runs many iterations.
    // wah_request_interrupt should cause WAH_STATUS_YIELDED at POLL.
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
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
        end}]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t params = { .i32 = 100 };

    assert_ok(wah_start(&ctx, 0, &params, 1));
    wah_request_interrupt(&ctx);
    wah_error_t err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_STATUS_YIELDED);
    assert_true(wah_is_suspended(&ctx));

    // Resume to completion
    err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_OK);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 5050); // 1+2+...+100

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_bulk_table_fill_interrupt(void) {
    printf("Testing bulk table.fill interrupt...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // Create a table with many entries, fill it, then read one entry.
    // func 0: table.fill(0, ref.null, 1000) then table.get 0 ref.is_null -> 1
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32]]} funcs {[0]} \
        tables {[funcref limits.i32/2 1000 1000]} \
        code {[{[] \
            i32.const 0 ref.null funcref i32.const 1000 table.fill 0 \
            i32.const 500 table.get 0 ref.is_null \
        end}]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod));
    assert_ok(wah_instantiate(&ctx));

    // Run with interrupt -- should yield and resume correctly
    assert_ok(wah_start(&ctx, 0, NULL, 0));
    wah_request_interrupt(&ctx);
    wah_error_t err = wah_resume(&ctx);
    // 1000 entries is small (< 2^24 check interval), so the fill completes
    // before the check fires. The interrupt will be caught at the next POLL.
    if (err == WAH_STATUS_YIELDED) {
        err = wah_resume(&ctx);
    }
    assert_eq_i32(err, WAH_OK);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 1); // table entry is null

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_bulk_memory_fill_interrupt(void) {
    printf("Testing bulk memory.fill interrupt...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // Create a module with 1 page memory, fill it, then load a byte.
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32]]} funcs {[0]} \
        memories {[limits.i32/1 1]} \
        code {[{[] \
            i32.const 0 i32.const 42 i32.const 65536 memory.fill 0 \
            i32.const 100 i32.load8_u 0 0 \
        end}]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod));
    assert_ok(wah_instantiate(&ctx));

    assert_ok(wah_start(&ctx, 0, NULL, 0));
    wah_request_interrupt(&ctx);
    wah_error_t err = wah_resume(&ctx);
    // 64KB fill is small, interrupt caught at next POLL
    while (err > 0) {
        err = wah_resume(&ctx);
    }
    assert_eq_i32(err, WAH_OK);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 42);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_bulk_memory_copy_interrupt(void) {
    printf("Testing bulk memory.copy interrupt...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // Fill first half with 0xAA, copy to second half, load from second half.
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32]]} funcs {[0]} \
        memories {[limits.i32/1 1]} \
        code {[{[] \
            i32.const 0 i32.const 170 i32.const 32768 memory.fill 0 \
            i32.const 32768 i32.const 0 i32.const 32768 memory.copy 0 0 \
            i32.const 50000 i32.load8_u 0 0 \
        end}]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod));
    assert_ok(wah_instantiate(&ctx));

    assert_ok(wah_start(&ctx, 0, NULL, 0));
    wah_request_interrupt(&ctx);
    wah_error_t err = wah_resume(&ctx);
    while (err > 0) {
        err = wah_resume(&ctx);
    }
    assert_eq_i32(err, WAH_OK);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 170); // 0xAA

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_bulk_memory_fill_large(void) {
    printf("Testing bulk memory.fill > 2^24 bytes with actual chunk interrupt...\n");
    wah_module_t env = {0};
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_new_module(&env));
    assert_ok(wah_module_export_func(&env, "interrupt", "()", host_request_interrupt, NULL, NULL));

    // 512 pages = 32MB > 2^24 (16MB). The chunked fill must cross at least one
    // chunk boundary, so wah_bulk_should_interrupt fires between chunks.
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32], fn [] []]} \
        imports {[ {'env'} {'interrupt'} fn# 1 ]} \
        funcs {[0]} \
        memories {[limits.i32/2 512 512]} \
        code {[{[] \
            call 0 \
            i32.const 0 i32.const 0xAB i32.const %d32 memory.fill 0 \
            i32.const %d32 i32.load8_u 0 0 \
        end}]}", 512 * 65536, 512 * 65536 - 1));
    assert_ok(wah_exec_context_create(&ctx, &mod));
    assert_ok(wah_link_module(&ctx, "env", &env));
    assert_ok(wah_instantiate(&ctx));

    // The host call requests interrupt immediately before the bulk op, so the
    // interrupt is picked up at a bulk-op chunk boundary.
    assert_ok(wah_start(&ctx, 1, NULL, 0));
    wah_error_t err = wah_resume(&ctx);
    // With 32MB fill and 16MB chunks, the first chunk completes then
    // the interrupt check fires -> WAH_STATUS_YIELDED.
    assert_eq_i32(err, WAH_STATUS_YIELDED);
    assert_true(wah_is_suspended(&ctx));
    assert_eq_u32(ctx.memory_base[0], 0xAB);
    assert_eq_u32(ctx.memory_base[(1u << 24) - 1], 0xAB);
    assert_eq_u32(ctx.memory_base[1u << 24], 0x00);
    printf("  yielded after first chunk (expected)\n");

    // Resume to completion
    while ((err = wah_resume(&ctx)) > 0) {
        // No more interrupts, should complete
    }
    assert_eq_i32(err, WAH_OK);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0xAB);
    printf("  32MB fill completed correctly after resume\n");

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
    wah_free_module(&env);
}

static void test_bulk_memory_copy_large(void) {
    printf("Testing bulk memory.copy > 2^24 bytes with partial commit interrupt...\n");
    wah_module_t env = {0};
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    const uint32_t chunk = 1u << 24;
    const uint32_t copy_size = chunk * 2;
    const uint32_t dest = copy_size;

    assert_ok(wah_new_module(&env));
    assert_ok(wah_module_export_func(&env, "interrupt", "()", host_request_interrupt, NULL, NULL));

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32], fn [] []]} \
        imports {[ {'env'} {'interrupt'} fn# 1 ]} \
        funcs {[0]} \
        memories {[limits.i32/2 1024 1024]} \
        code {[{[] \
            call 0 \
            i32.const %d32 i32.const 0 i32.const %d32 memory.copy 0 0 \
            i32.const %d32 i32.load8_u 0 0 \
        end}]}", (int32_t)dest, (int32_t)copy_size, (int32_t)(dest + copy_size - 1)));
    assert_ok(wah_exec_context_create(&ctx, &mod));
    assert_ok(wah_link_module(&ctx, "env", &env));
    assert_ok(wah_instantiate(&ctx));

    memset(ctx.memory_base, 0xCD, copy_size);

    assert_ok(wah_start(&ctx, 1, NULL, 0));
    wah_error_t err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_STATUS_YIELDED);
    assert_true(wah_is_suspended(&ctx));
    assert_eq_u32(ctx.memory_base[dest], 0xCD);
    assert_eq_u32(ctx.memory_base[dest + chunk - 1], 0xCD);
    assert_eq_u32(ctx.memory_base[dest + chunk], 0x00);

    while ((err = wah_resume(&ctx)) > 0) {
    }
    assert_eq_i32(err, WAH_OK);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0xCD);
    assert_eq_u32(ctx.memory_base[dest + copy_size - 1], 0xCD);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
    wah_free_module(&env);
}

static void test_bulk_memory_init_large(void) {
    printf("Testing bulk memory.init > 2^24 bytes with partial commit interrupt...\n");
    wah_module_t env = {0};
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    const uint32_t chunk = 1u << 24;
    const uint32_t init_size = chunk * 2;

    assert_ok(wah_new_module(&env));
    assert_ok(wah_module_export_func(&env, "interrupt", "()", host_request_interrupt, NULL, NULL));

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32], fn [] []]} \
        imports {[ {'env'} {'interrupt'} fn# 1 ]} \
        funcs {[0]} \
        memories {[limits.i32/2 512 512]} \
        datacount {1} \
        code {[{[] \
            call 0 \
            i32.const 0 i32.const 0 i32.const %d32 memory.init 0 0 \
            i32.const %d32 i32.load8_u 0 0 \
        end}]} \
        data {[data.passive {%'00'}]}", (int32_t)init_size, (int32_t)(init_size - 1)));
    replace_data_segment_with_fill(&mod, 0, init_size, 0x5A);

    assert_ok(wah_exec_context_create(&ctx, &mod));
    assert_ok(wah_link_module(&ctx, "env", &env));
    assert_ok(wah_instantiate(&ctx));

    assert_ok(wah_start(&ctx, 1, NULL, 0));
    wah_error_t err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_STATUS_YIELDED);
    assert_eq_u32(ctx.memory_base[0], 0x5A);
    assert_eq_u32(ctx.memory_base[chunk - 1], 0x5A);
    assert_eq_u32(ctx.memory_base[chunk], 0x00);

    while ((err = wah_resume(&ctx)) > 0) {
    }
    assert_eq_i32(err, WAH_OK);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0x5A);
    assert_eq_u32(ctx.memory_base[init_size - 1], 0x5A);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
    wah_free_module(&env);
}

static void test_bulk_memory_copy_backward_large(void) {
    printf("Testing overlapping bulk memory.copy backward > 2^24 bytes with partial commit interrupt...\n");
    wah_module_t env = {0};
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    const uint32_t chunk = 1u << 24;
    const uint32_t copy_size = chunk * 2;
    const uint32_t dest = chunk / 2;

    assert_ok(wah_new_module(&env));
    assert_ok(wah_module_export_func(&env, "interrupt", "()", host_request_interrupt, NULL, NULL));

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32], fn [] []]} \
        imports {[ {'env'} {'interrupt'} fn# 1 ]} \
        funcs {[0]} \
        memories {[limits.i32/2 640 640]} \
        code {[{[] \
            call 0 \
            i32.const %d32 i32.const 0 i32.const %d32 memory.copy 0 0 \
            i32.const %d32 i32.load8_u 0 0 \
        end}]}", (int32_t)dest, (int32_t)copy_size, (int32_t)(dest + chunk - 1)));
    assert_ok(wah_exec_context_create(&ctx, &mod));
    assert_ok(wah_link_module(&ctx, "env", &env));
    assert_ok(wah_instantiate(&ctx));

    memset(ctx.memory_base, 0xA1, chunk);
    memset(ctx.memory_base + chunk, 0xB2, chunk);

    assert_ok(wah_start(&ctx, 1, NULL, 0));
    wah_error_t err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_STATUS_YIELDED);
    assert_eq_u32(ctx.memory_base[dest + chunk - 1], 0xB2);
    assert_eq_u32(ctx.memory_base[dest + chunk], 0xB2);
    assert_eq_u32(ctx.memory_base[dest + copy_size - 1], 0xB2);

    while ((err = wah_resume(&ctx)) > 0) {
    }
    assert_eq_i32(err, WAH_OK);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0xA1);
    assert_eq_u32(ctx.memory_base[dest + copy_size - 1], 0xB2);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
    wah_free_module(&env);
}

static void test_bulk_memory64_fill_large(void) {
    printf("Testing memory64 memory.fill > 2^24 bytes with actual chunk interrupt...\n");
    wah_module_t env = {0};
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    const uint32_t chunk = 1u << 24;
    const uint32_t fill_size = chunk * 2;

    assert_ok(wah_new_module(&env));
    assert_ok(wah_module_export_func(&env, "interrupt", "()", host_request_interrupt, NULL, NULL));

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32], fn [] []]} \
        imports {[ {'env'} {'interrupt'} fn# 1 ]} \
        funcs {[0]} \
        memories {[limits.i64/2 512 512]} \
        code {[{[] \
            call 0 \
            i64.const 0 i32.const 0xC3 i64.const %d64 memory.fill 0 \
            i64.const %d64 i32.load8_u 0 0 \
        end}]}", (int64_t)fill_size, (int64_t)(fill_size - 1)));
    assert_ok(wah_exec_context_create(&ctx, &mod));
    assert_ok(wah_link_module(&ctx, "env", &env));
    assert_ok(wah_instantiate(&ctx));

    assert_ok(wah_start(&ctx, 1, NULL, 0));
    wah_error_t err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_STATUS_YIELDED);
    assert_eq_u32(ctx.memory_base[0], 0xC3);
    assert_eq_u32(ctx.memory_base[chunk - 1], 0xC3);
    assert_eq_u32(ctx.memory_base[chunk], 0x00);

    while ((err = wah_resume(&ctx)) > 0) {
    }
    assert_eq_i32(err, WAH_OK);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0xC3);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
    wah_free_module(&env);
}

static void test_bulk_memory64_init_large(void) {
    printf("Testing memory64 memory.init > 2^24 bytes with partial commit interrupt...\n");
    wah_module_t env = {0};
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    const uint32_t chunk = 1u << 24;
    const uint32_t init_size = chunk * 2;

    assert_ok(wah_new_module(&env));
    assert_ok(wah_module_export_func(&env, "interrupt", "()", host_request_interrupt, NULL, NULL));

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32], fn [] []]} \
        imports {[ {'env'} {'interrupt'} fn# 1 ]} \
        funcs {[0]} \
        memories {[limits.i64/2 512 512]} \
        datacount {1} \
        code {[{[] \
            call 0 \
            i64.const 0 i32.const 0 i32.const %d32 memory.init 0 0 \
            i64.const %d64 i32.load8_u 0 0 \
        end}]} \
        data {[data.passive {%'00'}]}", (int32_t)init_size, (int64_t)(init_size - 1)));
    replace_data_segment_with_fill(&mod, 0, init_size, 0x6B);

    assert_ok(wah_exec_context_create(&ctx, &mod));
    assert_ok(wah_link_module(&ctx, "env", &env));
    assert_ok(wah_instantiate(&ctx));

    assert_ok(wah_start(&ctx, 1, NULL, 0));
    wah_error_t err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_STATUS_YIELDED);
    assert_eq_u32(ctx.memory_base[0], 0x6B);
    assert_eq_u32(ctx.memory_base[chunk - 1], 0x6B);
    assert_eq_u32(ctx.memory_base[chunk], 0x00);

    while ((err = wah_resume(&ctx)) > 0) {
    }
    assert_eq_i32(err, WAH_OK);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0x6B);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
    wah_free_module(&env);
}

static void test_bulk_memory64_copy_large(void) {
    printf("Testing memory64 memory.copy > 2^24 bytes with partial commit interrupt...\n");
    wah_module_t env = {0};
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    const uint32_t chunk = 1u << 24;
    const uint32_t copy_size = chunk * 2;
    const uint32_t dest = copy_size;

    assert_ok(wah_new_module(&env));
    assert_ok(wah_module_export_func(&env, "interrupt", "()", host_request_interrupt, NULL, NULL));

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32], fn [] []]} \
        imports {[ {'env'} {'interrupt'} fn# 1 ]} \
        funcs {[0]} \
        memories {[limits.i64/2 1024 1024]} \
        code {[{[] \
            call 0 \
            i64.const %d64 i64.const 0 i64.const %d64 memory.copy 0 0 \
            i64.const %d64 i32.load8_u 0 0 \
        end}]}", (int64_t)dest, (int64_t)copy_size, (int64_t)(dest + copy_size - 1)));
    assert_ok(wah_exec_context_create(&ctx, &mod));
    assert_ok(wah_link_module(&ctx, "env", &env));
    assert_ok(wah_instantiate(&ctx));

    memset(ctx.memory_base, 0xD4, copy_size);

    assert_ok(wah_start(&ctx, 1, NULL, 0));
    wah_error_t err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_STATUS_YIELDED);
    assert_eq_u32(ctx.memory_base[dest], 0xD4);
    assert_eq_u32(ctx.memory_base[dest + chunk - 1], 0xD4);
    assert_eq_u32(ctx.memory_base[dest + chunk], 0x00);

    while ((err = wah_resume(&ctx)) > 0) {
    }
    assert_eq_i32(err, WAH_OK);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0xD4);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
    wah_free_module(&env);
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
    test_resume_br_table();
    test_resume_tail_call_deep();
    test_resume_return_call_indirect();
    test_resume_exception_catch();
    test_resume_multi_value();
    test_resume_gc_funcref_on_stack();
    test_resume_gc_ref_across_calls();
    test_cancel_with_live_refs();
    test_destroy_with_live_refs();
    test_cancel_with_pending_exception();
    test_resume_gc_ref_global();
    test_poll_yield_without_fuel();
    test_bulk_table_fill_interrupt();
    test_bulk_memory_fill_interrupt();
    test_bulk_memory_copy_interrupt();
    test_bulk_memory_fill_large();
    test_bulk_memory_copy_large();
    test_bulk_memory_init_large();
    test_bulk_memory_copy_backward_large();
    test_bulk_memory64_fill_large();
    test_bulk_memory64_init_large();
    test_bulk_memory64_copy_large();

    printf("\n=== All resume tests passed ===\n");
    return 0;
}
