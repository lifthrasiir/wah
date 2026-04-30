#include "../wah.h"
#include "common.h"
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
    assert_ok(wah_exec_context_create(&c1, &m1, NULL));
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
    assert_ok(wah_exec_context_create(&c2, &m2, NULL));
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
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
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
    assert_ok(wah_exec_context_create(&c2, &m2, NULL));
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
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
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
    assert_ok(wah_exec_context_create(&c2, &m2, NULL));
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
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
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
    assert_ok(wah_exec_context_create(&c2, &m2, NULL));
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
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
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
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
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
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
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
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
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
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
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
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
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
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
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
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_err(wah_call(&ctx, 0, NULL, 0, &result), WAH_ERROR_TRAP);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_ref_test_cast_set_bitset_multiword() {
    printf("Testing ref.test cast-set bitset across multiple words...\n");

    enum { TYPE_COUNT = 70 };
    char spec[12000];
    size_t off = 0;
#define APPEND_SPEC(...) do { \
        int n = snprintf(spec + off, sizeof(spec) - off, __VA_ARGS__); \
        assert_true(n >= 0 && (size_t)n < sizeof(spec) - off); \
        off += (size_t)n; \
    } while (0)

    APPEND_SPEC("wasm types {[ sub [] struct [i32 mut]");
    for (int i = 1; i < TYPE_COUNT; ++i) {
        APPEND_SPEC(", sub [%d] struct [i32 mut]", i - 1);
    }
    APPEND_SPEC(", fn [] [i32] ]} funcs {[ %d ]} code {[ {[] ", TYPE_COUNT);
    APPEND_SPEC("struct.new_default %d ref.test 0 ", TYPE_COUNT - 1);
    APPEND_SPEC("struct.new_default %d ref.test %d i32.add end } ]}", TYPE_COUNT - 1, TYPE_COUNT - 1);
#undef APPEND_SPEC

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    assert_true(module.repr_count > 64);

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 2);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// Cross-module GC opcodes: fctx->module must be used instead of ctx->module
// for typeidx_to_repr, repr_infos, and type_defs lookups.

static void test_cross_module_struct_new_get_set() {
    printf("Testing cross-module struct.new / struct.get / struct.set...\n");

    const char *provider_spec = "wasm \
        types {[ struct [i32 mut], fn [i32] [type.ref.null 0], fn [type.ref.null 0] [i32], \
                 fn [type.ref.null 0, i32] [type.ref.null 0] ]} \
        funcs {[ 1, 2, 3 ]} \
        exports {[ {'make'} fn# 0, {'read'} fn# 1, {'write'} fn# 2 ]} \
        code {[ \
            {[] local.get 0 struct.new 0 end }, \
            {[] local.get 0 struct.get 0 0 end }, \
            {[] local.get 0 local.get 1 struct.set 0 0 local.get 0 end } \
        ]}";

    // Consumer defines the same struct type for cross-module type matching
    const char *consumer_spec = "wasm \
        types {[ struct [i32 mut], fn [i32] [type.ref.null 0], fn [type.ref.null 0] [i32], \
                 fn [type.ref.null 0, i32] [type.ref.null 0], fn [] [i32] ]} \
        imports {[ \
            {'p'} {'make'} fn# 1, \
            {'p'} {'read'} fn# 2, \
            {'p'} {'write'} fn# 3 \
        ]} \
        funcs {[ 4 ]} \
        code {[ {[] \
            i32.const 10 call 0 \
            i32.const 20 call 2 \
            call 1 \
        end } ]}";

    wah_module_t provider = {0}, consumer = {0};
    assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
    assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &consumer, NULL));
    assert_ok(wah_link_module(&ctx, "p", &provider));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 3, NULL, 0, &result));
    assert_eq_i32(result.i32, 20);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&consumer);
    wah_free_module(&provider);
}

static void test_cross_module_struct_new_default() {
    printf("Testing cross-module struct.new_default...\n");

    const char *provider_spec = "wasm \
        types {[ struct [i32 mut], fn [] [type.ref.null 0] ]} \
        funcs {[ 1 ]} \
        exports {[ {'make'} fn# 0 ]} \
        code {[ {[] struct.new_default 0 end } ]}";

    const char *consumer_spec = "wasm \
        types {[ struct [i32 mut], fn [] [type.ref.null 0], fn [] [i32] ]} \
        imports {[ {'p'} {'make'} fn# 1 ]} \
        funcs {[ 2 ]} \
        code {[ {[] call 0 ref.is_null end } ]}";

    wah_module_t provider = {0}, consumer = {0};
    assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
    assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &consumer, NULL));
    assert_ok(wah_link_module(&ctx, "p", &provider));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 0);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&consumer);
    wah_free_module(&provider);
}

