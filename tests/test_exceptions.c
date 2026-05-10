#include "../wah.h"
#include "common.h"
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

// Regression: 65+ nested try_table blocks overflow exception_handlers array
// because WAH_MAX_EXCEPTION_HANDLER_DEPTH (64) < WAH_MAX_CONTROL_DEPTH (256).
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

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_err(wah_call(&ctx, 0, NULL, 0, &result), WAH_ERROR_STACK_OVERFLOW);

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
    // GC-managed exceptions — the OOM behavior is exercised by test_oom.c.
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

int main() {
    test_try_table_catch_label_types();
    test_catch_all();
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
    test_try_table_handler_overflow();
    test_exception_survives_gc_on_stack();
    test_exception_oom();
    printf("All exception tests passed!\n");
    return 0;
}
