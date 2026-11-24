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
    printf("Testing an i32 result set as i64 is refused, not to use anything above i32 in memory addresses...\n");
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
    assert_err(wah_call(&ctx, 1, NULL, 0, &r), WAH_ERROR_MISUSE);
    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
    wah_free_module(&host);
}

static wah_exec_context_t *ctx_to_free;
static void free_ctx(wah_call_context_t *ctx, void *userdata) {
    (void)ctx, (void)userdata;
    wah_free_exec_context(ctx_to_free); // Misuse: the context or its link domain is running
}

// Keeps an exception in a global, so that the call cancelled with an error sweeps exceptions, then loads from p.mem
static const char *CONSUMER_SPEC = "wasm types {[ fn [] [], fn [] [i32], fn [] [i32, exnref], fn [i32] [] ]} \
    imports {[ {'h'} {'f'} fn# 0, {'p'} {'mem'} mem# limits.i32/1 1 ]} funcs {[ 1 ]} tags {[ tag.type# 3 ]} \
    globals {[ exnref mut ref.null exnref end ]} code {[ {[] \
        block 2 try_table void [catch_ref 0 0] i32.const 1 throw 0 end i32.const 0 ref.null exnref end \
        global.set 0 drop call 0 i32.const 100 i32.load 2 0 end} ]}";

static void test_free_running(bool self) {
    printf("Testing freeing %s from a host function...\n", self ? "the running context" : "a linked context");
    wah_module_t prov = {0}, cons = {0}, host = {0};
    wah_exec_context_t pctx = {0}, ctx = {0};
    assert_ok(wah_parse_module_from_spec(&prov, "wasm memories {[ limits.i32/1 1 ]} exports {[ {'mem'} mem# 0 ]}"));
    assert_ok(wah_new_module(&host, NULL));
    assert_ok(wah_export_func(&host, "f", "()", free_ctx, NULL, NULL));
    assert_ok(wah_parse_module_from_spec(&cons, CONSUMER_SPEC));
    assert_ok(wah_new_exec_context(&pctx, &prov, NULL));
    assert_ok(wah_instantiate(&pctx));
    assert_ok(wah_new_exec_context(&ctx, &cons, NULL));
    assert_ok(wah_link_module(&ctx, "h", &host));
    assert_ok(wah_link_context(&ctx, "p", &pctx));
    assert_ok(wah_instantiate(&ctx));
    ctx_to_free = self ? &ctx : &pctx;
    wah_value_t r;
    wah_error_t err = wah_call(&ctx, 1, NULL, 0, &r);
    if (self) { // Not freed
        assert_ok(err);
        assert_eq_i32(r.i32, 0);
    } else { // Not to use the freed memory
        assert_err(err, WAH_ERROR_MISUSE);
    }
    wah_free_exec_context(&ctx);
    if (self) wah_free_exec_context(&pctx);
    wah_free_module(&cons);
    wah_free_module(&prov);
    wah_free_module(&host);
}

static int64_t leaked_bits;
static void misuse_accessors(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    leaked_bits = wah_param_i64(ctx, 1); // anyref
    assert_eq_i32(wah_param_i32(ctx, (size_t)1 << 40), 0);
    assert_true(wah_param_ref(ctx, 0) == NULL); // i32
    assert_true(wah_param_type(ctx, (size_t)1 << 40) != WAH_TYPE_I32);
    wah_result_i32(ctx, 1, 42);
    wah_result_i64(ctx, 0, 0x4141414141414140LL); // anyref
}
static void misuse_result_index(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    wah_result_i32(ctx, 1, 42);
    wah_result_i32(ctx, (size_t)1 << 40, 1);
}
static void misuse_result_ref(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    wah_result_i32(ctx, 1, 42);
    wah_result_ref(ctx, 1, NULL); // i32
}

