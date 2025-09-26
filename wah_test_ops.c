#define WAH_IMPLEMENTATION
#include "wah.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#define FLOAT_TOLERANCE 1e-6

// --- Test Helper Functions ---

// Generic test runner
#define DEFINE_RUN_TEST(T, ty, fmt, compare) \
int run_test_##T(const char* test_name, const uint8_t* wasm_bytecode, size_t bytecode_size, \
                 wah_value_t* params, uint32_t param_count, ty expected_result, bool expect_trap) { \
    wah_module_t module; \
    wah_exec_context_t ctx; \
    wah_error_t err; \
    wah_value_t result; \
    int test_status = 0; /* 0 for pass, 1 for fail */ \
    \
    err = wah_parse_module(wasm_bytecode, bytecode_size, &module); \
    if (err != WAH_OK) { \
        printf("  %s: Failed to parse module: %s\n", test_name, wah_strerror(err)); \
        return 1; \
    } \
    err = wah_exec_context_create(&ctx, &module); \
    if (err != WAH_OK) { \
        fprintf(stderr, "  %s: Error creating execution context: %s\n", test_name, wah_strerror(err)); \
        wah_free_module(&module); \
        return 1; \
    } \
    \
    err = wah_call(&ctx, &module, 0, params, param_count, &result); \
    if (expect_trap) { \
        if (err == WAH_ERROR_TRAP) { \
            printf("  %s: PASSED. Expected trap.\n", test_name); \
        } else { \
            printf("  %s: FAILED! Expected trap, but got %s\n", test_name, wah_strerror(err)); \
            test_status = 1; \
        } \
    } else { \
        if (err != WAH_OK) { \
            printf("  %s: Failed to execute: %s\n", test_name, wah_strerror(err)); \
            test_status = 1; \
        } else { \
            if (compare) { \
                printf("  %s: PASSED. Result: " fmt "\n", test_name, result.T); \
            } else { \
                printf("  %s: FAILED! Expected " fmt ", got " fmt "\n", test_name, expected_result, result.T); \
                test_status = 1; \
            } \
        } \
    } \
    \
    wah_exec_context_destroy(&ctx); \
    wah_free_module(&module); \
    return test_status; \
}

#define FLOAT_COMPARE(result_val, expected_val, type) \
    ((isnan(result_val) && isnan(expected_val)) || \
     (isinf(result_val) && isinf(expected_val) && (signbit(result_val) == signbit(expected_val))) || \
     (fabs(result_val - expected_val) <= FLOAT_TOLERANCE))

DEFINE_RUN_TEST(i32, int32_t, "%d", result.i32 == expected_result)
DEFINE_RUN_TEST(i64, int64_t, "%lld", result.i64 == expected_result)
DEFINE_RUN_TEST(f32, float, "%f", FLOAT_COMPARE(result.f32, expected_result, float))
DEFINE_RUN_TEST(f64, double, "%f", FLOAT_COMPARE(result.f64, expected_result, double))

#define run_test_i32(n,b,p,e,t) run_test_i32(n,b,sizeof(b),p,sizeof(p)/sizeof(*p),e,t)
#define run_test_i64(n,b,p,e,t) run_test_i64(n,b,sizeof(b),p,sizeof(p)/sizeof(*p),e,t)
#define run_test_f32(n,b,p,e,t) run_test_f32(n,b,sizeof(b),p,sizeof(p)/sizeof(*p),e,t)
#define run_test_f64(n,b,p,e,t) run_test_f64(n,b,sizeof(b),p,sizeof(p)/sizeof(*p),e,t)

// --- Templates ---

// (module (func (param T) (result U) (OPCODE (local.get 0))))
#define UNARY_TEST_WASM(arg_ty, ret_ty, opcode) { \
    0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, \
    0x01, 0x06, 0x01, 0x60, 0x01, arg_ty, 0x01, ret_ty, \
    0x03, 0x02, 0x01, 0x00, \
    0x0a, 0x07, 0x01, 0x05, 0x00, 0x20, 0x00, opcode, 0x0b \
}
#define UNARY_TEST_WASM_FC(arg_ty, ret_ty, subopcode) { \
    0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, \
    0x01, 0x06, 0x01, 0x60, 0x01, arg_ty, 0x01, ret_ty, \
    0x03, 0x02, 0x01, 0x00, \
    0x0a, 0x08, 0x01, 0x06, 0x00, 0x20, 0x00, 0xfc, subopcode, 0x0b \
};

