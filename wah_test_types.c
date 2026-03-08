#define WAH_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include "wah.h"
#include "wah_testutils.c"

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\n", message); \
        return 1; \
    } else { \
        printf("PASS: %s\n", message); \
    } \
} while(0)

#define CHECK_ERR(err, expected, message) do { \
    if (err != expected) { \
        fprintf(stderr, "FAIL: %s (got %d, expected %d)\n", message, err, expected); \
        return 1; \
    } else { \
        printf("PASS: %s\n", message); \
    } \
} while(0)

#define CHECK_FLOAT(val, exp, epsilon, message) do { \
    if (fabsf(val - exp) > epsilon) { \
        fprintf(stderr, "FAIL: %s (got %f, expected %f)\n", message, val, exp); \
        return 1; \
    } else { \
        printf("PASS: %s\n", message); \
    } \
} while(0)

#define CHECK_DOUBLE(val, exp, epsilon, message) do { \
    if (fabs(val - exp) > epsilon) { \
        fprintf(stderr, "FAIL: %s (got %f, expected %f)\n", message, val, exp); \
        return 1; \
    } else { \
        printf("PASS: %s\n", message); \
    } \
} while(0)

static const char *memory_access_spec = "wasm \
    types {[ \
        fn [i32, i32] [], fn [i32] [i32], \
        fn [i32, i64] [], fn [i32] [i64], \
        fn [i32, f32] [], fn [i32] [f32], \
        fn [i32, f64] [], fn [i32] [f64], \
    ]} \
    funcs {[ \
        0, 1, 1, \
        0, 1, 1, \
        0, 1, \
        2, 3, \
        2, 3, 3, \
        2, 3, 3, \
        2, 3, 3, \
        4, 5, \
        6, 7, \
    ]} \
    memories {[ limits.i32/1 1 ]} \
    exports {[ \
        {'mem'} mem# 0, \
        {'store_i8'} fn# 0, {'load_i32_8s'} fn# 1, {'load_i32_8u'} fn# 2, \
        {'store_i16'} fn# 3, {'load_i32_16s'} fn# 4, {'load_i32_16u'} fn# 5, \
        {'store_i32'} fn# 6, {'load_i32'} fn# 7, \
        {'store_i64'} fn# 8, {'load_i64'} fn# 9, \
        {'store_i64_8'} fn# 10, {'load_i64_8s'} fn# 11, {'load_i64_8u'} fn# 12, \
        {'store_i64_16'} fn# 13, {'load_i64_16s'} fn# 14, {'load_i64_16u'} fn# 15, \
        {'store_i64_32'} fn# 16, {'load_i64_32s'} fn# 17, {'load_i64_32u'} fn# 18, \
        {'store_f32'} fn# 19, {'load_f32'} fn# 20, \
        {'store_f64'} fn# 21, {'load_f64'} fn# 22, \
    ]} \
    code {[ \
        {[] local.get 0 local.get 1 i32.store8   align=1 offset=0 end}, \
        {[] local.get 0             i32.load8_s  align=1 offset=0 end}, \
        {[] local.get 0             i32.load8_u  align=1 offset=0 end}, \
        {[] local.get 0 local.get 1 i32.store16  align=2 offset=0 end}, \
        {[] local.get 0             i32.load16_s align=2 offset=0 end}, \
        {[] local.get 0             i32.load16_u align=2 offset=0 end}, \
        {[] local.get 0 local.get 1 i32.store    align=4 offset=0 end}, \
        {[] local.get 0             i32.load     align=4 offset=0 end}, \
        {[] local.get 0 local.get 1 i64.store    align=8 offset=0 end}, \
        {[] local.get 0             i64.load     align=8 offset=0 end}, \
        {[] local.get 0 local.get 1 i64.store8   align=1 offset=0 end}, \
        {[] local.get 0             i64.load8_s  align=1 offset=0 end}, \
        {[] local.get 0             i64.load8_u  align=1 offset=0 end}, \
        {[] local.get 0 local.get 1 i64.store16  align=2 offset=0 end}, \
        {[] local.get 0             i64.load16_s align=2 offset=0 end}, \
        {[] local.get 0             i64.load16_u align=2 offset=0 end}, \
        {[] local.get 0 local.get 1 i64.store32  align=4 offset=0 end}, \
        {[] local.get 0             i64.load32_s align=4 offset=0 end}, \
        {[] local.get 0             i64.load32_u align=4 offset=0 end}, \
        {[] local.get 0 local.get 1 f32.store    align=4 offset=0 end}, \
        {[] local.get 0             f32.load     align=4 offset=0 end}, \
        {[] local.get 0 local.get 1 f64.store    align=8 offset=0 end}, \
        {[] local.get 0             f64.load     align=8 offset=0 end}, \
    ]}";

