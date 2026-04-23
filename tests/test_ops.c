#define WAH_IMPLEMENTATION
#include "../wah.h"
#include "common.c"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>

#define FLOAT_TOLERANCE 1e-6

// --- Test Helper Functions ---

// Generic test runner
#define DEFINE_RUN_TEST(T, ty, fmt, compare) \
void run_test_##T(const char* test_name, const char *spec, \
                  wah_value_t* params, uint32_t param_count, ty expected_result, bool expect_trap) { \
    wah_module_t module; \
    wah_exec_context_t ctx; \
    wah_value_t result; \
    \
    assert_ok(wah_parse_module_from_spec(&module, spec)); \
    assert_ok(wah_exec_context_create(&ctx, &module)); \
    \
    if (expect_trap) { \
        assert_err(wah_call(&ctx, 0, params, param_count, &result), WAH_ERROR_TRAP); \
    } else { \
        assert_ok(wah_call(&ctx, 0, params, param_count, &result)); \
        if (!(compare)) { \
            printf("  %s: FAILED! Expected " fmt ", got " fmt "\n", test_name, expected_result, result.T); \
            exit(1); \
        } \
    } \
    \
    wah_exec_context_destroy(&ctx); \
    wah_free_module(&module); \
}

#define FLOAT_COMPARE(result_val, expected_val, type) \
    ((isnan(result_val) && isnan(expected_val)) || \
     (isinf(result_val) && isinf(expected_val) && (signbit(result_val) == signbit(expected_val))) || \
     (fabs(result_val - expected_val) <= FLOAT_TOLERANCE))

DEFINE_RUN_TEST(i32, int32_t, "%" PRId32, result.i32 == expected_result)
DEFINE_RUN_TEST(i64, int64_t, "%" PRId64, result.i64 == expected_result)
DEFINE_RUN_TEST(f32, float, "%f", FLOAT_COMPARE(result.f32, expected_result, float))
DEFINE_RUN_TEST(f64, double, "%f", FLOAT_COMPARE(result.f64, expected_result, double))

// --- Templates ---

// (module (func (param T) (result U) (OPCODE (local.get 0))))
#define UNARY_WASM_SPEC(arg_ty, ret_ty, opcode) "wasm \
    types {[ fn [" #arg_ty "] [" #ret_ty "] ]} \
    funcs {[ 0 ]} \
    code {[ {[] local.get 0 " opcode " end} ]} \
"

// (module (func (param T1 T2) (result U) (OPCODE (local.get 0) (local.get 1))))
#define BINARY_WASM_SPEC(lhs_ty, rhs_ty, ret_ty, opcode) "wasm \
    types {[ fn [" #lhs_ty ", " #rhs_ty "] [" #ret_ty "] ]} \
    funcs {[ 0 ]} \
    code {[ {[] local.get 0 local.get 1 " opcode " end} ]} \
"

#define CHECK_UNARY(label, arg_ty, arg, ret_ty, ret) do { \
    wah_value_t params[1] = { { .arg_ty = arg } }; \
    run_test_##ret_ty(label, wasm_spec, params, 1, ret, false); \
} while (0)

#define CHECK_UNARY_TRAP(label, arg_ty, arg, ret_ty) do { \
    wah_value_t params[1] = { { .arg_ty = arg } }; \
    run_test_##ret_ty(label, wasm_spec, params, 1, 0, true); \
} while (0)

#define CHECK_BINARY(label, lhs_ty, lhs, rhs_ty, rhs, ret_ty, ret) do { \
    wah_value_t params[2] = { { .lhs_ty = lhs }, { .rhs_ty = rhs } }; \
    run_test_##ret_ty(label, wasm_spec, params, 2, ret, false); \
} while (0)

#define CHECK_BINARY_TRAP(label, lhs_ty, lhs, rhs_ty, rhs, ret_ty) do { \
    wah_value_t params[2] = { { .lhs_ty = lhs }, { .rhs_ty = rhs } }; \
    run_test_##ret_ty(label, wasm_spec, params, 2, 0, true); \
} while (0)

// --- Individual Test Functions ---

void test_i32_and() {
    static const char *wasm_spec = BINARY_WASM_SPEC(i32, i32, i32, "i32.and");

    printf("Testing I32.AND...\n");
    CHECK_BINARY("I32.AND (0xFF & 0x0F)", i32, 0xFF, i32, 0x0F, i32, 0x0F);
}

void test_i32_eq() {
    static const char *wasm_spec = BINARY_WASM_SPEC(i32, i32, i32, "i32.eq");

    printf("Testing I32.EQ...\n");
    CHECK_BINARY("I32.EQ (42 == 42)", i32, 42, i32, 42, i32, 1);
    CHECK_BINARY("I32.EQ (42 == 24)", i32, 42, i32, 24, i32, 0);
}

void test_i32_eqz() {
    static const char *wasm_spec = UNARY_WASM_SPEC(i32, i32, "i32.eqz");

    printf("Testing I32.EQZ...\n");
    CHECK_UNARY("I32.EQZ (0)", i32, 0, i32, 1);
    CHECK_UNARY("I32.EQZ (1)", i32, 1, i32, 0);
    CHECK_UNARY("I32.EQZ (42)", i32, 42, i32, 0);
    CHECK_UNARY("I32.EQZ (-1)", i32, -1, i32, 0);
}

void test_i64_eqz() {
    static const char *wasm_spec = UNARY_WASM_SPEC(i64, i32, "i64.eqz");

    printf("Testing I64.EQZ...\n");
    CHECK_UNARY("I64.EQZ (0)", i64, 0ULL, i32, 1);
    CHECK_UNARY("I64.EQZ (1)", i64, 1ULL, i32, 0);
    CHECK_UNARY("I64.EQZ (42)", i64, 42ULL, i32, 0);
    CHECK_UNARY("I64.EQZ (-1)", i64, -1LL, i32, 0);
}

