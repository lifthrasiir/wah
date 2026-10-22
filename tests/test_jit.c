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

static void print_value(const char *label, const wah_value_t *v) {
    printf(" %s ", label);
    for (int i = 15; i >= 0; i--) printf("%02x%s", v->v128.u8[i], i == 8 ? "_" : "");
}

// Calls func_idx with each of nsets sets of params in both modes and compares errors and results bit by bit.
// Unused bytes of results are zeroes, so whole values are compared.
static void check_calls(const char *spec, wah_module_t *env, uint32_t func_idx, const wah_value_t *params,
                        uint32_t nsets, uint32_t nparams, uint32_t nresults) {
    jit_inst_t a, b;
    inst_new(&a, spec, false, env);
    inst_new(&b, spec, true, env);
    for (uint32_t s = 0; s < nsets; s++) {
        const wah_value_t *p = params + (size_t)s * nparams;
        wah_value_t ra[8], rb[8];
        memset(ra, 0, sizeof(ra));
        memset(rb, 0, sizeof(rb));
        uint32_t na = 0, nb = 0;
        wah_error_t ea = wah_call_multi(&a.ctx, func_idx, p, nparams, ra, 8, &na);
        wah_error_t eb = wah_call_multi(&b.ctx, func_idx, p, nparams, rb, 8, &nb);
        if (ea != eb) {
            printf("  func %u: native %s vs interpreted %s\n  spec: %s\n", func_idx, wah_strerror(ea), wah_strerror(eb), spec);
            exit(1);
        }
        assert_true(ran_natively(&a) == EXPECT_NATIVE);
        assert_true(!ran_natively(&b));
        if (ea != WAH_OK) continue;
        assert_eq_u64(na, nresults);
        assert_eq_u64(nb, nresults);
        for (uint32_t i = 0; i < nresults; i++) {
            if (memcmp(&ra[i], &rb[i], sizeof(wah_value_t)) != 0) {
                printf("  func %u result %u:", func_idx, i);
                print_value("native", &ra[i]);
                print_value("vs interpreted", &rb[i]);
                printf("\n  params:");
                for (uint32_t j = 0; j < nparams; j++) print_value("", &p[j]);
                printf("\n  spec: %s\n", spec);
                exit(1);
            }
        }
    }
    inst_free(&a);
    inst_free(&b);
}

static void check_call(const char *spec, wah_module_t *env, uint32_t func_idx, const wah_value_t *params,
                       uint32_t nparams, uint32_t nresults) {
    check_calls(spec, env, func_idx, params, 1, nparams, nresults);
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

    // Offsets of 16 MiB or more, which need more than an immediate add on AArch64
    const char *big = "wasm types {[ fn [i32] [i32] ]} funcs {[ 0 ]} memories {[ limits.i32/1 257 ]} code {[ \
        {[] local.get 0 local.get 0 i32.store 2 0x1000000 local.get 0 i32.load 2 0x1000000 \
            i32.const 0 i32.load 2 0x1000000 i32.add end} ]}";
    static const int32_t big_addrs[] = { 0, 4, 65532, 65533, 65536, -1 };
    for (size_t i = 0; i < COUNT(big_addrs); i++) check_i32(big, 0, big_addrs[i]);
}

// memory64 is always bounds-checked, also with guard pages, along with 32-bit memories in the same module
static void test_memory64(void) {
    printf("Testing memory64...\n");
    static const int64_t addrs[] = {
        0, 1, 8, 100, 528, 535, 536, 65520, 65527, 65528, 65535, 65536, -1, -8, -16, 16, 0x100000000LL,
        -0x7ffffff0LL, -0x80000000LL, -0x100000000LL, INT64_MAX, INT64_MIN,
    };
    static wah_value_t params[COUNT(addrs)];
    for (size_t i = 0; i < COUNT(addrs); i++) params[i].i64 = addrs[i];

    // Accesses at an offset to memories 0 and 2 (both 64-bit), the last one being u64 LEB128 of 0xfffffffffffffff0
    static const char *offsets[] = { "0", "7", "65000", "0x7ffffff0", "0x80000000", "0x100000000", "%'f0ffffffffffffffff01'" };
    char spec[4096];
    for (size_t k = 0; k < COUNT(offsets); k++) {
        const char *o = offsets[k];
        snprintf(spec, sizeof(spec), "wasm types {[ fn [i64] [i64] ]} funcs {[ 0, 0, 0, 0 ]} \
            memories {[ limits.i64/1 1, limits.i32/1 1, limits.i64/1 1 ]} code {[ \
            {[] local.get 0 local.get 0 i64.store 3 %s local.get 0 i64.load 3 %s \
                local.get 0 i32.load8_u 0 %s i64.extend_i32_u i64.add end}, \
            {[] local.get 0 local.get 0 i64.const 3 i64.mul i64.store 67 2 %s local.get 0 i64.load32_s 66 2 %s \
                local.get 0 i32.const 0x1234 i32.store16 65 2 %s local.get 0 i64.load16_u 65 2 %s i64.add end}, \
            {[] local.get 0 i64.const 0 i64x2.splat v128.store 4 %s local.get 0 local.get 0 v128.load 4 %s \
                local.get 0 v128.load16_splat 1 %s i16x8.add v128.load32_lane 2 %s 1 i64x2.extract_lane 1 end}, \
            {[] local.get 0 local.get 0 i64x2.splat i32x4.abs v128.store32_lane 2 %s 3 local.get 0 i64.load 3 %s end} ]}",
            o, o, o, o, o, o, o, o, o, o, o, o, o);
        for (uint32_t fn = 0; fn < 4; fn++) check_calls(spec, NULL, fn, params, COUNT(addrs), 1, 1);
    }

    // memory.size and memory.grow
    const char *grow = "wasm types {[ fn [i64] [i64] ]} funcs {[ 0, 0 ]} \
        memories {[ limits.i64/2 1 3, limits.i32/1 1, limits.i64/2 0 2 ]} code {[ \
        {[] local.get 0 memory.grow 0 memory.size 0 i64.const 16 i64.shl i64.add \
            local.get 0 i64.const 65536 i64.mul i64.const 8 i64.sub i64.load 3 0 i64.add end}, \
        {[] local.get 0 memory.grow 2 memory.size 2 i64.add memory.size 1 i64.extend_i32_u i64.add \
            i64.const 65528 i64.load 67 2 0 i64.add end} ]}";
    static const int64_t pages[] = { -1, 0, 1, 2, 3, 0x100000000LL, INT64_MIN };
    for (size_t i = 0; i < COUNT(pages); i++) {
        wah_value_t p = { .i64 = pages[i] };
        check_call(grow, NULL, 0, &p, 1, 1);
        check_call(grow, NULL, 1, &p, 1, 1);
    }

    // Bulk operations, including copies between 32-bit and 64-bit memories
    const char *bulk = "wasm types {[ fn [i64] [i64] ]} funcs {[ 0, 0, 0, 0, 0 ]} \
        memories {[ limits.i64/1 1, limits.i32/1 1, limits.i64/1 1 ]} datacount {1} code {[ \
        {[] local.get 0 i32.const 0x55 i64.const 100 memory.fill 0 i64.const 0 local.get 0 i64.const 50 memory.copy 0 0 \
            i64.const 30 i64.load 3 0 end}, \
        {[] i64.const 0 i32.const 0x66 i64.const 64 memory.fill 0 \
            i32.const 1000 local.get 0 i32.const 40 memory.copy 1 0 i32.const 1008 i64.load 67 1 0 end}, \
        {[] i32.const 0 i32.const 0x77 i32.const 64 memory.fill 1 \
            local.get 0 i32.const 4 i32.const 20 memory.copy 0 1 local.get 0 i64.load 3 0 end}, \
        {[] local.get 0 i32.const 1 i32.const 3 memory.init 0 2 local.get 0 i64.load32_u 66 2 0 \
            local.get 0 i32.const 0x11 i64.const 9 memory.fill 2 i64.const 0 local.get 0 i64.const 9 memory.copy 0 2 \
            i64.const 0 i64.load 3 0 i64.add end}, \
        {[] local.get 0 i32.const 0 i32.const 4 memory.init 0 0 local.get 0 i64.load32_u 2 0 end} ]} \
        data {[ data.passive {%'aabbccdd'} ]}";
    for (uint32_t fn = 0; fn < 5; fn++) check_calls(bulk, NULL, fn, params, COUNT(addrs), 1, 1);
}