static void test_cross_module_array_new_get_set_len() {
    printf("Testing cross-module array.new / array.get / array.set / array.len...\n");

    // Provider: type 0 = array mut i32
    const char *provider_spec = "wasm \
        types {[ array i32 mut, \
                 fn [i32, i32] [type.ref.null 0], \
                 fn [type.ref.null 0] [i32], \
                 fn [type.ref.null 0, i32, i32] [], \
                 fn [type.ref.null 0, i32] [i32] ]} \
        funcs {[ 1, 2, 3, 4 ]} \
        exports {[ {'make'} fn# 0, {'len'} fn# 1, {'set'} fn# 2, {'get'} fn# 3 ]} \
        code {[ \
            {[] local.get 0 local.get 1 array.new 0 end }, \
            {[] local.get 0 array.len end }, \
            {[] local.get 0 local.get 1 local.get 2 array.set 0 end }, \
            {[] local.get 0 local.get 1 array.get 0 end } \
        ]}";

    const char *consumer_spec = "wasm \
        types {[ array i32 mut, \
                 fn [i32, i32] [type.ref.null 0], fn [type.ref.null 0] [i32], \
                 fn [type.ref.null 0, i32, i32] [], fn [type.ref.null 0, i32] [i32], fn [] [i32] ]} \
        imports {[ \
            {'p'} {'make'} fn# 1, {'p'} {'len'} fn# 2, \
            {'p'} {'set'} fn# 3, {'p'} {'get'} fn# 4 \
        ]} \
        funcs {[ 5 ]} \
        code {[ {[1 type.ref.null 0] \
            i32.const 0 i32.const 3 call 0 local.set 0 \
            local.get 0 i32.const 1 i32.const 42 call 2 \
            local.get 0 call 1 \
            local.get 0 i32.const 1 call 3 \
            i32.add \
        end } ]}";

    wah_module_t provider = {0}, consumer = {0};
    assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
    assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &consumer, NULL));
    assert_ok(wah_link_module(&ctx, "p", &provider));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 4, NULL, 0, &result));
    // len=3, get[1]=42, result = 3+42 = 45
    assert_eq_i32(result.i32, 45);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&consumer);
    wah_free_module(&provider);
}

static void test_cross_module_array_new_fixed() {
    printf("Testing cross-module array.new_fixed...\n");

    const char *provider_spec = "wasm \
        types {[ array i32 mut, fn [i32, i32, i32] [type.ref.null 0], fn [type.ref.null 0, i32] [i32] ]} \
        funcs {[ 1, 2 ]} \
        exports {[ {'make3'} fn# 0, {'get'} fn# 1 ]} \
        code {[ \
            {[] local.get 0 local.get 1 local.get 2 array.new_fixed 0 3 end }, \
            {[] local.get 0 local.get 1 array.get 0 end } \
        ]}";

    const char *consumer_spec = "wasm \
        types {[ array i32 mut, fn [i32, i32, i32] [type.ref.null 0], fn [type.ref.null 0, i32] [i32], fn [] [i32] ]} \
        imports {[ {'p'} {'make3'} fn# 1, {'p'} {'get'} fn# 2 ]} \
        funcs {[ 3 ]} \
        code {[ {[] \
            i32.const 10 i32.const 20 i32.const 30 call 0 \
            i32.const 2 call 1 \
        end } ]}";

    wah_module_t provider = {0}, consumer = {0};
    assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
    assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &consumer, NULL));
    assert_ok(wah_link_module(&ctx, "p", &provider));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 2, NULL, 0, &result));
    assert_eq_i32(result.i32, 30);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&consumer);
    wah_free_module(&provider);
}

static void test_cross_module_array_new_default() {
    printf("Testing cross-module array.new_default...\n");

    const char *provider_spec = "wasm \
        types {[ array i32 mut, fn [i32] [type.ref.null 0], fn [type.ref.null 0, i32] [i32] ]} \
        funcs {[ 1, 2 ]} \
        exports {[ {'make'} fn# 0, {'get'} fn# 1 ]} \
        code {[ \
            {[] local.get 0 array.new_default 0 end }, \
            {[] local.get 0 local.get 1 array.get 0 end } \
        ]}";

    const char *consumer_spec = "wasm \
        types {[ array i32 mut, fn [i32] [type.ref.null 0], fn [type.ref.null 0, i32] [i32], fn [] [i32] ]} \
        imports {[ {'p'} {'make'} fn# 1, {'p'} {'get'} fn# 2 ]} \
        funcs {[ 3 ]} \
        code {[ {[] i32.const 3 call 0 i32.const 0 call 1 end } ]}";

    wah_module_t provider = {0}, consumer = {0};
    assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
    assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &consumer, NULL));
    assert_ok(wah_link_module(&ctx, "p", &provider));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 2, NULL, 0, &result));
    assert_eq_i32(result.i32, 0);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&consumer);
    wah_free_module(&provider);
}