void test_i32_popcnt() {
    static const char *wasm_spec = UNARY_WASM_SPEC(i32, i32, "i32.popcnt");

    printf("Testing I32.POPCNT...\n");
    CHECK_UNARY("I32.POPCNT (0xAA)", i32, 0b10101010 /*0xAA*/, i32, 4);
}

void test_i64_clz() {
    static const char *wasm_spec = UNARY_WASM_SPEC(i64, i64, "i64.clz");

    printf("Testing I64.CLZ...\n");
    CHECK_UNARY("I64.CLZ (0x00...0FF)", i64, 0x00000000000000FFULL /*56 leading zeros*/, i64, 56);
}

void test_f64_nearest() {
    static const char *wasm_spec = UNARY_WASM_SPEC(f64, f64, "f64.nearest");

    printf("Testing F64.NEAREST...\n");
    CHECK_UNARY("F64.NEAREST (2.5)", f64, 2.5, f64, 2.0);
    CHECK_UNARY("F64.NEAREST (3.5)", f64, 3.5, f64, 4.0);
    CHECK_UNARY("F64.NEAREST (-2.5)", f64, -2.5, f64, -2.0);
    CHECK_UNARY("F64.NEAREST (-3.5)", f64, -3.5, f64, -4.0);
}

void test_i32_rotl() {
    static const char *wasm_spec = BINARY_WASM_SPEC(i32, i32, i32, "i32.rotl");

    printf("Testing I32.ROTL...\n");
    CHECK_BINARY("I32.ROTL (0x80000001, 1)", i32, 0x80000001 /*1000...0001*/, i32, 1, i32, 0x00000003);
}

void test_f32_min() {
    static const char *wasm_spec = BINARY_WASM_SPEC(f32, f32, f32, "f32.min");

    printf("Testing F32.MIN...\n");
    CHECK_BINARY("F32.MIN (10.0f, 20.0f)", f32, 10.0f, f32, 20.0f, f32, 10.0f);
    CHECK_BINARY("F32.MIN (5.0f, -5.0f)", f32, 5.0f, f32, -5.0f, f32, -5.0f);
}

void test_i32_wrap_i64() {
    static const char *wasm_spec = UNARY_WASM_SPEC(i64, i32, "i32.wrap_i64");

    printf("Testing I32.WRAP_I64...\n");
    CHECK_UNARY("I32.WRAP_I64 (0x123456789ABCDEF0)", i64, 0x123456789ABCDEF0LL, i32, (int32_t)0x9ABCDEF0);
    CHECK_UNARY("I32.WRAP_I64 (0xFFFFFFFF12345678)", i64, 0xFFFFFFFF12345678LL, i32, (int32_t)0x12345678);
}

void test_i32_trunc_f32_s() {
    static const char *wasm_spec = UNARY_WASM_SPEC(f32, i32, "i32.trunc_f32_s");

    printf("Testing I32.TRUNC_F32_S...\n");
    CHECK_UNARY("I32.TRUNC_F32_S (10.5f)", f32, 10.5f, i32, 10);
    CHECK_UNARY("I32.TRUNC_F32_S (-10.5f)", f32, -10.5f, i32, -10);
    CHECK_UNARY_TRAP("I32.TRUNC_F32_S (NaN)", f32, NAN, i32);
    CHECK_UNARY_TRAP("I32.TRUNC_F32_S (Infinity)", f32, INFINITY, i32);
    CHECK_UNARY_TRAP("I32.TRUNC_F32_S (too large)", f32, 2147483648.0f /*INT32_MAX + 1*/, i32);
}

void test_i32_trunc_f32_u() {
    static const char *wasm_spec = UNARY_WASM_SPEC(f32, i32, "i32.trunc_f32_u");

    printf("Testing I32.TRUNC_F32_U...\n");
    CHECK_UNARY("I32.TRUNC_F32_U (10.5f)", f32, 10.5f, i32, 10);
    CHECK_UNARY_TRAP("I32.TRUNC_F32_U (-10.5f)", f32, -10.5f, i32);
    CHECK_UNARY_TRAP("I32.TRUNC_F32_U (too large)", f32, 4294967296.0f /*UINT32_MAX + 1*/, i32);
}

void test_i32_trunc_f64_s() {
    static const char *wasm_spec = UNARY_WASM_SPEC(f64, i32, "i32.trunc_f64_s");

    printf("Testing I32.TRUNC_F64_S...\n");
    CHECK_UNARY("I32.TRUNC_F64_S (10.5)", f64, 10.5, i32, 10);
    CHECK_UNARY("I32.TRUNC_F64_S (-10.5)", f64, -10.5, i32, -10);
    CHECK_UNARY_TRAP("I32.TRUNC_F64_S (NaN)", f64, NAN, i32);
    CHECK_UNARY_TRAP("I32.TRUNC_F64_S (Infinity)", f64, INFINITY, i32);
    CHECK_UNARY_TRAP("I32.TRUNC_F64_S (too large)", f64, 2147483648.0 /*INT32_MAX + 1*/, i32);
}

void test_i32_trunc_f64_u() {
    static const char *wasm_spec = UNARY_WASM_SPEC(f64, i32, "i32.trunc_f64_u");

    printf("Testing I32.TRUNC_F64_U...\n");
    CHECK_UNARY("I32.TRUNC_F64_U (10.5)", f64, 10.5, i32, 10);
    CHECK_UNARY_TRAP("I32.TRUNC_F64_U (-10.5)", f64, -10.5, i32);
    CHECK_UNARY_TRAP("I32.TRUNC_F64_U (too large)", f64, 4294967296.0 /*UINT32_MAX + 1*/, i32);
}

