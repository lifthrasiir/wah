#define WAH_IMPLEMENTATION
#include "../wah.h"
#include "common.c"
#include <stdio.h>

// fbee5c2: Clamp runtime memory max_pages to spec page limit.
static void test_memory_grow_clamp() {
    printf("Testing memory.grow clamp to spec limit (fbee5c2)...\n");

    // i32 memory with no max (unbounded). Runtime clamps to 65536.
    const char *spec = "wasm \
        types {[ fn [i32] [i32] ]} \
        funcs {[ 0 ]} \
        memories {[ limits.i32/1 1 ]} \
        code {[ {[] local.get 0 memory.grow 0 end } ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_instantiate(&ctx));

    // Grow by 0 should return current size (1)
    wah_value_t params[1], result;
    params[0].i32 = 0;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 1);

    // Grow by 65536 would exceed clamped max of 65536 (current=1, want=65537)
    params[0].i32 = 65536;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, -1);

    // Grow by 65535 should succeed (total=65536, within clamped max)
    params[0].i32 = 65535;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 1);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// ce6b0c8: Improve memory64 and table64 limits handling.
static void test_memory64_large_limits_parsing() {
    printf("Testing memory64 large limits parsing (ce6b0c8)...\n");

    // memory64 with large but valid limits
    const char *spec = "wasm \
        types {[]} \
        memories {[ limits.i64/2 1 100000 ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    assert_eq_u32(module.memory_count, 1);
    wah_free_module(&module);

    // memory64 unbounded
    const char *spec2 = "wasm \
        types {[]} \
        memories {[ limits.i64/1 0 ]}";

    wah_module_t module2 = {0};
    assert_ok(wah_parse_module_from_spec(&module2, spec2));
    wah_free_module(&module2);
}

// 9256e42: Tables now support 64-bit indices.
static void test_table64_basic() {
    printf("Testing table64 basic ops (9256e42)...\n");

    // Table with i64 indices: table.size returns i64, table.grow takes i64
    const char *spec = "wasm \
        types {[ fn [] [i64], fn [] [i64] ]} \
        funcs {[ 0, 1 ]} \
        tables {[ funcref limits.i64/1 2 ]} \
        code {[ \
            {[] table.size 0 end }, \
            {[] ref.null funcref i64.const 3 table.grow 0 end } \
        ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;

    // table.size should return 2 (as i64)
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i64(result.i64, 2);

    // table.grow by 3 should return old size 2
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i64(result.i64, 2);

    // table.size should now be 5
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i64(result.i64, 5);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// bf0a840: memory.init/fill correctly expects i64 address type for memory64.
static void test_memory64_address_types() {
    printf("Testing memory64 address type validation (bf0a840)...\n");

    // Negative: memory64 memory.fill with i32 address should fail validation
    const char *bad_fill = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        memories {[ limits.i64/1 1 ]} \
        code {[ {[] \
            i32.const 0 \
            i32.const 0 \
            i32.const 1 \
            memory.fill 0 \
        end } ]}";

    wah_module_t bad = {0};
    assert_err(wah_parse_module_from_spec(&bad, bad_fill), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad);

    // Positive: memory64 memory.fill with i64 address should pass
    const char *good_fill = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        memories {[ limits.i64/1 1 ]} \
        code {[ {[] \
            i64.const 0 \
            i32.const 0 \
            i64.const 1 \
            memory.fill 0 \
        end } ]}";

    wah_module_t good = {0};
    assert_ok(wah_parse_module_from_spec(&good, good_fill));
    wah_free_module(&good);
}

// f8f9730: Fix table.copy size-operand type for mixed i32/i64 tables.
static void test_table_copy_mixed_addr() {
    printf("Testing table.copy mixed address types (f8f9730)...\n");

    // Negative: dst=i32 table, src=i64 table, size should be i32.
    // But if we pass i64 for size it should fail.
    const char *bad_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 2, funcref limits.i64/1 2 ]} \
        code {[ {[] \
            i32.const 0 \
            i64.const 0 \
            i64.const 1 \
            table.copy 0 1 \
        end } ]}";

    wah_module_t bad = {0};
    assert_err(wah_parse_module_from_spec(&bad, bad_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad);

    // Positive: dst=i32, src=i64, size=i32
    const char *good_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 2, funcref limits.i64/1 2 ]} \
        code {[ {[] \
            i32.const 0 \
            i64.const 0 \
            i32.const 1 \
            table.copy 0 1 \
        end } ]}";

    wah_module_t good = {0};
    assert_ok(wah_parse_module_from_spec(&good, good_spec));
    wah_free_module(&good);
}

// --- Tests moved from test_memory.c ---

