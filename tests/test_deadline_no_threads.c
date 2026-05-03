#define WAH_NO_THREADS
#define WAH_IMPLEMENTATION
#include "../wah.h"
#include "common.h"
#include <stdio.h>

int main(void) {
    printf("Testing deadline disabled when WAH_NO_THREADS is defined...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};
    wah_limits_t limits = {0};
    limits.deadline_us = 1;
    wah_exec_options_t options = { .limits = limits };

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] []]} funcs {[0]} \
        code {[{[] end}]}"));
    assert_err(wah_new_exec_context(&ctx, &mod, &options), WAH_ERROR_DISABLED_FEATURE);

    // wah_set_limits should reject deadline_us with the same error.
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    wah_limits_t lim = {0};
    lim.deadline_us = 1;
    assert_err(wah_set_limits(&ctx, &lim), WAH_ERROR_DISABLED_FEATURE);
    wah_free_exec_context(&ctx);

    wah_free_module(&mod);

    printf("\n=== deadline no-thread test passed ===\n");
    return 0;
}