void test_i64_extend_i32_s() {
    static const char *wasm_spec = UNARY_WASM_SPEC(i32, i64, "i64.extend_i32_s");

    printf("Testing I64.EXTEND_I32_S...\n");
    CHECK_UNARY("I64.EXTEND_I32_S (12345)", i32, 12345, i64, 12345LL);
    CHECK_UNARY("I64.EXTEND_I32_S (-12345)", i32, -12345, i64, -12345LL);
    CHECK_UNARY("I64.EXTEND_I32_S (0x80000000)", i32, 0x80000000, i64, (int64_t)0xFFFFFFFF80000000LL); // Smallest signed 32-bit integer
}

void test_i64_extend_i32_u() {
    static const char *wasm_spec = UNARY_WASM_SPEC(i32, i64, "i64.extend_i32_u");

    printf("Testing I64.EXTEND_I32_U...\n");
    CHECK_UNARY("I64.EXTEND_I32_U (12345)", i32, 12345, i64, 12345LL);
    CHECK_UNARY("I64.EXTEND_I32_U (0xFFFFFFFF)", i32, 0xFFFFFFFF, i64, 0x00000000FFFFFFFFLL); // Largest unsigned 32-bit integer
}

void test_i64_trunc_f32_s() {
    static const char *wasm_spec = UNARY_WASM_SPEC(f32, i64, "i64.trunc_f32_s");

    printf("Testing I64.TRUNC_F32_S...\n");
    CHECK_UNARY("I64.TRUNC_F32_S (10.5f)", f32, 10.5f, i64, 10LL);
    CHECK_UNARY("I64.TRUNC_F32_S (-10.5f)", f32, -10.5f, i64, -10LL);
    CHECK_UNARY_TRAP("I64.TRUNC_F32_S (NaN)", f32, NAN, i64);
    CHECK_UNARY_TRAP("I64.TRUNC_F32_S (Infinity)", f32, INFINITY, i64);
    CHECK_UNARY_TRAP("I64.TRUNC_F32_S (too large)", f32, 9223372036854775808.0f /*INT64_MAX + 1*/, i64);
}

void test_i64_trunc_f32_u() {
    static const char *wasm_spec = UNARY_WASM_SPEC(f32, i64, "i64.trunc_f32_u");

    printf("Testing I64.TRUNC_F32_U...\n");
    CHECK_UNARY("I64.TRUNC_F32_U (10.5f)", f32, 10.5f, i64, 10LL);
    CHECK_UNARY_TRAP("I64.TRUNC_F32_U (-10.5f)", f32, -10.5f, i64);
    CHECK_UNARY_TRAP("I64.TRUNC_F32_U (too large)", f32, 18446744073709551616.0f /*UINT64_MAX + 1*/, i64);
}

void test_i64_trunc_f64_s() {
    static const char *wasm_spec = UNARY_WASM_SPEC(f64, i64, "i64.trunc_f64_s");

    printf("Testing I64.TRUNC_F64_S...\n");
    CHECK_UNARY("I64.TRUNC_F64_S (10.5)", f64, 10.5, i64, 10LL);
    CHECK_UNARY("I64.TRUNC_F64_S (-10.5)", f64, -10.5, i64, -10LL);
    CHECK_UNARY_TRAP("I64.TRUNC_F64_S (NaN)", f64, NAN, i64);
    CHECK_UNARY_TRAP("I64.TRUNC_F64_S (Infinity)", f64, INFINITY, i64);
    CHECK_UNARY_TRAP("I64.TRUNC_F64_S (too large)", f64, 9223372036854775808.0 /*INT64_MAX + 1*/, i64);
}

void test_i64_trunc_f64_u() {
    static const char *wasm_spec = UNARY_WASM_SPEC(f64, i64, "i64.trunc_f64_u");

    printf("Testing I64.TRUNC_F64_U...\n");
    CHECK_UNARY("I64.TRUNC_F64_U (10.5)", f64, 10.5, i64, 10LL);
    CHECK_UNARY_TRAP("I64.TRUNC_F64_U (-10.5)", f64, -10.5, i64);
    CHECK_UNARY_TRAP("I64.TRUNC_F64_U (too large)", f64, 18446744073709551616.0 /*UINT64_MAX + 1*/, i64);
}

void test_f32_convert_i32_s() {
    static const char *wasm_spec = UNARY_WASM_SPEC(i32, f32, "f32.convert_i32_s");

    printf("Testing F32.CONVERT_I32_S...\n");
    CHECK_UNARY("F32.CONVERT_I32_S (12345)", i32, 12345, f32, 12345.0f);
    CHECK_UNARY("F32.CONVERT_I32_S (-12345)", i32, -12345, f32, -12345.0f);
}

void test_f32_convert_i32_u() {
    static const char *wasm_spec = UNARY_WASM_SPEC(i32, f32, "f32.convert_i32_u");

    printf("Testing F32.CONVERT_I32_U...\n");
    CHECK_UNARY("F32.CONVERT_I32_U (12345)", i32, 12345, f32, 12345.0f);
    CHECK_UNARY("F32.CONVERT_I32_U (0xFFFFFFFF)", i32, 0xFFFFFFFF /*UINT32_MAX*/, f32, 4294967295.0f);
}