// (module (func (param T1 T2) (result U) (OPCODE (local.get 0) (local.get 1))))
#define BINARY_TEST_WASM(lhs_ty, rhs_ty, ret_ty, opcode) { \
    0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, \
    0x01, 0x07, 0x01, 0x60, 0x02, lhs_ty, rhs_ty, 0x01, ret_ty, \
    0x03, 0x02, 0x01, 0x00, \
    0x0a, 0x09, 0x01, 0x07, 0x00, 0x20, 0x00, 0x20, 0x01, opcode, 0x0b \
}

#define CHECK_UNARY(label, arg_ty, arg, ret_ty, ret) do { \
    wah_value_t params[1] = { { .arg_ty = arg } }; \
    failures += run_test_##ret_ty(label, test_wasm, params, ret, false); \
} while (0)

#define CHECK_UNARY_TRAP(label, arg_ty, arg, ret_ty) do { \
    wah_value_t params[1] = { { .arg_ty = arg } }; \
    failures += run_test_##ret_ty(label, test_wasm, params, 0, true); \
} while (0)

#define CHECK_BINARY(label, lhs_ty, lhs, rhs_ty, rhs, ret_ty, ret) do { \
    wah_value_t params[2] = { { .lhs_ty = lhs }, { .rhs_ty = rhs } }; \
    failures += run_test_##ret_ty(label, test_wasm, params, ret, false); \
} while (0)

#define CHECK_BINARY_TRAP(label, lhs_ty, lhs, rhs_ty, rhs, ret_ty) do { \
    wah_value_t params[2] = { { .lhs_ty = lhs }, { .rhs_ty = rhs } }; \
    failures += run_test_##ret_ty(label, test_wasm, params, 0, true); \
} while (0)

// --- Individual Test Functions ---

int test_i32_and() {
    int failures = 0;
    static const uint8_t test_wasm[] = BINARY_TEST_WASM(0x7f, 0x7f, 0x7f, 0x71);

    printf("\n=== Testing I32.AND ===\n");
    CHECK_BINARY("I32.AND (0xFF & 0x0F)", i32, 0xFF, i32, 0x0F, i32, 0x0F);
    return failures;
}

int test_i32_eq() {
    int failures = 0;
    static const uint8_t test_wasm[] = BINARY_TEST_WASM(0x7f, 0x7f, 0x7f, 0x46);

    printf("\n=== Testing I32.EQ ===\n");
    CHECK_BINARY("I32.EQ (42 == 42)", i32, 42, i32, 42, i32, 1);
    CHECK_BINARY("I32.EQ (42 == 24)", i32, 42, i32, 24, i32, 0);
    return failures;
}

int test_i32_popcnt() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM(0x7f, 0x7f, 0x69);

    printf("\n=== Testing I32.POPCNT ===\n");
    CHECK_UNARY("I32.POPCNT (0xAA)", i32, 0b10101010 /*0xAA*/, i32, 4);
    return failures;
}

int test_i64_clz() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM(0x7e, 0x7e, 0x79);

    printf("\n=== Testing I64.CLZ ===\n");
    CHECK_UNARY("I64.CLZ (0x00...0FF)", i64, 0x00000000000000FFULL /*56 leading zeros*/, i64, 56);
    return failures;
}

int test_f64_nearest() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM(0x7c, 0x7c, 0x9e);

    printf("\n=== Testing F64.NEAREST ===\n");
    CHECK_UNARY("F64.NEAREST (2.5)", f64, 2.5, f64, 2.0);
    CHECK_UNARY("F64.NEAREST (3.5)", f64, 3.5, f64, 4.0);
    CHECK_UNARY("F64.NEAREST (-2.5)", f64, -2.5, f64, -2.0);
    CHECK_UNARY("F64.NEAREST (-3.5)", f64, -3.5, f64, -4.0);
    return failures;
}

int test_i32_rotl() {
    int failures = 0;
    static const uint8_t test_wasm[] = BINARY_TEST_WASM(0x7f, 0x7f, 0x7f, 0x77);

    printf("\n=== Testing I32.ROTL ===\n");
    CHECK_BINARY("I32.ROTL (0x80000001, 1)", i32, 0x80000001 /*1000...0001*/, i32, 1, i32, 0x00000003);
    return failures;
}

int test_f32_min() {
    int failures = 0;
    const uint8_t test_wasm[] = BINARY_TEST_WASM(0x7d, 0x7d, 0x7d, 0x96);

    printf("\n=== Testing F32.MIN ===\n");
    CHECK_BINARY("F32.MIN (10.0f, 20.0f)", f32, 10.0f, f32, 20.0f, f32, 10.0f);
    CHECK_BINARY("F32.MIN (5.0f, -5.0f)", f32, 5.0f, f32, -5.0f, f32, -5.0f);
    return failures;
}

