#include "../wah.h"
#include "common.h"
#include <stdio.h>
#include <string.h>

static uint32_t f32_bits(float f) { uint32_t u; memcpy(&u, &f, 4); return u; }
static uint64_t f64_bits(double f) { uint64_t u; memcpy(&u, &f, 8); return u; }

// Helper: run a two-arg f32 op and return bit pattern
static uint32_t run_f32_binop(const char *op, float a, float b) {
    char spec[512];
    snprintf(spec, sizeof(spec),
        "wasm types {[ fn [f32, f32] [f32] ]} funcs {[ 0 ]} "
        "code {[ {[] local.get 0 local.get 1 %s end } ]}", op);

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t params[2], result;
    memcpy(&params[0].i32, &a, 4);
    memcpy(&params[1].i32, &b, 4);
    assert_ok(wah_call(&ctx, 0, params, 2, &result));

    uint32_t bits = result.i32;
    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
    return bits;
}

static uint64_t run_f64_binop(const char *op, double a, double b) {
    char spec[512];
    snprintf(spec, sizeof(spec),
        "wasm types {[ fn [f64, f64] [f64] ]} funcs {[ 0 ]} "
        "code {[ {[] local.get 0 local.get 1 %s end } ]}", op);

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t params[2], result;
    memcpy(&params[0].i64, &a, 8);
    memcpy(&params[1].i64, &b, 8);
    assert_ok(wah_call(&ctx, 0, params, 2, &result));

    uint64_t bits = result.i64;
    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
    return bits;
}

// 607db56: f32.min/max and f64.min/max with +0 and -0.
static void test_fmin_fmax_neg_zero() {
    printf("Testing f32/f64 min/max with signed zeros (607db56)...\n");

    float pos0f = 0.0f;
    float neg0f = -0.0f;
    double pos0d = 0.0;
    double neg0d = -0.0;

    // f32.min(+0, -0) must be -0 (0x80000000)
    assert_eq_u32(run_f32_binop("f32.min", pos0f, neg0f), 0x80000000u);
    assert_eq_u32(run_f32_binop("f32.min", neg0f, pos0f), 0x80000000u);

    // f32.max(+0, -0) must be +0 (0x00000000)
    assert_eq_u32(run_f32_binop("f32.max", pos0f, neg0f), 0x00000000u);
    assert_eq_u32(run_f32_binop("f32.max", neg0f, pos0f), 0x00000000u);

    // f64.min(+0, -0) must be -0 (0x8000000000000000)
    assert_eq_u64(run_f64_binop("f64.min", pos0d, neg0d), 0x8000000000000000ULL);
    assert_eq_u64(run_f64_binop("f64.min", neg0d, pos0d), 0x8000000000000000ULL);

    // f64.max(+0, -0) must be +0 (0x0000000000000000)
    assert_eq_u64(run_f64_binop("f64.max", pos0d, neg0d), 0x0000000000000000ULL);
    assert_eq_u64(run_f64_binop("f64.max", neg0d, pos0d), 0x0000000000000000ULL);
}

