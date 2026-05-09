#include "../wah.h"
#include "common.h"
#include "wah_impl.h"
#include <stdio.h>

// Regression test for OOB read in wah_ref_test_heap_type (lines 10532-10533).
// When a GC object allocated by module A (with struct types, high repr_id)
// is tested with ref.test (ref struct) or ref.test (ref array) in module B
// (with no struct/array types, repr_count=0), the code accesses
// repr_infos[repr_id] without bounds checking.

static void test_cross_module_ref_test_abstract_struct_oob() {
    printf("Testing cross-module ref.test abstract struct OOB (security regression)...\n");

    // Provider: defines 4 struct types to get repr_id >= 3, then exports a
    // function that creates a struct of the last type and returns it as anyref.
    const char *provider_spec = "wasm \
        types {[ struct [i32 mut], struct [i32 mut, i32 mut], \
                 struct [i32 mut, i32 mut, i32 mut], \
                 struct [i32 mut, i32 mut, i32 mut, i32 mut], \
                 fn [] [anyref] ]} \
        funcs {[ 4 ]} \
        exports {[ {'make'} fn# 0 ]} \
        code {[ {[] struct.new_default 3 end } ]}";

    // Consumer: has NO struct types at all (repr_count will be 0).
    // It imports make() and calls ref.test with abstract struct heap type.
    const char *consumer_spec = "wasm \
        types {[ fn [] [anyref], fn [] [i32] ]} \
        imports {[ {'p'} {'make'} fn# 0 ]} \
        funcs {[ 1 ]} \
        code {[ {[] call 0 ref.test structref end } ]}";

    wah_module_t provider = {0}, consumer = {0};
    assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
    assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

    // Use wah_link_context so the provider gets its own exec context with
    // correct module pointer for cross-module calls.
    wah_exec_context_t provider_ctx = {0};
    assert_ok(wah_new_exec_context(&provider_ctx, &provider, NULL));
    assert_ok(wah_gc_start(&provider_ctx));
    assert_ok(wah_instantiate(&provider_ctx));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &consumer, NULL));
    assert_ok(wah_link_context(&ctx, "p", &provider_ctx));
    assert_ok(wah_instantiate(&ctx));

    // This should return 1 (the object IS a struct) without crashing.
    // Before the fix, this triggers an OOB read on consumer's repr_infos[].
    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 1);

    wah_free_exec_context(&ctx);
    wah_free_exec_context(&provider_ctx);
    wah_free_module(&consumer);
    wah_free_module(&provider);
}

static void test_cross_module_ref_test_abstract_array_oob() {
    printf("Testing cross-module ref.test abstract array OOB (security regression)...\n");

    // Provider: defines an array type and exports a factory returning anyref.
    const char *provider_spec = "wasm \
        types {[ array i32 mut, fn [] [anyref] ]} \
        funcs {[ 1 ]} \
        exports {[ {'make'} fn# 0 ]} \
        code {[ {[] i32.const 0 i32.const 5 array.new 0 end } ]}";

    // Consumer: has NO array/struct types. Tests ref.test with abstract array.
    // 0x6a is the heap type byte for 'array'.
    const char *consumer_spec = "wasm \
        types {[ fn [] [anyref], fn [] [i32] ]} \
        imports {[ {'p'} {'make'} fn# 0 ]} \
        funcs {[ 1 ]} \
        code {[ {[] call 0 ref.test %'6a' end } ]}";

    wah_module_t provider = {0}, consumer = {0};
    assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
    assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

    wah_exec_context_t provider_ctx = {0};
    assert_ok(wah_new_exec_context(&provider_ctx, &provider, NULL));
    assert_ok(wah_gc_start(&provider_ctx));
    assert_ok(wah_instantiate(&provider_ctx));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &consumer, NULL));
    assert_ok(wah_link_context(&ctx, "p", &provider_ctx));
    assert_ok(wah_instantiate(&ctx));

    // Should return 1 (it IS an array) without OOB read.
    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 1);

    wah_free_exec_context(&ctx);
    wah_free_exec_context(&provider_ctx);
    wah_free_module(&consumer);
    wah_free_module(&provider);
}

// Regression test for wah_bind_frame_module falling back to the consumer's
// exec context when the linked module has no internal ctx (no tags).
// fctx->module then points to the wrong module, causing OOB accesses on
// typeidx_to_repr / repr_infos / type_defs when executing provider code.
static void test_link_module_frame_ctx_wrong_module() {
    printf("Testing wah_link_module frame_ctx uses correct module (security regression)...\n");

    // Provider: struct type + factory function returning anyref.
    // type 0 = struct [i32 mut]
    // type 1 = fn [] -> [anyref]
    const char *provider_spec = "wasm \
        types {[ struct [i32 mut], fn [] [anyref] ]} \
        funcs {[ 1 ]} \
        exports {[ {'make'} fn# 0 ]} \
        code {[ {[] struct.new_default 0 end } ]}";

    // Consumer: NO struct types — different type layout from provider.
    // type 0 = fn [] -> [anyref]  (import signature)
    // type 1 = fn [] -> [i32]     (local function)
    // Provider's struct.new_default uses typeidx 0, but consumer's type 0 is
    // a func type. If fctx->module wrongly points to consumer, the typeidx
    // lookup hits the wrong type table.
    const char *consumer_spec = "wasm \
        types {[ fn [] [anyref], fn [] [i32] ]} \
        imports {[ {'p'} {'make'} fn# 0 ]} \
        funcs {[ 1 ]} \
        code {[ {[] call 0 ref.is_null end } ]}";

    wah_module_t provider = {0}, consumer = {0};
    assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
    assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &consumer, NULL));
    assert_ok(wah_link_module(&ctx, "p", &provider));
    assert_ok(wah_gc_start(&ctx));
    assert_ok(wah_instantiate(&ctx));

    // Should return 0 (non-null struct ref → ref.is_null = 0).
    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 0);

    wah_free_exec_context(&ctx);
    wah_free_module(&consumer);
    wah_free_module(&provider);
}

int main() {
    test_cross_module_ref_test_abstract_struct_oob();
    test_cross_module_ref_test_abstract_array_oob();
    test_link_module_frame_ctx_wrong_module();
    printf("All cross-module reference security tests passed!\n");
    return 0;
}
