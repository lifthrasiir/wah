#define WAH_NO_THREADS
#define WAH_IMPLEMENTATION
#include "../wah.h"
#include "common.h"
#include <stdio.h>

int main(void) {
    printf("Testing deadline disabled when WAH_NO_THREADS is defined...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};
    wah_rlimits_t limits = {0};
    limits.deadline = 1;
    wah_exec_options_t options = { .limits = limits };

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] []]} funcs {[0]} \
        code {[{[] end}]}"));
    assert_err(wah_exec_context_create(&ctx, &mod, &options), WAH_ERROR_DISABLED_FEATURE);
    wah_free_module(&mod);

    printf("\n=== deadline no-thread test passed ===\n");
    return 0;
}
