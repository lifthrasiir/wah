#define WAH_FORCE_SWITCH
#define WAH_IMPLEMENTATION
#include "../wah.h"
#include "common.h"
#include <stdio.h>

int main(void) {
    printf("Testing switch-based dispatch...\n");

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module,
        "wasm "
        "types {[ fn [i32, i32] [i32] ]} "
        "funcs {[ 0 ]} "
        "exports {[ {'add'} fn# 0 ]} "
        "code {[ {[] local.get 0 local.get 1 i32.add end} ]}"));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &module, NULL));

    wah_value_t params[2] = { {.i32 = 17}, {.i32 = 25} };
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, params, 2, &result));
    assert_eq_i32(result.i32, 42);

    wah_free_exec_context(&ctx);
    wah_free_module(&module);

    printf("Switch-based dispatch test passed!\n");
    return 0;
}