// Results computed right into pinned registers of the locals they set, where operands may be the same registers
static void test_result_hints(void) {
    printf("Testing results set to pinned locals...\n");
    static const char *ops[] = { "add", "sub", "mul", "and", "or", "xor", "shl", "shr_s", "shr_u", "rotl", "rotr" };
    char spec[2048];
    for (int is64 = 0; is64 < 2; is64++) {
        const char *t = is64 ? "i64" : "i32";
        for (size_t k = 0; k < COUNT(ops); k++) {
            const char *o = ops[k];
            snprintf(spec, sizeof(spec), "wasm types {[ fn [%s, %s] [%s, %s] ]} funcs {[ 0 ]} code {[ {[] "
                "local.get 0 local.get 1 %s.%s local.set 0 "                                      // x = x op y
                "local.get 1 local.get 0 %s.%s local.set 0 "                                      // x = y op x
                "local.get 0 local.get 0 %s.%s local.set 1 "                                      // y = x op x
                "local.get 1 local.get 1 %s.%s local.set 1 "                                      // y = y op y
                "local.get 0 %s.const 7 %s.%s local.set 0 "                                       // x = x op 7
                "%s.const 7 local.get 0 %s.%s local.set 0 "                                       // x = 7 op x
                "local.get 1 local.get 0 %s.%s local.tee 0 local.get 0 %s.%s local.set 1 "        // tee
                "local.get 0 local.get 1 local.get 0 %s.%s local.set 0 local.get 0 %s.%s local.set 1 " // old x on the stack
                "local.get 0 local.get 1 end} ]}",
                t, t, t, t, t, o, t, o, t, o, t, o, t, t, o, t, t, o, t, o, t, o, t, o, t, o);
            static const int64_t samples[][2] = { { 0, 0 }, { 1, 2 }, { -1, 3 }, { 0x7fffffff, -2 }, { 0x123456789LL, 37 } };
            for (size_t i = 0; i < COUNT(samples); i++) {
                wah_value_t p[2];
                if (is64) p[0].i64 = samples[i][0], p[1].i64 = samples[i][1];
                else p[0].i32 = (int32_t)samples[i][0], p[1].i32 = (int32_t)samples[i][1];
                check_call(spec, NULL, 0, p, 2, 2);
            }
        }
    }
    // Loads, also from the address in the local being set
    const char *loads = "wasm types {[ fn [i32, i32] [i32, i32] ]} funcs {[ 0 ]} memories {[ limits.i32/1 1 ]} code {[ {[1 i64] \
        local.get 0 i32.const 0xfff0 i32.and local.tee 0 local.get 1 i32.store 2 0 \
        local.get 0 i32.const 4 i32.add local.get 0 i32.store 2 0 \
        local.get 0 i32.load 2 0 local.set 1 local.get 0 i32.load 2 4 local.set 0 local.get 0 i32.load16_s 1 1 local.set 0 \
        local.get 0 i32.const 0xfff0 i32.and local.tee 0 i64.load8_s 0 3 local.set 2 \
        local.get 0 local.get 0 i32.load8_u 0 0 local.set 0 local.get 0 i32.sub local.set 1 \
        local.get 0 local.get 1 local.get 2 i32.wrap_i64 i32.add end} ]}";
    for (size_t i = 0; i < COUNT(I32_SAMPLES); i++) {
        wah_value_t p[2] = { { .i32 = I32_SAMPLES[i] }, { .i32 = I32_SAMPLES[COUNT(I32_SAMPLES) - 1 - i] } };
        check_call(loads, NULL, 0, p, 2, 2);
    }
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
// Bulk operations on memory64 yielding in the middle by a deadline, with their 64-bit operands updated
static void test_memory64_bulk_yield(void) {
    printf("Testing memory64 bulk operations yielding in the middle...\n");
    const char *spec = "wasm types {[ fn [] [i64] ]} funcs {[ 0 ]} \
        memories {[ limits.i64/1 512, limits.i32/1 512 ]} code {[ {[] \
        i64.const 1 i32.const 0x5a i64.const 0x1fffffe memory.fill 0 \
        i64.const 3 i64.const 0 i64.const 0x1fffff0 memory.copy 0 0 \
        i64.const 0 i64.const 5 i64.const 0x1fffff0 memory.copy 0 0 \
        i32.const 7 i64.const 0 i32.const 0x1fffff0 memory.copy 1 0 \
        i64.const 0x1fffff8 i64.load 3 0 i64.const 0x1000000 i64.load 3 0 i64.add \
        i32.const 0x1fffff0 i64.load 67 1 0 i64.add i32.const 0 i64.load 67 1 0 i64.add end} ]}";
    wah_value_t r[2];
    for (int native = 0; native < 2; native++) {
        jit_inst_t a;
        inst_new(&a, spec, !native, NULL);
        wah_limits_t lim = { .deadline_us = 50 };
        assert_ok(wah_set_limits(&a.ctx, &lim));
        assert_ok(wah_start(&a.ctx, 0, NULL, 0));
        int yields = 0;
        wah_error_t err;
        while ((err = wah_resume(&a.ctx)) == WAH_STATUS_YIELDED) {
            yields++;
            assert_ok(wah_set_limits(&a.ctx, &lim));
        }
        assert_ok(err);
        uint32_t n;
        assert_ok(wah_finish(&a.ctx, &r[native], 1, &n));
        assert_true(yields > 0);
        assert_true(ran_natively(&a) == (native && EXPECT_NATIVE));
        inst_free(&a);
    }
    assert_eq_i64(r[0].i64, r[1].i64);
}

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

// i32 values whose 64-bit representations may have upper bits set, used as addresses, br_table indices and
// zero-extended operands. Params are given with garbage in their upper halves, which wasm can't observe.
static void test_i32_upper_bits(void) {
    printf("Testing upper bits of i32 values...\n");
    const char *spec = "wasm types {[ fn [i32] [i64], fn [i32] [i32] ]} funcs {[ 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 1, 0 ]} \
        memories {[ limits.i32/2 1 2 ]} globals {[ i32 mut i32.const -1 end ]} code {[ \
        {[] local.get 0 i64.extend_i32_u end}, \
        {[] local.get 0 i32.load 2 0 end}, \
        {[] i32.const -1 i64.extend_i32_u end}, \
        {[] i32.const -4 i32.load 2 0 end}, \
        {[] local.get 0 i64.extend_i32_s i64.const 0x100000000 i64.or i32.wrap_i64 i64.extend_i32_u end}, \
        {[] local.get 0 i64.extend_i32_u i64.const 0x700000000 i64.or i32.wrap_i64 i32.load 2 0 end}, \
        {[] local.get 0 f64.convert_i32_s f32.demote_f64 i32.reinterpret_f32 i64.extend_i32_u end}, \
        {[] local.get 0 memory.grow 0 i64.extend_i32_u end}, \
        {[] global.get 0 i64.extend_i32_u end}, \
        {[1 i32] i32.const -4 local.set 1 local.get 1 i32.load 2 0 end}, \
        {[] block void block void block void \
            local.get 0 i64.extend_i32_u i64.const 0x100000000 i64.or i32.wrap_i64 br_table [0, 1] 2 \
            end i32.const 10 return end i32.const 11 return end i32.const 12 end}, \
        {[] local.get 0 f64.convert_i32_u i64.trunc_sat_f64_u end} ]}";
    static const int32_t args[] = { 0, 1, 4, -1, -4, 65532, 65533, 0x10000, (int32_t)0x80000000 };
    for (size_t i = 0; i < COUNT(args); i++) {
        wah_value_t p = { .i64 = (int64_t)(0xdeadbeef00000000ull | (uint32_t)args[i]) };
        for (uint32_t fn = 0; fn < 12; fn++) check_call(spec, NULL, fn, &p, 1, 1);
    }
}

// Vectors with lane boundaries, special floats and edge cases of conversions
static const wah_v128_t V_SAMPLES[] = {
    { .u64 = { 0, 0 } },
    { .u64 = { UINT64_MAX, UINT64_MAX } },
    { .u8 = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 } },
    { .u8 = { 0x80, 0x7f, 0x01, 0xfe, 0x00, 0xff, 0x40, 0xc0, 0x10, 0x20, 0x30, 0x0f, 0xf0, 0x55, 0xaa, 0x81 } },
    { .u32 = { 0x3fc00000, 0xc0200000, 0x7fc00000, 0x80000000 } },  // 1.5, -2.5, NaN, -0
    { .u32 = { 0x7f800000, 0xff800000, 0x7fa00001, 0xffc00123 } },  // inf, -inf, sNaN, negative NaN with payload
    { .u32 = { 0x4f000000, 0xcf000001, 0x4f800000, 0x3efffffd } },  // 2^31, -2^31 - 256, 2^32, 0.4999999
    { .u32 = { 0x00000001, 0x80000001, 0x4f32d05e, 0xbf800000 } },  // Subnormals, 3e9, -1
    { .u32 = { 0x40200000, 0x40600000, 0xbf000000, 0x4b000001 } },  // 2.5, 3.5, -0.5, 8388609
    { .u64 = { 0x3ff8000000000000ull, 0x7ff8000000000000ull } },   // 1.5, NaN
    { .u64 = { 0x8000000000000000ull, 0xfff8000000000123ull } },   // -0, negative NaN with payload
    { .u64 = { 0x41effffffff00000ull, 0xc1e0000000180000ull } },   // 4294967295.5, -2147483648.75
    { .u64 = { 0x7ff0000000000000ull, 0x7e37e43c8800759cull } },   // inf, 1e300
    { .u64 = { 0xc1e0000000200000ull, 0x41dfffffffd00000ull } },   // -2147483649, 2147483647.25
    { .u16 = { 0x8000, 0x7fff, 1, 0xffff, 0x4000, 0xc000, 12345, 0x8001 } },
    { .u32 = { 0x80000000u, 0x7fffffffu, 0x8000, 0xffffffffu } },
};
#define NV COUNT(V_SAMPLES)