#define FUNC_store_i8 0
#define FUNC_load_i32_8s 1
#define FUNC_load_i32_8u 2
#define FUNC_store_i16 3
#define FUNC_load_i32_16s 4
#define FUNC_load_i32_16u 5
#define FUNC_store_i32 6
#define FUNC_load_i32 7
#define FUNC_store_i64 8
#define FUNC_load_i64 9
#define FUNC_store_i64_8 10
#define FUNC_load_i64_8s 11
#define FUNC_load_i64_8u 12
#define FUNC_store_i64_16 13
#define FUNC_load_i64_16s 14
#define FUNC_load_i64_16u 15
#define FUNC_store_i64_32 16
#define FUNC_load_i64_32s 17
#define FUNC_load_i64_32u 18
#define FUNC_store_f32 19
#define FUNC_load_f32 20
#define FUNC_store_f64 21
#define FUNC_load_f64 22

int run_test(const char* test_name, const char* wasm_spec, int (*test_func)(wah_module_t*, wah_exec_context_t*)) {
    printf("\n--- Testing %s ---\n", test_name);
    wah_module_t module;
    wah_exec_context_t ctx; // Declare context
    wah_error_t err = wah_parse_module_from_spec(&module, wasm_spec);
    if (err != WAH_OK) {
        fprintf(stderr, "FAIL: Parsing %s (error: %s)\n", test_name, wah_strerror(err));
        return 1;
    }
    printf("PASS: Parsing %s\n", test_name);

    // Create execution context
    err = wah_exec_context_create(&ctx, &module);
    if (err != WAH_OK) {
        fprintf(stderr, "FAIL: Creating execution context for %s (error: %d)\n", test_name, err);
        wah_free_module(&module);
        return 1;
    }

    int result = test_func(&module, &ctx); // Pass context to test_func
    wah_exec_context_destroy(&ctx); // Destroy context
    wah_free_module(&module);
    return result;
}

static const char *i64_add_spec = "wasm \
    types {[ fn [i64, i64] [i64] ]} \
    funcs {[ 0 ]} \
    code {[ {[] local.get 0 local.get 1 i64.add end } ]}";

int test_i64_add(wah_module_t* module, wah_exec_context_t* ctx) {
    wah_value_t params[2];
    wah_value_t result;
    params[0].i64 = 10000000000LL; // 10^10
    params[1].i64 = 25000000000LL; // 2.5 * 10^10
    wah_error_t err = wah_call(ctx, module, 0, params, 2, &result);
    CHECK_ERR(err, WAH_OK, "i64_add wah_call");
    CHECK(result.i64 == 35000000000LL, "i64_add result");
    return 0;
}