static void test_accessors(void) {
    printf("Testing call context accessors with wrong indices or types...\n");
    void (*fns[])(wah_call_context_t *, void *) = { misuse_accessors, misuse_result_index, misuse_result_ref };
    for (size_t i = 0; i < sizeof(fns) / sizeof(*fns); i++) {
        wah_module_t mod = {0}, host = {0};
        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_module(&host, NULL));
        assert_ok(wah_export_func(&host, "f", "(i32, anyref) -> (anyref, i32)", fns[i], NULL, NULL));
        assert_ok(wah_parse_module_from_spec(&mod, "wasm types {[ struct [i32 mut], fn [i32, anyref] [anyref, i32], \
            fn [] [i32] ]} imports {[ {'h'} {'f'} fn# 1 ]} funcs {[ 2 ]} code {[ {[] \
                i32.const 7 i32.const 99 struct.new 0 call 0 drop ref.test.null eqref end} ]}"));
        assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
        assert_ok(wah_link_module(&ctx, "h", &host));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t r;
        leaked_bits = 0;
        assert_err(wah_call(&ctx, 1, NULL, 0, &r), WAH_ERROR_MISUSE);
        assert_eq_i64(leaked_bits, 0);
        wah_free_exec_context(&ctx);
        wah_free_module(&mod);
        wah_free_module(&host);
    }
}

static void test_change_used_module(void) {
    printf("Testing modules used by exec contexts can't be changed...\n");
    wah_module_t mod = {0}, linked = {0};
    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_module(&mod, NULL));
    assert_ok(wah_new_module(&linked, NULL));
    assert_ok(wah_export_global_i32(&mod, "g0", true, 1));
    assert_ok(wah_export_global_i32(&linked, "h0", true, 1));
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL)); // Sizes globals of the context
    assert_ok(wah_link_module(&ctx, "linked", &linked));
    char name[16];
    for (int i = 1; i < 40; i++) {
        snprintf(name, sizeof(name), "g%d", i);
        assert_err(wah_export_global_i32(&mod, name, true, i), WAH_ERROR_MISUSE);
        assert_err(wah_export_global_i32(&linked, name, true, i), WAH_ERROR_MISUSE);
    }
    wah_type_t t;
    assert_err(wah_define_type(&mod, &t, "fn () -> ()"), WAH_ERROR_MISUSE);
    assert_err(wah_export_func(&mod, "f", "() -> ()", free_ctx, NULL, NULL), WAH_ERROR_MISUSE);
    assert_err(wah_export_memory(&linked, "m", 1, 1), WAH_ERROR_MISUSE);
    assert_ok(wah_instantiate(&ctx));
    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
    wah_free_module(&linked);
}

static void *fake_ref;
static void result_fake_ref(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    wah_result_ref(ctx, 0, fake_ref);
}

static void test_raw_refs(void) {
    printf("Testing references not from wah_gc_alloc_host are refused...\n");
    WAH_ALIGNAS(16) static uint8_t buf[64]; // Zeroes are like a header of a struct, not a host object
    fake_ref = buf + 32;
    wah_module_t mod = {0}, host = {0};
    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_module(&host, NULL));
    assert_ok(wah_export_func(&host, "f", "() -> anyref", result_fake_ref, NULL, NULL));
    assert_ok(wah_parse_module_from_spec(&mod, "wasm types {[ fn [anyref] [i32], fn [] [anyref], fn [] [i32] ]} \
        imports {[ {'h'} {'f'} fn# 1 ]} funcs {[ 0, 2 ]} code {[ \
        {[] local.get 0 ref.is_null end}, {[] call 0 ref.is_null end} ]}"));
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_link_module(&ctx, "h", &host));
    assert_ok(wah_instantiate(&ctx));
    wah_value_t p = { .ref = fake_ref }, r;
    assert_err(wah_call(&ctx, 1, &p, 1, &r), WAH_ERROR_MISUSE);
    assert_err(wah_call(&ctx, 2, NULL, 0, &r), WAH_ERROR_MISUSE);
    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
    wah_free_module(&host);
}

int main(void) {
    test_trap_with_non_error();
    test_raw_refs();
    test_change_used_module();
    test_accessors();
    test_free_running(false);
    test_free_running(true);
    test_wide_i32_result();
    printf("All release misuse tests passed!\n");
    return 0;
}