static void test_cross_module_array_fill() {
    printf("Testing cross-module array.fill...\n");

    // make_and_fill(val): creates array[3], fills all with val, returns array
    const char *provider_spec = "wasm \
        types {[ array i32 mut, fn [i32] [type.ref.null 0], fn [type.ref.null 0, i32] [i32] ]} \
        funcs {[ 1, 2 ]} \
        exports {[ {'make'} fn# 0, {'get'} fn# 1 ]} \
        code {[ \
            {[1 type.ref.null 0] \
                i32.const 0 i32.const 3 array.new 0 local.set 1 \
                local.get 1 i32.const 0 local.get 0 i32.const 3 array.fill 0 \
                local.get 1 end }, \
            {[] local.get 0 local.get 1 array.get 0 end } \
        ]}";

    const char *consumer_spec = "wasm \
        types {[ array i32 mut, fn [i32] [type.ref.null 0], fn [type.ref.null 0, i32] [i32], fn [] [i32] ]} \
        imports {[ {'p'} {'make'} fn# 1, {'p'} {'get'} fn# 2 ]} \
        funcs {[ 3 ]} \
        code {[ {[] i32.const 77 call 0 i32.const 1 call 1 end } ]}";

    wah_module_t provider = {0}, consumer = {0};
    assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
    assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &consumer, NULL));
    assert_ok(wah_link_module(&ctx, "p", &provider));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 2, NULL, 0, &result));
    assert_eq_i32(result.i32, 77);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&consumer);
    wah_free_module(&provider);
}

