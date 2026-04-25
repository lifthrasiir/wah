#define WAH_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // For memcmp
#include "../wah.h"
#include "common.h"

// Helper to compare v128 results and print byte-by-byte if they don't match.
static void compare_and_print_v128_result(const char* test_name, const wah_v128_t* actual, const wah_v128_t* expected) {
    if (memcmp(actual, expected, sizeof(wah_v128_t)) != 0) {
        fprintf(stderr, "Result v128 does NOT match expected value for %s.\n", test_name);
        fprintf(stderr, "Expected: ");
        for (int i = 0; i < 16; ++i) {
            fprintf(stderr, "%02x ", expected->u8[i]);
        }
        fprintf(stderr, "\nActual:   ");
        for (int i = 0; i < 16; ++i) {
            fprintf(stderr, "%02x ", actual->u8[i]);
        }
        fprintf(stderr, "\n");
        exit(1);
    }
}

// Helper to set up module and execution context
static void wah_test_setup_contextv(const char* test_name, const char *spec, wah_module_t* out_module, wah_exec_context_t* out_ctx, va_list args) {
    printf("Testing %s...\n", test_name);
    assert_ok(wah_parse_module_from_specv(out_module, spec, args));
    assert_ok(wah_exec_context_create(out_ctx, out_module));
}

static void wah_test_setup_context(const char* test_name, const char *spec, wah_module_t* out_module, wah_exec_context_t* out_ctx, ...) {
    va_list args;
    va_start(args, out_ctx);
    wah_test_setup_contextv(test_name, spec, out_module, out_ctx, args);
    va_end(args);
}

// Helper to execute a WASM function and return its result
static void wah_test_execute_function(wah_exec_context_t* ctx, wah_value_t* out_result) {
    uint32_t func_idx = 0; // Assuming the test function is always at index 0
    assert_ok(wah_call(ctx, func_idx, NULL, 0, out_result));
}

static const char *unary_op_wasm_spec = "wasm \
    types {[ fn [] [v128] ]} \
    funcs {[ 0 ]} \
    code {[ {[] v128.const %v128 %t end} ]} \
";

static const char *binary_op_wasm_spec = "wasm \
    types {[ fn [] [v128] ]} \
    funcs {[ 0 ]} \
    code {[ {[] v128.const %v128 v128.const %v128 %t end} ]} \
";

static const char *ternary_op_wasm_spec = "wasm \
    types {[ fn [] [v128] ]} \
    funcs {[ 0 ]} \
    code {[ {[] v128.const %v128 v128.const %v128 v128.const %v128 %t end} ]} \
";

void run_simd_v128_op_test(const char* test_name, const wah_v128_t* expected_val, const char *spec, ...) {
    wah_module_t module;
    wah_exec_context_t ctx;

    va_list args;
    va_start(args, spec);
    wah_test_setup_contextv(test_name, spec, &module, &ctx, args);
    va_end(args);

    wah_value_t result;
    wah_test_execute_function(&ctx, &result);
    compare_and_print_v128_result(test_name, &result.v128, (const wah_v128_t*)expected_val);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

#define DEFINE_OPCODE() \
    const char *opcode_end = strchr(test_name, ' '); \
    const size_t opcode_len = opcode_end ? (size_t)(opcode_end - test_name) : strlen(test_name); \
    char opcode[64]; \
    if (opcode_len >= 64) abort(); \
    memcpy(opcode, test_name, opcode_len); \
    opcode[opcode_len] = '\0'

void run_simd_nullary_op_test(const char* test_name, const char *spec, const wah_v128_t* expected_val) {
    DEFINE_OPCODE();
    run_simd_v128_op_test(test_name, expected_val, spec, opcode);
}

void run_simd_unary_op_test(const char* test_name, const char *spec, const wah_v128_t* operand1, const wah_v128_t* expected_val) {
    DEFINE_OPCODE();
    run_simd_v128_op_test(test_name, expected_val, spec, operand1, opcode);
}

void run_simd_binary_op_test(const char* test_name, const char *spec, const wah_v128_t* operand1, const wah_v128_t* operand2, const wah_v128_t* expected_val) {
    DEFINE_OPCODE();
    run_simd_v128_op_test(test_name, expected_val, spec, operand1, operand2, opcode);
}

void run_simd_ternary_op_test(const char* test_name, const char* spec, const wah_v128_t* operand1, const wah_v128_t* operand2, const wah_v128_t* operand3, const wah_v128_t* expected_val) {
    DEFINE_OPCODE();
    run_simd_v128_op_test(test_name, expected_val, spec, operand1, operand2, operand3, opcode);
}

void run_simd_extract_lane_test(const char* test_name, const char* spec, const wah_v128_t* operand, const wah_value_t* expected_val, wah_type_t expected_type) {
    wah_module_t module;
    wah_exec_context_t ctx;

    wah_test_setup_context(test_name, spec, &module, &ctx, operand);

    wah_value_t result;
    wah_test_execute_function(&ctx, &result);
    switch (expected_type) {
        case WAH_TYPE_I32: assert_eq_i32(result.i32, expected_val->i32); break;
        case WAH_TYPE_I64: assert_eq_i64(result.i64, expected_val->i64); break;
        case WAH_TYPE_F32: assert_eq_f32(result.f32, expected_val->f32, 0); break;
        case WAH_TYPE_F64: assert_eq_f64(result.f64, expected_val->f64, 0); break;
        default: abort();
    }

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

void run_simd_splat_test(const char* test_name, const char* spec, const wah_value_t* operand_scalar, const wah_v128_t* expected_val) {
    DEFINE_OPCODE();
    run_simd_v128_op_test(test_name, expected_val, spec, operand_scalar, opcode);
}

void test_v128_load_store() {
    wah_v128_t expected_v128_val = {{0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}};
    run_simd_v128_op_test("v128.load and v128.store", &expected_v128_val, "wasm \
        types {[ fn [] [v128] ]} \
        funcs {[ 0 ]} \
        memories {[ limits.i32/1 1 ]} \
        code {[ \
            {[] \
                i32.const 0 \
                v128.const %'112233445566778899aabbccddeeff00' \
                v128.store align=1 offset=0x1234 \
                i32.const 0 \
                v128.load align=1 offset=0x1233 \
            end} \
        ]}");
}

void test_v128_const() {
    wah_v128_t expected_v128_val = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10}};
    run_simd_v128_op_test("v128.const", &expected_v128_val, "wasm \
        types {[ fn [] [v128] ]} \
        funcs {[ 0 ]} \
        code {[ {[] v128.const %'0102030405060708090a0b0c0d0e0f10' end} ]} \
    ");
}

