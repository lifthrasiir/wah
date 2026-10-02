#include "../wah.h"
#include "wah_impl.h"
#include "common.h"
#include <stdio.h>
#include <string.h>

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
    assert_ok(wah_new_exec_context(&ctx, &good, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);

    wah_free_exec_context(&ctx);
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

static void test_try_table_params_respect_block_floor() {
    printf("Testing try_table params cannot be taken from an outer block...\n");

    // The i32 param of try_table lives outside of the enclosing block, so it must be rejected.
    // Accepting it used to make `br_if 1` compute a negative drop count.
    wah_module_t bad = {0};
    assert_err(wah_parse_module_from_spec(&bad, "wasm \
        types {[ fn [] [i32], fn [i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            i32.const 7 \
            block 0 \
                try_table 1 [] \
                    i32.const 1 \
                    br_if 1 \
                end \
                i32.const 5 \
            end \
            drop \
        end } ]}"), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad);

    // Same shape but with the param inside the block is fine.
    wah_module_t good = {0};
    assert_ok(wah_parse_module_from_spec(&good, "wasm \
        types {[ fn [] [i32], fn [i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            block 0 \
                i32.const 7 \
                try_table 1 [] \
                    i32.const 1 \
                    br_if 1 \
                end \
            end \
        end } ]}"));
    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &good, NULL));
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 7);
    wah_free_exec_context(&ctx);
    wah_free_module(&good);
}

static void test_try_table_body_in_unreachable_code() {
    printf("Testing try_table body in unreachable code is validated normally...\n");

    // The try_table body starts with an empty stack even when the try_table itself is unreachable.
    wah_module_t bad = {0};
    assert_err(wah_parse_module_from_spec(&bad, "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            unreachable \
            try_table void [] \
                i32.add \
                drop \
            end \
        end } ]}"), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad);
}

// A catch targeting an outer label must rewind the operand stack to that label's height,
// not just to the try_table's height.
static void test_catch_to_outer_label_rewinds_stack() {
    printf("Testing catch to an outer label rewinds the operand stack...\n");
    static const char *const bodies[] = {
        // throw in the same frame
        "i32.const 100 block void i32.const 7 try_table void [catch_all 0] throw 0 end drop end end",
        // throw in a callee
        "i32.const 100 block void i32.const 7 try_table void [catch_all 0] call 1 end drop end end",
        // try_table with params
        "i32.const 100 block void i32.const 7 i32.const 8 try_table 2 [catch_all 0] drop throw 0 end drop end end",
        // catch delivering a value to a block result
        "i32.const 100 block 3 i32.const 7 i32.const 8 try_table void [catch 1 0] i32.const 9 throw 1 end drop drop i32.const 0 end drop end",
    };
    for (size_t i = 0; i < sizeof(bodies) / sizeof(*bodies); ++i) {
        char spec[1024];
        snprintf(spec, sizeof(spec), "wasm \
            types {[ fn [] [], fn [] [i32], fn [i32] [], fn [] [i32] ]} \
            funcs {[ 1, 0 ]} \
            tags {[ tag.type# 0, tag.type# 2 ]} \
            code {[ {[] %s }, {[] throw 0 end} ]}", bodies[i]);
        wah_module_t mod = {0};
        assert_ok(wah_parse_module_from_spec(&mod, spec));
        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t result;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
        assert_eq_i32(result.i32, 100);
        wah_free_exec_context(&ctx);
        wah_free_module(&mod);
    }
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
    assert_ok(wah_new_exec_context(&ctx, &module, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 77);

    wah_free_exec_context(&ctx);
    wah_free_module(&module);
}

// The exception handler stack used to be limited to 64 handlers over all frames.
static void test_deep_handlers_in_recursion() {
    printf("Testing more than 64 handlers in recursion...\n");

    // f(n) enters a try_table and recurses until n = 0, which throws to the innermost handler.
    const char *spec = "wasm \
        types {[ fn [i32] [i32], fn [i32] [] ]} \
        funcs {[ 0 ]} \
        tags {[ tag.type# 1 ]} \
        code {[ {[] \
            block void \
                try_table void [catch_all 0] \
                    local.get 0 i32.eqz \
                    if void i32.const 0 throw 0 end \
                    local.get 0 i32.const 1 i32.sub call 0 \
                    return \
                end \
            end \
            i32.const 123 \
        end } ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &module, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t param = { .i32 = 1000 }, result;
    assert_ok(wah_call(&ctx, 0, &param, 1, &result));
    assert_eq_i32(result.i32, 123);

    wah_free_exec_context(&ctx);
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
    assert_ok(wah_new_exec_context(&provider_ctx, &provider, NULL));
    assert_ok(wah_instantiate(&provider_ctx));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &consumer, NULL));
    assert_ok(wah_link_context(&ctx, "provider", &provider_ctx));
    assert_ok(wah_instantiate(&ctx));

    // Consumer's func 1 calls thrower (import), catches the thrown tag 0 value.
    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 55);

    wah_free_exec_context(&ctx);
    wah_free_exec_context(&provider_ctx);
    wah_free_module(&consumer);
    wah_free_module(&provider);
}

