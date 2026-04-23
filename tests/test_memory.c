#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define WAH_IMPLEMENTATION
#include "../wah.h"
#include "common.c"

void wah_test_data_and_bulk_memory_ops() {
    wah_module_t module;
    wah_exec_context_t ctx;
    wah_value_t params[3]; // params array size is 3, as max args for any function is 3
    wah_value_t result;

    printf("Running data segments and bulk memory operations tests...\n");

    const char *wasm_data_and_bulk_memory_spec = "wasm \
        types {[ \
            fn [i32, i32, i32] [], \
            fn [i32] [i32], \
        ]} \
        funcs {[ 0, 0, 0, 1 ]} \
        memories {[ limits.i32/1 1 ]} \
        exports {[ \
            {'init_mem_data0'} fn# 0, \
            {'init_mem_data1'} fn# 1, \
            {'copy_mem'} fn# 2, \
            {'get_byte'} fn# 3, \
        ]} \
        datacount { 2 } \
        code {[ \
            {[] local.get 0 local.get 1 local.get 2 memory.init 0 0 end}, \
            {[] local.get 0 local.get 1 local.get 2 memory.init 1 0 end}, \
            {[] local.get 0 local.get 1 local.get 2 memory.copy 0 0 end}, \
            {[] local.get 0 i32.load8_u align=1 offset=0 end}, \
        ]} \
        data {[ \
            data.active.table#0 i32.const 0 end {%'01020304'}, \
            data.passive {%'05060708'}, \
        ]}";

    // Test 1: Parse module using DSL
    assert_ok(wah_parse_module_from_spec(&module, wasm_data_and_bulk_memory_spec));
    assert_eq_u32(module.memory_count, 1);
    assert_eq_u32(module.data_segment_count, 2);
    assert_true(module.has_data_count_section == true);

    // Test 2: Create and instantiate execution context
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_true(ctx.memories[0] != NULL);
    assert_eq_u32(ctx.memory_sizes[0], WAH_WASM_PAGE_SIZE);
    assert_ok(wah_instantiate(&ctx));

    // Verify initial memory state (active data segment 0 should be initialized)
    assert_eq_u32(ctx.memories[0][0], 0x01);
    assert_eq_u32(ctx.memories[0][1], 0x02);
    assert_eq_u32(ctx.memories[0][2], 0x03);
    assert_eq_u32(ctx.memories[0][3], 0x04);
    assert_eq_u32(ctx.memories[0][4], 0x00);

    // Test 3: memory.init - initialize data segment 0 at memory offset 100, data segment offset 0
    uint32_t dest_offset_1 = 100;
    uint32_t src_offset_1 = 0;
    uint32_t size_1 = 4;
    params[0].i32 = dest_offset_1; // dest_offset
    params[1].i32 = src_offset_1;  // src_offset
    params[2].i32 = size_1;        // size
    assert_ok(wah_call(&ctx, 0, params, 3, NULL)); // Call init_mem_data0 (func 0)

    // Verify memory contents after memory.init
    params[0].i32 = dest_offset_1;
    assert_ok(wah_call(&ctx, 3, params, 1, &result)); // Call get_byte (func 3)
    assert_eq_i32(result.i32, 0x01);
    params[0].i32 = dest_offset_1 + 1;
    assert_ok(wah_call(&ctx, 3, params, 1, &result));
    assert_eq_i32(result.i32, 0x02);

    // Test 4: memory.init - initialize data segment 1 at memory offset 110, data segment offset 1
    uint32_t dest_offset_2 = 110;
    uint32_t src_offset_2 = 1; // Start from the second byte of data segment 1 (value 0x06)
    uint32_t size_2 = 2; // Copy 2 bytes (0x06, 0x07)
    params[0].i32 = dest_offset_2; // dest_offset
    params[1].i32 = src_offset_2;  // src_offset
    params[2].i32 = size_2;        // size
    assert_ok(wah_call(&ctx, 1, params, 3, NULL)); // Call init_mem_data1 (func 1)

    // Verify memory contents after memory.init with non-zero src_offset
    params[0].i32 = dest_offset_2;
    assert_ok(wah_call(&ctx, 3, params, 1, &result)); // Call get_byte (func 3)
    assert_eq_i32(result.i32, 0x06);
    params[0].i32 = dest_offset_2 + 1;
    assert_ok(wah_call(&ctx, 3, params, 1, &result));
    assert_eq_i32(result.i32, 0x07);

    // Test 5: memory.copy - copy 4 bytes from offset 100 to offset 200
    uint32_t copy_dest = 200;
    uint32_t copy_src = 100;
    uint32_t copy_size = 4;
    params[0].i32 = copy_dest; // dest
    params[1].i32 = copy_src; // src
    params[2].i32 = copy_size; // size
    assert_ok(wah_call(&ctx, 2, params, 3, NULL)); // Call copy_mem (func 2)

    // Verify memory contents after memory.copy
    params[0].i32 = copy_dest;
    assert_ok(wah_call(&ctx, 3, params, 1, &result)); // Call get_byte (func 3)
    assert_eq_i32(result.i32, 0x01);
    params[0].i32 = copy_dest + 1;
    assert_ok(wah_call(&ctx, 3, params, 1, &result));
    assert_eq_i32(result.i32, 0x02);

    // Test 6: memory.init - out of bounds (dest_offset + size > memory_size)
    params[0].i32 = WAH_WASM_PAGE_SIZE - 2; // dest_offset (2 bytes before end)
    params[1].i32 = 0; // src_offset
    params[2].i32 = 4; // size (will go out of bounds)
    assert_err(wah_call(&ctx, 0, params, 3, NULL), WAH_ERROR_MEMORY_OUT_OF_BOUNDS);

    // Test 7: memory.init - out of bounds (src_offset + size > segment->data_len)
    params[0].i32 = 0; // dest_offset
    params[1].i32 = 3; // src_offset (data segment 0 has 4 bytes, 3 + 2 = 5 > 4)
    params[2].i32 = 2; // size
    assert_err(wah_call(&ctx, 0, params, 3, NULL), WAH_ERROR_TRAP);

    // Test 8: memory.copy - out of bounds (dest)
    params[0].i32 = WAH_WASM_PAGE_SIZE - 2; // dest
    params[1].i32 = 0; // src
    params[2].i32 = 4; // size
    assert_err(wah_call(&ctx, 2, params, 3, NULL), WAH_ERROR_MEMORY_OUT_OF_BOUNDS);

    // Test 9: memory.copy - out of bounds (src)
    params[0].i32 = 0; // dest
    params[1].i32 = WAH_WASM_PAGE_SIZE - 2; // src
    params[2].i32 = 4; // size
    assert_err(wah_call(&ctx, 2, params, 3, NULL), WAH_ERROR_MEMORY_OUT_OF_BOUNDS);

    // Final Cleanup
    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

void test_memory_api() {
    wah_module_t module;
    wah_exec_context_t ctx;
    wah_value_t params[3];
    wah_value_t result;

    printf("Running memory tests...\n");

    // Test 1: Parse module
    const char *wasm_memory_spec = "wasm \
        types {[ \
            fn [i32, i32] [], \
            fn [i32] [i32], \
        ]} \
        funcs {[ 0, 1 ]} \
        memories {[ limits.i32/1 1 ]} \
        exports {[ \
            {'store'} fn# 0, \
            {'load'} fn# 1, \
        ]} \
        code {[ \
            {[] local.get 0 local.get 1 i32.store align=4 offset=0 end}, \
            {[] local.get 0 i32.load align=4 offset=0 end}, \
        ]}";
    assert_ok(wah_parse_module_from_spec(&module, wasm_memory_spec));
    assert_eq_u32(module.memory_count, 1);
    assert_eq_u64(module.memories[0].min_pages, 1);

    // Test 2: Create execution context
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_true(ctx.memories[0] != NULL);
    assert_eq_u32(ctx.memory_sizes[0], WAH_WASM_PAGE_SIZE);

    // Test 3: Store a value
    uint32_t test_address = 1024;
    int32_t test_value = 0xDEADBEEF;
    params[0].i32 = test_address;
    params[1].i32 = test_value;
    assert_ok(wah_call(&ctx, 0, params, 2, NULL)); // Call store_val (func 0)

    // Verify directly in memory
    int32_t *mem_ptr = (int32_t*)(ctx.memories[0] + test_address);
    assert_eq_i32(*mem_ptr, test_value);

    // Test 4: Load the value
    params[0].i32 = test_address;
    assert_ok(wah_call(&ctx, 1, params, 1, &result)); // Call load_val (func 1)
    assert_eq_i32(result.i32, test_value);

    // Test 5: Memory out-of-bounds store
    uint32_t oob_address_store = WAH_WASM_PAGE_SIZE - 2; // 2 bytes before end, trying to store 4 bytes
    params[0].i32 = oob_address_store;
    params[1].i32 = 0x12345678;
    assert_err(wah_call(&ctx, 0, params, 2, NULL), WAH_ERROR_MEMORY_OUT_OF_BOUNDS);

    // Test 6: Memory out-of-bounds load
    uint32_t oob_address_load = WAH_WASM_PAGE_SIZE - 2; // 2 bytes before end, trying to load 4 bytes
    params[0].i32 = oob_address_load;
    assert_err(wah_call(&ctx, 1, params, 1, &result), WAH_ERROR_MEMORY_OUT_OF_BOUNDS);

    // Test 7: Offset overflow with wrap-around
    uint32_t base_addr_overflow = 0xFFFFFFF0; // A high address
    uint32_t offset_overflow = 0x20;         // An offset that causes overflow
    uint32_t expected_effective_addr = (base_addr_overflow + offset_overflow); // Expected wrapped address

    // Ensure the expected_effective_addr is within bounds for our test
    // We want it to wrap around to a valid address within the first page
    assert_true(expected_effective_addr < WAH_WASM_PAGE_SIZE - 4 && "Expected effective address out of test bounds");

    int32_t test_value_overflow = 0xCAFEBABE;

    // Store the value using the overflowed address
    params[0].i32 = base_addr_overflow;
    params[1].i32 = test_value_overflow;
    assert_err(wah_call(&ctx, 0, params, 2, NULL), WAH_ERROR_MEMORY_OUT_OF_BOUNDS);

    // Load the value using the overflowed address
    params[0].i32 = base_addr_overflow;
    assert_err(wah_call(&ctx, 1, params, 1, &result), WAH_ERROR_MEMORY_OUT_OF_BOUNDS);

    // Cleanup for first module
    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

void test_memory_ops() {
    wah_module_t module;
    wah_exec_context_t ctx;
    wah_value_t params[3];
    wah_value_t result;

    printf("Running memory operations tests (size, grow, fill)...\n");

    // Test 8: Parse new module for memory operations
    const char* wasm_memory_spec_2 = "wasm \
        types {[ \
            fn [] [i32], \
            fn [i32] [i32], \
            fn [i32, i32, i32] [], \
        ]} \
        funcs {[ 0, 1, 2 ]} \
        memories {[ limits.i32/2 1 2 ]} \
        exports {[ \
            {'mem'} mem# 0, \
            {'get_memory_size'} fn# 0, \
            {'grow_memory'} fn# 1, \
            {'fill_memory'} fn# 2, \
        ]} \
        code {[ \
            {[] memory.size 0 end}, \
            {[] local.get 0 memory.grow 0 end}, \
            {[] local.get 0 local.get 1 local.get 2 memory.fill 0 end}, \
        ]}";
    assert_ok(wah_parse_module_from_spec(&module, wasm_memory_spec_2));
    assert_eq_u32(module.memory_count, 1);
    assert_eq_u64(module.memories[0].min_pages, 1);
    assert_eq_u64(module.memories[0].max_pages, 2);

    // Test 9: Create execution context for memory operations
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_true(ctx.memories[0] != NULL);
    assert_eq_u32(ctx.memory_sizes[0], WAH_WASM_PAGE_SIZE);

    // Test 10: memory.size - initial
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result)); // Call get_memory_size (func 0)
    assert_eq_i32(result.i32, 1);

    // Test 11: memory.grow - success
    params[0].i32 = 1; // Grow by 1 page
    assert_ok(wah_call(&ctx, 1, params, 1, &result)); // Call grow_memory (func 1)
    assert_eq_i32(result.i32, 1);
    assert_eq_u32(ctx.memory_sizes[0], 2 * WAH_WASM_PAGE_SIZE);

    // Test 12: memory.size - after grow
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result)); // Call get_memory_size (func 0)
    assert_eq_i32(result.i32, 2);

    // Test 13: memory.grow - failure (exceed max_pages)
    params[0].i32 = 1; // Grow by 1 page (total 3, max 2)
    assert_ok(wah_call(&ctx, 1, params, 1, &result)); // Should return -1, not trap
    assert_eq_i32(result.i32, -1);
    assert_eq_u32(ctx.memory_sizes[0], 2 * WAH_WASM_PAGE_SIZE);

    // Test 14: memory.fill - basic
    uint32_t fill_offset = 100;
    uint8_t fill_value = 0xAA;
    uint32_t fill_size = 256;
    params[0].i32 = fill_offset; // offset
    params[1].i32 = fill_value;  // value
    params[2].i32 = fill_size;   // size
    assert_ok(wah_call(&ctx, 2, params, 3, NULL)); // Call fill_memory (func 2)

    // Verify filled memory directly
    for (uint32_t i = 0; i < fill_size; ++i) {
        assert_eq_u32(ctx.memories[0][fill_offset + i], fill_value);
    }

    // Test 15: memory.fill - out of bounds
    uint32_t oob_fill_offset = ctx.memory_sizes[0] - 100; // Near end of memory
    uint32_t oob_fill_size = 200; // Will go out of bounds
    params[0].i32 = oob_fill_offset;
    params[1].i32 = 0xBB;
    params[2].i32 = oob_fill_size;
    assert_err(wah_call(&ctx, 2, params, 3, NULL), WAH_ERROR_MEMORY_OUT_OF_BOUNDS);

    // Final Cleanup
    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

