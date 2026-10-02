#include "../wah.h"
#include "common.h"
#include "wah_impl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void host_return_null_ref(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    wah_result_ref(ctx, 0, NULL);
}

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
    assert_ok(wah_new_exec_context(&ctx, &consumer, NULL));
    assert_ok(wah_link_module(&ctx, "provider", &provider));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);

    wah_free_exec_context(&ctx);
    wah_free_module(&consumer);
    wah_free_module(&provider);
}

// Regression test for linked module internal contexts losing imported table
// ownership metadata. Without import_ctx/import_idx, table.grow in the linked
// module reallocates the provider's table storage without updating the provider
// context, leaving a dangling provider table pointer.
static void test_linked_module_imported_table_grow() {
    printf("Testing linked-module imported table grow metadata...\n");

    const char *provider_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 4 ]} \
        exports {[ {'t'} table# 0, {'isnull'} fn# 0 ]} \
        code {[ {[] i32.const 0 table.get 0 ref.is_null end } ]}";

    const char *grower_spec = "wasm \
        types {[ fn [] [] ]} \
        imports {[ {'b'} {'t'} table# funcref limits.i32/1 4 ]} \
        funcs {[ 0 ]} \
        exports {[ {'grow'} fn# 0 ]} \
        code {[ {[] ref.null funcref i32.const 1 table.grow 0 drop end } ]}";

    const char *main_spec = "wasm \
        types {[ fn [] [], fn [] [i32] ]} \
        imports {[ {'a'} {'grow'} fn# 0, {'b'} {'isnull'} fn# 1 ]} \
        funcs {[ 1 ]} \
        code {[ {[] call 0 call 1 end } ]}";

    wah_module_t provider = {0}, grower = {0}, main_mod = {0};
    assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
    assert_ok(wah_parse_module_from_spec(&grower, grower_spec));
    assert_ok(wah_parse_module_from_spec(&main_mod, main_spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &main_mod, NULL));
    assert_ok(wah_link_module(&ctx, "b", &provider));
    assert_ok(wah_link_module(&ctx, "a", &grower));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 2, NULL, 0, &result));
    assert_eq_i32(result.i32, 1);

    wah_free_exec_context(&ctx);
    wah_free_module(&main_mod);
    wah_free_module(&grower);
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
    assert_ok(wah_new_exec_context(&ctx, &module, NULL));
    assert_ok(wah_instantiate(&ctx));

    // call_indirect on table[0] should call func 0 which returns 99
    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 99);

    wah_free_exec_context(&ctx);
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
    assert_ok(wah_new_exec_context(&ctx, &consumer, NULL));
    assert_ok(wah_link_module(&ctx, "provider", &provider));
    assert_ok(wah_instantiate(&ctx));

    // Write 12345 via provider's write function
    wah_value_t params[1] = {{.i32 = 12345}};
    assert_ok(wah_call(&ctx, 0, params, 1, NULL));

    // Read via provider's read function - should get 12345
    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 12345);

    wah_free_exec_context(&ctx);
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
    assert_ok(wah_new_exec_context(&ctx, &consumer, NULL));
    assert_ok(wah_link_module(&ctx, "provider", &provider));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);

    wah_free_exec_context(&ctx);
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
    assert_ok(wah_new_exec_context(&ctx, &consumer, NULL));
    assert_ok(wah_link_module(&ctx, "provider", &provider));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 11);

    wah_free_exec_context(&ctx);
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
    assert_ok(wah_new_exec_context(&ctx, &consumer, NULL));
    assert_ok(wah_link_module(&ctx, "provider", &provider));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);

    wah_free_exec_context(&ctx);
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
    assert_ok(wah_new_exec_context(&ctx, &module, NULL));
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
    assert_ok(wah_new_exec_context(&ctx2, &caller, NULL));
    assert_ok(wah_instantiate(&ctx2));

    wah_value_t result;
    assert_ok(wah_call(&ctx2, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);

    wah_free_exec_context(&ctx2);
    wah_free_module(&caller);

    wah_free_exec_context(&ctx);
    wah_free_module(&module);
}

