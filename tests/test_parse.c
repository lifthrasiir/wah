#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#include "../wah.h"
#include "common.h"
#include "wah_impl.h"

static void test_parse_module_argument_errors() {
    printf("Running test_parse_module_argument_errors...\n");

    uint8_t short_binary[4] = {0};
    uint8_t header_only[8] = {0};
    wah_module_t module = {0};

    assert_err(wah_parse_module(NULL, header_only, sizeof(header_only), NULL), WAH_ERROR_MISUSE);
    assert_err(wah_parse_module(&module, NULL, 0, NULL), WAH_ERROR_MISUSE);
    assert_err(wah_parse_module(&module, short_binary, sizeof(short_binary), NULL), WAH_ERROR_UNEXPECTED_EOF);
}

static void test_malformed_code_body_size_wasm() {
    printf("Running test_malformed_code_body_size_wasm...\n");
    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

#define REPEAT10(x) x x x x x x x x x x
#define REPEAT100(x) REPEAT10(REPEAT10(x))
#define REPEAT1000(x) REPEAT10(REPEAT10(REPEAT10(x)))

    // This WASM module has an oversized code body (body_size exceeds section bounds).
    const char *wasm_spec = "wasm \
        types {[ fn [i32 i32] [i32] ]} \
        funcs {[ 0 ]} \
        exports {[ {'000000'} fn# 0 ]} \
        code %'3001aefffa308d8e3030303030" \
        REPEAT1000("30") REPEAT1000("30") REPEAT1000("30") REPEAT1000("30") \
        REPEAT10("30") REPEAT10("30") REPEAT10("30") REPEAT10("30") REPEAT10("30") "'";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_MALFORMED);
    wah_free_module(&module);
}

static void test_zero_params_zero_results_func_type() {
    printf("Running test_zero_params_zero_results_func_type...\n");
    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    // This WASM module contains a function type with zero parameters and zero results.
    // This is used to test that the parser correctly handles zero-count vectors,
    // specifically avoiding `malloc(0)` which has implementation-defined behavior.
    const char *wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        code {[ {[] end} ]}";

    assert_ok(wah_parse_module_from_spec(&module, wasm_spec));
    wah_free_module(&module);
}

static void test_invalid_section_order_mem_table() {
    printf("Running test_invalid_section_order_mem_table...\n");
    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    // This WASM module has the Memory Section before the Table Section (invalid order)
    const char *wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        memories {[ limits.i32/1 1 ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        funcs {[ 0 ]} \
        code {[ {[] end} ]}";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_MALFORMED);
    wah_free_module(&module);
}

void test_invalid_element_segment_func_idx() {
    printf("Running test_invalid_element_segment_func_idx...\n");

    // Minimal WASM binary with an element section referencing an out-of-bounds function index
    // (module
    //   (type $0 (func))
    //   (func $f0 (type $0) nop)
    //   (table $0 1 funcref)
    //   (elem $0 (i32.const 0) $f0 $f1) ;; $f1 does not exist, func_idx 1 is out of bounds
    // )
    const char *wasm_binary_invalid_element_segment_func_idx_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        elements {[ elem.active.table#0 { i32.const 0 end } 0 1 ]} \
        code {[ {[] nop end } ]}";

    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    assert_err(wah_parse_module_from_spec(&module, wasm_binary_invalid_element_segment_func_idx_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);
}

void test_code_section_no_function_section() {
    printf("Running test_code_section_no_function_section...\n");

    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    // This WASM module has a code section but no function section.
    // This should result in WAH_ERROR_MALFORMED because wasm_function_count will be 0
    // but the code section count will be > 0.
    const char *wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        code {[ {[] end} ]}";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_MALFORMED);
    wah_free_module(&module);
}

void test_function_section_no_code_section() {
    printf("Running test_function_section_no_code_section...\n");

    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    // This WASM module has a function section but no code section.
    // This should result in WAH_ERROR_MALFORMED because wasm_function_count will be > 0
    // but module->code_count will be 0.
    const char *wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]}";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_MALFORMED);
    wah_free_module(&module);
}

// memory.init without a data count section is malformed per spec.
static void test_parse_data_no_datacount_memory_init_fails() {
    printf("Running test_parse_data_no_datacount_memory_init_fails...\n");

    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    const char *wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        memories {[ limits.i32/1 1 ]} \
        exports {[ {'memory'} mem# 0, {'test_func'} fn# 0 ]} \
        code {[ {[] i32.const 0 i32.const 0 i32.const 5 memory.init 0 0 end} ]} \
        data {[ data.active.table#0 i32.const 0 end {'hello'} ]}";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_MALFORMED);
    wah_free_module(&module);
}

// Test case for deferred data segment validation failure.
// One data segment (index 0), but memory.init tries to use data_idx 1.
static void test_deferred_data_validation_failure() {
    printf("Running test_deferred_data_validation_failure...\n");

    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    const char *wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        memories {[ limits.i32/1 1 ]} \
        exports {[ {'memory'} mem# 0, {'test_func'} fn# 0 ]} \
        datacount { 1 } \
        code {[ {[] i32.const 0 i32.const 0 i32.const 5 memory.init 1 0 end } ]} \
        data {[ data.active.table#0 i32.const 0 end {'hello'} ]}";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);
}

// Undefined opcode 0x09 is rejected as malformed.
static void test_unused_opcode_validation_failure() {
    printf("Running test_unused_opcode_validation_failure...\n");

    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    const char *wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        code {[ {[] %'09' end} ]}";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_MALFORMED);
    wah_free_module(&module);
}

// datacount section present but no data section is malformed.
static void test_datacount_no_data_section() {
    printf("Running test_datacount_no_data_section...\n");
    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    const char *wasm_spec = "wasm \
        types {[ fn [i32] [i32] ]} \
        datacount { 2 }";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_MALFORMED);
    wah_free_module(&module);
}

// Test case for END opcode not at the end of function body.
// According to WASM spec, END (0x0b) must be the very last opcode in a function body.
static void test_end_opcode_not_at_end() {
    printf("Running test_end_opcode_not_at_end...\n");

    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    // This should fail validation because END is not the last opcode
    const char *wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        code {[ {[] nop end i32.const 0} ]}";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);
}

// Test case for multiple END opcodes in function body.
// This should also fail validation.
static void test_multiple_end_opcodes() {
    printf("Running test_multiple_end_opcodes...\n");

    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    // This should fail validation because there are multiple END opcodes
    const char *wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        code {[ {[] end end} ]}";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);
}

// Test case for CALL opcode with out-of-bounds function index.
// This should fail validation.
static void test_call_out_of_bounds_func_idx() {
    printf("Running test_call_out_of_bounds_func_idx...\n");

    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    // Only 1 function (index 0), but CALL tries to call index 1
    const char *wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        code {[ {[] call 1 end} ]}";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);
}

// Test case for CALL_INDIRECT with out-of-bounds type index.
// This should fail validation.
static void test_call_indirect_out_of_bounds_type_idx() {
    printf("Running test_call_indirect_out_of_bounds_type_idx...\n");

    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    // Only 1 type (index 0), but CALL_INDIRECT tries to use type index 1
    const char *wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        code {[ {[] i32.const 0 call_indirect 1 0 end} ]}";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);
}

// Test case for CALL_INDIRECT with out-of-bounds table index.
// This should fail validation.
static void test_call_indirect_out_of_bounds_table_idx() {
    printf("Running test_call_indirect_out_of_bounds_table_idx...\n");

    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    // Only 1 table (index 0), but CALL_INDIRECT tries to use table index 1
    const char *wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        code {[ {[] i32.const 0 call_indirect 0 1 end} ]}";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);
}

// Test case for TABLE_GET with out-of-bounds table index.
// This should fail validation.
static void test_table_get_out_of_bounds_table_idx() {
    printf("Running test_table_get_out_of_bounds_table_idx...\n");

    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    // Only 1 table (index 0), but TABLE_GET tries to use table index 1
    const char *wasm_spec = "wasm \
        types {[ fn [] [funcref] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        code {[ {[] i32.const 0 table.get 1 end} ]}";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);
}

// Test case for TABLE_SET with out-of-bounds table index.
// This should fail validation.
static void test_table_set_out_of_bounds_table_idx() {
    printf("Running test_table_set_out_of_bounds_table_idx...\n");

    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    // Only 1 table (index 0), but TABLE_SET tries to use table index 1
    const char *wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        code {[ {[] i32.const 0 ref.null funcref table.set 1 end} ]}";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);
}

// Test case for TABLE_SIZE with out-of-bounds table index.
// This should fail validation.
static void test_table_size_out_of_bounds_table_idx() {
    printf("Running test_table_size_out_of_bounds_table_idx...\n");

    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    // Only 1 table (index 0), but TABLE_SIZE tries to use table index 1
    const char *wasm_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        code {[ {[] table.size 1 end} ]}";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);
}

// Test case for TABLE_GROW with out-of-bounds table index.
// This should fail validation.
static void test_table_grow_out_of_bounds_table_idx() {
    printf("Running test_table_grow_out_of_bounds_table_idx...\n");

    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    // Only 1 table (index 0), but TABLE_GROW tries to use table index 1
    const char *wasm_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        code {[ {[] i32.const 1 ref.null funcref table.grow 1 end} ]}";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);
}

// Test case for TABLE_FILL with out-of-bounds table index.
// This should fail validation.
static void test_table_fill_out_of_bounds_table_idx() {
    printf("Running test_table_fill_out_of_bounds_table_idx...\n");

    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    // Only 1 table (index 0), but TABLE_FILL tries to use table index 1
    const char *wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        code {[ {[] i32.const 0 ref.null funcref i32.const 0 table.fill 1 end} ]}";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);
}

// Test case for TABLE_COPY with out-of-bounds dst table index.
// This should fail validation.
static void test_table_copy_out_of_bounds_dst_table_idx() {
    printf("Running test_table_copy_out_of_bounds_dst_table_idx...\n");

    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    // Only 2 tables (indices 0, 1), but TABLE_COPY tries to use dst table index 2
    const char *wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 1, funcref limits.i32/1 1 ]} \
        code {[ {[] i32.const 0 i32.const 0 i32.const 0 table.copy 2 0 end} ]}";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);
}

// Test case for TABLE_COPY with out-of-bounds src table index.
// This should fail validation.
static void test_table_copy_out_of_bounds_src_table_idx() {
    printf("Running test_table_copy_out_of_bounds_src_table_idx...\n");

    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    // Only 2 tables (indices 0, 1), but TABLE_COPY tries to use src table index 2
    const char *wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 1, funcref limits.i32/1 1 ]} \
        code {[ {[] i32.const 0 i32.const 0 i32.const 0 table.copy 0 2 end} ]}";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);
}

// Test case for TABLE_INIT with out-of-bounds element index.
// This should fail validation.
static void test_table_init_out_of_bounds_elem_idx() {
    printf("Running test_table_init_out_of_bounds_elem_idx...\n");

    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    // Only 1 element segment (index 0), but TABLE_INIT tries to use element index 1
    const char *wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 10 ]} \
        elements {[ elem.passive elem.funcref [0] ]} \
        code {[ {[] i32.const 0 i32.const 0 i32.const 0 table.init 1 0 end} ]}";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);
}

// Test case for TABLE_INIT with out-of-bounds table index.
// This should fail validation.
static void test_table_init_out_of_bounds_table_idx() {
    printf("Running test_table_init_out_of_bounds_table_idx...\n");

    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    // Only 1 table (index 0), but TABLE_INIT tries to use table index 1
    const char *wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 10 ]} \
        elements {[ elem.passive elem.funcref [0] ]} \
        code {[ {[] i32.const 0 i32.const 0 i32.const 0 table.init 0 1 end} ]}";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);
}

// Test case for ELEM_DROP with out-of-bounds element index.
// This should fail validation.
static void test_elem_drop_out_of_bounds_elem_idx() {
    printf("Running test_elem_drop_out_of_bounds_elem_idx...\n");

    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    // Only 1 element segment (index 0), but ELEM_DROP tries to use element index 1
    const char *wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 10 ]} \
        elements {[ elem.passive elem.funcref [0] ]} \
        code {[ {[] elem.drop 1 end} ]}";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);
}