void test_f32_convert_i64_s() {
    static const char *wasm_spec = UNARY_WASM_SPEC(i64, f32, "f32.convert_i64_s");

    printf("Testing F32.CONVERT_I64_S...\n");
    CHECK_UNARY("F32.CONVERT_I64_S (123456789012345)", i64, 123456789012345LL, f32, 123456788103168.0f); // Precision loss expected
    CHECK_UNARY("F32.CONVERT_I64_S (-123456789012345)", i64, -123456789012345LL, f32, -123456788103168.0f); // Precision loss expected
}

void test_f32_convert_i64_u() {
    static const char *wasm_spec = UNARY_WASM_SPEC(i64, f32, "f32.convert_i64_u");

    printf("Testing F32.CONVERT_I64_U...\n");
    CHECK_UNARY("F32.CONVERT_I64_U (123456789012345)", i64, 123456789012345ULL, f32, 123456788103168.0f); // Precision loss expected
    CHECK_UNARY("F32.CONVERT_I64_U (0xFFFFFFFFFFFFFFFF)", i64, 0xFFFFFFFFFFFFFFFFULL /*UINT64_MAX*/, f32, 1.8446744073709552e+19f); // Precision loss expected
}

void test_f32_demote_f64() {
    static const char *wasm_spec = UNARY_WASM_SPEC(f64, f32, "f32.demote_f64");

    printf("Testing F32.DEMOTE_F64...\n");
    CHECK_UNARY("F32.DEMOTE_F64 (123.456)", f64, 123.456, f32, 123.456f);
    CHECK_UNARY("F32.DEMOTE_F64 (large double to float)", f64, 1.2345678901234567e+300, f32, INFINITY); // A large double that will become infinity in float
}

void test_f64_convert_i32_s() {
    static const char *wasm_spec = UNARY_WASM_SPEC(i32, f64, "f64.convert_i32_s");

    printf("Testing F64.CONVERT_I32_S...\n");
    CHECK_UNARY("F64.CONVERT_I32_S (12345)", i32, 12345, f64, 12345.0);
    CHECK_UNARY("F64.CONVERT_I32_S (-12345)", i32, -12345, f64, -12345.0);
}

void test_f64_convert_i32_u() {
    static const char *wasm_spec = UNARY_WASM_SPEC(i32, f64, "f64.convert_i32_u");

    printf("Testing F64.CONVERT_I32_U...\n");
    CHECK_UNARY("F64.CONVERT_I32_U (12345)", i32, 12345, f64, 12345.0);
    CHECK_UNARY("F64.CONVERT_I32_U (0xFFFFFFFF)", i32, 0xFFFFFFFF /*UINT32_MAX*/, f64, 4294967295.0);
}

void test_f64_convert_i64_s() {
    static const char *wasm_spec = UNARY_WASM_SPEC(i64, f64, "f64.convert_i64_s");

    printf("Testing F64.CONVERT_I64_S...\n");
    CHECK_UNARY("F64.CONVERT_I64_S (1234567890123456789)", i64, 1234567890123456789LL, f64, 1234567890123456768.0); // Precision loss expected
    CHECK_UNARY("F64.CONVERT_I64_S (-1234567890123456789)", i64, -1234567890123456789LL, f64, -1234567890123456768.0); // Precision loss expected
}

void test_f64_convert_i64_u() {
    static const char *wasm_spec = UNARY_WASM_SPEC(i64, f64, "f64.convert_i64_u");

    printf("Testing F64.CONVERT_I64_U...\n");
    CHECK_UNARY("F64.CONVERT_I64_U (1234567890123456789)", i64, 1234567890123456789ULL, f64, 1234567890123456768.0); // Precision loss expected
    CHECK_UNARY("F64.CONVERT_I64_U (0xFFFFFFFFFFFFFFFF)", i64, 0xFFFFFFFFFFFFFFFFULL /*UINT64_MAX*/, f64, 1.8446744073709552e+19); // Precision loss expected
}

void test_f64_promote_f32() {
    static const char *wasm_spec = UNARY_WASM_SPEC(f32, f64, "f64.promote_f32");

    printf("Testing F64.PROMOTE_F32...\n");
    CHECK_UNARY("F64.PROMOTE_F32 (123.456f)", f32, 123.456f, f64, (double)123.456f);
}

void test_i32_reinterpret_f32() {
    static const char *wasm_spec = UNARY_WASM_SPEC(f32, i32, "i32.reinterpret_f32");

    printf("Testing I32.REINTERPRET_F32...\n");
    CHECK_UNARY("I32.REINTERPRET_F32 (1.0f)", f32, 1.0f, i32, 0x3F800000);
}

void test_i64_reinterpret_f64() {
    static const char *wasm_spec = UNARY_WASM_SPEC(f64, i64, "i64.reinterpret_f64");

    printf("Testing I64.REINTERPRET_F64...\n");
    CHECK_UNARY("I64.REINTERPRET_F64 (1.0)", f64, 1.0, i64, 0x3FF0000000000000ULL);
}

void test_f32_reinterpret_i32() {
    static const char *wasm_spec = UNARY_WASM_SPEC(i32, f32, "f32.reinterpret_i32");

    printf("Testing F32.REINTERPRET_I32...\n");
    CHECK_UNARY("F32.REINTERPRET_I32 (0x3F800000)", i32, 0x3F800000, f32, 1.0f);
}

void test_f64_reinterpret_i64() {
    static const char *wasm_spec = UNARY_WASM_SPEC(i64, f64, "f64.reinterpret_i64");

    printf("Testing F64.REINTERPRET_I64...\n");
    CHECK_UNARY("F64.REINTERPRET_I64 (0x3FF0000000000000)", i64, 0x3FF0000000000000ULL, f64, 1.0);
}

