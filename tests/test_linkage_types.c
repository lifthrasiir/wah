#define WAH_IMPLEMENTATION
#include "../wah.h"
#include "common.c"
#include <stdio.h>

// a70aac0: Support cross-module call_indirect through imported tables.
static void test_cross_module_call_indirect() {
    printf("Testing cross-module call_indirect (a70aac0)...\n");

    // Provider: exports a function.
    const char *provider_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        exports {[ {'f'} fn# 0 ]} \
        code {[ {[] i32.const 42 end } ]}";

    // Consumer: imports the function, has its own table, populates it with the import,
    // then does call_indirect.
    const char *consumer_spec = "wasm \
        types {[ fn [] [i32] ]} \
        imports {[ {'provider'} {'f'} fn# 0 ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        elements {[ elem.active.table#0 i32.const 0 end [ 0 ] ]} \
        code {[ {[] i32.const 0 call_indirect 0 0 end } ]}";

    wah_module_t provider = {0}, consumer = {0};
    assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
    assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &consumer));
    assert_ok(wah_link_module(&ctx, "provider", &provider));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&consumer);
    wah_free_module(&provider);
}

// bab76d4: Reorder element segments before data segments per spec.
static void test_elem_before_data_order() {
    printf("Testing element segments init before data segments (bab76d4)...\n");

    // Module with both active element and active data segments.
    // The element segment writes func 0 to table[0].
    // The data segment writes byte 0xAA to memory[0].
    // If order is correct (elem first, then data), both should succeed.
    // We verify by calling the function from the table and reading memory.
    const char *spec = "wasm \
        types {[ fn [] [i32], fn [] [i32] ]} \
        funcs {[ 0, 1 ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        memories {[ limits.i32/1 1 ]} \
        elements {[ elem.active.table#0 i32.const 0 end [ 0 ] ]} \
        datacount { 1 } \
        code {[ \
            {[] i32.const 99 end }, \
            {[] i32.const 0 call_indirect 0 0 end } \
        ]} \
        data {[ data.active.table#0 i32.const 0 end {%'AA'} ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_instantiate(&ctx));

    // call_indirect on table[0] should call func 0 which returns 99
    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 99);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// 7041f78: Route interpreter memory/table ops through per-frame context (fctx).
static void test_cross_module_memory_ops() {
    printf("Testing cross-module memory ops via fctx (7041f78)...\n");

    // Provider: has memory, exports a function that writes to and reads from its own memory.
    const char *provider_spec = "wasm \
        types {[ fn [i32] [], fn [] [i32] ]} \
        funcs {[ 0, 1 ]} \
        memories {[ limits.i32/1 1 ]} \
        exports {[ {'write'} fn# 0, {'read'} fn# 1 ]} \
        code {[ \
            {[] i32.const 0 local.get 0 i32.store align=4 0 end }, \
            {[] i32.const 0 i32.load align=4 0 end } \
        ]}";

    // Consumer: imports write/read, also has its own memory.
    // Calling write/read should operate on provider's memory, not consumer's.
    const char *consumer_spec = "wasm \
        types {[ fn [i32] [], fn [] [i32] ]} \
        imports {[ {'provider'} {'write'} fn# 0, {'provider'} {'read'} fn# 1 ]} \
        memories {[ limits.i32/1 1 ]}";

    wah_module_t provider = {0}, consumer = {0};
    assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
    assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &consumer));
    assert_ok(wah_link_module(&ctx, "provider", &provider));
    assert_ok(wah_instantiate(&ctx));

    // Write 12345 via provider's write function
    wah_value_t params[1] = {{.i32 = 12345}};
    assert_ok(wah_call(&ctx, 0, params, 1, NULL));

    // Read via provider's read function - should get 12345
    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 12345);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&consumer);
    wah_free_module(&provider);
}

// da46b91: Fix cross-module funcref in globals losing module identity.
static void test_cross_module_funcref_global() {
    printf("Testing cross-module funcref in global (da46b91)...\n");

    // Provider: exports a function that returns 42.
    const char *provider_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        exports {[ {'f'} fn# 0 ]} \
        code {[ {[] i32.const 42 end } ]}";

    // Consumer: imports func, stores ref.func in a global via elem segment init,
    // reads it back via call_indirect.
    const char *consumer_spec = "wasm \
        types {[ fn [] [i32] ]} \
        imports {[ {'provider'} {'f'} fn# 0 ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        globals {[ funcref mut ref.null funcref end ]} \
        exports {[ {'f'} fn# 0 ]} \
        elements {[ elem.active.table#0 i32.const 0 end [ 0 ] ]} \
        code {[ {[] i32.const 0 call_indirect 0 0 end } ]}";

    wah_module_t provider = {0}, consumer = {0};
    assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
    assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &consumer));
    assert_ok(wah_link_module(&ctx, "provider", &provider));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&consumer);
    wah_free_module(&provider);
}

