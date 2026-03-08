#define WAH_IMPLEMENTATION
#include "wah.h"
// Include wah_testutils.c for wah_parse_module_from_spec
#include "wah_testutils.c"
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
static const uint32_t NON_CANONICAL_F32_NAN_BITS = 0x7fa00000U; // Example: qNaN with payload
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

static wah_error_t test_f32_store_canonicalization() {
    printf("Testing f32.store canonicalization...\n");
    wah_module_t module;
    wah_exec_context_t exec_ctx;
    wah_error_t err;
    wah_value_t result;

    err = create_test_module(&module);
    if (err != WAH_OK) {
        fprintf(stderr, "Error parsing module: %s\n", wah_strerror(err));
        return err;
    }

    err = wah_exec_context_create(&exec_ctx, &module);
    if (err != WAH_OK) {
        fprintf(stderr, "Error creating execution context: %s\n", wah_strerror(err));
        wah_free_module(&module);
        return err;
    }

    wah_value_t param;
    param.f32 = NON_CANONICAL_F32_NAN;
    err = wah_call(&exec_ctx, &module, 0, &param, 1, &result); // Call "test_store" (func_idx 0)
    if (err != WAH_OK) {
        fprintf(stderr, "Error calling test_store: %s\n", wah_strerror(err));
        wah_exec_context_destroy(&exec_ctx);
        wah_free_module(&module);
        return err;
    }

    uint32_t result_bits = float_to_bits(result.f32);
    if (result_bits == NON_CANONICAL_F32_NAN_BITS) {
        printf("f32.store test PASSED: Stored non-canonical NaN was preserved.\n");
    } else {
        printf("f32.store test FAILED: Stored non-canonical NaN was NOT preserved.\n");
        printf("  Expected non-canonical bits: 0x%08X\n", NON_CANONICAL_F32_NAN_BITS);
        printf("  Actual result bits:        0x%08X\n", result_bits);
        err = WAH_ERROR_VALIDATION_FAILED;
    }

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
    return err;
}

static wah_error_t test_f32_const_canonicalization() {
    printf("\nTesting f32.const canonicalization...\n");
    wah_module_t module;
    wah_exec_context_t exec_ctx;
    wah_error_t err;
    wah_value_t result;

    err = create_test_module(&module);
    if (err != WAH_OK) {
        fprintf(stderr, "Error re-parsing module for test_const: %s\n", wah_strerror(err));
        return err;
    }

    err = wah_exec_context_create(&exec_ctx, &module);
    if (err != WAH_OK) {
        fprintf(stderr, "Error creating execution context for test_const: %s\n", wah_strerror(err));
        wah_free_module(&module);
        return err;
    }

    err = wah_call(&exec_ctx, &module, 1, NULL, 0, &result); // Call "test_const" (func_idx 1)
    if (err != WAH_OK) {
        fprintf(stderr, "Error calling test_const: %s\n", wah_strerror(err));
        wah_exec_context_destroy(&exec_ctx);
        wah_free_module(&module);
        return err;
    }

    uint32_t result_bits = float_to_bits(result.f32);
    if (result_bits == NON_CANONICAL_F32_NAN_BITS) {
        printf("f32.const test PASSED: Constant non-canonical NaN was preserved.\n");
    } else {
        printf("f32.const test FAILED: Constant non-canonical NaN was NOT preserved.\n");
        printf("  Expected non-canonical bits: 0x%08X\n", NON_CANONICAL_F32_NAN_BITS);
        printf("  Actual result bits:        0x%08X\n", result_bits);
        err = WAH_ERROR_VALIDATION_FAILED;
    }

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
    return err;
}

static wah_error_t test_f32_reinterpret_nan_canonicalization() {
    printf("\nTesting f32.reinterpret_i32 and i32.reinterpret_f32 canonicalization...\n");
    wah_module_t module;
    wah_exec_context_t exec_ctx;
    wah_error_t err;
    wah_value_t result;

    err = create_test_module(&module);
    if (err != WAH_OK) {
        fprintf(stderr, "Error re-parsing module for f32_reinterpret_nan: %s\n", wah_strerror(err));
        return err;
    }

    err = wah_exec_context_create(&exec_ctx, &module);
    if (err != WAH_OK) {
        fprintf(stderr, "Error creating execution context for f32_reinterpret_nan: %s\n", wah_strerror(err));
        wah_free_module(&module);
        return err;
    }

    wah_value_t param;
    param.i32 = NON_CANONICAL_F32_NAN_BITS; // Pass non-canonical NaN bit pattern as i32
    err = wah_call(&exec_ctx, &module, 2, &param, 1, &result); // Call "test_f32_reinterpret_nan" (func_idx 2)
    if (err != WAH_OK) {
        fprintf(stderr, "Error calling test_f32_reinterpret_nan: %s\n", wah_strerror(err));
        wah_exec_context_destroy(&exec_ctx);
        wah_free_module(&module);
        return err;
    }

    uint32_t result_i32_bits = (uint32_t)result.i32;
    if (result_i32_bits == NON_CANONICAL_F32_NAN_BITS) {
        printf("f32.reinterpret_i32/i32.reinterpret_f32 test PASSED: Reinterpreted non-canonical NaN was preserved with the absence of any interleaving operations.\n");
    } else {
        printf("f32.reinterpret_i32/i32.reinterpret_f32 test FAILED: Reinterpreted non-canonical NaN was NOT preserved.\n");
        printf("  Expected non-canonical bits: 0x%08X\n", NON_CANONICAL_F32_NAN_BITS);
        printf("  Actual result bits:        0x%08X\n", result_i32_bits);
        err = WAH_ERROR_VALIDATION_FAILED;
    }

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
    return err;
}

