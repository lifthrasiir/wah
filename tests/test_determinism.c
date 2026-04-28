#include "../wah.h"
// Include wah_testutils.c for wah_parse_module_from_spec
#include "common.h"
#include <stdio.h>
#include <string.h>
#include <math.h> // For isnan, though we'll use bit patterns for comparison
#include <stdlib.h> // For malloc, free
#include <inttypes.h>

// Helper to convert float to uint32_t bit pattern
static uint32_t float_to_bits(float f) {
    union { uint32_t i; float f; } u = { .f = f };
    return u.i;
}

// Helper to convert double to uint64_t bit pattern
static uint64_t double_to_bits(double d) {
    union { uint64_t i; double d; } u = { .d = d };
    return u.i;
}

// WebAssembly canonical NaN bit patterns (from wah.h)
#define TEST_WASM_F32_CANONICAL_NAN_BITS 0x7fc00000U
#define TEST_WASM_F64_CANONICAL_NAN_BITS 0x7ff8000000000000ULL

static const uint64_t NON_CANONICAL_F64_NAN_BITS_1 = 0x7ff0000000000001ULL;
static const uint64_t NON_CANONICAL_F64_NAN_BITS_2 = 0x7ff0000000000002ULL;

// A non-canonical NaN for testing (e.g., quiet NaN with some payload bits)
// Sign: 0, Exponent: all 1s, Mantissa: 010...0 (bit 22 is 0, bit 21 is 1)
// Canonical would be 0x7fc00000 (bit 22 is 1, rest 0)
#define NON_CANONICAL_F32_NAN_BITS 0x7fa00000U
static const union { uint32_t i; float f; } non_canonical_f32_nan_union = { .i = NON_CANONICAL_F32_NAN_BITS };
#define NON_CANONICAL_F32_NAN non_canonical_f32_nan_union.f

// Helper function to create the shared test module
static wah_error_t create_test_module(wah_module_t *module) {
    return wah_parse_module_from_spec(module, "wasm \
        types {[ \
            fn [f32] [f32], \
            fn [] [f32], \
            fn [i32] [i32], \
            fn [i64, i64] [i64], \
            fn [f32, f32] [f32], \
            fn [f32] [f32], \
            fn [f64, f64] [f64], \
            fn [f32] [f64], \
            fn [f64] [f32], \
        ]} \
        funcs {[ 0, 1, 2, 3, 4, 5, 6, 7, 8 ]} \
        memories {[ limits.i32/1 1 ]} \
        exports {[ \
            {'mem'} mem# 0, \
            {'test_store'} fn# 0, \
            {'test_const'} fn# 1, \
            {'test_f32_reinterpret_nan'} fn# 2, \
            {'test_f64_add_nan'} fn# 3, \
            {'test_f32_add_nan'} fn# 4, \
            {'test_f32_sqrt_nan'} fn# 5, \
            {'test_f64_min_nan'} fn# 6, \
            {'test_f64_promote_f32_nan_canonicalization'} fn# 7, \
            {'test_f32_demote_f64_nan_canonicalization'} fn# 8, \
        ]} \
        code {[ \
            {[] i32.const 0 local.get 0 f32.store align=4 offset=0 i32.const 0 f32.load align=4 offset=0 end}, \
            {[] f32.const %i32 end}, \
            {[] local.get 0 f32.reinterpret_i32 i32.reinterpret_f32 end}, \
            {[] local.get 0 f64.reinterpret_i64 local.get 1 f64.reinterpret_i64 f64.add i64.reinterpret_f64 end}, \
            {[] local.get 0 local.get 1 f32.add end}, \
            {[] local.get 0 f32.sqrt end}, \
            {[] local.get 0 local.get 1 f64.min end}, \
            {[] local.get 0 f64.promote_f32 end}, \
            {[] local.get 0 f32.demote_f64 end}, \
        ]}",
        NON_CANONICAL_F32_NAN_BITS
    );
}

static void test_f32_store_canonicalization() {
    printf("Testing f32.store canonicalization...\n");
    wah_module_t module;
    wah_exec_context_t exec_ctx;
    wah_value_t result;

    assert_ok(create_test_module(&module));
    assert_ok(wah_exec_context_create(&exec_ctx, &module));

    wah_value_t param;
    param.f32 = NON_CANONICAL_F32_NAN;
    assert_ok(wah_call(&exec_ctx, 0, &param, 1, &result)); // Call "test_store" (func_idx 0)

    uint32_t result_bits = float_to_bits(result.f32);
    if (result_bits != NON_CANONICAL_F32_NAN_BITS) {
        printf("f32.store test FAILED: Stored non-canonical NaN was NOT preserved.\n");
        printf("  Expected non-canonical bits: 0x%08X\n", NON_CANONICAL_F32_NAN_BITS);
        printf("  Actual result bits:        0x%08X\n", result_bits);
        exit(1);
    }

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
}

