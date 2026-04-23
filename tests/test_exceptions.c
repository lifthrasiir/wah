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

int main() {
    test_try_table_catch_label_types();
    test_catch_all();
    printf("All exception tests passed!\n");
    return 0;
}
