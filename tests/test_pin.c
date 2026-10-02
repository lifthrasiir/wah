// Test for pinned references in host functions

#include "../wah.h"
#include "common.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

// Returns the reference given in the previous call and keeps the current one, so that references are kept
// alive by the host across calls and garbage collections.
static void *prev_pinned = NULL;
static void host_swap(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    void *cur = wah_param_pinned_ref(ctx, 0);
    wah_result_ref(ctx, 0, prev_pinned);
    wah_unpin_ref_from_host(ctx, prev_pinned); // Does nothing for NULL
    prev_pinned = cur;
}

static void host_return_unpinned(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    void *ref = wah_param_pinned_ref(ctx, 0);
    wah_unpin_ref_from_host(ctx, ref);
    wah_result_ref(ctx, 0, ref);
}

static void host_unpin_twice(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    void *ref = wah_param_pinned_ref(ctx, 0);
    wah_unpin_ref_from_host(ctx, ref);
    wah_unpin_ref_from_host(ctx, ref);
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
    wah_unpin_ref_from_host(ctx, ref);
}

static void host_return_bogus(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    wah_result_ref(ctx, 0, (void *)(uintptr_t)0x1002); // Tagged like a pinned reference that was never issued
}

static void *raw_host_result = NULL;
static void host_return_raw(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    wah_result_ref(ctx, 0, raw_host_result);
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

// 0: make(i) -> anyref struct, 1: make_i31(i) -> anyref i31, 2: use(anyref) -> i32 field or i31 value,
// 3: churn(), 4: make_ext(i) -> externref struct
static void make_call_module(wah_module_t *mod) {
    assert_ok(wah_parse_module_from_spec(mod, "wasm \
        types {[ struct [i32 mut], fn [i32] [anyref], fn [anyref] [i32], fn [] [], fn [i32] [externref] ]} \
        funcs {[ 1, 1, 2, 3, 4 ]} \
        code {[ {[] local.get 0 struct.new 0 end}, \
                {[] local.get 0 ref.i31 end}, \
                {[] local.get 0 ref.test i31ref if i32 local.get 0 ref.cast i31ref i31.get_s \
                    else local.get 0 ref.cast 0 struct.get 0 0 end end}, \
                {[1 i32] loop void i32.const 0 struct.new 0 drop \
                    local.get 0 i32.const 1 i32.add local.tee 0 i32.const 200000 i32.lt_u br_if 0 end end}, \
                {[] local.get 0 struct.new 0 extern.convert_any end} ]}"));
}

static void test_call_pin(void) {
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};
    make_call_module(&mod);
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));
    wah_value_t arg = { .i32 = 42 }, r, r2;

    printf("Testing GC references returned by wah_call are sanitized...\n");
    assert_ok(wah_call(&ctx, 0, &arg, 1, &r));
    assert_true(r.ref != NULL);
    assert_err(wah_call(&ctx, 2, &r, 1, &r2), WAH_ERROR_MISUSE);

    printf("Testing references pinned by wah_call_pin survive garbage collections...\n");
    wah_value_t pinned[3];
    for (int32_t i = 0; i < 3; ++i) {
        arg.i32 = 10 + i;
        assert_ok(wah_call_pin(&ctx, i == 1 ? 1 : 0, &arg, 1, &pinned[i]));
        assert_ok(wah_call(&ctx, 3, NULL, 0, NULL));
    }
    for (int32_t i = 0; i < 3; ++i) {
        assert_ok(wah_call(&ctx, 2, &pinned[i], 1, &r));
        assert_eq_i32(r.i32, 10 + i);
    }

    printf("Testing wah_finish_pin pins results...\n");
    arg.i32 = 77;
    assert_ok(wah_start(&ctx, 0, &arg, 1));
    assert_ok(wah_resume(&ctx));
    uint32_t actual = 0;
    assert_ok(wah_finish_pin(&ctx, &r2, 1, &actual));
    assert_eq_u32(actual, 1);
    assert_ok(wah_call(&ctx, 3, NULL, 0, NULL));
    assert_ok(wah_call(&ctx, 2, &r2, 1, &r));
    assert_eq_i32(r.i32, 77);
    assert_ok(wah_unpin_ref(&ctx, r2.ref));

    printf("Testing released or mistyped pinned references are rejected...\n");
    assert_ok(wah_unpin_ref(&ctx, pinned[0].ref));
    assert_err(wah_call(&ctx, 2, &pinned[0], 1, &r), WAH_ERROR_MISUSE);
    assert_err(wah_unpin_ref(&ctx, pinned[0].ref), WAH_ERROR_MISUSE);
    assert_ok(wah_unpin_ref(&ctx, NULL));
    arg.i32 = 5;
    wah_value_t ext;
    assert_ok(wah_call_pin(&ctx, 4, &arg, 1, &ext));
    assert_err(wah_call(&ctx, 2, &ext, 1, &r), WAH_ERROR_MISUSE); // externref is not an anyref

    // Remaining pinned references are released with the context
    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_pin_host_ref(void) {
    printf("Testing host objects pinned by wah_pin_ref survive garbage collections...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};
    // 0: churn(), 1: keep(externref), 2: get() -> externref, 3: id(anyref) -> anyref,
    // 4: field((ref null 0)) -> i32, 5: is_null(eqref) -> i32
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[ struct [i32 mut], fn [] [], fn [externref] [], fn [] [externref], fn [anyref] [anyref], \
                 fn [%'6300'] [i32], fn [eqref] [i32] ]} \
        funcs {[ 1, 2, 3, 4, 5, 6 ]} globals {[ externref mut ref.null externref end ]} \
        code {[ {[1 i32] loop void i32.const 0 struct.new 0 drop \
                    local.get 0 i32.const 1 i32.add local.tee 0 i32.const 200000 i32.lt_u br_if 0 end end}, \
                {[] local.get 0 global.set 0 end}, \
                {[] global.get 0 end}, \
                {[] local.get 0 end}, \
                {[] local.get 0 struct.get 0 0 end}, \
                {[] local.get 0 ref.is_null end} ]}"));
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    void *host = wah_gc_alloc_host(&ctx, 16);
    assert_true(host != NULL);
    void *pinned = NULL;
    assert_ok(wah_pin_ref(&ctx, host, &pinned));
    assert_true(pinned != NULL && pinned != host);
    assert_ok(wah_call(&ctx, 0, NULL, 0, NULL));

    // Both the raw and pinned forms of a pinned host object can be passed to externref and anyref
    wah_value_t arg = { .ref = host }, r;
    assert_ok(wah_call(&ctx, 1, &arg, 1, NULL));
    assert_ok(wah_call(&ctx, 0, NULL, 0, NULL));
    assert_ok(wah_call(&ctx, 2, NULL, 0, &r));
    assert_true(r.ref == host);
    arg.ref = pinned;
    assert_ok(wah_call(&ctx, 1, &arg, 1, NULL));
    assert_ok(wah_call(&ctx, 3, &arg, 1, &r));
    assert_true(r.ref == host);

    printf("Testing host objects are rejected for parameters other than externref and anyref...\n");
    for (int i = 0; i < 2; i++) {
        arg.ref = i ? pinned : host;
        assert_err(wah_call(&ctx, 4, &arg, 1, &r), WAH_ERROR_MISUSE);
        assert_err(wah_call(&ctx, 5, &arg, 1, &r), WAH_ERROR_MISUSE);
    }

    printf("Testing host functions can't return host objects for results other than externref and anyref...\n");
    {
        wah_module_t user = {0}, hmod = {0};
        wah_exec_context_t uctx = {0};
        assert_ok(wah_parse_module_from_spec(&user, "wasm \
            types {[ fn [] [eqref], fn [] [i32] ]} \
            imports {[ {'h'} {'f'} fn# 0 ]} funcs {[ 1 ]} \
            code {[ {[] call 0 ref.is_null end} ]}"));
        assert_ok(wah_new_module(&hmod, NULL));
        assert_ok(wah_export_func(&hmod, "f", "() -> eqref", host_return_raw, NULL, NULL));
        assert_ok(wah_new_exec_context(&uctx, &user, NULL));
        assert_ok(wah_link_module(&uctx, "h", &hmod));
        assert_ok(wah_instantiate(&uctx));
        raw_host_result = wah_gc_alloc_host(&uctx, 16);
        void *keep = NULL;
        assert_ok(wah_pin_ref(&uctx, raw_host_result, &keep)); // Kept alive, but still returned raw
        assert_err(wah_call(&uctx, 1, NULL, 0, &r), WAH_ERROR_MISUSE);
        wah_free_exec_context(&uctx);
        wah_free_module(&user);
        wah_free_module(&hmod);
    }

    printf("Testing wah_pin_ref accepts only host objects...\n");
    void *out = host;
    assert_ok(wah_pin_ref(&ctx, NULL, &out));
    assert_true(out == NULL);
    assert_err(wah_pin_ref(&ctx, pinned, &out), WAH_ERROR_MISUSE);
    assert_err(wah_pin_ref(&ctx, (void *)~(uintptr_t)0, &out), WAH_ERROR_MISUSE);
    assert_ok(wah_unpin_ref(&ctx, pinned));
    assert_err(wah_unpin_ref(&ctx, pinned), WAH_ERROR_MISUSE);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void host_return_nothing(wah_call_context_t *ctx, void *userdata) { (void)ctx; (void)userdata; }

// NULL used to be accepted for non-nullable references from the host, both as parameters and as results.
static void test_non_nullable_host_refs(void) {
    printf("Testing NULL is rejected for non-nullable references from the host...\n");
    wah_module_t mod = {0}, host = {0};
    wah_exec_context_t ctx = {0};
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[ fn [] [type.ref.extern], fn [type.ref.extern] [i32], fn [] [i32] ]} \
        imports {[ {'h'} {'f'} fn# 0 ]} funcs {[ 1, 2 ]} \
        code {[ {[] i32.const 1 end}, {[] call 0 drop i32.const 1 end} ]}"));
    setup(&ctx, &mod, &host, "() -> ref extern", host_return_nothing);
    wah_value_t param = { .ref = NULL }, r;
    assert_err(wah_call(&ctx, 1, &param, 1, &r), WAH_ERROR_MISUSE);
    assert_err(wah_call(&ctx, 2, NULL, 0, &r), WAH_ERROR_MISUSE);
    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
    wah_free_module(&host);
}

// Like host_swap, but the host also runs another context of the link domain after setting the result
static wah_exec_context_t *reentry_ctx = NULL;
static void host_swap_reenter(wah_call_context_t *ctx, void *userdata) {
    host_swap(ctx, userdata);
    assert_ok(wah_call_by_name(reentry_ctx, "churn", NULL, 0, NULL));
}

static void test_swap_reenter(void) {
    printf("Testing results of a host function kept alive while it re-enters the link domain...\n");
    wah_module_t qmod = {0}, pmod = {0}, host = {0};
    wah_exec_context_t q = {0}, p = {0};
    // Objects live in q's heap and are only referenced from its global, which churn clears before collecting
    assert_ok(wah_parse_module_from_spec(&qmod, "wasm \
        types {[ struct [i32 mut], fn [i32] [anyref], fn [] [] ]} funcs {[ 1, 2, 2 ]} \
        globals {[ anyref mut ref.null anyref end ]} \
        exports {[ {'get'} fn# 0, {'churn'} fn# 1, {'fill'} fn# 2 ]} \
        code {[ {[] global.get 0 end}, \
                {[1 i32] ref.null anyref global.set 0 loop void i32.const 0 struct.new 0 drop \
                    local.get 0 i32.const 1 i32.add local.tee 0 i32.const 200000 i32.lt_u br_if 0 end end}, \
                {[] i32.const 4242 struct.new 0 global.set 0 end} ]}"));
    assert_ok(wah_parse_module_from_spec(&pmod, "wasm \
        types {[ struct [i32 mut], fn [i32] [anyref], fn [anyref] [anyref], fn [i32] [i32] ]} \
        imports {[ {'Q'} {'get'} fn# 1, {'h'} {'f'} fn# 2 ]} funcs {[ 3 ]} exports {[ {'run'} fn# 2 ]} \
        code {[ {[1 anyref] local.get 0 call 0 call 1 local.tee 1 ref.is_null if i32 i32.const -1 \
                 else local.get 1 ref.cast 0 struct.get 0 0 end end} ]}"));
    assert_ok(wah_new_exec_context(&q, &qmod, NULL));
    assert_ok(wah_instantiate(&q));
    assert_ok(wah_new_module(&host, NULL));
    assert_ok(wah_export_func(&host, "f", "(anyref) -> anyref", host_swap_reenter, NULL, NULL));
    assert_ok(wah_new_exec_context(&p, &pmod, NULL));
    assert_ok(wah_link_context(&p, "Q", &q));
    assert_ok(wah_link_module(&p, "h", &host));
    assert_ok(wah_instantiate(&p));
    reentry_ctx = &q;
    prev_pinned = NULL;
    for (int32_t i = 0; i < 4; ++i) {
        assert_ok(wah_call_by_name(&q, "fill", NULL, 0, NULL));
        assert_eq_i32(run(&p, i, WAH_OK), i == 0 ? -1 : 4242);
    }
    wah_free_exec_context(&p);
    wah_free_exec_context(&q);
    prev_pinned = NULL;
    wah_free_module(&pmod);
    wah_free_module(&qmod);
    wah_free_module(&host);
}

static void test_host_ref_before_instantiation(void) {
    printf("Testing host objects given to the first call survive collections in start functions...\n");
    const char *calls[] = { "call", "call_pin", "call_multi", "start" };
    for (int i = 0; i < 4; i++) {
        wah_module_t mod = {0};
        wah_exec_context_t ctx = {0};
        assert_ok(wah_parse_module_from_spec(&mod, "wasm \
            types {[ struct [i32 mut], fn [] [], fn [i64, externref] [externref] ]} \
            funcs {[ 1, 2 ]} start { 0 } \
            code {[ {[1 i32] loop void i32.const 0 struct.new 0 drop \
                        local.get 0 i32.const 1 i32.add local.tee 0 i32.const 200000 i32.lt_u br_if 0 end end}, \
                    {[] local.get 1 end} ]}"));
        assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
        assert_ok(wah_gc_start(&ctx));
        void *host = wah_gc_alloc_host(&ctx, 16);
        assert_true(host != NULL);
        memcpy(host, "host object here", 16);
        // The i64 looks like a pointer to make sure that only references are pinned
        wah_value_t args[2] = { { .i64 = (int64_t)(uintptr_t)host + 64 }, { .ref = host } }, r = {0};
        printf("  via %s\n", calls[i]);
        switch (i) {
        case 0: assert_ok(wah_call(&ctx, 1, args, 2, &r)); break;
        case 1: {
            assert_ok(wah_call_pin(&ctx, 1, args, 2, &r));
            assert_ok(wah_unpin_ref(&ctx, r.ref));
            r.ref = host;
            break;
        }
        case 2: assert_ok(wah_call_multi(&ctx, 1, args, 2, &r, 1, NULL)); break;
        case 3: {
            assert_ok(wah_start(&ctx, 1, args, 2));
            assert_ok(wah_resume(&ctx));
            assert_ok(wah_finish(&ctx, &r, 1, NULL));
            break;
        }
        }
        assert_true(r.ref == host);
        assert_true(memcmp(host, "host object here", 16) == 0);
        wah_free_exec_context(&ctx);
        wah_free_module(&mod);
    }
}

int main(void) {
    test_swap("(externref) -> externref", "externref", "extern.convert_any", "any.convert_extern");
    test_swap("(anyref) -> anyref", "anyref", "", "");
    test_swap_reenter();
    test_non_nullable_host_refs();

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

    test_call_pin();
    test_pin_host_ref();
    test_host_ref_before_instantiation();

    printf("All pin tests passed!\n");
    return 0;
}
