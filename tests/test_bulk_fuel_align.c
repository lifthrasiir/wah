// Regression test: array.copy and array.init_data must round bulk chunks
// to element-size boundaries when yielding for fuel exhaustion.
// WAH_BULK_ITEMS_PER_FUEL=7 makes fuel grant 7-byte chunks, which is not
// a multiple of esz=4 (i32 arrays). Before the fix, the truncated
// elem_done = done/esz on yield would desync dst (element-indexed) from
// src (byte-indexed in array.init_data), corrupting resumed copies.
#define WAH_BULK_CHECK_INTERVAL 64
#define WAH_BULK_ITEMS_PER_FUEL 7
#define WAH_IMPLEMENTATION
#include "common.h"

static const wah_parse_options_t fuel_opts = { .features = WAH_FEATURE_ALL, .enable_fuel_metering = true };

#define PARSE_FUEL(mod, spec, ...) \
    assert_ok(wah_parse_module_from_spec_ex((mod), &fuel_opts, (spec), ##__VA_ARGS__))

// ============================================================
// array.init_data with i32 elements: fuel chunk misalignment
// ============================================================
static void test_array_init_data_i32_fuel_align(void) {
    printf("Testing array.init_data i32 fuel chunk alignment...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // 32 i32 elements = 128 bytes of data.
    // Data segment: 0x01000000 0x02000000 ... 0x20000000 (LE i32 1..32)
    // func 0: create i32 array[32], init from data, return sum of all elements
    PARSE_FUEL(&mod, "wasm \
        types {[array i32 mut, fn [] [i32]]} \
        funcs {[1]} \
        datacount {1} \
        code {[{[1 type.ref.null 0, 1 i32, 1 i32] \
            i32.const 0 i32.const 32 array.new 0 local.set 0 \
            local.get 0 i32.const 0 i32.const 0 i32.const 32 array.init_data 0 0 \
            i32.const 0 local.set 1 \
            i32.const 0 local.set 2 \
            block void loop void \
                local.get 2 i32.const 32 i32.ge_u br_if 1 \
                local.get 1 local.get 0 local.get 2 array.get 0 i32.add local.set 1 \
                local.get 2 i32.const 1 i32.add local.set 2 \
                br 0 \
            end end \
            local.get 1 \
        end}]} \
        data {[data.passive {%'" \
            "01000000" "02000000" "03000000" "04000000" \
            "05000000" "06000000" "07000000" "08000000" \
            "09000000" "0A000000" "0B000000" "0C000000" \
            "0D000000" "0E000000" "0F000000" "10000000" \
            "11000000" "12000000" "13000000" "14000000" \
            "15000000" "16000000" "17000000" "18000000" \
            "19000000" "1A000000" "1B000000" "1C000000" \
            "1D000000" "1E000000" "1F000000" "20000000" \
            "'}]}");

    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    // Run with fuel=1 per step to maximize yield frequency.
    // With ITEMS_PER_FUEL=7 and esz=4, each fuel grants 7 bytes.
    // Before the fix, chunk=7 meant done=7, elem_done=7/4=1, losing 3 bytes
    // of alignment on each yield.
    assert_ok(wah_set_fuel(&ctx, 1));
    assert_ok(wah_start(&ctx, 0, NULL, 0));
    int suspensions = 0;
    wah_error_t err;
    while ((err = wah_resume(&ctx)) > 0) {
        suspensions++;
        assert_ok(wah_set_fuel(&ctx, 1));
    }
    assert_ok(err);
    assert_true(suspensions > 0);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));

    // sum(1..32) = 32*33/2 = 528
    printf("  result=%d (expected 528), suspensions=%d\n", result.i32, suspensions);
    assert_eq_i32(result.i32, 528);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

// ============================================================
// array.copy with i32 elements: fuel chunk misalignment
// ============================================================
static void test_array_copy_i32_fuel_align(void) {
    printf("Testing array.copy i32 fuel chunk alignment...\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    // Create src array with distinct values, copy to dst, verify all elements
    PARSE_FUEL(&mod, "wasm \
        types {[array i32 mut, fn [] [i32]]} \
        funcs {[1]} \
        code {[{[2 type.ref.null 0, 1 i32, 1 i32, 1 i32] \
            i32.const 0 i32.const 64 array.new 0 local.set 0 \
            i32.const 0 i32.const 64 array.new 0 local.set 1 \
            i32.const 0 local.set 2 \
            block void loop void \
                local.get 2 i32.const 64 i32.ge_u br_if 1 \
                local.get 0 local.get 2 local.get 2 i32.const 100 i32.add array.set 0 \
                local.get 2 i32.const 1 i32.add local.set 2 \
                br 0 \
            end end \
            local.get 1 i32.const 0 local.get 0 i32.const 0 i32.const 64 array.copy 0 0 \
            i32.const 0 local.set 3 \
            i32.const 0 local.set 2 \
            block void loop void \
                local.get 2 i32.const 64 i32.ge_u br_if 1 \
                local.get 3 \
                local.get 1 local.get 2 array.get 0 \
                local.get 2 i32.const 100 i32.add \
                i32.eq \
                i32.add local.set 3 \
                local.get 2 i32.const 1 i32.add local.set 2 \
                br 0 \
            end end \
            local.get 3 \
        end}]}");

    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    assert_ok(wah_set_fuel(&ctx, 1));
    assert_ok(wah_start(&ctx, 0, NULL, 0));
    int suspensions = 0;
    wah_error_t err;
    while ((err = wah_resume(&ctx)) > 0) {
        suspensions++;
        assert_ok(wah_set_fuel(&ctx, 1));
    }
    assert_ok(err);
    assert_true(suspensions > 0);

    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));

    // All 64 elements should match (count of matching elements)
    printf("  matching=%d (expected 64), suspensions=%d\n", result.i32, suspensions);
    assert_eq_i32(result.i32, 64);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

int main(void) {
    test_array_init_data_i32_fuel_align();
    test_array_copy_i32_fuel_align();
    printf("\n=== All bulk fuel alignment tests passed ===\n");
    return 0;
}
