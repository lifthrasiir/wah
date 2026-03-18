#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define WAH_IMPLEMENTATION
#include "wah.h"
#include "wah_testutils.c"

void wah_test_table_indirect_call() {
    printf("Running wah_test_table_indirect_call...\n");

    wah_module_t module;
    wah_error_t err = wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [i32] [i32], fn [i32] [i32] ]} \
        funcs {[ 0, 0, 0, 0 ]} \
        tables {[ funcref limits.i32/1 2 ]} \
        exports {[ \
            {'add_one'} fn# 0, \
            {'sub_one'} fn# 1, \
            {'call_indirect_add'} fn# 2, \
            {'call_indirect_sub'} fn# 3, \
        ]} \
        elements {[ elem.passive i32.const 0 end [0, 1] ]} \
        code {[ \
            {[] local.get 0 i32.const 1 i32.add end}, \
            {[] local.get 0 i32.const 1 i32.sub end}, \
            {[] local.get 0 i32.const 0 call_indirect 0 0 end}, \
            {[] local.get 0 i32.const 1 call_indirect 0 0 end}, \
        ]}");
    assert(err == WAH_OK);

    wah_exec_context_t exec_ctx;
    err = wah_exec_context_create(&exec_ctx, &module);
    assert(err == WAH_OK);

    // Find the exported function indices
    uint32_t add_one_func_idx = (uint32_t)-1;
    uint32_t sub_one_func_idx = (uint32_t)-1;
    uint32_t call_indirect_add_func_idx = (uint32_t)-1;
    uint32_t call_indirect_sub_func_idx = (uint32_t)-1;

    // NOTE: Export section parsing is not implemented yet, so we'll hardcode for now
    // Based on the WASM binary, func 0 is add_one, func 1 is sub_one, func 2 is call_indirect_add, func 3 is call_indirect_sub
    add_one_func_idx = 0;
    sub_one_func_idx = 1;
    call_indirect_add_func_idx = 2;
    call_indirect_sub_func_idx = 3;

    (void)add_one_func_idx; // Suppress warning
    (void)sub_one_func_idx; // Suppress warning

    // Instantiate the module (resolves imports and initializes element segments)
    err = wah_instantiate(&exec_ctx);
    assert(err == WAH_OK);

    // Test call_indirect_add (calls add_one indirectly)
    wah_value_t params_add[1] = {{.i32 = 10}};
    wah_value_t result_add;
    err = wah_call(&exec_ctx, call_indirect_add_func_idx, params_add, 1, &result_add);
    assert(err == WAH_OK);
    assert(result_add.i32 == 11);

    // Test call_indirect_sub (calls sub_one indirectly)
    wah_value_t params_sub[1] = {{.i32 = 10}};
    wah_value_t result_sub;
    err = wah_call(&exec_ctx, call_indirect_sub_func_idx, params_sub, 1, &result_sub);
    assert(err == WAH_OK);
    assert(result_sub.i32 == 9);

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);

    printf("wah_test_table_indirect_call passed.\n");
}

int main() {
    wah_test_table_indirect_call();
    return 0;
}
