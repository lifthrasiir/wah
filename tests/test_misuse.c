#include <stdio.h>
#include <string.h>
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

static void test_module_builder_misuse(void) {
    printf("Testing module builder misuse...\n");
    wah_type_t type;
    assert_err(wah_define_type(NULL, &type, "fn () -> ()"), WAH_ERROR_MISUSE);
    assert_err(wah_export_memory(NULL, "mem", 1, 1), WAH_ERROR_MISUSE);
    assert_err(wah_export_global_i32(NULL, "g", false, 0), WAH_ERROR_MISUSE);
    assert_err(wah_export_global_f64(NULL, "g", false, 0.0), WAH_ERROR_MISUSE);

    wah_module_t mod = {0};
    assert_ok(wah_new_module(&mod, NULL));
    assert_err(wah_export_global_v128(&mod, "g", false, NULL), WAH_ERROR_MISUSE); // Used to dereference it
    wah_free_module(&mod);
}

static void test_freeable_after_failure(void) {
    printf("Testing outputs can be freed after early failures...\n");

    // Garbage would be freed as pointers if the output were not initialized before failing
    static const uint8_t short_binary[] = { 0x00, 'a', 's', 'm' };
    wah_alloc_t bad_alloc = { 0 };
    wah_module_t module, empty;
    wah_exec_context_t ctx;

    memset(&module, 0xa5, sizeof(module));
    assert_err(wah_parse_module(&module, short_binary, sizeof(short_binary), NULL), WAH_ERROR_UNEXPECTED_EOF);
    wah_free_module(&module);

    memset(&module, 0xa5, sizeof(module));
    assert_err(wah_parse_module(&module, NULL, 0, NULL), WAH_ERROR_MISUSE);
    wah_free_module(&module);

    memset(&module, 0xa5, sizeof(module));
    assert_err(wah_new_module(&module, &bad_alloc), WAH_ERROR_MISUSE);
    wah_free_module(&module);

    assert_ok(wah_parse_module_from_spec(&empty, "wasm"));
    memset(&ctx, 0xa5, sizeof(ctx));
    assert_err(wah_new_exec_context(&ctx, &empty, &(wah_exec_options_t){ .alloc = &bad_alloc }), WAH_ERROR_MISUSE);
    wah_free_exec_context(&ctx);

    memset(&ctx, 0xa5, sizeof(ctx));
    assert_err(wah_new_exec_context(&ctx, NULL, NULL), WAH_ERROR_MISUSE);
    wah_free_exec_context(&ctx);
    wah_free_module(&empty);
}

int main(void) {
    test_exec_context_misuse();
    test_fuel_misuse();
    test_gc_misuse();
    test_module_builder_misuse();
    test_freeable_after_failure();
    printf("API misuse tests passed\n");
    return 0;
}