static void test_throw_ref_local_use_after_free() {
    printf("Testing throw_ref does not free exnref still held in a local...\n");

    // Regression: throw_ref used to free the original exception before throwing
    // a copy. If the exnref was also stored in a local (via local.tee), the local
    // becomes a dangling pointer. Re-reading the local and using throw_ref on it
    // triggers a use-after-free (detectable by ASan).
    //
    // Structure:
    //   1. throw 0 with i32=42
    //   2. catch_ref catches -> delivers (i32, exnref) to INNER block
    //   3. local.tee 0 saves exnref, throw_ref re-throws (copy)
    //   4. T_MID catches -> delivers i32 to MID block
    //   5. local.get 0 reads the (previously dangling) exnref
    //   6. throw_ref re-throws from the local -> T_OUTER catches
    //   7. Function returns i32 = 42
    const char *spec = "wasm \
        types {[ fn [] [i32], fn [i32] [], fn [] [i32, exnref] ]} \
        funcs {[ 0 ]} \
        tags {[ tag.type# 1 ]} \
        code {[ {[1 exnref] \
            block i32 \
              try_table void [catch 0 0] \
                block i32 \
                  try_table void [catch 0 0] \
                    block 2 \
                      try_table void [catch_ref 0 0] \
                        i32.const 42 \
                        throw 0 \
                      end \
                      i32.const 0 \
                      ref.null exnref \
                    end \
                    local.tee 0 \
                    throw_ref \
                    unreachable \
                  end \
                  i32.const -1 \
                end \
                drop \
                local.get 0 \
                throw_ref \
                unreachable \
              end \
              i32.const -1 \
            end \
        end } ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &module, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);

    wah_free_exec_context(&ctx);
    wah_free_module(&module);
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
    assert_ok(wah_new_exec_context(&ctx, &module, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);

    wah_free_exec_context(&ctx);
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
    assert_ok(wah_new_exec_context(&ctx, &module, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 20);

    wah_free_exec_context(&ctx);
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
    assert_ok(wah_new_exec_context(&ctx, &module, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 55);

    wah_free_exec_context(&ctx);
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
    assert_ok(wah_new_exec_context(&provider_ctx, &provider, NULL));
    assert_ok(wah_instantiate(&provider_ctx));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &consumer, NULL));
    assert_ok(wah_link_context(&ctx, "provider", &provider_ctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);

    wah_free_exec_context(&ctx);
    wah_free_exec_context(&provider_ctx);
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
    assert_ok(wah_new_exec_context(&provider_ctx, &provider, NULL));
    assert_ok(wah_instantiate(&provider_ctx));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &consumer, NULL));
    assert_ok(wah_link_context(&ctx, "provider", &provider_ctx));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, -1);

    wah_free_exec_context(&ctx);
    wah_free_exec_context(&provider_ctx);
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
    assert_ok(wah_new_exec_context(&ctx, &consumer, NULL));
    assert_ok(wah_link_module(&ctx, "provider", &provider));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 77);

    wah_free_exec_context(&ctx);
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
    assert_ok(wah_new_exec_context(&ctx, &consumer, NULL));
    assert_ok(wah_link_module(&ctx, "provider", &provider));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, -1);

    wah_free_exec_context(&ctx);
    wah_free_module(&consumer);
    wah_free_module(&provider);
}

