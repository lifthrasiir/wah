// Tests for wahjit.h. Each module is run both natively and interpreted, and their outcomes should agree.
#define WAH_IMPLEMENTATION
#include "../wahjit.h"
#include "common.h"

#ifdef WAH_JIT
#define EXPECT_NATIVE true
#else
#define EXPECT_NATIVE false
#endif

typedef struct {
    wah_module_t module;
    wah_exec_context_t ctx;
} jit_inst_t;

static void inst_new(jit_inst_t *inst, const char *spec, bool disable_jit, wah_module_t *env) {
    wah_parse_options_t opts = { .disable_jit = disable_jit };
    wah_error_t err = wah_parse_module_from_spec_ex(&inst->module, &opts, spec);
    if (err != WAH_OK) printf("  spec: %s\n", spec);
    assert_ok(err);
    assert_ok(wah_new_exec_context(&inst->ctx, &inst->module, NULL));
    if (env) assert_ok(wah_link_module(&inst->ctx, "env", env));
    assert_ok(wah_instantiate(&inst->ctx));
}

static void inst_free(jit_inst_t *inst) {
    wah_free_exec_context(&inst->ctx);
    wah_free_module(&inst->module);
}

static bool ran_natively(const jit_inst_t *inst) {
#ifdef WAH_JIT
    return inst->module.jit && inst->ctx.jit;
#else
    (void)inst;
    return false;
#endif
}

// Calls func_idx with params in both modes and compares errors and results bit by bit.
static void check_call(const char *spec, wah_module_t *env, uint32_t func_idx, const wah_value_t *params,
                       uint32_t nparams, uint32_t nresults) {
    jit_inst_t a, b;
    inst_new(&a, spec, false, env);
    inst_new(&b, spec, true, env);
    wah_value_t ra[8], rb[8];
    memset(ra, 0, sizeof(ra));
    memset(rb, 0, sizeof(rb));
    uint32_t na = 0, nb = 0;
    wah_error_t ea = wah_call_multi(&a.ctx, func_idx, params, nparams, ra, 8, &na);
    wah_error_t eb = wah_call_multi(&b.ctx, func_idx, params, nparams, rb, 8, &nb);
    if (ea != eb) {
        printf("  func %u: native %s vs interpreted %s\n", func_idx, wah_strerror(ea), wah_strerror(eb));
        exit(1);
    }
    assert_true(ran_natively(&a) == EXPECT_NATIVE);
    assert_true(!ran_natively(&b));
    if (ea == WAH_OK) {
        assert_eq_u64(na, nresults);
        assert_eq_u64(nb, nresults);
        for (uint32_t i = 0; i < nresults; i++) {
            if (ra[i].i64 != rb[i].i64) {
                printf("  func %u result %u: native 0x%llx vs interpreted 0x%llx\n", func_idx, i,
                       (unsigned long long)ra[i].i64, (unsigned long long)rb[i].i64);
                exit(1);
            }
        }
    }
    inst_free(&a);
    inst_free(&b);
}

static void check_i32(const char *spec, uint32_t func_idx, int32_t arg) {
    wah_value_t p = { .i32 = arg };
    check_call(spec, NULL, func_idx, &p, 1, 1);
}

static const int32_t I32_SAMPLES[] = { 0, 1, -1, 2, 7, 31, 32, 33, 100, -100, 0x7fffffff, (int32_t)0x80000000, 12345678 };
static const int64_t I64_SAMPLES[] = { 0, 1, -1, 3, 63, 64, 65, 1000, -1000, INT64_MAX, INT64_MIN, 0x123456789abcdefLL };
#define COUNT(a) (sizeof(a) / sizeof(*(a)))