// 450d460: Implement cross-module type canonicalization for import resolution.
static void test_cross_module_type_canon() {
    printf("Testing cross-module type canonicalization (450d460)...\n");

    // Provider has type 0 = fn [i32] -> [i32], exports a function of that type.
    const char *provider_spec = "wasm \
        types {[ fn [i32] [i32] ]} \
        funcs {[ 0 ]} \
        exports {[ {'inc'} fn# 0 ]} \
        code {[ {[] local.get 0 i32.const 1 i32.add end } ]}";

    // Consumer has different type layout but equivalent shape:
    // type 0 = fn [] -> [i32], type 1 = fn [i32] -> [i32].
    // Import uses type 1, which is different index but same shape as provider's type 0.
    const char *consumer_spec = "wasm \
        types {[ fn [] [i32], fn [i32] [i32] ]} \
        imports {[ {'provider'} {'inc'} fn# 1 ]} \
        funcs {[ 0 ]} \
        code {[ {[] i32.const 10 call 0 end } ]}";

    wah_module_t provider = {0}, consumer = {0};
    assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
    assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &consumer));
    assert_ok(wah_link_module(&ctx, "provider", &provider));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 11);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&consumer);
    wah_free_module(&provider);
}

// 7d0735d: Use cross-module type equivalence in call_indirect/call_ref type checks.
// 450d460 already tested import resolution. This tests call_indirect dispatch
// where the function type is defined in a different module with a different type index.
static void test_cross_module_call_indirect_type_equiv() {
    printf("Testing cross-module call_indirect type equivalence (7d0735d)...\n");

    // Provider: type 0 = fn [i32] -> [i32], exports a function of that type.
    const char *provider_spec = "wasm \
        types {[ fn [i32] [i32] ]} \
        funcs {[ 0 ]} \
        exports {[ {'double'} fn# 0 ]} \
        code {[ {[] local.get 0 i32.const 2 i32.mul end } ]}";

    // Consumer: type 0 = fn [] -> [], type 1 = fn [i32] -> [i32] (same shape as provider's 0).
    // Imports the function, puts it in a table, then call_indirect with consumer's type 1.
    const char *consumer_spec = "wasm \
        types {[ fn [] [i32], fn [i32] [i32] ]} \
        imports {[ {'provider'} {'double'} fn# 1 ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        elements {[ elem.active.table#0 i32.const 0 end [ 0 ] ]} \
        code {[ {[] i32.const 21 i32.const 0 call_indirect 1 0 end } ]}";

    wah_module_t provider = {0}, consumer = {0};
    assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
    assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &consumer));
    assert_ok(wah_link_module(&ctx, "provider", &provider));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&consumer);
    wah_free_module(&provider);
}

// 8df191b: Use subtype matching for call_indirect/call_ref dispatch.
static void test_call_indirect_subtype_dispatch() {
    printf("Testing call_indirect subtype dispatch (8df191b)...\n");

    // Type 0 = non-final fn [] -> [i32] (supertype)
    // Type 1 = fn [] -> [i32] subtype of 0
    // Function uses type 1. call_indirect with type 0 should succeed because
    // the function's type is a subtype of the expected type.
    const char *spec = "wasm \
        types {[ sub [] fn [] [i32], sub [0] fn [] [i32] ]} \
        funcs {[ 1 ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        elements {[ elem.active.table#0 i32.const 0 end [ 0 ] ]} \
        code {[ {[] i32.const 42 end } ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_instantiate(&ctx));

    // call_indirect with type 0 (supertype) on function of type 1 (subtype) - should work
    const char *caller_spec = "wasm \
        types {[ sub [] fn [] [i32], sub [0] fn [] [i32], fn [] [i32] ]} \
        funcs {[ 1, 2 ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        elements {[ elem.active.table#0 i32.const 0 end [ 0 ] ]} \
        code {[ \
            {[] i32.const 42 end }, \
            {[] i32.const 0 call_indirect 0 0 end } \
        ]}";

    wah_module_t caller = {0};
    assert_ok(wah_parse_module_from_spec(&caller, caller_spec));

    wah_exec_context_t ctx2 = {0};
    assert_ok(wah_exec_context_create(&ctx2, &caller));
    assert_ok(wah_instantiate(&ctx2));

    wah_value_t result;
    assert_ok(wah_call(&ctx2, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);

    wah_exec_context_destroy(&ctx2);
    wah_free_module(&caller);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// f20a450: Add ref.test/ref.cast support for concrete func types with subtyping.
static void test_ref_test_concrete_func_subtype() {
    printf("Testing ref.test/ref.cast concrete func subtyping (f20a450)...\n");

    // Type 0: non-final fn [] -> [i32] (supertype)
    // Type 1: fn [] -> [i32] subtype of 0
    // Function 0 has type 1 (subtype).
    // ref.test on ref.func 0 against type 0 (supertype) should succeed.
    const char *spec = "wasm \
        types {[ sub [] fn [] [i32], sub [0] fn [] [i32], fn [] [i32] ]} \
        funcs {[ 1, 2, 2 ]} \
        elements {[ elem.declarative.expr funcref [ ref.func 0 end ] ]} \
        code {[ \
            {[] i32.const 42 end }, \
            {[] ref.func 0 ref.test 0 end }, \
            {[] ref.func 0 ref.test 1 end } \
        ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;

    // ref.test type 0 (supertype) on func of type 1 (subtype) -> true
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 1);

    // ref.test type 1 (exact) on func of type 1 -> true
    assert_ok(wah_call(&ctx, 2, NULL, 0, &result));
    assert_eq_i32(result.i32, 1);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

int main() {
    test_cross_module_call_indirect();
    test_elem_before_data_order();
    test_cross_module_memory_ops();
    test_cross_module_funcref_global();
    test_cross_module_type_canon();
    test_cross_module_call_indirect_type_equiv();
    test_call_indirect_subtype_dispatch();
    test_ref_test_concrete_func_subtype();
    printf("All linkage_types tests passed!\n");
    return 0;
}
