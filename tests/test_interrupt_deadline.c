#include <stdint.h>
#include <stdio.h>
#include "../wah.h"
#include "wah_impl.h"
#include "common.h"

#if defined(_WIN32)
#include <windows.h>
static void test_sleep_ms(unsigned ms) { Sleep(ms); }
#else
#include <time.h>
static void test_sleep_ms(unsigned ms) {
    struct timespec ts;
    ts.tv_sec = (time_t)(ms / 1000);
    ts.tv_nsec = (long)(ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}
#endif

static const wah_parse_options_t fuel_opts = {
    .features = WAH_FEATURE_ALL,
    .enable_fuel_metering = true,
};

static void test_interrupt_without_gc(void) {
    printf("Testing interrupt without GC...\n");
    fflush(stdout);
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_parse_module_from_spec_ex(&mod, &fuel_opts, "wasm \
        types {[fn [] []]} funcs {[0]} \
        code {[{[] loop void br 0 end end}]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_start(&ctx, 0, NULL, 0));

    assert_false(wah_is_interrupted(&ctx));
    wah_request_interrupt(&ctx);
    assert_true(wah_is_interrupted(&ctx));
    assert_err(wah_resume(&ctx), WAH_STATUS_YIELDED);
    assert_false(wah_is_interrupted(&ctx));
    assert_true(wah_is_suspended(&ctx));

    wah_cancel(&ctx);
    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void host_observe_interrupt(wah_call_context_t *call, void *userdata) {
    (void)userdata;
    wah_result_i32(call, 0, wah_debug_is_interrupted_from_host(call) ? 1 : 0);
}

static void test_is_interrupted_in_host_function(void) {
    printf("Testing wah_is_interrupted in host function...\n");
    fflush(stdout);
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};
    wah_value_t result = {0};

    assert_ok(wah_new_module(&mod, NULL));
    assert_ok(wah_module_export_func(&mod, "observe", "() -> i32", host_observe_interrupt, NULL, NULL));
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_request_interrupt(&ctx);
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 1);
    assert_true(wah_is_interrupted(&ctx));
    wah_cancel(&ctx);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_cancel_consumes_interrupt(void) {
    printf("Testing cancel consumes interrupt...\n");
    fflush(stdout);
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_parse_module_from_spec_ex(&mod, &fuel_opts, "wasm \
        types {[fn [] []]} funcs {[0]} \
        code {[{[] loop void br 0 end end}]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_start(&ctx, 0, NULL, 0));
    wah_request_interrupt(&ctx);
    assert_true(wah_is_interrupted(&ctx));
    wah_cancel(&ctx);
    assert_false(wah_is_interrupted(&ctx));

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_interrupt_and_fuel_priority(void) {
    printf("Testing fuel priority over interrupt at TICK...\n");
    fflush(stdout);
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_parse_module_from_spec_ex(&mod, &fuel_opts, "wasm \
        types {[fn [] []]} funcs {[0]} \
        code {[{[] nop end}]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_start(&ctx, 0, NULL, 0));
    wah_set_fuel(&ctx, 0);
    wah_request_interrupt(&ctx);
    assert_err(wah_resume(&ctx), WAH_STATUS_FUEL_EXHAUSTED);
    assert_true(wah_is_interrupted(&ctx));
    wah_cancel(&ctx);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_short_deadline_yields_and_rearms(void) {
    printf("Testing short deadline yields and rearms...\n");
    fflush(stdout);
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};
    wah_exec_options_t options = {0};
    options.limits.deadline_us = 10000;

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] []]} funcs {[0]} \
        code {[{[] loop void br 0 end end}]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod, &options));
    assert_ok(wah_start(&ctx, 0, NULL, 0));
    assert_err(wah_resume(&ctx), WAH_STATUS_YIELDED);
    assert_true(wah_is_suspended(&ctx));
    assert_err(wah_resume(&ctx), WAH_STATUS_YIELDED);
    wah_cancel(&ctx);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_fast_call_disarms_deadline(void) {
    printf("Testing fast call disarms deadline...\n");
    fflush(stdout);
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};
    wah_exec_options_t options = {0};
    options.limits.deadline_us = 50000;

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32]]} funcs {[0]} \
        code {[{[] i32.const 42 end}]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod, &options));

    wah_value_t result = {0};
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);
    test_sleep_ms(100);
    assert_false(wah_is_interrupted(&ctx));

    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_destroy_while_deadline_armed(void) {
    printf("Testing destroy while deadline timer is armed...\n");
    fflush(stdout);
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};
    wah_exec_options_t options = {0};
    options.limits.deadline_us = 5000000;

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] []]} funcs {[0]} \
        code {[{[] loop void br 0 end end}]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod, &options));
    assert_ok(wah_start(&ctx, 0, NULL, 0));
    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

int main(void) {
    test_interrupt_without_gc();
    test_is_interrupted_in_host_function();
    test_cancel_consumes_interrupt();
    test_interrupt_and_fuel_priority();
    test_short_deadline_yields_and_rearms();
    test_fast_call_disarms_deadline();
    test_destroy_while_deadline_armed();

    printf("\n=== All interrupt/deadline tests passed ===\n");
    return 0;
}