void test_i32_trunc_sat_f32_s() {
    static const char *wasm_spec = UNARY_WASM_SPEC(f32, i32, "i32.trunc_sat_f32_s");

    printf("Testing I32.TRUNC_SAT_F32_S...\n");
    CHECK_UNARY("I32.TRUNC_SAT_F32_S (10.5f)", f32, 10.5f, i32, 10);
    CHECK_UNARY("I32.TRUNC_SAT_F32_S (-10.5f)", f32, -10.5f, i32, -10);
    CHECK_UNARY("I32.TRUNC_SAT_F32_S (NaN)", f32, NAN, i32, 0); // NaN should result in 0
    CHECK_UNARY("I32.TRUNC_SAT_F32_S (Infinity)", f32, INFINITY, i32, INT32_MAX); // Positive Infinity should saturate to INT32_MAX
    CHECK_UNARY("I32.TRUNC_SAT_F32_S (-Infinity)", f32, -INFINITY, i32, INT32_MIN); // Negative Infinity should saturate to INT32_MIN
    CHECK_UNARY("I32.TRUNC_SAT_F32_S (too large)", f32, 2147483648.0f /*INT32_MAX + 1*/, i32, INT32_MAX); // Value greater than INT32_MAX should saturate to INT32_MAX
    CHECK_UNARY("I32.TRUNC_SAT_F32_S (too small)", f32, -2147483649.0f /*INT32_MIN - 1*/, i32, INT32_MIN); // Value less than INT32_MIN should saturate to INT32_MIN
}

void test_i32_trunc_sat_f32_u() {
    static const char *wasm_spec = UNARY_WASM_SPEC(f32, i32, "i32.trunc_sat_f32_u");

    printf("Testing I32.TRUNC_SAT_F32_U...\n");
    CHECK_UNARY("I32.TRUNC_SAT_F32_U (10.5f)", f32, 10.5f, i32, 10);
    CHECK_UNARY("I32.TRUNC_SAT_F32_U (NaN)", f32, NAN, i32, 0); // NaN should result in 0
    CHECK_UNARY("I32.TRUNC_SAT_F32_U (Infinity)", f32, INFINITY, i32, (int32_t)UINT32_MAX); // Positive Infinity should saturate to UINT32_MAX
    CHECK_UNARY("I32.TRUNC_SAT_F32_U (-Infinity)", f32, -INFINITY, i32, 0); // Negative Infinity should saturate to 0
    CHECK_UNARY("I32.TRUNC_SAT_F32_U (too large)", f32, 4294967296.0f /*UINT32_MAX + 1*/, i32, (int32_t)UINT32_MAX); // Value greater than UINT32_MAX should saturate to UINT32_MAX
    CHECK_UNARY("I32.TRUNC_SAT_F32_U (negative)", f32, -0.5f, i32, 0); // Value less than 0 should saturate to 0
}

void test_i32_trunc_sat_f64_s() {
    static const char *wasm_spec = UNARY_WASM_SPEC(f64, i32, "i32.trunc_sat_f64_s");

    printf("Testing I32.TRUNC_SAT_F64_S...\n");
    CHECK_UNARY("I32.TRUNC_SAT_F64_S (10.5)", f64, 10.5, i32, 10);
    CHECK_UNARY("I32.TRUNC_SAT_F64_S (-10.5)", f64, -10.5, i32, -10);
    CHECK_UNARY("I32.TRUNC_SAT_F64_S (NaN)", f64, NAN, i32, 0); // NaN should result in 0
    CHECK_UNARY("I32.TRUNC_SAT_F64_S (Infinity)", f64, INFINITY, i32, INT32_MAX); // Positive Infinity should saturate to INT32_MAX
    CHECK_UNARY("I32.TRUNC_SAT_F64_S (-Infinity)", f64, -INFINITY, i32, INT32_MIN); // Negative Infinity should saturate to INT32_MIN
    CHECK_UNARY("I32.TRUNC_SAT_F64_S (too large)", f64, 2147483648.0 /*INT32_MAX + 1*/, i32, INT32_MAX); // Value greater than INT32_MAX should saturate to INT32_MAX
    CHECK_UNARY("I32.TRUNC_SAT_F64_S (too small)", f64, -2147483649.0 /*INT32_MIN - 1*/, i32, INT32_MIN); // Value less than INT32_MIN should saturate to INT32_MIN
}

void test_i32_trunc_sat_f64_u() {
    static const char *wasm_spec = UNARY_WASM_SPEC(f64, i32, "i32.trunc_sat_f64_u");

    printf("Testing I32.TRUNC_SAT_F64_U...\n");
    CHECK_UNARY("I32.TRUNC_SAT_F64_U (10.5)", f64, 10.5, i32, 10);
    CHECK_UNARY("I32.TRUNC_SAT_F64_U (NaN)", f64, NAN, i32, 0); // NaN should result in 0
    CHECK_UNARY("I32.TRUNC_SAT_F64_U (Infinity)", f64, INFINITY, i32, (int32_t)UINT32_MAX); // Positive Infinity should saturate to UINT32_MAX
    CHECK_UNARY("I32.TRUNC_SAT_F64_U (-Infinity)", f64, -INFINITY, i32, 0); // Negative Infinity should saturate to 0
    CHECK_UNARY("I32.TRUNC_SAT_F64_U (too large)", f64, 4294967296.0 /*UINT32_MAX + 1*/, i32, (int32_t)UINT32_MAX); // Value greater than UINT32_MAX should saturate to UINT32_MAX
    CHECK_UNARY("I32.TRUNC_SAT_F64_U (negative)", f64, -0.5, i32, 0); // Value less than 0 should saturate to 0
}