static void test_f32_const_canonicalization() {
    printf("Testing f32.const canonicalization...\n");
    wah_module_t module;
    wah_exec_context_t exec_ctx;
    wah_value_t result;

    assert_ok(create_test_module(&module));
    assert_ok(wah_exec_context_create(&exec_ctx, &module));

    assert_ok(wah_call(&exec_ctx, 1, NULL, 0, &result)); // Call "test_const" (func_idx 1)

    uint32_t result_bits = float_to_bits(result.f32);
    if (result_bits != NON_CANONICAL_F32_NAN_BITS) {
        printf("f32.const test FAILED: Constant non-canonical NaN was NOT preserved.\n");
        printf("  Expected non-canonical bits: 0x%08X\n", NON_CANONICAL_F32_NAN_BITS);
        printf("  Actual result bits:        0x%08X\n", result_bits);
        exit(1);
    }

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
}

static void test_f32_reinterpret_nan_canonicalization() {
    printf("Testing f32.reinterpret_i32 and i32.reinterpret_f32 canonicalization...\n");
    wah_module_t module;
    wah_exec_context_t exec_ctx;
    wah_value_t result;

    assert_ok(create_test_module(&module));
    assert_ok(wah_exec_context_create(&exec_ctx, &module));

    wah_value_t param;
    param.i32 = NON_CANONICAL_F32_NAN_BITS; // Pass non-canonical NaN bit pattern as i32
    assert_ok(wah_call(&exec_ctx, 2, &param, 1, &result)); // Call "test_f32_reinterpret_nan" (func_idx 2)

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);

    uint32_t result_i32_bits = (uint32_t)result.i32;
    if (result_i32_bits != NON_CANONICAL_F32_NAN_BITS) {
        printf("f32.reinterpret_i32/i32.reinterpret_f32 test FAILED: Reinterpreted non-canonical NaN was NOT preserved.\n");
        printf("  Expected non-canonical bits: 0x%08X\n", NON_CANONICAL_F32_NAN_BITS);
        printf("  Actual result bits:        0x%08X\n", result_i32_bits);
        exit(1);
    }

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
}

static void test_f64_add_nan_canonicalization() {
    printf("Testing f64.add canonicalization with non-canonical NaNs...\n");
    wah_module_t module;
    wah_exec_context_t exec_ctx;
    wah_value_t result;

    assert_ok(create_test_module(&module));
    assert_ok(wah_exec_context_create(&exec_ctx, &module));

    wah_value_t params_f64[2];
    params_f64[0].i64 = (int64_t)NON_CANONICAL_F64_NAN_BITS_1;
    params_f64[1].i64 = (int64_t)NON_CANONICAL_F64_NAN_BITS_2;
    assert_ok(wah_call(&exec_ctx, 3, params_f64, 2, &result)); // Call "test_f64_add_nan" (func_idx 3)

    uint64_t result_i64_bits = (uint64_t)result.i64;
    if (result_i64_bits != TEST_WASM_F64_CANONICAL_NAN_BITS) {
        printf("f64.add test FAILED: Adding non-canonical NaNs did NOT result in canonical NaN.\n");
        printf("  Expected canonical bits: 0x%llX\n", TEST_WASM_F64_CANONICAL_NAN_BITS);
        printf("  Actual result bits:    0x%" PRIx64 "\n", result_i64_bits);
        exit(1);
    }

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
}

static void test_f32_add_nan_canonicalization() {
    printf("Testing f32.add canonicalization with non-canonical NaNs...\n");
    wah_module_t module;
    wah_exec_context_t exec_ctx;
    wah_value_t result;

    assert_ok(create_test_module(&module));
    assert_ok(wah_exec_context_create(&exec_ctx, &module));

    wah_value_t params_f32[2];
    params_f32[0].f32 = NON_CANONICAL_F32_NAN;
    params_f32[1].f32 = NON_CANONICAL_F32_NAN;
    assert_ok(wah_call(&exec_ctx, 4, params_f32, 2, &result)); // Call "test_f32_add_nan" (func_idx 4)

    uint32_t result_bits = float_to_bits(result.f32);
    if (result_bits != TEST_WASM_F32_CANONICAL_NAN_BITS) {
        printf("f32.add test FAILED: Adding non-canonical NaNs did NOT result in canonical NaN.\n");
        printf("  Expected canonical bits: 0x%08X\n", TEST_WASM_F32_CANONICAL_NAN_BITS);
        printf("  Actual result bits:    0x%08X\n", result_bits);
        exit(1);
    }

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
}