int test_i32_store_unaligned(wah_module_t* module, wah_exec_context_t* ctx) {
    wah_value_t params[2];
    wah_value_t result;
    wah_error_t err;
    int failed_checks = 0;

    // Initialize memory to zeros
    for (int i = 0; i < 8; ++i) {
        params[0].i32 = i;
        params[1].i32 = 0;
        err = wah_call(ctx, module, FUNC_store_i8, params, 2, NULL);
        CHECK(err == WAH_OK, "store_i8 0");
    }

    // Store a value at an unaligned address (e.g., address 1)
    params[0].i32 = 1; // Unaligned address
    params[1].i32 = 0xAABBCCDD; // Value
    err = wah_call(ctx, module, FUNC_store_i32, params, 2, NULL);
    CHECK(err == WAH_OK, "store_i32 0xAABBCCDD at unaligned address 1");

    // Verify memory content by loading bytes
    // Expected: memory[0]=00, memory[1]=DD, memory[2]=CC, memory[3]=BB, memory[4]=AA, memory[5]=00
    params[0].i32 = 0;
    err = wah_call(ctx, module, FUNC_load_i32_8u, params, 1, &result);
    CHECK(err == WAH_OK, "load_i32_8u from address 0");
    CHECK(result.i32 == 0x00, "memory[0] is 0x00");

    params[0].i32 = 1;
    err = wah_call(ctx, module, FUNC_load_i32_8u, params, 1, &result);
    CHECK(err == WAH_OK, "load_i32_8u from address 1");
    CHECK(result.i32 == 0xDD, "memory[1] is 0xDD");

    params[0].i32 = 2;
    err = wah_call(ctx, module, FUNC_load_i32_8u, params, 1, &result);
    CHECK(err == WAH_OK, "load_i32_8u from address 2");
    CHECK(result.i32 == 0xCC, "memory[2] is 0xCC");

    params[0].i32 = 3;
    err = wah_call(ctx, module, FUNC_load_i32_8u, params, 1, &result);
    CHECK(err == WAH_OK, "load_i32_8u from address 3");
    CHECK(result.i32 == 0xBB, "memory[3] is 0xBB");

    params[0].i32 = 4;
    err = wah_call(ctx, module, FUNC_load_i32_8u, params, 1, &result);
    CHECK(err == WAH_OK, "load_i32_8u from address 4");
    CHECK(result.i32 == 0xAA, "memory[4] is 0xAA");

    params[0].i32 = 5;
    err = wah_call(ctx, module, FUNC_load_i32_8u, params, 1, &result);
    CHECK(err == WAH_OK, "load_i32_8u from address 5");
    CHECK(result.i32 == 0x00, "memory[5] is 0x00");

    return failed_checks;
}

static const char *f32_mul_spec = "wasm \
    types {[ fn [f32, f32] [f32] ]} \
    funcs {[ 0 ]} \
    code {[ {[] local.get 0 local.get 1 f32.mul end } ]}";

int test_f32_mul(wah_module_t* module, wah_exec_context_t* ctx) {
    wah_value_t params[2];
    wah_value_t result;
    params[0].f32 = 12.5f;
    params[1].f32 = -4.0f;
    wah_error_t err = wah_call(ctx, module, 0, params, 2, &result);
    CHECK_ERR(err, WAH_OK, "f32_mul wah_call");
    CHECK_FLOAT(result.f32, -50.0f, 1e-6, "f32_mul result");
    return 0;
}

static const char *f64_sub_spec = "wasm \
    types {[ fn [f64, f64] [f64] ]} \
    funcs {[ 0 ]} \
    code {[ {[] local.get 0 local.get 1 f64.sub end } ]}";

int test_f64_sub(wah_module_t* module, wah_exec_context_t* ctx) {
    wah_value_t params[2];
    wah_value_t result;
    params[0].f64 = 3.1415926535;
    params[1].f64 = 0.0000000005;
    wah_error_t err = wah_call(ctx, module, 0, params, 2, &result);
    CHECK_ERR(err, WAH_OK, "f64_sub wah_call");
    CHECK_DOUBLE(result.f64, 3.1415926530, 1e-9, "f64_sub result");
    return 0;
}

static const char *i64_overflow_spec = "wasm \
    types {[ fn [] [i32] ]} \
    funcs {[ 0 ]} \
    code {[ {[] i64.const 9223372036854775807 i64.const 1 i64.add i64.const 0 i64.lt_s end } ]}";

