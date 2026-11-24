// Tests for APIs used to embed WAH as a sandbox: memory access from the host, host-defined memory64,
// hooks on memory growth, and host functions suspending their activations.
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

typedef struct {
    wah_error_t ret;
    int calls;
    uint32_t mem_idx;
    uint64_t old_bytes, new_bytes;
} grow_state_t;

static wah_error_t grow_hook(void *userdata, uint32_t mem_idx, uint64_t old_bytes, uint64_t new_bytes) {
    grow_state_t *s = (grow_state_t *)userdata;
    s->calls++;
    s->mem_idx = mem_idx;
    s->old_bytes = old_bytes;
    s->new_bytes = new_bytes;
    return s->ret;
}

static void test_memory_grow_hook(void) {
    printf("Testing wah_set_memory_grow_hook...\n");
    wah_module_t host = {0};
    assert_ok(wah_new_module(&host, NULL));
    assert_ok(wah_export_memory(&host, "a", 1, 1));
    assert_ok(wah_export_memory(&host, "b", 1, 4));
    wah_exec_context_t hctx = {0};
    assert_ok(wah_new_exec_context(&hctx, &host, NULL));
    grow_state_t s = { .ret = WAH_OK };
    assert_err(wah_set_memory_grow_hook(&hctx, grow_hook, &s), WAH_ERROR_MISUSE); // Not instantiated
    assert_ok(wah_instantiate(&hctx));
    assert_ok(wah_set_memory_grow_hook(&hctx, grow_hook, &s));

    // Memory 0 imports "b" (memory 1 of the owner), memory 1 is defined with its own hook
    wah_module_t mod = {0};
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[ fn [i32] [i32], fn [] [i32] ]} \
        imports {[ {'h'} {'b'} mem# limits.i32/1 1 ]} \
        funcs {[ 0, 1, 0 ]} memories {[ limits.i32/2 0 2 ]} \
        exports {[ {'grow'} fn# 0, {'size'} fn# 1, {'grow1'} fn# 2 ]} \
        code {[ {[] local.get 0 memory.grow 0 end}, {[] memory.size 0 end}, {[] local.get 0 memory.grow 1 end} ]}"));
    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_link_context(&ctx, "h", &hctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t r, p = { .i32 = 1 };
    assert_ok(wah_call_by_name(&ctx, "grow", &p, 1, &r));
    assert_eq_i32(r.i32, 1);
    assert_eq_i32(s.calls, 1);
    assert_eq_u32(s.mem_idx, 1); // Of the owner
    assert_eq_u64(s.old_bytes, 65536);
    assert_eq_u64(s.new_bytes, 2 * 65536);

    // Growing by nothing, or beyond the maximum, doesn't call the hook
    p.i32 = 0;
    assert_ok(wah_call_by_name(&ctx, "grow", &p, 1, &r));
    assert_eq_i32(r.i32, 2);
    p.i32 = 3;
    assert_ok(wah_call_by_name(&ctx, "grow", &p, 1, &r));
    assert_eq_i32(r.i32, -1);
    assert_eq_i32(s.calls, 1);

    // WAH_ERROR_OUT_OF_MEMORY refuses to grow
    s.ret = WAH_ERROR_OUT_OF_MEMORY;
    p.i32 = 1;
    assert_ok(wah_call_by_name(&ctx, "grow", &p, 1, &r));
    assert_eq_i32(r.i32, -1);
    assert_eq_i32(s.calls, 2);
    assert_ok(wah_call_by_name(&ctx, "size", NULL, 0, &r));
    assert_eq_i32(r.i32, 2);

    // Other errors trap
    s.ret = WAH_ERROR_TOO_LARGE;
    assert_err(wah_call_by_name(&ctx, "grow", &p, 1, &r), WAH_ERROR_TOO_LARGE);
    assert_eq_i32(s.calls, 3);
    assert_ok(wah_call_by_name(&ctx, "size", NULL, 0, &r));
    assert_eq_i32(r.i32, 2);

    // Memories defined by the importer are not the owner's; NULL removes the hook
    assert_ok(wah_call_by_name(&ctx, "grow1", &p, 1, &r));
    assert_eq_i32(r.i32, 0);
    assert_eq_i32(s.calls, 3);
    grow_state_t s2 = { .ret = WAH_ERROR_TRAP };
    assert_ok(wah_set_memory_grow_hook(&ctx, grow_hook, &s2));
    assert_err(wah_call_by_name(&ctx, "grow1", &p, 1, &r), WAH_ERROR_TRAP);
    assert_eq_i32(s2.calls, 1);
    assert_eq_u32(s2.mem_idx, 1);
    assert_eq_u64(s2.old_bytes, 65536);
    assert_ok(wah_set_memory_grow_hook(&hctx, NULL, NULL));
    s.ret = WAH_ERROR_TRAP;
    assert_ok(wah_call_by_name(&ctx, "grow", &p, 1, &r));
    assert_eq_i32(r.i32, 2);
    assert_eq_i32(s.calls, 3);

    wah_free_exec_context(&ctx);
    wah_free_exec_context(&hctx);
    wah_free_module(&mod);
    wah_free_module(&host);
}

typedef struct {
    int suspensions; // Left before returning
    int calls;
    int32_t param;
} wait_state_t;

// Returns twice its parameter, after suspending as many times as asked
static void host_wait(wah_call_context_t *cc, void *userdata) {
    wait_state_t *s = (wait_state_t *)userdata;
    s->calls++;
    if (s->calls > 1) assert_eq_i32(wah_param_i32(cc, 0), s->param); // Retried with the same parameter
    s->param = wah_param_i32(cc, 0);
    if (s->suspensions > 0) {
        s->suspensions--;
        wah_result_i32(cc, 0, -1); // Discarded
        wah_suspend(cc);
        return;
    }
    wah_result_i32(cc, 0, s->param * 2);
}

static void host_wait_ref(wah_call_context_t *cc, void *userdata) {
    wait_state_t *s = (wait_state_t *)userdata;
    s->calls++;
    assert_not_null(wah_param_ref(cc, 0));
    if (s->suspensions > 0) {
        s->suspensions--;
        wah_suspend(cc);
    }
}

// Runs an activation to completion, returning the number of yields
static int run_yielding(wah_exec_context_t *ctx, uint32_t func_idx, int32_t arg, int32_t *result) {
    wah_value_t p = { .i32 = arg }, r;
    assert_ok(wah_start(ctx, func_idx, &p, 1));
    int yields = 0;
    wah_error_t err;
    while ((err = wah_resume(ctx)) == WAH_STATUS_YIELDED) {
        assert_true(wah_is_suspended(ctx));
        yields++;
    }
    assert_ok(err);
    uint32_t n;
    assert_ok(wah_finish(ctx, &r, 1, &n));
    *result = r.i32;
    return yields;
}

static void test_host_suspend(void) {
    printf("Testing wah_suspend...\n");
    wait_state_t s = {0};
    wah_module_t host = {0};
    assert_ok(wah_new_module(&host, NULL));
    assert_ok(wah_export_func(&host, "wait", "(i32) -> i32", host_wait, &s, NULL));

    // 1: direct call with a value below its arguments, 2: through a nested call, 3: call_indirect,
    // 4: return_call, 5: return_call_indirect
    wah_module_t mod = {0};
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[ fn [i32] [i32] ]} imports {[ {'h'} {'wait'} fn# 0 ]} \
        funcs {[ 0, 0, 0, 0, 0 ]} tables {[ funcref limits.i32/1 1 ]} \
        elements {[ elem.active.table#0 i32.const 0 end [0] ]} code {[ \
        {[] i32.const 100 local.get 0 call 0 i32.add end}, \
        {[] i32.const 1000 local.get 0 call 1 i32.add end}, \
        {[] i32.const 100 local.get 0 i32.const 0 call_indirect 0 0 i32.add end}, \
        {[] local.get 0 return_call 0 end}, \
        {[] local.get 0 i32.const 0 return_call_indirect 0 0 end} ]}"));
    wah_exec_context_t hctx = {0}, ctx = {0};
    assert_ok(wah_new_exec_context(&hctx, &host, NULL));
    assert_ok(wah_instantiate(&hctx));
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_link_context(&ctx, "h", &hctx));
    assert_ok(wah_instantiate(&ctx));

    static const struct { uint32_t func; int32_t base; } cases[] = {
        { 1, 100 }, { 2, 1100 }, { 3, 100 }, { 4, 0 }, { 5, 0 }, { 0, 0 } // 0: the host function itself
    };
    for (size_t i = 0; i < sizeof(cases) / sizeof(*cases); i++) {
        for (int n = 0; n < 3; n++) {
            s = (wait_state_t){ .suspensions = n };
            int32_t r;
            assert_eq_i32(run_yielding(&ctx, cases[i].func, 21, &r), n);
            assert_eq_i32(r, cases[i].base + 42);
            assert_eq_i32(s.calls, n + 1);
        }
    }

    // wah_call can't be resumed, so it cancels the activation
    s = (wait_state_t){ .suspensions = 1 };
    wah_value_t p = { .i32 = 1 }, r;
    assert_err(wah_call(&ctx, 1, &p, 1, &r), WAH_STATUS_YIELDED);
    assert_eq_i32((int32_t)wah_exec_state(&ctx), WAH_EXEC_READY);
    assert_ok(wah_call(&ctx, 1, &p, 1, &r));
    assert_eq_i32(r.i32, 102);

    // Cancelled while suspended in the host function
    s = (wait_state_t){ .suspensions = 1 };
    assert_ok(wah_start(&ctx, 2, &p, 1));
    assert_err(wah_resume(&ctx), WAH_STATUS_YIELDED);
    wah_cancel(&ctx);
    assert_ok(wah_call(&ctx, 2, &p, 1, &r));
    assert_eq_i32(r.i32, 1102);

    wah_free_exec_context(&ctx);
    wah_free_exec_context(&hctx);
    wah_free_module(&mod);
    wah_free_module(&host);
}

static void test_host_suspend_gc(void) {
    printf("Testing wah_suspend with references on the stack...\n");
    wait_state_t s = {0};
    wah_module_t host = {0};
    assert_ok(wah_new_module(&host, NULL));
    assert_ok(wah_export_func(&host, "wait", "(funcref) -> ()", host_wait_ref, &s, NULL));
    wah_module_t mod = {0};
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[ fn [funcref] [], fn [i32] [i32] ]} imports {[ {'h'} {'wait'} fn# 0 ]} \
        funcs {[ 1 ]} exports {[ {'f'} fn# 1 ]} code {[ {[1 funcref] \
            ref.func 1 local.set 1 ref.func 1 call 0 local.get 1 ref.is_null end} ]}"));
    wah_exec_context_t hctx = {0}, ctx = {0};
    assert_ok(wah_new_exec_context(&hctx, &host, NULL));
    assert_ok(wah_instantiate(&hctx));
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_link_context(&ctx, "h", &hctx));
    assert_ok(wah_instantiate(&ctx));

    s = (wait_state_t){ .suspensions = 3 };
    wah_value_t p = { .i32 = 0 }, r;
    assert_ok(wah_start(&ctx, 1, &p, 1));
    int yields = 0;
    wah_error_t err;
    while ((err = wah_resume(&ctx)) == WAH_STATUS_YIELDED) {
        yields++;
        wah_debug_gc_step(&ctx);
        assert_true(wah_gc_verify_heap(&ctx));
    }
    assert_ok(err);
    assert_eq_i32(yields, 3);
    uint32_t n;
    assert_ok(wah_finish(&ctx, &r, 1, &n));
    assert_eq_i32(r.i32, 0);

    wah_free_exec_context(&ctx);
    wah_free_exec_context(&hctx);
    wah_free_module(&mod);
    wah_free_module(&host);
}

int main(void) {
    test_memory_data();
    test_memory_grow_hook();
    test_host_suspend();
    test_host_suspend_gc();
    printf("All embedding tests passed.\n");
    return 0;
}