static void test_array_copy() {
    printf("Testing array.copy...\n");

    // Create two arrays, fill src with values, copy a range to dst, verify
    const char *spec = "wasm \
        types {[ array i32 mut, fn [] [i32] ]} \
        funcs {[ 1 ]} \
        code {[ {[2 type.ref.null 0] \
            i32.const 0 i32.const 4 array.new 0 local.set 0 \
            i32.const 0 i32.const 4 array.new 0 local.set 1 \
            local.get 0 i32.const 0 i32.const 10 array.set 0 \
            local.get 0 i32.const 1 i32.const 20 array.set 0 \
            local.get 0 i32.const 2 i32.const 30 array.set 0 \
            local.get 0 i32.const 3 i32.const 40 array.set 0 \
            local.get 1 i32.const 1 local.get 0 i32.const 0 i32.const 3 array.copy 0 0 \
            local.get 1 i32.const 1 array.get 0 \
            local.get 1 i32.const 2 array.get 0 i32.add \
            local.get 1 i32.const 3 array.get 0 i32.add \
        end } ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 60);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_array_new_data() {
    printf("Testing array.new_data...\n");

    // Passive data segment with bytes [0x0A, 0x14, 0x1E, 0x28] (10, 20, 30, 40)
    // array.new_data creates an i8 array from data segment
    const char *spec = "wasm \
        types {[ array i8 mut, fn [] [i32] ]} \
        funcs {[ 1 ]} \
        datacount { 1 } \
        code {[ {[] \
            i32.const 0 i32.const 4 array.new_data 0 0 \
            i32.const 0 array.get_u 0 \
            i32.const 10 i32.eq \
        end } ]} \
        data {[ data.passive {%'0A141E28'} ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 1);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_array_init_data() {
    printf("Testing array.init_data...\n");

    // Create array with default values, then init from passive data segment
    const char *spec = "wasm \
        types {[ array i8 mut, fn [] [i32] ]} \
        funcs {[ 1 ]} \
        datacount { 1 } \
        code {[ {[1 type.ref.null 0] \
            i32.const 0 i32.const 4 array.new 0 local.set 0 \
            local.get 0 i32.const 1 i32.const 0 i32.const 2 array.init_data 0 0 \
            local.get 0 i32.const 0 array.get_u 0 \
            local.get 0 i32.const 1 array.get_u 0 i32.add \
            local.get 0 i32.const 2 array.get_u 0 i32.add \
        end } ]} \
        data {[ data.passive {%'0A141E28'} ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    // array[0] = 0 (default), array[1] = 0x0A (10), array[2] = 0x14 (20)
    assert_eq_i32(result.i32, 30);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_array_new_huge_i8_length_oom() {
    printf("Testing array.new huge i8 length reports OOM...\n");

    const char *spec = "wasm \
        types {[ array i8 mut, fn [] [i32] ]} \
        funcs {[ 1 ]} \
        code {[ {[] \
            i32.const 7 i32.const -12 array.new 0 \
            i32.const 1 array.get 0 \
        end } ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_err(wah_call(&ctx, 0, NULL, 0, &result), WAH_ERROR_OUT_OF_MEMORY);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_struct_packed_fields() {
    printf("Testing struct with packed i8/i16 fields...\n");

    // struct type 0: {i8 mut, i16 mut}
    // func type 1: () -> (i32)
    // Create struct, read packed fields with sign/zero extension
    const char *spec = "wasm \
        types {[ struct [i8 mut, i16 mut], fn [] [i32] ]} \
        funcs {[ 1 ]} \
        code {[ {[1 type.ref.null 0] \
            i32.const 200 \
            i32.const 40000 \
            struct.new 0 \
            local.set 0 \
            local.get 0 struct.get_u 0 0 \
            local.get 0 struct.get_s 0 1 \
            i32.add \
        end } ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    // i8: 200 as u8, get_u -> 200
    // i16: 40000 as u16, get_s -> (int16_t)40000 = -25536
    assert_eq_i32(result.i32, 200 + (-25536));

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_struct_i64_f64_fields() {
    printf("Testing struct with i64 and f64 fields...\n");

    const char *spec = "wasm \
        types {[ struct [i64 mut, f64 mut], fn [] [i64] ]} \
        funcs {[ 1 ]} \
        code {[ {[1 type.ref.null 0] \
            i64.const 123456789 \
            f64.const 3.14f64 \
            struct.new 0 \
            local.set 0 \
            local.get 0 struct.get 0 0 \
        end } ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i64(result.i64, 123456789);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_struct_v128_field() {
    printf("Testing struct with v128 field (regression: size=0 in struct handlers)...\n");

    const char *spec = "wasm \
        types {[ struct [v128 mut], fn [] [v128] ]} \
        funcs {[ 1 ]} \
        code {[ {[1 type.ref.null 0] \
            v128.const %'0102030405060708090a0b0c0d0e0f10' \
            struct.new 0 \
            local.set 0 \
            local.get 0 struct.get 0 0 \
        end } ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    wah_v128_t expected = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10}};
    assert_true(memcmp(&result.v128, &expected, sizeof(wah_v128_t)) == 0);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_br_on_null_with_drop() {
    printf("Testing br_on_null with stack values to drop...\n");

    // Type 0: struct {i32 mut}
    // Type 1: fn [] [i32] (main func type)
    // Type 2: fn [i32] [i32] (block type: takes i32 param, returns i32)
    //
    // Push i32(99) as block input. Inside block: push i32(42), push null ref.
    // br_on_null branches: keeps i32(42) as result, drops param i32(99).
    // Fallthrough: drop ref, drop extra i32, leave param as result.
    const char *spec = "wasm \
        types {[ struct [i32 mut], fn [] [i32], fn [i32] [i32] ]} \
        funcs {[ 1 ]} \
        code {[ {[] \
            i32.const 99 \
            block 2 \
                i32.const 42 \
                ref.null 0 \
                br_on_null 0 \
                drop \
                drop \
            end \
        end } ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    // br_on_null branches (null ref), keeps top i32 (42), drops param i32 (99)
    assert_eq_i32(result.i32, 42);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_br_on_non_null_with_drop() {
    printf("Testing br_on_non_null with stack values to drop...\n");

    // Type 0: struct {i32 mut}
    // Type 1: fn [] [i32] (main func type)
    // Type 2: fn [i32] [(ref null 0)] (block type: takes i32 param, returns ref)
    //
    // Push i32(99) as block input. Inside: create struct(77), br_on_non_null.
    // Branch: ref is non-null, branches with ref as result, drops param i32(99).
    // Fallthrough: drop i32 param, push ref.null.
    const char *spec = "wasm \
        types {[ struct [i32 mut], fn [] [i32], fn [i32] [type.ref.null 0] ]} \
        funcs {[ 1 ]} \
        code {[ {[] \
            i32.const 99 \
            block 2 \
                i32.const 77 struct.new 0 \
                br_on_non_null 0 \
                drop \
                ref.null 0 \
            end \
            struct.get 0 0 \
        end } ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    // br_on_non_null branches with the non-null ref, drops param i32(99)
    // struct field 0 = 77
    assert_eq_i32(result.i32, 77);

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
    test_ref_test_cast_set_bitset_multiword();
    test_cross_module_struct_new_get_set();
    test_cross_module_struct_new_default();
    test_cross_module_array_new_get_set_len();
    test_cross_module_array_new_fixed();
    test_cross_module_array_new_default();
    test_cross_module_array_fill();
    test_array_copy();
    test_array_new_data();
    test_array_init_data();
    test_array_new_huge_i8_length_oom();
    test_struct_packed_fields();
    test_struct_i64_f64_fields();
    test_struct_v128_field();
    test_br_on_null_with_drop();
    test_br_on_non_null_with_drop();
    printf("All GC ops tests passed!\n");
    return 0;
}