int test_i64_overflow(wah_module_t* module, wah_exec_context_t* ctx) {
    wah_value_t result;
    wah_error_t err = wah_call(ctx, module, 0, NULL, 0, &result);
    CHECK_ERR(err, WAH_OK, "i64_overflow wah_call");
    // INT64_MAX + 1 wraps around to INT64_MIN, which is < 0, so comparison should be true (1)
    CHECK(result.i32 == 1, "i64_overflow result");
    return 0;
}

static const char *f64_div_zero_spec = "wasm \
    types {[ fn [] [f64] ]} \
    funcs {[ 0 ]} \
    code {[ {[] f64.const 1.0f64 f64.const 0.0f64 f64.div end } ]}";

int test_f64_div_zero(wah_module_t* module, wah_exec_context_t* ctx) {
    wah_value_t result;
    wah_error_t err = wah_call(ctx, module, 0, NULL, 0, &result);
    CHECK_ERR(err, WAH_OK, "f64_div_zero wah_call");
    CHECK(isinf(result.f64) && result.f64 > 0, "f64_div_zero result is +inf");
    return 0;
}

static const char *i64_div_overflow_spec = "wasm \
    types {[ fn [] [i64] ]} \
    funcs {[ 0 ]} \
    code {[ {[] i64.const -9223372036854775808 i64.const -1 i64.div_s end } ]}";

int test_i64_div_overflow(wah_module_t* module, wah_exec_context_t* ctx) {
    wah_value_t result;
    wah_error_t err = wah_call(ctx, module, 0, NULL, 0, &result);
    CHECK_ERR(err, WAH_ERROR_TRAP, "i64_div_overflow wah_call traps");
    return 0;
}

static const char *i64_const_drop_spec = "wasm \
    types {[ fn [] [] ]} \
    funcs {[ 0 ]} \
    code {[ {[] i64.const 123 drop end } ]}";

int test_i64_const_drop(wah_module_t* module, wah_exec_context_t* ctx) {
    // This test only needs to parse successfully.
    (void)module;
    (void)ctx;
    return 0;
}

int test_i32_load8_s_sign_extension(wah_module_t* module, wah_exec_context_t* ctx) {
    wah_value_t params[2];
    wah_value_t result;
    wah_error_t err;

    // Test 1: Store -1 (0xFF) and load with sign extension
    params[0].i32 = 0; // address
    params[1].i32 = -1; // value (0xFF as byte)
    err = wah_call(ctx, module, FUNC_store_i8, params, 2, NULL);
    CHECK_ERR(err, WAH_OK, "store_i8 -1");

    params[0].i32 = 0; // address
    err = wah_call(ctx, module, FUNC_load_i32_8s, params, 1, &result);
    CHECK_ERR(err, WAH_OK, "load_i32_8s -1");
    CHECK(result.i32 == -1, "i32.load8_s -1 result");

    // Test 2: Store 0x7F (127) and load with sign extension
    params[0].i32 = 1; // address
    params[1].i32 = 127; // value (0x7F as byte)
    err = wah_call(ctx, module, FUNC_store_i8, params, 2, NULL);
    CHECK_ERR(err, WAH_OK, "store_i8 127");

    params[0].i32 = 1; // address
    err = wah_call(ctx, module, FUNC_load_i32_8s, params, 1, &result);
    CHECK_ERR(err, WAH_OK, "load_i32_8s 127");
    CHECK(result.i32 == 127, "i32.load8_s 127 result");

    // Test 3: Store 0x80 (-128) and load with sign extension
    params[0].i32 = 2; // address
    params[1].i32 = -128; // value (0x80 as byte)
    err = wah_call(ctx, module, FUNC_store_i8, params, 2, NULL);
    CHECK_ERR(err, WAH_OK, "store_i8 -128");

    params[0].i32 = 2; // address
    err = wah_call(ctx, module, FUNC_load_i32_8s, params, 1, &result);
    CHECK_ERR(err, WAH_OK, "load_i32_8s -128");
    CHECK(result.i32 == -128, "i32.load8_s -128 result");

    return 0;
}