// Test case for SIMD I8X16_EXTRACT_LANE_S with out-of-bounds lane index.
// This should fail validation.
static void test_i8x16_extract_lane_s_out_of_bounds_lane_idx() {
    printf("Running test_i8x16_extract_lane_s_out_of_bounds_lane_idx...\n");

    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    // i8x16 has 16 lanes (indices 0-15), but EXTRACT_LANE tries to use lane index 16 (hex 10)
    const char *wasm_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] v128.const %'00000000000000000000000000000000' i8x16.extract_lane_s %'10' end} ]}";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);
}

// Test case for SIMD I32X4_EXTRACT_LANE with out-of-bounds lane index.
// This should fail validation.
static void test_i32x4_extract_lane_out_of_bounds_lane_idx() {
    printf("Running test_i32x4_extract_lane_out_of_bounds_lane_idx...\n");

    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    // i32x4 has 4 lanes (indices 0-3), but EXTRACT_LANE tries to use lane index 4
    const char *wasm_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] v128.const %'00000000000000000000000000000000' i32x4.extract_lane %'04' end} ]}";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);
}

// Test case for SIMD V128_LOAD8_LANE with out-of-bounds lane index.
// This should fail validation.
static void test_v128_load8_lane_out_of_bounds_lane_idx() {
    printf("Running test_v128_load8_lane_out_of_bounds_lane_idx...\n");

    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    // v128.load8_lane has 16 lanes (indices 0-15), but tries to use lane index 16 (hex 10)
    const char *wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        memories {[ limits.i32/1 1 ]} \
        code {[ {[] i32.const 0 v128.const %'00000000000000000000000000000000' v128.load8_lane 0 0 %'10' end} ]}";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);
}

// Test case for SIMD V128_LOAD32_LANE with out-of-bounds lane index.
// This should fail validation.
static void test_v128_load32_lane_out_of_bounds_lane_idx() {
    printf("Running test_v128_load32_lane_out_of_bounds_lane_idx...\n");

    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    // v128.load32_lane has 4 lanes (indices 0-3), but tries to use lane index 4
    const char *wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        memories {[ limits.i32/1 1 ]} \
        code {[ {[] i32.const 0 v128.const %'00000000000000000000000000000000' v128.load32_lane 2 0 %'04' end} ]}";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);
}

// Test case for MEMORY_INIT with out-of-bounds memory index.
// This should fail validation.
static void test_memory_init_out_of_bounds_mem_idx() {
    printf("Running test_memory_init_out_of_bounds_mem_idx...\n");

    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    // Only 1 memory (index 0), but MEMORY_INIT tries to use memory index 1
    const char *wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        memories {[ limits.i32/1 1 ]} \
        code {[ {[] i32.const 0 i32.const 0 i32.const 5 memory.init 0 1 end} ]} \
        data {[ data.passive {'hello'} ]}";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);
}

// Test case for MEMORY_INIT with out-of-bounds data segment index.
// This should fail validation (deferred validation).
static void test_memory_init_out_of_bounds_data_idx() {
    printf("Running test_memory_init_out_of_bounds_data_idx...\n");

    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    // Only 1 data segment (index 0), but MEMORY_INIT tries to use data index 1
    const char *wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        memories {[ limits.i32/1 1 ]} \
        datacount { 1 } \
        code {[ {[] i32.const 0 i32.const 0 i32.const 5 memory.init 1 0 end} ]} \
        data {[ data.passive {'hello'} ]}";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);
}

// Test case for MEMORY_COPY with out-of-bounds dest memory index.
// This should fail validation.
static void test_memory_copy_out_of_bounds_dest_mem_idx() {
    printf("Running test_memory_copy_out_of_bounds_dest_mem_idx...\n");

    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    // Only 1 memory (index 0), but MEMORY_COPY tries to use dest memory index 1
    const char *wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        memories {[ limits.i32/1 1 ]} \
        code {[ {[] i32.const 0 i32.const 0 i32.const 0 memory.copy 1 0 end} ]}";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);
}