static void test_integer_ops(void) {
    printf("Testing integer operations...\n");
    // Binary operations with both a register and constant operand, also returning operands to check they are kept
    static const char *ops32[] = {
        "i32.add", "i32.sub", "i32.mul", "i32.div_s", "i32.div_u", "i32.rem_s", "i32.rem_u", "i32.and", "i32.or",
        "i32.xor", "i32.shl", "i32.shr_s", "i32.shr_u", "i32.rotl", "i32.rotr", "i32.eq", "i32.ne", "i32.lt_s",
        "i32.lt_u", "i32.gt_s", "i32.gt_u", "i32.le_s", "i32.le_u", "i32.ge_s", "i32.ge_u",
    };
    static const char *ops64[] = {
        "i64.add", "i64.sub", "i64.mul", "i64.div_s", "i64.div_u", "i64.rem_s", "i64.rem_u", "i64.and", "i64.or",
        "i64.xor", "i64.shl", "i64.shr_s", "i64.shr_u", "i64.rotl", "i64.rotr",
    };
    static const char *cmp64[] = {
        "i64.eq", "i64.ne", "i64.lt_s", "i64.lt_u", "i64.gt_s", "i64.gt_u", "i64.le_s", "i64.le_u", "i64.ge_s", "i64.ge_u",
    };
    char spec[1024];
    for (size_t k = 0; k < COUNT(ops32); k++) {
        snprintf(spec, sizeof(spec), "wasm types {[ fn [i32, i32] [i32, i32, i32], fn [i32] [i32, i32] ]} funcs {[ 0, 1, 1, 1 ]} code {[ "
                 "{[] local.get 0 local.get 1 %s local.get 0 local.get 1 end}, {[] local.get 0 i32.const 5 %s local.get 0 end}, "
                 "{[] local.get 0 i32.const -1 %s local.get 0 end}, {[] local.get 0 i32.const 4096 %s local.get 0 end} ]}",
                 ops32[k], ops32[k], ops32[k], ops32[k]);
        for (size_t i = 0; i < COUNT(I32_SAMPLES); i++) {
            for (size_t j = 0; j < COUNT(I32_SAMPLES); j++) {
                wah_value_t p[2] = { { .i32 = I32_SAMPLES[i] }, { .i32 = I32_SAMPLES[j] } };
                check_call(spec, NULL, 0, p, 2, 3);
            }
            for (uint32_t fn = 1; fn <= 3; fn++) check_call(spec, NULL, fn, (wah_value_t[]){ { .i32 = I32_SAMPLES[i] } }, 1, 2);
        }
    }
    for (size_t k = 0; k < COUNT(ops64) + COUNT(cmp64); k++) {
        bool cmp = k >= COUNT(ops64);
        const char *op = cmp ? cmp64[k - COUNT(ops64)] : ops64[k];
        const char *rt = cmp ? "i32" : "i64";
        snprintf(spec, sizeof(spec), "wasm types {[ fn [i64, i64] [%s, i64, i64] ]} funcs {[ 0, 0 ]} code {[ "
                 "{[] local.get 0 local.get 1 %s local.get 0 local.get 1 end}, "
                 "{[] local.get 0 i64.const 3 %s local.get 0 local.get 1 end} ]}", rt, op, op);
        for (size_t i = 0; i < COUNT(I64_SAMPLES); i++) {
            for (size_t j = 0; j < COUNT(I64_SAMPLES); j++) {
                check_call(spec, NULL, 0, (wah_value_t[]){ { .i64 = I64_SAMPLES[i] }, { .i64 = I64_SAMPLES[j] } }, 2, 3);
            }
            check_call(spec, NULL, 1, (wah_value_t[]){ { .i64 = I64_SAMPLES[i] }, { .i64 = 0 } }, 2, 3);
        }
    }
    // Unary operations and conversions
    static const char *un32[] = {
        "i32.clz", "i32.ctz", "i32.popcnt", "i32.eqz", "i32.extend8_s", "i32.extend16_s",
        "i64.extend_i32_s i32.wrap_i64", "i64.extend_i32_u i64.const 32 i64.shr_u i32.wrap_i64",
        "i64.extend_i32_s i64.const 32 i64.shr_s i32.wrap_i64",
    };
    for (size_t k = 0; k < COUNT(un32); k++) {
        snprintf(spec, sizeof(spec), "wasm types {[ fn [i32] [i32, i32] ]} funcs {[ 0 ]} code {[ {[] local.get 0 %s local.get 0 end} ]}", un32[k]);
        for (size_t i = 0; i < COUNT(I32_SAMPLES); i++) check_call(spec, NULL, 0, (wah_value_t[]){ { .i32 = I32_SAMPLES[i] } }, 1, 2);
    }
    static const char *un64[] = { "i64.clz", "i64.ctz", "i64.popcnt", "i64.extend8_s", "i64.extend16_s", "i64.extend32_s" };
    for (size_t k = 0; k < COUNT(un64); k++) {
        snprintf(spec, sizeof(spec), "wasm types {[ fn [i64, i64] [i64, i64] ]} funcs {[ 0 ]} code {[ {[] local.get 0 %s local.get 0 end} ]}", un64[k]);
        for (size_t i = 0; i < COUNT(I64_SAMPLES); i++) check_call(spec, NULL, 0, (wah_value_t[]){ { .i64 = I64_SAMPLES[i] }, { .i64 = 0 } }, 2, 2);
    }
}