static void test_memory64_basic() {
    wah_module_t module;
    wah_exec_context_t ctx;
    wah_value_t params[2];
    wah_value_t result;

    printf("Running memory64 basic load/store tests...\n");

    const char *spec = "wasm \
        types {[ \
            fn [i64, i32] [], \
            fn [i64] [i32], \
        ]} \
        funcs {[ 0, 1 ]} \
        memories {[ limits.i64/1 1 ]} \
        exports {[ \
            {'store'} fn# 0, \
            {'load'}  fn# 1, \
        ]} \
        code {[ \
            {[] local.get 0 local.get 1 i32.store align=4 offset=0 end}, \
            {[] local.get 0 i32.load align=4 offset=0 end}, \
        ]}";

    assert_ok(wah_parse_module_from_spec(&module, spec));
    assert_eq_u32(module.memory_count, 1);
    assert_true(module.memories[0].addr_type == WAH_TYPE_I64);
    assert_eq_u64(module.memories[0].min_pages, 1);

    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_true(ctx.memories[0].data != NULL);

    params[0].i64 = 1024LL;
    params[1].i32 = (int32_t)0xDEADBEEF;
    assert_ok(wah_call(&ctx, 0, params, 2, NULL));

    int32_t *mem_ptr = (int32_t*)(ctx.memories[0].data + 1024);
    assert_eq_i32(*mem_ptr, (int32_t)0xDEADBEEF);

    params[0].i64 = 1024LL;
    assert_ok(wah_call(&ctx, 1, params, 1, &result));
    assert_eq_i32(result.i32, (int32_t)0xDEADBEEF);

    params[0].i64 = (int64_t)(WAH_WASM_PAGE_SIZE - 2);
    params[1].i32 = 0x12345678;
    assert_err(wah_call(&ctx, 0, params, 2, NULL), WAH_ERROR_MEMORY_OUT_OF_BOUNDS);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_memory64_size_grow() {
    wah_module_t module;
    wah_exec_context_t ctx;
    wah_value_t params[1];
    wah_value_t result;

    printf("Running memory64 size/grow tests...\n");

    const char *spec = "wasm \
        types {[ \
            fn [] [i64], \
            fn [i64] [i64], \
        ]} \
        funcs {[ 0, 1 ]} \
        memories {[ limits.i64/2 1 2 ]} \
        exports {[ \
            {'size'} fn# 0, \
            {'grow'} fn# 1, \
        ]} \
        code {[ \
            {[] memory.size 0 end}, \
            {[] local.get 0 memory.grow 0 end}, \
        ]}";

    assert_ok(wah_parse_module_from_spec(&module, spec));
    assert_true(module.memories[0].addr_type == WAH_TYPE_I64);
    assert_eq_u64(module.memories[0].min_pages, 1);
    assert_eq_u64(module.memories[0].max_pages, 2);

    assert_ok(wah_exec_context_create(&ctx, &module));

    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i64(result.i64, 1);

    params[0].i64 = 1;
    assert_ok(wah_call(&ctx, 1, params, 1, &result));
    assert_eq_i64(result.i64, 1);

    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i64(result.i64, 2);

    params[0].i64 = 1;
    assert_ok(wah_call(&ctx, 1, params, 1, &result));
    assert_eq_i64(result.i64, -1);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_memory64_data_segment() {
    wah_module_t module;
    wah_exec_context_t ctx;
    wah_value_t params[1];
    wah_value_t result;

    printf("Running memory64 data segment tests...\n");

    const char *spec = "wasm \
        types {[ fn [i64] [i32] ]} \
        funcs {[ 0 ]} \
        memories {[ limits.i64/1 1 ]} \
        exports {[ {'get_byte'} fn# 0 ]} \
        code {[ \
            {[] local.get 0 i32.load8_u align=1 offset=0 end} \
        ]} \
        data {[ \
            data.active.table#0 i64.const 0 end {%'01020304'} \
        ]}";

    assert_ok(wah_parse_module_from_spec(&module, spec));
    assert_eq_u32(module.data_segment_count, 1);

    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_instantiate(&ctx));

    assert_eq_u32(ctx.memories[0].data[0], 0x01);
    assert_eq_u32(ctx.memories[0].data[1], 0x02);
    assert_eq_u32(ctx.memories[0].data[2], 0x03);
    assert_eq_u32(ctx.memories[0].data[3], 0x04);

    params[0].i64 = 0;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 0x01);

    params[0].i64 = 2;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 0x03);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_memory64_validation() {
    wah_module_t module;

    printf("Running memory64 validation tests...\n");

    const char *spec_i32_i64_addr = "wasm \
        types {[ fn [i64] [i32] ]} \
        funcs {[ 0 ]} \
        memories {[ limits.i32/1 1 ]} \
        code {[ \
            {[] local.get 0 i32.load align=4 offset=0 end} \
        ]}";
    assert_err(wah_parse_module_from_spec(&module, spec_i32_i64_addr), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);

    const char *spec_i64_i32_addr = "wasm \
        types {[ fn [i32] [i32] ]} \
        funcs {[ 0 ]} \
        memories {[ limits.i64/1 1 ]} \
        code {[ \
            {[] local.get 0 i32.load align=4 offset=0 end} \
        ]}";
    assert_err(wah_parse_module_from_spec(&module, spec_i64_i32_addr), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);

    const char *spec_i32_i64_offset = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        memories {[ limits.i32/1 1 ]} \
        code {[ {[] end} ]} \
        data {[ \
            data.active.table#0 i64.const 0 end {%'01020304'} \
        ]}";
    assert_err(wah_parse_module_from_spec(&module, spec_i32_i64_offset), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);

    const char *spec_i64_i32_offset = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        memories {[ limits.i64/1 1 ]} \
        code {[ {[] end} ]} \
        data {[ \
            data.active.table#0 i32.const 0 end {%'01020304'} \
        ]}";
    assert_err(wah_parse_module_from_spec(&module, spec_i64_i32_offset), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);
}

static void test_memory64_large_limits() {
    printf("Running test_memory64_large_limits (regression for uint32 overflow)...\n");
    wah_module_t module = {0};
    const char *spec = "wasm memories {[ limits.i64/2 1 8589934592 ]}";
    assert_ok(wah_parse_module_from_spec(&module, spec));
    assert_eq_u64(module.memories[0].min_pages, 1);
    assert_eq_u64(module.memories[0].max_pages, 8589934592ULL);
    wah_free_module(&module);
}

int main() {
    test_memory_grow_clamp();
    test_memory64_large_limits_parsing();
    test_table64_basic();
    test_memory64_address_types();
    test_table_copy_mixed_addr();
    test_memory64_basic();
    test_memory64_size_grow();
    test_memory64_data_segment();
    test_memory64_validation();
    test_memory64_large_limits();
    printf("All memory64/table64 tests passed!\n");
    return 0;
}