static void test_return_inside_try_table() {
    printf("Testing return inside try_table (exception handler unwinding)...\n");

    // Function with try_table, uses return to exit directly.
    // The exception handler should be unwound by return.
    const char *spec = "wasm \
        types {[ fn [] [i32], fn [i32] [] ]} \
        funcs {[ 0 ]} \
        tags {[ tag.type# 1 ]} \
        code {[ {[] \
            block void \
                try_table void [catch_all 0] \
                    i32.const 42 \
                    return \
                end \
            end \
            i32.const 0 \
        end } ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &module, NULL));
    assert_ok(wah_instantiate(&ctx));
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);
    wah_free_exec_context(&ctx);
    wah_free_module(&module);
}

// Note: return_call_ref/return_call inside try_table are rejected by validation
// (tail calls inside try_table are disallowed per spec). The exception handler
// unwinding for RETURN_CALL_REF (lines 10182-10183) is covered when a callee
// function (called normally from inside try_table) itself does return_call_ref.

static void test_end_inside_try_table() {
    printf("Testing function END with exception handler unwinding...\n");

    // func 0 (callee): called from within try_table; returns normally via END.
    // func 1 (caller): has try_table, calls func 0 which returns normally.
    // When func 0's END executes, the caller's exception handler is on the stack
    // but at a higher call_depth, so it shouldn't be unwound by func 0.
    // Then func 1 ends normally, and its own handler gets unwound.
    const char *spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0, 0 ]} \
        code {[ \
            {[] i32.const 55 end }, \
            {[] \
                block void \
                    try_table i32 [catch_all 0] \
                        call 0 \
                    end \
                    return \
                end \
                i32.const -1 \
            end } \
        ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &module, NULL));
    assert_ok(wah_instantiate(&ctx));
    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 55);
    wah_free_exec_context(&ctx);
    wah_free_module(&module);
}

// Regression: wah_link_module tag import shallow-copies type_index from the
// provider module's type space.  If the provider's tag type sits at a higher
// index than the consumer has types, THROW performs an OOB read on
// consumer->types[].
static void test_link_module_tag_type_index_cross_module() {
    printf("Testing wah_link_module tag type_index cross-module (regression)...\n");

    // Provider has 4 types; the tag uses type index 3 (fn [i32] -> []).
    // The first three types are dummies so the tag index is high.
    const char *provider_spec = "wasm \
        types {[ fn [] [], fn [] [], fn [] [], fn [i32] [] ]} \
        tags {[ tag.type# 3 ]} \
        exports {[ {'tag'} export.tag 0 ]}";

    // Consumer has only 2 types.  Tag import maps to local type index 0
    // (fn [i32] -> []), which is structurally identical to provider's type 3.
    // After the shallow copy bug, the tag instance keeps type_index=3 from
    // the provider, causing an OOB read on consumer->types[3] during THROW.
    const char *consumer_spec = "wasm \
        types {[ fn [i32] [], fn [] [i32] ]} \
        imports {[ {'provider'} {'tag'} tag# tag.type# 0 ]} \
        funcs {[ 1 ]} \
        code {[ {[] \
            block i32 \
                try_table void [catch 0 1] \
                    i32.const 88 \
                    throw 0 \
                end \
                i32.const -1 \
            end \
        end } ]}";

    wah_module_t provider = {0}, consumer = {0};
    assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
    assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &consumer, NULL));
    assert_ok(wah_link_module(&ctx, "provider", &provider));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 88);

    wah_free_exec_context(&ctx);
    wah_free_module(&consumer);
    wah_free_module(&provider);
}

