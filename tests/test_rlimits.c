#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../wah.h"
#include "common.h"

static void test_default_rlimits(void) {
    printf("Testing default rlimits...\n");
    wah_rlimits_t lim = wah_default_rlimits();
    assert_eq_u64(lim.max_stack_bytes, 0);
    assert_eq_u64(lim.max_memory_bytes, 0);
    assert_eq_u64(lim.fuel, 0);
    assert_eq_u64(lim.epoch_deadline, 0);
}

static void test_create_with_limits(void) {
    printf("Testing create_with_limits...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [i32, i32] [i32]]} funcs {[0]} \
        code {[{[] local.get 0 local.get 1 i32.add end}]}"));

    // Create with default limits
    wah_rlimits_t lim = wah_default_rlimits();
    assert_ok(wah_exec_context_create_with_limits(&ctx, &mod, &lim));

    wah_rlimits_t out;
    wah_exec_context_get_limits(&ctx, &out);
    assert_true(out.max_stack_bytes > 0);

    wah_value_t params[2] = { {.i32 = 10}, {.i32 = 20} };
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, params, 2, &result));
    assert_eq_i32(result.i32, 30);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_set_limits(void) {
    printf("Testing set_limits...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_parse_module_from_spec(&mod,
        "wasm types {[fn [] []]} funcs {[0]} "
        "code {[{[] end}]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod));

    wah_rlimits_t lim = { .max_stack_bytes = 4096 };
    assert_ok(wah_exec_context_set_limits(&ctx, &lim));

    wah_rlimits_t out;
    wah_exec_context_get_limits(&ctx, &out);
    assert_eq_u64(out.max_stack_bytes, 4096);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_set_limits_rejects_while_suspended(void) {
    printf("Testing set_limits rejects while suspended...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // Simple function that we start but don't resume fully
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32]]} funcs {[0]} \
        code {[{[] i32.const 42 end}]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod));

    // wah_start sets up the frame; context is now SUSPENDED
    assert_ok(wah_start(&ctx, 0, NULL, 0));
    assert_true(wah_is_suspended(&ctx));

    // set_limits should fail because state != READY
    wah_rlimits_t lim = { .max_stack_bytes = 4096 };
    assert_err(wah_exec_context_set_limits(&ctx, &lim), WAH_ERROR_MISUSE);

    wah_cancel(&ctx);
    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// Deep recursion: func calls itself until stack overflow.
// func 0: call 0; end
static void test_deep_recursion_overflow(void) {
    printf("Testing deep recursion stack overflow...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] []]} funcs {[0]} \
        code {[{[] call 0 end}]}"));

    // Small stack: ~4KB. Each frame is ~96 bytes (64-bit), so ~40 frames max.
    wah_rlimits_t lim = { .max_stack_bytes = 4096 };
    assert_ok(wah_exec_context_create_with_limits(&ctx, &mod, &lim));

    assert_err(wah_call(&ctx, 0, NULL, 0, NULL), WAH_ERROR_STACK_OVERFLOW);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// Deep recursion with default stack should also eventually overflow.
static void test_deep_recursion_default_stack(void) {
    printf("Testing deep recursion with default stack...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] []]} funcs {[0]} \
        code {[{[] call 0 end}]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod));

    assert_err(wah_call(&ctx, 0, NULL, 0, NULL), WAH_ERROR_STACK_OVERFLOW);

    // Context should be usable after overflow
    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// Wide operand stack: function pushes many values (large max_stack_depth)
// with a small stack budget → preflight catches overflow at call entry.
static void test_wide_operand_stack_preflight(void) {
    printf("Testing wide operand stack preflight...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // Function that pushes 16 i32 consts, adds them all, returns i32.
    // max_stack_depth = 16, local_count = 0, so max_frame_slots = 16.
    // Each value slot = 16 bytes, so 16 slots = 256 bytes for values alone.
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32]]} funcs {[0]} \
        code {[{[] \
            i32.const 1 i32.const 1 i32.const 1 i32.const 1 \
            i32.const 1 i32.const 1 i32.const 1 i32.const 1 \
            i32.const 1 i32.const 1 i32.const 1 i32.const 1 \
            i32.const 1 i32.const 1 i32.const 1 i32.const 1 \
            i32.add i32.add i32.add i32.add i32.add i32.add i32.add \
            i32.add i32.add i32.add i32.add i32.add i32.add i32.add i32.add \
        end}]}"));

    // Need 1 frame (~96 bytes) + 16 value slots (256 bytes) = ~352 bytes min.
    // Use 256 bytes: not enough.
    wah_rlimits_t lim = { .max_stack_bytes = 256 };
    assert_ok(wah_exec_context_create_with_limits(&ctx, &mod, &lim));

    wah_value_t result;
    assert_err(wah_call(&ctx, 0, NULL, 0, &result), WAH_ERROR_STACK_OVERFLOW);

    wah_exec_context_destroy(&ctx);

    // With enough stack, it should succeed.
    lim.max_stack_bytes = 4096;
    assert_ok(wah_exec_context_create_with_limits(&ctx, &mod, &lim));

    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 16);

    wah_exec_context_destroy(&ctx);
}

// Mixed: moderate recursion + moderate locals.
// func(i32) -> i32: 4 locals, call self recursively.
static void test_mixed_recursion_and_locals(void) {
    printf("Testing mixed recursion + locals...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [i32] [i32]]} funcs {[0]} \
        code {[{[1 i32] \
            local.get 0 \
            if i32 \
                local.get 0 i32.const 1 i32.sub \
                call 0 \
                i32.const 1 i32.add \
            else \
                i32.const 0 \
            end \
        end}]}"));

    // Large enough stack for moderate depth
    wah_rlimits_t lim = { .max_stack_bytes = 16384 };
    assert_ok(wah_exec_context_create_with_limits(&ctx, &mod, &lim));

    wah_value_t params = { .i32 = 5 };
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, &params, 1, &result));
    assert_eq_i32(result.i32, 5);

    // Deep enough to overflow
    params.i32 = 10000;
    assert_err(wah_call(&ctx, 0, &params, 1, &result), WAH_ERROR_STACK_OVERFLOW);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// Recovery: after stack overflow, the context should be reusable.
static void test_recovery_after_overflow(void) {
    printf("Testing recovery after stack overflow...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [i32] [i32]]} funcs {[0]} \
        code {[{[] \
            local.get 0 i32.eqz \
            if i32 \
                i32.const 42 \
            else \
                local.get 0 i32.const 1 i32.sub call 0 \
            end \
        end}]}"));

    wah_rlimits_t lim = { .max_stack_bytes = 8192 };
    assert_ok(wah_exec_context_create_with_limits(&ctx, &mod, &lim));

    // Overflow
    wah_value_t params = { .i32 = 100000 };
    wah_value_t result;
    assert_err(wah_call(&ctx, 0, &params, 1, &result), WAH_ERROR_STACK_OVERFLOW);

    // Recover: small depth should work
    params.i32 = 3;
    assert_ok(wah_call(&ctx, 0, &params, 1, &result));
    assert_eq_i32(result.i32, 42);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

int main(void) {
    test_default_rlimits();
    test_create_with_limits();
    test_set_limits();
    test_set_limits_rejects_while_suspended();
    test_deep_recursion_overflow();
    test_deep_recursion_default_stack();
    test_wide_operand_stack_preflight();
    test_mixed_recursion_and_locals();
    test_recovery_after_overflow();

    printf("\n--- All rlimits tests passed ---\n");
    return 0;
}