// Hex digits of a vector for `v128.const %'...'` in specs
static const char *v128_hex(const wah_v128_t *v, char *buf) {
    for (int i = 0; i < 16; i++) snprintf(buf + 2 * i, 3, "%02x", v->u8[i]);
    return buf;
}

static void test_simd_binary(void) {
    printf("Testing binary SIMD operations...\n");
    static const char *ops[] = {
        "v128.and", "v128.andnot", "v128.or", "v128.xor", "i8x16.swizzle",
        "i8x16.eq", "i8x16.ne", "i8x16.lt_s", "i8x16.lt_u", "i8x16.gt_s", "i8x16.gt_u", "i8x16.le_s", "i8x16.le_u", "i8x16.ge_s", "i8x16.ge_u",
        "i16x8.eq", "i16x8.ne", "i16x8.lt_s", "i16x8.lt_u", "i16x8.gt_s", "i16x8.gt_u", "i16x8.le_s", "i16x8.le_u", "i16x8.ge_s", "i16x8.ge_u",
        "i32x4.eq", "i32x4.ne", "i32x4.lt_s", "i32x4.lt_u", "i32x4.gt_s", "i32x4.gt_u", "i32x4.le_s", "i32x4.le_u", "i32x4.ge_s", "i32x4.ge_u",
        "i64x2.eq", "i64x2.ne", "i64x2.lt_s", "i64x2.gt_s", "i64x2.le_s", "i64x2.ge_s",
        "f32x4.eq", "f32x4.ne", "f32x4.lt", "f32x4.gt", "f32x4.le", "f32x4.ge",
        "f64x2.eq", "f64x2.ne", "f64x2.lt", "f64x2.gt", "f64x2.le", "f64x2.ge",
        "i8x16.narrow_i16x8_s", "i8x16.narrow_i16x8_u", "i16x8.narrow_i32x4_s", "i16x8.narrow_i32x4_u",
        "i8x16.add", "i8x16.add_sat_s", "i8x16.add_sat_u", "i8x16.sub", "i8x16.sub_sat_s", "i8x16.sub_sat_u",
        "i8x16.min_s", "i8x16.min_u", "i8x16.max_s", "i8x16.max_u", "i8x16.avgr_u",
        "i16x8.add", "i16x8.add_sat_s", "i16x8.add_sat_u", "i16x8.sub", "i16x8.sub_sat_s", "i16x8.sub_sat_u", "i16x8.mul",
        "i16x8.min_s", "i16x8.min_u", "i16x8.max_s", "i16x8.max_u", "i16x8.avgr_u", "i16x8.q15mulr_sat_s",
        "i16x8.extmul_low_i8x16_s", "i16x8.extmul_high_i8x16_s", "i16x8.extmul_low_i8x16_u", "i16x8.extmul_high_i8x16_u",
        "i32x4.add", "i32x4.sub", "i32x4.mul", "i32x4.min_s", "i32x4.min_u", "i32x4.max_s", "i32x4.max_u", "i32x4.dot_i16x8_s",
        "i32x4.extmul_low_i16x8_s", "i32x4.extmul_high_i16x8_s", "i32x4.extmul_low_i16x8_u", "i32x4.extmul_high_i16x8_u",
        "i64x2.add", "i64x2.sub", "i64x2.mul",
        "i64x2.extmul_low_i32x4_s", "i64x2.extmul_high_i32x4_s", "i64x2.extmul_low_i32x4_u", "i64x2.extmul_high_i32x4_u",
        "f32x4.add", "f32x4.sub", "f32x4.mul", "f32x4.div", "f32x4.min", "f32x4.max", "f32x4.pmin", "f32x4.pmax",
        "f64x2.add", "f64x2.sub", "f64x2.mul", "f64x2.div", "f64x2.min", "f64x2.max", "f64x2.pmin", "f64x2.pmax",
        "i8x16.relaxed_swizzle", "f32x4.relaxed_min", "f32x4.relaxed_max", "f64x2.relaxed_min", "f64x2.relaxed_max",
        "i16x8.relaxed_q15mulr_s", "i16x8.relaxed_dot_i8x16_i7x16_s",
    };
    static wah_value_t params[NV * NV * 2];
    for (size_t i = 0; i < NV; i++) {
        for (size_t j = 0; j < NV; j++) {
            params[2 * (i * NV + j)].v128 = V_SAMPLES[i];
            params[2 * (i * NV + j) + 1].v128 = V_SAMPLES[j];
        }
    }
    char spec[2048], c[33];
    v128_hex(&V_SAMPLES[3], c);
    for (size_t k = 0; k < COUNT(ops); k++) {
        const char *op = ops[k];
        // Operands are pinned locals, temporaries, slots, the same register or constants
        snprintf(spec, sizeof(spec), "wasm types {[ fn [v128, v128] [v128, v128, v128] ]} funcs {[ 0, 0, 0, 0, 0, 0 ]} code {[ "
                 "{[] local.get 0 local.get 1 %s local.get 0 local.get 1 end}, "
                 "{[] local.get 0 v128.not v128.not local.get 1 v128.not v128.not %s local.get 0 local.get 1 end}, "
                 "{[] local.get 0 local.get 1 v128.not v128.not %s local.get 0 local.get 1 end}, "
                 "{[] local.get 0 local.get 1 i32.const 0 if void end %s local.get 0 local.get 1 end}, "
                 "{[] local.get 0 local.get 0 %s local.get 0 v128.const %%'%s' %s local.get 1 local.get 1 %s end}, "
                 "{[] v128.const %%'%s' local.get 1 %s local.get 0 v128.not local.get 1 end} ]}",
                 op, op, op, op, op, c, op, op, c, op);
        for (uint32_t fn = 0; fn < 6; fn++) check_calls(spec, NULL, fn, params, NV * NV, 2, 3);
    }
}

