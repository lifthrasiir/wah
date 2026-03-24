#include <stdio.h>
#include <string.h>
#include <assert.h>

#define WAH_IMPLEMENTATION
#include "wah.h"
#include "wah_testutils.c"

static void test_malformed_code_body_size_wasm() {
    printf("Running test_malformed_code_body_size_wasm...\n");
    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

#define REPEAT10(x) x x x x x x x x x x
#define REPEAT100(x) REPEAT10(REPEAT10(x))
#define REPEAT1000(x) REPEAT10(REPEAT10(REPEAT10(x)))

    // This WASM module is known to cause a validation failure due to an oversized code body.
    const char *wasm_spec = "wasm \
        types {[ fn [i32 i32] [i32] ]} \
        funcs {[ 0 ]} \
        exports {[ {'000000'} fn# 0 ]} \
        code %'3001aefffa308d8e3030303030" \
        REPEAT1000("30") REPEAT1000("30") REPEAT1000("30") REPEAT1000("30") \
        REPEAT10("30") REPEAT10("30") REPEAT10("30") REPEAT10("30") REPEAT10("30") "'";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_VALIDATION_FAILED);
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

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_VALIDATION_FAILED);
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
    // This should result in WAH_ERROR_VALIDATION_FAILED because module->function_count will be 0
    // but the code section count will be > 0.
    const char *wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        code {[ {[] end} ]}";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);
}

void test_function_section_no_code_section() {
    printf("Running test_function_section_no_code_section...\n");

    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    // This WASM module has a function section but no code section.
    // This should result in WAH_ERROR_VALIDATION_FAILED because module->function_count will be > 0
    // but module->code_count will be 0.
    const char *wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]}";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);
}

// Test case for parsing a module with a data section but no datacount section,
// and a code section using memory.init. This should fail validation currently.
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

    assert_ok(wah_parse_module_from_spec(&module, wasm_spec));
    wah_free_module(&module);
}

// Test case for deferred data segment validation failure.
// No datacount section, one data segment (index 0), but memory.init tries to use data_idx 1.
static void test_deferred_data_validation_failure() {
    printf("Running test_deferred_data_validation_failure...\n");

    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    const char *wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        memories {[ limits.i32/1 1 ]} \
        exports {[ {'memory'} mem# 0, {'test_func'} fn# 0 ]} \
        code {[ {[] i32.const 0 i32.const 0 i32.const 5 memory.init 1 0 end } ]} \
        data {[ data.active.table#0 i32.const 0 end {'hello'} ]}";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);
}

// Test case for an unused opcode (0x09) in the code section
static void test_unused_opcode_validation_failure() {
    printf("Running test_unused_opcode_validation_failure...\n");

    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    const char *wasm_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        code {[ {[] %'09' end} ]}";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);
}

// Test case for datacount section present, but no data section.
// This should result in WAH_ERROR_VALIDATION_FAILED.
static void test_datacount_no_data_section() {
    printf("Running test_datacount_no_data_section...\n");
    wah_module_t module;
    memset(&module, 0, sizeof(wah_module_t));

    const char *wasm_spec = "wasm \
        types {[ fn [i32] [i32] ]} \
        datacount { 2 }";

    assert_err(wah_parse_module_from_spec(&module, wasm_spec), WAH_ERROR_VALIDATION_FAILED);
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
        data {[ data.passive {'hello'} ]} \
        code {[ {[] i32.const 0 i32.const 0 i32.const 5 memory.init 0 1 end} ]}";

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
        data {[ data.passive {'hello'} ]} \
        code {[ {[] i32.const 0 i32.const 0 i32.const 5 memory.init 1 0 end} ]}";

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

        wah_error_t err = wah_parse_module(hang_tests[i].binary, hang_tests[i].len, &module);

        if (err == WAH_ERROR_VALIDATION_FAILED) {
            printf("  - PASSED: %s correctly returned WAH_ERROR_VALIDATION_FAILED.\n", hang_tests[i].name);
            wah_free_module(&module);
        } else if (err == WAH_ERROR_TOO_LARGE) {
            printf("  - PASSED: %s correctly returned WAH_ERROR_TOO_LARGE.\n", hang_tests[i].name);
            wah_free_module(&module);
        } else if (err == WAH_ERROR_UNEXPECTED_EOF) {
            printf("  - PASSED: %s correctly returned WAH_ERROR_UNEXPECTED_EOF.\n", hang_tests[i].name);
            wah_free_module(&module);
        } else {
            fprintf(stderr, "Assertion failed: Expected WAH_ERROR_VALIDATION_FAILED, WAH_ERROR_TOO_LARGE, or WAH_ERROR_UNEXPECTED_EOF for %s, but got %s\n", hang_tests[i].name, wah_strerror(err));
            exit(1);
        }
    }
}

int main(void) {
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

    printf("All parser tests passed!\n");
    return 0;
}