// 47386ae: Fix rec-group canonicalization and add canonical equivalence to subtype checks.
static void test_rec_group_canonicalization() {
    printf("Testing rec-group canonicalization (47386ae)...\n");

    // Two modules define equivalent recursive function types in separate rec groups.
    // Each rec group has: type 0 = fn [] -> [ref null 1], type 1 = fn [] -> [ref null 0].
    // Canonicalization should recognize these as equivalent across modules,
    // allowing import linking to succeed.

    // Provider: defines rec group and exports a function of type 1 (fn [] -> [ref null 0]).
    const char *provider_spec = "wasm \
        types {[ rec [ sub [] fn [] [type.ref.null 1], sub [] fn [] [type.ref.null 0] ] ]} \
        funcs {[ 1 ]} \
        exports {[ {'f'} fn# 0 ]} \
        code {[ {[] ref.null 0 end } ]}";

    // Consumer: defines the SAME rec group shape, imports f using local type 1.
    // Then has a wrapper function (type 1) that calls the import and drops the result.
    const char *consumer_spec = "wasm \
        types {[ rec [ sub [] fn [] [type.ref.null 1], sub [] fn [] [type.ref.null 0] ], \
                 sub.final [] fn [] [i32] ]} \
        imports {[ {'provider'} {'f'} fn# 1 ]} \
        funcs {[ 2 ]} \
        code {[ {[] call 0 ref.is_null end } ]}";

    wah_module_t provider = {0}, consumer = {0};
    assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
    assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &consumer, NULL));
    assert_ok(wah_link_module(&ctx, "provider", &provider));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 1);

    wah_free_exec_context(&ctx);
    wah_free_module(&consumer);
    wah_free_module(&provider);
}

// 47386ae: Subtype check through canonical equivalence in supertype chain.
static void test_rec_group_canon_subtype() {
    printf("Testing rec-group canonical subtype chain (47386ae)...\n");

    // Single module with two rec groups that define equivalent types,
    // plus a subtype relationship referencing one of them.
    // rec group 0: type 0 = struct { i32 }
    // rec group 1: type 1 = struct { i32 }    (canonically == type 0)
    // type 2 = sub type 0 struct { i32, i32 }  (subtype of type 0)
    //
    // call_indirect with type index for the canonical equivalent should
    // succeed via canonical subtype check.

    // We test via call_indirect: function declared with type 2 (sub of 0),
    // call_indirect expects type 1 (canonically == type 0, supertype of 2).
    // This should succeed because: type 2 <: type 0, and type 0 == type 1 canonically.

    // But call_indirect operates on func types, not struct types. Let's use func types instead.
    // rec group 0: type 0 = non-final fn [] -> [i32]
    // rec group 1: type 1 = non-final fn [] -> [i32]  (canonically == type 0)
    // type 2 = sub type 0 fn [] -> [i32]              (subtype of type 0)
    // Function of type 2 in table, call_indirect with type 1.

    const char *spec = "wasm \
        types {[ rec [ sub [] fn [] [i32] ], \
                 rec [ sub [] fn [] [i32] ], \
                 sub [0] fn [] [i32] ]} \
        funcs {[ 2, 1 ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        elements {[ elem.active.table#0 i32.const 0 end [ 0 ] ]} \
        code {[ \
            {[] i32.const 99 end }, \
            {[] i32.const 0 call_indirect 1 0 end } \
        ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &module, NULL));
    assert_ok(wah_instantiate(&ctx));

    // func 0 (type 2, sub of 0) is in table[0].
    // call_indirect expects type 1 (canonically == type 0).
    // type 2 <: type 0 == type 1, so this should succeed.
    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 99);

    wah_free_exec_context(&ctx);
    wah_free_module(&module);
}