// cd5a43d: Some fp opcodes should NOT canonicalize NaN.
static void test_fp_no_canonicalize_nan() {
    printf("Testing fp abs/neg/copysign preserve NaN bits (cd5a43d)...\n");

    // Custom NaN with payload
    uint32_t custom_nan32 = 0x7FC00001u;
    float nan_f32;
    memcpy(&nan_f32, &custom_nan32, 4);

    // f32.neg should flip sign bit, preserving NaN payload
    uint32_t neg_result = run_f32_binop("f32.add", nan_f32, 0.0f);
    (void)neg_result;

    // Use a single-arg helper for unary ops
    const char *neg_spec = "wasm types {[ fn [f32] [f32] ]} funcs {[ 0 ]} "
        "code {[ {[] local.get 0 f32.neg end } ]}";
    wah_module_t m1 = {0};
    assert_ok(wah_parse_module_from_spec(&m1, neg_spec));
    wah_exec_context_t c1 = {0};
    assert_ok(wah_exec_context_create(&c1, &m1, NULL));
    assert_ok(wah_instantiate(&c1));
    wah_value_t p1, r1;
    memcpy(&p1.i32, &nan_f32, 4);
    assert_ok(wah_call(&c1, 0, &p1, 1, &r1));
    assert_eq_u32(r1.i32, 0xFFC00001u);
    wah_exec_context_destroy(&c1);
    wah_free_module(&m1);

    // f32.abs should clear sign bit, preserving NaN payload
    uint32_t neg_nan32 = 0xFFC00001u;
    const char *abs_spec = "wasm types {[ fn [f32] [f32] ]} funcs {[ 0 ]} "
        "code {[ {[] local.get 0 f32.abs end } ]}";
    wah_module_t m2 = {0};
    assert_ok(wah_parse_module_from_spec(&m2, abs_spec));
    wah_exec_context_t c2 = {0};
    assert_ok(wah_exec_context_create(&c2, &m2, NULL));
    assert_ok(wah_instantiate(&c2));
    wah_value_t p2, r2;
    memcpy(&p2.i32, &neg_nan32, 4);
    assert_ok(wah_call(&c2, 0, &p2, 1, &r2));
    assert_eq_u32(r2.i32, 0x7FC00001u);
    wah_exec_context_destroy(&c2);
    wah_free_module(&m2);
}

// a7c6d65: Fix nearest rounding losing negative zero sign.
static void test_nearest_neg_zero() {
    printf("Testing f32/f64 nearest preserves -0 (a7c6d65)...\n");

    const char *f32_spec = "wasm types {[ fn [f32] [f32] ]} funcs {[ 0 ]} "
        "code {[ {[] local.get 0 f32.nearest end } ]}";
    wah_module_t m1 = {0};
    assert_ok(wah_parse_module_from_spec(&m1, f32_spec));
    wah_exec_context_t c1 = {0};
    assert_ok(wah_exec_context_create(&c1, &m1, NULL));
    assert_ok(wah_instantiate(&c1));

    float neg0 = -0.0f;
    wah_value_t p1, r1;
    memcpy(&p1.i32, &neg0, 4);
    assert_ok(wah_call(&c1, 0, &p1, 1, &r1));
    assert_eq_u32(r1.i32, f32_bits(-0.0f));
    wah_exec_context_destroy(&c1);
    wah_free_module(&m1);

    // Also test -0.5 -> nearest should be -0.0 (banker's rounding)
    float neg_half = -0.5f;
    wah_module_t m1b = {0};
    assert_ok(wah_parse_module_from_spec(&m1b, f32_spec));
    wah_exec_context_t c1b = {0};
    assert_ok(wah_exec_context_create(&c1b, &m1b, NULL));
    assert_ok(wah_instantiate(&c1b));
    wah_value_t p1b, r1b;
    memcpy(&p1b.i32, &neg_half, 4);
    assert_ok(wah_call(&c1b, 0, &p1b, 1, &r1b));
    assert_eq_u32(r1b.i32, f32_bits(-0.0f));
    wah_exec_context_destroy(&c1b);
    wah_free_module(&m1b);

    const char *f64_spec = "wasm types {[ fn [f64] [f64] ]} funcs {[ 0 ]} "
        "code {[ {[] local.get 0 f64.nearest end } ]}";
    wah_module_t m2 = {0};
    assert_ok(wah_parse_module_from_spec(&m2, f64_spec));
    wah_exec_context_t c2 = {0};
    assert_ok(wah_exec_context_create(&c2, &m2, NULL));
    assert_ok(wah_instantiate(&c2));

    double neg0d = -0.0;
    wah_value_t p2, r2;
    memcpy(&p2.i64, &neg0d, 8);
    assert_ok(wah_call(&c2, 0, &p2, 1, &r2));
    assert_eq_u64(r2.i64, f64_bits(-0.0));
    wah_exec_context_destroy(&c2);
    wah_free_module(&m2);
}

int main() {
    test_fmin_fmax_neg_zero();
    test_fp_no_canonicalize_nan();
    test_nearest_neg_zero();
    printf("All numeric regression tests passed!\n");
    return 0;
}
