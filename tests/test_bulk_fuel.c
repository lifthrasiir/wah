#define WAH_BULK_CHECK_INTERVAL 64
#define WAH_BULK_ITEMS_PER_FUEL 16
#define WAH_IMPLEMENTATION
#include "common.h"

#define REP8(x) x x x x x x x x
#define REP64(x) REP8(REP8(x))
#define REP128(x) REP64(x) REP64(x)

static const wah_parse_options_t fuel_opts = { .features = WAH_FEATURE_ALL, .enable_fuel_metering = true };

#define PARSE_FUEL(mod, spec, ...) \
    assert_ok(wah_parse_module_from_spec_ex((mod), &fuel_opts, (spec), ##__VA_ARGS__))

// Resume helper: run with given fuel, 1 fuel at a time, return total fuel consumed.
static int64_t run_resume(wah_exec_context_t *ctx, uint32_t func, wah_value_t *params, uint32_t nparams,
                          int64_t fuel_per_step) {
    int64_t total_fuel = 0;
    wah_set_fuel(ctx, fuel_per_step);
    total_fuel += fuel_per_step;
    assert_ok(wah_start(ctx, func, params, nparams));
    wah_error_t err;
    while ((err = wah_resume(ctx)) > 0) {
        total_fuel += fuel_per_step;
        wah_set_fuel(ctx, fuel_per_step);
    }
    assert_ok(err);
    return total_fuel;
}

// ============================================================
// memory.fill: fuel should be proportional to size
// ============================================================
static void test_memory_fill_fuel(void) {
    printf("Testing memory.fill fuel proportionality...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // func(i32 dst, i32 val, i32 size) = memory.fill
    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32, i32] []]} funcs {[0]} \
        memories {[limits.i32/1 1]} \
        code {[{[] local.get 0 local.get 1 local.get 2 memory.fill 0 end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    // Small fill: 16 bytes. Cost = ceil(16/16) = 1 fuel for bulk + instruction fuel.
    wah_value_t p1[] = { {.i32 = 0}, {.i32 = 0xAA}, {.i32 = 16} };
    wah_set_fuel(&ctx, 10000);
    assert_ok(wah_call(&ctx, 0, p1, 3, NULL));
    int64_t small_cost = 10000 - wah_get_fuel(&ctx);

    // Large fill: 256 bytes. Cost = ceil(256/16) = 16 fuel for bulk.
    wah_value_t p2[] = { {.i32 = 0}, {.i32 = 0xBB}, {.i32 = 256} };
    wah_set_fuel(&ctx, 10000);
    assert_ok(wah_call(&ctx, 0, p2, 3, NULL));
    int64_t large_cost = 10000 - wah_get_fuel(&ctx);

    printf("  fill(16) cost=%lld, fill(256) cost=%lld\n",
           (long long)small_cost, (long long)large_cost);
    assert_true(large_cost > small_cost);
    assert_true(large_cost >= small_cost + 14); // at least 15 more fuel for 240 more bytes

    // Verify byte 0 and byte 255 are correct
    assert_eq_u32(ctx.memory_base[0], 0xBB);
    assert_eq_u32(ctx.memory_base[255], 0xBB);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// ============================================================
// memory.fill: fuel exhaustion mid-fill with resume
// ============================================================
static void test_memory_fill_fuel_resume(void) {
    printf("Testing memory.fill fuel exhaustion and resume...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32, i32] []]} funcs {[0]} \
        memories {[limits.i32/1 1]} \
        code {[{[] local.get 0 local.get 1 local.get 2 memory.fill 0 end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    memset(ctx.memory_base, 0, 512);

    // Fill 256 bytes with fuel=5 per step. Each step covers 5*16=80 bytes.
    wah_value_t p[] = { {.i32 = 0}, {.i32 = 0xCD}, {.i32 = 256} };
    wah_set_fuel(&ctx, 5);
    assert_ok(wah_start(&ctx, 0, p, 3));

    int suspensions = 0;
    wah_error_t err;
    while ((err = wah_resume(&ctx)) > 0) {
        assert_eq_i32(err, WAH_STATUS_FUEL_EXHAUSTED);
        suspensions++;
        // Verify partial progress: some bytes should be filled
        bool some_filled = false;
        for (int i = 0; i < 256; i++) {
            if (ctx.memory_base[i] == 0xCD) { some_filled = true; break; }
        }
        if (suspensions > 1) assert_true(some_filled);
        wah_set_fuel(&ctx, 5);
    }
    assert_eq_i32(err, WAH_OK);
    assert_true(suspensions > 0);

    wah_value_t dummy;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &dummy, 0, &actual));

    // All 256 bytes should be filled
    for (int i = 0; i < 256; i++)
        assert_eq_u32(ctx.memory_base[i], 0xCD);

    printf("  fill(256) resumed %d times with fuel=5/step\n", suspensions);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// ============================================================
// memory.copy: fuel proportional + resume
// ============================================================
static void test_memory_copy_fuel_resume(void) {
    printf("Testing memory.copy fuel exhaustion and resume...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // func(dst, src, size) = memory.copy
    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32, i32] []]} funcs {[0]} \
        memories {[limits.i32/1 1]} \
        code {[{[] local.get 0 local.get 1 local.get 2 memory.copy 0 0 end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    // Fill source region with pattern
    for (int i = 0; i < 256; i++) ctx.memory_base[i] = (uint8_t)i;
    memset(ctx.memory_base + 512, 0, 256);

    // Copy 256 bytes: src=0, dst=512
    wah_value_t p[] = { {.i32 = 512}, {.i32 = 0}, {.i32 = 256} };
    int64_t total = run_resume(&ctx, 0, p, 3, 3);

    wah_value_t dummy;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &dummy, 0, &actual));

    for (int i = 0; i < 256; i++)
        assert_eq_u32(ctx.memory_base[512 + i], (uint8_t)i);

    printf("  copy(256) total fuel=%lld\n", (long long)total);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// ============================================================
// memory.copy backward (overlapping) fuel resume
// ============================================================
static void test_memory_copy_backward_fuel_resume(void) {
    printf("Testing memory.copy backward (overlapping) fuel resume...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32, i32] []]} funcs {[0]} \
        memories {[limits.i32/1 1]} \
        code {[{[] local.get 0 local.get 1 local.get 2 memory.copy 0 0 end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    // Pattern: bytes 0..127 = 0xA0..0xFF-ish
    for (int i = 0; i < 128; i++) ctx.memory_base[i] = (uint8_t)(0xA0 + i);

    // Overlapping copy: dst=32, src=0, size=128 (forward overlap → backward copy)
    wah_value_t p[] = { {.i32 = 32}, {.i32 = 0}, {.i32 = 128} };
    run_resume(&ctx, 0, p, 3, 3);

    wah_value_t dummy;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &dummy, 0, &actual));

    for (int i = 0; i < 128; i++)
        assert_eq_u32(ctx.memory_base[32 + i], (uint8_t)(0xA0 + i));

    printf("  backward copy verified\n");

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// ============================================================
// memory.init fuel resume
// ============================================================
static void test_memory_init_fuel_resume(void) {
    printf("Testing memory.init fuel resume...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // 128-byte passive data segment: 0xAA repeated
    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32, i32] []]} funcs {[0]} \
        memories {[limits.i32/1 1]} \
        datacount {1} \
        code {[{[] local.get 0 local.get 1 local.get 2 memory.init 0 0 end}]} \
        data {[data.passive {%'" REP128("AA") "'}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    memset(ctx.memory_base, 0, 256);

    wah_value_t p[] = { {.i32 = 0}, {.i32 = 0}, {.i32 = 128} };
    run_resume(&ctx, 0, p, 3, 3);

    wah_value_t dummy;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &dummy, 0, &actual));

    assert_eq_u32(ctx.memory_base[0], 0xAA);
    assert_eq_u32(ctx.memory_base[127], 0xAA);
    assert_eq_u32(ctx.memory_base[128], 0x00);

    printf("  memory.init(128) verified\n");

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// ============================================================
// table.fill fuel resume
// ============================================================
static void test_table_fill_fuel_resume(void) {
    printf("Testing table.fill fuel resume...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // func 0: table.fill(offset, ref.null, size) then table.get(offset) ref.is_null
    // func 1 (filler): takes (i32 offset, i32 size), fills with null, returns table.get(offset).is_null
    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32] [i32]]} funcs {[0]} \
        tables {[funcref limits.i32/2 256 256]} \
        code {[{[] \
            local.get 0 ref.null funcref local.get 1 table.fill 0 \
            local.get 0 table.get 0 ref.is_null \
        end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    // Fill 200 entries
    wah_value_t p[] = { {.i32 = 0}, {.i32 = 200} };
    run_resume(&ctx, 0, p, 2, 3);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 1); // is_null

    printf("  table.fill(200) fuel resume verified\n");

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// ============================================================
// table.copy fuel resume
// ============================================================
static void test_table_copy_fuel_resume(void) {
    printf("Testing table.copy fuel resume...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // func 0: table.copy(dst, src, size)
    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32, i32] []]} \
        funcs {[0]} \
        tables {[funcref limits.i32/2 256 256]} \
        code {[ \
            {[] local.get 0 local.get 1 local.get 2 table.copy 0 0 end} \
        ]}");
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    // Copy 100 null entries from table[0..100] to table[100..200]
    wah_value_t p[] = { {.i32 = 100}, {.i32 = 0}, {.i32 = 100} };
    run_resume(&ctx, 0, p, 3, 3);

    wah_value_t dummy;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &dummy, 0, &actual));

    printf("  table.copy(100) fuel resume verified\n");

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// ============================================================
// table.copy backward (overlapping) fuel resume
// ============================================================
static void test_table_copy_backward_fuel_resume(void) {
    printf("Testing table.copy backward fuel resume...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // func 0: table.copy(dst, src, size) — same table, overlapping triggers backward
    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32, i32] []]} \
        funcs {[0]} \
        tables {[funcref limits.i32/2 256 256]} \
        code {[ \
            {[] local.get 0 local.get 1 local.get 2 table.copy 0 0 end} \
        ]}");
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    // Overlapping copy: dst=16, src=0, size=64 (backward because dst > src and overlaps)
    wah_value_t p[] = { {.i32 = 16}, {.i32 = 0}, {.i32 = 64} };
    run_resume(&ctx, 0, p, 3, 3);

    wah_value_t dummy;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &dummy, 0, &actual));

    printf("  table.copy backward fuel resume verified\n");

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// ============================================================
// table.init fuel resume
// ============================================================
static void test_table_init_fuel_resume(void) {
    printf("Testing table.init fuel resume...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // passive element segment with 64 function refs to func 0
    PARSE_FUEL(&mod, "wasm \
        types {[fn [] [], fn [i32, i32, i32] [i32]]} \
        funcs {[0, 1]} \
        tables {[funcref limits.i32/2 256 256]} \
        elements {[ elem.passive elem.funcref [" REP64("0, ") "] ]} \
        code {[ \
            {[] end}, \
            {[] local.get 0 local.get 1 local.get 2 table.init 0 0 \
                local.get 0 table.get 0 ref.is_null end} \
        ]}");
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t p[] = { {.i32 = 10}, {.i32 = 0}, {.i32 = 64} };
    run_resume(&ctx, 1, p, 3, 3);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0); // not null — filled with func refs

    printf("  table.init(80) fuel resume verified\n");

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// ============================================================
// array.fill fuel resume (GC)
// ============================================================
static void test_array_fill_fuel_resume(void) {
    printf("Testing array.fill fuel resume...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // array type 0: array i32 mut
    // func 0: create array of 200 elements, fill with 42, return array.get(100)
    PARSE_FUEL(&mod, "wasm \
        types {[array i32 mut, fn [] [i32]]} \
        funcs {[1]} \
        code {[{[1 type.ref.null 0] \
            i32.const 0 i32.const 200 array.new 0 local.set 0 \
            local.get 0 i32.const 0 i32.const 42 i32.const 200 array.fill 0 \
            local.get 0 i32.const 100 array.get 0 \
        end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    // Run with small fuel to force suspensions during array.fill
    run_resume(&ctx, 0, NULL, 0, 3);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 42);

    printf("  array.fill(200) fuel resume verified\n");

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// ============================================================
// array.copy fuel resume (GC)
// ============================================================
static void test_array_copy_fuel_resume(void) {
    printf("Testing array.copy fuel resume...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // Create src array with pattern, copy to dst, verify
    PARSE_FUEL(&mod, "wasm \
        types {[array i32 mut, fn [] [i32]]} \
        funcs {[1]} \
        code {[{[2 type.ref.null 0] \
            i32.const 99 i32.const 200 array.new 0 local.set 0 \
            i32.const 0  i32.const 200 array.new 0 local.set 1 \
            local.get 1 i32.const 0 local.get 0 i32.const 0 i32.const 200 array.copy 0 0 \
            local.get 1 i32.const 150 array.get 0 \
        end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    run_resume(&ctx, 0, NULL, 0, 3);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 99);

    printf("  array.copy(200) fuel resume verified\n");

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// ============================================================
// array.init_data fuel resume (GC)
// ============================================================
static void test_array_init_data_fuel_resume(void) {
    printf("Testing array.init_data fuel resume...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // i8 array, init from 128-byte passive data segment (all 0xBB)
    PARSE_FUEL(&mod, "wasm \
        types {[array i8 mut, fn [] [i32]]} \
        funcs {[1]} \
        datacount {1} \
        code {[{[1 type.ref.null 0] \
            i32.const 0 i32.const 128 array.new 0 local.set 0 \
            local.get 0 i32.const 0 i32.const 0 i32.const 128 array.init_data 0 0 \
            local.get 0 i32.const 0 array.get_u 0 \
            local.get 0 i32.const 127 array.get_u 0 i32.add \
        end}]} \
        data {[data.passive {%'" REP128("BB") "'}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    run_resume(&ctx, 0, NULL, 0, 3);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0xBB + 0xBB);

    printf("  array.init_data(128) fuel resume verified\n");

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// ============================================================
// array.init_elem fuel resume (GC)
// ============================================================
static void test_array_init_elem_fuel_resume(void) {
    printf("Testing array.init_elem fuel resume...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // funcref array, init from passive element segment with 64 null entries
    PARSE_FUEL(&mod, "wasm \
        types {[sub [] array funcref mut, fn [] [i32]]} \
        funcs {[1]} \
        elements {[ elem.passive.expr funcref [" REP64("ref.null funcref end, ") "] ]} \
        code {[{[1 type.ref.null 0] \
            ref.null funcref i32.const 64 array.new 0 local.set 0 \
            local.get 0 i32.const 0 i32.const 0 i32.const 64 array.init_elem 0 0 \
            local.get 0 i32.const 32 array.get 0 ref.is_null \
        end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    run_resume(&ctx, 0, NULL, 0, 3);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 1); // null funcref

    printf("  array.init_elem(64) fuel resume verified\n");

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// ============================================================
// Fuel cost proportionality: fill(0) vs fill(N) should differ
// ============================================================
static void test_bulk_fuel_proportional(void) {
    printf("Testing bulk fuel proportionality across sizes...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [i32, i32, i32] []]} funcs {[0]} \
        memories {[limits.i32/1 1]} \
        code {[{[] local.get 0 local.get 1 local.get 2 memory.fill 0 end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    // fill(0) should be cheapest (bulk cost = 0)
    wah_value_t p0[] = { {.i32 = 0}, {.i32 = 0}, {.i32 = 0} };
    wah_set_fuel(&ctx, 10000);
    assert_ok(wah_call(&ctx, 0, p0, 3, NULL));
    int64_t cost0 = 10000 - wah_get_fuel(&ctx);

    // fill(1) should cost 1 more than fill(0)
    wah_value_t p1[] = { {.i32 = 0}, {.i32 = 0}, {.i32 = 1} };
    wah_set_fuel(&ctx, 10000);
    assert_ok(wah_call(&ctx, 0, p1, 3, NULL));
    int64_t cost1 = 10000 - wah_get_fuel(&ctx);

    // fill(WAH_BULK_ITEMS_PER_FUEL) = 16 bytes, should cost 1 bulk fuel
    wah_value_t p16[] = { {.i32 = 0}, {.i32 = 0}, {.i32 = WAH_BULK_ITEMS_PER_FUEL} };
    wah_set_fuel(&ctx, 10000);
    assert_ok(wah_call(&ctx, 0, p16, 3, NULL));
    int64_t cost16 = 10000 - wah_get_fuel(&ctx);

    // fill(WAH_BULK_ITEMS_PER_FUEL * 10) = 160 bytes, 10 bulk fuel
    wah_value_t p160[] = { {.i32 = 0}, {.i32 = 0}, {.i32 = WAH_BULK_ITEMS_PER_FUEL * 10} };
    wah_set_fuel(&ctx, 10000);
    assert_ok(wah_call(&ctx, 0, p160, 3, NULL));
    int64_t cost160 = 10000 - wah_get_fuel(&ctx);

    printf("  fill(0)=%lld fill(1)=%lld fill(%d)=%lld fill(%d)=%lld\n",
           (long long)cost0, (long long)cost1,
           WAH_BULK_ITEMS_PER_FUEL, (long long)cost16,
           WAH_BULK_ITEMS_PER_FUEL * 10, (long long)cost160);

    assert_eq_i64(cost1, cost0 + 1);  // 1 byte → ceil(1/16) = 1
    assert_eq_i64(cost16, cost0 + 1); // exactly 16 bytes → 1
    assert_eq_i64(cost160, cost0 + 10); // exactly 160 bytes → 10

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// ============================================================
// No metering: bulk fuel is not charged
// ============================================================
static void test_bulk_no_metering(void) {
    printf("Testing bulk ops without fuel metering (no charge)...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // Parse WITHOUT fuel metering
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [i32, i32, i32] []]} funcs {[0]} \
        memories {[limits.i32/1 1]} \
        code {[{[] local.get 0 local.get 1 local.get 2 memory.fill 0 end}]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    // Even with fuel set, no METER/TICK opcodes, so fuel won't change
    wah_set_fuel(&ctx, 100);
    wah_value_t p[] = { {.i32 = 0}, {.i32 = 0xAA}, {.i32 = 1000} };
    assert_ok(wah_call(&ctx, 0, p, 3, NULL));
    assert_eq_i64(wah_get_fuel(&ctx), 100); // unchanged

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// ============================================================
// table64 table.fill fuel resume
// ============================================================
static void test_table64_fill_fuel_resume(void) {
    printf("Testing table64 table.fill fuel resume...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [i64, i64] [i32]]} funcs {[0]} \
        tables {[funcref limits.i64/2 200 200]} \
        code {[{[] \
            local.get 0 ref.null funcref local.get 1 table.fill 0 \
            local.get 0 table.get 0 ref.is_null \
        end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t p[] = { {.i64 = 0}, {.i64 = 150} };
    run_resume(&ctx, 0, p, 2, 3);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 1);

    printf("  table64.fill(150) fuel resume verified\n");

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// ============================================================
// table64 table.copy fuel resume
// ============================================================
static void test_table64_copy_fuel_resume(void) {
    printf("Testing table64 table.copy fuel resume...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [i64, i64, i64] []]} funcs {[0]} \
        tables {[funcref limits.i64/2 200 200]} \
        code {[{[] local.get 0 local.get 1 local.get 2 table.copy 0 0 end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t p[] = { {.i64 = 100}, {.i64 = 0}, {.i64 = 100} };
    run_resume(&ctx, 0, p, 3, 3);

    wah_value_t dummy;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &dummy, 0, &actual));

    printf("  table64.copy(100) fuel resume verified\n");

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_table64_copy_backward_fuel_resume(void) {
    printf("Testing table64 table.copy backward fuel resume...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [i64, i64, i64] []]} funcs {[0]} \
        tables {[funcref limits.i64/2 200 200]} \
        code {[{[] local.get 0 local.get 1 local.get 2 table.copy 0 0 end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t p[] = { {.i64 = 16}, {.i64 = 0}, {.i64 = 100} };
    run_resume(&ctx, 0, p, 3, 3);

    wah_value_t dummy;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &dummy, 0, &actual));

    printf("  table64.copy backward fuel resume verified\n");

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_table_copy_i32_to_i64_fuel_resume(void) {
    printf("Testing mixed-address table.copy i32_to_i64 fuel resume...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [] [], fn [i64, i32, i32] [i32]]} \
        funcs {[0, 1]} \
        tables {[funcref limits.i64/2 200 200, funcref limits.i32/2 200 200]} \
        elements {[ elem.passive elem.funcref [" REP64("0, ") "] ]} \
        code {[ \
            {[] end}, \
            {[] \
                i32.const 0 i32.const 0 i32.const 64 table.init 0 1 \
                local.get 0 local.get 1 local.get 2 table.copy 0 1 \
                local.get 0 table.get 0 ref.is_null \
            end} \
        ]}");
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t p[] = { {.i64 = 20}, {.i32 = 0}, {.i32 = 64} };
    run_resume(&ctx, 1, p, 3, 3);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0);

    printf("  mixed-address table.copy i32_to_i64 fuel resume verified\n");

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_table_copy_i64_to_i32_fuel_resume(void) {
    printf("Testing mixed-address table.copy i64_to_i32 fuel resume...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [] [], fn [i32, i64, i32] [i32]]} \
        funcs {[0, 1]} \
        tables {[funcref limits.i32/2 200 200, funcref limits.i64/2 200 200]} \
        elements {[ elem.passive elem.funcref [" REP64("0, ") "] ]} \
        code {[ \
            {[] end}, \
            {[] \
                i64.const 0 i32.const 0 i32.const 64 table.init 0 1 \
                local.get 0 local.get 1 local.get 2 table.copy 0 1 \
                local.get 0 table.get 0 ref.is_null \
            end} \
        ]}");
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t p[] = { {.i32 = 20}, {.i64 = 0}, {.i32 = 64} };
    run_resume(&ctx, 1, p, 3, 3);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0);

    printf("  mixed-address table.copy i64_to_i32 fuel resume verified\n");

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// ============================================================
// table64 table.init fuel resume
// ============================================================
static void test_table64_init_fuel_resume(void) {
    printf("Testing table64 table.init fuel resume...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [] [], fn [i64, i32, i32] [i32]]} \
        funcs {[0, 1]} \
        tables {[funcref limits.i64/2 200 200]} \
        elements {[ elem.passive elem.funcref [" REP64("0, ") "] ]} \
        code {[ \
            {[] end}, \
            {[] local.get 0 local.get 1 local.get 2 table.init 0 0 \
                local.get 0 table.get 0 ref.is_null end} \
        ]}");
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t p[] = { {.i64 = 10}, {.i32 = 0}, {.i32 = 64} };
    run_resume(&ctx, 1, p, 3, 3);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0); // not null

    printf("  table64.init(80) fuel resume verified\n");

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// ============================================================
// memory64 memory.fill fuel resume
// ============================================================
static void test_memory64_fill_fuel_resume(void) {
    printf("Testing memory64 memory.fill fuel resume...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [i64, i32, i64] []]} funcs {[0]} \
        memories {[limits.i64/2 1 1]} \
        code {[{[] local.get 0 local.get 1 local.get 2 memory.fill 0 end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t p[] = { {.i64 = 0}, {.i32 = 0xEE}, {.i64 = 256} };
    run_resume(&ctx, 0, p, 3, 3);

    wah_value_t dummy;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &dummy, 0, &actual));

    assert_eq_u32(ctx.memory_base[0], 0xEE);
    assert_eq_u32(ctx.memory_base[255], 0xEE);

    printf("  memory64.fill(256) fuel resume verified\n");

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// ============================================================
// memory64 memory.copy backward fuel resume
// ============================================================
static void test_memory64_copy_backward_fuel_resume(void) {
    printf("Testing memory64 memory.copy backward fuel resume...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    PARSE_FUEL(&mod, "wasm \
        types {[fn [i64, i64, i64] []]} funcs {[0]} \
        memories {[limits.i64/2 1 1]} \
        code {[{[] local.get 0 local.get 1 local.get 2 memory.copy 0 0 end}]}");
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    for (int i = 0; i < 128; i++) ctx.memory_base[i] = (uint8_t)(0xA0 + i);

    // Overlapping: dst=32, src=0, size=128 → backward
    wah_value_t p[] = { {.i64 = 32}, {.i64 = 0}, {.i64 = 128} };
    run_resume(&ctx, 0, p, 3, 3);

    wah_value_t dummy;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &dummy, 0, &actual));

    for (int i = 0; i < 128; i++)
        assert_eq_u32(ctx.memory_base[32 + i], (uint8_t)(0xA0 + i));

    printf("  memory64.copy backward fuel resume verified\n");

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

int main(void) {
    test_memory_fill_fuel();
    test_memory_fill_fuel_resume();
    test_memory_copy_fuel_resume();
    test_memory_copy_backward_fuel_resume();
    test_memory_init_fuel_resume();
    test_table_fill_fuel_resume();
    test_table_copy_fuel_resume();
    test_table_copy_backward_fuel_resume();
    test_table_init_fuel_resume();
    test_array_fill_fuel_resume();
    test_array_copy_fuel_resume();
    test_array_init_data_fuel_resume();
    test_array_init_elem_fuel_resume();
    test_bulk_fuel_proportional();
    test_bulk_no_metering();
    test_table64_fill_fuel_resume();
    test_table64_copy_fuel_resume();
    test_table64_copy_backward_fuel_resume();
    test_table_copy_i32_to_i64_fuel_resume();
    test_table_copy_i64_to_i32_fuel_resume();
    test_table64_init_fuel_resume();
    test_memory64_fill_fuel_resume();
    test_memory64_copy_backward_fuel_resume();

    printf("\n=== All bulk fuel tests passed ===\n");
    return 0;
}