// Test case for MEMORY_COPY with out-of-bounds src memory index.
// This should fail validation.
static void test_memory_copy_out_of_bounds_src_mem_idx() {
    printf("Running test_memory_copy_out_of_bounds_src_mem_idx...\n");

    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    // Only 1 memory (index 0), but MEMORY_COPY tries to use src memory index 1
    const char *wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        memories {[ limits.i32/1 1 ]} \
        code {[ {[] i32.const 0 i32.const 0 i32.const 0 memory.copy 0 1 end} ]}";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);
}

// Regression tests for soft "hang" test cases found by fuzzers.
static void test_all_hang_wasm_parsing_errors() {
    printf("Running test_all_hang_wasm_parsing_errors...\n");

    struct {
        const uint8_t *binary;
        unsigned int len;
        const char *name;
    } hang_tests[] = {
        {
            .binary = (const uint8_t[]){
                0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x07, 0xff, 0x80,
                0xf1, 0x30, 0x30, 0x30, 0x30
            },
            .len = 17,
            .name = "large_type_section_count_0"
        },
        {
            .binary = (const uint8_t[]){
                0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x04, 0x15, 0x90, 0xff,
                0xff, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
                0x30, 0x30, 0x30
            },
            .len = 29,
            .name = "large_table_section_count_0"
        },
        {
            .binary = (const uint8_t[]){
                0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x07, 0x01, 0x60,
                0x02, 0x7f, 0x7f, 0x01, 0x7f, 0x03, 0x02, 0x01, 0x00, 0x07, 0x0a, 0x01,
                0x06, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x0a, 0x08, 0x01,
                0xb5, 0x30, 0x30, 0xff, 0xff, 0xff, 0x7d, 0x7d
            },
            .len = 45,
            .name = "incorrect_code_section_size_0"
        }
    };

    for (size_t i = 0; i < sizeof(hang_tests) / sizeof(hang_tests[0]); ++i) {
        wah_module_t module;
        memset(&module, 0, sizeof(wah_module_t));

        wah_error_t err = wah_parse_module(&module, hang_tests[i].binary, hang_tests[i].len, NULL);

        if (err == WAH_ERROR_VALIDATION_FAILED || err == WAH_ERROR_TOO_LARGE ||
            err == WAH_ERROR_UNEXPECTED_EOF || err == WAH_ERROR_MALFORMED) {
            printf("  - PASSED: %s correctly returned %s.\n", hang_tests[i].name, wah_strerror(err));
            wah_free_module(&module);
        } else {
            fprintf(stderr, "Assertion failed: Expected a parse error for %s, but got %s\n", hang_tests[i].name, wah_strerror(err));
            exit(1);
        }
    }
}

static void test_deep_const_expr_stack_instantiation() {
    printf("Running test_deep_const_expr_stack_instantiation...\n");

    const uint8_t wasm[] = {
        0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00,
        0x01, 0x04, 0x01, 0x60, 0x00, 0x00, 0x06, 0x36,
        0x01, 0x7f, 0x01, 0x41, 0x00, 0x41, 0x00, 0x41,
        0x00, 0x41, 0x00, 0x41, 0x00, 0x41, 0x00, 0x41,
        0x00, 0x41, 0x00, 0x41, 0x00, 0x41, 0x00, 0x41,
        0x00, 0x41, 0x00, 0x41, 0x00, 0x41, 0x00, 0x41,
        0x00, 0x41, 0x00, 0x41, 0x00, 0x6a, 0x6a, 0x6a,
        0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a,
        0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x0b
    };

    wah_module_t module = {0};
    wah_exec_context_t ctx = {0};
    assert_ok(wah_parse_module(&module, wasm, sizeof(wasm), NULL));
    assert_ok(wah_new_exec_context(&ctx, &module, NULL));
    assert_ok(wah_instantiate(&ctx));
    wah_free_exec_context(&ctx);
    wah_free_module(&module);
}

static void test_control_frame_cleanup_after_block_type_eof() {
    printf("Running test_control_frame_cleanup_after_block_type_eof...\n");

    const uint8_t wasm[] = {
        0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00,
        0x01, 0x05, 0x01, 0x60, 0x00, 0x01, 0x6e, 0x03,
        0x02, 0x01, 0x00, 0x0a, 0x0f, 0x01, 0x0d, 0x00,
        0x02, 0x6e, 0xd0, 0x6e, 0xfb, 0x18, 0x03, 0x00,
        0x6e, 0x6d, 0x0b, 0x03
    };

    wah_module_t module = {0};
    assert_err(wah_parse_module(&module, wasm, sizeof(wasm), NULL), WAH_ERROR_UNEXPECTED_EOF);
    wah_free_module(&module);
}

static void test_reject_huge_local_count_before_allocation() {
    printf("Running test_reject_huge_local_count_before_allocation...\n");

    const uint8_t wasm[] = {
        0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00,
        0x01, 0x04, 0x01, 0x60, 0x00, 0x00, 0x03, 0x02,
        0x01, 0x00, 0x0a, 0x0c, 0x01, 0x0a, 0x02, 0xff,
        0xff, 0xff, 0xa1, 0x0f, 0x7f, 0x01, 0x7f, 0x0b
    };

    wah_module_t module = {0};
    assert_err(wah_parse_module(&module, wasm, sizeof(wasm), NULL), WAH_ERROR_TOO_LARGE);
    wah_free_module(&module);
}

static void test_reject_huge_function_type_counts_before_allocation() {
    printf("Running test_reject_huge_function_type_counts_before_allocation...\n");

    const uint8_t wasm[] = {
        0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00,
        0x01, 0x0c, 0x02, 0x60, 0x01, 0x7f, 0xfe, 0xff,
        0xff, 0xff, 0x02, 0x7f, 0x7f, 0x60, 0x00, 0xff,
        0xff, 0xff, 0xff, 0x02, 0x01, 0x00, 0x01, 0x00,
        0x60, 0x40, 0x00, 0x17, 0x9c, 0x1b, 0x0a, 0x12,
        0x01, 0x10, 0x00, 0x20, 0x00, 0x04, 0x01, 0x02,
        0x05, 0x41, 0x02, 0x05, 0x41, 0x03, 0x41, 0x04,
        0x0b, 0x0b
    };

    wah_module_t module = {0};
    assert_err(wah_parse_module(&module, wasm, sizeof(wasm), NULL), WAH_ERROR_MALFORMED);
    wah_free_module(&module);
}

static void test_reject_huge_br_table_count_before_allocation() {
    printf("Running test_reject_huge_br_table_count_before_allocation...\n");

    const uint8_t wasm[] = {
        0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00,
        0x01, 0x08, 0x02, 0x60, 0x00, 0x01, 0x7f, 0x60,
        0x00, 0x00, 0x03, 0x03, 0x02, 0x00, 0x01, 0x0d,
        0x03, 0x01, 0x00, 0x01, 0x0a, 0x22, 0x02, 0x0f,
        0x00, 0x02, 0x40, 0x1f, 0x40, 0x95, 0xff, 0xff,
        0xff, 0x01, 0x50, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01,
        0x02, 0x00, 0x10, 0x01, 0x0b, 0x41, 0x7f, 0x0b
    };

    wah_module_t module = {0};
    assert_err(wah_parse_module(&module, wasm, sizeof(wasm), NULL), WAH_ERROR_MALFORMED);
    wah_free_module(&module);
}

static void test_truncated_i8x16_shuffle_immediate() {
    printf("Running test_truncated_i8x16_shuffle_immediate...\n");

    const uint8_t wasm[] = {
        0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00,
        0x01, 0x05, 0x01, 0x60, 0x00, 0x01, 0x7b, 0x03,
        0x02, 0x01, 0x00, 0x0a, 0x1c, 0x01, 0x1a, 0x00,
        0xfd, 0x0c, 0x00, 0x01, 0x02, 0x03, 0xff, 0x05,
        0x05, 0x06, 0x3b, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x01, 0xfd,
        0x0d, 0x00, 0x00
    };

    wah_module_t module = {0};
    assert_err(wah_parse_module(&module, wasm, sizeof(wasm), NULL), WAH_ERROR_UNEXPECTED_EOF);
    wah_free_module(&module);
}