static void test_linked_module_imported_tag_identity() {
    printf("Testing linked module imported tag identity is resolved (regression)...\n");

    // Provider exports two tags of the same type but distinct identity:
    //   tag 0: (i32) -> ()  exported as "a"
    //   tag 1: (i32) -> ()  exported as "b"
    // Linked module imports both, throws with tag 0 but try_table catches
    // only tag 1. Before the fix, both imported tags had NULL identity, so
    // NULL == NULL made them match incorrectly (type confusion / tag confusion).
    const char *provider_spec = "wasm \
        types {[ fn [i32] [] ]} \
        tags {[ tag.type# 0, tag.type# 0 ]} \
        exports {[ {'a'} export.tag 0, {'b'} export.tag 1 ]}";

    // Linked module:
    //   import tag 0 = provider.a
    //   import tag 1 = provider.b
    //   func 0: () -> (i32)
    //     try_table catches tag 1 (should NOT match throw of tag 0)
    //       throw tag 0 with i32.const 42
    //     catch_all returns -1
    const char *linked_spec = "wasm \
        types {[ fn [i32] [], fn [] [i32] ]} \
        imports {[ \
            {'provider'} {'a'} tag# tag.type# 0, \
            {'provider'} {'b'} tag# tag.type# 0 \
        ]} \
        funcs {[ 1 ]} \
        exports {[ {'test'} fn# 0 ]} \
        code {[ {[] \
            block i32 \
                block void \
                    try_table void [catch 1 1, catch_all 0] \
                        i32.const 42 \
                        throw 0 \
                    end \
                end \
                i32.const -1 \
                br 0 \
            end \
        end } ]}";

    // Primary module: imports linked.test, calls it
    const char *primary_spec = "wasm \
        types {[ fn [] [i32] ]} \
        imports {[ {'linked'} {'test'} fn# 0 ]} \
        funcs {[ 0 ]} \
        exports {[ {'main'} fn# 1 ]} \
        code {[ {[] call 0 end } ]}";

    wah_module_t provider = {0}, linked = {0}, primary = {0};
    assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
    assert_ok(wah_parse_module_from_spec(&linked, linked_spec));
    assert_ok(wah_parse_module_from_spec(&primary, primary_spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &primary, NULL));
    assert_ok(wah_link_module(&ctx, "provider", &provider));
    assert_ok(wah_link_module(&ctx, "linked", &linked));
    assert_ok(wah_instantiate(&ctx));

    // Tag 0 and tag 1 have different identities, so catch 1 should NOT
    // match throw 0. The exception propagates to catch_all -> returns -1.
    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, -1);

    wah_free_exec_context(&ctx);
    wah_free_module(&primary);
    wah_free_module(&linked);
    wah_free_module(&provider);
}

static void test_link_module_tag_context_has_gc() {
    printf("Testing wah_link_module tag context has GC state...\n");

    wah_module_t provider = {0};
    wah_module_t primary = {0};
    assert_ok(wah_parse_module_from_spec(&provider, "wasm \
        types {[ fn [] [] ]} \
        tags {[ tag.type# 0 ]}"));
    assert_ok(wah_parse_module_from_spec(&primary, "wasm"));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &primary, NULL));
    assert_ok(wah_link_module(&ctx, "provider", &provider));
    assert_ok(wah_instantiate(&ctx));

    const wah_exec_context_t *linked_ctx = wah_debug_linked_ctx(&ctx, 0);
    assert_not_null(linked_ctx);
    wah_gc_heap_stats_t stats = {0};
    wah_gc_heap_stats(linked_ctx, &stats);
    assert_true(stats.allocation_threshold > 0);

    wah_free_exec_context(&ctx);
    wah_free_module(&primary);
    wah_free_module(&provider);
}

// Tag imports copied slots in link order, so a re-exported import of a later module (or of the primary) was copied
// before it was resolved, and NULL identities matched each other.
static void test_reexported_tag_identity_in_link_order() {
    printf("Testing re-exported tag identities do not depend on the link order...\n");

    const char *def_spec = "wasm \
        types {[ fn [i32] [] ]} \
        tags {[ tag.type# 0, tag.type# 0 ]} \
        exports {[ {'a'} export.tag 0, {'b'} export.tag 1 ]}";
    const char *mid_spec = "wasm \
        types {[ fn [i32] [] ]} \
        imports {[ {'def'} {'a'} tag# tag.type# 0, {'def'} {'b'} tag# tag.type# 0 ]} \
        exports {[ {'a'} export.tag 0, {'b'} export.tag 1 ]}";
    const char *top_spec = "wasm \
        types {[ fn [i32] [] ]} \
        imports {[ {'mid'} {'a'} tag# tag.type# 0, {'mid'} {'b'} tag# tag.type# 0 ]} \
        exports {[ {'a'} export.tag 0, {'b'} export.tag 1 ]}";
    // Throws b and catches a, which should not match
    const char *primary_spec = "wasm \
        types {[ fn [i32] [], fn [] [i32] ]} \
        imports {[ {'top'} {'a'} tag# tag.type# 0, {'top'} {'b'} tag# tag.type# 0 ]} \
        funcs {[ 1 ]} \
        code {[ {[] \
            block i32 \
                try_table void [catch 0 0] i32.const 42 throw 1 end \
                i32.const -1 \
            end \
        end } ]}";

    wah_module_t def = {0}, mid = {0}, top = {0}, primary = {0};
    assert_ok(wah_parse_module_from_spec(&def, def_spec));
    assert_ok(wah_parse_module_from_spec(&mid, mid_spec));
    assert_ok(wah_parse_module_from_spec(&top, top_spec));
    assert_ok(wah_parse_module_from_spec(&primary, primary_spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &primary, NULL));
    assert_ok(wah_link_module(&ctx, "top", &top));
    assert_ok(wah_link_module(&ctx, "mid", &mid));
    assert_ok(wah_link_module(&ctx, "def", &def));
    assert_ok(wah_instantiate(&ctx));
    wah_value_t result;
    assert_err(wah_call(&ctx, 0, NULL, 0, &result), WAH_ERROR_EXCEPTION);

    wah_free_exec_context(&ctx);
    wah_free_module(&primary);
    wah_free_module(&top);
    wah_free_module(&mid);
    wah_free_module(&def);
}

