#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include "../wah.h"
#include "common.h"

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

void run_test(const char* test_name, const char* wasm_spec, void (*test_func)(wah_exec_context_t*)) {
    printf("Testing %s...\n", test_name);
    wah_module_t module;
    wah_exec_context_t ctx; // Declare context
    assert_ok(wah_parse_module_from_spec(&module, wasm_spec));
    assert_ok(wah_exec_context_create(&ctx, &module));
    test_func(&ctx); // Pass context to test_func
    wah_exec_context_destroy(&ctx); // Destroy context
    wah_free_module(&module);
}

static const char *i64_add_spec = "wasm \
    types {[ fn [i64, i64] [i64] ]} \
    funcs {[ 0 ]} \
    code {[ {[] local.get 0 local.get 1 i64.add end } ]}";

void test_i64_add(wah_exec_context_t* ctx) {
    wah_value_t params[2];
    wah_value_t result;
    params[0].i64 = 10000000000LL; // 10^10
    params[1].i64 = 25000000000LL; // 2.5 * 10^10
    assert_ok(wah_call(ctx, 0, params, 2, &result));
    assert_eq_i64(result.i64, 35000000000LL);
}

void test_i32_store_unaligned(wah_exec_context_t* ctx) {
    wah_value_t params[2];
    wah_value_t result;

    // Initialize memory to zeros
    for (int i = 0; i < 8; ++i) {
        params[0].i32 = i;
        params[1].i32 = 0;
        assert_ok(wah_call(ctx, FUNC_store_i8, params, 2, NULL));
    }

    // Store a value at an unaligned address (e.g., address 1)
    params[0].i32 = 1; // Unaligned address
    params[1].i32 = 0xAABBCCDD; // Value
    assert_ok(wah_call(ctx, FUNC_store_i32, params, 2, NULL));

    // Verify memory content by loading bytes
    // Expected: memory[0]=00, memory[1]=DD, memory[2]=CC, memory[3]=BB, memory[4]=AA, memory[5]=00
    params[0].i32 = 0;
    assert_ok(wah_call(ctx, FUNC_load_i32_8u, params, 1, &result));
    assert_eq_i32(result.i32, 0x00);

    params[0].i32 = 1;
    assert_ok(wah_call(ctx, FUNC_load_i32_8u, params, 1, &result));
    assert_eq_i32(result.i32, 0xDD);

    params[0].i32 = 2;
    assert_ok(wah_call(ctx, FUNC_load_i32_8u, params, 1, &result));
    assert_eq_i32(result.i32, 0xCC);

    params[0].i32 = 3;
    assert_ok(wah_call(ctx, FUNC_load_i32_8u, params, 1, &result));
    assert_eq_i32(result.i32, 0xBB);

    params[0].i32 = 4;
    assert_ok(wah_call(ctx, FUNC_load_i32_8u, params, 1, &result));
    assert_eq_i32(result.i32, 0xAA);

    params[0].i32 = 5;
    assert_ok(wah_call(ctx, FUNC_load_i32_8u, params, 1, &result));
    assert_eq_i32(result.i32, 0x00);
}

static const char *f32_mul_spec = "wasm \
    types {[ fn [f32, f32] [f32] ]} \
    funcs {[ 0 ]} \
    code {[ {[] local.get 0 local.get 1 f32.mul end } ]}";

void test_f32_mul(wah_exec_context_t* ctx) {
    wah_value_t params[2];
    wah_value_t result;
    params[0].f32 = 12.5f;
    params[1].f32 = -4.0f;
    assert_ok(wah_call(ctx, 0, params, 2, &result));
    assert_eq_f32(result.f32, -50.0f, 1e-6);
}

static const char *f64_sub_spec = "wasm \
    types {[ fn [f64, f64] [f64] ]} \
    funcs {[ 0 ]} \
    code {[ {[] local.get 0 local.get 1 f64.sub end } ]}";

void test_f64_sub(wah_exec_context_t* ctx) {
    wah_value_t params[2];
    wah_value_t result;
    params[0].f64 = 3.1415926535;
    params[1].f64 = 0.0000000005;
    assert_ok(wah_call(ctx, 0, params, 2, &result));
    assert_eq_f64(result.f64, 3.1415926530, 1e-9);
}