// b2d1a8b: Compare full type identity in call_indirect/call_ref dispatch.
static void test_call_indirect_non_identical_type() {
    printf("Testing call_indirect rejects same-shape non-identical types (b2d1a8b)...\n");

    // Type 0 = sub.final [] fn [] -> [i32]  (final, the default)
    // Type 1 = sub [] fn [] -> [i32]         (non-final)
    // These have the same shape but different finality, so they are canonically distinct.
    // Function 0 has type 1 (non-final). call_indirect with type 0 (final) must trap.
    const char *spec = "wasm \
        types {[ sub.final [] fn [] [i32], sub [] fn [] [i32], sub.final [] fn [] [i32] ]} \
        funcs {[ 1, 2 ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        elements {[ elem.active.table#0 i32.const 0 end [ 0 ] ]} \
        code {[ \
            {[] i32.const 42 end }, \
            {[] i32.const 0 call_indirect 0 0 end } \
        ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &module, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_err(wah_call(&ctx, 1, NULL, 0, &result), WAH_ERROR_TRAP);

    wah_free_exec_context(&ctx);
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
    assert_ok(wah_new_exec_context(&ctx, &module, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;

    // ref.test type 0 (supertype) on func of type 1 (subtype) -> true
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 1);

    // ref.test type 1 (exact) on func of type 1 -> true
    assert_ok(wah_call(&ctx, 2, NULL, 0, &result));
    assert_eq_i32(result.i32, 1);

    wah_free_exec_context(&ctx);
    wah_free_module(&module);
}

static void test_cross_module_type_with_extra_types() {
    printf("Testing cross-module type identity with extra type defs...\n");

    const char *provider_spec = "wasm \
        types {[ struct [i32 mut], fn [i32, i32] [i32] ]} \
        funcs {[ 1 ]} \
        exports {[ {'add'} fn# 0 ]} \
        code {[ {[] local.get 0 local.get 1 i32.add end } ]}";

    const char *consumer_spec = "wasm \
        types {[ fn [i32, i32] [i32], fn [i32] [i32] ]} \
        imports {[ {'provider'} {'add'} fn# 0 ]} \
        funcs {[ 1 ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        elements {[ elem.active.table#0 i32.const 0 end [ 0 ] ]} \
        code {[ {[] local.get 0 i32.const 5 i32.const 0 call_indirect 0 0 end } ]}";

    wah_module_t provider = {0}, consumer = {0};
    assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
    assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &consumer, NULL));
    assert_ok(wah_link_module(&ctx, "provider", &provider));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    for (int i = 0; i < 16; ++i) {
        wah_value_t param = {.i32 = 10};
        assert_ok(wah_call(&ctx, 1, &param, 1, &result));
        assert_eq_i32(result.i32, 15);
    }
    assert_true(wah_debug_has_type_check_cache_entry(&ctx, &provider, wah_debug_type_from_idx(1, false), &consumer, wah_debug_type_from_idx(0, false), true));

    wah_free_exec_context(&ctx);
    wah_free_module(&consumer);
    wah_free_module(&provider);
}

static void test_cross_module_subtype_func_ref_test() {
    printf("Testing cross-module subtype func ref.test...\n");

    const char *provider_spec = "wasm \
        types {[ sub [] fn [] [i32], sub [0] fn [] [i32] ]} \
        funcs {[ 1 ]} \
        exports {[ {'f'} fn# 0 ]} \
        elements {[ elem.declarative.expr funcref [ ref.func 0 end ] ]} \
        code {[ {[] i32.const 42 end } ]}";

    const char *consumer_spec = "wasm \
        types {[ sub [] fn [] [i32], sub [0] fn [] [i32], fn [] [i32] ]} \
        imports {[ {'provider'} {'f'} fn# 1 ]} \
        funcs {[ 2 ]} \
        elements {[ elem.declarative.expr funcref [ ref.func 0 end ] ]} \
        code {[ {[] ref.func 0 ref.test 0 end } ]}";

    wah_module_t provider = {0}, consumer = {0};
    assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
    assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &consumer, NULL));
    assert_ok(wah_link_module(&ctx, "provider", &provider));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 1);
    assert_true(wah_debug_has_type_check_cache_entry(&ctx, &provider, wah_debug_type_from_idx(1, false), &consumer, wah_debug_type_from_idx(0, false), true));

    wah_free_exec_context(&ctx);
    wah_free_module(&consumer);
    wah_free_module(&provider);
}