static void test_try_table_handler_overflow() {
    printf("Testing try_table exception handler overflow...\n");

    #define REP8(x) x x x x x x x x
    #define REP65(x) REP8(REP8(x)) x
    char *spec = "wasm \
        types {[fn [] []]} \
        funcs {[0]} \
        code {[{[] " REP65("try_table void [] ") REP65("end ") "end}]}";

    wah_module_t mod = {0};
    assert_ok(wah_parse_module_from_spec(&mod, spec));

    // Handlers are limited by the stack budget, which is too small for 65 handlers here
    for (int small_stack = 0; small_stack <= 1; small_stack++) {
        wah_exec_options_t opts = { .limits = { .max_stack_bytes = small_stack ? 1024 : 0 } };
        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &mod, &opts));
        assert_ok(wah_instantiate(&ctx));

        wah_value_t result;
        assert_err(wah_call(&ctx, 0, NULL, 0, &result), small_stack ? WAH_ERROR_STACK_OVERFLOW : WAH_OK);

        wah_free_exec_context(&ctx);
    }
    wah_free_module(&mod);
}

// Runs `body` 100 times in a loop; leaving a try_table through a branch must drop its handler.
// With `fuel` > 0, fuel is refilled in small steps so that slow-path islands get executed as well.
static void check_try_table_branch_out(const char *body, int64_t fuel) {
    wah_parse_options_t opts = { .enable_fuel_metering = fuel > 0 };
    wah_module_t mod = {0};
    assert_ok(wah_parse_module_from_spec_ex(&mod, &opts, "wasm \
        types {[ fn [] [i32], fn [] [] ]} funcs {[0]} tags {[ tag.type# 1 ]} \
        code {[{[1 i32] \
            loop void %t \
                local.get 0 i32.const 1 i32.add local.tee 0 i32.const 100 i32.lt_u br_if 0 \
            end \
            local.get 0 \
        end}]}", body));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result = {0};
    if (fuel > 0) {
        assert_ok(wah_set_fuel(&ctx, fuel));
        assert_ok(wah_start(&ctx, 0, NULL, 0));
        wah_error_t err;
        while ((err = wah_resume(&ctx)) == WAH_STATUS_FUEL_EXHAUSTED) assert_ok(wah_set_fuel(&ctx, fuel));
        assert_ok(err);
        assert_ok(wah_finish(&ctx, &result, 1, NULL));
    } else {
        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    }
    assert_eq_i32(result.i32, 100);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_try_table_branch_out_drops_handler() {
    printf("Testing branches out of try_table drop its handler...\n");
    static const char *const bodies[] = {
        "block void try_table void [catch_all 0] br 1 end end",
        "block void try_table void [catch_all 0] i32.const 1 br_if 1 end end",
        "block void try_table void [catch_all 0] i32.const 0 br_table [1] 1 end end",
        "try_table void [] br 0 end",
        "try_table void [] local.get 0 i32.const 1 i32.add local.tee 0 i32.const 99 i32.lt_u br_if 1 end", // back to the loop header
        // catch_all to labels outside of another try_table
        "block void try_table void [] try_table void [catch_all 1] throw 0 end end end",
        ("block void try_table void [] try_table void [catch_all 2] "
         "local.get 0 i32.const 1 i32.add local.tee 0 i32.const 99 i32.lt_u if void throw 0 end end end end"),
    };
    for (size_t i = 0; i < sizeof(bodies) / sizeof(*bodies); i++) {
        check_try_table_branch_out(bodies[i], 0);
        check_try_table_branch_out(bodies[i], 7);
    }
}

static void test_try_table_stale_handler_does_not_catch() {
    printf("Testing a throw after leaving try_table is not caught by it...\n");

    wah_module_t mod = {0};
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[ fn [] [i32], fn [] [] ]} funcs {[0]} tags {[ tag.type# 1 ]} \
        code {[{[] \
            block void \
                block void \
                    try_table void [catch_all 1] br 1 end \
                end \
                throw 0 \
            end \
            i32.const 7 \
        end}]}"));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_err(wah_call(&ctx, 0, NULL, 0, &result), WAH_ERROR_EXCEPTION);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

// Phase 0: Exception survives GC cycle while reachable from operand stack (via catch_ref).
static void test_exception_survives_gc_on_stack() {
    printf("Testing exception survives GC cycle on operand stack...\n");

    // catch_ref puts exnref on the stack. throw_ref re-throws it.
    // Outer catch receives the original value intact.
    const char *spec = "wasm \
        types {[ fn [] [i32], fn [i32] [], fn [] [i32, exnref] ]} \
        funcs {[ 0 ]} \
        tags {[ tag.type# 1 ]} \
        code {[ {[] \
            block i32 \
                try_table void [catch 0 0] \
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

    wah_module_t mod = {0};
    assert_ok(wah_parse_module_from_spec(&mod, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

// Phase 0: OOM during exception allocation returns error without crash.
static void test_exception_oom() {
    printf("Testing OOM during exception allocation...\n");

    // A module that throws. We run it under the OOM test infra in test_oom.c,
    // but here we just verify throw+catch works at the basic level after
    // GC-managed exceptions -- the OOM behavior is exercised by test_oom.c.
    const char *spec = "wasm \
        types {[ fn [] [i32], fn [i32] [] ]} \
        funcs {[ 0 ]} \
        tags {[ tag.type# 1 ]} \
        code {[ {[] \
            block i32 \
                try_table void [catch 0 0] \
                    i32.const 99 \
                    throw 0 \
                end \
                i32.const -1 \
            end \
        end } ]}";

    wah_module_t mod = {0};
    assert_ok(wah_parse_module_from_spec(&mod, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 99);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

// The payload of a catch to the function label was not counted in max_stack_depth when the body ends unreachable,
// so rethrowing a wide exception there wrote past the frame (and here past the stack buffer).
static void test_ref_test_exn() {
    printf("Testing ref.test and ref.cast on exnref...\n");

    const char *spec = "wasm \
        types {[ fn [] [], fn [] [i32] ]} \
        funcs {[ 1, 1 ]} \
        tags {[ tag.type# 0 ]} \
        code {[ \
            {[] \
                block exnref \
                    try_table void [catch_all_ref 0] throw 0 end \
                    unreachable \
                end \
                ref.test exnref \
            end }, \
            {[] \
                block exnref \
                    try_table void [catch_all_ref 0] throw 0 end \
                    unreachable \
                end \
                ref.cast exnref drop i32.const 1 \
            end } \
        ]}";

    wah_module_t mod = {0};
    assert_ok(wah_parse_module_from_spec(&mod, spec));
    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 1);
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 1);
    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_catch_to_function_label_counts_payload() {
    printf("Testing catch to the function label counts the payload in the stack depth...\n");

    char i32s[64 * 4], consts[64 * 16];
    i32s[0] = consts[0] = '\0';
    for (int i = 0; i < 64; i++) {
        strcat(i32s, i ? ",i32" : "i32");
        char buf[16];
        snprintf(buf, sizeof buf, " i32.const %d", i);
        strcat(consts, buf);
    }
    char spec[4096];
    snprintf(spec, sizeof spec, "wasm \
        types {[ fn [%s] [], fn [] [], fn [] [%s] ]} \
        funcs {[ 1, 2 ]} \
        tags {[ tag.type# 0 ]} \
        globals {[ exnref mut ref.null exnref end ]} \
        code {[ \
            {[] \
                block exnref \
                    try_table void [catch_all_ref 0] %s throw 0 end \
                    unreachable \
                end \
                global.set 0 \
            end }, \
            {[] \
                try_table void [catch 0 0] global.get 0 throw_ref end \
                unreachable \
            end } \
        ]}", i32s, i32s, consts);

    wah_module_t mod = {0};
    assert_ok(wah_parse_module_from_spec(&mod, spec));
    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));
    assert_ok(wah_call(&ctx, 0, NULL, 0, NULL));

    wah_limits_t lim, small;
    wah_get_limits(&ctx, &lim);
    small = lim;
    small.max_stack_bytes = 512; // Fits the frame but not the payload
    assert_ok(wah_set_limits(&ctx, &small));
    wah_value_t results[64];
    uint32_t actual = 0;
    assert_err(wah_call_multi(&ctx, 1, NULL, 0, results, 64, &actual), WAH_ERROR_STACK_OVERFLOW);

    assert_ok(wah_set_limits(&ctx, &lim));
    assert_ok(wah_call_multi(&ctx, 1, NULL, 0, results, 64, &actual));
    assert_eq_u32(actual, 64);
    for (int i = 0; i < 64; i++) assert_eq_i32(results[i].i32, i);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_cancel_does_not_free_exnref_in_global() {
    printf("Testing wah_cancel_internal does not free exnref held in a global...\n");

    // Regression: wah_cancel_internal used to walk all GC objects and free every
    // exception unconditionally. If an exnref was stored in a global via catch_ref
    // + global.set, a subsequent failed call (which invokes wah_cancel_internal)
    // freed it, leaving a dangling pointer. Reading the global afterwards is UAF.
    //
    // func 0 (store_exn): throw 0 with 42, catch_ref, global.set 0, return i32
    // func 1 (do_trap): unreachable
    // func 2 (read_exn): global.get 0, throw_ref inside try_table, catch, return i32
    const char *spec = "wasm \
        types {[ fn [] [i32], fn [i32] [], fn [] [i32, exnref], fn [] [] ]} \
        funcs {[ 0, 3, 0 ]} \
        tags {[ tag.type# 1 ]} \
        globals {[ exnref mut ref.null exnref end ]} \
        exports {[ {'store_exn'} fn# 0, {'do_trap'} fn# 1, {'read_exn'} fn# 2 ]} \
        code {[ \
            {[] \
                block 2 \
                    try_table void [catch_ref 0 0] \
                        i32.const 42 \
                        throw 0 \
                    end \
                    i32.const 0 \
                    ref.null exnref \
                end \
                global.set 0 \
            end }, \
            {[] unreachable end }, \
            {[] \
                block i32 \
                    try_table void [catch 0 0] \
                        global.get 0 \
                        throw_ref \
                    end \
                    i32.const -1 \
                end \
            end } \
        ]}";

    wah_module_t mod = {0};
    assert_ok(wah_parse_module_from_spec(&mod, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    // Step 1: store exception in global
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);

    // Step 2: trap -- triggers wah_cancel_internal
    wah_error_t err = wah_call(&ctx, 1, NULL, 0, &result);
    assert(err == WAH_ERROR_TRAP);

    // Step 3: read exnref from global and re-throw -- UAF if cancel freed it
    assert_ok(wah_call(&ctx, 2, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

int main() {
    test_try_table_catch_label_types();
    test_try_table_params_respect_block_floor();
    test_try_table_body_in_unreachable_code();
    test_catch_to_outer_label_rewinds_stack();
    test_catch_all();
    test_deep_handlers_in_recursion();
    test_cross_module_throw_tag_context();
    test_throw_ref_local_use_after_free();
    test_catch_ref_and_throw_ref();
    test_nested_try_table();
    test_try_table_no_catch_propagates();
    test_cross_module_exception_tag_identity();
    test_exception_tag_mismatch_no_catch();
    test_link_module_tag_identity();
    test_link_module_tag_mismatch();
    test_return_inside_try_table();
    test_end_inside_try_table();
    test_link_module_tag_type_index_cross_module();
    test_link_module_tag_context_has_gc();
    test_linked_module_imported_tag_identity();
    test_reexported_tag_identity_in_link_order();
    test_try_table_handler_overflow();
    test_try_table_branch_out_drops_handler();
    test_try_table_stale_handler_does_not_catch();
    test_exception_survives_gc_on_stack();
    test_exception_oom();
    test_cancel_does_not_free_exnref_in_global();
    test_catch_to_function_label_counts_payload();
    test_ref_test_exn();
    printf("All exception tests passed!\n");
    return 0;
}
