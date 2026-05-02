#include "../wah.h"
#include "wah_impl.h"
#include "common.h"
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
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
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
    assert_eq_u32(wah_module_memory_count(&module), 1);
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
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
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
    assert_eq_u32(wah_module_memory_count(&module), 1);
    { wah_memory_desc_t md;
      assert_ok(wah_module_memory(&module, 0, &md));
      assert_true(md.addr_type == WAH_TYPE_I64); assert_eq_u64(md.min_pages, 1); }

    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
    assert_true(wah_debug_memory_data(&ctx, 0) != NULL);

    params[0].i64 = 1024LL;
    params[1].i32 = (int32_t)0xDEADBEEF;
    assert_ok(wah_call(&ctx, 0, params, 2, NULL));

    int32_t *mem_ptr = (int32_t*)(wah_debug_memory_data(&ctx, 0) + 1024);
    assert_eq_i32(*mem_ptr, (int32_t)0xDEADBEEF);

    params[0].i64 = 1024LL;
    assert_ok(wah_call(&ctx, 1, params, 1, &result));
    assert_eq_i32(result.i32, (int32_t)0xDEADBEEF);

    params[0].i64 = (int64_t)(wah_debug_wasm_page_size() - 2);
    params[1].i32 = 0x12345678;
    assert_err(wah_call(&ctx, 0, params, 2, NULL), WAH_ERROR_MEMORY_OUT_OF_BOUNDS);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_memory64_nonzero_memory_scalar_load_store() {
    wah_module_t module = {0};
    wah_exec_context_t ctx = {0};
    wah_value_t result;

    printf("Running memory64 non-zero memory scalar load/store tests...\n");

    const char *spec = "wasm \
        types {[ fn [] [i64] ]} \
        funcs {[ 0 ]} \
        memories {[ limits.i32/1 1, limits.i64/1 1 ]} \
        code {[ {[] \
            i64.const 0 i32.const 287454020 i32.store align=4.mem# 1 offset=0 \
            i64.const 8 i64.const 1234605616436508552 i64.store align=8.mem# 1 offset=0 \
            i64.const 24 f32.const 1.5f32 f32.store align=4.mem# 1 offset=0 \
            i64.const 32 f64.const 2.5f64 f64.store align=8.mem# 1 offset=0 \
            i64.const 40 i32.const -1 i32.store8 align=1.mem# 1 offset=0 \
            i64.const 42 i32.const 32769 i32.store16 align=2.mem# 1 offset=0 \
            i64.const 44 i64.const -2 i64.store8 align=1.mem# 1 offset=0 \
            i64.const 46 i64.const 32770 i64.store16 align=2.mem# 1 offset=0 \
            i64.const 48 i64.const 2147483651 i64.store32 align=4.mem# 1 offset=0 \
            i64.const 0 i32.load align=4.mem# 1 offset=0 drop \
            i64.const 8 i64.load align=8.mem# 1 offset=0 drop \
            i64.const 24 f32.load align=4.mem# 1 offset=0 drop \
            i64.const 32 f64.load align=8.mem# 1 offset=0 drop \
            i64.const 40 i32.load8_s align=1.mem# 1 offset=0 drop \
            i64.const 40 i32.load8_u align=1.mem# 1 offset=0 drop \
            i64.const 42 i32.load16_s align=2.mem# 1 offset=0 drop \
            i64.const 42 i32.load16_u align=2.mem# 1 offset=0 drop \
            i64.const 44 i64.load8_s align=1.mem# 1 offset=0 drop \
            i64.const 44 i64.load8_u align=1.mem# 1 offset=0 drop \
            i64.const 46 i64.load16_s align=2.mem# 1 offset=0 drop \
            i64.const 46 i64.load16_u align=2.mem# 1 offset=0 drop \
            i64.const 48 i64.load32_s align=4.mem# 1 offset=0 drop \
            i64.const 48 i64.load32_u align=4.mem# 1 offset=0 drop \
            i64.const 42 \
        end } ]}";

    assert_ok(wah_parse_module_from_spec(&module, spec));
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
    assert_ok(wah_instantiate(&ctx));
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i64(result.i64, 42);

    uint8_t *mem = wah_debug_memory_data(&ctx, 1);
    assert_eq_u32(mem[0], 0x44);
    assert_eq_u32(mem[1], 0x33);
    assert_eq_u32(mem[2], 0x22);
    assert_eq_u32(mem[3], 0x11);
    assert_eq_u32(mem[40], 0xff);
    assert_eq_u32(mem[42], 0x01);
    assert_eq_u32(mem[43], 0x80);
    assert_eq_u32(mem[44], 0xfe);
    assert_eq_u32(mem[46], 0x02);
    assert_eq_u32(mem[47], 0x80);
    assert_eq_u32(mem[48], 0x03);
    assert_eq_u32(mem[49], 0x00);
    assert_eq_u32(mem[50], 0x00);
    assert_eq_u32(mem[51], 0x80);

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
    { wah_memory_desc_t md;
      assert_ok(wah_module_memory(&module, 0, &md));
      assert_true(md.addr_type == WAH_TYPE_I64); assert_eq_u64(md.min_pages, 1); assert_eq_u64(md.max_pages, 2); }

    assert_ok(wah_exec_context_create(&ctx, &module, NULL));

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
    assert_eq_u32(wah_debug_module_data_segment_count(&module), 1);

    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
    assert_ok(wah_instantiate(&ctx));

    assert_eq_u32(wah_debug_memory_data(&ctx, 0)[0], 0x01);
    assert_eq_u32(wah_debug_memory_data(&ctx, 0)[1], 0x02);
    assert_eq_u32(wah_debug_memory_data(&ctx, 0)[2], 0x03);
    assert_eq_u32(wah_debug_memory_data(&ctx, 0)[3], 0x04);

    params[0].i64 = 0;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 0x01);

    params[0].i64 = 2;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 0x03);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_memory64_data_segment_negative_offset() {
    printf("Testing memory64 active data segment negative offset bounds...\n");

    const char *spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        memories {[ limits.i64/1 1 ]} \
        code {[ {[] end} ]} \
        data {[ data.active.table#0 i64.const -1 end {%'01'} ]}";

    wah_module_t module = {0};
    wah_exec_context_t ctx = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
    assert_err(wah_instantiate(&ctx), WAH_ERROR_MEMORY_OUT_OF_BOUNDS);

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
    { wah_memory_desc_t md; assert_ok(wah_module_memory(&module, 0, &md));
      assert_eq_u64(md.min_pages, 1); assert_eq_u64(md.max_pages, 8589934592ULL); }
    wah_free_module(&module);
}

static void test_memory64_grow_negative_pages() {
    printf("Testing memory64 grow with negative pages...\n");

    const char *spec = "wasm \
        types {[ fn [i64] [i64] ]} \
        funcs {[ 0 ]} \
        memories {[ limits.i64/2 1 10 ]} \
        code {[ {[] local.get 0 memory.grow 0 end } ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t params[1], result;

    params[0].i64 = -1;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i64(result.i64, -1);

    params[0].i64 = -1000;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i64(result.i64, -1);

    params[0].i64 = 0;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i64(result.i64, 1);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_memory64_grow_propagation() {
    printf("Testing memory64 grow propagation to imported context...\n");

    const char *provider_spec = "wasm \
        types {[ fn [] [i64] ]} \
        funcs {[ 0 ]} \
        memories {[ limits.i64/2 1 10 ]} \
        exports {[ {'mem'} mem# 0, {'size'} fn# 0 ]} \
        code {[ {[] memory.size 0 end } ]}";

    const char *consumer_spec = "wasm \
        types {[ fn [i64] [i64], fn [] [i64] ]} \
        imports {[ {'provider'} {'mem'} export.memory limits.i64/2 1 10, \
                   {'provider'} {'size'} fn# 1 ]} \
        funcs {[ 0 ]} \
        code {[ {[] local.get 0 memory.grow 0 end } ]}";

    wah_module_t provider = {0}, consumer = {0};
    assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
    assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

    wah_exec_context_t pctx = {0};
    assert_ok(wah_exec_context_create(&pctx, &provider, NULL));
    assert_ok(wah_instantiate(&pctx));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &consumer, NULL));
    assert_ok(wah_link_context(&ctx, "provider", &pctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t params[1], result;

    // func index: 0 = import 'size', 1 = local grow func
    params[0].i64 = 2;
    assert_ok(wah_call(&ctx, 1, params, 1, &result));
    assert_eq_i64(result.i64, 1);

    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i64(result.i64, 3);

    wah_exec_context_destroy(&ctx);
    wah_exec_context_destroy(&pctx);
    wah_free_module(&consumer);
    wah_free_module(&provider);
}

static void test_table64_grow_negative_delta() {
    printf("Testing table64 grow with negative delta...\n");

    const char *spec = "wasm \
        types {[ fn [i64] [i64], fn [] [i64] ]} \
        funcs {[ 0, 1 ]} \
        tables {[ funcref limits.i64/2 2 10 ]} \
        code {[ \
            {[] ref.null funcref local.get 0 table.grow 0 end }, \
            {[] table.size 0 end } \
        ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t params[1], result;

    params[0].i64 = -1;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i64(result.i64, -1);

    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i64(result.i64, 2);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_table64_grow_propagation() {
    printf("Testing table64 grow propagation to imported context...\n");

    const char *provider_spec = "wasm \
        types {[ fn [] [i64], fn [i64] [i64] ]} \
        funcs {[ 0, 1 ]} \
        tables {[ funcref limits.i64/2 2 10 ]} \
        exports {[ {'tab'} table# 0, {'size'} fn# 0, {'grow'} fn# 1 ]} \
        code {[ \
            {[] table.size 0 end }, \
            {[] ref.null funcref local.get 0 table.grow 0 end } \
        ]}";

    const char *consumer_spec = "wasm \
        types {[ fn [] [i64], fn [i64] [i64] ]} \
        imports {[ {'provider'} {'tab'} export.table funcref limits.i64/2 2 10, \
                   {'provider'} {'size'} fn# 0, \
                   {'provider'} {'grow'} fn# 1 ]} \
        funcs {[ 1 ]} \
        code {[ {[] ref.null funcref local.get 0 table.grow 0 end } ]}";

    wah_module_t provider = {0}, consumer = {0};
    assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
    assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

    wah_exec_context_t pctx = {0};
    assert_ok(wah_exec_context_create(&pctx, &provider, NULL));
    assert_ok(wah_instantiate(&pctx));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &consumer, NULL));
    assert_ok(wah_link_context(&ctx, "provider", &pctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t params[1], result;

    // func index: 0 = import 'size', 1 = import 'grow', 2 = local grow func
    params[0].i64 = 3;
    assert_ok(wah_call(&ctx, 2, params, 1, &result));
    assert_eq_i64(result.i64, 2);

    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i64(result.i64, 5);

    wah_exec_context_destroy(&ctx);
    wah_exec_context_destroy(&pctx);
    wah_free_module(&consumer);
    wah_free_module(&provider);
}

static void test_memory_invalid_flags() {
    printf("Testing memory section with invalid flags...\n");

    wah_module_t module = {0};
    assert_err(wah_parse_module_from_spec(&module, "wasm memories {[ %'06 01' ]}"),
               WAH_ERROR_MALFORMED);
    wah_free_module(&module);
}

// ============================================================
// memory.copy: i32 -> i64 (mixed addressing, runtime)
// ============================================================
static void test_memory_copy_i32_to_i64(void) {
    printf("Testing memory.copy i32->i64 mixed addressing...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // mem 0: i32, mem 1: i64
    // func 0: store(i32 addr, i32 val) to mem 0
    // func 1: load(i64 addr) from mem 1
    // func 2: memory.copy dst=1(i64) src=0(i32) size(i32)
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [i32, i32] [], fn [i64] [i32], fn [i64, i32, i32] []]} \
        funcs {[0, 1, 2]} \
        memories {[limits.i32/1 1, limits.i64/1 1]} \
        code {[ \
            {[] local.get 0 local.get 1 i32.store align=4.mem# 0 offset=0 end}, \
            {[] local.get 0 i32.load align=4.mem# 1 offset=0 end}, \
            {[] local.get 0 local.get 1 local.get 2 memory.copy 1 0 end} \
        ]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    // Write 0xCAFEBABE to mem 0 at offset 100
    wah_value_t p_store[] = { {.i32 = 100}, {.i32 = (int32_t)0xCAFEBABE} };
    assert_ok(wah_call(&ctx, 0, p_store, 2, NULL));

    // Copy 4 bytes from mem 0 offset 100 to mem 1 offset 200
    wah_value_t p_copy[] = { {.i64 = 200}, {.i32 = 100}, {.i32 = 4} };
    assert_ok(wah_call(&ctx, 2, p_copy, 3, NULL));

    // Read from mem 1 at offset 200
    wah_value_t p_load[] = { {.i64 = 200} };
    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, p_load, 1, &result));
    assert_eq_i32(result.i32, (int32_t)0xCAFEBABE);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// ============================================================
// memory.copy: i64 -> i32 (mixed addressing, runtime)
// ============================================================
static void test_memory_copy_i64_to_i32(void) {
    printf("Testing memory.copy i64->i32 mixed addressing...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // mem 0: i64, mem 1: i32
    // func 0: store to mem 0 (i64 addr)
    // func 1: load from mem 1 (i32 addr)
    // func 2: memory.copy dst=1(i32) src=0(i64) size(i32)
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [i64, i32] [], fn [i32] [i32], fn [i32, i64, i32] []]} \
        funcs {[0, 1, 2]} \
        memories {[limits.i64/1 1, limits.i32/1 1]} \
        code {[ \
            {[] local.get 0 local.get 1 i32.store align=4.mem# 0 offset=0 end}, \
            {[] local.get 0 i32.load align=4.mem# 1 offset=0 end}, \
            {[] local.get 0 local.get 1 local.get 2 memory.copy 1 0 end} \
        ]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    // Write 0xDEADBEEF to mem 0 at offset 300
    wah_value_t p_store[] = { {.i64 = 300}, {.i32 = (int32_t)0xDEADBEEF} };
    assert_ok(wah_call(&ctx, 0, p_store, 2, NULL));

    // Copy 4 bytes from mem 0 offset 300 to mem 1 offset 400
    wah_value_t p_copy[] = { {.i32 = 400}, {.i64 = 300}, {.i32 = 4} };
    assert_ok(wah_call(&ctx, 2, p_copy, 3, NULL));

    // Read from mem 1 at offset 400
    wah_value_t p_load[] = { {.i32 = 400} };
    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, p_load, 1, &result));
    assert_eq_i32(result.i32, (int32_t)0xDEADBEEF);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// ============================================================
// memory.copy: i64 -> i64 (both memories i64)
// ============================================================
static void test_memory_copy_i64_to_i64(void) {
    printf("Testing memory.copy i64->i64...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // Two i64 memories
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [i64, i32] [], fn [i64] [i32], fn [i64, i64, i64] []]} \
        funcs {[0, 1, 2]} \
        memories {[limits.i64/1 1, limits.i64/1 1]} \
        code {[ \
            {[] local.get 0 local.get 1 i32.store align=4.mem# 0 offset=0 end}, \
            {[] local.get 0 i32.load align=4.mem# 1 offset=0 end}, \
            {[] local.get 0 local.get 1 local.get 2 memory.copy 1 0 end} \
        ]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t p_store[] = { {.i64 = 500}, {.i32 = 0x12345678} };
    assert_ok(wah_call(&ctx, 0, p_store, 2, NULL));

    wah_value_t p_copy[] = { {.i64 = 600}, {.i64 = 500}, {.i64 = 4} };
    assert_ok(wah_call(&ctx, 2, p_copy, 3, NULL));

    wah_value_t p_load[] = { {.i64 = 600} };
    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, p_load, 1, &result));
    assert_eq_i32(result.i32, 0x12345678);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// ============================================================
// table.copy: i32 -> i64 (mixed addressing, runtime)
// ============================================================
static void test_table_copy_i32_to_i64(void) {
    printf("Testing table.copy i32->i64 mixed addressing...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // table 0: i32, table 1: i64
    // func 0: returns 42 (target for refs)
    // func 1: table.copy dst=1(i64) src=0(i32) size(i32), then call_indirect from table 1
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32]]} \
        funcs {[0, 0]} \
        tables {[funcref limits.i32/1 4, funcref limits.i64/1 4]} \
        elements {[elem.active.table#0 i32.const 0 end [0]]} \
        code {[ \
            {[] i32.const 42 end}, \
            {[] \
                i64.const 1 i32.const 0 i32.const 1 table.copy 1 0 \
                i64.const 1 call_indirect 0 1 \
            end} \
        ]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// ============================================================
// table.copy: i64 -> i32 (mixed addressing, runtime)
// ============================================================
static void test_table_copy_i64_to_i32(void) {
    printf("Testing table.copy i64->i32 mixed addressing...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // table 0: i64, table 1: i32
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32]]} \
        funcs {[0, 0]} \
        tables {[funcref limits.i64/1 4, funcref limits.i32/1 4]} \
        elements {[elem.active.table#0 i64.const 0 end [0]]} \
        code {[ \
            {[] i32.const 77 end}, \
            {[] \
                i32.const 2 i64.const 0 i32.const 1 table.copy 1 0 \
                i32.const 2 call_indirect 0 1 \
            end} \
        ]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 77);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// ============================================================
// table.copy: i64 -> i64
// ============================================================
static void test_table_copy_i64_to_i64(void) {
    printf("Testing table.copy i64->i64...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32]]} \
        funcs {[0, 0]} \
        tables {[funcref limits.i64/1 4, funcref limits.i64/1 4]} \
        elements {[elem.active.table#0 i64.const 0 end [0]]} \
        code {[ \
            {[] i32.const 99 end}, \
            {[] \
                i64.const 3 i64.const 0 i64.const 1 table.copy 1 0 \
                i64.const 3 call_indirect 0 1 \
            end} \
        ]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 99);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// ============================================================
// Bounds overflow: memory.copy with huge offset+size (must trap, not crash)
// ============================================================
static void test_memory_copy_bounds_overflow(void) {
    printf("Testing memory.copy bounds overflow trap...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // Single i64 memory, 1 page
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [i64, i64, i64] []]} \
        funcs {[0]} \
        memories {[limits.i64/1 1]} \
        code {[{[] local.get 0 local.get 1 local.get 2 memory.copy 0 0 end}]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    // dest=1, src=0, size=UINT64_MAX -> dest+size overflows u64
    wah_value_t p[] = { {.i64 = 1}, {.i64 = 0}, {.i64 = (int64_t)UINT64_MAX} };
    assert_err(wah_call(&ctx, 0, p, 3, NULL), WAH_ERROR_MEMORY_OUT_OF_BOUNDS);

    // dest=UINT64_MAX, src=0, size=1 -> dest+size overflows
    wah_value_t p2[] = { {.i64 = (int64_t)UINT64_MAX}, {.i64 = 0}, {.i64 = 1} };
    assert_err(wah_call(&ctx, 0, p2, 3, NULL), WAH_ERROR_MEMORY_OUT_OF_BOUNDS);

    // size=0 with large offset should be OK (spec: zero-length is bounds-checked)
    wah_value_t p3[] = { {.i64 = 65537}, {.i64 = 0}, {.i64 = 0} };
    assert_err(wah_call(&ctx, 0, p3, 3, NULL), WAH_ERROR_MEMORY_OUT_OF_BOUNDS);

    // size=0 within bounds should succeed
    wah_value_t p4[] = { {.i64 = 0}, {.i64 = 0}, {.i64 = 0} };
    assert_ok(wah_call(&ctx, 0, p4, 3, NULL));

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// ============================================================
// Bounds overflow: memory.fill with huge offset+size
// ============================================================
static void test_memory_fill_bounds_overflow(void) {
    printf("Testing memory.fill bounds overflow trap...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [i64, i32, i64] []]} \
        funcs {[0]} \
        memories {[limits.i64/1 1]} \
        code {[{[] local.get 0 local.get 1 local.get 2 memory.fill 0 end}]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t p[] = { {.i64 = 1}, {.i32 = 0}, {.i64 = (int64_t)UINT64_MAX} };
    assert_err(wah_call(&ctx, 0, p, 3, NULL), WAH_ERROR_MEMORY_OUT_OF_BOUNDS);

    wah_value_t p2[] = { {.i64 = (int64_t)UINT64_MAX}, {.i32 = 0}, {.i64 = 1} };
    assert_err(wah_call(&ctx, 0, p2, 3, NULL), WAH_ERROR_MEMORY_OUT_OF_BOUNDS);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// ============================================================
// Bounds overflow: table.copy with huge offset+size
// ============================================================
static void test_table_copy_bounds_overflow(void) {
    printf("Testing table.copy bounds overflow trap...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [i64, i64, i64] []]} \
        funcs {[0]} \
        tables {[funcref limits.i64/1 4]} \
        code {[{[] local.get 0 local.get 1 local.get 2 table.copy 0 0 end}]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t p[] = { {.i64 = 1}, {.i64 = 0}, {.i64 = (int64_t)UINT64_MAX} };
    assert_err(wah_call(&ctx, 0, p, 3, NULL), WAH_ERROR_TRAP);

    wah_value_t p2[] = { {.i64 = (int64_t)UINT64_MAX}, {.i64 = 0}, {.i64 = 1} };
    assert_err(wah_call(&ctx, 0, p2, 3, NULL), WAH_ERROR_TRAP);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// ============================================================
// Bounds overflow: table.fill with huge offset+size
// ============================================================
static void test_table_fill_bounds_overflow(void) {
    printf("Testing table.fill bounds overflow trap...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [i64, i64] []]} \
        funcs {[0]} \
        tables {[funcref limits.i64/1 4]} \
        code {[{[] local.get 0 ref.null funcref local.get 1 table.fill 0 end}]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t p[] = { {.i64 = 1}, {.i64 = (int64_t)UINT64_MAX} };
    assert_err(wah_call(&ctx, 0, p, 2, NULL), WAH_ERROR_TRAP);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// ============================================================
// table.set with i64 index: verify pop order (val on top, idx below)
// ============================================================
static void test_table64_set_get(void) {
    printf("Testing table64 set/get pop order...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // func 0: returns 55 (target)
    // func 1: table.set table[2] = ref.func 0, then table.get table[2], call_indirect
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32]]} \
        funcs {[0, 0]} \
        tables {[funcref limits.i64/1 4]} \
        exports {[{'f0'} fn# 0]} \
        code {[ \
            {[] i32.const 55 end}, \
            {[] \
                i64.const 2 ref.func 0 table.set 0 \
                i64.const 2 table.get 0 \
                ref.is_null \
                if i32 \
                    i32.const -1 \
                else \
                    i64.const 2 call_indirect 0 0 \
                end \
            end} \
        ]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 55);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// ============================================================
// memory64 memory.fill: large fill with i64 addressing
// ============================================================
static void test_memory64_fill_runtime(void) {
    printf("Testing memory64 memory.fill runtime...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // func(i64 dst, i32 val, i64 size)
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [i64, i32, i64] [], fn [i64] [i32]]} \
        funcs {[0, 1]} \
        memories {[limits.i64/1 1]} \
        code {[ \
            {[] local.get 0 local.get 1 local.get 2 memory.fill 0 end}, \
            {[] local.get 0 i32.load8_u align=1 offset=0 end} \
        ]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    // Fill 1000 bytes at offset 100 with 0xBB
    wah_value_t p[] = { {.i64 = 100}, {.i32 = 0xBB}, {.i64 = 1000} };
    assert_ok(wah_call(&ctx, 0, p, 3, NULL));

    // Verify first and last byte
    wah_value_t r;
    wah_value_t ld[] = { {.i64 = 100} };
    assert_ok(wah_call(&ctx, 1, ld, 1, &r));
    assert_eq_i32(r.i32, 0xBB);

    ld[0].i64 = 1099;
    assert_ok(wah_call(&ctx, 1, ld, 1, &r));
    assert_eq_i32(r.i32, 0xBB);

    // Byte before and after should be 0
    ld[0].i64 = 99;
    assert_ok(wah_call(&ctx, 1, ld, 1, &r));
    assert_eq_i32(r.i32, 0);

    ld[0].i64 = 1100;
    assert_ok(wah_call(&ctx, 1, ld, 1, &r));
    assert_eq_i32(r.i32, 0);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// ============================================================
// memory64 memory.init: verify dest is i64, src/size are i32
// ============================================================
static void test_memory64_init_runtime(void) {
    printf("Testing memory64 memory.init runtime...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // func(i64 dest, i32 src_offset, i32 size) = memory.init
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [i64, i32, i32] [], fn [i64] [i32]]} \
        funcs {[0, 1]} \
        memories {[limits.i64/1 1]} \
        datacount {1} \
        code {[ \
            {[] local.get 0 local.get 1 local.get 2 memory.init 0 0 end}, \
            {[] local.get 0 i32.load8_u align=1 offset=0 end} \
        ]} \
        data {[data.passive {%'AABBCCDD'}]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    // Init 4 bytes from data segment to memory at offset 200
    wah_value_t p[] = { {.i64 = 200}, {.i32 = 0}, {.i32 = 4} };
    assert_ok(wah_call(&ctx, 0, p, 3, NULL));

    wah_value_t r;
    wah_value_t ld[] = { {.i64 = 200} };
    assert_ok(wah_call(&ctx, 1, ld, 1, &r));
    assert_eq_i32(r.i32, 0xAA);

    ld[0].i64 = 203;
    assert_ok(wah_call(&ctx, 1, ld, 1, &r));
    assert_eq_i32(r.i32, 0xDD);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// ============================================================
// memory.copy mixed: validation type checks
// ============================================================
static void test_memory_copy_mixed_validation(void) {
    printf("Testing memory.copy mixed addressing validation...\n");

    // i32 dst, i64 src: size should be min(i32,i64)=i32.
    // Using i64 for size must fail.
    wah_module_t bad = {0};
    assert_err(wah_parse_module_from_spec(&bad, "wasm \
        types {[fn [] []]} funcs {[0]} \
        memories {[limits.i32/1 1, limits.i64/1 1]} \
        code {[{[] \
            i32.const 0 i64.const 0 i64.const 1 memory.copy 0 1 \
        end}]}"), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad);

    // Same but with i32 for size should pass
    wah_module_t good = {0};
    assert_ok(wah_parse_module_from_spec(&good, "wasm \
        types {[fn [] []]} funcs {[0]} \
        memories {[limits.i32/1 1, limits.i64/1 1]} \
        code {[{[] \
            i32.const 0 i64.const 0 i32.const 1 memory.copy 0 1 \
        end}]}"));
    wah_free_module(&good);

    // i64 dst, i32 src: size=i32, dst=i64, src=i32
    wah_module_t good2 = {0};
    assert_ok(wah_parse_module_from_spec(&good2, "wasm \
        types {[fn [] []]} funcs {[0]} \
        memories {[limits.i64/1 1, limits.i32/1 1]} \
        code {[{[] \
            i64.const 0 i32.const 0 i32.const 1 memory.copy 0 1 \
        end}]}"));
    wah_free_module(&good2);

    // Both i64: size=i64
    wah_module_t good3 = {0};
    assert_ok(wah_parse_module_from_spec(&good3, "wasm \
        types {[fn [] []]} funcs {[0]} \
        memories {[limits.i64/1 1, limits.i64/1 1]} \
        code {[{[] \
            i64.const 0 i64.const 0 i64.const 1 memory.copy 0 1 \
        end}]}"));
    wah_free_module(&good3);
}

int main() {
    test_memory_grow_clamp();
    test_memory64_large_limits_parsing();
    test_table64_basic();
    test_memory64_address_types();
    test_table_copy_mixed_addr();
    test_memory64_basic();
    test_memory64_nonzero_memory_scalar_load_store();
    test_memory64_size_grow();
    test_memory64_data_segment();
    test_memory64_data_segment_negative_offset();
    test_memory64_validation();
    test_memory64_large_limits();
    test_memory64_grow_negative_pages();
    test_memory64_grow_propagation();
    test_table64_grow_negative_delta();
    test_table64_grow_propagation();
    test_memory_invalid_flags();
    test_memory_copy_i32_to_i64();
    test_memory_copy_i64_to_i32();
    test_memory_copy_i64_to_i64();
    test_table_copy_i32_to_i64();
    test_table_copy_i64_to_i32();
    test_table_copy_i64_to_i64();
    test_memory_copy_bounds_overflow();
    test_memory_fill_bounds_overflow();
    test_table_copy_bounds_overflow();
    test_table_fill_bounds_overflow();
    test_table64_set_get();
    test_memory64_fill_runtime();
    test_memory64_init_runtime();
    test_memory_copy_mixed_validation();
    printf("All memory64/table64 tests passed!\n");
    return 0;
}
