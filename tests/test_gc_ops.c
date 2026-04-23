#define WAH_IMPLEMENTATION
#include "../wah.h"
#include "common.c"
#include <stdio.h>

// d39a915: Add i31 and extern conversion GC opcodes.
static void test_i31_ops() {
    printf("Testing i31 ops (d39a915)...\n");

    // ref.i31 wraps i32 to i31ref, i31.get_s/u extracts it
    const char *spec_s = "wasm \
        types {[ fn [i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] local.get 0 ref.i31 i31.get_s end } ]}";

    wah_module_t m1 = {0};
    assert_ok(wah_parse_module_from_spec(&m1, spec_s));
    wah_exec_context_t c1 = {0};
    assert_ok(wah_exec_context_create(&c1, &m1));
    assert_ok(wah_instantiate(&c1));

    wah_value_t params[1], result;

    // Positive value
    params[0].i32 = 42;
    assert_ok(wah_call(&c1, 0, params, 1, &result));
    assert_eq_i32(result.i32, 42);

    // Max i31 value (2^30 - 1 = 1073741823)
    params[0].i32 = 0x3FFFFFFF;
    assert_ok(wah_call(&c1, 0, params, 1, &result));
    assert_eq_i32(result.i32, 0x3FFFFFFF);

    // Negative via sign extension: 0x40000000 wraps to i31, get_s returns negative
    params[0].i32 = 0x40000000;
    assert_ok(wah_call(&c1, 0, params, 1, &result));
    assert_eq_i32(result.i32, -0x40000000);

    wah_exec_context_destroy(&c1);
    wah_free_module(&m1);

    // i31.get_u
    const char *spec_u = "wasm \
        types {[ fn [i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] local.get 0 ref.i31 i31.get_u end } ]}";

    wah_module_t m2 = {0};
    assert_ok(wah_parse_module_from_spec(&m2, spec_u));
    wah_exec_context_t c2 = {0};
    assert_ok(wah_exec_context_create(&c2, &m2));
    assert_ok(wah_instantiate(&c2));

    params[0].i32 = 0x7FFFFFFF;
    assert_ok(wah_call(&c2, 0, params, 1, &result));
    assert_eq_u32((uint32_t)result.i32, 0x7FFFFFFFu);

    wah_exec_context_destroy(&c2);
    wah_free_module(&m2);
}

static void test_extern_convert() {
    printf("Testing extern conversion ops (d39a915)...\n");

    // any.convert_extern(extern.convert_any(ref.null any)) == ref.null
    const char *spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            ref.null anyref \
            extern.convert_any \
            any.convert_extern \
            ref.is_null \
        end } ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 1);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);

    // Round-trip with a non-null i31 ref
    const char *spec2 = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            i32.const 99 \
            ref.i31 \
            extern.convert_any \
            any.convert_extern \
            ref.is_null \
        end } ]}";

    wah_module_t m2 = {0};
    assert_ok(wah_parse_module_from_spec(&m2, spec2));
    wah_exec_context_t c2 = {0};
    assert_ok(wah_exec_context_create(&c2, &m2));
    assert_ok(wah_instantiate(&c2));

    assert_ok(wah_call(&c2, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 0);

    wah_exec_context_destroy(&c2);
    wah_free_module(&m2);
}

// 4997e24: Add GC validation and fix i31 ref_eq semantics.
static void test_ref_eq_i31() {
    printf("Testing ref.eq with i31 values (4997e24)...\n");

    // Equal i31 values
    const char *spec = "wasm \
        types {[ fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] local.get 0 ref.i31 local.get 1 ref.i31 ref.eq end } ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t params[2], result;

    // Equal values
    params[0].i32 = 42; params[1].i32 = 42;
    assert_ok(wah_call(&ctx, 0, params, 2, &result));
    assert_eq_i32(result.i32, 1);

    // Unequal values
    params[0].i32 = 42; params[1].i32 = 43;
    assert_ok(wah_call(&ctx, 0, params, 2, &result));
    assert_eq_i32(result.i32, 0);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);

    // i31 vs null
    const char *spec2 = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] i32.const 0 ref.i31 ref.null eqref ref.eq end } ]}";

    wah_module_t m2 = {0};
    assert_ok(wah_parse_module_from_spec(&m2, spec2));
    wah_exec_context_t c2 = {0};
    assert_ok(wah_exec_context_create(&c2, &m2));
    assert_ok(wah_instantiate(&c2));

    assert_ok(wah_call(&c2, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 0);

    wah_exec_context_destroy(&c2);
    wah_free_module(&m2);
}