static const char *i64_overflow_spec = "wasm \
    types {[ fn [] [i32] ]} \
    funcs {[ 0 ]} \
    code {[ {[] i64.const 9223372036854775807 i64.const 1 i64.add i64.const 0 i64.lt_s end } ]}";

void test_i64_overflow(wah_exec_context_t* ctx) {
    wah_value_t result;
    assert_ok(wah_call(ctx, 0, NULL, 0, &result));
    // INT64_MAX + 1 wraps around to INT64_MIN, which is < 0, so comparison should be true (1)
    assert_eq_i32(result.i32, 1);
}

static const char *f64_div_zero_spec = "wasm \
    types {[ fn [] [f64] ]} \
    funcs {[ 0 ]} \
    code {[ {[] f64.const 1.0f64 f64.const 0.0f64 f64.div end } ]}";

void test_f64_div_zero(wah_exec_context_t* ctx) {
    wah_value_t result;
    assert_ok(wah_call(ctx, 0, NULL, 0, &result));
    assert_true(isinf(result.f64) && result.f64 > 0 && "f64_div_zero result is +inf");
}

static const char *i64_div_overflow_spec = "wasm \
    types {[ fn [] [i64] ]} \
    funcs {[ 0 ]} \
    code {[ {[] i64.const -9223372036854775808 i64.const -1 i64.div_s end } ]}";

void test_i64_div_overflow(wah_exec_context_t* ctx) {
    wah_value_t result;
    assert_err(wah_call(ctx, 0, NULL, 0, &result), WAH_ERROR_TRAP);
}

static const char *i64_const_drop_spec = "wasm \
    types {[ fn [] [] ]} \
    funcs {[ 0 ]} \
    code {[ {[] i64.const 123 drop end } ]}";

void test_i64_const_drop(wah_exec_context_t* ctx) {
    // This test only needs to parse successfully.
    (void)ctx;
}

void test_i32_load8_s_sign_extension(wah_exec_context_t* ctx) {
    wah_value_t params[2];
    wah_value_t result;

    // Test 1: Store -1 (0xFF) and load with sign extension
    params[0].i32 = 0; // address
    params[1].i32 = -1; // value (0xFF as byte)
    assert_ok(wah_call(ctx, FUNC_store_i8, params, 2, NULL));

    params[0].i32 = 0; // address
    assert_ok(wah_call(ctx, FUNC_load_i32_8s, params, 1, &result));
    assert_eq_i32(result.i32, -1);

    // Test 2: Store 0x7F (127) and load with sign extension
    params[0].i32 = 1; // address
    params[1].i32 = 127; // value (0x7F as byte)
    assert_ok(wah_call(ctx, FUNC_store_i8, params, 2, NULL));

    params[0].i32 = 1; // address
    assert_ok(wah_call(ctx, FUNC_load_i32_8s, params, 1, &result));
    assert_eq_i32(result.i32, 127);

    // Test 3: Store 0x80 (-128) and load with sign extension
    params[0].i32 = 2; // address
    params[1].i32 = -128; // value (0x80 as byte)
    assert_ok(wah_call(ctx, FUNC_store_i8, params, 2, NULL));

    params[0].i32 = 2; // address
    assert_ok(wah_call(ctx, FUNC_load_i32_8s, params, 1, &result));
    assert_eq_i32(result.i32, -128);
}

void test_i64_load8_s_sign_extension(wah_exec_context_t* ctx) {
    wah_value_t params[2];
    wah_value_t result;

    // Test case 1: -1 (0xFF) should sign extend to -1 (0xFFFFFFFFFFFFFFFF)
    params[0].i32 = 0; // Address
    params[1].i64 = -1; // Value to store (will be truncated to 0xFF)
    assert_ok(wah_call(ctx, FUNC_store_i64_8, params, 2, NULL));

    assert_ok(wah_call(ctx, FUNC_load_i64_8s, params, 1, &result));
    assert_eq_i64(result.i64, -1);

    // Test case 2: 127 (0x7F) should not sign extend
    params[0].i32 = 0; // Address
    params[1].i64 = 127; // Value to store
    assert_ok(wah_call(ctx, FUNC_store_i64_8, params, 2, NULL));

    assert_ok(wah_call(ctx, FUNC_load_i64_8s, params, 1, &result));
    assert_eq_i64(result.i64, 127);

    // Test case 3: -128 (0x80) should sign extend to -128
    params[0].i32 = 0; // Address
    params[1].i64 = -128; // Value to store
    assert_ok(wah_call(ctx, FUNC_store_i64_8, params, 2, NULL));

    assert_ok(wah_call(ctx, FUNC_load_i64_8s, params, 1, &result));
    assert_eq_i64(result.i64, -128);
}

