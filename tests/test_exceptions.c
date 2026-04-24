#define WAH_IMPLEMENTATION
#include "../wah.h"
#include "common.c"
#include <stdio.h>

// 27d668b: Validate try_table catch handler types against target label types.
static void test_try_table_catch_label_types() {
    printf("Testing try_table catch handler label types (27d668b)...\n");

    // Positive: tag with (i32) params, catch handler targets a block (void -> i32).
    // try_table void, 1 catch: {kind=0(catch), tag=0, label=1(outer block)}
    // Encoding: try_table(0x1F) void(0x40) count(0x01) kind(0x00) tag(0x00) label(0x01)
    const char *good_spec = "wasm \
        types {[ fn [] [i32], fn [i32] [] ]} \
        funcs {[ 0 ]} \
        tags {[ tag.type# 1 ]} \
        code {[ {[] \
            block i32 \
                try_table void [catch 0 1] \
                    i32.const 42 \
                    throw 0 \
                end \
                i32.const 99 \
            end \
        end } ]}";

    wah_module_t good = {0};
    assert_ok(wah_parse_module_from_spec(&good, good_spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &good));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&good);

    // Negative: tag with (i32) params, but catch handler targets block (void -> i64).
    // The i32 from the tag doesn't match i64.
    const char *bad_spec = "wasm \
        types {[ fn [] [i64], fn [i32] [] ]} \
        funcs {[ 0 ]} \
        tags {[ tag.type# 1 ]} \
        code {[ {[] \
            block i64 \
                try_table void [catch 0 1] \
                    i32.const 42 \
                    throw 0 \
                end \
                i64.const 99 \
            end \
        end } ]}";

    wah_module_t bad = {0};
    assert_err(wah_parse_module_from_spec(&bad, bad_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad);
}

// 20f1b66: Add support for exceptions: throw[_ref], try_table.
// Runtime test for catch_all: same structure as the working catch test but with catch_all.
static void test_catch_all() {
    printf("Testing catch_all (20f1b66)...\n");

    // catch_all (kind=2) delivers no values.
    // Label indices in catch are relative to outside the try_table (+1 adjusted internally).
    // Label 0 from catch -> resolves to enclosing block (block void).
    // After catch branches to block void, we push i32.const 77 and return.
    const char *spec = "wasm \
        types {[ fn [] [i32], fn [i32] [] ]} \
        funcs {[ 0 ]} \
        tags {[ tag.type# 1 ]} \
        code {[ {[] \
            block void \
                try_table void [catch_all 0] \
                    i32.const 42 \
                    throw 0 \
                end \
            end \
            i32.const 77 \
        end } ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 77);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// 084715d: Fix cross-module throw using wrong tag instance context.
static void test_cross_module_throw_tag_context() {
    printf("Testing cross-module throw tag context (084715d)...\n");

    // Provider: defines tag 0 (fn [i32] -> []) and exports a function that throws it,
    // plus exports the tag itself.
    const char *provider_spec = "wasm \
        types {[ fn [i32] [], fn [] [] ]} \
        funcs {[ 1 ]} \
        tags {[ tag.type# 0 ]} \
        exports {[ {'thrower'} fn# 0, {'tag'} export.tag 0 ]} \
        code {[ {[] i32.const 55 throw 0 end } ]}";

    // Consumer: imports the provider's tag and throwing function.
    // Also has its OWN local tag (different type: fn [i64] -> []) at tag index 1.
    // The imported tag is at tag index 0.
    // If throw uses the wrong tag table (ctx instead of fctx),
    // it would pick up the consumer's tag layout, causing mismatch.
    const char *consumer_spec = "wasm \
        types {[ fn [i32] [], fn [] [], fn [] [i32], fn [i64] [] ]} \
        imports {[ {'provider'} {'thrower'} fn# 1, {'provider'} {'tag'} tag# tag.type# 0 ]} \
        funcs {[ 2 ]} \
        tags {[ tag.type# 3 ]} \
        code {[ {[] \
            block i32 \
                try_table void [catch 0 1] \
                    call 0 \
                end \
                i32.const 0 \
            end \
        end } ]}";

    wah_module_t provider = {0}, consumer = {0};
    assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
    assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

    // Tag imports require a linked context (not just module), because tag instances
    // are created at instantiation time.
    wah_exec_context_t provider_ctx = {0};
    assert_ok(wah_exec_context_create(&provider_ctx, &provider));
    assert_ok(wah_instantiate(&provider_ctx));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &consumer));
    assert_ok(wah_link_context(&ctx, "provider", &provider_ctx));
    assert_ok(wah_instantiate(&ctx));

    // Consumer's func 1 calls thrower (import), catches the thrown tag 0 value.
    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 55);

    wah_exec_context_destroy(&ctx);
    wah_exec_context_destroy(&provider_ctx);
    wah_free_module(&consumer);
    wah_free_module(&provider);
}