void test_all_v128_loads() {
    const char *load_test_wasm_spec = "wasm \
        types {[ fn [] [v128] ]} \
        funcs {[ 0 ]} \
        memories {[ limits.i32/1 1 ]} \
        code {[ {[] i32.const 0 %t align=1 offset=0 end} ]} \
        data {[ data.active.table#0 i32.const 0 end {%'0182038405860788098A0B8C0D8E0F90'} ]}";

    // v128.load8x8_s: 0x01, 0x82(-126), 0x03, 0x84(-124), 0x05, 0x86(-122), 0x07, 0x88(-120) sign-extended to 16-bit
    wah_v128_t expected_8x8_s = {{0x01, 0x00, 0x82, 0xFF, 0x03, 0x00, 0x84, 0xFF, 0x05, 0x00, 0x86, 0xFF, 0x07, 0x00, 0x88, 0xFF}};
    run_simd_nullary_op_test("v128.load8x8_s", load_test_wasm_spec, &expected_8x8_s);

    // v128.load8x8_u: 0x01, 0x82, 0x03, 0x84, 0x05, 0x86, 0x07, 0x88
    wah_v128_t expected_8x8_u = {{0x01, 0x00, 0x82, 0x00, 0x03, 0x00, 0x84, 0x00, 0x05, 0x00, 0x86, 0x00, 0x07, 0x00, 0x88, 0x00}};
    run_simd_nullary_op_test("v128.load8x8_u", load_test_wasm_spec, &expected_8x8_u);

    // v128.load16x4_s: 0x8201, 0x8403, 0x8605, 0x8807 sign-extended to 32-bit
    wah_v128_t expected_16x4_s = {{0x01, 0x82, 0xFF, 0xFF, 0x03, 0x84, 0xFF, 0xFF, 0x05, 0x86, 0xFF, 0xFF, 0x07, 0x88, 0xFF, 0xFF}};
    run_simd_nullary_op_test("v128.load16x4_s", load_test_wasm_spec, &expected_16x4_s);

    // v128.load16x4_u:  0x8201, 0x8403, 0x8605, 0x8807 zero-extended to 32-bit
    wah_v128_t expected_16x4_u = {{0x01, 0x82, 0x00, 0x00, 0x03, 0x84, 0x00, 0x00, 0x05, 0x86, 0x00, 0x00, 0x07, 0x88, 0x00, 0x00}};
    run_simd_nullary_op_test("v128.load16x4_u", load_test_wasm_spec, &expected_16x4_u);

    // v128.load32x2_s: 0x84038201, 0x88078605 sign-extended to 64-bit
    wah_v128_t expected_32x2_s = {{0x01, 0x82, 0x03, 0x84, 0xFF, 0xFF, 0xFF, 0xFF, 0x05, 0x86, 0x07, 0x88, 0xFF, 0xFF, 0xFF, 0xFF}};
    run_simd_nullary_op_test("v128.load32x2_s", load_test_wasm_spec, &expected_32x2_s);

    // v128.load32x2_u: 0x84038201, 0x88078605 zero-extended to 64-bit
    wah_v128_t expected_32x2_u = {{0x01, 0x82, 0x03, 0x84, 0x00, 0x00, 0x00, 0x00, 0x05, 0x86, 0x07, 0x88, 0x00, 0x00, 0x00, 0x00}};
    run_simd_nullary_op_test("v128.load32x2_u", load_test_wasm_spec, &expected_32x2_u);

    // v128.load8_splat: 0x01 splatted
    wah_v128_t expected_8_splat = {{0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01}};
    run_simd_nullary_op_test("v128.load8_splat", load_test_wasm_spec, &expected_8_splat);

    // v128.load16_splat: 0x8201 splatted
    wah_v128_t expected_16_splat = {{0x01, 0x82, 0x01, 0x82, 0x01, 0x82, 0x01, 0x82, 0x01, 0x82, 0x01, 0x82, 0x01, 0x82, 0x01, 0x82}};
    run_simd_nullary_op_test("v128.load16_splat", load_test_wasm_spec, &expected_16_splat);

    // v128.load32_splat: 0x84038201 splatted
    wah_v128_t expected_32_splat = {{0x01, 0x82, 0x03, 0x84, 0x01, 0x82, 0x03, 0x84, 0x01, 0x82, 0x03, 0x84, 0x01, 0x82, 0x03, 0x84}};
    run_simd_nullary_op_test("v128.load32_splat", load_test_wasm_spec, &expected_32_splat);

    // v128.load64_splat: 0x8807860584038201 splatted
    wah_v128_t expected_64_splat = {{0x01, 0x82, 0x03, 0x84, 0x05, 0x86, 0x07, 0x88, 0x01, 0x82, 0x03, 0x84, 0x05, 0x86, 0x07, 0x88}};
    run_simd_nullary_op_test("v128.load64_splat", load_test_wasm_spec, &expected_64_splat);

    // v128.load32_zero: 0x84038201, rest zero
    wah_v128_t expected_32_zero = {{0x01, 0x82, 0x03, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    run_simd_nullary_op_test("v128.load32_zero", load_test_wasm_spec, &expected_32_zero);

    // v128.load64_zero: 0x8807860584038201, rest zero
    wah_v128_t expected_64_zero = {{0x01, 0x82, 0x03, 0x84, 0x05, 0x86, 0x07, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    run_simd_nullary_op_test("v128.load64_zero", load_test_wasm_spec, &expected_64_zero);

    const char *load_lane_test_wasm_spec = "wasm \
        types {[ fn [] [v128] ]} \
        funcs {[ 0 ]} \
        memories {[ limits.i32/1 1 ]} \
        code {[ \
            {[] \
                i32.const 0 \
                v128.const %'00000000000000000000000000000000' \
                %t align=1 offset=0 %'00' \
            end} \
        ]} \
        data {[ \
            data.active.table#0 i32.const 0 end {%'0182038405860788098A0B8C0D8E0F90'} \
        ]}";

    // v128.load8_lane: initial all zeros, lane 0 gets 0x01
    wah_v128_t expected_8_lane = {{0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    run_simd_nullary_op_test("v128.load8_lane", load_lane_test_wasm_spec, &expected_8_lane);

    // v128.load16_lane: initial all zeros, lane 0 gets 0x8201
    wah_v128_t expected_16_lane = {{0x01, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    run_simd_nullary_op_test("v128.load16_lane", load_lane_test_wasm_spec, &expected_16_lane);

    // v128.load32_lane: initial all zeros, lane 0 gets 0x84038201
    wah_v128_t expected_32_lane = {{0x01, 0x82, 0x03, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    run_simd_nullary_op_test("v128.load32_lane", load_lane_test_wasm_spec, &expected_32_lane);

    // v128.load64_lane: initial all zeros, lane 0 gets 0x8807860584038201
    wah_v128_t expected_64_lane = {{0x01, 0x82, 0x03, 0x84, 0x05, 0x86, 0x07, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    run_simd_nullary_op_test("v128.load64_lane", load_lane_test_wasm_spec, &expected_64_lane);
}

void test_v128_not() {
    wah_v128_t operand = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10}};
    wah_v128_t expected = {{0xFE, 0xFD, 0xFC, 0xFB, 0xFA, 0xF9, 0xF8, 0xF7, 0xF6, 0xF5, 0xF4, 0xF3, 0xF2, 0xF1, 0xF0, 0xEF}};
    run_simd_unary_op_test("v128.not", unary_op_wasm_spec, &operand, &expected);
}

void test_v128_bitselect() {
    wah_v128_t v1 = {{0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0}};
    wah_v128_t v2 = {{0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F}};
    wah_v128_t v3 = {{0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}};
    wah_v128_t expected = {{0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5}};
    run_simd_ternary_op_test("v128.bitselect", ternary_op_wasm_spec, &v1, &v2, &v3, &expected);
}

void run_simd_any_true_test(const char* test_name, const char* spec, const wah_v128_t* operand, int32_t expected_val) {
    wah_module_t module;
    wah_exec_context_t ctx;

    wah_test_setup_context(test_name, spec, &module, &ctx, operand);

    wah_value_t result;
    wah_test_execute_function(&ctx, &result);
    assert_eq_i32(result.i32, expected_val);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

void test_v128_any_true() {
    const char *v128_any_true_wasm_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] v128.const %v128 v128.any_true end} ]} \
    ";

    wah_v128_t operand_true = {{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    wah_v128_t operand_false = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    run_simd_any_true_test("v128.any_true (true)", v128_any_true_wasm_spec, &operand_true, 1);
    run_simd_any_true_test("v128.any_true (false)", v128_any_true_wasm_spec, &operand_false, 0);
}

void test_i8x16_add() {
    wah_v128_t operand1 = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10}};
    wah_v128_t operand2 = {{0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01}};
    wah_v128_t expected = {{0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11}};
    run_simd_binary_op_test("i8x16.add", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_f32x4_add() {
    wah_v128_t operand1 = { .f32 = {1.0f, 2.0f, 3.0f, 4.0f} }, operand2 = { .f32 = {5.0, 6.0, 7.0, 8.0} };
    wah_v128_t expected = { .f32 = {6.0f, 8.0f, 10.0f, 12.0f} };
    run_simd_binary_op_test("f32x4.add", binary_op_wasm_spec, &operand1, &operand2, &expected);

    wah_v128_t nan_op1 = { .u32 = {0x7fa00000, 0x7fa00000, 0x3f800000, 0x7fa00000} };
    wah_v128_t nan_op2 = { .u32 = {0x7fa00000, 0x3f800000, 0x7fa00000, 0x7fa00000} };
    wah_v128_t nan_expected = { .u32 = {0x7fc00000, 0x7fc00000, 0x7fc00000, 0x7fc00000} };
    run_simd_binary_op_test("f32x4.add (nan canonicalization)", binary_op_wasm_spec, &nan_op1, &nan_op2, &nan_expected);
}

void test_v128_and() {
    wah_v128_t operand1 = {{0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00}};
    wah_v128_t operand2 = {{0xF0, 0xF0, 0x0F, 0x0F, 0xAA, 0x55, 0xAA, 0x55, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80}};
    wah_v128_t expected = {{0xF0, 0x00, 0x0F, 0x00, 0xAA, 0x00, 0xAA, 0x00, 0x01, 0x00, 0x04, 0x00, 0x10, 0x00, 0x40, 0x00}};
    run_simd_binary_op_test("v128.and", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_v128_andnot() {
    wah_v128_t operand1 = {{0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00}};
    wah_v128_t operand2 = {{0xF0, 0xF0, 0x0F, 0x0F, 0xAA, 0x55, 0xAA, 0x55, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80}};
    wah_v128_t expected = {{0x0F, 0x00, 0xF0, 0x00, 0x55, 0x00, 0x55, 0x00, 0xFE, 0x00, 0xFB, 0x00, 0xEF, 0x00, 0xBF, 0x00}};
    run_simd_binary_op_test("v128.andnot", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_v128_or() {
    wah_v128_t operand1 = {{0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00}};
    wah_v128_t operand2 = {{0xF0, 0xF0, 0x0F, 0x0F, 0xAA, 0x55, 0xAA, 0x55, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80}};
    wah_v128_t expected = {{0xFF, 0xF0, 0xFF, 0x0F, 0xFF, 0x55, 0xFF, 0x55, 0xFF, 0x02, 0xFF, 0x08, 0xFF, 0x20, 0xFF, 0x80}};
    run_simd_binary_op_test("v128.or", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_v128_xor() {
    wah_v128_t operand1 = {{0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00}};
    wah_v128_t operand2 = {{0xF0, 0xF0, 0x0F, 0x0F, 0xAA, 0x55, 0xAA, 0x55, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80}};
    wah_v128_t expected = {{0x0F, 0xF0, 0xF0, 0x0F, 0x55, 0x55, 0x55, 0x55, 0xFE, 0x02, 0xFB, 0x08, 0xEF, 0x20, 0xBF, 0x80}};
    run_simd_binary_op_test("v128.xor", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i8x16_add_sat_s() {
    wah_v128_t operand1 = {{0x01, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    wah_v128_t operand2 = {{0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    wah_v128_t expected = {{0x02, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    run_simd_binary_op_test("i8x16.add_sat_s", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i8x16_add_sat_u() {
    wah_v128_t operand1 = {{0x01, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    wah_v128_t operand2 = {{0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    wah_v128_t expected = {{0x02, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    run_simd_binary_op_test("i8x16.add_sat_u", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i8x16_sub() {
    wah_v128_t operand1 = {{0x05, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    wah_v128_t operand2 = {{0x02, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    wah_v128_t expected = {{0x03, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    run_simd_binary_op_test("i8x16.sub", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i8x16_sub_sat_s() {
    wah_v128_t operand1 = {{0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    wah_v128_t operand2 = {{0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    wah_v128_t expected = {{0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    run_simd_binary_op_test("i8x16.sub_sat_s", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i8x16_sub_sat_u() {
    wah_v128_t operand1 = {{0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    wah_v128_t operand2 = {{0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    wah_v128_t expected = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    run_simd_binary_op_test("i8x16.sub_sat_u", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i16x8_sub() {
    wah_v128_t operand1 = {{0x01, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 0x0001, 0x0005
    wah_v128_t operand2 = {{0x02, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 0x0002, 0x0003
    wah_v128_t expected = {{0xFF, 0xFF, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 0xFFFF (-1), 0x0002
    run_simd_binary_op_test("i16x8.sub", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i32x4_mul() {
    wah_v128_t operand1 = {{0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 2, 3
    wah_v128_t operand2 = {{0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 3, 4
    wah_v128_t expected = {{0x06, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 6, 12
    run_simd_binary_op_test("i32x4.mul", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i16x8_eq() {
    wah_v128_t operand1 = {{0x01, 0x00, 0x02, 0x00, 0x03, 0x00, 0x04, 0x00, 0x05, 0x00, 0x06, 0x00, 0x07, 0x00, 0x08, 0x00}};
    wah_v128_t operand2 = {{0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00}};
    wah_v128_t expected = {{0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00}};
    run_simd_binary_op_test("i16x8.eq", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i16x8_ne() {
    wah_v128_t operand1 = {{0x01, 0x00, 0x02, 0x00, 0x03, 0x00, 0x04, 0x00, 0x05, 0x00, 0x06, 0x00, 0x07, 0x00, 0x08, 0x00}};
    wah_v128_t operand2 = {{0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00}};
    wah_v128_t expected = {{0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF}};
    run_simd_binary_op_test("i16x8.ne", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i16x8_lt_s() {
    wah_v128_t operand1 = {{0x01, 0x00, 0x02, 0x00, 0x00, 0x80, 0xFF, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 1, 2, -32768, 32767
    wah_v128_t operand2 = {{0x02, 0x00, 0x01, 0x00, 0xFF, 0x7F, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 2, 1, 32767, -32768
    wah_v128_t expected = {{0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 1<2, 2<1(F), -32768<32767, 32767<-32768(F)
    run_simd_binary_op_test("i16x8.lt_s", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i16x8_lt_u() {
    wah_v128_t operand1 = {{0x01, 0x00, 0x02, 0x00, 0x00, 0x80, 0xFF, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 1, 2, 32768, 32767
    wah_v128_t operand2 = {{0x02, 0x00, 0x01, 0x00, 0xFF, 0x7F, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 2, 1, 32767, 32768
    wah_v128_t expected = {{0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 1<2, 2<1(F), 32768<32767(F), 32767<32768
    run_simd_binary_op_test("i16x8.lt_u", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i16x8_gt_s() {
    wah_v128_t operand1 = {{0x02, 0x00, 0x01, 0x00, 0xFF, 0x7F, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 2, 1, 32767, -32768
    wah_v128_t operand2 = {{0x01, 0x00, 0x02, 0x00, 0x00, 0x80, 0xFF, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 1, 2, -32768, 32767
    wah_v128_t expected = {{0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 2>1, 1>2(F), 32767>-32768, -32768>32767(F)
    run_simd_binary_op_test("i16x8.gt_s", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i16x8_gt_u() {
    wah_v128_t operand1 = {{0x02, 0x00, 0x01, 0x00, 0xFF, 0x7F, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 2, 1, 32767, 32768
    wah_v128_t operand2 = {{0x01, 0x00, 0x02, 0x00, 0x00, 0x80, 0xFF, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 1, 2, 32768, 32767
    wah_v128_t expected = {{0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 2>1, 1>2(F), 32767>32768(F), 32768>32767
    run_simd_binary_op_test("i16x8.gt_u", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i16x8_le_s() {
    wah_v128_t operand1 = {{0x01, 0x00, 0x02, 0x00, 0x00, 0x80, 0xFF, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 1, 2, -32768, 32767
    wah_v128_t operand2 = {{0x01, 0x00, 0x01, 0x00, 0x00, 0x80, 0xFF, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 1, 1, -32768, 32767
    wah_v128_t expected = {{0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}}; // 1<=1, 2<=1(F), -32768<=-32768, 32767<=32767, 0<=0 (remaining 4 lanes)
    run_simd_binary_op_test("i16x8.le_s", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i16x8_le_u() {
    wah_v128_t operand1 = {{0x01, 0x00, 0x02, 0x00, 0x00, 0x80, 0xFF, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 1, 2, 32768, 32767
    wah_v128_t operand2 = {{0x01, 0x00, 0x01, 0x00, 0x00, 0x80, 0xFF, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 1, 1, 32768, 32767
    wah_v128_t expected = {{0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}}; // 1<=1, 2<=1(F), 32768<=32768, 32767<=32767, 0<=0 (remaining 4 lanes)
    run_simd_binary_op_test("i16x8.le_u", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i16x8_ge_s() {
    wah_v128_t operand1 = {{0x01, 0x00, 0x02, 0x00, 0x00, 0x80, 0xFF, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 1, 2, -32768, 32767
    wah_v128_t operand2 = {{0x01, 0x00, 0x01, 0x00, 0x00, 0x80, 0xFF, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 1, 1, -32768, 32767
    wah_v128_t expected = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}}; // 1>=1, 2>=1, -32768>=-32768, 32767>=32767, 0>=0 (remaining 4 lanes)
    run_simd_binary_op_test("i16x8.ge_s", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i16x8_ge_u() {
    wah_v128_t operand1 = {{0x01, 0x00, 0x02, 0x00, 0x00, 0x80, 0xFF, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 1, 2, 32768, 32767
    wah_v128_t operand2 = {{0x01, 0x00, 0x01, 0x00, 0x00, 0x80, 0xFF, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 1, 1, 32768, 32767
    wah_v128_t expected = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}}; // 1>=1, 2>=1, 32768>=32768, 32767>=32767, 0>=0 (remaining 4 lanes)
    run_simd_binary_op_test("i16x8.ge_u", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i8x16_eq() {
    wah_v128_t operand1 = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10}};
    wah_v128_t operand2 = {{0x01, 0x00, 0x03, 0x00, 0x05, 0x00, 0x07, 0x00, 0x09, 0x00, 0x0B, 0x00, 0x0D, 0x00, 0x0F, 0x00}};
    wah_v128_t expected = {{0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00}};
    run_simd_binary_op_test("i8x16.eq", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i8x16_ne() {
    wah_v128_t operand1 = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10}};
    wah_v128_t operand2 = {{0x01, 0x00, 0x03, 0x00, 0x05, 0x00, 0x07, 0x00, 0x09, 0x00, 0x0B, 0x00, 0x0D, 0x00, 0x0F, 0x00}};
    wah_v128_t expected = {{0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF}};
    run_simd_binary_op_test("i8x16.ne", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i8x16_lt_s() {
    wah_v128_t operand1 = {{0x01, 0x02, 0x80, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 1, 2, -128, 127
    wah_v128_t operand2 = {{0x02, 0x01, 0x7F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 2, 1, 127, -128
    wah_v128_t expected = {{0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 1<2, 2<1(F), -128<127, 127<-128(F)
    run_simd_binary_op_test("i8x16.lt_s", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i8x16_lt_u() {
    wah_v128_t operand1 = {{0x01, 0x02, 0x80, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 1, 2, 128, 127
    wah_v128_t operand2 = {{0x02, 0x01, 0x7F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 2, 1, 127, 128
    wah_v128_t expected = {{0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 1<2, 2<1(F), 128<127(F), 127<128
    run_simd_binary_op_test("i8x16.lt_u", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i8x16_gt_s() {
    wah_v128_t operand1 = {{0x02, 0x01, 0x7F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 2, 1, 127, -128
    wah_v128_t operand2 = {{0x01, 0x02, 0x80, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 1, 2, -128, 127
    wah_v128_t expected = {{0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 2>1, 1>2(F), 127>-128, -128>127(F)
    run_simd_binary_op_test("i8x16.gt_s", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i8x16_gt_u() {
    wah_v128_t operand1 = {{0x02, 0x01, 0x7F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 2, 1, 127, 128
    wah_v128_t operand2 = {{0x01, 0x02, 0x80, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 1, 2, 128, 127
    wah_v128_t expected = {{0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 2>1, 1>2(F), 127>128(F), 128>127
    run_simd_binary_op_test("i8x16.gt_u", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i8x16_le_s() {
    wah_v128_t operand1 = {{0x01, 0x02, 0x80, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 1, 2, -128, 127
    wah_v128_t operand2 = {{0x01, 0x01, 0x80, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 1, 1, -128, 127
    wah_v128_t expected = {{0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}}; // 1<=1, 2<=1(F), -128<=-128, 127<=127, 0<=0 (remaining 12 bytes)
    run_simd_binary_op_test("i8x16.le_s", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i8x16_le_u() {
    wah_v128_t operand1 = {{0x01, 0x02, 0x80, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 1, 2, 128, 127
    wah_v128_t operand2 = {{0x01, 0x01, 0x80, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 1, 1, 128, 127
    wah_v128_t expected = {{0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}}; // 1<=1, 2<=1(F), 128<=128, 127<=127, 0<=0 (remaining 12 bytes)
    run_simd_binary_op_test("i8x16.le_u", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i8x16_ge_s() {
    wah_v128_t operand1 = {{0x01, 0x02, 0x80, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 1, 2, -128, 127
    wah_v128_t operand2 = {{0x01, 0x01, 0x80, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 1, 1, -128, 127
    wah_v128_t expected = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}}; // 1>=1, 2>=1, -128>=-128, 127>=127, 0>=0 (remaining 12 bytes)
    run_simd_binary_op_test("i8x16.ge_s", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i8x16_ge_u() {
    wah_v128_t operand1 = {{0x01, 0x02, 0x80, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 1, 2, 128, 127
    wah_v128_t operand2 = {{0x01, 0x01, 0x80, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 1, 1, 128, 127
    wah_v128_t expected = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}}; // 1>=1, 2>=1, 128>=128, 127>=127, 0>=0 (remaining 12 bytes)
    run_simd_binary_op_test("i8x16.ge_u", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i32x4_eq() {
    wah_v128_t operand1 = { .i32 = {1, 2, 3, 4} }, operand2 = { .i32 = {1, 0, 3, 0} }, expected = { .u32 = {~0U, 0, ~0U, 0} };
    run_simd_binary_op_test("i32x4.eq", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i32x4_ne() {
    wah_v128_t operand1 = { .i32 = {1, 2, 3, 4} }, operand2 = { .i32 = {1, 0, 3, 0} }, expected = { .u32 = {0, ~0U, 0, ~0U} };
    run_simd_binary_op_test("i32x4.ne", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i32x4_lt_s() {
    wah_v128_t operand1 = { .i32 = {1, 2, -2147483648, 2147483647} }, operand2 = { .i32 = {2, 1, 2147483647, -2147483648} };
    wah_v128_t expected = { .u32 = {~0U, 0, ~0U, 0} }; // 1<2, 2<1(F), -2147483648<2147483647, 2147483647<-2147483648(F)
    run_simd_binary_op_test("i32x4.lt_s", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i32x4_lt_u() {
    wah_v128_t operand1 = { .u32 = {1, 2, 2147483648U, 2147483647U} }, operand2 = { .u32 = {2, 1, 2147483647U, 2147483648U} };
    wah_v128_t expected = { .u32 = {~0U, 0, 0, ~0U} }; // 1<2, 2<1(F), 2147483648<2147483647(F), 2147483647<2147483648
    run_simd_binary_op_test("i32x4.lt_u", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i32x4_gt_s() {
    wah_v128_t operand1 = { .i32 = {2, 1, 2147483647, -2147483648} }, operand2 = { .i32 = {1, 2, -2147483648, 2147483647} };
    wah_v128_t expected = { .u32 = {~0U, 0, ~0U, 0} }; // 2>1, 1>2(F), 2147483647>-2147483648, -2147483648>2147483647(F)
    run_simd_binary_op_test("i32x4.gt_s", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i32x4_gt_u() {
    wah_v128_t operand1 = { .u32 = {2, 1, 2147483647U, 2147483648U} }, operand2 = { .u32 = {1, 2, 2147483648U, 2147483647U} };
    wah_v128_t expected = { .u32 = {~0U, 0, 0, ~0U} }; // 2>1, 1>2(F), 2147483647>2147483648(F), 2147483648>2147483647
    run_simd_binary_op_test("i32x4.gt_u", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i32x4_le_s() {
    wah_v128_t operand1 = { .i32 = {1, 2, -2147483648, 2147483647} }, operand2 = { .i32 = {1, 1, -2147483648, 2147483647} };
    wah_v128_t expected = { .u32 = {~0U, 0, ~0U, ~0U} }; // 1<=1, 2<=1(F), -2147483648<=-2147483648, 2147483647<=2147483647
    run_simd_binary_op_test("i32x4.le_s", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i32x4_le_u() {
    wah_v128_t operand1 = { .u32 = {1, 2, 2147483648U, 2147483647U} }, operand2 = { .u32 = {1, 1, 2147483648U, 2147483647U} };
    wah_v128_t expected = { .u32 = {~0U, 0, ~0U, ~0U} }; // 1<=1, 2<=1(F), 2147483648<=2147483648, 2147483647<=2147483647
    run_simd_binary_op_test("i32x4.le_u", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i32x4_ge_s() {
    wah_v128_t operand1 = { .i32 = {1, 2, -2147483648, 2147483647} }, operand2 = { .i32 = {1, 1, -2147483648, 2147483647} };
    wah_v128_t expected = { .u32 = {~0U, ~0U, ~0U, ~0U} }; // 1>=1, 2>=1, -2147483648>=-2147483648, 2147483647>=2147483647
    run_simd_binary_op_test("i32x4.ge_s", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i32x4_ge_u() {
    wah_v128_t operand1 = { .u32 = {1, 2, 2147483648U, 2147483647U} }, operand2 = { .u32 = {1, 1, 2147483648U, 2147483647U} };
    wah_v128_t expected = { .u32 = {~0U, ~0U, ~0U, ~0U} }; // 1>=1, 2>=1, 2147483648>=2147483648, 2147483647>=2147483647
    run_simd_binary_op_test("i32x4.ge_u", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i64x2_eq() {
    wah_v128_t operand1 = { .i64 = {1, 2} }, operand2 = { .i64 = {1, 0} }, expected = { .u64 = {~0ULL, 0} };
    run_simd_binary_op_test("i64x2.eq", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i64x2_ne() {
    wah_v128_t operand1 = { .i64 = {1, 2} }, operand2 = { .i64 = {1, 0} }, expected = { .u64 = {0, ~0ULL} };
    run_simd_binary_op_test("i64x2.ne", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i64x2_lt_s() {
    wah_v128_t operand1 = { .i64 = {1, 2} }, operand2 = { .i64 = {2, 1} }, expected = { .u64 = {~0ULL, 0} }; // 1<2, 2<1(F)
    run_simd_binary_op_test("i64x2.lt_s", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i64x2_gt_s() {
    wah_v128_t operand1 = { .i64 = {2, 1} }, operand2 = { .i64 = {1, 2} }, expected = { .u64 = {~0ULL, 0} }; // 2>1, 1>2(F)
    run_simd_binary_op_test("i64x2.gt_s", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i64x2_le_s() {
    wah_v128_t operand1 = { .i64 = {1, 2} }, operand2 = { .i64 = {1, 2} }, expected = { .u64 = {~0ULL, ~0ULL} }; // 1<=1, 2<=2
    run_simd_binary_op_test("i64x2.le_s", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i64x2_ge_s() {
    wah_v128_t operand1 = { .i64 = {1, 2} }, operand2 = { .i64 = {1, 2} }, expected = { .u64 = {~0ULL, ~0ULL} }; // 1>=1, 2>=2
    run_simd_binary_op_test("i64x2.ge_s", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_f32x4_eq() {
    wah_v128_t operand1 = { .f32 = {1.0f, 2.0f, 3.0f, 4.0f} }, operand2 = { .f32 = {1.0f, 0.0f, 3.0f, 0.0f} };
    wah_v128_t expected = { .u32 = {~0U, 0, ~0U, 0} };
    run_simd_binary_op_test("f32x4.eq", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_f32x4_ne() {
    wah_v128_t operand1 = { .f32 = {1.0f, 2.0f, 3.0f, 4.0f} }, operand2 = { .f32 = {1.0f, 0.0f, 3.0f, 0.0f} };
    wah_v128_t expected = { .u32 = {0, ~0U, 0, ~0U} };
    run_simd_binary_op_test("f32x4.ne", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_f32x4_lt() {
    wah_v128_t operand1 = { .f32 = {1.0f, 2.0f, 3.0f, 4.0f} }, operand2 = { .f32 = {2.0f, 1.0f, 3.0f, 5.0f} };
    wah_v128_t expected = { .u32 = {~0U, 0, 0, ~0U} }; // 1<2, 2<1(F), 3<3(F), 4<5
    run_simd_binary_op_test("f32x4.lt", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_f32x4_gt() {
    wah_v128_t operand1 = { .f32 = {2.0f, 1.0f, 3.0f, 5.0f} }, operand2 = { .f32 = {1.0f, 2.0f, 3.0f, 4.0f} };
    wah_v128_t expected = { .u32 = {~0U, 0, 0, ~0U} }; // 2>1, 1>2(F), 3>3(F), 5>4
    run_simd_binary_op_test("f32x4.gt", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_f32x4_le() {
    wah_v128_t operand1 = { .f32 = {1.0f, 2.0f, 3.0f, 4.0f} }, operand2 = { .f32 = {1.0f, 1.0f, 3.0f, 5.0f} };
    wah_v128_t expected = { .u32 = {~0U, 0, ~0U, ~0U} }; // 1<=1, 2<=1(F), 3<=3, 4<=5
    run_simd_binary_op_test("f32x4.le", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_f32x4_ge() {
    wah_v128_t operand1 = { .f32 = {1.0f, 2.0f, 3.0f, 5.0f} }, operand2 = { .f32 = {1.0f, 1.0f, 3.0f, 4.0f} };
    wah_v128_t expected = { .u32 = {~0U, ~0U, ~0U, ~0U} }; // 1>=1, 2>=1, 3>=3, 5>=4
    run_simd_binary_op_test("f32x4.ge", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_f64x2_eq() {
    wah_v128_t operand1 = { .f64 = {1.0, 2.0} }, operand2 = { .f64 = {1.0, 0.0} }, expected = { .u64 = {~0ULL, 0} };
    run_simd_binary_op_test("f64x2.eq", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_f64x2_ne() {
    wah_v128_t operand1 = { .f64 = {1.0, 2.0} }, operand2 = { .f64 = {1.0, 0.0} }, expected = { .u64 = {0, ~0ULL} };
    run_simd_binary_op_test("f64x2.ne", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_f64x2_lt() {
    wah_v128_t operand1 = { .f64 = {1.0, 2.0} }, operand2 = { .f64 = {2.0, 1.0} }, expected = { .u64 = {~0ULL, 0} }; // 1<2, 2<1(F)
    run_simd_binary_op_test("f64x2.lt", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_f64x2_gt() {
    wah_v128_t operand1 = { .f64 = {2.0, 1.0} }, operand2 = { .f64 = {1.0, 2.0} }, expected = { .u64 = {~0ULL, 0} }; // 2>1, 1>2(F)
    run_simd_binary_op_test("f64x2.gt", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_f64x2_le() {
    wah_v128_t operand1 = { .f64 = {1.0, 2.0} }, operand2 = { .f64 = {1.0, 1.0} }, expected = { .u64 = {~0ULL, 0} }; // 1<=1, 2<=1(F)
    run_simd_binary_op_test("f64x2.le", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_f64x2_ge() {
    wah_v128_t operand1 = { .f64 = {1.0, 2.0} }, operand2 = { .f64 = {1.0, 1.0} }, expected = { .u64 = {~0ULL, ~0ULL} }; // 1>=1, 2>=1
    run_simd_binary_op_test("f64x2.ge", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i8x16_shuffle() {
    // mask: 0, 1, 16, 17, 2, 3, 18, 19, 4, 5, 20, 21, 6, 7, 22, 23
    // result: vec1[0], vec1[1], vec2[0], vec2[1], vec1[2], vec1[3], vec2[2], vec2[3], ...
    wah_v128_t expected = {{0x00, 0x01, 0x10, 0x11, 0x02, 0x03, 0x12, 0x13, 0x04, 0x05, 0x14, 0x15, 0x06, 0x07, 0x16, 0x17}};
    run_simd_v128_op_test("i8x16.shuffle", &expected, "wasm \
        types {[ fn [] [v128] ]} \
        funcs {[ 0 ]} \
        code {[ \
            {[] \
                v128.const %'000102030405060708090A0B0C0D0E0F' \
                v128.const %'101112131415161718191A1B1C1D1E1F' \
                i8x16.shuffle %'00011011020312130405141506071617' \
            end} \
        ]} \
    ");
}

void test_i8x16_extract_lane_s() {
    wah_v128_t operand = {{0x00, 0x81, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F}};
    wah_value_t expected = {.i32 = -127}; // operand.i8[1] is 0x81 = -127
    run_simd_extract_lane_test("i8x16.extract_lane_s", "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] v128.const %v128 i8x16.extract_lane_s %'01' end} ]} \
    ", &operand, &expected, WAH_TYPE_I32);
}

void test_i8x16_replace_lane() {
    wah_v128_t operand_v128 = {{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F}};
    wah_value_t operand_scalar = {.i32 = 0xAA}; // Replace with 0xAA
    wah_v128_t expected = {{0x00, 0xAA, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F}};
    run_simd_v128_op_test("i8x16.replace_lane", &expected, "wasm \
        types {[ fn [] [v128] ]} \
        funcs {[ 0 ]} \
        code {[ {[] v128.const %v128 i32.const %vd32 i8x16.replace_lane %'01' end} ]} \
    ", &operand_v128, &operand_scalar);
}

void test_i8x16_swizzle() {
    // mask: 0, 2, 16, 18, 1, 3, 17, 19, 4, 5, 20, 21, 6, 7, 22, 23
    // result: data[0], data[2], 0, 0, data[1], data[3], 0, 0, ...
    wah_v128_t expected = {{0xA0, 0xA2, 0x00, 0x00, 0xA1, 0xA3, 0x00, 0x00, 0xA4, 0xA5, 0x00, 0x00, 0xA6, 0xA7, 0x00, 0x00}};
    run_simd_v128_op_test("i8x16.swizzle", &expected, "wasm \
        types {[ fn [] [v128] ]} \
        funcs {[ 0 ]} \
        code {[ \
            {[] \
                v128.const %'A0A1A2A3A4A5A6A7A8A9AAABACADAEAF' \
                v128.const %'00021012010311130405141506071617' \
                i8x16.swizzle \
            end} \
        ]} \
    ");
}

#define SPLAT_WASM_SPEC(scalarop) "wasm \
    types {[ fn [] [v128] ]} \
    funcs {[ 0 ]} \
    code {[ {[] " scalarop " %t end} ]} \
"

static const char *i32_splat_wasm_spec = SPLAT_WASM_SPEC("i32.const %vd32");
static const char *i64_splat_wasm_spec = SPLAT_WASM_SPEC("i64.const %vd64");
static const char *f32_splat_wasm_spec = SPLAT_WASM_SPEC("f32.const %vf32");
static const char *f64_splat_wasm_spec = SPLAT_WASM_SPEC("f64.const %vf64");

void test_i8x16_splat() {
    wah_value_t operand_scalar = {.i32 = 0xBE};
    wah_v128_t expected = {{0xBE, 0xBE, 0xBE, 0xBE, 0xBE, 0xBE, 0xBE, 0xBE, 0xBE, 0xBE, 0xBE, 0xBE, 0xBE, 0xBE, 0xBE, 0xBE}};
    run_simd_splat_test("i8x16.splat", i32_splat_wasm_spec, &operand_scalar, &expected);
}

void test_i16x8_splat() {
    wah_value_t operand_scalar = {.i32 = 0xBEAF};
    wah_v128_t expected = {{0xAF, 0xBE, 0xAF, 0xBE, 0xAF, 0xBE, 0xAF, 0xBE, 0xAF, 0xBE, 0xAF, 0xBE, 0xAF, 0xBE, 0xAF, 0xBE}};
    run_simd_splat_test("i16x8.splat", i32_splat_wasm_spec, &operand_scalar, &expected);
}

void test_i32x4_splat() {
    wah_value_t operand_scalar = {.i32 = 0xDEADBEEF};
    wah_v128_t expected = {{0xEF, 0xBE, 0xAD, 0xDE, 0xEF, 0xBE, 0xAD, 0xDE, 0xEF, 0xBE, 0xAD, 0xDE, 0xEF, 0xBE, 0xAD, 0xDE}};
    run_simd_splat_test("i32x4.splat", i32_splat_wasm_spec, &operand_scalar, &expected);
}

void test_i64x2_splat() {
    wah_value_t operand_scalar = {.i64 = 0xDEADBEEFCAFEBABEULL};
    wah_v128_t expected = {{0xBE, 0xBA, 0xFE, 0xCA, 0xEF, 0xBE, 0xAD, 0xDE, 0xBE, 0xBA, 0xFE, 0xCA, 0xEF, 0xBE, 0xAD, 0xDE}};
    run_simd_splat_test("i64x2.splat", i64_splat_wasm_spec, &operand_scalar, &expected);
}

void test_f32x4_splat() {
    wah_value_t operand_scalar = {.f32 = 123.456f};
    wah_v128_t expected;
    for (int i = 0; i < 4; ++i) expected.f32[i] = 123.456f;
    run_simd_splat_test("f32x4.splat", f32_splat_wasm_spec, &operand_scalar, &expected);
}

void test_f64x2_splat() {
    wah_value_t operand_scalar = {.f64 = 123.456789};
    wah_v128_t expected;
    for (int i = 0; i < 2; ++i) expected.f64[i] = 123.456789;
    run_simd_splat_test("f64x2.splat", f64_splat_wasm_spec, &operand_scalar, &expected);
}

void test_i32x4_trunc_sat_f32x4_s() {
    wah_v128_t operand = { .f32 = {1.5f, -2.5f, 2147483647.0f, -2147483648.0f} };
    wah_v128_t expected = { .i32 = {1, -2, 2147483647, -2147483648} };
    run_simd_unary_op_test("i32x4.trunc_sat_f32x4_s", unary_op_wasm_spec, &operand, &expected);
}

void test_i32x4_trunc_sat_f32x4_u() {
    wah_v128_t operand = { .f32 = {1.5f, -2.5f, 4294967295.0f, 0.0f} };
    wah_v128_t expected = { .u32 = {1, 0, 4294967295U, 0} };
    run_simd_unary_op_test("i32x4.trunc_sat_f32x4_u", unary_op_wasm_spec, &operand, &expected);
}

void test_f32x4_convert_i32x4_s() {
    wah_v128_t operand = { .i32 = {1, -2, 3, -4} };
    wah_v128_t expected = { .f32 = {1.0f, -2.0f, 3.0f, -4.0f} };
    run_simd_unary_op_test("f32x4.convert_i32x4_s", unary_op_wasm_spec, &operand, &expected);
}

void test_f32x4_convert_i32x4_u() {
    wah_v128_t operand = { .u32 = {1, 2, 3, 4} };
    wah_v128_t expected = { .f32 = {1.0f, 2.0f, 3.0f, 4.0f} };
    run_simd_unary_op_test("f32x4.convert_i32x4_u", unary_op_wasm_spec, &operand, &expected);
}

void test_i32x4_trunc_sat_f64x2_s_zero() {
    wah_v128_t operand = { .f64 = {1.5, -2.5} };
    wah_v128_t expected = { .i32 = {1, -2, 0, 0} };
    run_simd_unary_op_test("i32x4.trunc_sat_f64x2_s_zero", unary_op_wasm_spec, &operand, &expected);
}

void test_i32x4_trunc_sat_f64x2_u_zero() {
    wah_v128_t operand = { .f64 = {1.5, -2.5} };
    wah_v128_t expected = { .u32 = {1, 0, 0, 0} };
    run_simd_unary_op_test("i32x4.trunc_sat_f64x2_u_zero", unary_op_wasm_spec, &operand, &expected);
}

void test_f64x2_convert_low_i32x4_s() {
    wah_v128_t operand = { .i32 = {1, -2, 3, -4} };
    wah_v128_t expected = { .f64 = {1.0, -2.0} };
    run_simd_unary_op_test("f64x2.convert_low_i32x4_s", unary_op_wasm_spec, &operand, &expected);
}

void test_f64x2_convert_low_i32x4_u() {
    wah_v128_t operand = { .u32 = {1, 2, 3, 4} };
    wah_v128_t expected = { .f64 = {1.0, 2.0} };
    run_simd_unary_op_test("f64x2.convert_low_i32x4_u", unary_op_wasm_spec, &operand, &expected);
}

void test_f32x4_demote_f64x2_zero() {
    wah_v128_t operand = { .f64 = {1.5, 2.5} };
    wah_v128_t expected = { .f32 = {1.5f, 2.5f, 0.0f, 0.0f} };
    run_simd_unary_op_test("f32x4.demote_f64x2_zero", unary_op_wasm_spec, &operand, &expected);
}

void test_f64x2_promote_low_f32x4() {
    wah_v128_t operand = { .f32 = {1.5f, 2.5f, 3.5f, 4.5f} };
    wah_v128_t expected = { .f64 = {1.5, 2.5} };
    run_simd_unary_op_test("f64x2.promote_low_f32x4", unary_op_wasm_spec, &operand, &expected);
}

void test_i8x16_abs() {
    wah_v128_t v = {{0x01, 0xFF, 0x00, 0x7F, 0x80, 0x0A, 0xF6, 0x00, 0x01, 0xFF, 0x00, 0x7F, 0x80, 0x0A, 0xF6, 0x00}};
    wah_v128_t expected = {{0x01, 0x01, 0x00, 0x7F, 0x80, 0x0A, 0x0A, 0x00, 0x01, 0x01, 0x00, 0x7F, 0x80, 0x0A, 0x0A, 0x00}};
    run_simd_unary_op_test("i8x16.abs", unary_op_wasm_spec, &v, &expected);
}

void test_i8x16_shl() {
    const char *i8x16_shl_wasm = "wasm \
        types {[ fn [] [v128] ]} \
        funcs {[ 0 ]} \
        code {[ {[] v128.const %v128 i32.const %d32 i8x16.shl end} ]} \
    ";

    wah_v128_t v = {{0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80}};
    wah_v128_t expected = {{0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00}};
    run_simd_v128_op_test("i8x16.shl", &expected, i8x16_shl_wasm, &v, 1);

    v = (wah_v128_t){{0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01}};
    expected = (wah_v128_t){{0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}};
    run_simd_v128_op_test("i8x16.shl", &expected, i8x16_shl_wasm, &v, 2);
}

void test_i8x16_min_s() {
    wah_v128_t v1 = {{0x01, 0x05, 0x7F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x05, 0x7F, 0x80, 0x00, 0x00, 0x00, 0x00}};
    wah_v128_t v2 = {{0x02, 0x03, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00}};
    wah_v128_t expected = {{0x01, 0x03, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00}};
    run_simd_binary_op_test("i8x16.min_s", binary_op_wasm_spec, &v1, &v2, &expected);
}

void test_i8x16_avgr_u() {
    wah_v128_t v1 = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10}};
    wah_v128_t v2 = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10}};
    wah_v128_t expected = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10}};
    run_simd_binary_op_test("i8x16.avgr_u", binary_op_wasm_spec, &v1, &v2, &expected);

    v1 = (wah_v128_t){{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    v2 = (wah_v128_t){{0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01}};
    expected = (wah_v128_t){{0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01}};
    run_simd_binary_op_test("i8x16.avgr_u", binary_op_wasm_spec, &v1, &v2, &expected);
}

void test_i16x8_extend_low_i8x16_s() {
    wah_v128_t v = {{0x01, 0xFF, 0x00, 0x7F, 0x80, 0x0A, 0xF6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    wah_v128_t expected = {{0x01, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x7F, 0x00, 0x80, 0xFF, 0x0A, 0x00, 0xF6, 0xFF, 0x00, 0x00}}; // i16 values
    run_simd_unary_op_test("i16x8.extend_low_i8x16_s", unary_op_wasm_spec, &v, &expected);
}

void test_i16x8_narrow_i32x4_s() {
    wah_v128_t v1 = {{0x01, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // i32: 1, -1
    wah_v128_t v2 = {{0x00, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // i32: -32768, 65535 (saturates to 32767)
    wah_v128_t expected = {{0x01, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xFF, 0x7F, 0x00, 0x00, 0x00, 0x00}}; // i16: 1, -1, 0, 0, -32768, 32767, 0, 0
    run_simd_binary_op_test("i16x8.narrow_i32x4_s", binary_op_wasm_spec, &v1, &v2, &expected);
}

void test_i16x8_q15mulr_sat_s() {
    wah_v128_t v1 = {{0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40}}; // 0.5 in Q15
    wah_v128_t v2 = {{0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40}}; // 0.5 in Q15
    wah_v128_t expected = {{0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20}}; // 0.25 in Q15
    run_simd_binary_op_test("i16x8.q15mulr_sat_s", binary_op_wasm_spec, &v1, &v2, &expected);

    v1 = (wah_v128_t){{0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F}}; // 1.0 in Q15
    v2 = (wah_v128_t){{0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F}}; // 1.0 in Q15
    expected = (wah_v128_t){{0xFE, 0x7F, 0xFE, 0x7F, 0xFE, 0x7F, 0xFE, 0x7F, 0xFE, 0x7F, 0xFE, 0x7F, 0xFE, 0x7F, 0xFE, 0x7F}}; // 32766 in Q15 (saturated)
    run_simd_binary_op_test("i16x8.q15mulr_sat_s", binary_op_wasm_spec, &v1, &v2, &expected);
}

void test_i32x4_dot_i16x8_s() {
    wah_v128_t v1 = {{0x01, 0x00, 0x02, 0x00, 0x03, 0x00, 0x04, 0x00, 0x05, 0x00, 0x06, 0x00, 0x07, 0x00, 0x08, 0x00}}; // i16: 1, 2, 3, 4, 5, 6, 7, 8
    wah_v128_t v2 = {{0x02, 0x00, 0x03, 0x00, 0x04, 0x00, 0x05, 0x00, 0x06, 0x00, 0x07, 0x00, 0x08, 0x00, 0x09, 0x00}}; // i16: 2, 3, 4, 5, 6, 7, 8, 9
    wah_v128_t expected = {{0x08, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x48, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00}}; // i32: 8, 32, 72, 128
    run_simd_binary_op_test("i32x4.dot_i16x8_s", binary_op_wasm_spec, &v1, &v2, &expected);
}

void test_i32x4_extmul_low_i16x8_s() {
    wah_v128_t v1 = {{0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // i16: 1, 2
    wah_v128_t v2 = {{0x02, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // i16: 2, 3
    wah_v128_t expected = {{0x02, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // i32: 2, 6
    run_simd_binary_op_test("i32x4.extmul_low_i16x8_s", binary_op_wasm_spec, &v1, &v2, &expected);
}

void test_i64x2_abs() {
    wah_v128_t v = {{0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}}; // i64: 1, -1
    wah_v128_t expected = {{0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // i64: 1, 1
    run_simd_unary_op_test("i64x2.abs", unary_op_wasm_spec, &v, &expected);
}

void test_i64x2_extend_high_i32x4_u() {
    wah_v128_t v = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00}}; // u32: ?, ?, 0xFFFFFFFF, 0
    wah_v128_t expected = {{0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // u64: 0xFFFFFFFF, 0
    run_simd_unary_op_test("i64x2.extend_high_i32x4_u", unary_op_wasm_spec, &v, &expected);
}

void test_f32x4_abs() {
    wah_v128_t v = {{0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0xBF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // f32: 1.0, -1.0, 0.0
    wah_v128_t expected = {{0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // f32: 1.0, 1.0, 0.0
    run_simd_unary_op_test("f32x4.abs", unary_op_wasm_spec, &v, &expected);
}

void test_f32x4_ceil() {
    wah_v128_t v = {{0x00, 0x00, 0x20, 0x40, 0x00, 0x00, 0x20, 0xC0, 0xCD, 0xCC, 0xCC, 0x3F, 0x00, 0x00, 0x00, 0x00}}; // f32: 2.5, -2.5, 1.6
    wah_v128_t expected = {{0x00, 0x00, 0x40, 0x40, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00}}; // f32: 3.0, -2.0, 2.0
    run_simd_unary_op_test("f32x4.ceil", unary_op_wasm_spec, &v, &expected);
}

void test_f32x4_min() {
    wah_v128_t v1 = {{0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00}}; // f32: 1.0, 0.0, 1.0
    wah_v128_t v2 = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // f32: 0.0, 1.0, 0.0
    wah_v128_t expected = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // f32: 0.0, 0.0, 0.0
    run_simd_binary_op_test("f32x4.min", binary_op_wasm_spec, &v1, &v2, &expected);

    wah_v128_t nan_v1 = { .u32 = {0x7fa00000, 0x3f800000, 0x7fa00000, 0x7fa00000} };
    wah_v128_t nan_v2 = { .u32 = {0x3f800000, 0x7fa00000, 0x7fa00000, 0x3f800000} };
    wah_v128_t nan_expected = { .u32 = {0x7fc00000, 0x7fc00000, 0x7fc00000, 0x7fc00000} };
    run_simd_binary_op_test("f32x4.min (nan canonicalization)", binary_op_wasm_spec, &nan_v1, &nan_v2, &nan_expected);

    wah_v128_t pos_zero = { .f32 = {0.0f, -0.0f, 0.0f, -0.0f} };
    wah_v128_t neg_zero = { .f32 = {-0.0f, 0.0f, -0.0f, 0.0f} };
    wah_v128_t sz_expected = { .f32 = {-0.0f, -0.0f, -0.0f, -0.0f} };
    run_simd_binary_op_test("f32x4.min (signed zero)", binary_op_wasm_spec, &pos_zero, &neg_zero, &sz_expected);
}

void test_f64x2_neg() {
    wah_v128_t v = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xBF}}; // f64: 1.0, -1.0
    wah_v128_t expected = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xBF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x3F}}; // f64: -1.0, 1.0
    run_simd_unary_op_test("f64x2.neg", unary_op_wasm_spec, &v, &expected);
}

void test_f64x2_sqrt() {
    wah_v128_t v = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // f64: 4.0, 0.0
    wah_v128_t expected = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // f64: 2.0, 0.0
    run_simd_unary_op_test("f64x2.sqrt", unary_op_wasm_spec, &v, &expected);

    wah_v128_t nan_v = { .u64 = {0x7ff0000000000001ULL, 0x7ff4000000000000ULL} };
    wah_v128_t nan_expected = { .u64 = {0x7ff8000000000000ULL, 0x7ff8000000000000ULL} };
    run_simd_unary_op_test("f64x2.sqrt (nan canonicalization)", unary_op_wasm_spec, &nan_v, &nan_expected);
}

void test_i8x16_relaxed_swizzle() {
    wah_v128_t data = {{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F}};
    wah_v128_t mask = {{0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00}};
    wah_v128_t expected = {{0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00}};
    run_simd_binary_op_test("i8x16.relaxed_swizzle", binary_op_wasm_spec, &data, &mask, &expected);

    // Test with out-of-bounds mask indices (should result in 0)
    mask = (wah_v128_t){{0x10, 0x11, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D}};
    expected = (wah_v128_t){{0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D}};
    run_simd_binary_op_test("i8x16.relaxed_swizzle (oob mask)", binary_op_wasm_spec, &data, &mask, &expected);
}

void test_i32x4_relaxed_trunc_f32x4_s() {
    wah_v128_t operand = { .f32 = {1.5f, -2.5f, 2147483647.0f, -2147483648.0f} };
    wah_v128_t expected = { .i32 = {1, -2, 2147483647, -2147483648} };
    run_simd_unary_op_test("i32x4.relaxed_trunc_f32x4_s", unary_op_wasm_spec, &operand, &expected);
}

void test_i32x4_relaxed_trunc_f32x4_u() {
    wah_v128_t operand = { .f32 = {1.5f, -2.5f, 4294967295.0f, 0.0f} };
    wah_v128_t expected = { .u32 = {1, 0, 4294967295U, 0} };
    run_simd_unary_op_test("i32x4.relaxed_trunc_f32x4_u", unary_op_wasm_spec, &operand, &expected);
}

void test_f32x4_relaxed_madd() {
    wah_v128_t a = { .f32 = {1.0f, 2.0f, 3.0f, 4.0f} };
    wah_v128_t b = { .f32 = {2.0f, 3.0f, 4.0f, 5.0f} };
    wah_v128_t c = { .f32 = {0.5f, 1.0f, 1.5f, 2.0f} };
    wah_v128_t expected = { .f32 = {2.5f, 7.0f, 13.5f, 22.0f} }; // (a*b)+c
    run_simd_ternary_op_test("f32x4.relaxed_madd", ternary_op_wasm_spec, &a, &b, &c, &expected);

    wah_v128_t nan_a = { .u32 = {0x7fa00000, 0x3f800000, 0x7fa00000, 0x3f800000} };
    wah_v128_t nan_b = { .u32 = {0x3f800000, 0x7fa00000, 0x3f800000, 0x3f800000} };
    wah_v128_t nan_c = { .u32 = {0x3f800000, 0x3f800000, 0x3f800000, 0x7fa00000} };
    wah_v128_t nan_expected = { .u32 = {0x7fc00000, 0x7fc00000, 0x7fc00000, 0x7fc00000} };
    run_simd_ternary_op_test("f32x4.relaxed_madd (nan canonicalization)", ternary_op_wasm_spec, &nan_a, &nan_b, &nan_c, &nan_expected);
}

// Test case for f32x4.relaxed_madd determinism (non-FMA behavior)
void test_f32x4_relaxed_madd_determinism() {
    // Values chosen to expose FMA vs. non-FMA differences for f32
    // a = 1 + 2^-20
    // b = 1 + 2^-20
    // c = -(1 + 2^-19)
    // Mathematically: a*b+c = (1 + 2^-19 + 2^-40) - (1 + 2^-19) = 2^-40
    // Non-FMA (two roundings): fl(a*b) = 1 + 2^-19 (2^-40 is lost)
    //                          fl(fl(a*b) + c) = fl((1 + 2^-19) - (1 + 2^-19)) = 0.0f
    // FMA (one rounding): fl(a*b+c) = 2^-40

    float val_a = 1.0f + powf(2.0f, -20.0f);
    float val_c = -(1.0f + powf(2.0f, -19.0f));

    wah_v128_t a = { .f32 = {val_a, val_a, val_a, val_a} };
    wah_v128_t b = { .f32 = {val_a, val_a, val_a, val_a} }; // b is same as a
    wah_v128_t c = { .f32 = {val_c, val_c, val_c, val_c} };

    // Expected result for non-FMA behavior (separate multiply and add)
    wah_v128_t expected = { .f32 = {0.0f, 0.0f, 0.0f, 0.0f} };

    run_simd_ternary_op_test("f32x4.relaxed_madd (determinism)", ternary_op_wasm_spec, &a, &b, &c, &expected);

    // NMADD determinism: c - a*b, non-FMA behavior
    // Same values: c - a*b = -(1+2^-19) - (1+2^-20)*(1+2^-20)
    // Non-FMA: fl(a*b) = 1 + 2^-19, c - fl(a*b) = -(1+2^-19) - (1+2^-19) = -(2+2^-18)
    // FMA would differ
    float nmadd_expected = val_c - (float)(val_a * val_a);
    wah_v128_t nmadd_a = { .f32 = {val_a, val_a, val_a, val_a} };
    wah_v128_t nmadd_b = { .f32 = {val_a, val_a, val_a, val_a} };
    wah_v128_t nmadd_c = { .f32 = {val_c, val_c, val_c, val_c} };
    wah_v128_t nmadd_exp = { .f32 = {nmadd_expected, nmadd_expected, nmadd_expected, nmadd_expected} };
    run_simd_ternary_op_test("f32x4.relaxed_nmadd (determinism)", ternary_op_wasm_spec, &nmadd_a, &nmadd_b, &nmadd_c, &nmadd_exp);
}

void test_i8x16_relaxed_laneselect() {
    wah_v128_t v1 = {{0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}};
    wah_v128_t v2 = {{0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55}};
    wah_v128_t v3 = {{0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00}};
    wah_v128_t expected = {{0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55}};
    run_simd_ternary_op_test("i8x16.relaxed_laneselect", ternary_op_wasm_spec, &v1, &v2, &v3, &expected);
}

void test_f32x4_relaxed_min() {
    wah_v128_t v1 = { .f32 = {1.0f, 5.0f, 3.0f, 8.0f} };
    wah_v128_t v2 = { .f32 = {2.0f, 3.0f, 3.0f, 7.0f} };
    wah_v128_t expected = { .f32 = {1.0f, 3.0f, 3.0f, 7.0f} };
    run_simd_binary_op_test("f32x4.relaxed_min", binary_op_wasm_spec, &v1, &v2, &expected);

    wah_v128_t nan_v1 = { .u32 = {0x7fa00000, 0x3f800000, 0x7fa00000, 0x3f800000} };
    wah_v128_t nan_v2 = { .u32 = {0x3f800000, 0x7fa00000, 0x3f800000, 0x7fa00000} };
    wah_v128_t nan_expected = { .u32 = {0x7fc00000, 0x7fc00000, 0x7fc00000, 0x7fc00000} };
    run_simd_binary_op_test("f32x4.relaxed_min (nan canonicalization)", binary_op_wasm_spec, &nan_v1, &nan_v2, &nan_expected);

    wah_v128_t pos_zero = { .f32 = {0.0f, -0.0f, 0.0f, -0.0f} };
    wah_v128_t neg_zero = { .f32 = {-0.0f, 0.0f, -0.0f, 0.0f} };
    wah_v128_t sz_expected = { .f32 = {-0.0f, -0.0f, -0.0f, -0.0f} };
    run_simd_binary_op_test("f32x4.relaxed_min (signed zero)", binary_op_wasm_spec, &pos_zero, &neg_zero, &sz_expected);
}

void test_i16x8_relaxed_q15mulr_s() {
    wah_v128_t v1 = {{0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40}}; // 0.5 in Q15
    wah_v128_t v2 = {{0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40}}; // 0.5 in Q15
    wah_v128_t expected = {{0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20}}; // 0.25 in Q15
    run_simd_binary_op_test("i16x8.relaxed_q15mulr_s", binary_op_wasm_spec, &v1, &v2, &expected);

    v1 = (wah_v128_t){{0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F}}; // 1.0 in Q15
    v2 = (wah_v128_t){{0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F}}; // 1.0 in Q15
    expected = (wah_v128_t){{0xFE, 0x7F, 0xFE, 0x7F, 0xFE, 0x7F, 0xFE, 0x7F, 0xFE, 0x7F, 0xFE, 0x7F, 0xFE, 0x7F, 0xFE, 0x7F}}; // 32766 in Q15 (saturated)
    run_simd_binary_op_test("i16x8.relaxed_q15mulr_s", binary_op_wasm_spec, &v1, &v2, &expected);
}

void test_simd_fp_nan_canonicalization() {
    wah_v128_t nan32 = { .u32 = {0x7fa00000, 0x7fa00000, 0x7fa00000, 0x7fa00000} };
    wah_v128_t one32 = { .f32 = {1.0f, 1.0f, 1.0f, 1.0f} };
    wah_v128_t canon32 = { .u32 = {0x7fc00000, 0x7fc00000, 0x7fc00000, 0x7fc00000} };

    wah_v128_t nan64 = { .u64 = {0x7ff0000000000001ULL, 0x7ff4000000000000ULL} };
    wah_v128_t one64 = { .u64 = {0x3ff0000000000000ULL, 0x3ff0000000000000ULL} };
    wah_v128_t canon64 = { .u64 = {0x7ff8000000000000ULL, 0x7ff8000000000000ULL} };

    run_simd_binary_op_test("f32x4.sub (nan canonicalization)", binary_op_wasm_spec, &nan32, &one32, &canon32);
    run_simd_binary_op_test("f32x4.mul (nan canonicalization)", binary_op_wasm_spec, &nan32, &one32, &canon32);
    run_simd_binary_op_test("f32x4.div (nan canonicalization)", binary_op_wasm_spec, &nan32, &one32, &canon32);
    run_simd_unary_op_test("f32x4.sqrt (nan canonicalization)", unary_op_wasm_spec, &nan32, &canon32);
    run_simd_binary_op_test("f32x4.max (nan canonicalization)", binary_op_wasm_spec, &nan32, &one32, &canon32);

    run_simd_binary_op_test("f64x2.add (nan canonicalization)", binary_op_wasm_spec, &nan64, &one64, &canon64);
    run_simd_binary_op_test("f64x2.sub (nan canonicalization)", binary_op_wasm_spec, &nan64, &one64, &canon64);
    run_simd_binary_op_test("f64x2.mul (nan canonicalization)", binary_op_wasm_spec, &nan64, &one64, &canon64);
    run_simd_binary_op_test("f64x2.div (nan canonicalization)", binary_op_wasm_spec, &nan64, &one64, &canon64);
    run_simd_binary_op_test("f64x2.min (nan canonicalization)", binary_op_wasm_spec, &nan64, &one64, &canon64);
    run_simd_binary_op_test("f64x2.max (nan canonicalization)", binary_op_wasm_spec, &nan64, &one64, &canon64);

    run_simd_unary_op_test("f32x4.ceil (nan canonicalization)", unary_op_wasm_spec, &nan32, &canon32);
    run_simd_unary_op_test("f32x4.floor (nan canonicalization)", unary_op_wasm_spec, &nan32, &canon32);
    run_simd_unary_op_test("f32x4.trunc (nan canonicalization)", unary_op_wasm_spec, &nan32, &canon32);
    run_simd_unary_op_test("f32x4.nearest (nan canonicalization)", unary_op_wasm_spec, &nan32, &canon32);
    run_simd_unary_op_test("f64x2.ceil (nan canonicalization)", unary_op_wasm_spec, &nan64, &canon64);
    run_simd_unary_op_test("f64x2.floor (nan canonicalization)", unary_op_wasm_spec, &nan64, &canon64);
    run_simd_unary_op_test("f64x2.trunc (nan canonicalization)", unary_op_wasm_spec, &nan64, &canon64);
    run_simd_unary_op_test("f64x2.nearest (nan canonicalization)", unary_op_wasm_spec, &nan64, &canon64);

    run_simd_binary_op_test("f32x4.relaxed_max (nan canonicalization)", binary_op_wasm_spec, &nan32, &one32, &canon32);
    run_simd_binary_op_test("f64x2.relaxed_min (nan canonicalization)", binary_op_wasm_spec, &nan64, &one64, &canon64);
    run_simd_binary_op_test("f64x2.relaxed_max (nan canonicalization)", binary_op_wasm_spec, &nan64, &one64, &canon64);

    wah_v128_t nan_c32 = { .u32 = {0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000} };
    run_simd_ternary_op_test("f64x2.relaxed_madd (nan canonicalization)", ternary_op_wasm_spec, &nan64, &one64, &one64, &canon64);
    run_simd_ternary_op_test("f32x4.relaxed_nmadd (nan canonicalization)", ternary_op_wasm_spec, &nan32, &one32, &nan_c32, &canon32);
    run_simd_ternary_op_test("f64x2.relaxed_nmadd (nan canonicalization)", ternary_op_wasm_spec, &nan64, &one64, &one64, &canon64);
}

void test_f32x4_pmax_nan_handling() {
    wah_v128_t operand1 = { .f32 = {1.0f, NAN, 3.0f, NAN} };
    wah_v128_t operand2 = { .f32 = {2.0f, 2.0f, NAN, NAN} };
    wah_v128_t expected = { .f32 = {2.0f, WAH_CANONICAL_NAN32.f, 3.0f, WAH_CANONICAL_NAN32.f} };
    run_simd_binary_op_test("f32x4.pmax (nan_handling)", binary_op_wasm_spec, &operand1, &operand2, &expected);
}

void test_i16x8_relaxed_dot_i8x16_i7x16_s() {
    // a (i8x16): [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]
    // b: [1, 127, 2, 126, 3, 125, 4, 124, 5, 123, 6, 122, 7, 121, 8, 120]
    // Deterministic profile: b treated as signed i8 directly (no i7 sign extension)
    wah_v128_t a = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10}};
    wah_v128_t b = {{0x01, 0x7F, 0x02, 0x7E, 0x03, 0x7D, 0x04, 0x7C, 0x05, 0x7B, 0x06, 0x7A, 0x07, 0x79, 0x08, 0x78}};

    // Lane 0: 1*1 + 2*127 = 255, Lane 1: 3*2 + 4*126 = 510, Lane 2: 5*3 + 6*125 = 765
    // Lane 3: 7*4 + 8*124 = 1020, Lane 4: 9*5 + 10*123 = 1275, Lane 5: 11*6 + 12*122 = 1530
    // Lane 6: 13*7 + 14*121 = 1785, Lane 7: 15*8 + 16*120 = 2040
    wah_v128_t expected = { .i16 = {255, 510, 765, 1020, 1275, 1530, 1785, 2040} };

    run_simd_binary_op_test("i16x8.relaxed_dot_i8x16_i7x16_s", binary_op_wasm_spec, &a, &b, &expected);

    // Test with all positive values
    a = (wah_v128_t){{0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01}};
    b = (wah_v128_t){{0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01}};
    expected = (wah_v128_t){{0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00}};
    run_simd_binary_op_test("i16x8.relaxed_dot_i8x16_i7x16_s (positive)", binary_op_wasm_spec, &a, &b, &expected);

    // Test with zero values
    a = (wah_v128_t){{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    b = (wah_v128_t){{0x01, 0x7F, 0x02, 0x7E, 0x03, 0x7D, 0x04, 0x7C, 0x05, 0x7B, 0x06, 0x7A, 0x07, 0x79, 0x08, 0x78}};
    expected = (wah_v128_t){{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    run_simd_binary_op_test("i16x8.relaxed_dot_i8x16_i7x16_s (zero a)", binary_op_wasm_spec, &a, &b, &expected);
}

void test_i32x4_relaxed_dot_i8x16_i7x16_add_s() {
    // a (i8x16): [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]
    // b: [1, 127, 2, 126, 3, 125, 4, 124, 5, 123, 6, 122, 7, 121, 8, 120]
    // c (i32x4): [100, 200, 300, 400]
    // Deterministic profile: b treated as signed i8 directly
    wah_v128_t a = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10}};
    wah_v128_t b = {{0x01, 0x7F, 0x02, 0x7E, 0x03, 0x7D, 0x04, 0x7C, 0x05, 0x7B, 0x06, 0x7A, 0x07, 0x79, 0x08, 0x78}};
    wah_v128_t c = { .i32 = {100, 200, 300, 400} };

    // Lane 0: 100 + (1*1 + 2*127 + 3*2 + 4*126) = 100 + (1+254+6+504) = 100+765 = 865
    // Lane 1: 200 + (5*3 + 6*125 + 7*4 + 8*124) = 200 + (15+750+28+992) = 200+1785 = 1985
    // Lane 2: 300 + (9*5 + 10*123 + 11*6 + 12*122) = 300 + (45+1230+66+1464) = 300+2805 = 3105
    // Lane 3: 400 + (13*7 + 14*121 + 15*8 + 16*120) = 400 + (91+1694+120+1920) = 400+3825 = 4225
    wah_v128_t expected = { .i32 = {865, 1985, 3105, 4225} };

    run_simd_ternary_op_test("i32x4.relaxed_dot_i8x16_i7x16_add_s", ternary_op_wasm_spec, &a, &b, &c, &expected);

    // Test with zero accumulator
    c = (wah_v128_t){ .i32 = {0, 0, 0, 0} };
    expected = (wah_v128_t){ .i32 = {765, 1785, 2805, 3825} };
    run_simd_ternary_op_test("i32x4.relaxed_dot_i8x16_i7x16_add_s (zero acc)", ternary_op_wasm_spec, &a, &b, &c, &expected);
}

int main() {
    test_v128_const();
    test_v128_load_store();
    test_all_v128_loads();
    test_v128_not();
    test_i8x16_add();
    test_f32x4_add();
    test_v128_and();
    test_v128_andnot();
    test_v128_or();
    test_v128_xor();
    test_i8x16_add_sat_s();
    test_i8x16_add_sat_u();
    test_i8x16_sub();
    test_i8x16_sub_sat_s();
    test_i8x16_sub_sat_u();
    test_i16x8_sub();
    test_i32x4_mul();
    test_i16x8_eq();
    test_i16x8_ne();
    test_i16x8_lt_s();
    test_i16x8_lt_u();
    test_i16x8_gt_s();
    test_i16x8_gt_u();
    test_i16x8_le_s();
    test_i16x8_le_u();
    test_i16x8_ge_s();
    test_i16x8_ge_u();
    test_i8x16_eq();
    test_i8x16_ne();
    test_i8x16_lt_s();
    test_i8x16_lt_u();
    test_i8x16_gt_s();
    test_i8x16_gt_u();
    test_i8x16_le_s();
    test_i8x16_le_u();
    test_i8x16_ge_s();
    test_i8x16_ge_u();
    test_i32x4_eq();
    test_i32x4_ne();
    test_i32x4_lt_s();
    test_i32x4_lt_u();
    test_i32x4_gt_s();
    test_i32x4_gt_u();
    test_i32x4_le_s();
    test_i32x4_le_u();
    test_i32x4_ge_s();
    test_i32x4_ge_u();
    test_i64x2_eq();
    test_i64x2_ne();
    test_i64x2_lt_s();
    test_i64x2_gt_s();
    test_i64x2_le_s();
    test_i64x2_ge_s();
    test_f32x4_eq();
    test_f32x4_ne();
    test_f32x4_lt();
    test_f32x4_gt();
    test_f32x4_le();
    test_f32x4_ge();
    test_f64x2_eq();
    test_f64x2_ne();
    test_f64x2_lt();
    test_f64x2_gt();
    test_f64x2_le();
    test_f64x2_ge();

    test_i8x16_shuffle();
    test_i8x16_extract_lane_s();
    test_i8x16_replace_lane();
    test_i8x16_swizzle();
    test_i8x16_splat();
    test_i16x8_splat();
    test_i32x4_splat();
    test_i64x2_splat();
    test_f32x4_splat();
    test_f64x2_splat();

    test_v128_bitselect();
    test_v128_any_true();
    test_i32x4_trunc_sat_f32x4_s();
    test_i32x4_trunc_sat_f32x4_u();
    test_f32x4_convert_i32x4_s();
    test_f32x4_convert_i32x4_u();
    test_i32x4_trunc_sat_f64x2_s_zero();
    test_i32x4_trunc_sat_f64x2_u_zero();
    test_f64x2_convert_low_i32x4_s();
    test_f64x2_convert_low_i32x4_u();
    test_f32x4_demote_f64x2_zero();
    test_f64x2_promote_low_f32x4();

    test_i8x16_abs();
    test_i8x16_shl();
    test_i8x16_min_s();
    test_i8x16_avgr_u();
    test_i16x8_extend_low_i8x16_s();
    test_i16x8_narrow_i32x4_s();
    test_i16x8_q15mulr_sat_s();
    test_i32x4_dot_i16x8_s();
    test_i32x4_extmul_low_i16x8_s();
    test_i64x2_abs();
    test_i64x2_extend_high_i32x4_u();
    test_f32x4_abs();
    test_f32x4_ceil();
    test_f32x4_min();
    test_f64x2_neg();
    test_f64x2_sqrt();
    test_f32x4_pmax_nan_handling();
    test_simd_fp_nan_canonicalization();

    // Relaxed SIMD operations
    test_i8x16_relaxed_swizzle();
    test_i32x4_relaxed_trunc_f32x4_s();
    test_i32x4_relaxed_trunc_f32x4_u();
    test_f32x4_relaxed_madd();
    test_f32x4_relaxed_madd_determinism();
    test_i8x16_relaxed_laneselect();
    test_f32x4_relaxed_min();
    test_i16x8_relaxed_q15mulr_s();
    test_i16x8_relaxed_dot_i8x16_i7x16_s();
    test_i32x4_relaxed_dot_i8x16_i7x16_add_s();

    fprintf(stderr, "All tests passed.\n");
    return 0;
}