static wah_error_t test_f64_add_nan_canonicalization() {
    printf("\nTesting f64.add canonicalization with non-canonical NaNs...\n");
    wah_module_t module;
    wah_exec_context_t exec_ctx;
    wah_error_t err;
    wah_value_t result;

    err = create_test_module(&module);
    if (err != WAH_OK) {
        fprintf(stderr, "Error re-parsing module for f64_add_nan: %s\n", wah_strerror(err));
        return err;
    }

    err = wah_exec_context_create(&exec_ctx, &module);
    if (err != WAH_OK) {
        fprintf(stderr, "Error creating execution context for f64_add_nan: %s\n", wah_strerror(err));
        wah_free_module(&module);
        return err;
    }

    wah_value_t params_f64[2];
    params_f64[0].i64 = (int64_t)NON_CANONICAL_F64_NAN_BITS_1;
    params_f64[1].i64 = (int64_t)NON_CANONICAL_F64_NAN_BITS_2;
    err = wah_call(&exec_ctx, &module, 3, params_f64, 2, &result); // Call "test_f64_add_nan" (func_idx 3)
    if (err != WAH_OK) {
        fprintf(stderr, "Error calling test_f64_add_nan: %s\n", wah_strerror(err));
        wah_exec_context_destroy(&exec_ctx);
        wah_free_module(&module);
        return err;
    }

    uint64_t result_i64_bits = (uint64_t)result.i64;
    if (result_i64_bits == TEST_WASM_F64_CANONICAL_NAN_BITS) {
        printf("f64.add test PASSED: Adding non-canonical NaNs resulted in canonical NaN.\n");
    } else {
        printf("f64.add test FAILED: Adding non-canonical NaNs did NOT result in canonical NaN.\n");
        printf("  Expected canonical bits: 0x%llX\n", TEST_WASM_F64_CANONICAL_NAN_BITS);
        printf("  Actual result bits:    0x%" PRIx64 "\n", result_i64_bits);
        err = WAH_ERROR_VALIDATION_FAILED;
    }

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
    return err;
}

static wah_error_t test_f32_add_nan_canonicalization() {
    printf("\nTesting f32.add canonicalization with non-canonical NaNs...\n");
    wah_module_t module;
    wah_exec_context_t exec_ctx;
    wah_error_t err;
    wah_value_t result;

    err = create_test_module(&module);
    if (err != WAH_OK) {
        fprintf(stderr, "Error re-parsing module for f32_add_nan: %s\n", wah_strerror(err));
        return err;
    }

    err = wah_exec_context_create(&exec_ctx, &module);
    if (err != WAH_OK) {
        fprintf(stderr, "Error creating execution context for f32_add_nan: %s\n", wah_strerror(err));
        wah_free_module(&module);
        return err;
    }

    wah_value_t params_f32[2];
    params_f32[0].f32 = NON_CANONICAL_F32_NAN;
    params_f32[1].f32 = NON_CANONICAL_F32_NAN;
    err = wah_call(&exec_ctx, &module, 4, params_f32, 2, &result); // Call "test_f32_add_nan" (func_idx 4)
    if (err != WAH_OK) {
        fprintf(stderr, "Error calling test_f32_add_nan: %s\n", wah_strerror(err));
        wah_exec_context_destroy(&exec_ctx);
        wah_free_module(&module);
        return err;
    }

    uint32_t result_bits = float_to_bits(result.f32);
    if (result_bits == TEST_WASM_F32_CANONICAL_NAN_BITS) {
        printf("f32.add test PASSED: Adding non-canonical NaNs resulted in canonical NaN.\n");
    } else {
        printf("f32.add test FAILED: Adding non-canonical NaNs did NOT result in canonical NaN.\n");
        printf("  Expected canonical bits: 0x%08X\n", TEST_WASM_F32_CANONICAL_NAN_BITS);
        printf("  Actual result bits:    0x%08X\n", result_bits);
        err = WAH_ERROR_VALIDATION_FAILED;
    }

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
    return err;
}