static void test_float_ops(void) {
    printf("Testing float operations...\n");
    static const char *bin[] = { "add", "sub", "mul", "div", "min", "max", "copysign" };
    static const char *cmp[] = { "eq", "ne", "lt", "gt", "le", "ge" };
    static const char *un[] = { "abs", "neg", "ceil", "floor", "trunc", "nearest", "sqrt" };
    static const uint64_t f64s[] = {
        0, 0x8000000000000000ull, 0x3ff0000000000000ull, 0xbff8000000000000ull, 0x4004000000000000ull,
        0x7ff0000000000000ull, 0xfff0000000000000ull, 0x7ff8000000000000ull, 0x7ff4000000000001ull,
        0xfff8000000000123ull, 0x0000000000000001ull, 0x43e0000000000000ull, 0xc1e0000000200000ull,
    };
    static const uint32_t f32s[] = {
        0, 0x80000000u, 0x3f800000u, 0xbfc00000u, 0x40200000u, 0x7f800000u, 0xff800000u, 0x7fc00000u,
        0x7fa00001u, 0xffc00123u, 0x00000001u, 0x4f000000u, 0xcf000000u,
    };
    char spec[1024];
    for (int w = 0; w < 2; w++) {
        const char *t = w ? "f64" : "f32";
        size_t n = w ? COUNT(f64s) : COUNT(f32s);
        for (size_t k = 0; k < COUNT(bin) + COUNT(cmp) + COUNT(un); k++) {
            const char *rt = t, *op;
            bool unary = false;
            if (k < COUNT(bin)) op = bin[k];
            else if (k < COUNT(bin) + COUNT(cmp)) { op = cmp[k - COUNT(bin)]; rt = "i32"; }
            else { op = un[k - COUNT(bin) - COUNT(cmp)]; unary = true; }
            snprintf(spec, sizeof(spec), "wasm types {[ fn [%s, %s] [%s, %s, %s] ]} funcs {[ 0 ]} code {[ "
                     "{[] local.get 0 %s %s.%s local.get 0 local.get 1 end} ]}", t, t, rt, t, t, unary ? "" : "local.get 1", t, op);
            for (size_t i = 0; i < n; i++) {
                for (size_t j = 0; j < (unary ? 1 : n); j++) {
                    wah_value_t p[2];
                    memset(p, 0, sizeof(p));
                    if (w) { p[0].i64 = (int64_t)f64s[i]; p[1].i64 = (int64_t)f64s[j]; }
                    else { p[0].i32 = (int32_t)f32s[i]; p[1].i32 = (int32_t)f32s[j]; }
                    check_call(spec, NULL, 0, p, 2, 3);
                }
            }
        }
        // Conversions to integers, trapping and saturating
        static const char *conv[] = {
            "i32.trunc_%s_s", "i32.trunc_%s_u", "i64.trunc_%s_s", "i64.trunc_%s_u",
            "i32.trunc_sat_%s_s", "i32.trunc_sat_%s_u", "i64.trunc_sat_%s_s", "i64.trunc_sat_%s_u",
        };
        for (size_t k = 0; k < COUNT(conv); k++) {
            char op[64];
            snprintf(op, sizeof(op), conv[k], t);
            snprintf(spec, sizeof(spec), "wasm types {[ fn [%s] [%s, %s] ]} funcs {[ 0 ]} code {[ {[] local.get 0 %s local.get 0 end} ]}",
                     t, conv[k][1] == '6' ? "i64" : "i32", t, op);
            for (size_t i = 0; i < n; i++) {
                wah_value_t p;
                memset(&p, 0, sizeof(p));
                if (w) p.i64 = (int64_t)f64s[i];
                else p.i32 = (int32_t)f32s[i];
                check_call(spec, NULL, 0, &p, 1, 2);
            }
        }
    }
    static const char *misc[] = {
        "fn [f64] [f32, f64] ]} funcs {[ 0 ]} code {[ {[] local.get 0 f32.demote_f64 local.get 0 end} ]}",
        "fn [f32] [f64, f32] ]} funcs {[ 0 ]} code {[ {[] local.get 0 f64.promote_f32 local.get 0 end} ]}",
        "fn [f32] [i32, f32] ]} funcs {[ 0 ]} code {[ {[] local.get 0 i32.reinterpret_f32 local.get 0 end} ]}",
        "fn [f64] [i64, f64] ]} funcs {[ 0 ]} code {[ {[] local.get 0 f64.const 1.5f64 f64.add i64.reinterpret_f64 local.get 0 end} ]}",
        "fn [i64] [f32, i64] ]} funcs {[ 0 ]} code {[ {[] local.get 0 f32.convert_i64_s local.get 0 end} ]}",
        "fn [i64] [f32, i64] ]} funcs {[ 0 ]} code {[ {[] local.get 0 f32.convert_i64_u local.get 0 end} ]}",
        "fn [i64] [f64, i64] ]} funcs {[ 0 ]} code {[ {[] local.get 0 i32.wrap_i64 f64.convert_i32_s local.get 0 end} ]}",
        "fn [i64] [f64, i64] ]} funcs {[ 0 ]} code {[ {[] local.get 0 i32.wrap_i64 f64.convert_i32_u local.get 0 end} ]}",
        "fn [i64] [f64, i64] ]} funcs {[ 0 ]} code {[ {[] local.get 0 f64.convert_i64_u f64.const 0.25f64 f64.mul local.get 0 end} ]}",
    };
    for (size_t k = 0; k < COUNT(misc); k++) {
        snprintf(spec, sizeof(spec), "wasm types {[ %s", misc[k]);
        for (size_t i = 0; i < COUNT(f64s); i++) {
            wah_value_t p = { .i64 = (int64_t)f64s[i] };
            check_call(spec, NULL, 0, &p, 1, 2);
            p.i64 = (int64_t)f32s[i];
            check_call(spec, NULL, 0, &p, 1, 2);
        }
    }
}

