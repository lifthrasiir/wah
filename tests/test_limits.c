#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../wah.h"
#include "common.h"

static void test_create_with_limits(void) {
    printf("Testing create_with_limits...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [i32, i32] [i32]]} funcs {[0]} \
        code {[{[] local.get 0 local.get 1 i32.add end}]}"));

    // Create with default limits
    wah_exec_options_t options = { .limits = {0} };
    assert_ok(wah_new_exec_context(&ctx, &mod, &options));

    wah_limits_t out;
    wah_get_limits(&ctx, &out);
    assert_true(out.max_stack_bytes > 0);

    wah_value_t params[2] = { {.i32 = 10}, {.i32 = 20} };
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, params, 2, &result));
    assert_eq_i32(result.i32, 30);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_set_limits(void) {
    printf("Testing set_limits...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_parse_module_from_spec(&mod,
        "wasm types {[fn [] []]} funcs {[0]} "
        "code {[{[] end}]}"));
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));

    wah_limits_t lim = { .max_stack_bytes = 4096 };
    assert_ok(wah_set_limits(&ctx, &lim));

    wah_limits_t out;
    wah_get_limits(&ctx, &out);
    assert_eq_u64(out.max_stack_bytes, 4096);

    wah_free_exec_context(&ctx);
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
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));

    // wah_start sets up the frame; context is now SUSPENDED
    assert_ok(wah_start(&ctx, 0, NULL, 0));
    assert_true(wah_is_suspended(&ctx));

    // set_limits should fail because state != READY
    wah_limits_t lim = { .max_stack_bytes = 4096 };
    assert_err(wah_set_limits(&ctx, &lim), WAH_ERROR_MISUSE);

    wah_cancel(&ctx);
    wah_free_exec_context(&ctx);
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
    wah_exec_options_t options = { .limits = { .max_stack_bytes = 4096 } };
    assert_ok(wah_new_exec_context(&ctx, &mod, &options));

    assert_err(wah_call(&ctx, 0, NULL, 0, NULL), WAH_ERROR_STACK_OVERFLOW);

    wah_free_exec_context(&ctx);
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
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));

    assert_err(wah_call(&ctx, 0, NULL, 0, NULL), WAH_ERROR_STACK_OVERFLOW);

    // Context should be usable after overflow
    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

// Wide operand stack: function pushes many values (large max_stack_depth)
// with a small stack budget -> preflight catches overflow at call entry.
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
    wah_exec_options_t options = { .limits = { .max_stack_bytes = 256 } };
    assert_ok(wah_new_exec_context(&ctx, &mod, &options));

    wah_value_t result;
    assert_err(wah_call(&ctx, 0, NULL, 0, &result), WAH_ERROR_STACK_OVERFLOW);

    wah_free_exec_context(&ctx);

    // With enough stack, it should succeed.
    options.limits.max_stack_bytes = 4096;
    assert_ok(wah_new_exec_context(&ctx, &mod, &options));

    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 16);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
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
    wah_exec_options_t options = { .limits = { .max_stack_bytes = 16384 } };
    assert_ok(wah_new_exec_context(&ctx, &mod, &options));

    wah_value_t params = { .i32 = 5 };
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, &params, 1, &result));
    assert_eq_i32(result.i32, 5);

    // Deep enough to overflow
    params.i32 = 10000;
    assert_err(wah_call(&ctx, 0, &params, 1, &result), WAH_ERROR_STACK_OVERFLOW);

    wah_free_exec_context(&ctx);
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

    wah_exec_options_t options = { .limits = { .max_stack_bytes = 8192 } };
    assert_ok(wah_new_exec_context(&ctx, &mod, &options));

    // Overflow
    wah_value_t params = { .i32 = 100000 };
    wah_value_t result;
    assert_err(wah_call(&ctx, 0, &params, 1, &result), WAH_ERROR_STACK_OVERFLOW);

    // Recover: small depth should work
    params.i32 = 3;
    assert_ok(wah_call(&ctx, 0, &params, 1, &result));
    assert_eq_i32(result.i32, 42);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

// --- Phase 2: Runtime memory budget ---

#define PAGE_SIZE 65536