static void test_reject_huge_element_count_before_allocation() {
    printf("Running test_reject_huge_element_count_before_allocation...\n");

    const uint8_t wasm[] = {
        0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00,
        0x01, 0x04, 0x01, 0x60, 0x00, 0x00, 0x09, 0x0a,
        0x01, 0x05, 0x63, 0x80, 0x00, 0xbc, 0xbc, 0xbc, // (ref null 0) with a padded index
        0xc2, 0x01, 0x01, 0xbf, 0x01, 0x00, 0x41, 0x00,
        0xfd, 0x0c, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x3a, 0x00, 0x00, 0x61, 0x73, 0x6d,
        0x01, 0x00, 0x00, 0x00, 0x01, 0x05, 0x01, 0x60,
        0x00, 0x01, 0x7b, 0x03, 0x02, 0x01, 0x00, 0x00,
        0x03, 0x01, 0x00, 0x01, 0x0a, 0x0a, 0x01, 0x08,
        0x00, 0x41, 0x00, 0xfd, 0x09, 0x00, 0xbc, 0xbc
    };

    wah_module_t module = {0};
    assert_err(wah_parse_module(&module, wasm, sizeof(wasm), NULL), WAH_ERROR_MALFORMED);
    wah_free_module(&module);
}

static void test_reject_huge_memory_min_with_exec_limit() {
    printf("Running test_reject_huge_memory_min_with_exec_limit...\n");

    const uint8_t wasm[] = {
        0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00,
        0x05, 0x08, 0x01, 0x04, 0xff, 0x80, 0x80, 0x80,
        0x80, 0x20
    };

    wah_module_t module = {0};
    wah_exec_context_t ctx = {0};
    wah_exec_options_t options = {
        .limits = {
            .max_memory_bytes = 64 * 1024 * 1024,
        }
    };
    assert_ok(wah_parse_module(&module, wasm, sizeof(wasm), NULL));
    assert_err(wah_new_exec_context(&ctx, &module, &options), WAH_ERROR_TOO_LARGE);
    wah_free_module(&module);
}

static void test_reject_huge_rec_group_count() {
    printf("Running test_reject_huge_rec_group_count...\n");

    // Type section with rec group (0x4E) declaring group_count=1000000 but
    // only a few bytes of actual data. Should fail quickly without excessive
    // allocation.
    // Bytes: magic+version, type section (id=1, size=6), rec_count=1,
    //        0x4E (rec marker), group_count=1000000 (LEB128: C0 84 3D)
    const uint8_t wasm[] = {
        0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00,
        0x01, 0x06, 0x01, 0x4e, 0xc0, 0x84, 0x3d, 0x60
    };

    wah_parse_options_t opts = { .features = WAH_FEATURE_ALL };
    wah_module_t module = {0};
    wah_error_t err = wah_parse_module(&module, wasm, sizeof(wasm), &opts);
    assert_true(err != WAH_OK);
    wah_free_module(&module);
}

static void test_unreachable_array_new_fixed_huge_length() {
    printf("Running test_unreachable_array_new_fixed_huge_length...\n");

    const uint8_t wasm[] = {
        0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00,
        0x01, 0x08, 0x02, 0x5e, 0x78, 0x01, 0x60, 0x00,
        0x01, 0x7f, 0x03, 0x02, 0x01, 0x01, 0x0c, 0x01,
        0x01, 0x0a, 0x14, 0x01, 0x12, 0x00, 0x00, 0x00,
        0x41, 0x04, 0xfb, 0x08, 0x00, 0xb0, 0xff, 0xfa,
        0xfd, 0x00, 0x01, 0x00, 0x04, 0x1e, 0x28
    };

    wah_module_t module = {0};
    assert_err(wah_parse_module(&module, wasm, sizeof(wasm), NULL), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);
}

// 22d534b: Reject (very slightly) overlong signed LEB128 i64 encodings.
static void test_overlong_sleb128_i64() {
    printf("Testing overlong signed LEB128 i64 rejection (22d534b)...\n");

    // Valid 10-byte signed LEB128 for -1: nine 0xFF bytes then 0x7F
    const char *good_spec = "wasm \
        types {[ fn [] [i64] ]} \
        funcs {[ 0 ]} \
        code {[ { [] %'42' %'FFFFFFFFFFFFFFFFFF7F' end } ]}";
    wah_module_t good = {0};
    assert_ok(wah_parse_module_from_spec(&good, good_spec));
    wah_free_module(&good);

    // Valid 10-byte signed LEB128 for 0: nine 0x80 bytes then 0x00
    const char *good_zero = "wasm \
        types {[ fn [] [i64] ]} \
        funcs {[ 0 ]} \
        code {[ { [] %'42' %'80808080808080808000' end } ]}";
    wah_module_t good2 = {0};
    assert_ok(wah_parse_module_from_spec(&good2, good_zero));
    wah_free_module(&good2);

    // Overlong: 10-byte encoding with last byte 0x01 (not 0x00 or 0x7F)
    const char *bad_spec = "wasm \
        types {[ fn [] [i64] ]} \
        funcs {[ 0 ]} \
        code {[ { [] %'42' %'80808080808080808001' end } ]}";
    wah_module_t bad = {0};
    assert_err(wah_parse_module_from_spec(&bad, bad_spec), WAH_ERROR_TOO_LARGE);
    wah_free_module(&bad);

    // Overlong negative: last byte 0x7E instead of 0x7F
    const char *bad_neg = "wasm \
        types {[ fn [] [i64] ]} \
        funcs {[ 0 ]} \
        code {[ { [] %'42' %'FFFFFFFFFFFFFFFFFF7E' end } ]}";
    wah_module_t bad2 = {0};
    assert_err(wah_parse_module_from_spec(&bad2, bad_neg), WAH_ERROR_TOO_LARGE);
    wah_free_module(&bad2);
}

// 1249e11: Validate start function type must be () -> ().
static void test_start_function_type() {
    printf("Testing start function type validation (1249e11)...\n");

    // Positive: start function with () -> ()
    const char *good_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        start { 0 } \
        code {[ {[] end } ]}";
    wah_module_t good = {0};
    assert_ok(wah_parse_module_from_spec(&good, good_spec));
    assert_true(wah_debug_module_has_start_function(&good));
    assert_eq_u32(wah_debug_module_start_function_idx(&good), 0);
    wah_free_module(&good);

    // Negative: start function with (i32) -> ()
    const char *bad_params = "wasm \
        types {[ fn [i32] [] ]} \
        funcs {[ 0 ]} \
        start { 0 } \
        code {[ {[] end } ]}";
    wah_module_t bad1 = {0};
    assert_err(wah_parse_module_from_spec(&bad1, bad_params), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad1);

    // Negative: start function with () -> (i32)
    const char *bad_results = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        start { 0 } \
        code {[ {[] i32.const 0 end } ]}";
    wah_module_t bad2 = {0};
    assert_err(wah_parse_module_from_spec(&bad2, bad_results), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad2);
}

// 9d3de74: Validate table index bounds in element section parsing.
static void test_elem_oob_table_idx() {
    printf("Testing element section OOB table index (9d3de74)...\n");

    // Active element targeting non-existent table (no tables at all)
    const char *no_table = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        elements {[ elem.active.table#0 i32.const 0 end [ 0 ] ]} \
        code {[ {[] end } ]}";
    wah_module_t m1 = {0};
    assert_err(wah_parse_module_from_spec(&m1, no_table), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&m1);

    // Active element targeting table index 5 when only 1 table exists
    const char *oob_idx = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        elements {[ elem.active.table# 5 i32.const 0 end elem.funcref [ 0 ] ]} \
        code {[ {[] end } ]}";
    wah_module_t m2 = {0};
    assert_err(wah_parse_module_from_spec(&m2, oob_idx), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&m2);
}

static void test_unknown_export_kind() {
    printf("Running test_unknown_export_kind...\n");
    wah_module_t module = {0};
    assert_err(wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [] [] ]} funcs {[ 0 ]} code {[ {[] end} ]} \
        exports {%'01 01 78 05 00'}"),
        WAH_ERROR_MALFORMED);
    wah_free_module(&module);
}

static void test_unknown_element_segment_flags() {
    printf("Running test_unknown_element_segment_flags...\n");
    wah_module_t module = {0};
    // flags=4 (elem.active.expr.table#0) should succeed as a baseline
    assert_ok(wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [] [] ]} funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        elements {[ elem.active.expr.table#0 i32.const 0 end [ref.func 0 end] ]} \
        code {[ {[] end} ]}"));
    wah_free_module(&module);
    module = (wah_module_t){0};
    // flags=8 has same binary layout as flags=4 but is invalid per spec
    assert_err(wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [] [] ]} funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        elements {[ %'08' i32.const 0 end [ref.func 0 end] ]} \
        code {[ {[] end} ]}"),
        WAH_ERROR_MALFORMED);
    wah_free_module(&module);
}

static void test_unknown_data_segment_flags() {
    printf("Running test_unknown_data_segment_flags...\n");
    wah_module_t module = {0};
    assert_err(wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [] [] ]} funcs {[ 0 ]} memories {[ limits.i32/1 1 ]} \
        code {[ {[] end} ]} datacount { 1 } data {%'01 03 00'}"),
        WAH_ERROR_MALFORMED);
    wah_free_module(&module);
}