void test_multiple_memories() {
    wah_module_t module;
    wah_exec_context_t ctx;
    wah_value_t params[3];
    wah_value_t result;

    printf("Running multiple memories tests...\n");

    // Module with 2 memories:
    //   Memory 0: 1 page (no max)
    //   Memory 1: 1 page, max 2 pages
    const char *spec = "wasm \
        types {[ \
            fn [i32, i32] [], \
            fn [i32] [i32], \
            fn [] [i32], \
            fn [i32] [i32], \
            fn [i32, i32, i32] [], \
        ]} \
        funcs {[ 0, 1, 0, 1, 2, 2, 3, 4 ]} \
        memories {[ limits.i32/1 1, limits.i32/2 1 2 ]} \
        exports {[ \
            {'store0'} fn# 0, \
            {'load0'}  fn# 1, \
            {'store1'} fn# 2, \
            {'load1'}  fn# 3, \
            {'size0'}  fn# 4, \
            {'size1'}  fn# 5, \
            {'grow1'}  fn# 6, \
            {'copy1to0'} fn# 7, \
        ]} \
        code {[ \
            {[] local.get 0 local.get 1 i32.store align=4 offset=0 end}, \
            {[] local.get 0 i32.load align=4 offset=0 end}, \
            {[] local.get 0 local.get 1 i32.store align=4.mem# 1 offset=0 end}, \
            {[] local.get 0 i32.load align=4.mem# 1 offset=0 end}, \
            {[] memory.size 0 end}, \
            {[] memory.size 1 end}, \
            {[] local.get 0 memory.grow 1 end}, \
            {[] local.get 0 local.get 1 local.get 2 memory.copy 0 1 end}, \
        ]}";

    assert_ok(wah_parse_module_from_spec(&module, spec));
    assert_eq_u32(module.memory_count, 2);

    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_true(ctx.memories[0] != NULL);
    assert_true(ctx.memories[1] != NULL);
    assert_eq_u32(ctx.memory_count, 2);
    assert_eq_u32(ctx.memory_sizes[0], WAH_WASM_PAGE_SIZE);
    assert_eq_u32(ctx.memory_sizes[1], WAH_WASM_PAGE_SIZE);
    // memory_base/memory_size must be aliases to memory 0
    assert_true(ctx.memory_base == ctx.memories[0]);
    assert_eq_u32(ctx.memory_size, ctx.memory_sizes[0]);

    // Test 1: store to memory 0, verify memory 1 is unaffected
    params[0].i32 = 100; params[1].i32 = 0xAABBCCDD;
    assert_ok(wah_call(&ctx, 0, params, 2, NULL));
    assert_eq_i32(*(int32_t*)(ctx.memories[0] + 100), (int32_t)0xAABBCCDD);
    assert_eq_i32(*(int32_t*)(ctx.memories[1] + 100), 0);

    // Test 2: store to memory 1, verify memory 0 is unaffected
    params[0].i32 = 100; params[1].i32 = 0x11223344;
    assert_ok(wah_call(&ctx, 2, params, 2, NULL));
    assert_eq_i32(*(int32_t*)(ctx.memories[0] + 100), (int32_t)0xAABBCCDD);
    assert_eq_i32(*(int32_t*)(ctx.memories[1] + 100), 0x11223344);

    // Test 3: load from memory 0
    params[0].i32 = 100;
    assert_ok(wah_call(&ctx, 1, params, 1, &result));
    assert_eq_i32(result.i32, (int32_t)0xAABBCCDD);

    // Test 4: load from memory 1
    params[0].i32 = 100;
    assert_ok(wah_call(&ctx, 3, params, 1, &result));
    assert_eq_i32(result.i32, 0x11223344);

    // Test 5: memory.size 0 and memory.size 1
    assert_ok(wah_call(&ctx, 4, NULL, 0, &result));
    assert_eq_i32(result.i32, 1);
    assert_ok(wah_call(&ctx, 5, NULL, 0, &result));
    assert_eq_i32(result.i32, 1);

    // Test 6: memory.grow on memory 1 (not memory 0)
    params[0].i32 = 1;
    assert_ok(wah_call(&ctx, 6, params, 1, &result));
    assert_eq_i32(result.i32, 1); // returns old size
    assert_eq_u32(ctx.memory_sizes[1], 2 * WAH_WASM_PAGE_SIZE);
    assert_eq_u32(ctx.memory_sizes[0], WAH_WASM_PAGE_SIZE); // memory 0 unchanged

    // Test 7: memory.grow beyond max_pages of memory 1 should fail
    params[0].i32 = 1; // would make 3 pages, but max is 2
    assert_ok(wah_call(&ctx, 6, params, 1, &result));
    assert_eq_i32(result.i32, -1);
    assert_eq_u32(ctx.memory_sizes[1], 2 * WAH_WASM_PAGE_SIZE); // unchanged

    // Test 8: memory.size 1 after grow
    assert_ok(wah_call(&ctx, 5, NULL, 0, &result));
    assert_eq_i32(result.i32, 2);
    assert_eq_u32(ctx.memory_size, WAH_WASM_PAGE_SIZE); // memory_base alias unchanged

    // Test 9: memory.copy from memory 1 to memory 0
    // copy1to0(dst_in_mem0, src_in_mem1, n)
    params[0].i32 = 200; params[1].i32 = 100; params[2].i32 = 4;
    assert_ok(wah_call(&ctx, 7, params, 3, NULL));
    assert_eq_i32(*(int32_t*)(ctx.memories[0] + 200), 0x11223344);
    assert_eq_i32(*(int32_t*)(ctx.memories[0] + 100), (int32_t)0xAABBCCDD); // original unaffected

    // Test 10: out-of-bounds access on memory 1 (after grow, it has 2 pages)
    params[0].i32 = 2 * WAH_WASM_PAGE_SIZE - 2; params[1].i32 = 0;
    assert_err(wah_call(&ctx, 2, params, 2, NULL), WAH_ERROR_MEMORY_OUT_OF_BOUNDS);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

void test_multiple_memories_data_segment() {
    wah_module_t module;
    wah_exec_context_t ctx;
    wah_value_t params[1];
    wah_value_t result;

    printf("Running multiple memories data segment tests...\n");

    // Module with 2 memories, active data segments targeting each memory
    // data.active.table# syntax: 0x02 <memidx> <offset_expr>
    const char *spec = "wasm \
        types {[ \
            fn [i32] [i32], \
            fn [i32] [i32], \
        ]} \
        funcs {[ 0, 1 ]} \
        memories {[ limits.i32/1 1, limits.i32/1 1 ]} \
        exports {[ \
            {'get_byte0'} fn# 0, \
            {'get_byte1'} fn# 1, \
        ]} \
        code {[ \
            {[] local.get 0 i32.load8_u align=1 offset=0 end}, \
            {[] local.get 0 i32.load8_u align=1.mem# 1 offset=0 end}, \
        ]} \
        data {[ \
            data.active.table#0 i32.const 0 end {%'01020304'}, \
            data.active.table# 1 i32.const 0 end {%'0A0B0C0D'}, \
        ]}";

    assert_ok(wah_parse_module_from_spec(&module, spec));
    assert_eq_u32(module.memory_count, 2);
    assert_eq_u32(module.data_segment_count, 2);

    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_instantiate(&ctx));

    // Active data segment 0 should be in memory 0
    assert_eq_u32(ctx.memories[0][0], 0x01);
    assert_eq_u32(ctx.memories[0][1], 0x02);
    // Active data segment 1 should be in memory 1
    assert_eq_u32(ctx.memories[1][0], 0x0A);
    assert_eq_u32(ctx.memories[1][1], 0x0B);

    // Verify via wasm functions
    params[0].i32 = 0;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 0x01);

    params[0].i32 = 0;
    assert_ok(wah_call(&ctx, 1, params, 1, &result));
    assert_eq_i32(result.i32, 0x0A);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