int test_i32_wrap_i64() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM(0x7e, 0x7f, 0xa7);

    printf("\n=== Testing I32.WRAP_I64 ===\n");
    CHECK_UNARY("I32.WRAP_I64 (0x123456789ABCDEF0)", i64, 0x123456789ABCDEF0LL, i32, (int32_t)0x9ABCDEF0);
    CHECK_UNARY("I32.WRAP_I64 (0xFFFFFFFF12345678)", i64, 0xFFFFFFFF12345678LL, i32, (int32_t)0x12345678);
    return failures;
}

int test_i32_trunc_f32_s() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM(0x7d, 0x7f, 0xa8);

    printf("\n=== Testing I32.TRUNC_F32_S ===\n");
    CHECK_UNARY("I32.TRUNC_F32_S (10.5f)", f32, 10.5f, i32, 10);
    CHECK_UNARY("I32.TRUNC_F32_S (-10.5f)", f32, -10.5f, i32, -10);
    CHECK_UNARY_TRAP("I32.TRUNC_F32_S (NaN)", f32, NAN, i32);
    CHECK_UNARY_TRAP("I32.TRUNC_F32_S (Infinity)", f32, INFINITY, i32);
    CHECK_UNARY_TRAP("I32.TRUNC_F32_S (too large)", f32, 2147483648.0f /*INT32_MAX + 1*/, i32);
    return failures;
}

int test_i32_trunc_f32_u() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM(0x7d, 0x7f, 0xa9);

    printf("\n=== Testing I32.TRUNC_F32_U ===\n");
    CHECK_UNARY("I32.TRUNC_F32_U (10.5f)", f32, 10.5f, i32, 10);
    CHECK_UNARY_TRAP("I32.TRUNC_F32_U (-10.5f)", f32, -10.5f, i32);
    CHECK_UNARY_TRAP("I32.TRUNC_F32_U (too large)", f32, 4294967296.0f /*UINT32_MAX + 1*/, i32);
    return failures;
}

int test_i32_trunc_f64_s() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM(0x7c, 0x7f, 0xaa);

    printf("\n=== Testing I32.TRUNC_F64_S ===\n");
    CHECK_UNARY("I32.TRUNC_F64_S (10.5)", f64, 10.5, i32, 10);
    CHECK_UNARY("I32.TRUNC_F64_S (-10.5)", f64, -10.5, i32, -10);
    CHECK_UNARY_TRAP("I32.TRUNC_F64_S (NaN)", f64, NAN, i32);
    CHECK_UNARY_TRAP("I32.TRUNC_F64_S (Infinity)", f64, INFINITY, i32);
    CHECK_UNARY_TRAP("I32.TRUNC_F64_S (too large)", f64, 2147483648.0 /*INT32_MAX + 1*/, i32);
    return failures;
}

int test_i32_trunc_f64_u() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM(0x7c, 0x7f, 0xab);

    printf("\n=== Testing I32.TRUNC_F64_U ===\n");
    CHECK_UNARY("I32.TRUNC_F64_U (10.5)", f64, 10.5, i32, 10);
    CHECK_UNARY_TRAP("I32.TRUNC_F64_U (-10.5)", f64, -10.5, i32);
    CHECK_UNARY_TRAP("I32.TRUNC_F64_U (too large)", f64, 4294967296.0 /*UINT32_MAX + 1*/, i32);
    return failures;
}

int test_i64_extend_i32_s() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM(0x7f, 0x7e, 0xac);

    printf("\n=== Testing I64.EXTEND_I32_S ===\n");
    CHECK_UNARY("I64.EXTEND_I32_S (12345)", i32, 12345, i64, 12345LL);
    CHECK_UNARY("I64.EXTEND_I32_S (-12345)", i32, -12345, i64, -12345LL);
    CHECK_UNARY("I64.EXTEND_I32_S (0x80000000)", i32, 0x80000000, i64, (int64_t)0xFFFFFFFF80000000LL); // Smallest signed 32-bit integer
    return failures;
}

int test_i64_extend_i32_u() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM(0x7f, 0x7e, 0xad);

    printf("\n=== Testing I64.EXTEND_I32_U ===\n");
    CHECK_UNARY("I64.EXTEND_I32_U (12345)", i32, 12345, i64, 12345LL);
    CHECK_UNARY("I64.EXTEND_I32_U (0xFFFFFFFF)", i32, 0xFFFFFFFF, i64, 0x00000000FFFFFFFFLL); // Largest unsigned 32-bit integer
    return failures;
}