static void test_count_overflow() {
    printf("Running test_count_overflow...\n");
    wah_module_t module = {0};
    assert_err(wah_parse_module_from_spec(&module, "wasm \
        types {%'80 80 80 80 08'}"),
        WAH_ERROR_MALFORMED);
    wah_free_module(&module);
}

static void test_fuzz_ref_validation_regressions() {
    printf("Running test_fuzz_ref_validation_regressions...\n");

    {
        const uint8_t bad_ref_cast_operand[] = {
            0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00,
            0x01, 0x09, 0x02, 0x5f, 0x01, 0x7f, 0x01, 0x60,
            0x00, 0x01, 0x7f, 0x03, 0x02, 0x01, 0x01, 0x07,
            0x05, 0x01, 0x01, 0x66, 0x00, 0x00, 0x0a, 0x20,
            0x01, 0x1e, 0x01, 0x01, 0x6e, 0xfb, 0x01, 0x00,
            0x21, 0x00, 0x41, 0x10, 0xfb, 0x17, 0x00, 0x41,
            0xcd, 0x00, 0xfb, 0x05, 0x00, 0x00, 0x20, 0x00,
            0xfb, 0x17, 0x00, 0xfb, 0x02, 0x00, 0x00, 0x0b
        };
        wah_module_t module = {0};
        assert_err(wah_parse_module(&module, bad_ref_cast_operand, sizeof(bad_ref_cast_operand), NULL),
                   WAH_ERROR_VALIDATION_FAILED);
        wah_free_module(&module);
    }

    {
        const uint8_t too_large_heap_type[] = {
            0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00,
            0x01, 0x0a, 0x02, 0x50, 0x00, 0x5e, 0x7f, 0x01,
            0x60, 0x00, 0x01, 0x7f, 0x03, 0x03, 0x02, 0x01,
            0x01, 0x0a, 0x2b, 0x02, 0x1d, 0x01, 0x01, 0x63,
            0xff, 0xff, 0xff, 0xff, 0x07, 0x00, 0x21, 0x00,
            0x29, 0x00, 0x41, 0x02, 0x41, 0xe3, 0x00, 0xfb,
            0x11, 0x00, 0x20, 0x00, 0x00, 0x00, 0x01, 0x00,
            0x02, 0x50, 0x00, 0x01, 0x0b, 0x0b, 0x00, 0x41,
            0x00, 0x41, 0x00, 0x41, 0x03, 0xfb, 0x06, 0x20
        };
        wah_module_t module = {0};
        assert_err(wah_parse_module(&module, too_large_heap_type, sizeof(too_large_heap_type), NULL),
                   WAH_ERROR_TOO_LARGE);
        wah_free_module(&module);
    }
}

static void test_import_table_rejects_invalid_flags(void) {
    printf("Running test_import_table_rejects_invalid_flags...\n");
    wah_module_t module = {0};
    // Table import with flags=0x02 (invalid, only 0x00/0x01/0x04/0x05 are valid)
    assert_err(wah_parse_module_from_spec(&module,
        "wasm types {[fn [] []]} imports {[{'m'} {'t'} export.table funcref %'02' 0]}"),
        WAH_ERROR_MALFORMED);
    wah_free_module(&module);
}

static void test_import_table_requires_memory64_feature(void) {
    printf("Running test_import_table_requires_memory64_feature...\n");
    wah_module_t module = {0};
    wah_parse_options_t opts = { .features = WAH_FEATURE_WASM_V2 };
    // Table import with flags=0x04 (table64) should require memory64 feature
    assert_err(wah_parse_module_from_spec_ex(&module, &opts,
        "wasm types {[fn [] []]} imports {[{'m'} {'t'} export.table funcref limits.i64/1 0]}"),
        WAH_ERROR_DISABLED_FEATURE);
    wah_free_module(&module);
}

static void test_import_memory_requires_memory64_feature(void) {
    printf("Running test_import_memory_requires_memory64_feature...\n");
    wah_module_t module = {0};
    wah_parse_options_t opts = { .features = WAH_FEATURE_WASM_V2 };
    // Memory import with flags=0x04 (memory64) should require memory64 feature
    assert_err(wah_parse_module_from_spec_ex(&module, &opts,
        "wasm types {[fn [] []]} imports {[{'m'} {'mem'} export.memory limits.i64/1 0]}"),
        WAH_ERROR_DISABLED_FEATURE);
    wah_free_module(&module);
}

static void test_import_tag_requires_exception_feature(void) {
    printf("Running test_import_tag_requires_exception_feature...\n");
    wah_module_t module = {0};
    wah_parse_options_t opts = { .features = WAH_FEATURE_WASM_V2 };
    // Tag import should require exception feature
    assert_err(wah_parse_module_from_spec_ex(&module, &opts,
        "wasm types {[fn [] []]} imports {[{'m'} {'tag'} export.tag 0 0]}"),
        WAH_ERROR_DISABLED_FEATURE);
    wah_free_module(&module);
}

static void test_v128_locals_and_block_types_require_simd_feature(void) {
    printf("Running test_v128_locals_and_block_types_require_simd_feature...\n");
    wah_module_t module = {0};
    wah_parse_options_t opts = { .features = WAH_FEATURE_MVP };
    assert_err(wah_parse_module_from_spec_ex(&module, &opts,
        "wasm types {[fn [] []]} funcs {[0]} code {[{[1 v128] end}]}"),
        WAH_ERROR_DISABLED_FEATURE);
    wah_free_module(&module);
    assert_err(wah_parse_module_from_spec_ex(&module, &opts,
        "wasm types {[fn [] []]} funcs {[0]} code {[{[] block v128 unreachable end drop end}]}"),
        WAH_ERROR_DISABLED_FEATURE);
    wah_free_module(&module);
    opts.features = WAH_FEATURE_MVP | WAH_FEATURE_REF_TYPES;
    assert_err(wah_parse_module_from_spec_ex(&module, &opts,
        "wasm types {[fn [] []]} funcs {[0]} code {[{[] unreachable select.typed [v128] drop end}]}"),
        WAH_ERROR_DISABLED_FEATURE);
    wah_free_module(&module);
}

static void test_typed_select_requires_reftypes_feature(void) {
    printf("Running test_typed_select_requires_reftypes_feature...\n");
    wah_module_t module = {0};
    wah_parse_options_t opts = { .features = WAH_FEATURE_MVP };
    const char *spec = "wasm types {[fn [] []]} funcs {[0]} \
        code {[{[] i32.const 1 i32.const 2 i32.const 0 select.typed [i32] drop end}]}";
    assert_err(wah_parse_module_from_spec_ex(&module, &opts, spec), WAH_ERROR_DISABLED_FEATURE);
    wah_free_module(&module);
    opts.features = WAH_FEATURE_MVP | WAH_FEATURE_REF_TYPES;
    assert_ok(wah_parse_module_from_spec_ex(&module, &opts, spec));
    wah_free_module(&module);
}