static void test_simd_unary(void) {
    printf("Testing unary and ternary SIMD operations...\n");
    static const char *ops[] = {
        "v128.not", "i8x16.abs", "i8x16.neg", "i8x16.popcnt",
        "i16x8.extadd_pairwise_i8x16_s", "i16x8.extadd_pairwise_i8x16_u", "i16x8.abs", "i16x8.neg",
        "i16x8.extend_low_i8x16_s", "i16x8.extend_high_i8x16_s", "i16x8.extend_low_i8x16_u", "i16x8.extend_high_i8x16_u",
        "i32x4.extadd_pairwise_i16x8_s", "i32x4.extadd_pairwise_i16x8_u", "i32x4.abs", "i32x4.neg",
        "i32x4.extend_low_i16x8_s", "i32x4.extend_high_i16x8_s", "i32x4.extend_low_i16x8_u", "i32x4.extend_high_i16x8_u",
        "i64x2.abs", "i64x2.neg",
        "i64x2.extend_low_i32x4_s", "i64x2.extend_high_i32x4_s", "i64x2.extend_low_i32x4_u", "i64x2.extend_high_i32x4_u",
        "f32x4.ceil", "f32x4.floor", "f32x4.trunc", "f32x4.nearest", "f32x4.abs", "f32x4.neg", "f32x4.sqrt",
        "f64x2.ceil", "f64x2.floor", "f64x2.trunc", "f64x2.nearest", "f64x2.abs", "f64x2.neg", "f64x2.sqrt",
        "i32x4.trunc_sat_f32x4_s", "i32x4.trunc_sat_f32x4_u", "f32x4.convert_i32x4_s", "f32x4.convert_i32x4_u",
        "i32x4.trunc_sat_f64x2_s_zero", "i32x4.trunc_sat_f64x2_u_zero", "f64x2.convert_low_i32x4_s", "f64x2.convert_low_i32x4_u",
        "f32x4.demote_f64x2_zero", "f64x2.promote_low_f32x4",
        "i32x4.relaxed_trunc_f32x4_s", "i32x4.relaxed_trunc_f32x4_u", "i32x4.relaxed_trunc_f64x2_s", "i32x4.relaxed_trunc_f64x2_u",
    };
    static wah_value_t params[NV];
    for (size_t i = 0; i < NV; i++) params[i].v128 = V_SAMPLES[i];
    char spec[2048], c[33];
    for (size_t k = 0; k < COUNT(ops); k++) {
        const char *op = ops[k];
        snprintf(spec, sizeof(spec), "wasm types {[ fn [v128] [v128, v128] ]} funcs {[ 0, 0, 0, 0 ]} code {[ "
                 "{[] local.get 0 %s local.get 0 end}, {[] local.get 0 v128.not v128.not %s local.get 0 end}, "
                 "{[] local.get 0 i32.const 0 if void end %s local.get 0 end}, {[] v128.const %%'%s' %s local.get 0 end} ]}",
                 op, op, op, v128_hex(&V_SAMPLES[6], c), op);
        for (uint32_t fn = 0; fn < 4; fn++) check_calls(spec, NULL, fn, params, NV, 1, 2);
    }

    static const char *ternary[] = {
        "v128.bitselect", "f32x4.relaxed_madd", "f32x4.relaxed_nmadd", "f64x2.relaxed_madd", "f64x2.relaxed_nmadd",
        "i8x16.relaxed_laneselect", "i16x8.relaxed_laneselect", "i32x4.relaxed_laneselect", "i64x2.relaxed_laneselect",
        "i32x4.relaxed_dot_i8x16_i7x16_add_s",
    };
    static const size_t thirds[] = { 0, 1, 3, 4, 12 };
    static wah_value_t params3[NV * NV * COUNT(thirds) * 3];
    size_t n = 0;
    for (size_t i = 0; i < NV; i++) {
        for (size_t j = 0; j < NV; j++) {
            for (size_t l = 0; l < COUNT(thirds); l++) {
                params3[n++].v128 = V_SAMPLES[i];
                params3[n++].v128 = V_SAMPLES[j];
                params3[n++].v128 = V_SAMPLES[thirds[l]];
            }
        }
    }
    for (size_t k = 0; k < COUNT(ternary); k++) {
        const char *op = ternary[k];
        snprintf(spec, sizeof(spec), "wasm types {[ fn [v128, v128, v128] [v128, v128, v128, v128] ]} funcs {[ 0, 0, 0, 0, 0, 0 ]} code {[ "
                 "{[] local.get 0 local.get 1 local.get 2 %s local.get 0 local.get 1 local.get 2 end}, "
                 "{[] local.get 0 v128.not v128.not local.get 1 v128.not v128.not local.get 2 v128.not v128.not %s "
                 "local.get 0 local.get 1 local.get 2 end}, "
                 "{[] local.get 0 local.get 1 local.get 2 v128.not v128.not %s local.get 0 local.get 1 local.get 2 end}, "
                 "{[] local.get 0 v128.not v128.not local.get 1 local.get 2 %s local.get 0 local.get 1 local.get 2 end}, "
                 "{[] local.get 0 local.get 1 v128.not v128.not local.get 2 %s local.get 0 local.get 1 local.get 2 end}, "
                 "{[] local.get 0 local.get 1 local.get 2 i32.const 0 if void end %s local.get 0 local.get 1 local.get 2 end} ]}",
                 op, op, op, op, op, op);
        for (uint32_t fn = 0; fn < 6; fn++) check_calls(spec, NULL, fn, params3, (uint32_t)(n / 3), 3, 4);
    }
}

