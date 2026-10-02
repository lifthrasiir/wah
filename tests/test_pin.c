// Test for pinned references in host functions

#include "../wah.h"
#include "common.h"
#include <stdio.h>
#include <stdint.h>

// Returns the reference given in the previous call and keeps the current one, so that references are kept
// alive by the host across calls and garbage collections.
static void *prev_pinned = NULL;
static void host_swap(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    void *cur = wah_param_pinned_ref(ctx, 0);
    wah_result_ref(ctx, 0, prev_pinned);
    wah_unpin_ref(ctx, prev_pinned); // Does nothing for NULL
    prev_pinned = cur;
}

static void host_return_unpinned(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    void *ref = wah_param_pinned_ref(ctx, 0);
    wah_unpin_ref(ctx, ref);
    wah_result_ref(ctx, 0, ref);
}

static void host_unpin_twice(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    void *ref = wah_param_pinned_ref(ctx, 0);
    wah_unpin_ref(ctx, ref);
    wah_unpin_ref(ctx, ref);
    wah_result_ref(ctx, 0, NULL);
}

static void host_identity(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    wah_result_ref(ctx, 0, wah_param_ref(ctx, 0));
}

static void host_return_pinned(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    void *ref = wah_param_pinned_ref(ctx, 0);
    wah_result_ref(ctx, 0, ref);
    wah_unpin_ref(ctx, ref);
}

static void host_return_bogus(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    wah_result_ref(ctx, 0, (void *)(uintptr_t)0x1002); // Tagged like a pinned reference that was never issued
}

// Module importing h.f of the given type, where `call` wraps a struct of the given i32 field into the parameter
// and `ret` unwraps the result into the field or -1 if null. Function 1 churns the GC heap.
static void make_module(wah_module_t *mod, const char *param, const char *result, const char *call,
                        const char *ret) {
    assert_ok(wah_parse_module_from_spec(mod, "wasm \
        types {[ struct [i32 mut], fn [%t] [%t], fn [i32] [i32], fn [] [] ]} \
        imports {[ {'h'} {'f'} fn# 1 ]} funcs {[ 2, 3 ]} \
        exports {[ {'run'} fn# 1, {'churn'} fn# 2 ]} \
        code {[ {[1 %t] local.get 0 struct.new 0 %t call 0 local.tee 1 ref.is_null if i32 i32.const -1 \
                    else local.get 1 %t ref.cast 0 struct.get 0 0 end end}, \
                {[1 i32] loop void i32.const 0 struct.new 0 drop \
                    local.get 0 i32.const 1 i32.add local.tee 0 i32.const 200000 i32.lt_u br_if 0 end end} ]}",
        param, result, result, call, ret));
}

static void setup(wah_exec_context_t *ctx, wah_module_t *mod, wah_module_t *host, const char *type,
                  wah_func_t fn) {
    assert_ok(wah_new_module(host, NULL));
    assert_ok(wah_export_func(host, "f", type, fn, NULL, NULL));
    assert_ok(wah_new_exec_context(ctx, mod, NULL));
    assert_ok(wah_link_module(ctx, "h", host));
    assert_ok(wah_instantiate(ctx));
}

static int32_t run(wah_exec_context_t *ctx, int32_t i, wah_error_t expected) {
    wah_value_t arg = { .i32 = i }, r = { .i32 = 0 };
    assert_err(wah_call_by_name(ctx, "run", &arg, 1, &r), expected);
    return r.i32;
}

static void test_swap(const char *type, const char *param, const char *call, const char *ret) {
    printf("Testing a host function keeping pinned %s references across calls...\n", param);
    wah_module_t mod = {0}, host = {0};
    wah_exec_context_t ctx = {0};
    make_module(&mod, param, param, call, ret);
    setup(&ctx, &mod, &host, type, host_swap);
    prev_pinned = NULL;
    assert_eq_i32(run(&ctx, 1, WAH_OK), -1);
    for (int32_t i = 2; i <= 5; ++i) {
        assert_ok(wah_call_by_name(&ctx, "churn", NULL, 0, NULL));
        assert_eq_i32(run(&ctx, i, WAH_OK), i - 1);
    }
    // The last pinned reference is released with the context
    wah_free_exec_context(&ctx);
    prev_pinned = NULL;
    wah_free_module(&mod);
    wah_free_module(&host);
}

static void test_misuse(const char *desc, const char *type, const char *param, const char *result,
                        const char *call, const char *ret, wah_func_t fn, wah_error_t expected) {
    printf("Testing %s...\n", desc);
    wah_module_t mod = {0}, host = {0};
    wah_exec_context_t ctx = {0};
    make_module(&mod, param, result, call, ret);
    setup(&ctx, &mod, &host, type, fn);
    run(&ctx, 7, expected);
    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
    wah_free_module(&host);
}

int main(void) {
    test_swap("(externref) -> externref", "externref", "extern.convert_any", "any.convert_extern");
    test_swap("(anyref) -> anyref", "anyref", "", "");

    test_misuse("returning a pinned reference", "(externref) -> externref", "externref", "externref",
                "extern.convert_any", "any.convert_extern", host_return_pinned, WAH_OK);
    test_misuse("returning an unpinned reference is misuse", "(externref) -> externref", "externref", "externref",
                "extern.convert_any", "any.convert_extern", host_return_unpinned, WAH_ERROR_MISUSE);
    test_misuse("unpinning a reference twice is misuse", "(externref) -> externref", "externref", "externref",
                "extern.convert_any", "any.convert_extern", host_unpin_twice, WAH_ERROR_MISUSE);
    test_misuse("returning a sanitized reference is misuse", "(externref) -> externref", "externref", "externref",
                "extern.convert_any", "any.convert_extern", host_identity, WAH_ERROR_MISUSE);
    test_misuse("returning a bogus pinned reference is misuse", "(externref) -> externref", "externref",
                "externref", "extern.convert_any", "any.convert_extern", host_return_bogus, WAH_ERROR_MISUSE);
    test_misuse("returning a pinned reference of an incompatible type is misuse", "(anyref) -> externref",
                "anyref", "externref", "", "any.convert_extern", host_return_pinned, WAH_ERROR_MISUSE);

    printf("Testing pinning a null reference gives null...\n");
    {
        wah_module_t mod = {0}, host = {0};
        wah_exec_context_t ctx = {0};
        assert_ok(wah_parse_module_from_spec(&mod, "wasm \
            types {[ fn [externref] [externref], fn [] [i32] ]} \
            imports {[ {'h'} {'f'} fn# 0 ]} funcs {[ 1 ]} exports {[ {'run'} fn# 1 ]} \
            code {[ {[] ref.null externref call 0 ref.is_null end} ]}"));
        setup(&ctx, &mod, &host, "(externref) -> externref", host_return_pinned);
        wah_value_t r;
        assert_ok(wah_call_by_name(&ctx, "run", NULL, 0, &r));
        assert_eq_i32(r.i32, 1);
        wah_free_exec_context(&ctx);
        wah_free_module(&mod);
        wah_free_module(&host);
    }

    printf("All pin tests passed!\n");
    return 0;
}
