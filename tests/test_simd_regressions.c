#include "../wah.h"
#include "common.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

static void run_simd_test(const char *name, const char *spec_str, const wah_v128_t *expected, ...) {
    va_list args;
    va_start(args, expected);
    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_specv(&module, spec_str, args));
    va_end(args);

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &module, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    if (memcmp(&result.v128, expected, 16) != 0) {
        fprintf(stderr, "%s: FAILED\n  Expected: ", name);
        for (int i = 0; i < 16; i++) fprintf(stderr, "%02x ", expected->u8[i]);
        fprintf(stderr, "\n  Actual:   ");
        for (int i = 0; i < 16; i++) fprintf(stderr, "%02x ", result.v128.u8[i]);
        fprintf(stderr, "\n");
        exit(1);
    }

    wah_free_exec_context(&ctx);
    wah_free_module(&module);
}

static int32_t run_simd_i32_test(const char *name, const char *spec_str, ...) {
    printf("  %s\n", name);

    va_list args;
    va_start(args, spec_str);
    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_specv(&module, spec_str, args));
    va_end(args);

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &module, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));

    wah_free_exec_context(&ctx);
    wah_free_module(&module);
    return result.i32;
}

static const char *v128_to_i32_spec = "wasm \
    types {[ fn [] [i32] ]} \
    funcs {[ 0 ]} \
    code {[ {[] v128.const %v128 %t end } ]}";

static const char *v128v128_to_v128_spec = "wasm \
    types {[ fn [] [v128] ]} \
    funcs {[ 0 ]} \
    code {[ {[] v128.const %v128 v128.const %v128 %t end } ]}";

static const char *v128_i32_to_v128_spec = "wasm \
    types {[ fn [] [v128] ]} \
    funcs {[ 0 ]} \
    code {[ {[] v128.const %v128 i32.const %d32 %t end } ]}";

// a732299: SIMD ge/lt should use dedicated instructions due to NaNs.
static void test_simd_float_compare_nan() {
    printf("Testing SIMD float compare with NaN (a732299)...\n");

    // f32x4.lt: NaN < anything = false (all zeros)
    float nan_f = NAN;
    uint32_t nan_bits;
    memcpy(&nan_bits, &nan_f, 4);
    wah_v128_t v_nan = {0}; memset(&v_nan, 0, 16);
    memcpy(&v_nan.u8[0], &nan_bits, 4);
    memcpy(&v_nan.u8[4], &nan_bits, 4);
    memcpy(&v_nan.u8[8], &nan_bits, 4);
    memcpy(&v_nan.u8[12], &nan_bits, 4);

    float one = 1.0f;
    wah_v128_t v_one = {0};
    memcpy(&v_one.u8[0], &one, 4);
    memcpy(&v_one.u8[4], &one, 4);
    memcpy(&v_one.u8[8], &one, 4);
    memcpy(&v_one.u8[12], &one, 4);

    wah_v128_t expected_false = {0};
    run_simd_test("f32x4.lt (NaN, 1.0)", v128v128_to_v128_spec, &expected_false,
        &v_nan, &v_one, "f32x4.lt");

    // f32x4.ge: NaN >= anything = false
    run_simd_test("f32x4.ge (NaN, 1.0)", v128v128_to_v128_spec, &expected_false,
        &v_nan, &v_one, "f32x4.ge");

    // f64x2.lt with NaN
    double nan_d = NAN;
    uint64_t nan_bits64;
    memcpy(&nan_bits64, &nan_d, 8);
    wah_v128_t v_nan64 = {0};
    memcpy(&v_nan64.u8[0], &nan_bits64, 8);
    memcpy(&v_nan64.u8[8], &nan_bits64, 8);

    double one_d = 1.0;
    wah_v128_t v_one64 = {0};
    memcpy(&v_one64.u8[0], &one_d, 8);
    memcpy(&v_one64.u8[8], &one_d, 8);

    run_simd_test("f64x2.lt (NaN, 1.0)", v128v128_to_v128_spec, &expected_false,
        &v_nan64, &v_one64, "f64x2.lt");

    run_simd_test("f64x2.ge (NaN, 1.0)", v128v128_to_v128_spec, &expected_false,
        &v_nan64, &v_one64, "f64x2.ge");
}

// 70b9aa2: Fix faulty all_true opcodes.
static void test_simd_all_true() {
    printf("Testing SIMD all_true regressions (70b9aa2)...\n");

    // i8x16.all_true: one zero lane among non-zero -> false
    wah_v128_t v_almost_all = {0};
    memset(&v_almost_all, 0xFF, 16);
    v_almost_all.u8[7] = 0;
    assert_eq_i32(run_simd_i32_test("i8x16.all_true (one zero)", v128_to_i32_spec,
        &v_almost_all, "i8x16.all_true"), 0);

    // All non-zero -> true
    wah_v128_t v_all = {0};
    memset(&v_all, 0x01, 16);
    assert_eq_i32(run_simd_i32_test("i8x16.all_true (all set)", v128_to_i32_spec,
        &v_all, "i8x16.all_true"), 1);

    // i16x8.all_true: one zero lane
    wah_v128_t v_i16 = {0};
    for (int i = 0; i < 8; i++) { uint16_t val = 1; memcpy(&v_i16.u8[i*2], &val, 2); }
    uint16_t zero16 = 0; memcpy(&v_i16.u8[4], &zero16, 2);
    assert_eq_i32(run_simd_i32_test("i16x8.all_true (one zero)", v128_to_i32_spec,
        &v_i16, "i16x8.all_true"), 0);

    // i32x4.all_true: one zero lane
    wah_v128_t v_i32 = {0};
    uint32_t one32 = 1, zero32 = 0;
    memcpy(&v_i32.u8[0], &one32, 4);
    memcpy(&v_i32.u8[4], &one32, 4);
    memcpy(&v_i32.u8[8], &zero32, 4);
    memcpy(&v_i32.u8[12], &one32, 4);
    assert_eq_i32(run_simd_i32_test("i32x4.all_true (one zero)", v128_to_i32_spec,
        &v_i32, "i32x4.all_true"), 0);

    // i64x2.all_true: one zero lane (also f3623e9 regression)
    wah_v128_t v_i64 = {0};
    uint64_t one64 = 1, zero64 = 0;
    memcpy(&v_i64.u8[0], &one64, 8);
    memcpy(&v_i64.u8[8], &zero64, 8);
    assert_eq_i32(run_simd_i32_test("i64x2.all_true (one zero)", v128_to_i32_spec,
        &v_i64, "i64x2.all_true"), 0);

    // Both non-zero
    wah_v128_t v_i64_both = {0};
    memcpy(&v_i64_both.u8[0], &one64, 8);
    memcpy(&v_i64_both.u8[8], &one64, 8);
    assert_eq_i32(run_simd_i32_test("i64x2.all_true (both set)", v128_to_i32_spec,
        &v_i64_both, "i64x2.all_true"), 1);
}