static void test_memory_budget_initial_reject(void) {
    printf("Testing memory budget rejects module whose min pages exceed budget...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32]]} funcs {[0]} \
        memories {[limits.i32/2 1 4]} \
        code {[{[] memory.size 0 end}]}"));

    wah_exec_options_t options = { .limits = { .max_memory_bytes = PAGE_SIZE - 1 } };
    assert_err(wah_new_exec_context(&ctx, &mod, &options), WAH_ERROR_TOO_LARGE);

    options.limits.max_memory_bytes = PAGE_SIZE;
    assert_ok(wah_new_exec_context(&ctx, &mod, &options));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 1);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_memory_budget_table_initial(void) {
    printf("Testing memory budget charges initial table allocation...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32]]} funcs {[0]} \
        tables {[funcref limits.i32/2 10 100]} \
        code {[{[] table.size 0 end}]}"));

    uint64_t table_bytes = 10 * sizeof(wah_value_t);

    wah_exec_options_t options = { .limits = { .max_memory_bytes = table_bytes - 1 } };
    assert_err(wah_new_exec_context(&ctx, &mod, &options), WAH_ERROR_TOO_LARGE);

    options.limits.max_memory_bytes = table_bytes;
    assert_ok(wah_new_exec_context(&ctx, &mod, &options));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 10);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_memory_budget_combined(void) {
    printf("Testing memory budget covers memory + table total...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32]]} funcs {[0]} \
        tables {[funcref limits.i32/2 10 100]} \
        memories {[limits.i32/2 1 4]} \
        code {[{[] memory.size 0 end}]}"));

    uint64_t table_bytes = 10 * sizeof(wah_value_t);
    uint64_t total_needed = PAGE_SIZE + table_bytes;

    wah_exec_options_t options = { .limits = { .max_memory_bytes = PAGE_SIZE } };
    assert_err(wah_new_exec_context(&ctx, &mod, &options), WAH_ERROR_TOO_LARGE);

    options.limits.max_memory_bytes = total_needed;
    assert_ok(wah_new_exec_context(&ctx, &mod, &options));

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_memory_grow_budget(void) {
    printf("Testing memory.grow respects memory budget...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [i32] [i32], fn [] [i32]]} \
        funcs {[0, 1]} \
        memories {[limits.i32/2 1 10]} \
        code {[ \
            {[] local.get 0 memory.grow 0 end}, \
            {[] memory.size 0 end} \
        ]}"));

    wah_exec_options_t options = { .limits = { .max_memory_bytes = 2 * PAGE_SIZE } };
    assert_ok(wah_new_exec_context(&ctx, &mod, &options));

    wah_value_t params, result;

    params.i32 = 1;
    assert_ok(wah_call(&ctx, 0, &params, 1, &result));
    assert_eq_i32(result.i32, 1);

    params.i32 = 1;
    assert_ok(wah_call(&ctx, 0, &params, 1, &result));
    assert_eq_i32(result.i32, -1);

    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 2);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_table_grow_budget(void) {
    printf("Testing table.grow respects memory budget...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [i32] [i32], fn [] [i32]]} \
        funcs {[0, 1]} \
        tables {[funcref limits.i32/2 2 1000]} \
        code {[ \
            {[] ref.null funcref local.get 0 table.grow 0 end}, \
            {[] table.size 0 end} \
        ]}"));

    uint64_t init_bytes = 2 * sizeof(wah_value_t);
    uint64_t grow_bytes = 3 * sizeof(wah_value_t);

    wah_exec_options_t options = { .limits = { .max_memory_bytes = init_bytes + grow_bytes } };
    assert_ok(wah_new_exec_context(&ctx, &mod, &options));

    wah_value_t params, result;

    params.i32 = 3;
    assert_ok(wah_call(&ctx, 0, &params, 1, &result));
    assert_eq_i32(result.i32, 2);

    params.i32 = 1;
    assert_ok(wah_call(&ctx, 0, &params, 1, &result));
    assert_eq_i32(result.i32, -1);

    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 5);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_memory_grow_returns_neg1_not_trap(void) {
    printf("Testing memory.grow returns -1 on budget failure, not trap...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32]]} funcs {[0]} \
        memories {[limits.i32/2 1 10]} \
        code {[{[] i32.const 5 memory.grow 0 end}]}"));

    wah_exec_options_t options = { .limits = { .max_memory_bytes = PAGE_SIZE } };
    assert_ok(wah_new_exec_context(&ctx, &mod, &options));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, -1);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_no_memory_bytes(void) {
    printf("Testing no_memory_bytes enforces 0-byte limit...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // 0-page initial memory
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [i32] [i32]]} funcs {[0]} \
        memories {[limits.i32/2 0 10]} \
        code {[{[] local.get 0 memory.grow 0 end}]}"));

    wah_exec_options_t options = { .limits = { .no_memory_bytes = true } };
    assert_ok(wah_new_exec_context(&ctx, &mod, &options));

    wah_value_t params = { .i32 = 1 };
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, &params, 1, &result));
    assert_eq_i32(result.i32, -1);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_no_memory_bytes_rejects_with_max(void) {
    printf("Testing no_memory_bytes + max_memory_bytes > 0 is an error...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] []]} funcs {[0]} \
        code {[{[] end}]}"));

    wah_exec_options_t options = { .limits = { .no_memory_bytes = true, .max_memory_bytes = 100 } };
    assert_err(wah_new_exec_context(&ctx, &mod, &options), WAH_ERROR_MISUSE);

    wah_free_module(&mod);
}