static void test_catch_ref_and_throw_ref() {
    printf("Testing catch_ref and throw_ref...\n");

    const char *spec = "wasm \
        types {[ fn [] [i32], fn [i32] [], fn [] [i32, exnref] ]} \
        funcs {[ 0 ]} \
        tags {[ tag.type# 1 ]} \
        code {[ {[] \
            block i32 \
                try_table void [catch 0 1] \
                    block 2 \
                        try_table void [catch_ref 0 0] \
                            i32.const 42 \
                            throw 0 \
                        end \
                        i32.const 0 \
                        ref.null exnref \
                    end \
                    throw_ref \
                    unreachable \
                end \
                i32.const -1 \
            end \
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
}

static void test_nested_try_table() {
    printf("Testing nested try_table...\n");

    const char *spec = "wasm \
        types {[ fn [] [i32], fn [i32] [] ]} \
        funcs {[ 0 ]} \
        tags {[ tag.type# 1 ]} \
        code {[ {[] \
            block i32 \
                try_table void [catch 0 1] \
                    block void \
                        try_table void [catch_all 0] \
                            i32.const 10 \
                            throw 0 \
                        end \
                    end \
                    i32.const 20 \
                    throw 0 \
                end \
                i32.const 0 \
            end \
        end } ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 20);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_try_table_no_catch_propagates() {
    printf("Testing try_table uncaught exception propagates...\n");

    const char *spec = "wasm \
        types {[ fn [] [i32], fn [i32] [] ]} \
        funcs {[ 0 ]} \
        tags {[ tag.type# 1, tag.type# 1 ]} \
        code {[ {[] \
            block i32 \
                try_table void [catch 0 0] \
                    try_table void [catch 1 1] \
                        i32.const 55 \
                        throw 0 \
                    end \
                end \
                i32.const -1 \
            end \
        end } ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 55);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

static void test_cross_module_exception_tag_identity() {
    printf("Testing cross-module exception tag identity...\n");

    const char *provider_spec = "wasm \
        types {[ fn [i32] [], fn [] [] ]} \
        funcs {[ 1 ]} \
        tags {[ tag.type# 0 ]} \
        exports {[ {'thrower'} fn# 0, {'tag'} export.tag 0 ]} \
        code {[ {[] i32.const 42 throw 0 end } ]}";

    const char *consumer_spec = "wasm \
        types {[ fn [i32] [], fn [] [], fn [] [i32] ]} \
        imports {[ \
            {'provider'} {'thrower'} fn# 1, \
            {'provider'} {'tag'} tag# tag.type# 0 \
        ]} \
        funcs {[ 2 ]} \
        code {[ {[] \
            block i32 \
                try_table void [catch 0 1] \
                    call 0 \
                end \
                i32.const -1 \
            end \
        end } ]}";

    wah_module_t provider = {0}, consumer = {0};
    assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
    assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

    wah_exec_context_t provider_ctx = {0};
    assert_ok(wah_exec_context_create(&provider_ctx, &provider));
    assert_ok(wah_instantiate(&provider_ctx));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &consumer));
    assert_ok(wah_link_context(&ctx, "provider", &provider_ctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);

    wah_exec_context_destroy(&ctx);
    wah_exec_context_destroy(&provider_ctx);
    wah_free_module(&consumer);
    wah_free_module(&provider);
}