void test_i32_extend8_s() {
    static const char *wasm_spec = UNARY_WASM_SPEC(i32, i32, "i32.extend8_s");

    printf("Testing I32.EXTEND8_S...\n");
    CHECK_UNARY("I32.EXTEND8_S (0x7F)", i32, 0x7F, i32, 0x7F);
    CHECK_UNARY("I32.EXTEND8_S (0x80)", i32, 0x80, i32, (int32_t)0xFFFFFF80);
    CHECK_UNARY("I32.EXTEND8_S (0xFF)", i32, 0xFF, i32, (int32_t)0xFFFFFFFF);
    CHECK_UNARY("I32.EXTEND8_S (0x00000001)", i32, 0x00000001, i32, 0x00000001);
    CHECK_UNARY("I32.EXTEND8_S (0x12345678)", i32, 0x12345678, i32, 0x00000078);
    CHECK_UNARY("I32.EXTEND8_S (0x90ABCDEF)", i32, 0x90ABCDEF, i32, 0xFFFFFFEF);
    CHECK_UNARY("I32.EXTEND8_S (0xFFFFFFFF)", i32, 0xFFFFFFFF, i32, (int32_t)0xFFFFFFFF);
}

void test_i32_extend16_s() {
    static const char *wasm_spec = UNARY_WASM_SPEC(i32, i32, "i32.extend16_s");

    printf("Testing I32.EXTEND16_S...\n");
    CHECK_UNARY("I32.EXTEND16_S (0x7FFF)", i32, 0x7FFF, i32, 0x7FFF);
    CHECK_UNARY("I32.EXTEND16_S (0x8000)", i32, 0x8000, i32, (int32_t)0xFFFF8000);
    CHECK_UNARY("I32.EXTEND16_S (0xFFFF)", i32, 0xFFFF, i32, (int32_t)0xFFFFFFFF);
    CHECK_UNARY("I32.EXTEND16_S (0x00000001)", i32, 0x00000001, i32, 0x00000001);
    CHECK_UNARY("I32.EXTEND16_S (0x12345678)", i32, 0x12345678, i32, 0x00005678);
    CHECK_UNARY("I32.EXTEND16_S (0x90ABCDEF)", i32, 0x90ABCDEF, i32, 0xFFFFCDEF);
    CHECK_UNARY("I32.EXTEND16_S (0xFFFFFFFF)", i32, 0xFFFFFFFF, i32, (int32_t)0xFFFFFFFF);
}

void test_i64_extend8_s() {
    static const char *wasm_spec = UNARY_WASM_SPEC(i64, i64, "i64.extend8_s");

    printf("Testing I64.EXTEND8_S...\n");
    CHECK_UNARY("I64.EXTEND8_S (0x7F)", i64, 0x7F, i64, 0x7FLL);
    CHECK_UNARY("I64.EXTEND8_S (0x80)", i64, 0x80, i64, (int64_t)0xFFFFFFFFFFFFFF80LL);
    CHECK_UNARY("I64.EXTEND8_S (0xFF)", i64, 0xFF, i64, (int64_t)0xFFFFFFFFFFFFFFFFLL);
    CHECK_UNARY("I64.EXTEND8_S (0x0000000000000001)", i64, 0x0000000000000001LL, i64, 0x0000000000000001LL);
    CHECK_UNARY("I64.EXTEND8_S (0x1234567890ABCDEF)", i64, 0x1234567890ABCDEFLL, i64, (int64_t)0xFFFFFFFFFFFFFFEFLL);
    CHECK_UNARY("I64.EXTEND8_S (0xFDECBA9876543210)", i64, 0xFDECBA9876543210LL, i64, 0x0000000000000010LL);
    CHECK_UNARY("I64.EXTEND8_S (0xFFFFFFFFFFFFFFFF)", i64, 0xFFFFFFFFFFFFFFFFLL, i64, (int64_t)0xFFFFFFFFFFFFFFFFLL);
}

void test_i64_extend16_s() {
    static const char *wasm_spec = UNARY_WASM_SPEC(i64, i64, "i64.extend16_s");

    printf("Testing I64.EXTEND16_S...\n");
    CHECK_UNARY("I64.EXTEND16_S (0x7FFF)", i64, 0x7FFF, i64, 0x7FFFLL);
    CHECK_UNARY("I64.EXTEND16_S (0x8000)", i64, 0x8000, i64, (int64_t)0xFFFFFFFFFFFF8000LL);
    CHECK_UNARY("I64.EXTEND16_S (0xFFFF)", i64, 0xFFFF, i64, (int64_t)0xFFFFFFFFFFFFFFFFLL);
    CHECK_UNARY("I64.EXTEND16_S (0x0000000000000001)", i64, 0x0000000000000001LL, i64, 0x0000000000000001LL);
    CHECK_UNARY("I64.EXTEND16_S (0x1234567890ABCDEF)", i64, 0x1234567890ABCDEFLL, i64, (int64_t)0xFFFFFFFFFFFFCDEFLL);
    CHECK_UNARY("I64.EXTEND16_S (0xFDECBA9876543210)", i64, 0xFDECBA9876543210LL, i64, 0x0000000000003210LL);
    CHECK_UNARY("I64.EXTEND16_S (0xFFFFFFFFFFFFFFFF)", i64, 0xFFFFFFFFFFFFFFFFLL, i64, (int64_t)0xFFFFFFFFFFFFFFFFLL);
}

