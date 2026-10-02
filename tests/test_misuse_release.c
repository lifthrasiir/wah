// Misuse that only WAH_ASSERT catches in debug builds should still be handled in release builds
#undef WAH_ASSERT
#define WAH_ASSERT(cond) ((void)(cond))
#define WAH_IMPLEMENTATION
#include "../wah.h"
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

int main(void) {
    test_trap_with_non_error();
    printf("All release misuse tests passed!\n");
    return 0;
}