int test_i64_trunc_f32_s() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM(0x7d, 0x7e, 0xae);

    printf("\n=== Testing I64.TRUNC_F32_S ===\n");
    CHECK_UNARY("I64.TRUNC_F32_S (10.5f)", f32, 10.5f, i64, 10LL);
    CHECK_UNARY("I64.TRUNC_F32_S (-10.5f)", f32, -10.5f, i64, -10LL);
    CHECK_UNARY_TRAP("I64.TRUNC_F32_S (NaN)", f32, NAN, i64);
    CHECK_UNARY_TRAP("I64.TRUNC_F32_S (Infinity)", f32, INFINITY, i64);
    CHECK_UNARY_TRAP("I64.TRUNC_F32_S (too large)", f32, 9223372036854775808.0f /*INT64_MAX + 1*/, i64);
    return failures;
}

int test_i64_trunc_f32_u() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM(0x7d, 0x7e, 0xaf);

    printf("\n=== Testing I64.TRUNC_F32_U ===\n");
    CHECK_UNARY("I64.TRUNC_F32_U (10.5f)", f32, 10.5f, i64, 10LL);
    CHECK_UNARY_TRAP("I64.TRUNC_F32_U (-10.5f)", f32, -10.5f, i64);
    CHECK_UNARY_TRAP("I64.TRUNC_F32_U (too large)", f32, 18446744073709551616.0f /*UINT64_MAX + 1*/, i64);
    return failures;
}

int test_i64_trunc_f64_s() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM(0x7c, 0x7e, 0xb0);

    printf("\n=== Testing I64.TRUNC_F64_S ===\n");
    CHECK_UNARY("I64.TRUNC_F64_S (10.5)", f64, 10.5, i64, 10LL);
    CHECK_UNARY("I64.TRUNC_F64_S (-10.5)", f64, -10.5, i64, -10LL);
    CHECK_UNARY_TRAP("I64.TRUNC_F64_S (NaN)", f64, NAN, i64);
    CHECK_UNARY_TRAP("I64.TRUNC_F64_S (Infinity)", f64, INFINITY, i64);
    CHECK_UNARY_TRAP("I64.TRUNC_F64_S (too large)", f64, 9223372036854775808.0 /*INT64_MAX + 1*/, i64);
    return failures;
}

int test_i64_trunc_f64_u() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM(0x7c, 0x7e, 0xb1);

    printf("\n=== Testing I64.TRUNC_F64_U ===\n");
    CHECK_UNARY("I64.TRUNC_F64_U (10.5)", f64, 10.5, i64, 10LL);
    CHECK_UNARY_TRAP("I64.TRUNC_F64_U (-10.5)", f64, -10.5, i64);
    CHECK_UNARY_TRAP("I64.TRUNC_F64_U (too large)", f64, 18446744073709551616.0 /*UINT64_MAX + 1*/, i64);
    return failures;
}

int test_f32_convert_i32_s() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM(0x7f, 0x7d, 0xb2);

    printf("\n=== Testing F32.CONVERT_I32_S ===\n");
    CHECK_UNARY("F32.CONVERT_I32_S (12345)", i32, 12345, f32, 12345.0f);
    CHECK_UNARY("F32.CONVERT_I32_S (-12345)", i32, -12345, f32, -12345.0f);
    return failures;
}

int test_f32_convert_i32_u() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM(0x7f, 0x7d, 0xb3);

    printf("\n=== Testing F32.CONVERT_I32_U ===\n");
    CHECK_UNARY("F32.CONVERT_I32_U (12345)", i32, 12345, f32, 12345.0f);
    CHECK_UNARY("F32.CONVERT_I32_U (0xFFFFFFFF)", i32, 0xFFFFFFFF /*UINT32_MAX*/, f32, 4294967295.0f);
    return failures;
}

int test_f32_convert_i64_s() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM(0x7e, 0x7d, 0xb4);

    printf("\n=== Testing F32.CONVERT_I64_S ===\n");
    CHECK_UNARY("F32.CONVERT_I64_S (123456789012345)", i64, 123456789012345LL, f32, 123456788103168.0f); // Precision loss expected
    CHECK_UNARY("F32.CONVERT_I64_S (-123456789012345)", i64, -123456789012345LL, f32, -123456788103168.0f); // Precision loss expected
    return failures;
}

