#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define WAH_IMPLEMENTATION
#include "wah.h"
#include "wah_testutils.c"

// DSL spec for data and bulk memory operations test module
static const char *wasm_data_and_bulk_memory_spec =
    "wasm \
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

void wah_test_data_and_bulk_memory_ops() {
    wah_module_t module;
    wah_exec_context_t ctx;
    wah_error_t err;
    wah_value_t params[3]; // params array size is 3, as max args for any function is 3
    wah_value_t result;

    printf("\nRunning data segments and bulk memory operations tests...\n");

    // Test 1: Parse module using DSL
    err = wah_parse_module_from_spec(&module, wasm_data_and_bulk_memory_spec);
    printf("%s\n", wah_strerror(err));
    assert(err == WAH_OK && "Failed to parse data and bulk memory ops module");
    printf("Data and bulk memory ops module parsed successfully.\n");
    assert(module.memory_count == 1 && "Expected 1 memory section");
    assert(module.data_segment_count == 2 && "Expected 2 data segments");
    assert(module.has_data_count_section == true && "Expected data count section to be present");

    // Test 2: Create execution context
    err = wah_exec_context_create(&ctx, &module);
    assert(err == WAH_OK && "Failed to create execution context for data and bulk memory ops");
    printf("Execution context created successfully.\n");
    assert(ctx.memory_base != NULL && "Memory base should not be NULL");
    assert(ctx.memory_size == WAH_WASM_PAGE_SIZE && "Memory size should be 1 page");

    // Verify initial memory state (active data segment 0 should be initialized)
    assert(ctx.memory_base[0] == 0x01 && "Memory byte 0 should be 0x01");
    assert(ctx.memory_base[1] == 0x02 && "Memory byte 1 should be 0x02");
    assert(ctx.memory_base[2] == 0x03 && "Memory byte 2 should be 0x03");
    assert(ctx.memory_base[3] == 0x04 && "Memory byte 3 should be 0x04");
    assert(ctx.memory_base[4] == 0x00 && "Memory byte 4 should be 0x00 (not initialized by active segment)");
    printf("Initial memory state (active data segment 0) verified.\n");

    // Test 3: memory.init - initialize data segment 0 at memory offset 100, data segment offset 0
    uint32_t dest_offset_1 = 100;
    uint32_t src_offset_1 = 0;
    uint32_t size_1 = 4;
    params[0].i32 = dest_offset_1; // dest_offset
    params[1].i32 = src_offset_1;  // src_offset
    params[2].i32 = size_1;        // size
    err = wah_call(&ctx, &module, 0, params, 3, NULL); // Call init_mem_data0 (func 0)
    assert(err == WAH_OK && "Failed to call init_mem_data0 with src_offset 0");
    printf("memory.init successful. Initialized data segment 0 at memory offset %u from data segment offset %u.\n", dest_offset_1, src_offset_1);

    // Verify memory contents after memory.init
    params[0].i32 = dest_offset_1;
    err = wah_call(&ctx, &module, 3, params, 1, &result); // Call get_byte (func 3)
    assert(err == WAH_OK && result.i32 == 0x01 && "Memory byte at offset 100 should be 0x01");
    params[0].i32 = dest_offset_1 + 1;
    err = wah_call(&ctx, &module, 3, params, 1, &result);
    assert(err == WAH_OK && result.i32 == 0x02 && "Memory byte at offset 101 should be 0x02");
    printf("memory.init with src_offset 0 verification successful.\n");

    // Test 4: memory.init - initialize data segment 1 at memory offset 110, data segment offset 1
    uint32_t dest_offset_2 = 110;
    uint32_t src_offset_2 = 1; // Start from the second byte of data segment 1 (value 0x06)
    uint32_t size_2 = 2; // Copy 2 bytes (0x06, 0x07)
    params[0].i32 = dest_offset_2; // dest_offset
    params[1].i32 = src_offset_2;  // src_offset
    params[2].i32 = size_2;        // size
    err = wah_call(&ctx, &module, 1, params, 3, NULL); // Call init_mem_data1 (func 1)
    assert(err == WAH_OK && "Failed to call init_mem_data1 with non-zero src_offset");
    printf("memory.init successful. Initialized data segment 1 at memory offset %u from data segment offset %u.\n", dest_offset_2, src_offset_2);

    // Verify memory contents after memory.init with non-zero src_offset
    params[0].i32 = dest_offset_2;
    err = wah_call(&ctx, &module, 3, params, 1, &result); // Call get_byte (func 3)
    assert(err == WAH_OK && result.i32 == 0x06 && "Memory byte at offset 110 should be 0x06");
    params[0].i32 = dest_offset_2 + 1;
    err = wah_call(&ctx, &module, 3, params, 1, &result);
    assert(err == WAH_OK && result.i32 == 0x07 && "Memory byte at offset 111 should be 0x07");
    printf("memory.init with non-zero src_offset verification successful.\n");

    // Test 5: memory.copy - copy 4 bytes from offset 100 to offset 200
    uint32_t copy_dest = 200;
    uint32_t copy_src = 100;
    uint32_t copy_size = 4;
    params[0].i32 = copy_dest; // dest
    params[1].i32 = copy_src; // src
    params[2].i32 = copy_size; // size
    err = wah_call(&ctx, &module, 2, params, 3, NULL); // Call copy_mem (func 2)
    assert(err == WAH_OK && "Failed to call copy_mem");
    printf("memory.copy successful. Copied %u bytes from %u to %u.\n", copy_size, copy_src, copy_dest);

    // Verify memory contents after memory.copy
    params[0].i32 = copy_dest;
    err = wah_call(&ctx, &module, 3, params, 1, &result); // Call get_byte (func 3)
    assert(err == WAH_OK && result.i32 == 0x01 && "Memory byte at offset 200 should be 0x01");
    params[0].i32 = copy_dest + 1;
    err = wah_call(&ctx, &module, 3, params, 1, &result);
    assert(err == WAH_OK && result.i32 == 0x02 && "Memory byte at offset 201 should be 0x02");
    printf("memory.copy verification successful.\n");

    // Test 6: memory.init - out of bounds (dest_offset + size > memory_size)
    params[0].i32 = WAH_WASM_PAGE_SIZE - 2; // dest_offset (2 bytes before end)
    params[1].i32 = 0; // src_offset
    params[2].i32 = 4; // size (will go out of bounds)
    err = wah_call(&ctx, &module, 0, params, 3, NULL); // Call init_mem_data0 (func 0)
    assert(err == WAH_ERROR_MEMORY_OUT_OF_BOUNDS && "Expected memory out-of-bounds error for memory.init (dest)");
    printf("memory.init out-of-bounds (dest) test successful.\n");

    // Test 7: memory.init - out of bounds (src_offset + size > segment->data_len)
    params[0].i32 = 0; // dest_offset
    params[1].i32 = 3; // src_offset (data segment 0 has 4 bytes, 3 + 2 = 5 > 4)
    params[2].i32 = 2; // size
    err = wah_call(&ctx, &module, 0, params, 3, NULL); // Call init_mem_data0 (func 0)
    assert(err == WAH_ERROR_TRAP && "Expected trap for memory.init (src_offset out of bounds)");
    printf("memory.init out-of-bounds (src) test successful.\n");

    // Test 8: memory.copy - out of bounds (dest)
    params[0].i32 = WAH_WASM_PAGE_SIZE - 2; // dest
    params[1].i32 = 0; // src
    params[2].i32 = 4; // size
    err = wah_call(&ctx, &module, 2, params, 3, NULL); // Call copy_mem (func 2)
    assert(err == WAH_ERROR_MEMORY_OUT_OF_BOUNDS && "Expected memory out-of-bounds error for memory.copy (dest)");
    printf("memory.copy out-of-bounds (dest) test successful.\n");

    // Test 9: memory.copy - out of bounds (src)
    params[0].i32 = 0; // dest
    params[1].i32 = WAH_WASM_PAGE_SIZE - 2; // src
    params[2].i32 = 4; // size
    err = wah_call(&ctx, &module, 2, params, 3, NULL); // Call copy_mem (func 2)
    assert(err == WAH_ERROR_MEMORY_OUT_OF_BOUNDS && "Expected memory out-of-bounds error for memory.copy (src)");
    printf("memory.copy out-of-bounds (src) test successful.\n");

    // Final Cleanup
    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
    printf("Data segments and bulk memory operations tests passed!\n");
}