int test_i64_load8_s_sign_extension(wah_module_t* module, wah_exec_context_t* ctx) {
    wah_value_t params[2];
    wah_value_t result;
    wah_error_t err;
    int failed_checks = 0;

    // Test case 1: -1 (0xFF) should sign extend to -1 (0xFFFFFFFFFFFFFFFF)
    params[0].i32 = 0; // Address
    params[1].i64 = -1; // Value to store (will be truncated to 0xFF)
    err = wah_call(ctx, module, FUNC_store_i64_8, params, 2, NULL);
    CHECK(err == WAH_OK, "store_i64_8 -1");

    err = wah_call(ctx, module, FUNC_load_i64_8s, params, 1, &result);
    CHECK(err == WAH_OK, "load_i64_8s -1");
    CHECK(result.i64 == -1, "i64.load8_s -1 result");

    // Test case 2: 127 (0x7F) should not sign extend
    params[0].i32 = 0; // Address
    params[1].i64 = 127; // Value to store
    err = wah_call(ctx, module, FUNC_store_i64_8, params, 2, NULL);
    CHECK(err == WAH_OK, "store_i64_8 127");

    err = wah_call(ctx, module, FUNC_load_i64_8s, params, 1, &result);
    CHECK(err == WAH_OK, "load_i64_8s 127");
    CHECK(result.i64 == 127, "i64.load8_s 127 result");

    // Test case 3: -128 (0x80) should sign extend to -128
    params[0].i32 = 0; // Address
    params[1].i64 = -128; // Value to store
    err = wah_call(ctx, module, FUNC_store_i64_8, params, 2, NULL);
    CHECK(err == WAH_OK, "store_i64_8 -128");

    err = wah_call(ctx, module, FUNC_load_i64_8s, params, 1, &result);
    CHECK(err == WAH_OK, "load_i64_8s -128");
    CHECK(result.i64 == -128, "i64.load8_s -128 result");

    return failed_checks;
}

int test_i32_load16_s_sign_extension(wah_module_t* module, wah_exec_context_t* ctx) {
    wah_value_t params[2];
    wah_value_t result;
    wah_error_t err;

    // Test 1: Store -1 (0xFFFF) and load with sign extension
    params[0].i32 = 0; // address
    params[1].i32 = -1; // value (0xFFFF as 2 bytes)
    err = wah_call(ctx, module, FUNC_store_i16, params, 2, NULL);
    CHECK_ERR(err, WAH_OK, "store_i16 -1");

    params[0].i32 = 0; // address
    err = wah_call(ctx, module, FUNC_load_i32_16s, params, 1, &result);
    CHECK_ERR(err, WAH_OK, "load_i32_16s -1");
    CHECK(result.i32 == -1, "i32.load16_s -1 result");

    // Test 2: Store 0x7FFF (32767) and load with sign extension
    params[0].i32 = 2; // address
    params[1].i32 = 32767; // value (0x7FFF as 2 bytes)
    err = wah_call(ctx, module, FUNC_store_i16, params, 2, NULL);
    CHECK_ERR(err, WAH_OK, "store_i16 32767");

    params[0].i32 = 2; // address
    err = wah_call(ctx, module, FUNC_load_i32_16s, params, 1, &result);
    CHECK_ERR(err, WAH_OK, "load_i32_16s 32767");
    CHECK(result.i32 == 32767, "i32.load16_s 32767 result");

    // Test 3: Store 0x8000 (-32768) and load with sign extension
    params[0].i32 = 4; // address
    params[1].i32 = -32768; // value (0x8000 as 2 bytes)
    err = wah_call(ctx, module, FUNC_store_i16, params, 2, NULL);
    CHECK_ERR(err, WAH_OK, "store_i16 -32768");

    params[0].i32 = 4; // address
    err = wah_call(ctx, module, FUNC_load_i32_16s, params, 1, &result);
    CHECK_ERR(err, WAH_OK, "load_i32_16s -32768");
    CHECK(result.i32 == -32768, "i32.load16_s -32768 result");

    return 0;
}