int test_f32_convert_i64_u() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM(0x7e, 0x7d, 0xb5);

    printf("\n=== Testing F32.CONVERT_I64_U ===\n");
    CHECK_UNARY("F32.CONVERT_I64_U (123456789012345)", i64, 123456789012345ULL, f32, 123456788103168.0f); // Precision loss expected
    CHECK_UNARY("F32.CONVERT_I64_U (0xFFFFFFFFFFFFFFFF)", i64, 0xFFFFFFFFFFFFFFFFULL /*UINT64_MAX*/, f32, 1.8446744073709552e+19f); // Precision loss expected
    return failures;
}

int test_f32_demote_f64() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM(0x7c, 0x7d, 0xb6);

    printf("\n=== Testing F32.DEMOTE_F64 ===\n");
    CHECK_UNARY("F32.DEMOTE_F64 (123.456)", f64, 123.456, f32, 123.456f);
    CHECK_UNARY("F32.DEMOTE_F64 (large double to float)", f64, 1.2345678901234567e+300, f32, INFINITY); // A large double that will become infinity in float
    return failures;
}

int test_f64_convert_i32_s() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM(0x7f, 0x7c, 0xb7);

    printf("\n=== Testing F64.CONVERT_I32_S ===\n");
    CHECK_UNARY("F64.CONVERT_I32_S (12345)", i32, 12345, f64, 12345.0);
    CHECK_UNARY("F64.CONVERT_I32_S (-12345)", i32, -12345, f64, -12345.0);
    return failures;
}

int test_f64_convert_i32_u() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM(0x7f, 0x7c, 0xb8);

    printf("\n=== Testing F64.CONVERT_I32_U ===\n");
    CHECK_UNARY("F64.CONVERT_I32_U (12345)", i32, 12345, f64, 12345.0);
    CHECK_UNARY("F64.CONVERT_I32_U (0xFFFFFFFF)", i32, 0xFFFFFFFF /*UINT32_MAX*/, f64, 4294967295.0);
    return failures;
}

int test_f64_convert_i64_s() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM(0x7e, 0x7c, 0xb9);

    printf("\n=== Testing F64.CONVERT_I64_S ===\n");
    CHECK_UNARY("F64.CONVERT_I64_S (1234567890123456789)", i64, 1234567890123456789LL, f64, 1234567890123456768.0); // Precision loss expected
    CHECK_UNARY("F64.CONVERT_I64_S (-1234567890123456789)", i64, -1234567890123456789LL, f64, -1234567890123456768.0); // Precision loss expected
    return failures;
}

int test_f64_convert_i64_u() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM(0x7e, 0x7c, 0xba);

    printf("\n=== Testing F64.CONVERT_I64_U ===\n");
    CHECK_UNARY("F64.CONVERT_I64_U (1234567890123456789)", i64, 1234567890123456789ULL, f64, 1234567890123456768.0); // Precision loss expected
    CHECK_UNARY("F64.CONVERT_I64_U (0xFFFFFFFFFFFFFFFF)", i64, 0xFFFFFFFFFFFFFFFFULL /*UINT64_MAX*/, f64, 1.8446744073709552e+19); // Precision loss expected
    return failures;
}

int test_f64_promote_f32() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM(0x7d, 0x7c, 0xbb);

    printf("\n=== Testing F64.PROMOTE_F32 ===\n");
    CHECK_UNARY("F64.PROMOTE_F32 (123.456f)", f32, 123.456f, f64, (double)123.456f);
    return failures;
}

int test_i32_reinterpret_f32() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM(0x7d, 0x7f, 0xbc);

    printf("\n=== Testing I32.REINTERPRET_F32 ===\n");
    CHECK_UNARY("I32.REINTERPRET_F32 (1.0f)", f32, 1.0f, i32, 0x3F800000);
    return failures;
}

int test_i64_reinterpret_f64() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM(0x7c, 0x7e, 0xbd);

    printf("\n=== Testing I64.REINTERPRET_F64 ===\n");
    CHECK_UNARY("I64.REINTERPRET_F64 (1.0)", f64, 1.0, i64, 0x3FF0000000000000ULL);
    return failures;
}

int test_f32_reinterpret_i32() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM(0x7f, 0x7d, 0xbe);

    printf("\n=== Testing F32.REINTERPRET_I32 ===\n");
    CHECK_UNARY("F32.REINTERPRET_I32 (0x3F800000)", i32, 0x3F800000, f32, 1.0f);
    return failures;
}