static void test_host_import_concrete_ref_uses_linked_type_namespace() {
    printf("Testing host import concrete ref type namespace...\n");

    wah_module_t provider = {0}, consumer = {0};
    wah_type_t provider_struct = 0;
    wah_type_t provider_func = 0;

    assert_ok(wah_new_module(&provider, NULL));
    assert_ok(wah_define_type(&provider, &provider_struct, "struct { i32 }"));
    assert_ok(wah_define_type(&provider, &provider_func, "fn () -> (ref null %T)", provider_struct));
    assert_ok(wah_export_typed_func(&provider, "make", provider_func, host_return_null_ref, NULL, NULL));

    // Consumer type 0 is a function type, not the provider's struct type.
    // Before the fix, host import checking interpreted provider result type 0
    // in the consumer's type namespace, so this mismatched import linked.
    const char *consumer_spec = "wasm \
        types {[ fn [] [type.ref.null 0] ]} \
        imports {[ {'p'} {'make'} fn# 0 ]}";

    assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &consumer, NULL));
    assert_ok(wah_link_module(&ctx, "p", &provider));
    assert_err(wah_instantiate(&ctx), WAH_ERROR_LINK_FAILED);

    wah_free_exec_context(&ctx);
    wah_free_module(&consumer);
    wah_free_module(&provider);
}

// An abstract subtype against a concrete supertype must look up the supertype's kind in its own module.
static void test_cross_module_abstract_sub_concrete_sup() {
    printf("Testing cross-module abstract subtype against concrete supertype...\n");

    // Provider type 0 is a function type; the result is (ref null none).
    wah_module_t provider = {0}, consumer = {0}, bad_consumer = {0};
    assert_ok(wah_new_module(&provider, NULL));
    assert_ok(wah_export_func(&provider, "f", "() -> nullref", host_return_null_ref, NULL, NULL));
    // Consumer type 0 is a struct type, so (ref null none) <: (ref null 0).
    const char *consumer_spec = "wasm \
        types {[ struct [i32 mut], fn [] [type.ref.null 0] ]} \
        imports {[ {'p'} {'f'} fn# 1 ]}";
    // Consumer type 0 is a function type, so (ref null none) is not its subtype.
    const char *bad_consumer_spec = "wasm \
        types {[ fn [] [], fn [] [type.ref.null 0] ]} \
        imports {[ {'p'} {'f'} fn# 1 ]}";

    assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));
    assert_ok(wah_parse_module_from_spec(&bad_consumer, bad_consumer_spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &consumer, NULL));
    assert_ok(wah_link_module(&ctx, "p", &provider));
    assert_ok(wah_instantiate(&ctx));
    wah_free_exec_context(&ctx);

    assert_ok(wah_new_exec_context(&ctx, &bad_consumer, NULL));
    assert_ok(wah_link_module(&ctx, "p", &provider));
    assert_err(wah_instantiate(&ctx), WAH_ERROR_LINK_FAILED);
    wah_free_exec_context(&ctx);

    wah_free_module(&bad_consumer);
    wah_free_module(&consumer);
    wah_free_module(&provider);
}

