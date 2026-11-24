// Tests for APIs used to embed WAH as a sandbox: memory access from the host, and host-defined memory64.
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../wah.h"
#include "common.h"
#include "wah_impl.h"

static void test_memory_data(void) {
    printf("Testing wah_memory_data and wah_export_memory64...\n");
    wah_module_t host = {0};
    assert_ok(wah_new_module(&host, NULL));
    assert_ok(wah_export_memory(&host, "m32", 0, 2)); // Memories can start empty
    assert_ok(wah_export_memory64(&host, "m64", 1, 1));
    assert_err(wah_export_memory64(&host, "bad", 2, 1), WAH_ERROR_MISUSE);
    assert_err(wah_export_memory64(&host, "bad", (UINT64_C(1) << 48) + 1, UINT64_MAX), WAH_ERROR_MISUSE);

    wah_exec_context_t hctx = {0};
    assert_ok(wah_new_exec_context(&hctx, &host, NULL));
    uint8_t *data = NULL;
    uint64_t size = 1;
    assert_err(wah_memory_data(&hctx, 0, &data, &size), WAH_ERROR_MISUSE); // Not instantiated
    assert_err(wah_memory_data(NULL, 0, &data, &size), WAH_ERROR_MISUSE);
    assert_ok(wah_instantiate(&hctx));
    assert_err(wah_memory_data(&hctx, 2, &data, &size), WAH_ERROR_NOT_FOUND);
    assert_ok(wah_memory_data(&hctx, 0, &data, &size));
    assert_eq_u64(size, 0);
    assert_ok(wah_memory_data(&hctx, 1, &data, &size));
    assert_eq_u64(size, 65536);
    data[5] = 42;
    data[65535] = 7;

    wah_module_t mod = {0};
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[ fn [] [i32], fn [i32] [i32] ]} \
        imports {[ {'h'} {'m32'} mem# limits.i32/2 0 2, {'h'} {'m64'} mem# limits.i64/1 0 ]} \
        funcs {[ 0, 1 ]} exports {[ {'load'} fn# 0, {'grow'} fn# 1 ]} \
        code {[ {[] i64.const 5 i32.load8_u 64 1 0 i64.const 65535 i32.load8_u 64 1 0 i32.add end}, \
                {[] local.get 0 memory.grow 0 i32.const 3 i32.const 9 i32.store8 0 0 end} ]}"));
    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_link_context(&ctx, "h", &hctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t r, p = { .i32 = 1 };
    assert_ok(wah_call_by_name(&ctx, "load", NULL, 0, &r));
    assert_eq_i32(r.i32, 49);
    assert_ok(wah_call_by_name(&ctx, "grow", &p, 1, &r));
    assert_eq_i32(r.i32, 0);
    // Both the owner and the importer see the grown memory
    uint8_t *data2 = NULL;
    assert_ok(wah_memory_data(&hctx, 0, &data, &size));
    assert_eq_u64(size, 65536);
    assert_eq_i32(data[3], 9);
    assert_ok(wah_memory_data(&ctx, 0, &data2, &size));
    assert_eq_ptr(data2, data);
    assert_eq_u64(size, 65536);
    assert_ok(wah_memory_data(&ctx, 1, &data2, &size));
    assert_eq_u64(size, 65536);
    assert_eq_i32(data2[5], 42);

    wah_free_exec_context(&ctx);
    wah_free_exec_context(&hctx);
    wah_free_module(&mod);
    wah_free_module(&host);
}

int main(void) {
    test_memory_data();
    printf("All embedding tests passed.\n");
    return 0;
}