// c55a9f0: Implement array GC opcodes.
static void test_array_ops() {
    printf("Testing array GC ops (c55a9f0)...\n");

    // Define array type: (array (mut i32))
    // Type 0 = array type, type 1 = func type
    const char *spec = "wasm \
        types {[ sub [] array i32 mut, fn [] [i32] ]} \
        funcs {[ 1 ]} \
        code {[ {[] \
            i32.const 42 \
            i32.const 3 \
            array.new 0 \
            i32.const 1 \
            array.get 0 \
        end } ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);

    // array.new_default + array.set + array.get + array.len
    // Use local to hold the array ref
    const char *spec2 = "wasm \
        types {[ sub [] array i32 mut, fn [] [i32] ]} \
        funcs {[ 1, 1 ]} \
        code {[ \
            {[1 type.ref.null 0] \
                i32.const 5 \
                array.new_default 0 \
                local.set 0 \
                local.get 0 \
                i32.const 2 \
                i32.const 99 \
                array.set 0 \
                local.get 0 \
                i32.const 2 \
                array.get 0 \
            end }, \
            {[] \
                i32.const 0 \
                i32.const 3 \
                array.new 0 \
                array.len \
            end } \
        ]}";

    wah_module_t m2 = {0};
    assert_ok(wah_parse_module_from_spec(&m2, spec2));
    wah_exec_context_t c2 = {0};
    assert_ok(wah_exec_context_create(&c2, &m2));
    assert_ok(wah_instantiate(&c2));

    // array.set at index 2 then array.get
    assert_ok(wah_call(&c2, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 99);

    // array.len
    assert_ok(wah_call(&c2, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 3);

    wah_exec_context_destroy(&c2);
    wah_free_module(&m2);
}

// a4f274e: Support GC struct/array constructors in const expressions.
static void test_gc_const_expr() {
    printf("Testing GC struct in const expr (a4f274e)...\n");

    // Global initialized with struct.new in const expr, read field back.
    // Type 0 = struct { mut i32 }, type 1 = func type
    const char *spec = "wasm \
        types {[ sub [] struct [i32 mut], fn [] [i32] ]} \
        funcs {[ 1 ]} \
        globals {[ type.ref.null 0 immut i32.const 99 struct.new 0 end ]} \
        code {[ {[] global.get 0 struct.get 0 0 end } ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 99);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// b83aea8: Fix array_init_elem with dropped segments and size 0.
static void test_array_init_elem_dropped() {
    printf("Testing array_init_elem dropped segment size 0 (b83aea8)...\n");

    // Create a real array, init from elem, drop elem, then init again with size 0 (OK) vs size>0 (trap).
    const char *spec = "wasm \
        types {[ sub [] array funcref mut, fn [] [i32], fn [] [i32] ]} \
        funcs {[ 1, 2 ]} \
        elements {[ elem.passive.expr funcref [ ref.null funcref end ] ]} \
        code {[ \
            {[] \
                ref.null funcref \
                i32.const 2 \
                array.new 0 \
                i32.const 0 \
                i32.const 0 \
                i32.const 1 \
                array.init_elem 0 0 \
                elem.drop 0 \
                ref.null funcref \
                i32.const 2 \
                array.new 0 \
                i32.const 0 \
                i32.const 0 \
                i32.const 0 \
                array.init_elem 0 0 \
                i32.const 1 \
            end }, \
            {[] \
                ref.null funcref \
                i32.const 2 \
                array.new 0 \
                i32.const 0 \
                i32.const 0 \
                i32.const 1 \
                array.init_elem 0 0 \
                i32.const 0 \
            end } \
        ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    // func 0: init, drop, then init size=0 after drop -> OK
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 1);

    // func 1: elem was dropped by func 0, init size=1 -> trap
    assert_err(wah_call(&ctx, 1, NULL, 0, &result), WAH_ERROR_TRAP);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// 5fcc293: extern.convert_any validation and malformed GC binary fix.
static void test_extern_convert_validation() {
    printf("Testing extern.convert_any validation (5fcc293)...\n");

    // Positive: extern.convert_any on anyref value
    const char *good = "wasm \
        types {[ fn [] [externref] ]} \
        funcs {[ 0 ]} \
        code {[ {[] ref.null anyref extern.convert_any end } ]}";

    wah_module_t m1 = {0};
    assert_ok(wah_parse_module_from_spec(&m1, good));
    wah_free_module(&m1);

    // Malformed: struct with invalid mutability byte (should be WAH_ERROR_MALFORMED, not VALIDATION)
    const char *malformed = "wasm \
        types {[ sub [] struct [i32 %'02'] ]}";

    wah_module_t m3 = {0};
    assert_err(wah_parse_module_from_spec(&m3, malformed), WAH_ERROR_MALFORMED);
    wah_free_module(&m3);
}

// --- Tests moved from test_references.c ---

static void test_struct_new_get_set() {
    printf("Running test_struct_new_get_set...\n");

    const char *spec = "wasm \
        types {[ struct [i32 mut, i32 mut], fn [] [i32] ]} \
        funcs {[ 1 ]} \
        exports {[ {'f'} fn# 0 ]} \
        code {[ {[] i32.const 0 end } ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 0);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_struct_set_and_read() {
    printf("Running test_struct_set_and_read...\n");

    const char *spec = "wasm \
        types {[ struct [i32 mut, i32 mut], fn [] [i32] ]} \
        funcs {[ 1 ]} \
        exports {[ {'f'} fn# 0 ]} \
        code {[ {[1 type.ref.null 0] \
            struct.new_default 0 local.set 0 \
            local.get 0 i32.const 42 struct.set 0 0 \
            local.get 0 i32.const -7 struct.set 0 1 \
            local.get 0 struct.get 0 0 \
            local.get 0 struct.get 0 1 \
            i32.add \
            end } ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 35);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_struct_new_with_values() {
    printf("Running test_struct_new_with_values...\n");

    const char *spec = "wasm \
        types {[ struct [i32 mut, i32 mut], fn [] [i32] ]} \
        funcs {[ 1 ]} \
        exports {[ {'f'} fn# 0 ]} \
        code {[ {[] \
            i32.const 10 i32.const 20 struct.new 0 struct.get 0 1 \
            end } ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 20);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_array_new_get_set_len() {
    printf("Running test_array_new_get_set_len...\n");

    const char *spec = "wasm \
        types {[ array i32 mut, fn [] [i32], fn [] [i32] ]} \
        funcs {[ 1, 2 ]} \
        exports {[ {'get'} fn# 0, {'len'} fn# 1 ]} \
        code {[ \
            {[1 type.ref.null 0] \
                i32.const 0 i32.const 5 array.new 0 local.set 0 \
                local.get 0 i32.const 2 i32.const 99 array.set 0 \
                local.get 0 i32.const 2 array.get 0 \
                end }, \
            {[] \
                i32.const 0 i32.const 7 array.new 0 array.len \
                end } \
        ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 99);

    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 7);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_array_oob_trap() {
    printf("Running test_array_oob_trap...\n");

    const char *spec = "wasm \
        types {[ array i32 mut, fn [] [i32] ]} \
        funcs {[ 1 ]} \
        exports {[ {'f'} fn# 0 ]} \
        code {[ {[] \
            i32.const 0 i32.const 3 array.new 0 \
            i32.const 5 array.get 0 \
            end } ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_err(wah_call(&ctx, 0, NULL, 0, &result), WAH_ERROR_TRAP);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_struct_null_trap() {
    printf("Running test_struct_null_trap...\n");

    const char *spec = "wasm \
        types {[ struct [i32 mut], fn [] [i32] ]} \
        funcs {[ 1 ]} \
        exports {[ {'f'} fn# 0 ]} \
        code {[ {[] \
            ref.null 0 struct.get 0 0 \
            end } ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_exec_context_t ctx;
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_err(wah_call(&ctx, 0, NULL, 0, &result), WAH_ERROR_TRAP);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

int main() {
    test_i31_ops();
    test_extern_convert();
    test_ref_eq_i31();
    test_array_ops();
    test_gc_const_expr();
    test_array_init_elem_dropped();
    test_extern_convert_validation();
    test_struct_new_get_set();
    test_struct_set_and_read();
    test_struct_new_with_values();
    test_array_new_get_set_len();
    test_array_oob_trap();
    test_struct_null_trap();
    printf("All GC ops tests passed!\n");
    return 0;
}