int test_i64_load16_s_sign_extension(wah_module_t* module, wah_exec_context_t* ctx) {
    wah_value_t params[2];
    wah_value_t result;
    wah_error_t err;
    int failed_checks = 0;

    // Test case 1: -1 (0xFFFF) should sign extend to -1 (0xFFFFFFFFFFFFFFFF)
    params[0].i32 = 0; // Address
    params[1].i64 = -1; // Value to store (will be truncated to 0xFFFF)
    err = wah_call(ctx, module, FUNC_store_i64_16, params, 2, NULL);
    CHECK(err == WAH_OK, "store_i64_16 -1");

    err = wah_call(ctx, module, FUNC_load_i64_16s, params, 1, &result);
    CHECK(err == WAH_OK, "load_i64_16s -1");
    CHECK(result.i64 == -1, "i64.load16_s -1 result");

    // Test case 2: 32767 (0x7FFF) should not sign extend
    params[0].i32 = 0; // Address
    params[1].i64 = 32767; // Value to store
    err = wah_call(ctx, module, FUNC_store_i64_16, params, 2, NULL);
    CHECK(err == WAH_OK, "store_i64_16 32767");

    err = wah_call(ctx, module, FUNC_load_i64_16s, params, 1, &result);
    CHECK(err == WAH_OK, "load_i64_16s 32767");
    CHECK(result.i64 == 32767, "i64.load16_s 32767 result");

    // Test case 3: -32768 (0x8000) should sign extend to -32768
    params[0].i32 = 0; // Address
    params[1].i64 = -32768; // Value to store
    err = wah_call(ctx, module, FUNC_store_i64_16, params, 2, NULL);
    CHECK(err == WAH_OK, "store_i64_16 -32768");

    err = wah_call(ctx, module, FUNC_load_i64_16s, params, 1, &result);
    CHECK(err == WAH_OK, "load_i64_16s -32768");
    CHECK(result.i64 == -32768, "i64.load16_s -32768 result");

    return failed_checks;
}

int test_i64_load32_s_sign_extension(wah_module_t* module, wah_exec_context_t* ctx) {
    wah_value_t params[2];
    wah_value_t result;
    wah_error_t err;
    int failed_checks = 0;

    // Test case 1: -1 (0xFFFFFFFF) should sign extend to -1 (0xFFFFFFFFFFFFFFFF)
    params[0].i32 = 0; // Address
    params[1].i64 = -1; // Value to store (will be truncated to 0xFFFFFFFF)
    err = wah_call(ctx, module, FUNC_store_i64_32, params, 2, NULL);
    CHECK(err == WAH_OK, "store_i64_32 -1");

    err = wah_call(ctx, module, FUNC_load_i64_32s, params, 1, &result);
    CHECK(err == WAH_OK, "load_i64_32s -1");
    CHECK(result.i64 == -1, "i64.load32_s -1 result");

    // Test case 2: 2147483647 (0x7FFFFFFF) should not sign extend
    params[0].i32 = 0; // Address
    params[1].i64 = 2147483647LL; // Value to store
    err = wah_call(ctx, module, FUNC_store_i64_32, params, 2, NULL);
    CHECK(err == WAH_OK, "store_i64_32 2147483647");

    err = wah_call(ctx, module, FUNC_load_i64_32s, params, 1, &result);
    CHECK(err == WAH_OK, "load_i64_32s 2147483647");
    CHECK(result.i64 == 2147483647LL, "i64.load32_s 2147483647 result");

    // Test case 3: -2147483648 (0x80000000) should sign extend to -2147483648
    params[0].i32 = 0; // Address
    params[1].i64 = -2147483648LL; // Value to store
    err = wah_call(ctx, module, FUNC_store_i64_32, params, 2, NULL);
    CHECK(err == WAH_OK, "store_i64_32 -2147483648");

    err = wah_call(ctx, module, FUNC_load_i64_32s, params, 1, &result);
    CHECK(err == WAH_OK, "load_i64_32s -2147483648");
    CHECK(result.i64 == -2147483648LL, "i64.load32_s -2147483648 result");

    return failed_checks;
}