static void test_control(void) {
    printf("Testing control flow...\n");
    // Sum of 1..n with a loop, br_if fused with a comparison, and a select
    const char *sum = "wasm types {[ fn [i32] [i32] ]} funcs {[ 0 ]} code {[ {[1 i32] \
        block void loop void \
            local.get 0 i32.eqz br_if 1 \
            local.get 1 local.get 0 i32.add local.set 1 \
            local.get 0 i32.const 1 i32.sub local.set 0 br 0 \
        end end \
        local.get 1 i32.const 1000 local.get 1 i32.const 1000 i32.lt_u select end} ]}";
    for (size_t i = 0; i < 6; i++) check_i32(sum, 0, I32_SAMPLES[i] & 0xff);
    check_i32(sum, 0, 30);
    check_i32(sum, 0, 100000);

    // br_table with values carried to a block result, if/else with results
    const char *table = "wasm types {[ fn [i32] [i32] ]} funcs {[ 0 ]} code {[ {[] \
        block i32 block i32 block i32 \
            i32.const 7 local.get 0 i32.const 3 i32.add local.get 0 br_table [0, 1, 2, 0] 1 \
        end i32.const 100 i32.add end i32.const 200 i32.add end \
        local.get 0 i32.const 1 i32.and if i32 i32.const 1 else i32.const 2 end i32.add end} ]}";
    for (int32_t i = -1; i < 7; i++) check_i32(table, 0, i);

    // Recursion and stack exhaustion
    const char *fib = "wasm types {[ fn [i32] [i32] ]} funcs {[ 0, 0 ]} code {[ \
        {[] local.get 0 i32.const 2 i32.lt_s if i32 local.get 0 else \
            local.get 0 i32.const 1 i32.sub call 0 local.get 0 i32.const 2 i32.sub call 0 i32.add end end}, \
        {[] local.get 0 call 1 end} ]}";
    for (int32_t i = 0; i < 20; i += 3) check_i32(fib, 0, i);
    check_i32(fib, 1, 0);

    // Dead code after branches, with nested blocks
    const char *dead = "wasm types {[ fn [i32] [i32] ]} funcs {[ 0 ]} code {[ {[] \
        block i32 local.get 0 br 0 block void i32.const 1 drop end unreachable end \
        local.get 0 if void i32.const 9 return end i32.const 5 i32.add end} ]}";
    check_i32(dead, 0, 0);
    check_i32(dead, 0, 3);
    check_i32("wasm types {[ fn [i32] [i32] ]} funcs {[ 0 ]} code {[ {[] unreachable end} ]}", 0, 0);
}