int main() {
    wah_module_t module;
    wah_exec_context_t ctx;
    wah_error_t err;
    wah_value_t params[3];
    wah_value_t result;

    printf("Running memory tests...\n");

    // Test 1: Parse module
    err = wah_parse_module_from_spec(&module, "wasm \
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
        ]}");
    assert(err == WAH_OK && "Failed to parse module");
    printf("Module parsed successfully.\n");
    assert(module.memory_count == 1 && "Expected 1 memory section");
    assert(module.memories[0].min_pages == 1 && "Expected 1 min page");

    // Test 2: Create execution context
    err = wah_exec_context_create(&ctx, &module);
    assert(err == WAH_OK && "Failed to create execution context");
    printf("Execution context created successfully.\n");
    assert(ctx.memory_base != NULL && "Memory base should not be NULL");
    assert(ctx.memory_size == WAH_WASM_PAGE_SIZE && "Memory size should be 1 page");

    // Test 3: Store a value
    uint32_t test_address = 1024;
    int32_t test_value = 0xDEADBEEF;
    params[0].i32 = test_address;
    params[1].i32 = test_value;
    err = wah_call(&ctx, &module, 0, params, 2, NULL); // Call store_val (func 0)
    assert(err == WAH_OK && "Failed to call store_val");
    printf("Stored 0x%X at address %u.\n", test_value, test_address);

    // Verify directly in memory
    int32_t *mem_ptr = (int32_t*)(ctx.memory_base + test_address);
    assert(*mem_ptr == test_value && "Value not correctly stored in memory");
    printf("Direct memory verification successful.\n");

    // Test 4: Load the value
    params[0].i32 = test_address;
    err = wah_call(&ctx, &module, 1, params, 1, &result); // Call load_val (func 1)
    assert(err == WAH_OK && "Failed to call load_val");
    assert(result.i32 == test_value && "Loaded value does not match stored value");
    printf("Loaded 0x%X from address %u. Verification successful.\n", result.i32, test_address);

    // Test 5: Memory out-of-bounds store
    uint32_t oob_address_store = WAH_WASM_PAGE_SIZE - 2; // 2 bytes before end, trying to store 4 bytes
    params[0].i32 = oob_address_store;
    params[1].i32 = 0x12345678;
    err = wah_call(&ctx, &module, 0, params, 2, NULL);
    assert(err == WAH_ERROR_MEMORY_OUT_OF_BOUNDS && "Expected memory out-of-bounds error for store");
    printf("Memory out-of-bounds store test successful.\n");

    // Test 6: Memory out-of-bounds load
    uint32_t oob_address_load = WAH_WASM_PAGE_SIZE - 2; // 2 bytes before end, trying to load 4 bytes
    params[0].i32 = oob_address_load;
    err = wah_call(&ctx, &module, 1, params, 1, &result);
    assert(err == WAH_ERROR_MEMORY_OUT_OF_BOUNDS && "Expected memory out-of-bounds error for load");
    printf("Memory out-of-bounds load test successful.\n");

    // Test 7: Offset overflow with wrap-around
    printf("Testing offset overflow with wrap-around...\n");
    uint32_t base_addr_overflow = 0xFFFFFFF0; // A high address
    uint32_t offset_overflow = 0x20;         // An offset that causes overflow
    uint32_t expected_effective_addr = (base_addr_overflow + offset_overflow); // Expected wrapped address

    // Ensure the expected_effective_addr is within bounds for our test
    // We want it to wrap around to a valid address within the first page
    assert(expected_effective_addr < WAH_WASM_PAGE_SIZE - 4 && "Expected effective address out of test bounds");

    int32_t test_value_overflow = 0xCAFEBABE;

    // Store the value using the overflowed address
    params[0].i32 = base_addr_overflow;
    params[1].i32 = test_value_overflow;
    err = wah_call(&ctx, &module, 0, params, 2, NULL); // Call store_val (func 0)
    assert(err == WAH_ERROR_MEMORY_OUT_OF_BOUNDS && "Expected memory out-of-bounds error for store with overflow address");
    printf("Memory out-of-bounds store test with overflow address successful.\n");

    // Load the value using the overflowed address
    params[0].i32 = base_addr_overflow;
    err = wah_call(&ctx, &module, 1, params, 1, &result); // Call load_val (func 1)
    assert(err == WAH_ERROR_MEMORY_OUT_OF_BOUNDS && "Expected memory out-of-bounds error for load with overflow address");
    printf("Memory out-of-bounds load test with overflow address successful.\n");

    // Cleanup for first module
    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);

    printf("\nRunning memory operations tests (size, grow, fill)...\n");

    // Test 8: Parse new module for memory operations
    err = wah_parse_module_from_spec(&module, "wasm \
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
        ]}");
    if (err != WAH_OK) {
        printf("Failed to parse memory ops module. Error: %s\n", wah_strerror(err));
    }
    assert(err == WAH_OK && "Failed to parse memory ops module");
    printf("Memory ops module parsed successfully.\n");
    assert(module.memory_count == 1 && "Expected 1 memory section in ops module");
    assert(module.memories[0].min_pages == 1 && "Expected 1 min page in ops module");
    assert(module.memories[0].max_pages == 2 && "Expected 2 max pages in ops module");

    // Test 9: Create execution context for memory operations
    err = wah_exec_context_create(&ctx, &module);
    assert(err == WAH_OK && "Failed to create execution context for memory ops");
    printf("Execution context for memory ops created successfully.\n");
    assert(ctx.memory_base != NULL && "Memory base should not be NULL for ops");
    assert(ctx.memory_size == WAH_WASM_PAGE_SIZE && "Memory size should be 1 page for ops");

    // Test 10: memory.size - initial
    err = wah_call(&ctx, &module, 0, NULL, 0, &result); // Call get_memory_size (func 0)
    assert(err == WAH_OK && "Failed to call get_memory_size");
    assert(result.i32 == 1 && "Initial memory size should be 1 page");
    printf("Initial memory size: %d pages. Test successful.\n", result.i32);

    // Test 11: memory.grow - success
    params[0].i32 = 1; // Grow by 1 page
    err = wah_call(&ctx, &module, 1, params, 1, &result); // Call grow_memory (func 1)
    assert(err == WAH_OK && "Failed to call grow_memory");
    assert(result.i32 == 1 && "grow_memory should return old size (1)");
    assert(ctx.memory_size == (2 * WAH_WASM_PAGE_SIZE) && "Memory size should be 2 pages");
    printf("Memory grown by 1 page. New size: %d pages. Test successful.\n", ctx.memory_size / WAH_WASM_PAGE_SIZE);

    // Test 12: memory.size - after grow
    err = wah_call(&ctx, &module, 0, NULL, 0, &result); // Call get_memory_size (func 0)
    assert(err == WAH_OK && "Failed to call get_memory_size after grow");
    assert(result.i32 == 2 && "Memory size should be 2 pages after grow");
    printf("Memory size after grow: %d pages. Test successful.\n", result.i32);

    // Test 13: memory.grow - failure (exceed max_pages)
    params[0].i32 = 1; // Grow by 1 page (total 3, max 2)
    err = wah_call(&ctx, &module, 1, params, 1, &result); // Call grow_memory (func 1)
    assert(err == WAH_OK && "Failed to call grow_memory for failure test"); // Should return -1, not trap
    assert(result.i32 == -1 && "grow_memory should return -1 on failure");
    assert(ctx.memory_size == (2 * WAH_WASM_PAGE_SIZE) && "Memory size should remain 2 pages");
    printf("Memory grow failure test successful (returned -1). Current size: %d pages.\n", ctx.memory_size / WAH_WASM_PAGE_SIZE);

    // Test 14: memory.fill - basic
    uint32_t fill_offset = 100;
    uint8_t fill_value = 0xAA;
    uint32_t fill_size = 256;
    params[0].i32 = fill_offset; // offset
    params[1].i32 = fill_value;  // value
    params[2].i32 = fill_size;   // size
    err = wah_call(&ctx, &module, 2, params, 3, NULL); // Call fill_memory (func 2)
    assert(err == WAH_OK && "Failed to call fill_memory");
    printf("Memory fill basic test successful. Filled %u bytes from offset %u with 0x%02X.\n", fill_size, fill_offset, fill_value);

    // Verify filled memory directly
    for (uint32_t i = 0; i < fill_size; ++i) {
        assert(ctx.memory_base[fill_offset + i] == fill_value && "Memory fill verification failed");
    }
    printf("Memory fill verification successful.\n");

    // Test 15: memory.fill - out of bounds
    uint32_t oob_fill_offset = ctx.memory_size - 100; // Near end of memory
    uint32_t oob_fill_size = 200; // Will go out of bounds
    params[0].i32 = oob_fill_offset;
    params[1].i32 = 0xBB;
    params[2].i32 = oob_fill_size;
    err = wah_call(&ctx, &module, 2, params, 3, NULL);
    assert(err == WAH_ERROR_MEMORY_OUT_OF_BOUNDS && "Expected memory out-of-bounds error for fill");
    printf("Memory fill out-of-bounds test successful.\n");

    // Final Cleanup
    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
    printf("All memory tests passed!\n");

    wah_test_data_and_bulk_memory_ops();

    return 0;
}