static void test_f32_sqrt_nan_canonicalization() {
    printf("Testing f32.sqrt canonicalization with non-canonical NaN...\n");
    wah_module_t module;
    wah_exec_context_t exec_ctx;
    wah_value_t result;

    assert_ok(create_test_module(&module));
    assert_ok(wah_exec_context_create(&exec_ctx, &module));

    wah_value_t param;
    param.f32 = NON_CANONICAL_F32_NAN;
    assert_ok(wah_call(&exec_ctx, 5, &param, 1, &result)); // Call "test_f32_sqrt_nan" (func_idx 5)

    uint32_t result_bits = float_to_bits(result.f32);
    if (result_bits != TEST_WASM_F32_CANONICAL_NAN_BITS) {
        printf("f32.sqrt test FAILED: Sqrt of non-canonical NaN did NOT result in canonical NaN.\n");
        printf("  Expected canonical bits: 0x%08X\n", TEST_WASM_F32_CANONICAL_NAN_BITS);
        printf("  Actual result bits:    0x%08X\n", result_bits);
        exit(1);
    }

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
}

static void test_f64_min_nan_canonicalization() {
    printf("Testing f64.min canonicalization with non-canonical NaNs...\n");
    wah_module_t module;
    wah_exec_context_t exec_ctx;

    assert_ok(create_test_module(&module));
    assert_ok(wah_exec_context_create(&exec_ctx, &module));

    wah_value_t params_f64_min[2];
    params_f64_min[0].i64 = (int64_t)NON_CANONICAL_F64_NAN_BITS_1;
    params_f64_min[1].i64 = (int64_t)NON_CANONICAL_F64_NAN_BITS_2;
    wah_value_t result;
    assert_ok(wah_call(&exec_ctx, 6, params_f64_min, 2, &result)); // Call "test_f64_min_nan" (func_idx 6)

    union { uint64_t i; double d; } u = { .d = result.f64 };
    uint64_t result_i64_bits = u.i;
    if (result_i64_bits != TEST_WASM_F64_CANONICAL_NAN_BITS) {
        printf("f64.min test FAILED: Min of non-canonical NaNs did NOT result in canonical NaN.\n");
        printf("  Expected canonical bits: 0x%llX\n", TEST_WASM_F64_CANONICAL_NAN_BITS);
        printf("  Actual result bits:    0x%" PRIx64 "\n", result_i64_bits);
        exit(1);
    }

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
}

// Test function for f64.promote_f32 with NaN canonicalization
static void test_f64_promote_f32_nan_canonicalization() {
    printf("Running test_f64_promote_f32_nan_canonicalization...\n");

    wah_module_t module;
    wah_exec_context_t exec_ctx;

    assert_ok(create_test_module(&module));
    assert_ok(wah_exec_context_create(&exec_ctx, &module));

    wah_value_t param;
    param.i32 = (int32_t)0x7f800001; // Signaling NaN with payload 1

    wah_value_t result;
    assert_ok(wah_call(&exec_ctx, 7, &param, 1, &result)); // Call func_idx 7

    // Check if the result is the canonical f64 NaN
    if (double_to_bits(result.f64) != TEST_WASM_F64_CANONICAL_NAN_BITS) {
        printf("  FAIL: f64.promote_f32 did not produce canonical NaN. Expected 0x%llx, Got 0x%" PRIx64 "\n",
               TEST_WASM_F64_CANONICAL_NAN_BITS, double_to_bits(result.f64));
        exit(1);
    }

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
}

// Test function for f32.demote_f64 with NaN canonicalization
static void test_f32_demote_f64_nan_canonicalization() {
    printf("Running test_f32_demote_f64_nan_canonicalization...\n");

    wah_module_t module;
    wah_exec_context_t exec_ctx;

    assert_ok(create_test_module(&module));
    assert_ok(wah_exec_context_create(&exec_ctx, &module));

    wah_value_t param;
    param.i64 = (int64_t)0x7ff0000000000001ULL; // Signaling NaN with payload 1

    wah_value_t result;
    assert_ok(wah_call(&exec_ctx, 8, &param, 1, &result)); // Call func_idx 8

    // Check if the result is the canonical f32 NaN
    if (float_to_bits(result.f32) != TEST_WASM_F32_CANONICAL_NAN_BITS) {
        printf("  FAIL: f32.demote_f64 did not produce canonical NaN. Expected 0x%x, Got 0x%x\n",
               TEST_WASM_F32_CANONICAL_NAN_BITS, float_to_bits(result.f32));
        exit(1);
    }

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
}

int main() {
    test_f32_store_canonicalization();
    test_f32_const_canonicalization();
    test_f32_reinterpret_nan_canonicalization();
    test_f64_add_nan_canonicalization();
    test_f32_add_nan_canonicalization();
    test_f32_sqrt_nan_canonicalization();
    test_f64_min_nan_canonicalization();

    // NEW TESTS FOR NAN CANONICALIZATION
    test_f64_promote_f32_nan_canonicalization();
    test_f32_demote_f64_nan_canonicalization();

    printf("\nAll NaN canonicalization tests passed!\n");

    return 0;
}