static void test_memory(void) {
    printf("Testing memory...\n");
    const char *mem = "wasm types {[ fn [i32] [i64], fn [i32] [i32] ]} funcs {[ 0, 1, 1, 1 ]} memories {[ limits.i32/1 1 ]} code {[ \
        {[] local.get 0 i64.const 0x1122334455667788 i64.store 3 0 \
            local.get 0 i32.const 0xabcd i32.store16 1 8 \
            local.get 0 i64.load8_s 0 1 local.get 0 i64.load16_u 1 2 i64.add \
            local.get 0 i64.load32_s 2 4 i64.add local.get 0 i64.load 3 0 i64.xor \
            local.get 0 i32.load16_s 1 8 i64.extend_i32_s i64.add end}, \
        {[] local.get 0 memory.grow 0 memory.size 0 i32.const 16 i32.shl i32.add end}, \
        {[] local.get 0 i32.const 0x55 i32.const 100 memory.fill 0 i32.const 0 local.get 0 i32.const 50 memory.copy 0 0 \
            i32.const 30 i32.load 0 0 end}, \
        {[] local.get 0 f64.const 2.5f64 f64.store 3 0 local.get 0 f32.load 2 4 i32.trunc_sat_f32_s end} ]}";
    static const int32_t addrs[] = { 0, 1, 100, 65520, 65527, 65528, 65535, 65536, -1, -8 };
    for (size_t i = 0; i < COUNT(addrs); i++) {
        wah_value_t p = { .i32 = addrs[i] };
        check_call(mem, NULL, 0, &p, 1, 1);
        check_call(mem, NULL, 2, &p, 1, 1);
        check_call(mem, NULL, 3, &p, 1, 1);
    }
    for (int32_t pages = -1; pages < 3; pages++) check_i32(mem, 1, pages);

    // A second memory
    const char *mem2 = "wasm types {[ fn [i32] [i32] ]} funcs {[ 0 ]} memories {[ limits.i32/1 1, limits.i32/1 2 ]} code {[ \
        {[] local.get 0 i32.const 77 i32.store align=4.mem# 1 0 local.get 0 i32.load align=4.mem# 1 0 \
            local.get 0 i32.load 2 0 i32.add memory.size 1 i32.add end} ]}";
    check_i32(mem2, 0, 100);
    check_i32(mem2, 0, 131068);
    check_i32(mem2, 0, 131069);
}

static void host_twice(wah_call_context_t *call, void *userdata) {
    (void)userdata;
    wah_result_i64(call, 0, wah_param_i64(call, 0) * 2);
    wah_result_i32(call, 1, wah_param_i32(call, 1) + 1);
}

static void host_trap(wah_call_context_t *call, void *userdata) {
    (void)userdata;
    if (wah_param_i32(call, 0) == 13) wah_trap(call, WAH_ERROR_TRAP);
    wah_result_i32(call, 0, wah_param_i32(call, 0));
}

static void test_calls(void) {
    printf("Testing host and indirect calls...\n");
    wah_module_t env;
    assert_ok(wah_new_module(&env, NULL));
    assert_ok(wah_export_func(&env, "twice", "(i64, i32) -> (i64, i32)", host_twice, NULL, NULL));
    assert_ok(wah_export_func(&env, "trap", "(i32) -> i32", host_trap, NULL, NULL));
    const char *spec = "wasm types {[ fn [i64, i32] [i64, i32], fn [i32] [i32] ]} \
        imports {[ {'env'} {'twice'} fn# 0, {'env'} {'trap'} fn# 1 ]} \
        funcs {[ 1, 1, 0 ]} tables {[ funcref limits.i32/1 4 ]} \
        elements {[ elem.active.table#0 i32.const 0 end [1, 3, 4, 2] ]} code {[ \
        {[] i64.const 5 local.get 0 call 0 local.set 0 i32.wrap_i64 local.get 0 i32.add local.get 0 call 1 i32.add end}, \
        {[] local.get 0 local.get 0 local.get 0 i32.const 3 i32.and call_indirect 1 0 i32.add end}, \
        {[] local.get 0 local.get 1 i32.const 1 i32.add end} ]}";
    for (int32_t i = 0; i < 16; i++) {
        wah_value_t p = { .i32 = i };
        check_call(spec, &env, 2, &p, 1, 1);
        check_call(spec, &env, 3, &p, 1, 1);
    }
    wah_free_module(&env);
}