void test_i64_extend32_s() {
    static const char *wasm_spec = UNARY_WASM_SPEC(i64, i64, "i64.extend32_s");

    printf("Testing I64.EXTEND32_S...\n");
    CHECK_UNARY("I64.EXTEND32_S (0x7FFFFFFF)", i64, 0x7FFFFFFF, i64, 0x7FFFFFFFLL);
    CHECK_UNARY("I64.EXTEND32_S (0x80000000)", i64, 0x80000000, i64, (int64_t)0xFFFFFFFF80000000LL);
    CHECK_UNARY("I64.EXTEND32_S (0xFFFFFFFF)", i64, 0xFFFFFFFF, i64, (int64_t)0xFFFFFFFFFFFFFFFFLL);
    CHECK_UNARY("I64.EXTEND32_S (0x0000000000000001)", i64, 0x0000000000000001LL, i64, 0x0000000000000001LL);
    CHECK_UNARY("I64.EXTEND32_S (0x1234567890ABCDEF)", i64, 0x1234567890ABCDEFLL, i64, (int64_t)0xFFFFFFFF90ABCDEFLL);
    CHECK_UNARY("I64.EXTEND32_S (0xFDECBA9876543210)", i64, 0xFDECBA9876543210LL, i64, 0x0000000076543210LL);
    CHECK_UNARY("I64.EXTEND32_S (0xFFFFFFFFFFFFFFFF)", i64, 0xFFFFFFFFFFFFFFFFLL, i64, (int64_t)0xFFFFFFFFFFFFFFFFLL);
}

void test_i64_trunc_sat_f32_s() {
    static const char *wasm_spec = UNARY_WASM_SPEC(f32, i64, "i64.trunc_sat_f32_s");

    printf("Testing I64.TRUNC_SAT_F32_S...\n");
    CHECK_UNARY("I64.TRUNC_SAT_F32_S (10.5f)", f32, 10.5f, i64, 10LL);
    CHECK_UNARY("I64.TRUNC_SAT_F32_S (-10.5f)", f32, -10.5f, i64, -10LL);
    CHECK_UNARY("I64.TRUNC_SAT_F32_S (NaN)", f32, NAN, i64, 0LL); // NaN should result in 0
    CHECK_UNARY("I64.TRUNC_SAT_F32_S (Infinity)", f32, INFINITY, i64, INT64_MAX); // Positive Infinity should saturate to INT64_MAX
    CHECK_UNARY("I64.TRUNC_SAT_F32_S (-Infinity)", f32, -INFINITY, i64, INT64_MIN); // Negative Infinity should saturate to INT64_MIN
    CHECK_UNARY("I64.TRUNC_SAT_F32_S (too large)", f32, (float)INT64_MAX + 100.0f, i64, INT64_MAX); // Value greater than INT64_MAX should saturate to INT64_MAX
    CHECK_UNARY("I64.TRUNC_SAT_F32_S (too small)", f32, (float)INT64_MIN - 100.0f, i64, INT64_MIN); // Value less than INT64_MIN should saturate to INT64_MIN
}

void test_i64_trunc_sat_f32_u() {
    static const char *wasm_spec = UNARY_WASM_SPEC(f32, i64, "i64.trunc_sat_f32_u");

    printf("Testing I64.TRUNC_SAT_F32_U...\n");
    CHECK_UNARY("I64.TRUNC_SAT_F32_U (10.5f)", f32, 10.5f, i64, 10ULL);
    CHECK_UNARY("I64.TRUNC_SAT_F32_U (NaN)", f32, NAN, i64, 0ULL); // NaN should result in 0
    CHECK_UNARY("I64.TRUNC_SAT_F32_U (Infinity)", f32, INFINITY, i64, (int64_t)UINT64_MAX); // Positive Infinity should saturate to UINT64_MAX
    CHECK_UNARY("I64.TRUNC_SAT_F32_U (-Infinity)", f32, -INFINITY, i64, 0ULL); // Negative Infinity should saturate to 0
    CHECK_UNARY("I64.TRUNC_SAT_F32_U (too large)", f32, (float)UINT64_MAX + 100.0f, i64, (int64_t)UINT64_MAX); // Value greater than UINT64_MAX should saturate to UINT64_MAX
    CHECK_UNARY("I64.TRUNC_SAT_F32_U (negative)", f32, -0.5f, i64, 0ULL); // Value less than 0 should saturate to 0
}

void test_i64_trunc_sat_f64_s() {
    static const char *wasm_spec = UNARY_WASM_SPEC(f64, i64, "i64.trunc_sat_f64_s");

    printf("Testing I64.TRUNC_SAT_F64_S...\n");
    CHECK_UNARY("I64.TRUNC_SAT_F64_S (10.5)", f64, 10.5, i64, 10LL);
    CHECK_UNARY("I64.TRUNC_SAT_F64_S (-10.5)", f64, -10.5, i64, -10LL);
    CHECK_UNARY("I64.TRUNC_SAT_F64_S (NaN)", f64, NAN, i64, 0LL); // NaN should result in 0
    CHECK_UNARY("I64.TRUNC_SAT_F64_S (Infinity)", f64, INFINITY, i64, INT64_MAX); // Positive Infinity should saturate to INT64_MAX
    CHECK_UNARY("I64.TRUNC_SAT_F64_S (-Infinity)", f64, -INFINITY, i64, INT64_MIN); // Negative Infinity should saturate to INT64_MIN
    CHECK_UNARY("I64.TRUNC_SAT_F64_S (too large)", f64, (double)INT64_MAX + 100.0, i64, INT64_MAX); // Value greater than INT64_MAX should saturate to INT64_MAX
    CHECK_UNARY("I64.TRUNC_SAT_F64_S (too small)", f64, (double)INT64_MIN - 100.0, i64, INT64_MIN); // Value less than INT64_MIN should saturate to INT64_MIN
}