void test_i32_load16_s_sign_extension(wah_exec_context_t* ctx) {
    wah_value_t params[2];
    wah_value_t result;

    // Test 1: Store -1 (0xFFFF) and load with sign extension
    params[0].i32 = 0; // address
    params[1].i32 = -1; // value (0xFFFF as 2 bytes)
    assert_ok(wah_call(ctx, FUNC_store_i16, params, 2, NULL));

    params[0].i32 = 0; // address
    assert_ok(wah_call(ctx, FUNC_load_i32_16s, params, 1, &result));
    assert_eq_i32(result.i32, -1);

    // Test 2: Store 0x7FFF (32767) and load with sign extension
    params[0].i32 = 2; // address
    params[1].i32 = 32767; // value (0x7FFF as 2 bytes)
    assert_ok(wah_call(ctx, FUNC_store_i16, params, 2, NULL));

    params[0].i32 = 2; // address
    assert_ok(wah_call(ctx, FUNC_load_i32_16s, params, 1, &result));
    assert_eq_i32(result.i32, 32767);

    // Test 3: Store 0x8000 (-32768) and load with sign extension
    params[0].i32 = 4; // address
    params[1].i32 = -32768; // value (0x8000 as 2 bytes)
    assert_ok(wah_call(ctx, FUNC_store_i16, params, 2, NULL));

    params[0].i32 = 4; // address
    assert_ok(wah_call(ctx, FUNC_load_i32_16s, params, 1, &result));
    assert_eq_i32(result.i32, -32768);
}

void test_i64_load16_s_sign_extension(wah_exec_context_t* ctx) {
    wah_value_t params[2];
    wah_value_t result;

    // Test case 1: -1 (0xFFFF) should sign extend to -1 (0xFFFFFFFFFFFFFFFF)
    params[0].i32 = 0; // Address
    params[1].i64 = -1; // Value to store (will be truncated to 0xFFFF)
    assert_ok(wah_call(ctx, FUNC_store_i64_16, params, 2, NULL));

    assert_ok(wah_call(ctx, FUNC_load_i64_16s, params, 1, &result));
    assert_eq_i64(result.i64, -1);

    // Test case 2: 32767 (0x7FFF) should not sign extend
    params[0].i32 = 0; // Address
    params[1].i64 = 32767; // Value to store
    assert_ok(wah_call(ctx, FUNC_store_i64_16, params, 2, NULL));

    assert_ok(wah_call(ctx, FUNC_load_i64_16s, params, 1, &result));
    assert_eq_i64(result.i64, 32767);

    // Test case 3: -32768 (0x8000) should sign extend to -32768
    params[0].i32 = 0; // Address
    params[1].i64 = -32768; // Value to store
    assert_ok(wah_call(ctx, FUNC_store_i64_16, params, 2, NULL));

    assert_ok(wah_call(ctx, FUNC_load_i64_16s, params, 1, &result));
    assert_eq_i64(result.i64, -32768);
}