int test_f64_reinterpret_i64() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM(0x7e, 0x7c, 0xbf);

    printf("\n=== Testing F64.REINTERPRET_I64 ===\n");
    CHECK_UNARY("F64.REINTERPRET_I64 (0x3FF0000000000000)", i64, 0x3FF0000000000000ULL, f64, 1.0);
    return failures;
}

int test_i32_trunc_sat_f32_s() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM_FC(0x7d, 0x7f, 0x00);

    printf("\n=== Testing I32.TRUNC_SAT_F32_S ===\n");
    CHECK_UNARY("I32.TRUNC_SAT_F32_S (10.5f)", f32, 10.5f, i32, 10);
    CHECK_UNARY("I32.TRUNC_SAT_F32_S (-10.5f)", f32, -10.5f, i32, -10);
    CHECK_UNARY("I32.TRUNC_SAT_F32_S (NaN)", f32, NAN, i32, 0); // NaN should result in 0
    CHECK_UNARY("I32.TRUNC_SAT_F32_S (Infinity)", f32, INFINITY, i32, INT32_MAX); // Positive Infinity should saturate to INT32_MAX
    CHECK_UNARY("I32.TRUNC_SAT_F32_S (-Infinity)", f32, -INFINITY, i32, INT32_MIN); // Negative Infinity should saturate to INT32_MIN
    CHECK_UNARY("I32.TRUNC_SAT_F32_S (too large)", f32, 2147483648.0f /*INT32_MAX + 1*/, i32, INT32_MAX); // Value greater than INT32_MAX should saturate to INT32_MAX
    CHECK_UNARY("I32.TRUNC_SAT_F32_S (too small)", f32, -2147483649.0f /*INT32_MIN - 1*/, i32, INT32_MIN); // Value less than INT32_MIN should saturate to INT32_MIN
    return failures;
}

int test_i32_trunc_sat_f32_u() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM_FC(0x7d, 0x7f, 0x01);

    printf("\n=== Testing I32.TRUNC_SAT_F32_U ===\n");
    CHECK_UNARY("I32.TRUNC_SAT_F32_U (10.5f)", f32, 10.5f, i32, 10);
    CHECK_UNARY("I32.TRUNC_SAT_F32_U (NaN)", f32, NAN, i32, 0); // NaN should result in 0
    CHECK_UNARY("I32.TRUNC_SAT_F32_U (Infinity)", f32, INFINITY, i32, (int32_t)UINT32_MAX); // Positive Infinity should saturate to UINT32_MAX
    CHECK_UNARY("I32.TRUNC_SAT_F32_U (-Infinity)", f32, -INFINITY, i32, 0); // Negative Infinity should saturate to 0
    CHECK_UNARY("I32.TRUNC_SAT_F32_U (too large)", f32, 4294967296.0f /*UINT32_MAX + 1*/, i32, (int32_t)UINT32_MAX); // Value greater than UINT32_MAX should saturate to UINT32_MAX
    CHECK_UNARY("I32.TRUNC_SAT_F32_U (negative)", f32, -0.5f, i32, 0); // Value less than 0 should saturate to 0
    return failures;
}

int test_i32_trunc_sat_f64_s() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM_FC(0x7c, 0x7f, 0x02);

    printf("\n=== Testing I32.TRUNC_SAT_F64_S ===\n");
    CHECK_UNARY("I32.TRUNC_SAT_F64_S (10.5)", f64, 10.5, i32, 10);
    CHECK_UNARY("I32.TRUNC_SAT_F64_S (-10.5)", f64, -10.5, i32, -10);
    CHECK_UNARY("I32.TRUNC_SAT_F64_S (NaN)", f64, NAN, i32, 0); // NaN should result in 0
    CHECK_UNARY("I32.TRUNC_SAT_F64_S (Infinity)", f64, INFINITY, i32, INT32_MAX); // Positive Infinity should saturate to INT32_MAX
    CHECK_UNARY("I32.TRUNC_SAT_F64_S (-Infinity)", f64, -INFINITY, i32, INT32_MIN); // Negative Infinity should saturate to INT32_MIN
    CHECK_UNARY("I32.TRUNC_SAT_F64_S (too large)", f64, 2147483648.0 /*INT32_MAX + 1*/, i32, INT32_MAX); // Value greater than INT32_MAX should saturate to INT32_MAX
    CHECK_UNARY("I32.TRUNC_SAT_F64_S (too small)", f64, -2147483649.0 /*INT32_MIN - 1*/, i32, INT32_MIN); // Value less than INT32_MIN should saturate to INT32_MIN
    return failures;
}