static void test_exception_tag_mismatch_no_catch() {
    printf("Testing exception tag mismatch does not catch...\n");

    const char *provider_spec = "wasm \
        types {[ fn [i32] [], fn [] [] ]} \
        funcs {[ 1 ]} \
        tags {[ tag.type# 0 ]} \
        exports {[ {'thrower'} fn# 0 ]} \
        code {[ {[] i32.const 42 throw 0 end } ]}";

    const char *consumer_spec = "wasm \
        types {[ fn [i32] [], fn [] [], fn [] [i32] ]} \
        imports {[ {'provider'} {'thrower'} fn# 1 ]} \
        funcs {[ 2 ]} \
        tags {[ tag.type# 0 ]} \
        code {[ {[] \
            block i32 \
                block void \
                    try_table void [catch 0 1, catch_all 0] \
                        call 0 \
                    end \
                end \
                i32.const -1 \
                br 0 \
            end \
        end } ]}";

    wah_module_t provider = {0}, consumer = {0};
    assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
    assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

    wah_exec_context_t provider_ctx = {0};
    assert_ok(wah_exec_context_create(&provider_ctx, &provider));
    assert_ok(wah_instantiate(&provider_ctx));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &consumer));
    assert_ok(wah_link_context(&ctx, "provider", &provider_ctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, -1);

    wah_exec_context_destroy(&ctx);
    wah_exec_context_destroy(&provider_ctx);
    wah_free_module(&consumer);
    wah_free_module(&provider);
}

static void test_link_module_tag_identity() {
    printf("Testing wah_link_module tag instance identity...\n");

    // Provider: defines tag 0 (fn [i32] -> []) and exports a throwing function + the tag.
    // Consumer: imports both via wah_link_module (not wah_link_context).
    // Before the fix, wah_link_module-linked modules shared the primary context's
    // tag_instances, breaking tag identity matching.
    const char *provider_spec = "wasm \
        types {[ fn [i32] [], fn [] [] ]} \
        funcs {[ 1 ]} \
        tags {[ tag.type# 0 ]} \
        exports {[ {'thrower'} fn# 0, {'tag'} export.tag 0 ]} \
        code {[ {[] i32.const 77 throw 0 end } ]}";

    const char *consumer_spec = "wasm \
        types {[ fn [i32] [], fn [] [], fn [] [i32] ]} \
        imports {[ \
            {'provider'} {'thrower'} fn# 1, \
            {'provider'} {'tag'} tag# tag.type# 0 \
        ]} \
        funcs {[ 2 ]} \
        code {[ {[] \
            block i32 \
                try_table void [catch 0 1] \
                    call 0 \
                end \
                i32.const -1 \
            end \
        end } ]}";

    wah_module_t provider = {0}, consumer = {0};
    assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
    assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &consumer));
    assert_ok(wah_link_module(&ctx, "provider", &provider));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 77);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&consumer);
    wah_free_module(&provider);
}

static void test_link_module_tag_mismatch() {
    printf("Testing wah_link_module tag mismatch does not catch...\n");

    // Provider throws with its own local tag. Consumer tries to catch with a
    // different local tag of the same shape. Should NOT match.
    const char *provider_spec = "wasm \
        types {[ fn [i32] [], fn [] [] ]} \
        funcs {[ 1 ]} \
        tags {[ tag.type# 0 ]} \
        exports {[ {'thrower'} fn# 0 ]} \
        code {[ {[] i32.const 42 throw 0 end } ]}";

    const char *consumer_spec = "wasm \
        types {[ fn [i32] [], fn [] [], fn [] [i32] ]} \
        imports {[ {'provider'} {'thrower'} fn# 1 ]} \
        funcs {[ 2 ]} \
        tags {[ tag.type# 0 ]} \
        code {[ {[] \
            block i32 \
                block void \
                    try_table void [catch 0 1, catch_all 0] \
                        call 0 \
                    end \
                end \
                i32.const -1 \
                br 0 \
            end \
        end } ]}";

    wah_module_t provider = {0}, consumer = {0};
    assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
    assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &consumer));
    assert_ok(wah_link_module(&ctx, "provider", &provider));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, -1);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&consumer);
    wah_free_module(&provider);
}

int main() {
    test_try_table_catch_label_types();
    test_catch_all();
    test_cross_module_throw_tag_context();
    test_catch_ref_and_throw_ref();
    test_nested_try_table();
    test_try_table_no_catch_propagates();
    test_cross_module_exception_tag_identity();
    test_exception_tag_mismatch_no_catch();
    test_link_module_tag_identity();
    test_link_module_tag_mismatch();
    printf("All exception tests passed!\n");
    return 0;
}