static void test_simd_tests_and_shifts(void) {
    printf("Testing SIMD tests, bitmasks and shifts...\n");
    // Samples plus vectors with a single zero byte or a single negative byte
    static wah_value_t params[NV + 32];
    for (size_t i = 0; i < NV; i++) params[i].v128 = V_SAMPLES[i];
    for (int i = 0; i < 16; i++) {
        memset(&params[NV + i], 1, 16);
        params[NV + i].v128.u8[i] = 0;
        memset(&params[NV + 16 + i], 0, sizeof(wah_value_t));
        params[NV + 16 + i].v128.u8[i] = 0x80;
    }
    static const char *tests[] = {
        "v128.any_true", "i8x16.all_true", "i16x8.all_true", "i32x4.all_true", "i64x2.all_true",
        "i8x16.bitmask", "i16x8.bitmask", "i32x4.bitmask", "i64x2.bitmask",
    };
    char spec[2048];
    for (size_t k = 0; k < COUNT(tests); k++) {
        const char *op = tests[k];
        // Results as values and fused with branches
        snprintf(spec, sizeof(spec), "wasm types {[ fn [v128] [i32, v128] ]} funcs {[ 0, 0, 0, 0, 0 ]} code {[ "
                 "{[] local.get 0 %s local.get 0 end}, {[] local.get 0 v128.not v128.not %s local.get 0 end}, "
                 "{[] block i32 i32.const 1 local.get 0 %s br_if 0 drop i32.const 2 end local.get 0 end}, "
                 "{[] local.get 0 %s if i32 i32.const 3 else i32.const 4 end local.get 0 end}, "
                 "{[] i32.const 5 i32.const 6 local.get 0 %s select local.get 0 end} ]}",
                 op, op, op, op, op);
        for (uint32_t fn = 0; fn < 5; fn++) check_calls(spec, NULL, fn, params, (uint32_t)COUNT(params), 1, 2);
    }

    static const char *shifts[] = {
        "i8x16.shl", "i8x16.shr_s", "i8x16.shr_u", "i16x8.shl", "i16x8.shr_s", "i16x8.shr_u",
        "i32x4.shl", "i32x4.shr_s", "i32x4.shr_u", "i64x2.shl", "i64x2.shr_s", "i64x2.shr_u",
    };
    static const int32_t counts[] = { 0, 1, 2, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 65, -1, -8, 0x7fffffff };
    static wah_value_t sparams[NV * COUNT(counts) * 2];
    size_t n = 0;
    for (size_t i = 0; i < NV; i++) {
        for (size_t j = 0; j < COUNT(counts); j++) {
            sparams[n++].v128 = V_SAMPLES[i];
            memset(&sparams[n], 0, sizeof(wah_value_t));
            sparams[n++].i32 = counts[j];
        }
    }
    for (size_t k = 0; k < COUNT(shifts); k++) {
        const char *op = shifts[k];
        int len = snprintf(spec, sizeof(spec), "wasm types {[ fn [v128, i32] [v128, v128] ]} funcs {[ 0, 0, 0");
        for (size_t j = 0; j < COUNT(counts); j++) len += snprintf(spec + len, sizeof(spec) - (size_t)len, ", 0");
        len += snprintf(spec + len, sizeof(spec) - (size_t)len, " ]} code {[ {[] local.get 0 local.get 1 %s local.get 0 end}, "
                        "{[] local.get 0 v128.not v128.not local.get 1 i32.const 1 i32.add %s local.get 0 end}, "
                        "{[] local.get 0 local.get 1 i32.const 0 if void end %s local.get 0 end}", op, op, op);
        for (size_t j = 0; j < COUNT(counts); j++) {
            len += snprintf(spec + len, sizeof(spec) - (size_t)len, ", {[] local.get 0 i32.const %d %s local.get 0 end}", counts[j], op);
        }
        snprintf(spec + len, sizeof(spec) - (size_t)len, " ]}");
        for (uint32_t fn = 0; fn < 3 + COUNT(counts); fn++) check_calls(spec, NULL, fn, sparams, (uint32_t)(n / 2), 2, 2);
    }
}