void test_i64_trunc_sat_f64_u() {
    static const char *wasm_spec = UNARY_WASM_SPEC(f64, i64, "i64.trunc_sat_f64_u");

    printf("Testing I64.TRUNC_SAT_F64_U...\n");
    CHECK_UNARY("I64.TRUNC_SAT_F64_U (10.5)", f64, 10.5, i64, 10ULL);
    CHECK_UNARY("I64.TRUNC_SAT_F64_U (NaN)", f64, NAN, i64, 0ULL); // NaN should result in 0
    CHECK_UNARY("I64.TRUNC_SAT_F64_U (Infinity)", f64, INFINITY, i64, (int64_t)UINT64_MAX); // Positive Infinity should saturate to UINT64_MAX
    CHECK_UNARY("I64.TRUNC_SAT_F64_U (-Infinity)", f64, -INFINITY, i64, 0ULL); // Negative Infinity should saturate to 0
    CHECK_UNARY("I64.TRUNC_SAT_F64_U (too large)", f64, (double)UINT64_MAX + 100.0, i64, (int64_t)UINT64_MAX); // Value greater than UINT64_MAX should saturate to UINT64_MAX
    CHECK_UNARY("I64.TRUNC_SAT_F64_U (negative)", f64, -0.5, i64, 0ULL); // Value less than 0 should saturate to 0
}

// 3af6770: Fix wah_trunc_fN_to_iN for edge cases.
void test_trunc_edge_cases() {
    printf("Testing trunc edge cases (3af6770)...\n");

    // i32.trunc_f32_s: -0.0 should give 0 (not trap)
    {
        static const char *wasm_spec = UNARY_WASM_SPEC(f32, i32, "i32.trunc_f32_s");
        CHECK_UNARY("I32.TRUNC_F32_S (-0.0)", f32, -0.0f, i32, 0);
        // Value just inside range: -2147483648.0f is INT32_MIN, should succeed
        CHECK_UNARY("I32.TRUNC_F32_S (INT32_MIN)", f32, -2147483648.0f, i32, INT32_MIN);
    }

    // i32.trunc_f32_u: -0.0 should give 0 (not trap)
    {
        static const char *wasm_spec = UNARY_WASM_SPEC(f32, i32, "i32.trunc_f32_u");
        wah_value_t params[1] = { { .f32 = -0.0f } };
        run_test_i32("I32.TRUNC_F32_U (-0.0)", wasm_spec, params, 1, 0, false);
    }

    // i32.trunc_f64_s: -0.0 should give 0
    {
        static const char *wasm_spec = UNARY_WASM_SPEC(f64, i32, "i32.trunc_f64_s");
        CHECK_UNARY("I32.TRUNC_F64_S (-0.0)", f64, -0.0, i32, 0);
        // Just inside: 2147483647.0 is max i32
        CHECK_UNARY("I32.TRUNC_F64_S (2147483647.0)", f64, 2147483647.0, i32, 2147483647);
        // Just outside: 2147483648.0 should trap
        CHECK_UNARY_TRAP("I32.TRUNC_F64_S (2147483648.0)", f64, 2147483648.0, i32);
    }

    // i64.trunc_f64_s: -0.0 should give 0
    {
        static const char *wasm_spec = UNARY_WASM_SPEC(f64, i64, "i64.trunc_f64_s");
        CHECK_UNARY("I64.TRUNC_F64_S (-0.0)", f64, -0.0, i64, (int64_t)0);
    }

    // i64.trunc_f64_u: -0.0 should give 0
    {
        static const char *wasm_spec = UNARY_WASM_SPEC(f64, i64, "i64.trunc_f64_u");
        wah_value_t params[1] = { { .f64 = -0.0 } };
        run_test_i64("I64.TRUNC_F64_U (-0.0)", wasm_spec, params, 1, 0, false);
        // -0.99999 truncates to -0.0 which is 0, should not trap
        wah_value_t params2[1] = { { .f64 = -0.99999 } };
        run_test_i64("I64.TRUNC_F64_U (-0.99999)", wasm_spec, params2, 1, 0, false);
    }
}

int main() {
    test_i32_and();
    test_i32_eq();
    test_i32_eqz();
    test_i64_eqz();
    test_i32_popcnt();
    test_i64_clz();
    test_i32_rotl();
    test_f64_nearest();
    test_f32_min();

    test_i32_wrap_i64();
    test_i32_trunc_f32_s();
    test_i32_trunc_f32_u();
    test_i32_trunc_f64_s();
    test_i32_trunc_f64_u();

    test_i64_extend_i32_s();
    test_i64_extend_i32_u();
    test_i64_trunc_f32_s();
    test_i64_trunc_f32_u();
    test_i64_trunc_f64_s();
    test_i64_trunc_f64_u();

    test_f32_convert_i32_s();
    test_f32_convert_i32_u();
    test_f32_convert_i64_s();
    test_f32_convert_i64_u();

    test_f32_demote_f64();

    test_f64_convert_i32_s();
    test_f64_convert_i32_u();
    test_f64_convert_i64_s();
    test_f64_convert_i64_u();

    test_f64_promote_f32();

    test_i32_reinterpret_f32();
    test_i64_reinterpret_f64();
    test_f32_reinterpret_i32();
    test_f64_reinterpret_i64();

    test_i32_trunc_sat_f32_s();
    test_i32_trunc_sat_f32_u();
    test_i32_trunc_sat_f64_s();
    test_i32_trunc_sat_f64_u();

    test_i64_trunc_sat_f32_s();
    test_i64_trunc_sat_f32_u();
    test_i64_trunc_sat_f64_s();
    test_i64_trunc_sat_f64_u();

    test_i32_extend8_s();
    test_i32_extend16_s();
    test_i64_extend8_s();
    test_i64_extend16_s();
    test_i64_extend32_s();

    test_trunc_edge_cases();

    printf("\nSUMMARY: All tests PASSED!\n");
    return 0;
}