int test_i32_trunc_sat_f64_u() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM_FC(0x7c, 0x7f, 0x03);

    printf("\n=== Testing I32.TRUNC_SAT_F64_U ===\n");
    CHECK_UNARY("I32.TRUNC_SAT_F64_U (10.5)", f64, 10.5, i32, 10);
    CHECK_UNARY("I32.TRUNC_SAT_F64_U (NaN)", f64, NAN, i32, 0); // NaN should result in 0
    CHECK_UNARY("I32.TRUNC_SAT_F64_U (Infinity)", f64, INFINITY, i32, (int32_t)UINT32_MAX); // Positive Infinity should saturate to UINT32_MAX
    CHECK_UNARY("I32.TRUNC_SAT_F64_U (-Infinity)", f64, -INFINITY, i32, 0); // Negative Infinity should saturate to 0
    CHECK_UNARY("I32.TRUNC_SAT_F64_U (too large)", f64, 4294967296.0 /*UINT32_MAX + 1*/, i32, (int32_t)UINT32_MAX); // Value greater than UINT32_MAX should saturate to UINT32_MAX
    CHECK_UNARY("I32.TRUNC_SAT_F64_U (negative)", f64, -0.5, i32, 0); // Value less than 0 should saturate to 0
    return failures;
}

int test_i64_trunc_sat_f32_s() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM_FC(0x7d, 0x7e, 0x04);

    printf("\n=== Testing I64.TRUNC_SAT_F32_S ===\n");
    CHECK_UNARY("I64.TRUNC_SAT_F32_S (10.5f)", f32, 10.5f, i64, 10LL);
    CHECK_UNARY("I64.TRUNC_SAT_F32_S (-10.5f)", f32, -10.5f, i64, -10LL);
    CHECK_UNARY("I64.TRUNC_SAT_F32_S (NaN)", f32, NAN, i64, 0LL); // NaN should result in 0
    CHECK_UNARY("I64.TRUNC_SAT_F32_S (Infinity)", f32, INFINITY, i64, INT64_MAX); // Positive Infinity should saturate to INT64_MAX
    CHECK_UNARY("I64.TRUNC_SAT_F32_S (-Infinity)", f32, -INFINITY, i64, INT64_MIN); // Negative Infinity should saturate to INT64_MIN
    CHECK_UNARY("I64.TRUNC_SAT_F32_S (too large)", f32, (float)INT64_MAX + 100.0f, i64, INT64_MAX); // Value greater than INT64_MAX should saturate to INT64_MAX
    CHECK_UNARY("I64.TRUNC_SAT_F32_S (too small)", f32, (float)INT64_MIN - 100.0f, i64, INT64_MIN); // Value less than INT64_MIN should saturate to INT64_MIN
    return failures;
}

int test_i64_trunc_sat_f32_u() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM_FC(0x7d, 0x7e, 0x05);

    printf("\n=== Testing I64.TRUNC_SAT_F32_U ===\n");
    CHECK_UNARY("I64.TRUNC_SAT_F32_U (10.5f)", f32, 10.5f, i64, 10ULL);
    CHECK_UNARY("I64.TRUNC_SAT_F32_U (NaN)", f32, NAN, i64, 0ULL); // NaN should result in 0
    CHECK_UNARY("I64.TRUNC_SAT_F32_U (Infinity)", f32, INFINITY, i64, (int64_t)UINT64_MAX); // Positive Infinity should saturate to UINT64_MAX
    CHECK_UNARY("I64.TRUNC_SAT_F32_U (-Infinity)", f32, -INFINITY, i64, 0ULL); // Negative Infinity should saturate to 0
    CHECK_UNARY("I64.TRUNC_SAT_F32_U (too large)", f32, (float)UINT64_MAX + 100.0f, i64, (int64_t)UINT64_MAX); // Value greater than UINT64_MAX should saturate to UINT64_MAX
    CHECK_UNARY("I64.TRUNC_SAT_F32_U (negative)", f32, -0.5f, i64, 0ULL); // Value less than 0 should saturate to 0
    return failures;
}