// Reference value types used to be accepted with their features disabled.
static void test_ref_value_types_require_features(void) {
    printf("Running test_ref_value_types_require_features...\n");
    // GC and typed funcrefs imply reference types
    #define NO_REF_TYPES (WAH_FEATURE_REF_TYPES | WAH_FEATURE_GC | WAH_FEATURE_TYPED_FUNCREF)
    static const struct { const char *spec; wah_features_t missing; } cases[] = {
        { "wasm types {[fn [externref] []]}", NO_REF_TYPES },
        { "wasm types {[fn [] [funcref]]}", NO_REF_TYPES },
        { "wasm types {[fn [] []]} funcs {[0]} code {[{[1 externref] end}]}", NO_REF_TYPES },
        { "wasm types {[fn [] []]} funcs {[0]} code {[{[] block funcref unreachable end drop end}]}", NO_REF_TYPES },
        { "wasm globals {[externref immut ref.null externref end]}", NO_REF_TYPES },
        { "wasm imports {[{'m'} {'g'} global# funcref immut]}", NO_REF_TYPES },
        { "wasm tables {[externref limits.i32/1 1]}", NO_REF_TYPES },
        { "wasm imports {[{'m'} {'t'} table# externref limits.i32/1 1]}", NO_REF_TYPES },
        { "wasm types {[fn [type.ref.func] []]}", WAH_FEATURE_TYPED_FUNCREF },
        { "wasm types {[fn [] [], fn [type.ref.null 0] []]}", WAH_FEATURE_TYPED_FUNCREF },
        { "wasm types {[fn [anyref] []]}", WAH_FEATURE_GC },
        { "wasm types {[fn [type.ref.null.i31] []]}", WAH_FEATURE_GC },
        { "wasm types {[fn [type.ref.null.none] []]}", WAH_FEATURE_GC },
        { "wasm types {[fn [] []]} funcs {[0]} code {[{[1 type.ref.null.eq] end}]}", WAH_FEATURE_GC },
        { "wasm types {[fn [] []]} funcs {[0]} code {[{[] ref.null anyref drop end}]}", WAH_FEATURE_GC },
        { "wasm types {[fn [exnref] []]}", WAH_FEATURE_EXCEPTION },
        { "wasm types {[fn [type.ref.null.exn] []]}", WAH_FEATURE_EXCEPTION },
        { "wasm types {[fn [type.ref.null.exn] []]}", WAH_FEATURE_TYPED_FUNCREF },
        // Long forms of reference types are introduced by typed funcrefs, even for funcref and externref
        { "wasm types {[fn [type.ref.null.func] []]}", WAH_FEATURE_TYPED_FUNCREF },
        { "wasm types {[fn [] []]} funcs {[0]} code {[{[1 type.ref.null.extern] end}]}", WAH_FEATURE_TYPED_FUNCREF },
        { "wasm tables {[type.ref.null.func limits.i32/1 1]}", WAH_FEATURE_TYPED_FUNCREF },
        { "wasm types {[fn [] []]} funcs {[0]} code {[{[] unreachable ref.test exnref drop end}]}",
          WAH_FEATURE_EXCEPTION },
        { "wasm types {[fn [] []]} funcs {[0]} code {[{[] unreachable ref.cast.null exnref drop end}]}",
          WAH_FEATURE_EXCEPTION },
    };
    #undef NO_REF_TYPES
    for (size_t i = 0; i < sizeof(cases) / sizeof(*cases); i++) {
        wah_module_t module = {0};
        wah_parse_options_t opts = { .features = WAH_FEATURE_ALL };
        assert_ok(wah_parse_module_from_spec_ex(&module, &opts, cases[i].spec));
        wah_free_module(&module);
        opts.features = WAH_FEATURE_ALL & ~cases[i].missing;
        assert_err(wah_parse_module_from_spec_ex(&module, &opts, cases[i].spec), WAH_ERROR_DISABLED_FEATURE);
        wah_free_module(&module);
    }
}

// Section forms introduced by later proposals used to be accepted with them disabled.
static void test_section_forms_require_features(void) {
    printf("Running test_section_forms_require_features...\n");
    #define NO_REF_TYPES (WAH_FEATURE_REF_TYPES | WAH_FEATURE_GC | WAH_FEATURE_TYPED_FUNCREF)
    static const struct { const char *spec; wah_features_t missing; } cases[] = {
        { "wasm tables {[funcref limits.i32/1 1, funcref limits.i32/1 1]}", NO_REF_TYPES },
        { "wasm imports {[{'m'} {'t'} table# funcref limits.i32/1 1]} tables {[funcref limits.i32/1 1]}", NO_REF_TYPES },
        { "wasm types {[fn [] []]} funcs {[0]} elements {[elem.declarative elem.funcref [0]]} code {[{[] end}]}",
          NO_REF_TYPES },
        { "wasm types {[fn [] []]} funcs {[0]} elements {[elem.passive elem.funcref [0]]} code {[{[] end}]}",
          WAH_FEATURE_BULK_MEMORY },
        { "wasm types {[fn [] []]} funcs {[0]} tables {[funcref limits.i32/1 1]} \
           elements {[elem.active.table# 0 i32.const 0 end elem.funcref [0]]} code {[{[] end}]}",
          WAH_FEATURE_BULK_MEMORY },
        { "wasm memories {[limits.i32/1 1]} data {[data.active.table# 0 i32.const 0 end {%'00'}]}",
          WAH_FEATURE_BULK_MEMORY },
        { "wasm data {[data.passive {%'00'}]}", WAH_FEATURE_BULK_MEMORY },
        { "wasm datacount {0}", WAH_FEATURE_BULK_MEMORY },
        { "wasm types {[sub.final [] fn [] []]}", WAH_FEATURE_GC },
        { "wasm memories {[limits.i32/1 1, limits.i32/1 1]}", WAH_FEATURE_MULTI_MEMORY },
        { "wasm imports {[{'m'} {'a'} mem# limits.i32/1 1, {'m'} {'b'} mem# limits.i32/1 1]}",
          WAH_FEATURE_MULTI_MEMORY },
        { "wasm imports {[{'m'} {'a'} mem# limits.i32/1 1]} memories {[limits.i32/1 1]}", WAH_FEATURE_MULTI_MEMORY },
        // Even for memory 0, an explicit memory index is a multi-memory encoding (the align would be invalid)
        { "wasm types {[fn [] []]} funcs {[0]} memories {[limits.i32/1 1]} \
           code {[{[] i32.const 0 i32.load 66 0 0 drop end}]}", WAH_FEATURE_MULTI_MEMORY },
    };
    #undef NO_REF_TYPES
    for (size_t i = 0; i < sizeof(cases) / sizeof(*cases); i++) {
        wah_module_t module = {0};
        wah_parse_options_t opts = { .features = WAH_FEATURE_ALL };
        assert_ok(wah_parse_module_from_spec_ex(&module, &opts, cases[i].spec));
        wah_free_module(&module);
        opts.features = WAH_FEATURE_ALL & ~cases[i].missing;
        assert_err(wah_parse_module_from_spec_ex(&module, &opts, cases[i].spec), WAH_ERROR_DISABLED_FEATURE);
        wah_free_module(&module);
    }
}

// Block types as type indices are introduced by multi-value, but used to be accepted without it.
static void test_block_type_index_requires_multi_value(void) {
    printf("Running test_block_type_index_requires_multi_value...\n");
    const char *spec = "wasm types {[fn [] []]} funcs {[0]} code {[{[] block 0 end end}]}";
    wah_module_t module = {0};
    wah_parse_options_t opts = { .features = WAH_FEATURE_ALL };
    assert_ok(wah_parse_module_from_spec_ex(&module, &opts, spec));
    wah_free_module(&module);
    opts.features = WAH_FEATURE_ALL & ~WAH_FEATURE_MULTI_VALUE;
    assert_err(wah_parse_module_from_spec_ex(&module, &opts, spec), WAH_ERROR_DISABLED_FEATURE);
    wah_free_module(&module);
}

// Exporting a mutable global used to be accepted without the mutable globals feature.
static void test_mutable_global_export_requires_feature(void) {
    printf("Running test_mutable_global_export_requires_feature...\n");
    const char *spec = "wasm globals {[i32 mut i32.const 0 end]} exports {[{'g'} global# 0]}";
    wah_module_t module = {0};
    wah_parse_options_t opts = { .features = WAH_FEATURE_ALL };
    assert_ok(wah_parse_module_from_spec_ex(&module, &opts, spec));
    wah_free_module(&module);
    opts.features = WAH_FEATURE_ALL & ~WAH_FEATURE_MUTABLE_GLOBALS;
    assert_err(wah_parse_module_from_spec_ex(&module, &opts, spec), WAH_ERROR_DISABLED_FEATURE);
    wah_free_module(&module);
}