void test_i64_load32_s_sign_extension(wah_exec_context_t* ctx) {
    wah_value_t params[2];
    wah_value_t result;

    // Test case 1: -1 (0xFFFFFFFF) should sign extend to -1 (0xFFFFFFFFFFFFFFFF)
    params[0].i32 = 0; // Address
    params[1].i64 = -1; // Value to store (will be truncated to 0xFFFFFFFF)
    assert_ok(wah_call(ctx, FUNC_store_i64_32, params, 2, NULL));

    assert_ok(wah_call(ctx, FUNC_load_i64_32s, params, 1, &result));
    assert_eq_i64(result.i64, -1);

    // Test case 2: 2147483647 (0x7FFFFFFF) should not sign extend
    params[0].i32 = 0; // Address
    params[1].i64 = 2147483647LL; // Value to store
    assert_ok(wah_call(ctx, FUNC_store_i64_32, params, 2, NULL));

    assert_ok(wah_call(ctx, FUNC_load_i64_32s, params, 1, &result));
    assert_eq_i64(result.i64, 2147483647LL);

    // Test case 3: -2147483648 (0x80000000) should sign extend to -2147483648
    params[0].i32 = 0; // Address
    params[1].i64 = -2147483648LL; // Value to store
    assert_ok(wah_call(ctx, FUNC_store_i64_32, params, 2, NULL));

    assert_ok(wah_call(ctx, FUNC_load_i64_32s, params, 1, &result));
    assert_eq_i64(result.i64, -2147483648LL);
}

void test_i32_load_unaligned(wah_exec_context_t* ctx) {
    wah_value_t params[2];
    wah_value_t result;

    // Store a value at an aligned address (e.g., address 0)
    params[0].i32 = 0; // Address
    params[1].i32 = 0x12345678; // Value
    assert_ok(wah_call(ctx, FUNC_store_i32, params, 2, NULL));

    // Load from an unaligned address (e.g., address 1)
    params[0].i32 = 1; // Unaligned address
    assert_ok(wah_call(ctx, FUNC_load_i32, params, 1, &result));
    // Expected value: 0xXX123456 (assuming little-endian and memory[0]=78, memory[1]=56, memory[2]=34, memory[3]=12)
    // If we load from address 1, we get memory[1], memory[2], memory[3], memory[4]
    // Since memory[4] is 0, the value should be 0x00123456
    assert_eq_i32(result.i32, 0x00123456);

    // Store another value at an aligned address (e.g., address 4)
    params[0].i32 = 4; // Address
    params[1].i32 = 0xAABBCCDD; // Value
    assert_ok(wah_call(ctx, FUNC_store_i32, params, 2, NULL));

    // Load from an unaligned address (e.g., address 2)
    params[0].i32 = 2; // Unaligned address
    assert_ok(wah_call(ctx, FUNC_load_i32, params, 1, &result));
    // Expected value: 0xCCDD1234 (assuming little-endian and memory[0]=78, memory[1]=56, memory[2]=34, memory[3]=12, memory[4]=DD, memory[5]=CC, memory[6]=BB, memory[7]=AA)
    // If we load from address 2, we get memory[2], memory[3], memory[4], memory[5]
    // So, 0xCCDD1234
    assert_eq_i32(result.i32, (int32_t)0xCCDD1234);
}

int main() {
    run_test("i64_add", i64_add_spec, test_i64_add);
    run_test("f32_mul", f32_mul_spec, test_f32_mul);
    run_test("f64_sub", f64_sub_spec, test_f64_sub);
    run_test("i64_overflow", i64_overflow_spec, test_i64_overflow);
    run_test("f64_div_zero", f64_div_zero_spec, test_f64_div_zero);
    run_test("i64_div_overflow", i64_div_overflow_spec, test_i64_div_overflow);
    run_test("i64_const_drop", i64_const_drop_spec, test_i64_const_drop);
    run_test("i32_load8_s_sign_extension", memory_access_spec, test_i32_load8_s_sign_extension);
    run_test("i32_load16_s_sign_extension", memory_access_spec, test_i32_load16_s_sign_extension);
    run_test("i64_load8_s_sign_extension", memory_access_spec, test_i64_load8_s_sign_extension);
    run_test("i64_load16_s_sign_extension", memory_access_spec, test_i64_load16_s_sign_extension);
    run_test("i64_load32_s_sign_extension", memory_access_spec, test_i64_load32_s_sign_extension);
    run_test("i32_load_unaligned", memory_access_spec, test_i32_load_unaligned);
    run_test("i32_store_unaligned", memory_access_spec, test_i32_store_unaligned);

    printf("\n--- ALL TESTS PASSED ---\n");
    return 0;
}