int test_i32_load_unaligned(wah_module_t* module, wah_exec_context_t* ctx) {
    wah_value_t params[2];
    wah_value_t result;
    wah_error_t err;
    int failed_checks = 0;

    // Store a value at an aligned address (e.g., address 0)
    params[0].i32 = 0; // Address
    params[1].i32 = 0x12345678; // Value
    err = wah_call(ctx, module, FUNC_store_i32, params, 2, NULL);
    CHECK(err == WAH_OK, "store_i32 0x12345678 at address 0");

    // Load from an unaligned address (e.g., address 1)
    params[0].i32 = 1; // Unaligned address
    err = wah_call(ctx, module, FUNC_load_i32, params, 1, &result);
    CHECK(err == WAH_OK, "load_i32 from unaligned address 1");
    // Expected value: 0xXX123456 (assuming little-endian and memory[0]=78, memory[1]=56, memory[2]=34, memory[3]=12)
    // If we load from address 1, we get memory[1], memory[2], memory[3], memory[4]
    // Since memory[4] is 0, the value should be 0x00123456
    CHECK(result.i32 == 0x00123456, "i32.load from unaligned address 1 result");

    // Store another value at an aligned address (e.g., address 4)
    params[0].i32 = 4; // Address
    params[1].i32 = 0xAABBCCDD; // Value
    err = wah_call(ctx, module, FUNC_store_i32, params, 2, NULL);
    CHECK(err == WAH_OK, "store_i32 0xAABBCCDD at address 4");

    // Load from an unaligned address (e.g., address 2)
    params[0].i32 = 2; // Unaligned address
    err = wah_call(ctx, module, FUNC_load_i32, params, 1, &result);
    CHECK(err == WAH_OK, "load_i32 from unaligned address 2");
    // Expected value: 0xCCDD1234 (assuming little-endian and memory[0]=78, memory[1]=56, memory[2]=34, memory[3]=12, memory[4]=DD, memory[5]=CC, memory[6]=BB, memory[7]=AA)
    // If we load from address 2, we get memory[2], memory[3], memory[4], memory[5]
    // So, 0xCCDD1234
    CHECK(result.i32 == (int32_t)0xCCDD1234, "i32.load from unaligned address 2 result");

    return failed_checks;
}

int main() {
    int failed = 0;
    failed |= run_test("i64_add", i64_add_spec, test_i64_add);
    failed |= run_test("f32_mul", f32_mul_spec, test_f32_mul);
    failed |= run_test("f64_sub", f64_sub_spec, test_f64_sub);
    failed |= run_test("i64_overflow", i64_overflow_spec, test_i64_overflow);
    failed |= run_test("f64_div_zero", f64_div_zero_spec, test_f64_div_zero);
    failed |= run_test("i64_div_overflow", i64_div_overflow_spec, test_i64_div_overflow);
    failed |= run_test("i64_const_drop", i64_const_drop_spec, test_i64_const_drop);
    failed |= run_test("i32_load8_s_sign_extension", memory_access_spec, test_i32_load8_s_sign_extension);
    failed |= run_test("i32_load16_s_sign_extension", memory_access_spec, test_i32_load16_s_sign_extension);
    failed |= run_test("i64_load8_s_sign_extension", memory_access_spec, test_i64_load8_s_sign_extension);
    failed |= run_test("i64_load16_s_sign_extension", memory_access_spec, test_i64_load16_s_sign_extension);
    failed |= run_test("i64_load32_s_sign_extension", memory_access_spec, test_i64_load32_s_sign_extension);
    failed |= run_test("i32_load_unaligned", memory_access_spec, test_i32_load_unaligned);
    failed |= run_test("i32_store_unaligned", memory_access_spec, test_i32_store_unaligned);

    if (failed) {
        printf("\n--- SOME TESTS FAILED ---\n");
    } else {
        printf("\n--- ALL TESTS PASSED ---\n");
    }

    return failed;
}