// Tables with init expressions used to be accepted without the typed function references feature.
static void test_table_init_expr_requires_feature(void) {
    printf("Running test_table_init_expr_requires_feature...\n");
    const char *spec = "wasm tables {[ %'4000' funcref limits.i32/1 1 ref.null funcref end ]}";
    wah_module_t module = {0};
    wah_parse_options_t opts = { .features = WAH_FEATURE_ALL };
    assert_ok(wah_parse_module_from_spec_ex(&module, &opts, spec));
    wah_free_module(&module);
    opts.features = WAH_FEATURE_ALL & ~WAH_FEATURE_TYPED_FUNCREF;
    assert_err(wah_parse_module_from_spec_ex(&module, &opts, spec), WAH_ERROR_DISABLED_FEATURE);
    wah_free_module(&module);
}

// Constant expressions reading defined (not imported) globals used to be accepted without GC.
static void test_const_expr_defined_global_requires_gc(void) {
    printf("Running test_const_expr_defined_global_requires_gc...\n");
    const char *spec = "wasm globals {[ i32 immut i32.const 1 end, i32 immut global.get 0 end ]}";
    wah_module_t module = {0};
    wah_parse_options_t opts = { .features = WAH_FEATURE_ALL };
    assert_ok(wah_parse_module_from_spec_ex(&module, &opts, spec));
    wah_free_module(&module);
    opts.features = WAH_FEATURE_ALL & ~WAH_FEATURE_GC;
    assert_err(wah_parse_module_from_spec_ex(&module, &opts, spec), WAH_ERROR_DISABLED_FEATURE);
    wah_free_module(&module);
    // Imported globals are fine without GC
    assert_ok(wah_parse_module_from_spec_ex(&module, &opts, "wasm \
        imports {[ {'a'} {'b'} global# i32 immut ]} globals {[ i32 immut global.get 0 end ]}"));
    wah_free_module(&module);
}

// Limits are u64 even for i32 tables, whose range is checked by validation, like memories.
static void test_i32_table_limits_are_u64(void) {
    printf("Running test_i32_table_limits_are_u64...\n");
    wah_module_t module = {0};
    assert_err(wah_parse_module_from_spec(&module, "wasm tables {[ funcref %'00' %'8080808010' ]}"),
               WAH_ERROR_VALIDATION_FAILED);
    assert_err(wah_parse_module_from_spec(&module, "wasm tables {[ funcref %'01' 0 %'8080808010' ]}"),
               WAH_ERROR_VALIDATION_FAILED);
    assert_err(wah_parse_module_from_spec(&module, "wasm \
        imports {[ {'a'} {'b'} table# funcref %'00' %'8080808010' ]}"), WAH_ERROR_VALIDATION_FAILED);
    assert_ok(wah_parse_module_from_spec(&module, "wasm tables {[ funcref %'01' 0 %'ffffffff0f' ]}"));
    wah_free_module(&module);
}

// Tracks the peak of outstanding allocation bytes.
typedef struct { size_t cur, peak; } peak_alloc_t;

static void *peak_malloc(size_t size, void *ud) {
    peak_alloc_t *st = (peak_alloc_t *)ud;
    size_t *p = (size_t *)malloc(sizeof(size_t) * 2 + size);
    if (!p) return NULL;
    p[0] = size;
    st->cur += size;
    if (st->cur > st->peak) st->peak = st->cur;
    return p + 2;
}
static void peak_free(void *ptr, void *ud) {
    size_t *p = (size_t *)ptr - 2;
    ((peak_alloc_t *)ud)->cur -= p[0];
    free(p);
}
static void *peak_realloc(void *ptr, size_t size, void *ud) {
    size_t *p = (size_t *)ptr - 2;
    void *np = peak_malloc(size, ud);
    if (!np) return NULL;
    memcpy(np, ptr, p[0] < size ? p[0] : size);
    peak_free(ptr, ud);
    return np;
}

static size_t parse_peak_bytes(const char *spec) {
    peak_alloc_t st = {0};
    wah_alloc_t alloc = { peak_malloc, peak_realloc, peak_free, &st, 0 };
    wah_parse_options_t opts = { .alloc = &alloc };
    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec_ex(&module, &opts, spec));
    wah_free_module(&module);
    assert_eq_u64(st.cur, 0);
    return st.peak;
}

// Many locals are declared in a few bytes, so they shouldn't be expanded one by one.
static void test_local_decls_memory_amplification(void) {
    printf("Running test_local_decls_memory_amplification...\n");
    enum { N = 64 };
    static char spec[256 + N * 32];
    strcpy(spec, "wasm types {[fn [] []]} funcs {[0");
    for (int i = 1; i < N; i++) strcat(spec, ",0");
    strcat(spec, "]} code {[{[65535 i32] end}");
    for (int i = 1; i < N; i++) strcat(spec, ",{[65535 i32] end}");
    strcat(spec, "]}");
    // Expanded local types would take N * 256 KB = 16 MB
    assert_true(parse_peak_bytes(spec) < 2 * 1024 * 1024);
}

// Each type used to have a bitset over all GC types for casts, taking memory quadratic to the type count.
static void test_cast_metadata_memory_amplification(void) {
    printf("Running test_cast_metadata_memory_amplification...\n");
    enum { N = 10000 };
    static char spec[64 + N * 12];
    static const char head[] = "wasm types {[struct []", item[] = ",struct []";
    char *p = spec;
    memcpy(p, head, sizeof(head) - 1); p += sizeof(head) - 1;
    for (int i = 1; i < N; i++) { memcpy(p, item, sizeof(item) - 1); p += sizeof(item) - 1; }
    strcpy(p, "]}");
    // Bitsets would take N * N / 8 = 12.5 MB
    assert_true(parse_peak_bytes(spec) < 4 * 1024 * 1024);
}

// Out-of-range type indices in global, table and element types used to be rejected only after all sections were parsed,
// so a later malformed section (here a duplicate type section) took precedence.
static void test_early_type_index_rejection(void) {
    printf("Running test_early_type_index_rejection...\n");
    static const char *const specs[] = {
        "wasm imports {[{'m'} {'g'} global# type.ref.null 5 immut]} types {[]}",
        "wasm imports {[{'m'} {'t'} table# type.ref.null 5 limits.i32/1 1]} types {[]}",
        "wasm tables {[type.ref.null 5 limits.i32/1 1]} types {[]}",
        "wasm elements {[elem.passive.expr type.ref.null 5 []]} types {[]}",
    };
    for (size_t i = 0; i < sizeof(specs) / sizeof(*specs); i++) {
        wah_module_t module = {0};
        assert_err(wah_parse_module_from_spec(&module, specs[i]), WAH_ERROR_VALIDATION_FAILED);
        wah_free_module(&module);
    }
}

// struct.new_default (and struct.new in unreachable code) used to loop over all fields of a wide struct.
static double parse_wide_struct_news_seconds(const char *body) {
    enum { N = 50000 };
    static const char head[] = "wasm types {[struct [i32 immut", field[] = ",i32 immut",
        mid[] = "], fn [] []]} funcs {[1]} code {[{[] unreachable";
    size_t body_len = strlen(body);
    char *spec = (char *)malloc(128 + (size_t)N * (sizeof(field) + body_len));
    assert_true(spec != NULL);
    char *p = spec;
    memcpy(p, head, sizeof(head) - 1); p += sizeof(head) - 1;
    for (int i = 1; i < N; i++) { memcpy(p, field, sizeof(field) - 1); p += sizeof(field) - 1; }
    memcpy(p, mid, sizeof(mid) - 1); p += sizeof(mid) - 1;
    for (int i = 0; i < N; i++) { memcpy(p, body, body_len); p += body_len; }
    strcpy(p, " end}]}");
    wah_module_t module = {0};
    clock_t start = clock();
    assert_ok(wah_parse_module_from_spec(&module, spec));
    double elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;
    wah_free_module(&module);
    free(spec);
    return elapsed;
}