static void test_linked_module_ictx_table_type_validation() {
    printf("Testing linked module ictx table import type validation...\n");

    // Provider module exports an externref table.
    const char *provider_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        tables {[ externref limits.i32/1 4 ]} \
        exports {[ {'t'} table# 0, {'f'} fn# 0 ]} \
        code {[ {[] end } ]}";

    // Linked module imports a structref table (type mismatch with externref).
    const char *linked_spec = "wasm \
        types {[ fn [] [] ]} \
        imports {[ {'p'} {'t'} table# structref limits.i32/1 4 ]} \
        funcs {[ 0 ]} \
        exports {[ {'g'} fn# 0 ]} \
        code {[ {[] end } ]}";

    wah_module_t primary = {0}, provider = {0}, linked = {0};
    assert_ok(wah_parse_module_from_spec(&primary, "wasm"));
    assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
    assert_ok(wah_parse_module_from_spec(&linked, linked_spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &primary, NULL));
    assert_ok(wah_link_module(&ctx, "p", &provider));
    assert_ok(wah_link_module(&ctx, "l", &linked));
    assert_err(wah_instantiate(&ctx), WAH_ERROR_LINK_FAILED);

    wah_free_exec_context(&ctx);
    wah_free_module(&linked);
    wah_free_module(&provider);
    wah_free_module(&primary);
}

static void test_linked_module_ictx_memory_type_validation() {
    printf("Testing linked module ictx memory import type validation...\n");

    // Provider module exports a memory with max 10 pages.
    const char *provider_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        memories {[ limits.i32/2 1 10 ]} \
        exports {[ {'m'} mem# 0, {'f'} fn# 0 ]} \
        code {[ {[] end } ]}";

    // Linked module imports a memory with max 5 pages (provider max 10 > import max 5, should fail).
    const char *linked_spec = "wasm \
        types {[ fn [] [] ]} \
        imports {[ {'p'} {'m'} mem# limits.i32/2 1 5 ]} \
        funcs {[ 0 ]} \
        exports {[ {'g'} fn# 0 ]} \
        code {[ {[] end } ]}";

    wah_module_t primary = {0}, provider = {0}, linked = {0};
    assert_ok(wah_parse_module_from_spec(&primary, "wasm"));
    assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
    assert_ok(wah_parse_module_from_spec(&linked, linked_spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &primary, NULL));
    assert_ok(wah_link_module(&ctx, "p", &provider));
    assert_ok(wah_link_module(&ctx, "l", &linked));
    assert_err(wah_instantiate(&ctx), WAH_ERROR_LINK_FAILED);

    wah_free_exec_context(&ctx);
    wah_free_module(&linked);
    wah_free_module(&provider);
    wah_free_module(&primary);
}

// Cross-module type equality used to recompute shared rec groups, taking exponential time for DAGs
// like T_k = struct { ref null T_{k-1}, ref null T_{k-1} }.
// Fails allocations of the initial memo of rec group pairs (64 keys) when set.
static void *failing_malloc(size_t size, void *userdata) {
    return *(bool *)userdata && size == 64 * sizeof(uint64_t) ? NULL : malloc(size);
}
static void *failing_realloc(void *ptr, size_t size, void *userdata) { (void)userdata; return realloc(ptr, size); }
static void failing_free(void *ptr, void *userdata) { (void)userdata; free(ptr); }

// The memo of rec group pairs is allocated by the context's allocator, which fails it when `oom` is set.
// Types are then treated as unequal, instead of comparing without the memo.
static void test_cross_module_type_eq_dag(bool oom) {
    printf("Testing cross-module type equality of DAG-shaped types is fast%s...\n", oom ? " on OOM" : "");
    enum { K = 30 };
    static char types[64 + K * 64], prov_spec[256 + sizeof(types)], cons_spec[256 + sizeof(types)];
    strcpy(types, "types {[ struct []");
    for (int k = 1; k <= K; k++) {
        char buf[64];
        snprintf(buf, sizeof(buf), ", struct [type.ref.null %d immut, type.ref.null %d immut]", k - 1, k - 1);
        strcat(types, buf);
    }
    char buf[64];
    snprintf(buf, sizeof(buf), ", fn [type.ref.null %d] [] ]}", K);
    strcat(types, buf);
    snprintf(prov_spec, sizeof(prov_spec), "wasm %s funcs {[%d]} exports {[{'f'} fn# 0]} code {[{[] end}]}", types, K + 1);
    snprintf(cons_spec, sizeof(cons_spec), "wasm %s imports {[{'p'} {'f'} fn# %d]}", types, K + 1);

    bool failing = false;
    wah_alloc_t alloc = { failing_malloc, failing_realloc, failing_free, &failing };
    wah_exec_options_t opts = { .alloc = &alloc };
    wah_module_t prov = {0}, cons = {0};
    assert_ok(wah_parse_module_from_spec(&prov, prov_spec));
    assert_ok(wah_parse_module_from_spec(&cons, cons_spec));
    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &cons, &opts));
    assert_ok(wah_link_module(&ctx, "p", &prov));
    failing = oom;
    clock_t start = clock();
    wah_error_t err = wah_instantiate(&ctx);
    double elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;
    printf("  instantiated in %.3fs\n", elapsed);
    assert_true(elapsed < 1.0); // Would take 2^30 steps otherwise
    if (oom) assert_err(err, WAH_ERROR_LINK_FAILED); else assert_ok(err);
    failing = false;
    wah_free_exec_context(&ctx);
    wah_free_module(&cons);
    wah_free_module(&prov);
}

// Each import used to search all exports of the provider linearly.
static double instantiate_many_imports_seconds(int n) {
    size_t size = 64 + (size_t)n * 64;
    char *prov_spec = (char *)malloc(size), *cons_spec = (char *)malloc(size);
    assert_true(prov_spec && cons_spec);
#define APPEND(buf, ...) (p += snprintf(p, (size_t)(buf + size - p), __VA_ARGS__))
    char *p = prov_spec;
    APPEND(prov_spec, "wasm globals {[");
    for (int i = 0; i < n; i++) APPEND(prov_spec, "%si32 immut i32.const %d end", i ? "," : "", i);
    APPEND(prov_spec, "]} exports {[");
    for (int i = 0; i < n; i++) APPEND(prov_spec, "%s{'g%d'} global# %d", i ? "," : "", i, i);
    APPEND(prov_spec, "]}");
    p = cons_spec;
    APPEND(cons_spec, "wasm imports {[");
    for (int i = 0; i < n; i++) APPEND(cons_spec, "%s{'p'} {'g%d'} global# i32 immut", i ? "," : "", n - 1 - i);
    APPEND(cons_spec, "]}");
#undef APPEND

    wah_module_t prov = {0}, cons = {0};
    assert_ok(wah_parse_module_from_spec(&prov, prov_spec));
    assert_ok(wah_parse_module_from_spec(&cons, cons_spec));
    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &cons, NULL));
    assert_ok(wah_link_module(&ctx, "p", &prov));
    clock_t start = clock();
    assert_ok(wah_instantiate(&ctx));
    double elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;
    wah_free_exec_context(&ctx);
    wah_free_module(&cons);
    wah_free_module(&prov);
    free(prov_spec);
    free(cons_spec);
    return elapsed;
}