int test_i64_trunc_sat_f64_s() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM_FC(0x7c, 0x7e, 0x06);

    printf("\n=== Testing I64.TRUNC_SAT_F64_S ===\n");
    CHECK_UNARY("I64.TRUNC_SAT_F64_S (10.5)", f64, 10.5, i64, 10LL);
    CHECK_UNARY("I64.TRUNC_SAT_F64_S (-10.5)", f64, -10.5, i64, -10LL);
    CHECK_UNARY("I64.TRUNC_SAT_F64_S (NaN)", f64, NAN, i64, 0LL); // NaN should result in 0
    CHECK_UNARY("I64.TRUNC_SAT_F64_S (Infinity)", f64, INFINITY, i64, INT64_MAX); // Positive Infinity should saturate to INT64_MAX
    CHECK_UNARY("I64.TRUNC_SAT_F64_S (-Infinity)", f64, -INFINITY, i64, INT64_MIN); // Negative Infinity should saturate to INT64_MIN
    CHECK_UNARY("I64.TRUNC_SAT_F64_S (too large)", f64, (double)INT64_MAX + 100.0, i64, INT64_MAX); // Value greater than INT64_MAX should saturate to INT64_MAX
    CHECK_UNARY("I64.TRUNC_SAT_F64_S (too small)", f64, (double)INT64_MIN - 100.0, i64, INT64_MIN); // Value less than INT64_MIN should saturate to INT64_MIN
    return failures;
}

int test_i64_trunc_sat_f64_u() {
    int failures = 0;
    static const uint8_t test_wasm[] = UNARY_TEST_WASM_FC(0x7c, 0x7e, 0x07);

    printf("\n=== Testing I64.TRUNC_SAT_F64_U ===\n");
    CHECK_UNARY("I64.TRUNC_SAT_F64_U (10.5)", f64, 10.5, i64, 10ULL);
    CHECK_UNARY("I64.TRUNC_SAT_F64_U (NaN)", f64, NAN, i64, 0ULL); // NaN should result in 0
    CHECK_UNARY("I64.TRUNC_SAT_F64_U (Infinity)", f64, INFINITY, i64, (int64_t)UINT64_MAX); // Positive Infinity should saturate to UINT64_MAX
    CHECK_UNARY("I64.TRUNC_SAT_F64_U (-Infinity)", f64, -INFINITY, i64, 0ULL); // Negative Infinity should saturate to 0
    CHECK_UNARY("I64.TRUNC_SAT_F64_U (too large)", f64, (double)UINT64_MAX + 100.0, i64, (int64_t)UINT64_MAX); // Value greater than UINT64_MAX should saturate to UINT64_MAX
    CHECK_UNARY("I64.TRUNC_SAT_F64_U (negative)", f64, -0.5, i64, 0ULL); // Value less than 0 should saturate to 0
    return failures;
}

int main() {
    int total_failures = 0;

    total_failures += test_i32_and();
    total_failures += test_i32_eq();
    total_failures += test_i32_popcnt();
    total_failures += test_i64_clz();
    total_failures += test_i32_rotl();
    total_failures += test_f64_nearest();
    total_failures += test_f32_min();

    total_failures += test_i32_wrap_i64();
    total_failures += test_i32_trunc_f32_s();
    total_failures += test_i32_trunc_f32_u();
    total_failures += test_i32_trunc_f64_s();
    total_failures += test_i32_trunc_f64_u();

    total_failures += test_i64_extend_i32_s();
    total_failures += test_i64_extend_i32_u();
    total_failures += test_i64_trunc_f32_s();
    total_failures += test_i64_trunc_f32_u();
    total_failures += test_i64_trunc_f64_s();
    total_failures += test_i64_trunc_f64_u();

    total_failures += test_f32_convert_i32_s();
    total_failures += test_f32_convert_i32_u();
    total_failures += test_f32_convert_i64_s();
    total_failures += test_f32_convert_i64_u();

    total_failures += test_f32_demote_f64();

    total_failures += test_f64_convert_i32_s();
    total_failures += test_f64_convert_i32_u();
    total_failures += test_f64_convert_i64_s();
    total_failures += test_f64_convert_i64_u();

    total_failures += test_f64_promote_f32();

    total_failures += test_i32_reinterpret_f32();
    total_failures += test_i64_reinterpret_f64();
    total_failures += test_f32_reinterpret_i32();
    total_failures += test_f64_reinterpret_i64();

    total_failures += test_i32_trunc_sat_f32_s();
    total_failures += test_i32_trunc_sat_f32_u();
    total_failures += test_i32_trunc_sat_f64_s();
    total_failures += test_i32_trunc_sat_f64_u();

    total_failures += test_i64_trunc_sat_f32_s();
    total_failures += test_i64_trunc_sat_f32_u();
    total_failures += test_i64_trunc_sat_f64_s();
    total_failures += test_i64_trunc_sat_f64_u();

    if (total_failures > 0) {
        printf("\nSUMMARY: %d test(s) FAILED!\n", total_failures);
        return 1;
    } else {
        printf("\nSUMMARY: All tests PASSED!\n");
        return 0;
    }
}