static void host_interrupt(wah_call_context_t *call, void *userdata) {
    (void)userdata;
    wah_request_interrupt_from_host(call);
}

static void test_suspend(void) {
    printf("Testing suspension...\n");
    wah_module_t env;
    assert_ok(wah_new_module(&env, NULL));
    assert_ok(wah_export_func(&env, "interrupt", "() -> ()", host_interrupt, NULL, NULL));
    // Interrupts itself at every 100th iteration, in a loop and before a bulk operation
    const char *spec = "wasm types {[ fn [] [], fn [i32] [i32] ]} imports {[ {'env'} {'interrupt'} fn# 0 ]} \
        funcs {[ 1 ]} memories {[ limits.i32/1 1 ]} code {[ {[1 i32] \
        loop void \
            local.get 1 local.get 0 i32.add local.set 1 \
            local.get 0 i32.const 100 i32.rem_u i32.eqz if void call 0 end \
            local.get 0 i32.const 1 i32.sub local.tee 0 br_if 0 \
        end call 0 i32.const 0 i32.const 1 i32.const 1000 memory.fill 0 \
        i32.const 500 i32.load8_u 0 0 local.get 1 i32.add end} ]}";
    for (int native = 0; native < 2; native++) {
        jit_inst_t a;
        inst_new(&a, spec, !native, &env);
        wah_value_t p = { .i32 = 1000 }, r;
        assert_ok(wah_start(&a.ctx, 1, &p, 1));
        int yields = 0;
        wah_error_t err;
        while ((err = wah_resume(&a.ctx)) == WAH_STATUS_YIELDED) yields++;
        assert_ok(err);
        uint32_t n;
        assert_ok(wah_finish(&a.ctx, &r, 1, &n));
        assert_eq_i32(r.i32, 500500 + 1);
        assert_eq_i32(yields, 11);
        assert_true(ran_natively(&a) == (native && EXPECT_NATIVE));
        // Cancel in the middle, and run again
        assert_ok(wah_start(&a.ctx, 1, &p, 1));
        assert_true(wah_resume(&a.ctx) == WAH_STATUS_YIELDED);
        wah_cancel(&a.ctx);
        assert_ok(wah_start(&a.ctx, 1, &p, 1));
        while ((err = wah_resume(&a.ctx)) == WAH_STATUS_YIELDED) {}
        assert_ok(err);
        assert_ok(wah_finish(&a.ctx, &r, 1, &n));
        assert_eq_i32(r.i32, 500500 + 1);
        inst_free(&a);
    }
    wah_free_module(&env);

    // Infinite recursion runs out of the value stack
    const char *rec = "wasm types {[ fn [i32] [i32] ]} funcs {[ 0 ]} code {[ {[] local.get 0 call 0 end} ]}";
    check_i32(rec, 0, 0);
}

