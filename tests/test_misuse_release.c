// Misuse that only WAH_ASSERT catches in debug builds should still be handled in release builds,
// or at least should not be a safety issue. The JIT is also used where available.
#undef WAH_ASSERT
#define WAH_ASSERT(cond) ((void)(cond))
#define WAH_IMPLEMENTATION
#include "../wahjit.h"
#include "common.h"
#include <stdio.h>

static void trap_with_status(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    wah_trap(ctx, WAH_STATUS_YIELDED);
}

static void test_trap_with_non_error(void) {
    printf("Testing wah_trap with a non-error reason traps with WAH_ERROR_MISUSE...\n");
    wah_module_t mod = {0}, host = {0};
    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_module(&host, NULL));
    assert_ok(wah_export_func(&host, "f", "(i32) -> i32", trap_with_status, NULL, NULL));
    assert_ok(wah_parse_module_from_spec(&mod, "wasm types {[ fn [i32] [i32] ]} \
        imports {[ {'h'} {'f'} fn# 0 ]} funcs {[ 0 ]} code {[ {[] local.get 0 call 0 end} ]}"));
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_link_module(&ctx, "h", &host));
    assert_ok(wah_instantiate(&ctx));
    wah_value_t arg = { .i32 = 1 }, r;
    assert_err(wah_call(&ctx, 1, &arg, 1, &r), WAH_ERROR_MISUSE);
    assert_true(!wah_is_suspended(&ctx));
    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
    wah_free_module(&host);
}

static void result_wide_i32(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    wah_result_i64(ctx, 0, 0x100000008LL); // Misuse: the result is i32
}

static void test_wide_i32_result(void) {
    printf("Testing an i32 result set as i64 is still an i32 in memory addresses...\n");
    wah_module_t mod = {0}, host = {0};
    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_module(&host, NULL));
    assert_ok(wah_export_func(&host, "f", "() -> i32", result_wide_i32, NULL, NULL));
    assert_ok(wah_parse_module_from_spec(&mod, "wasm types {[ fn [] [i32] ]} \
        imports {[ {'h'} {'f'} fn# 0 ]} funcs {[ 0 ]} memories {[ limits.i32/1 1 ]} \
        code {[ {[] i32.const 8 i32.const 77 i32.store 2 0 call 0 i32.load 2 0 end} ]}"));
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_link_module(&ctx, "h", &host));
    assert_ok(wah_instantiate(&ctx));
    wah_value_t r;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &r));
    assert_eq_i32(r.i32, 77);
    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
    wah_free_module(&host);
}

int main(void) {
    test_trap_with_non_error();
    test_wide_i32_result();
    printf("All release misuse tests passed!\n");
    return 0;
}