static void test_simd_lanes(void) {
    printf("Testing SIMD lane operations...\n");
    static const struct { const char *op, *type; int lanes; } extracts[] = {
        { "i8x16.extract_lane_s", "i32", 16 }, { "i8x16.extract_lane_u", "i32", 16 },
        { "i16x8.extract_lane_s", "i32", 8 }, { "i16x8.extract_lane_u", "i32", 8 }, { "i32x4.extract_lane", "i32", 4 },
        { "i64x2.extract_lane", "i64", 2 }, { "f32x4.extract_lane", "f32", 4 }, { "f64x2.extract_lane", "f64", 2 },
    };
    static wah_value_t params[NV];
    for (size_t i = 0; i < NV; i++) params[i].v128 = V_SAMPLES[i];
    static char spec[16384];
    for (size_t k = 0; k < COUNT(extracts); k++) {
        // The result is used as a scalar again, and returned
        const char *t = extracts[k].type;
        const char *use = t[0] == 'f' ? (t[1] == '3' ? "f32.neg" : "f64.neg") : t[1] == '3' ? "i32.const 1 i32.add" : "i64.const 1 i64.add";
        int len = snprintf(spec, sizeof(spec), "wasm types {[ fn [v128] [%s, %s, v128] ]} funcs {[ 0", t, t);
        for (int l = 1; l < 2 * extracts[k].lanes; l++) len += snprintf(spec + len, sizeof(spec) - (size_t)len, ", 0");
        len += snprintf(spec + len, sizeof(spec) - (size_t)len, " ]} code {[ ");
        for (int l = 0; l < extracts[k].lanes; l++) {
            len += snprintf(spec + len, sizeof(spec) - (size_t)len, "%s{[] local.get 0 %s %d local.get 0 %s %d %s local.get 0 end}, "
                            "{[] local.get 0 v128.not v128.not %s %d local.get 0 v128.not %s %d local.get 0 end}",
                            l ? ", " : "", extracts[k].op, l, extracts[k].op, l, use, extracts[k].op, l, extracts[k].op, l);
        }
        snprintf(spec + len, sizeof(spec) - (size_t)len, " ]}");
        for (uint32_t fn = 0; fn < 2u * (uint32_t)extracts[k].lanes; fn++) check_calls(spec, NULL, fn, params, NV, 1, 3);
    }

    // Lanes replaced with scalars from locals, temporaries and constants
    static const struct { const char *op, *type, *c1, *c2; int lanes; uint64_t vals[4]; } replaces[] = {
        { "i8x16.replace_lane", "i32", "i32.const 0", "i32.const -129", 16, { 0, 0xffffffffu, 0x12345680u, 0x7f } },
        { "i16x8.replace_lane", "i32", "i32.const 0", "i32.const 0x18001", 8, { 0, 0xffffffffu, 0x12345680u, 0x7fff } },
        { "i32x4.replace_lane", "i32", "i32.const 0", "i32.const -5", 4, { 0, 0xffffffffu, 0x12345680u, 0x7fffffff } },
        { "i64x2.replace_lane", "i64", "i64.const 0", "i64.const -5", 2, { 0, UINT64_MAX, 0x123456789abcdefull, 1 } },
        { "f32x4.replace_lane", "f32", "f32.const 0.0f32", "f32.const -2.5f32", 4, { 0, 0x7fa00001u, 0xbf800000u, 0x80000000u } },
        { "f64x2.replace_lane", "f64", "f64.const 0.0f64", "f64.const -2.5f64", 2, { 0, 0x7ff4000000000001ull, 0xbff0000000000000ull, 1 } },
    };
    static wah_value_t rparams[NV * 4 * 2];
    for (size_t k = 0; k < COUNT(replaces); k++) {
        size_t n = 0;
        for (size_t i = 0; i < NV; i++) {
            for (size_t j = 0; j < 4; j++) {
                rparams[n++].v128 = V_SAMPLES[i];
                memset(&rparams[n], 0, sizeof(wah_value_t));
                rparams[n++].i64 = (int64_t)replaces[k].vals[j];
            }
        }
        const char *t = replaces[k].type;
        const char *tmp = t[0] == 'f' ? (t[1] == '3' ? "f32.neg" : "f64.neg") : t[1] == '3' ? "i32.const 3 i32.mul" : "i64.const 3 i64.mul";
        int len = snprintf(spec, sizeof(spec), "wasm types {[ fn [v128, %s] [v128, v128, %s] ]} funcs {[ 0", t, t);
        for (int l = 1; l < 4 * replaces[k].lanes; l++) len += snprintf(spec + len, sizeof(spec) - (size_t)len, ", 0");
        len += snprintf(spec + len, sizeof(spec) - (size_t)len, " ]} code {[ ");
        for (int l = 0; l < replaces[k].lanes; l++) {
            const char *op = replaces[k].op;
            len += snprintf(spec + len, sizeof(spec) - (size_t)len, "%s"
                            "{[] local.get 0 local.get 1 %s %d local.get 0 local.get 1 end}, "
                            "{[] local.get 0 v128.not v128.not local.get 1 %s %s %d local.get 0 local.get 1 end}, "
                            "{[] local.get 0 %s %s %d local.get 0 local.get 1 end}, "
                            "{[] local.get 0 %s %s %d local.get 0 local.get 1 end}",
                            l ? ", " : "", op, l, tmp, op, l, replaces[k].c1, op, l, replaces[k].c2, op, l);
        }
        snprintf(spec + len, sizeof(spec) - (size_t)len, " ]}");
        for (uint32_t fn = 0; fn < 4u * (uint32_t)replaces[k].lanes; fn++) check_calls(spec, NULL, fn, rparams, (uint32_t)(n / 2), 2, 3);
    }

    // Splats from locals, temporaries and constants
    static const struct { const char *op, *type, *tmp, *c1, *c2; uint64_t vals[4]; } splats[] = {
        { "i8x16.splat", "i32", "i32.const 3 i32.mul", "i32.const 0", "i32.const 0x1ff", { 0, 0xffffffffu, 0x12345680u, 0x7f } },
        { "i16x8.splat", "i32", "i32.const 3 i32.mul", "i32.const -1", "i32.const 0x18001", { 0, 0xffffffffu, 0x12345680u, 0x7fff } },
        { "i32x4.splat", "i32", "i32.const 3 i32.mul", "i32.const 0", "i32.const -5", { 0, 0xffffffffu, 0x12345680u, 0x7fffffff } },
        { "i64x2.splat", "i64", "i64.const 3 i64.mul", "i64.const 0", "i64.const 0x123456789", { 0, UINT64_MAX, 0x123456789abcdefull, 1 } },
        { "f32x4.splat", "f32", "f32.neg", "f32.const 0.0f32", "f32.const -2.5f32", { 0, 0x7fa00001u, 0xbf800000u, 0x80000000u } },
        { "f64x2.splat", "f64", "f64.neg", "f64.const 0.0f64", "f64.const -2.5f64", { 0, 0x7ff4000000000001ull, 0xbff0000000000000ull, 1 } },
    };
    for (size_t k = 0; k < COUNT(splats); k++) {
        wah_value_t sp[4];
        memset(sp, 0, sizeof(sp));
        for (int j = 0; j < 4; j++) sp[j].i64 = (int64_t)splats[k].vals[j];
        const char *op = splats[k].op;
        snprintf(spec, sizeof(spec), "wasm types {[ fn [%s] [v128, %s] ]} funcs {[ 0, 0, 0, 0 ]} code {[ "
                 "{[] local.get 0 %s local.get 0 end}, {[] local.get 0 %s %s local.get 0 end}, "
                 "{[] %s %s local.get 0 end}, {[] %s %s local.get 0 end} ]}",
                 splats[k].type, splats[k].type, op, splats[k].tmp, op, splats[k].c1, op, splats[k].c2, op);
        for (uint32_t fn = 0; fn < 4; fn++) check_calls(spec, NULL, fn, sp, 4, 1, 2);
    }

    // Shuffles from both operands, one of them, or with repeated lanes
    static const uint8_t masks[][16] = {
        { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
        { 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 },
        { 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 },
        { 0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 3, 31, 7, 18, 30, 0, 12, 12, 25, 1, 15, 16, 8, 9, 27, 4 },
        { 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31 },
        { 4, 5, 6, 7, 20, 21, 22, 23, 8, 9, 10, 11, 28, 29, 30, 31 },
    };
    static wah_value_t params2[NV * NV * 2];
    for (size_t i = 0; i < NV; i++) {
        for (size_t j = 0; j < NV; j++) {
            params2[2 * (i * NV + j)].v128 = V_SAMPLES[i];
            params2[2 * (i * NV + j) + 1].v128 = V_SAMPLES[j];
        }
    }
    for (size_t k = 0; k < COUNT(masks); k++) {
        char op[128];
        int len = snprintf(op, sizeof(op), "i8x16.shuffle");
        for (int i = 0; i < 16; i++) len += snprintf(op + len, sizeof(op) - (size_t)len, " %d", masks[k][i]);
        snprintf(spec, sizeof(spec), "wasm types {[ fn [v128, v128] [v128, v128, v128] ]} funcs {[ 0, 0, 0, 0 ]} code {[ "
                 "{[] local.get 0 local.get 1 %s local.get 0 local.get 1 end}, "
                 "{[] local.get 0 v128.not v128.not local.get 1 v128.not v128.not %s local.get 0 local.get 1 end}, "
                 "{[] local.get 0 local.get 1 i32.const 0 if void end %s local.get 0 local.get 1 end}, "
                 "{[] local.get 1 local.get 1 %s local.get 0 local.get 1 end} ]}", op, op, op, op);
        for (uint32_t fn = 0; fn < 4; fn++) check_calls(spec, NULL, fn, params2, NV * NV, 2, 3);
    }
}

static void test_simd_memory(void) {
    printf("Testing SIMD memory operations...\n");
    // The vector is stored at both ends of the memory, then loaded from the address
    static const struct { const char *op; int lanes; } loads[] = {
        { "v128.load 0", 0 }, { "v128.load8x8_s 0", 0 }, { "v128.load8x8_u 0", 0 }, { "v128.load16x4_s 0", 0 },
        { "v128.load16x4_u 0", 0 }, { "v128.load32x2_s 0", 0 }, { "v128.load32x2_u 0", 0 }, { "v128.load8_splat 0", 0 },
        { "v128.load16_splat 0", 0 }, { "v128.load32_splat 0", 0 }, { "v128.load64_splat 0", 0 },
        { "v128.load32_zero 0", 0 }, { "v128.load64_zero 0", 0 },
        { "v128.load8_lane 0", 16 }, { "v128.load16_lane 0", 8 }, { "v128.load32_lane 0", 4 }, { "v128.load64_lane 0", 2 },
        { "v128.store8_lane 0", 16 }, { "v128.store16_lane 0", 8 }, { "v128.store32_lane 0", 4 }, { "v128.store64_lane 0", 2 },
        { "v128.store 0", 0 },
    };
    static const int32_t addrs[] = { 0, 1, 3, 8, 15, 16, 17, 65504, 65512, 65519, 65520, 65521, 65528, 65532, 65535, 65536, -1, -16 };
    static wah_value_t params[COUNT(addrs) * 4 * 2];
    size_t n = 0;
    for (size_t i = 0; i < COUNT(addrs); i++) {
        for (size_t j = 2; j < 6; j++) {
            memset(&params[n], 0, sizeof(wah_value_t));
            params[n++].i32 = addrs[i];
            params[n++].v128 = V_SAMPLES[j];
        }
    }
    char spec[8192];
    for (size_t k = 0; k < COUNT(loads); k++) {
        bool store = strncmp(loads[k].op, "v128.store", 10) == 0;
        int nlanes = loads[k].lanes ? loads[k].lanes : 1;
        int len = snprintf(spec, sizeof(spec), "wasm types {[ fn [i32, v128] [v128, v128, v128] ]} funcs {[ 0, 0");
        for (int l = 1; l < nlanes; l++) len += snprintf(spec + len, sizeof(spec) - (size_t)len, ", 0, 0");
        len += snprintf(spec + len, sizeof(spec) - (size_t)len, " ]} memories {[ limits.i32/1 1 ]} code {[ ");
        for (int l = 0; l < nlanes; l++) {
            char lane[16] = "";
            if (loads[k].lanes) snprintf(lane, sizeof(lane), " %d", l);
            // With the access in the middle of other operations, and with an offset
            const char *prep = "i32.const 0 local.get 1 v128.store 0 0 i32.const 65520 local.get 1 i8x16.abs v128.store 0 0";
            const char *post = "i32.const 0 v128.load 0 0 i32.const 65520 v128.load 0 0";
            if (store) {
                len += snprintf(spec + len, sizeof(spec) - (size_t)len, "%s{[] %s local.get 0 local.get 1 v128.not %s 0%s local.get 1 %s end}, "
                                "{[] %s local.get 0 local.get 1 %s 7%s local.get 1 %s end}",
                                l ? ", " : "", prep, loads[k].op, lane, post, prep, loads[k].op, lane, post);
            } else {
                const char *vec = loads[k].lanes ? "local.get 1 v128.not" : "";
                len += snprintf(spec + len, sizeof(spec) - (size_t)len, "%s{[] %s local.get 0 %s %s 0%s local.get 1 local.get 1 end}, "
                                "{[] %s local.get 0 %s %s 7%s %s end}",
                                l ? ", " : "", prep, vec, loads[k].op, lane, prep, vec, loads[k].op, lane, post);
            }
        }
        snprintf(spec + len, sizeof(spec) - (size_t)len, " ]}");
        for (uint32_t fn = 0; fn < 2u * (uint32_t)nlanes; fn++) check_calls(spec, NULL, fn, params, (uint32_t)(n / 2), 2, 3);
    }

    // A second memory, where the alignment field has bit 6 set for the memory index
    const char *mem2 = "wasm types {[ fn [i32, v128] [v128, v128, v128] ]} funcs {[ 0 ]} \
        memories {[ limits.i32/1 1, limits.i32/1 1 ]} code {[ {[] \
        local.get 0 local.get 1 v128.store 68 1 0 local.get 0 local.get 1 v128.store32_lane 66 1 16 3 \
        local.get 0 v128.load 68 1 0 local.get 0 v128.load16_splat 65 1 16 \
        local.get 0 local.get 1 v128.load64_lane 67 1 4 1 end} ]}";
    check_calls(mem2, NULL, 0, params, (uint32_t)(n / 2), 2, 3);
}

static void host_v128(wah_call_context_t *call, void *userdata) {
    (void)userdata;
    wah_v128_t v = wah_param_v128(call, 0);
    for (int i = 0; i < 4; i++) v.u32[i] = v.u32[i] * 3 + (uint32_t)wah_param_i32(call, 1);
    wah_result_v128(call, 0, &v);
    wah_result_i32(call, 1, wah_param_i32(call, 1) ^ 0x55);
}

static void test_simd_values(void) {
    printf("Testing v128 values in locals, globals, calls and branches...\n");
    static wah_value_t params[NV * 3 * 3];
    static const int32_t conds[] = { 0, 1, 7 };
    size_t n = 0;
    for (size_t i = 0; i < NV; i++) {
        for (size_t j = 0; j < 3; j++) {
            params[n++].v128 = V_SAMPLES[i];
            params[n++].v128 = V_SAMPLES[(i + 5) % NV];
            memset(&params[n], 0, sizeof(wah_value_t));
            params[n++].i32 = conds[j];
        }
    }
    // Selects, untyped and typed, with conditions as flags, registers and slots, and v128 results of blocks and ifs
    const char *sel = "wasm types {[ fn [v128, v128, i32] [v128, v128] ]} funcs {[ 0, 0, 0, 0, 0 ]} code {[ \
        {[] local.get 0 local.get 1 local.get 2 select local.get 1 local.get 0 local.get 2 select.typed [v128] end}, \
        {[] local.get 0 v128.not local.get 1 v128.not local.get 2 i32.const 3 i32.lt_s select local.get 0 end}, \
        {[] local.get 0 local.get 1 local.get 2 i32.const 0 if void end select local.get 0 local.get 1 i32.const 0 if void end \
            local.get 2 select.typed [v128] end}, \
        {[] block v128 local.get 0 local.get 2 br_if 0 drop local.get 1 end \
            local.get 2 if v128 local.get 0 i8x16.abs else local.get 1 i8x16.neg end end}, \
        {[] block v128 block v128 local.get 1 local.get 0 local.get 2 br_table [0, 1] 0 end i64x2.neg end local.get 0 end} ]}";
    for (uint32_t fn = 0; fn < 5; fn++) check_calls(sel, NULL, fn, params, (uint32_t)(n / 3), 3, 2);

    // Globals, including an imported mutable one, a host function and an indirect call with v128 parameters
    wah_module_t env;
    assert_ok(wah_new_module(&env, NULL));
    wah_v128_t g0 = { .u32 = { 1, 2, 3, 4 } };
    assert_ok(wah_export_global_v128(&env, "g", true, &g0));
    assert_ok(wah_export_func(&env, "h", "(v128, i32) -> (v128, i32)", host_v128, NULL, NULL));
    char c[33], c2[33];
    char spec[4096];
    snprintf(spec, sizeof(spec), "wasm types {[ fn [v128, v128, i32] [v128, v128], fn [v128, i32] [v128, i32] ]} \
        imports {[ {'env'} {'g'} export.global v128 mut, {'env'} {'h'} fn# 1 ]} funcs {[ 0, 1, 0 ]} \
        tables {[ funcref limits.i32/1 2 ]} globals {[ v128 mut v128.const %%'%s' end, v128 immut v128.const %%'%s' end ]} \
        elements {[ elem.active.table#0 i32.const 0 end [0, 2] ]} code {[ \
        {[] global.get 0 local.get 0 i32x4.add global.set 0 global.get 1 global.get 2 i32x4.sub global.set 1 \
            local.get 1 local.get 2 call 0 drop global.get 0 global.get 1 i8x16.add \
            local.get 2 local.get 2 i32.const 1 i32.and call_indirect 1 0 i32.const 9 i32.add i8x16.splat i8x16.add end}, \
        {[] local.get 0 local.get 1 i64.extend_i32_s i64x2.splat i64x2.mul local.get 1 end}, \
        {[] local.get 0 local.get 1 i8x16.add local.get 2 i32x4.splat end} ]}",
        v128_hex(&V_SAMPLES[2], c), v128_hex(&V_SAMPLES[3], c2));
    check_calls(spec, &env, 1, params, (uint32_t)(n / 3), 3, 2);
    check_calls(spec, &env, 3, params, (uint32_t)(n / 3), 3, 2); // Function 2 is only called indirectly
    wah_free_module(&env);

    // More live vectors than registers, and v128 locals pinned in a loop along with f64 locals and spilled by calls
    int len = snprintf(spec, sizeof(spec), "wasm types {[ fn [v128, v128, i32] [v128, v128], fn [] [] ]} \
        imports {[ {'env'} {'interrupt'} fn# 1 ]} funcs {[ 0 ]} code {[ {[12 v128, 2 f64] \
        block void loop void");
    for (int k = 3; k <= 14; k++) {
        len += snprintf(spec + len, sizeof(spec) - (size_t)len, " local.get %d local.get %d i32x4.add local.get 2 i8x16.splat v128.xor local.set %d",
                        k, k % 2, k);
    }
    len += snprintf(spec + len, sizeof(spec) - (size_t)len, " local.get 15 local.get 3 f64x2.extract_lane 0 f64.add local.set 15 \
        local.get 2 i32.const 7 i32.and i32.eqz if void call 0 end \
        local.get 2 i32.const 1 i32.sub local.tee 2 i32.const 0 i32.gt_s br_if 0 end end");
    for (int k = 3; k <= 14; k++) len += snprintf(spec + len, sizeof(spec) - (size_t)len, " local.get %d", k);
    for (int k = 0; k < 20; k++) len += snprintf(spec + len, sizeof(spec) - (size_t)len, " local.get %d local.get 1 i16x8.mul", k % 2);
    for (int k = 0; k < 20 + 11; k++) len += snprintf(spec + len, sizeof(spec) - (size_t)len, " i8x16.add");
    snprintf(spec + len, sizeof(spec) - (size_t)len, " local.get 15 f64x2.splat end} ]}");
    wah_module_t ienv;
    assert_ok(wah_new_module(&ienv, NULL));
    assert_ok(wah_export_func(&ienv, "interrupt", "() -> ()", host_interrupt, NULL, NULL));
    for (size_t i = 0; i < NV; i++) {
        wah_value_t p[3] = { { .v128 = V_SAMPLES[i] }, { .v128 = V_SAMPLES[(i + 3) % NV] }, { .i32 = 30 } };
        wah_value_t r[2][2];
        int yields[2];
        for (int native = 0; native < 2; native++) {
            jit_inst_t a;
            inst_new(&a, spec, !native, &ienv);
            assert_ok(wah_start(&a.ctx, 1, p, 3));
            wah_error_t err;
            yields[native] = 0;
            while ((err = wah_resume(&a.ctx)) == WAH_STATUS_YIELDED) yields[native]++;
            assert_ok(err);
            uint32_t nr;
            memset(r[native], 0, sizeof(r[native]));
            assert_ok(wah_finish(&a.ctx, r[native], 2, &nr));
            assert_true(ran_natively(&a) == (native && EXPECT_NATIVE));
            inst_free(&a);
        }
        assert_true(memcmp(r[0], r[1], sizeof(r[0])) == 0);
        assert_eq_i32(yields[1], yields[0]);
        assert_eq_i32(yields[0], 3);
    }
    wah_free_module(&ienv);
}

// Functions over 1MB, where AArch64 places constants inline as literal loads can't reach the end of the function
static void test_simd_long_function(void) {
    printf("Testing a long function with SIMD constants...\n");
    size_t cap = 2 << 20;
    char *spec = (char *)malloc(cap);
    assert_true(spec != NULL);
    char c[33];
    size_t len = (size_t)snprintf(spec, cap, "wasm types {[ fn [v128, v128] [v128] ]} funcs {[ 0 ]} code {[ {[] \
        local.get 0 v128.const %%'%s' i8x16.add local.set 0", v128_hex(&V_SAMPLES[3], c));
    for (int k = 0; k < 30000; k++) len += (size_t)snprintf(spec + len, cap - len, " local.get 1 local.get 0 i64x2.mul local.set 1");
    snprintf(spec + len, cap - len, " local.get 1 v128.const %%'%s' i8x16.swizzle end} ]}", v128_hex(&V_SAMPLES[14], c));
    wah_value_t p[2] = { { .v128 = V_SAMPLES[2] }, { .v128 = V_SAMPLES[13] } };
    check_call(spec, NULL, 0, p, 2, 1);
    free(spec);
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
    const char *guard = getenv("WAH_TEST_GUARD_PAGES");
    if (guard && *guard) { // Natively compiled code relies on guard pages then
        printf("Guard pages: %s\n", wah_strerror(wah_enable_guard_pages(WAH_HANDLES_FAULT)));
    }
    test_integer_ops();
    test_float_ops();
    test_control();
    test_memory();
    test_memory64();
    test_calls();
    test_suspend();
    test_memory64_bulk_yield();
    test_pinned_locals();
    test_local_cache();
    test_result_hints();
    test_i32_upper_bits();
    test_simd_binary();
    test_simd_unary();
    test_simd_tests_and_shifts();
    test_simd_lanes();
    test_simd_memory();
    test_simd_values();
    test_simd_long_function();
    test_fallback();
    printf("All JIT tests passed.\n");
    return 0;
}