// Locals pinned to registers, with more locals than registers, across suspensions and calls
static void test_pinned_locals(void) {
    printf("Testing pinned locals...\n");
    // Old values on the stack survive setting the local, and sets from constants, slots and other locals
    const char *alias = "wasm types {[ fn [i32] [i32], fn [f64] [f64] ]} funcs {[ 0, 1 ]} code {[ {[1 i32] \
        local.get 0 local.get 0 i32.const 5 i32.add local.set 0 local.get 0 i32.sub \
        local.get 0 local.get 0 i32.const 3 i32.mul local.tee 0 i32.add local.get 0 i32.add i32.add \
        i32.const 7 local.set 1 local.get 1 i32.add \
        block i32 local.get 0 local.get 0 br_if 0 end local.set 1 local.get 1 i32.add \
        local.get 0 local.set 1 local.get 1 local.get 0 local.set 0 i32.add end}, \
        {[] local.get 0 local.get 0 f64.const 1.5f64 f64.mul local.set 0 local.get 0 f64.sub end} ]}";
    for (size_t i = 0; i < COUNT(I32_SAMPLES); i++) check_i32(alias, 0, I32_SAMPLES[i]);
    wah_value_t p = { .f64 = 3.25 };
    check_call(alias, NULL, 1, &p, 1, 1);

    // Pinned locals as operands of select and global.set, read again afterwards
    const char *ops = "wasm types {[ fn [i32, f64, f64] [i32, f64, i32, f64, f64, i32] ]} \
        imports {[ {'env'} {'g'} export.global i32 mut ]} funcs {[ 0 ]} \
        globals {[ i32 mut i32.const 5 end, f64 mut f64.const 0.5f64 end, i64 immut i64.const 7 end ]} code {[ {[] \
        local.get 0 i32.const 3 i32.mul global.set 0 local.get 1 global.set 2 \
        global.get 0 global.get 1 i32.add local.get 0 i32.add global.set 1 \
        global.get 1 global.get 2 local.get 2 f64.add \
        local.get 0 local.get 0 i32.const 1 i32.add local.get 0 select \
        local.get 1 local.get 2 local.get 0 select \
        local.get 1 local.get 2 f64.add global.set 2 global.get 2 local.get 1 f64.add \
        global.get 3 i32.wrap_i64 local.get 0 i32.add end} ]}";
    wah_module_t genv;
    assert_ok(wah_new_module(&genv, NULL));
    assert_ok(wah_export_global_i32(&genv, "g", true, 0));
    for (size_t i = 0; i < COUNT(I32_SAMPLES); i++) {
        wah_value_t q[3] = { { .i32 = I32_SAMPLES[i] }, { .f64 = -1.25 * i }, { .f64 = 2.5 } };
        check_call(ops, &genv, 0, q, 3, 6);
    }
    wah_free_module(&genv);

    // 12 i32 and 2 f64 locals updated in a loop which interrupts itself, and a recursive call
    char spec[4096];
    int n = snprintf(spec, sizeof(spec), "wasm types {[ fn [] [], fn [i32] [i32] ]} imports {[ {'env'} {'interrupt'} fn# 0 ]} \
        funcs {[ 1 ]} code {[ {[12 i32, 2 f64] local.get 0 i32.const 2 i32.lt_u if i32 local.get 0 else loop void");
    for (int k = 1; k <= 12; k++) {
        n += snprintf(spec + n, sizeof(spec) - (size_t)n, " local.get %d local.get 0 i32.add i32.const %d i32.xor local.set %d", k, k, k);
    }
    n += snprintf(spec + n, sizeof(spec) - (size_t)n, " local.get 13 local.get 0 f64.convert_i32_s f64.add local.tee 13 \
        local.get 14 f64.sub local.set 14 \
        local.get 0 i32.const 100 i32.rem_u i32.eqz if void call 0 end \
        local.get 0 i32.const 1 i32.sub local.tee 0 br_if 0 end \
        local.get 13 i32.trunc_f64_s local.get 14 i32.trunc_f64_s i32.add i32.const 1 call 1 i32.add");
    for (int k = 1; k <= 12; k++) n += snprintf(spec + n, sizeof(spec) - (size_t)n, " local.get %d i32.add", k);
    snprintf(spec + n, sizeof(spec) - (size_t)n, " end end} ]}");

    wah_module_t env;
    assert_ok(wah_new_module(&env, NULL));
    assert_ok(wah_export_func(&env, "interrupt", "() -> ()", host_interrupt, NULL, NULL));
    int32_t results[2];
    int yields[2];
    for (int native = 0; native < 2; native++) {
        jit_inst_t a;
        inst_new(&a, spec, !native, &env);
        wah_value_t arg = { .i32 = 1000 }, r;
        assert_ok(wah_start(&a.ctx, 1, &arg, 1));
        wah_error_t err;
        yields[native] = 0;
        while ((err = wah_resume(&a.ctx)) == WAH_STATUS_YIELDED) yields[native]++;
        assert_ok(err);
        uint32_t nr;
        assert_ok(wah_finish(&a.ctx, &r, 1, &nr));
        results[native] = r.i32;
        assert_true(ran_natively(&a) == (native && EXPECT_NATIVE));
        inst_free(&a);
    }
    assert_eq_i32(results[1], results[0]);
    assert_eq_i32(yields[1], yields[0]);
    assert_eq_i32(yields[0], 10);
    wah_free_module(&env);
}

