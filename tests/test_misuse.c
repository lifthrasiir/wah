#include <stdio.h>
#include "../wah.h"
#include "common.h"

static void test_exec_context_misuse(void) {
    printf("Testing exec context misuse handling...\n");

    wah_module_t module = {0};
    wah_exec_context_t ctx = {0};
    assert_ok(wah_parse_module_from_spec(&module, "wasm"));

    assert_err(wah_new_exec_context(NULL, &module, NULL), WAH_ERROR_MISUSE);
    assert_err(wah_new_exec_context(&ctx, NULL, NULL), WAH_ERROR_MISUSE);

    assert_ok(wah_new_exec_context(&ctx, &module, NULL));
    assert_err(wah_set_limits(NULL, &(wah_limits_t){0}), WAH_ERROR_MISUSE);
    assert_err(wah_set_limits(&ctx, NULL), WAH_ERROR_MISUSE);

    wah_limits_t out = { .max_stack_bytes = 123, .max_memory_bytes = 456, .fuel = 789 };
    wah_get_limits(NULL, &out);
    assert_eq_u64(out.max_stack_bytes, 0);
    assert_eq_u64(out.max_memory_bytes, 0);
    assert_eq_u64(out.fuel, 0);
    wah_get_limits(&ctx, NULL);

    wah_free_exec_context(&ctx);
    wah_free_module(&module);
}

static void test_fuel_misuse(void) {
    printf("Testing fuel misuse handling...\n");

    wah_module_t module = {0};
    wah_exec_context_t ctx = {0};
    assert_ok(wah_parse_module_from_spec(&module, "wasm"));

    assert_err(wah_set_fuel(NULL, 1), WAH_ERROR_MISUSE);
    assert_eq_i64(wah_get_fuel(NULL), 0);

    assert_ok(wah_new_exec_context(&ctx, &module, NULL));
    assert_err(wah_set_fuel(&ctx, 1), WAH_ERROR_DISABLED_FEATURE);

    wah_free_exec_context(&ctx);
    wah_free_module(&module);
}

static void test_gc_misuse(void) {
    printf("Testing GC misuse handling...\n");

    wah_gc_heap_stats_t stats = { .object_count = 42, .allocated_bytes = 99 };
    assert_err(wah_gc_start(NULL), WAH_ERROR_MISUSE);
    wah_gc_heap_stats(NULL, &stats);
    assert_eq_u32(stats.object_count, 0);
    assert_eq_u64(stats.allocated_bytes, 0);
    wah_gc_heap_stats(NULL, NULL);
    assert_false(wah_gc_verify_heap(NULL));
    assert_null(wah_gc_alloc_host(NULL, 16));

    wah_module_t module = {0};
    wah_exec_context_t ctx = {0};
    assert_ok(wah_parse_module_from_spec(&module, "wasm"));
    assert_ok(wah_new_exec_context(&ctx, &module, NULL));
    assert_true(wah_gc_verify_heap(&ctx));

    wah_free_exec_context(&ctx);
    wah_free_module(&module);
}

int main(void) {
    test_exec_context_misuse();
    test_fuel_misuse();
    test_gc_misuse();
    printf("API misuse tests passed\n");
    return 0;
}