static void test_no_memory_bytes_rejects_initial(void) {
    printf("Testing no_memory_bytes rejects module with initial memory...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] []]} funcs {[0]} \
        memories {[limits.i32/2 1 10]} \
        code {[{[] end}]}"));

    wah_exec_options_t options = { .limits = { .no_memory_bytes = true } };
    assert_err(wah_new_exec_context(&ctx, &mod, &options), WAH_ERROR_TOO_LARGE);

    wah_free_module(&mod);
}

static void test_zero_budget_default(void) {
    printf("Testing 0 budget (default) means no limit...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [i32] [i32]]} funcs {[0]} \
        memories {[limits.i32/2 1 100]} \
        code {[{[] local.get 0 memory.grow 0 end}]}"));

    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));

    wah_value_t params = { .i32 = 10 };
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, &params, 1, &result));
    assert_eq_i32(result.i32, 1);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_set_limits_memory_budget(void) {
    printf("Testing set_limits for memory budget...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [i32] [i32]]} funcs {[0]} \
        memories {[limits.i32/2 1 10]} \
        code {[{[] local.get 0 memory.grow 0 end}]}"));

    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));

    wah_limits_t lim = { .max_memory_bytes = 2 * PAGE_SIZE };
    assert_ok(wah_set_limits(&ctx, &lim));

    wah_value_t params, result;

    params.i32 = 1;
    assert_ok(wah_call(&ctx, 0, &params, 1, &result));
    assert_eq_i32(result.i32, 1);

    params.i32 = 1;
    assert_ok(wah_call(&ctx, 0, &params, 1, &result));
    assert_eq_i32(result.i32, -1);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_set_limits_rejects_below_committed(void) {
    printf("Testing set_limits rejects budget below committed bytes...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] []]} funcs {[0]} \
        memories {[limits.i32/2 2 10]} \
        code {[{[] end}]}"));

    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));

    wah_limits_t lim = { .max_memory_bytes = PAGE_SIZE };
    assert_err(wah_set_limits(&ctx, &lim), WAH_ERROR_TOO_LARGE);

    lim.max_memory_bytes = 2 * PAGE_SIZE;
    assert_ok(wah_set_limits(&ctx, &lim));

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_get_limits_reports_memory_budget(void) {
    printf("Testing get_limits reports memory budget...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] []]} funcs {[0]} \
        code {[{[] end}]}"));

    wah_exec_options_t options = { .limits = { .max_memory_bytes = 1024 * 1024 } };
    assert_ok(wah_new_exec_context(&ctx, &mod, &options));

    wah_limits_t out;
    wah_get_limits(&ctx, &out);
    assert_eq_u64(out.max_memory_bytes, 1024 * 1024);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_zero_page_memory_budget(void) {
    printf("Testing module with 0-page memory respects budget...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [i32] [i32]]} funcs {[0]} \
        memories {[limits.i32/2 0 10]} \
        code {[{[] local.get 0 memory.grow 0 end}]}"));

    wah_exec_options_t options = { .limits = { .max_memory_bytes = 1 } };
    assert_ok(wah_new_exec_context(&ctx, &mod, &options));

    wah_value_t params = { .i32 = 1 };
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, &params, 1, &result));
    assert_eq_i32(result.i32, -1);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_imported_memory_budget(void) {
    printf("Testing imported memory charges to importing context budget...\n");
    wah_module_t provider = {0};
    wah_module_t consumer = {0};
    wah_exec_context_t pctx = {0};
    wah_exec_context_t cctx = {0};

    assert_ok(wah_parse_module_from_spec(&provider, "wasm \
        types {[fn [] []]} funcs {[0]} \
        memories {[limits.i32/2 1 10]} \
        exports {[{'mem'} mem# 0]} \
        code {[{[] end}]}"));

    assert_ok(wah_parse_module_from_spec(&consumer, "wasm \
        types {[fn [i32] [i32]]} \
        imports {[{'provider'} {'mem'} export.memory limits.i32/1 1]} \
        funcs {[0]} \
        code {[{[] local.get 0 memory.grow 0 end}]}"));

    assert_ok(wah_new_exec_context(&pctx, &provider, NULL));

    wah_exec_options_t options = { .limits = { .max_memory_bytes = 2 * PAGE_SIZE } };
    assert_ok(wah_new_exec_context(&cctx, &consumer, &options));
    assert_ok(wah_link_module(&cctx, "provider", &provider));
    assert_ok(wah_instantiate(&cctx));

    wah_value_t params, result;

    params.i32 = 1;
    assert_ok(wah_call(&cctx, 0, &params, 1, &result));
    assert_eq_i32(result.i32, 1);

    params.i32 = 1;
    assert_ok(wah_call(&cctx, 0, &params, 1, &result));
    assert_eq_i32(result.i32, -1);

    wah_free_exec_context(&cctx);
    wah_free_exec_context(&pctx);
    wah_free_module(&consumer);
    wah_free_module(&provider);
}