static void test_wide_struct_new_validation_time(void) {
    printf("Running test_wide_struct_new_validation_time...\n");
    double new_default = parse_wide_struct_news_seconds(" struct.new_default 0 drop");
    double new_unreachable = parse_wide_struct_news_seconds(" struct.new 0 drop");
    printf("  struct.new_default: %.3fs, struct.new: %.3fs\n", new_default, new_unreachable);
    assert_true(new_default < 1.0); // Would take 2.5G steps otherwise
    assert_true(new_unreachable < 1.0);
}

// Distinct rec groups used to be compared against all previous ones of the same size.
static double parse_distinct_func_types_seconds(int n) {
    // Type i = fn (ref null i-1) -> (), all distinct
    char *spec = (char *)malloc(64 + (size_t)n * 40);
    assert_true(spec != NULL);
    char *p = spec + snprintf(spec, 64, "wasm types {[fn [] []");
    for (int i = 1; i < n; i++) p += snprintf(p, 40, ",fn [type.ref.null %d] []", i - 1);
    strcpy(p, "]}");
    wah_module_t module = {0};
    clock_t start = clock();
    assert_ok(wah_parse_module_from_spec(&module, spec));
    double elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;
    wah_free_module(&module);
    free(spec);
    return elapsed;
}

static void test_rec_group_canonicalization_time(void) {
    printf("Running test_rec_group_canonicalization_time...\n");
    double small = parse_distinct_func_types_seconds(10000), large = parse_distinct_func_types_seconds(40000);
    printf("  10000 types: %.3fs, 40000 types: %.3fs\n", small, large);
    assert_true(large < small * 8 + 0.1); // Quadratic would be 16x
}

// Entering a block used to copy the initialization states of all locals.
static double parse_many_blocks_seconds(const char *locals) {
    enum { N = 200000 };
    static const char block[] = " block void end";
    char *spec = (char *)malloc(256 + N * sizeof(block));
    assert_true(spec != NULL);
    char *p = spec + snprintf(spec, 256, "wasm types {[fn [] []]} funcs {[0]} code {[{[%s]", locals);
    for (int i = 0; i < N; i++) { memcpy(p, block, sizeof(block) - 1); p += sizeof(block) - 1; }
    strcpy(p, " end}]}");
    wah_module_t module = {0};
    clock_t start = clock();
    assert_ok(wah_parse_module_from_spec(&module, spec));
    double elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;
    wah_free_module(&module);
    free(spec);
    return elapsed;
}

static void test_block_entry_with_many_locals_time(void) {
    printf("Running test_block_entry_with_many_locals_time...\n");
    double few = parse_many_blocks_seconds("1 type.ref 0");
    double many = parse_many_blocks_seconds("1 type.ref 0, 65534 i32");
    printf("  1 local: %.3fs, 65535 locals: %.3fs\n", few, many);
    assert_true(many < few * 1.8 + 0.05); // Used to be about 3x
}

// Each POLL used to store a full bitmap and types of all references on the operand stack.
static void test_poll_ref_map_memory_amplification(void) {
    printf("Running test_poll_ref_map_memory_amplification...\n");
    enum { REFS = 1000, LOOPS = 10000 };
    static const char ref[] = " ref.null anyref", loop[] = " loop void end", alt[] = " i32.const 0 loop void end drop";
    char *spec = (char *)malloc(256 + REFS * sizeof(ref) + LOOPS * sizeof(alt));
    assert_true(spec != NULL);
    for (int k = 0; k < 2; k++) { // Same or alternating stack states
        char *p = spec + snprintf(spec, 256, "wasm types {[fn [] []]} funcs {[0]} code {[{[]");
        for (int i = 0; i < REFS; i++) { memcpy(p, ref, sizeof(ref) - 1); p += sizeof(ref) - 1; }
        for (int i = 0; i < LOOPS; i++) {
            if (k) { memcpy(p, alt, sizeof(alt) - 1); p += sizeof(alt) - 1; }
            else { memcpy(p, loop, sizeof(loop) - 1); p += sizeof(loop) - 1; }
        }
        strcpy(p, " return end}]}");
        // Full ref maps would take LOOPS * ~4 KB = 40 MB
        assert_true(parse_peak_bytes(spec) < 4 * 1024 * 1024);
    }
    free(spec);
}

// Tracking initialization of non-defaultable locals shouldn't take memory proportional to the nesting.
static void test_local_init_tracking_memory_amplification(void) {
    printf("Running test_local_init_tracking_memory_amplification...\n");
    // Preallocating for the maximum control depth would take 64K * 256 = 16 MB
    assert_true(parse_peak_bytes("wasm types {[fn [] []]} funcs {[0]} \
        code {[{[1 type.ref 0, 65534 i32] block void block void end end end}]}") < 2 * 1024 * 1024);
}

int main(void) {
    test_local_decls_memory_amplification();
    test_poll_ref_map_memory_amplification();
    test_block_entry_with_many_locals_time();
    test_rec_group_canonicalization_time();
    test_wide_struct_new_validation_time();
    test_early_type_index_rejection();
    test_cast_metadata_memory_amplification();
    test_local_init_tracking_memory_amplification();
    test_v128_locals_and_block_types_require_simd_feature();
    test_typed_select_requires_reftypes_feature();
    test_ref_value_types_require_features();
    test_section_forms_require_features();
    test_block_type_index_requires_multi_value();
    test_mutable_global_export_requires_feature();
    test_table_init_expr_requires_feature();
    test_const_expr_defined_global_requires_gc();
    test_i32_table_limits_are_u64();
    test_parse_module_argument_errors();
    test_zero_params_zero_results_func_type();
    test_invalid_section_order_mem_table();
    test_invalid_element_segment_func_idx();
    test_code_section_no_function_section();
    test_function_section_no_code_section();
    test_parse_data_no_datacount_memory_init_fails();
    test_deferred_data_validation_failure();
    test_unused_opcode_validation_failure();
    test_datacount_no_data_section();
    test_end_opcode_not_at_end();
    test_multiple_end_opcodes();
    test_malformed_code_body_size_wasm();

    test_call_out_of_bounds_func_idx();
    test_call_indirect_out_of_bounds_type_idx();
    test_call_indirect_out_of_bounds_table_idx();
    test_table_get_out_of_bounds_table_idx();
    test_table_set_out_of_bounds_table_idx();
    test_table_size_out_of_bounds_table_idx();
    test_table_grow_out_of_bounds_table_idx();
    test_table_fill_out_of_bounds_table_idx();
    test_table_copy_out_of_bounds_dst_table_idx();
    test_table_copy_out_of_bounds_src_table_idx();
    test_table_init_out_of_bounds_elem_idx();
    test_table_init_out_of_bounds_table_idx();
    test_elem_drop_out_of_bounds_elem_idx();
    test_i8x16_extract_lane_s_out_of_bounds_lane_idx();
    test_i32x4_extract_lane_out_of_bounds_lane_idx();
    test_v128_load8_lane_out_of_bounds_lane_idx();
    test_v128_load32_lane_out_of_bounds_lane_idx();
    test_memory_init_out_of_bounds_mem_idx();
    test_memory_init_out_of_bounds_data_idx();
    test_memory_copy_out_of_bounds_dest_mem_idx();
    test_memory_copy_out_of_bounds_src_mem_idx();

    test_all_hang_wasm_parsing_errors();
    test_deep_const_expr_stack_instantiation();
    test_control_frame_cleanup_after_block_type_eof();
    test_reject_huge_local_count_before_allocation();
    test_reject_huge_function_type_counts_before_allocation();
    test_reject_huge_br_table_count_before_allocation();
    test_truncated_i8x16_shuffle_immediate();
    test_reject_huge_element_count_before_allocation();
    test_reject_huge_memory_min_with_exec_limit();
    test_unreachable_array_new_fixed_huge_length();

    test_reject_huge_rec_group_count();
    test_overlong_sleb128_i64();
    test_start_function_type();
    test_elem_oob_table_idx();
    test_unknown_export_kind();
    test_unknown_element_segment_flags();
    test_unknown_data_segment_flags();
    test_count_overflow();
    test_fuzz_ref_validation_regressions();

    test_import_table_rejects_invalid_flags();
    test_import_table_requires_memory64_feature();
    test_import_memory_requires_memory64_feature();
    test_import_tag_requires_exception_feature();

    printf("All parser tests passed!\n");
    return 0;
}