static wah_error_t test_f32_sqrt_nan_canonicalization() {
    printf("\nTesting f32.sqrt canonicalization with non-canonical NaN...\n");
    wah_module_t module;
    wah_exec_context_t exec_ctx;
    wah_error_t err;
    wah_value_t result;

    err = create_test_module(&module);
    if (err != WAH_OK) {
        fprintf(stderr, "Error re-parsing module for f32_sqrt_nan: %s\n", wah_strerror(err));
        return err;
    }

    err = wah_exec_context_create(&exec_ctx, &module);
    if (err != WAH_OK) {
        fprintf(stderr, "Error creating execution context for f32_sqrt_nan: %s\n", wah_strerror(err));
        wah_free_module(&module);
        return err;
    }

    wah_value_t param;
    param.f32 = NON_CANONICAL_F32_NAN;
    err = wah_call(&exec_ctx, &module, 5, &param, 1, &result); // Call "test_f32_sqrt_nan" (func_idx 5)
    if (err != WAH_OK) {
        fprintf(stderr, "Error calling test_f32_sqrt_nan: %s\n", wah_strerror(err));
        wah_exec_context_destroy(&exec_ctx);
        wah_free_module(&module);
        return err;
    }

    uint32_t result_bits = float_to_bits(result.f32);
    if (result_bits == TEST_WASM_F32_CANONICAL_NAN_BITS) {
        printf("f32.sqrt test PASSED: Sqrt of non-canonical NaN resulted in canonical NaN.\n");
    } else {
        printf("f32.sqrt test FAILED: Sqrt of non-canonical NaN did NOT result in canonical NaN.\n");
        printf("  Expected canonical bits: 0x%08X\n", TEST_WASM_F32_CANONICAL_NAN_BITS);
        printf("  Actual result bits:    0x%08X\n", result_bits);
        err = WAH_ERROR_VALIDATION_FAILED;
    }

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
    return err;
}

static wah_error_t test_f64_min_nan_canonicalization() {
    printf("\nTesting f64.min canonicalization with non-canonical NaNs...\n");
    wah_module_t module;
    wah_exec_context_t exec_ctx;
    wah_error_t err;

    err = create_test_module(&module);
    if (err != WAH_OK) {
        fprintf(stderr, "Error re-parsing module for f64_min_nan: %s\n", wah_strerror(err));
        return err;
    }

    err = wah_exec_context_create(&exec_ctx, &module);
    if (err != WAH_OK) {
        fprintf(stderr, "Error creating execution context for f64_min_nan: %s\n", wah_strerror(err));
        wah_free_module(&module);
        return err;
    }

    wah_value_t params_f64_min[2];
    params_f64_min[0].i64 = (int64_t)NON_CANONICAL_F64_NAN_BITS_1;
    params_f64_min[1].i64 = (int64_t)NON_CANONICAL_F64_NAN_BITS_2;
    wah_value_t result;
    err = wah_call(&exec_ctx, &module, 6, params_f64_min, 2, &result); // Call "test_f64_min_nan" (func_idx 6)
    if (err != WAH_OK) {
        fprintf(stderr, "Error calling test_f64_min_nan: %s\n", wah_strerror(err));
        wah_exec_context_destroy(&exec_ctx);
        wah_free_module(&module);
        return err;
    }

    union { uint64_t i; double d; } u = { .d = result.f64 };
    uint64_t result_i64_bits = u.i;
    if (result_i64_bits == TEST_WASM_F64_CANONICAL_NAN_BITS) {
        printf("f64.min test PASSED: Min of non-canonical NaNs resulted in canonical NaN.\n");
    } else {
        printf("f64.min test FAILED: Min of non-canonical NaNs did NOT result in canonical NaN.\n");
        printf("  Expected canonical bits: 0x%llX\n", TEST_WASM_F64_CANONICAL_NAN_BITS);
        printf("  Actual result bits:    0x%" PRIx64 "\n", result_i64_bits);
        err = WAH_ERROR_VALIDATION_FAILED;
    }

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
    return err;
}