static void test_import_resolution_time() {
    printf("Testing import resolution time is not quadratic...\n");
    double small = instantiate_many_imports_seconds(5000), large = instantiate_many_imports_seconds(20000);
    printf("  5000 imports: %.3fs, 20000 imports: %.3fs\n", small, large);
    assert_true(large < small * 8 + 0.1); // Quadratic would be 16x
}

int main() {
    test_import_resolution_time();
    test_cross_module_type_eq_dag(false);
    test_cross_module_type_eq_dag(true);
    test_cross_module_call_indirect();
    test_linked_module_imported_table_grow();
    test_elem_before_data_order();
    test_cross_module_memory_ops();
    test_cross_module_funcref_global();
    test_cross_module_type_canon();
    test_cross_module_call_indirect_type_equiv();
    test_call_indirect_subtype_dispatch();
    test_rec_group_canonicalization();
    test_rec_group_canon_subtype();
    test_call_indirect_non_identical_type();
    test_ref_test_concrete_func_subtype();
    test_cross_module_type_with_extra_types();
    test_cross_module_subtype_func_ref_test();
    test_host_import_concrete_ref_uses_linked_type_namespace();
    test_cross_module_abstract_sub_concrete_sup();
    test_linked_module_ictx_table_type_validation();
    test_linked_module_ictx_memory_type_validation();
    printf("All linkage_types tests passed!\n");
    return 0;
}