// --- Phase 3: Fuel connection ---

static const wah_parse_options_t fuel_opts = { .features = WAH_FEATURE_ALL, .enable_fuel_metering = true };

#define PARSE_FUEL(mod, spec) \
    assert_ok(wah_parse_module_from_spec_ex((mod), &fuel_opts, (spec)))

static void test_fuel_via_limits(void) {
    printf("Testing fuel set through limits...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32] [i32]]} funcs {[0]} \
        code {[{[] local.get 0 local.get 1 i32.add end}]}");

    wah_exec_options_t options = { .limits = { .fuel = 100 } };
    assert_ok(wah_new_exec_context(&ctx, &mod, &options));

    assert_eq_i64(wah_get_fuel(&ctx), 100);

    wah_value_t params[2] = { {.i32 = 10}, {.i32 = 20} };
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, params, 2, &result));
    assert_eq_i32(result.i32, 30);
    assert_eq_i64(wah_get_fuel(&ctx), 96);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_fuel_via_limits_exhaustion(void) {
    printf("Testing fuel exhaustion via limits...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32] [i32]]} funcs {[0]} \
        code {[{[] local.get 0 local.get 1 i32.add end}]}");

    wah_exec_options_t options = { .limits = { .fuel = 3 } };
    assert_ok(wah_new_exec_context(&ctx, &mod, &options));

    wah_value_t params[2] = { {.i32 = 10}, {.i32 = 20} };
    wah_value_t result;
    assert_err(wah_call(&ctx, 0, params, 2, &result), WAH_STATUS_FUEL_EXHAUSTED);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_fuel_zero_limit_means_default(void) {
    printf("Testing fuel=0 in limits means default (no limit)...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32] [i32]]} funcs {[0]} \
        code {[{[] local.get 0 local.get 1 i32.add end}]}");

    wah_exec_options_t options = { .limits = { .fuel = 0 } };
    assert_ok(wah_new_exec_context(&ctx, &mod, &options));
    assert_eq_i64(wah_get_fuel(&ctx), INT64_MAX);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_fuel_refuel_via_set_limits(void) {
    printf("Testing refueling via set_limits...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32] [i32]]} funcs {[0]} \
        code {[{[] local.get 0 local.get 1 i32.add end}]}");

    wah_exec_options_t options = { .limits = { .fuel = 10 } };
    assert_ok(wah_new_exec_context(&ctx, &mod, &options));

    wah_value_t params[2] = { {.i32 = 1}, {.i32 = 2} };
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, params, 2, &result));
    assert_eq_i64(wah_get_fuel(&ctx), 6);

    wah_limits_t lim = { .fuel = 100 };
    assert_ok(wah_set_limits(&ctx, &lim));
    assert_eq_i64(wah_get_fuel(&ctx), 100);

    assert_ok(wah_call(&ctx, 0, params, 2, &result));
    assert_eq_i64(wah_get_fuel(&ctx), 96);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_fuel_resume_via_limits(void) {
    printf("Testing fuel resume after exhaustion via limits refuel...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32] [i32]]} funcs {[0]} \
        code {[{[] \
            block void \
                loop void \
                    local.get 0 local.get 1 i32.ge_u br_if 1 \
                    local.get 0 i32.const 1 i32.add local.set 0 \
                    br 0 \
                end \
            end \
            local.get 0 \
        end}]}");

    wah_exec_options_t options = { .limits = { .fuel = 5 } };
    assert_ok(wah_new_exec_context(&ctx, &mod, &options));

    wah_value_t params[2] = { {.i32 = 0}, {.i32 = 3} };
    assert_ok(wah_start(&ctx, 0, params, 2));

    int suspensions = 0;
    wah_error_t err;
    while ((err = wah_resume(&ctx)) == WAH_STATUS_FUEL_EXHAUSTED) {
        suspensions++;
        assert_ok(wah_set_fuel(&ctx, 5));
    }
    assert_ok(err);
    assert_true(suspensions > 0);

    wah_value_t result;
    assert_ok(wah_finish(&ctx, &result, 1, NULL));
    assert_eq_i32(result.i32, 3);

    printf("  loop(3) completed after %d fuel refills\n", suspensions);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_get_limits_reports_fuel(void) {
    printf("Testing get_limits reports current fuel...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32] [i32]]} funcs {[0]} \
        code {[{[] local.get 0 local.get 1 i32.add end}]}");

    wah_exec_options_t options = { .limits = { .fuel = 50 } };
    assert_ok(wah_new_exec_context(&ctx, &mod, &options));

    wah_limits_t out;
    wah_get_limits(&ctx, &out);
    assert_eq_u64(out.fuel, 50);

    wah_value_t params[2] = { {.i32 = 1}, {.i32 = 2} };
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, params, 2, &result));

    wah_get_limits(&ctx, &out);
    assert_eq_u64(out.fuel, 46);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_set_limits_partial(void) {
    printf("Testing set_limits changes only non-default fields...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32] [i32]]} funcs {[0]} \
        memories {[limits.i32/2 1 10]} \
        code {[{[] local.get 0 local.get 1 i32.add end}]}");

    wah_exec_options_t options = { .limits = { .max_stack_bytes = 8192, .max_memory_bytes = 4 * PAGE_SIZE, .fuel = 200 } };
    assert_ok(wah_new_exec_context(&ctx, &mod, &options));

    wah_limits_t before;
    wah_get_limits(&ctx, &before);
    assert_eq_u64(before.max_stack_bytes, 8192);
    assert_eq_u64(before.max_memory_bytes, 4 * PAGE_SIZE);
    assert_eq_u64(before.fuel, 200);

    // Set only fuel -- stack and memory should not change
    wah_limits_t fuel_only = { .fuel = 500 };
    assert_ok(wah_set_limits(&ctx, &fuel_only));

    wah_limits_t after;
    wah_get_limits(&ctx, &after);
    assert_eq_u64(after.max_stack_bytes, 8192);
    assert_eq_u64(after.max_memory_bytes, 4 * PAGE_SIZE);
    assert_eq_u64(after.fuel, 500);

    // Set only memory -- stack and fuel should not change
    wah_limits_t mem_only = { .max_memory_bytes = 2 * PAGE_SIZE };
    assert_ok(wah_set_limits(&ctx, &mem_only));

    wah_get_limits(&ctx, &after);
    assert_eq_u64(after.max_stack_bytes, 8192);
    assert_eq_u64(after.max_memory_bytes, 2 * PAGE_SIZE);
    assert_eq_u64(after.fuel, 500);

    // Set only stack -- memory and fuel should not change
    wah_limits_t stack_only = { .max_stack_bytes = 16384 };
    assert_ok(wah_set_limits(&ctx, &stack_only));

    wah_get_limits(&ctx, &after);
    assert_eq_u64(after.max_stack_bytes, 16384);
    assert_eq_u64(after.max_memory_bytes, 2 * PAGE_SIZE);
    assert_eq_u64(after.fuel, 500);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

int main(void) {
    // Phase 1: Stack limits
    test_create_with_limits();
    test_set_limits();
    test_set_limits_partial();
    test_set_limits_rejects_while_suspended();
    test_deep_recursion_overflow();
    test_deep_recursion_default_stack();
    test_wide_operand_stack_preflight();
    test_mixed_recursion_and_locals();
    test_recovery_after_overflow();

    // Phase 2: Runtime memory budget
    test_memory_budget_initial_reject();
    test_memory_budget_table_initial();
    test_memory_budget_combined();
    test_memory_grow_budget();
    test_table_grow_budget();
    test_memory_grow_returns_neg1_not_trap();
    test_no_memory_bytes();
    test_no_memory_bytes_rejects_with_max();
    test_no_memory_bytes_rejects_initial();
    test_zero_budget_default();
    test_set_limits_memory_budget();
    test_set_limits_rejects_below_committed();
    test_get_limits_reports_memory_budget();
    test_zero_page_memory_budget();
    test_imported_memory_budget();

    // Phase 3: Fuel connection
    test_fuel_via_limits();
    test_fuel_via_limits_exhaustion();
    test_fuel_zero_limit_means_default();
    test_fuel_refuel_via_set_limits();
    test_fuel_resume_via_limits();
    test_get_limits_reports_fuel();

    printf("\n--- All limits tests passed ---\n");
    return 0;
}