// Test function for f64.promote_f32 with NaN canonicalization
static wah_error_t test_f64_promote_f32_nan_canonicalization() {
    printf("Running test_f64_promote_f32_nan_canonicalization...\n");

    wah_module_t module;
    wah_exec_context_t exec_ctx;
    wah_error_t err;

    err = create_test_module(&module);
    if (err != WAH_OK) {
        fprintf(stderr, "Error parsing module: %s\n", wah_strerror(err));
        return err;
    }

    err = wah_exec_context_create(&exec_ctx, &module);
    if (err != WAH_OK) {
        fprintf(stderr, "Error creating execution context: %s\n", wah_strerror(err));
        wah_free_module(&module);
        return err;
    }

    wah_value_t param;
    param.i32 = (int32_t)0x7f800001; // Signaling NaN with payload 1

    wah_value_t result;
    err = wah_call(&exec_ctx, &module, 7, &param, 1, &result); // Call func_idx 7
    if (err != WAH_OK) {
        fprintf(stderr, "Error calling test_f64_promote_f32_nan_canonicalization: %s\n", wah_strerror(err));
        wah_exec_context_destroy(&exec_ctx);
        wah_free_module(&module);
        return err;
    }

    // Check if the result is the canonical f64 NaN
    if (double_to_bits(result.f64) == TEST_WASM_F64_CANONICAL_NAN_BITS) {
        printf("  PASS: f64.promote_f32 produced canonical NaN.\n");
    } else {
        printf("  FAIL: f64.promote_f32 did not produce canonical NaN. Expected 0x%llx, Got 0x%" PRIx64 "\n",
               TEST_WASM_F64_CANONICAL_NAN_BITS, double_to_bits(result.f64));
        err = WAH_ERROR_VALIDATION_FAILED;
    }

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
    return err;
}

// Test function for f32.demote_f64 with NaN canonicalization
static wah_error_t test_f32_demote_f64_nan_canonicalization() {
    printf("Running test_f32_demote_f64_nan_canonicalization...\n");

    wah_module_t module;
    wah_exec_context_t exec_ctx;
    wah_error_t err;

    err = create_test_module(&module);
    if (err != WAH_OK) {
        fprintf(stderr, "Error parsing module: %s\n", wah_strerror(err));
        return err;
    }

    err = wah_exec_context_create(&exec_ctx, &module);
    if (err != WAH_OK) {
        fprintf(stderr, "Error creating execution context: %s\n", wah_strerror(err));
        wah_free_module(&module);
        return err;
    }

    wah_value_t param;
    param.i64 = (int64_t)0x7ff0000000000001ULL; // Signaling NaN with payload 1

    wah_value_t result;
    err = wah_call(&exec_ctx, &module, 8, &param, 1, &result); // Call func_idx 8
    if (err != WAH_OK) {
        fprintf(stderr, "Error calling test_f32_demote_f64_nan_canonicalization: %s\n", wah_strerror(err));
        wah_exec_context_destroy(&exec_ctx);
        wah_free_module(&module);
        return err;
    }

    // Check if the result is the canonical f32 NaN
    if (float_to_bits(result.f32) == TEST_WASM_F32_CANONICAL_NAN_BITS) {
        printf("  PASS: f32.demote_f64 produced canonical NaN.\n");
    } else {
        printf("  FAIL: f32.demote_f64 did not produce canonical NaN. Expected 0x%x, Got 0x%x\n",
               TEST_WASM_F32_CANONICAL_NAN_BITS, float_to_bits(result.f32));
        err = WAH_ERROR_VALIDATION_FAILED;
    }

    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
    return err;
}

int main() {
    wah_error_t err = WAH_OK;

    err = test_f32_store_canonicalization();
    if (err != WAH_OK) return 1;

    err = test_f32_const_canonicalization();
    if (err != WAH_OK) return 1;

    err = test_f32_reinterpret_nan_canonicalization();
    if (err != WAH_OK) return 1;

    err = test_f64_add_nan_canonicalization();
    if (err != WAH_OK) return 1;

    err = test_f32_add_nan_canonicalization();
    if (err != WAH_OK) return 1;

    err = test_f32_sqrt_nan_canonicalization();
    if (err != WAH_OK) return 1;

    err = test_f64_min_nan_canonicalization();
    if (err != WAH_OK) return 1;

    // NEW TESTS FOR NAN CANONICALIZATION
    err = test_f64_promote_f32_nan_canonicalization();
    if (err != WAH_OK) {
        fprintf(stderr, "test_f64_promote_f32_nan_canonicalization FAILED: %s\n", wah_strerror(err));
        return 1;
    }

    err = test_f32_demote_f64_nan_canonicalization();
    if (err != WAH_OK) {
        fprintf(stderr, "test_f32_demote_f64_nan_canonicalization FAILED: %s\n", wah_strerror(err));
        return 1;
    }

    printf("\nAll NaN canonicalization tests passed!\n");

    return 0;
}