// 936ba3f: Fix the i64x2.mul implementation on SSE2.
static void test_simd_i64x2_mul() {
    printf("Testing i64x2.mul (936ba3f)...\n");

    uint64_t a0 = 0x100000002ULL, a1 = 0xFFFFFFFF00000001ULL;
    uint64_t b0 = 0x300000004ULL, b1 = 0x200000003ULL;
    uint64_t r0 = a0 * b0, r1 = a1 * b1;

    wah_v128_t va = {0}, vb = {0}, expected = {0};
    memcpy(&va.u8[0], &a0, 8); memcpy(&va.u8[8], &a1, 8);
    memcpy(&vb.u8[0], &b0, 8); memcpy(&vb.u8[8], &b1, 8);
    memcpy(&expected.u8[0], &r0, 8); memcpy(&expected.u8[8], &r1, 8);

    run_simd_test("i64x2.mul large values", v128v128_to_v128_spec, &expected,
        &va, &vb, "i64x2.mul");
}

// 1adb8fe: Make i8x16.shr_s correctly sign-extend.
static void test_simd_i8x16_shr_s() {
    printf("Testing i8x16.shr_s sign extension (1adb8fe)...\n");

    // 0x80 >> 1 (arithmetic) = 0xC0, not 0x40
    wah_v128_t input = {0};
    memset(input.u8, 0x80, 16);

    wah_v128_t expected = {0};
    memset(expected.u8, 0xC0, 16);

    run_simd_test("i8x16.shr_s (0x80 >> 1)", v128_i32_to_v128_spec, &expected,
        &input, (int32_t)1, "i8x16.shr_s");

    // 0xFE >> 1 (arithmetic) = 0xFF
    wah_v128_t input2 = {0};
    memset(input2.u8, 0xFE, 16);

    wah_v128_t expected2 = {0};
    memset(expected2.u8, 0xFF, 16);

    run_simd_test("i8x16.shr_s (0xFE >> 1)", v128_i32_to_v128_spec, &expected2,
        &input2, (int32_t)1, "i8x16.shr_s");
}

// a16439a: Correct wah_i16x8_narrow_i32x4_u_sse2.
static void test_simd_i16x8_narrow_i32x4_u() {
    printf("Testing i16x8.narrow_i32x4_u saturation (a16439a)...\n");

    // Inputs: first v128 = [neg, 0, 65535, 65536], second v128 = [100000, 0, 1, -1]
    // narrow_u clamps to [0, 65535]: [0, 0, 65535, 65535, 65535, 0, 1, 0]
    int32_t a[4] = {-1, 0, 65535, 65536};
    int32_t b[4] = {100000, 0, 1, -1};
    wah_v128_t va = {0}, vb = {0};
    memcpy(&va, a, 16);
    memcpy(&vb, b, 16);

    uint16_t exp[8] = {0, 0, 65535, 65535, 65535, 0, 1, 0};
    wah_v128_t expected = {0};
    memcpy(&expected, exp, 16);

    run_simd_test("i16x8.narrow_i32x4_u", v128v128_to_v128_spec, &expected,
        &va, &vb, "i16x8.narrow_i32x4_u");
}

// b031cf7: Fix double rounding in wah_f32x4_convert_i32x4_u_sse2.
static void test_simd_f32x4_convert_i32x4_u() {
    printf("Testing f32x4.convert_i32x4_u double rounding (b031cf7)...\n");

    // Values near 2^24 where double rounding shows up
    // 16777217 (2^24 + 1) should round to 16777216.0f (round to even)
    uint32_t vals[4] = {16777217u, 16777219u, 0u, 4294967295u};
    float expected_f[4] = {16777216.0f, 16777220.0f, 0.0f, 4294967296.0f};

    wah_v128_t input = {0}, expected = {0};
    memcpy(&input, vals, 16);
    memcpy(&expected, expected_f, 16);

    run_simd_test("f32x4.convert_i32x4_u near 2^24", "wasm \
        types {[ fn [] [v128] ]} \
        funcs {[ 0 ]} \
        code {[ {[] v128.const %v128 f32x4.convert_i32x4_u end } ]}",
        &expected, &input);
}

int main() {
    test_simd_float_compare_nan();
    test_simd_all_true();
    test_simd_i64x2_mul();
    test_simd_i8x16_shr_s();
    test_simd_i16x8_narrow_i32x4_u();
    test_simd_f32x4_convert_i32x4_u();
    printf("All SIMD regression tests passed!\n");
    return 0;
}