void test_memory_no_max_is_unbounded() {
    // Regression: limits.i32/1 (flags=0x00) must set max_pages=UINT64_MAX, not min_pages.
    printf("Running test_memory_no_max_is_unbounded...\n");

    wah_module_t module;
    wah_exec_context_t ctx;
    wah_value_t params[1], result;

    const char *spec = "wasm \
        types {[ fn [i32] [i32] ]} \
        funcs {[ 0 ]} \
        memories {[ limits.i32/1 1 ]} \
        exports {[ {'grow'} fn# 0 ]} \
        code {[ \
            {[] local.get 0 memory.grow 0 end}, \
        ]}";
    assert_ok(wah_parse_module_from_spec(&module, spec));
    assert_eq_u64(module.memories[0].min_pages, 1);
    assert_eq_u64(module.memories[0].max_pages, UINT64_MAX);

    assert_ok(wah_exec_context_create(&ctx, &module));

    // Growing by 3 from min=1 must succeed (no max limit)
    params[0].i32 = 3;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 1);  // returns old size in pages
    assert_eq_u32(ctx.memory_sizes[0], 4 * WAH_WASM_PAGE_SIZE);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// b425d61: Implement data.drop.
static void test_data_drop() {
    printf("Testing data.drop (b425d61)...\n");

    // Module with passive data segment. Function 0 does memory.init then data.drop.
    // Function 1 tries memory.init with size 0 after drop (should succeed).
    // Function 2 tries memory.init with size > 0 after drop (should trap).
    const char *spec = "wasm \
        types {[ fn [] [], fn [] [i32] ]} \
        funcs {[ 0, 0, 1 ]} \
        memories {[ limits.i32/1 1 ]} \
        datacount { 1 } \
        code {[ \
            {[] \
                i32.const 0 \
                i32.const 0 \
                i32.const 3 \
                memory.init 0 0 \
                data.drop 0 \
            end }, \
            {[] \
                i32.const 0 \
                i32.const 0 \
                i32.const 1 \
                memory.init 0 0 \
            end }, \
            {[] \
                i32.const 0 \
                i32.load align=1 0 \
            end } \
        ]} \
        data {[ data.passive {%'414243'} ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_instantiate(&ctx));

    // Call func 0: init + drop
    assert_ok(wah_call(&ctx, 0, NULL, 0, NULL));

    // Verify data was written: read byte at offset 0
    wah_value_t result;
    assert_ok(wah_call(&ctx, 2, NULL, 0, &result));
    assert_eq_i32(result.i32 & 0xFF, 0x41);

    // Call func 1: memory.init with size 1 after drop should trap
    assert_err(wah_call(&ctx, 1, NULL, 0, NULL), WAH_ERROR_TRAP);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// fd68111: Fix memory.grow to use actual max_pages and handle OOM gracefully.
static void test_memory_grow_max_pages() {
    printf("Testing memory.grow respects max_pages (fd68111)...\n");

    // Memory with min=1, max=2. Grow by 1 should succeed, grow by 1 more should fail.
    const char *spec = "wasm \
        types {[ fn [i32] [i32] ]} \
        funcs {[ 0 ]} \
        memories {[ limits.i32/2 1 2 ]} \
        code {[ {[] local.get 0 memory.grow 0 end } ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t params[1], result;

    // Grow by 1: should succeed, return old size (1)
    params[0].i32 = 1;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 1);

    // Grow by 1 more: should fail (already at max 2), return -1
    params[0].i32 = 1;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, -1);

    // Grow by 0: should succeed, return current size (2)
    params[0].i32 = 0;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 2);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

int main() {
    test_memory_api();
    test_memory_ops();
    wah_test_data_and_bulk_memory_ops();
    test_multiple_memories();
    test_multiple_memories_data_segment();
    test_memory_no_max_is_unbounded();
    test_data_drop();
    test_memory_grow_max_pages();

    printf("\nAll memory tests passed!\n");
    return 0;
}