// Unpinned locals whose values are reused from registers after local.set and local.tee
static void test_local_cache(void) {
    printf("Testing values of locals kept in registers...\n");
    // Locals 1-20 are pinned by a loop, so that 21-25 and 26-28 (f64) stay in slots
    char spec[8192];
    size_t cap = sizeof(spec);
    int n = snprintf(spec, cap, "wasm types {[ fn [i32] [i32], fn [i32] [i64] ]} funcs {[ 0, 1 ]} code {[ \
        {[] local.get 0 i32.const 7 i32.mul end}, {[25 i32, 3 f64] \
        local.get 0 i32.const 3 i32.and local.set 1 block void loop void");
    for (int k = 2; k <= 20; k++) {
        n += snprintf(spec + n, cap - (size_t)n, " local.get %d local.get %d i32.add i32.const %d i32.xor local.set %d",
                      k, k - 1, k, k);
    }
    n += snprintf(spec + n, cap - (size_t)n, " local.get 1 i32.const 1 i32.add local.tee 1 i32.const 9 i32.lt_u br_if 0 end end \
        local.get 0 i32.const 3 i32.mul local.tee 21 i32.const 1 i32.add local.set 22 \
        local.get 21 local.get 22 i32.add local.set 23 \
        local.get 23 local.get 0 local.set 23 local.get 23 i32.sub local.set 24 \
        local.get 0 i32.const 5 i32.add local.set 25 local.get 0 i32.const 6 i32.add local.set 25 \
        local.get 0 i32.const 9 i32.add local.set 22 i32.const 77 local.set 22 local.get 22 local.get 23 i32.add local.set 23 \
        local.get 0 i32.const 4 i32.add local.set 22 local.get 2 local.set 22 local.get 22 local.get 23 i32.add local.set 23");
    for (int k = 1; k <= 8; k++) n += snprintf(spec + n, cap - (size_t)n, " local.get 0 i32.const %d i32.mul", k * 11);
    for (int k = 1; k < 8; k++) n += snprintf(spec + n, cap - (size_t)n, " i32.add");
    n += snprintf(spec + n, cap - (size_t)n, " local.get 25 i32.add local.get 24 i32.add local.set 24 \
        local.get 0 f64.convert_i32_s f64.const 1.5f64 f64.add local.tee 26 f64.sqrt local.set 27 \
        local.get 26 local.get 27 f64.mul local.tee 28 local.get 26 local.set 28 local.get 28 f64.sub local.set 26 \
        local.get 21 call 0 local.set 22 local.get 21 local.get 22 i32.add local.set 21 \
        local.get 21 i64.extend_i32_u i64.const 32 i64.shl local.get 24 i64.extend_i32_u i64.or \
        local.get 23 local.get 25 i32.add i64.extend_i32_s i64.add \
        local.get 26 i64.trunc_sat_f64_s i64.add local.get 27 i64.trunc_sat_f64_s i64.add");
    for (int k = 1; k <= 20; k++) n += snprintf(spec + n, cap - (size_t)n, " local.get %d i64.extend_i32_u i64.add", k);
    snprintf(spec + n, cap - (size_t)n, " end} ]}");
    for (size_t i = 0; i < COUNT(I32_SAMPLES); i++) {
        wah_value_t p = { .i32 = I32_SAMPLES[i] };
        check_call(spec, NULL, 1, &p, 1, 1);
    }
}

static void test_fallback(void) {
    printf("Testing interpreter fallback...\n");
    // Modules using reference types are interpreted
    const char *spec = "wasm types {[ fn [] [i32] ]} funcs {[ 0 ]} code {[ {[] ref.null funcref ref.is_null end} ]}";
    jit_inst_t a;
    inst_new(&a, spec, false, NULL);
    wah_value_t r;
    assert_ok(wah_call(&a.ctx, 0, NULL, 0, &r));
    assert_eq_i32(r.i32, 1);
#ifdef WAH_JIT
    assert_true(a.module.jit == NULL);
#endif
    inst_free(&a);
}

int main(void) {
    test_integer_ops();
    test_float_ops();
    test_control();
    test_memory();
    test_calls();
    test_suspend();
    test_pinned_locals();
    test_local_cache();
    test_fallback();
    printf("All JIT tests passed.\n");
    return 0;
}
