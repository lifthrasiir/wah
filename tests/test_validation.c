#include "../wah.h"
#include "common.h"
#include <stdio.h>

// b13c680: Fix a validation bug where some block types are entirely skipped.
// Nested block/if/loop with complex block type (multi-param) must validate correctly.
static void test_block_type_not_skipped() {
    printf("Testing block type validation not skipped (b13c680)...\n");

    // Module with a func type (i32, i32) -> (i32) used as block type.
    // The block consumes two i32s from the stack and produces one i32.
    // func () -> (i32): push two i32s, then enter block (i32,i32)->(i32) that adds them.
    const char *spec = "wasm \
        types {[ fn [] [i32], fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            i32.const 10 \
            i32.const 20 \
            block 1 \
                i32.add \
            end \
        end } ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &module, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 30);

    wah_free_exec_context(&ctx);
    wah_free_module(&module);

    // Negative: wrong stack shape - block expects (i32, i32) but only one i32 on stack
    const char *bad_spec = "wasm \
        types {[ fn [] [i32], fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            i32.const 10 \
            block 1 \
                i32.add \
            end \
        end } ]}";

    wah_module_t bad_module = {0};
    wah_error_t err = wah_parse_module_from_spec(&bad_module, bad_spec);
    assert_err(err, WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad_module);
}

// b13c680: nested if with complex block type
static void test_if_complex_block_type() {
    printf("Testing if with complex block type (b13c680)...\n");

    // if (i32, i32) -> (i32): takes two i32s from stack, condition, then adds in then-branch
    const char *spec = "wasm \
        types {[ fn [] [i32], fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            i32.const 3 \
            i32.const 7 \
            i32.const 1 \
            if 1 \
                i32.add \
            else \
                i32.sub \
            end \
        end } ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &module, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 10);

    wah_free_exec_context(&ctx);
    wah_free_module(&module);
}

// b13c680: if without else - param types must equal result types
static void test_if_no_else_complex_block_type() {
    printf("Testing if-no-else with complex block type (b13c680)...\n");

    // if (i32) -> (i32) without else is valid because param == result type
    const char *spec = "wasm \
        types {[ fn [] [i32], fn [i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            i32.const 42 \
            i32.const 1 \
            if 1 \
                i32.const 1 \
                i32.add \
            end \
        end } ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &module, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 43);

    wah_free_exec_context(&ctx);
    wah_free_module(&module);

    // Negative: if (i32) -> (i64) without else - param != result type
    const char *bad_spec = "wasm \
        types {[ fn [] [i64], fn [i32] [i64] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            i32.const 42 \
            i32.const 1 \
            if 1 \
                drop \
                i64.const 99 \
            end \
        end } ]}";

    wah_module_t bad_module = {0};
    wah_error_t err = wah_parse_module_from_spec(&bad_module, bad_spec);
    assert_err(err, WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad_module);
}

// 1adf4f3: Fix validation of loop opcode w.r.t. complex block types.
// br targeting a loop uses param types (back-edge), not result types.
static void test_loop_complex_block_type() {
    printf("Testing loop with complex block type (1adf4f3)...\n");

    // loop (i32) -> (i32): br 0 jumps back to loop entry with param types
    // Counts down from 5, returning 0.
    // func takes i32 param, loop body uses local for the counter.
    const char *spec = "wasm \
        types {[ fn [i32] [i32], fn [i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            local.get 0 \
            loop 1 \
                local.set 0 \
                local.get 0 \
                i32.const 1 \
                i32.sub \
                local.tee 0 \
                local.get 0 \
                i32.const 0 \
                i32.gt_s \
                br_if 0 \
            end \
        end } ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &module, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t params[1] = {{.i32 = 5}};
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 0);

    wah_free_exec_context(&ctx);
    wah_free_module(&module);

    // Multi-value loop: loop (i32, i32) -> (i32)
    // The back-edge (br 0) must carry (i32, i32), the fallthrough produces (i32).
    const char *spec2 = "wasm \
        types {[ fn [] [i32], fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            i32.const 10 \
            i32.const 3 \
            loop 1 \
                i32.sub \
            end \
        end } ]}";

    wah_module_t module2 = {0};
    assert_ok(wah_parse_module_from_spec(&module2, spec2));

    wah_exec_context_t ctx2 = {0};
    assert_ok(wah_new_exec_context(&ctx2, &module2, NULL));
    assert_ok(wah_instantiate(&ctx2));

    wah_value_t result2;
    assert_ok(wah_call(&ctx2, 0, NULL, 0, &result2));
    assert_eq_i32(result2.i32, 7);

    wah_free_exec_context(&ctx2);
    wah_free_module(&module2);
}

// 9512680: Add missing value types to block types.
// funcref, externref, v128 as block result types.
static void test_block_value_types() {
    printf("Testing additional block value types (9512680)...\n");

    // block -> funcref
    const char *spec_funcref = "wasm \
        types {[ fn [] [funcref] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            block funcref \
                ref.null funcref \
            end \
        end } ]}";

    wah_module_t m1 = {0};
    assert_ok(wah_parse_module_from_spec(&m1, spec_funcref));
    wah_exec_context_t c1 = {0};
    assert_ok(wah_new_exec_context(&c1, &m1, NULL));
    assert_ok(wah_instantiate(&c1));
    wah_value_t r1;
    assert_ok(wah_call(&c1, 0, NULL, 0, &r1));
    assert_null(r1.ref);
    wah_free_exec_context(&c1);
    wah_free_module(&m1);

    // block -> externref
    const char *spec_externref = "wasm \
        types {[ fn [] [externref] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            block externref \
                ref.null externref \
            end \
        end } ]}";

    wah_module_t m2 = {0};
    assert_ok(wah_parse_module_from_spec(&m2, spec_externref));
    wah_exec_context_t c2 = {0};
    assert_ok(wah_new_exec_context(&c2, &m2, NULL));
    assert_ok(wah_instantiate(&c2));
    wah_value_t r2;
    assert_ok(wah_call(&c2, 0, NULL, 0, &r2));
    assert_null(r2.ref);
    wah_free_exec_context(&c2);
    wah_free_module(&m2);

    // Negative: block -> funcref but body produces i32
    const char *bad_spec = "wasm \
        types {[ fn [] [funcref] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            block funcref \
                i32.const 42 \
            end \
        end } ]}";

    wah_module_t bad = {0};
    assert_err(wah_parse_module_from_spec(&bad, bad_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad);
}

// 927fc70: Add missing empty validation checks.
// Empty-stack negative tests for drop, select, br_if.
static void test_empty_stack_validation() {
    printf("Testing empty stack validation (927fc70)...\n");

    // drop with empty stack
    const char *spec_drop = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        code {[ {[] drop end } ]}";
    wah_module_t m1 = {0};
    assert_err(wah_parse_module_from_spec(&m1, spec_drop), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&m1);

    // select with empty stack
    const char *spec_select = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] select end } ]}";
    wah_module_t m2 = {0};
    assert_err(wah_parse_module_from_spec(&m2, spec_select), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&m2);

    // br_if with empty stack (needs at least i32 condition)
    const char *spec_brif = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        code {[ {[] block void br_if 0 end end } ]}";
    wah_module_t m3 = {0};
    assert_err(wah_parse_module_from_spec(&m3, spec_brif), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&m3);
}

// 3c51e96: Validation stack should reset after unreachable.
static void test_unreachable_stack_reset() {
    printf("Testing unreachable stack reset (3c51e96)...\n");

    // unreachable in a void block, followed by type-correct return value after block
    const char *spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            block void \
                unreachable \
            end \
            i32.const 42 \
        end } ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_free_module(&module);

    // Unreachable in block with result: block produces matching type
    const char *spec2 = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            block i32 \
                unreachable \
                i32.const 99 \
            end \
        end } ]}";

    wah_module_t module2 = {0};
    assert_ok(wah_parse_module_from_spec(&module2, spec2));
    wah_free_module(&module2);
}

// cfbe177: Fix a bug where is_unreachable is retained too long.
static void test_unreachable_not_retained() {
    printf("Testing unreachable not retained past block (cfbe177)...\n");

    // After a block containing unreachable, the next block must enforce normal rules.
    // First block (void->void) has unreachable inside, then second block (void->i32)
    // must validate normally, not as unreachable.
    const char *spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            block void \
                unreachable \
            end \
            block i32 \
                i32.const 42 \
            end \
        end } ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_free_module(&module);

    // Negative: after block with unreachable, reachable code must still validate
    const char *bad_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            block void \
                unreachable \
            end \
            i32.add \
        end } ]}";

    wah_module_t bad = {0};
    assert_err(wah_parse_module_from_spec(&bad, bad_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad);
}

// 5369539: Correct stack height checks when unreachable.
static void test_unreachable_stack_height() {
    printf("Testing stack height checks when unreachable (5369539)...\n");

    // br in unreachable path should not require stack values above block floor
    const char *spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            block i32 \
                unreachable \
                br 0 \
            end \
        end } ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_free_module(&module);

    // Reachable path: br must have proper values
    const char *bad_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            block i32 \
                i32.const 1 \
                i32.const 2 \
            end \
        end } ]}";

    wah_module_t bad = {0};
    assert_err(wah_parse_module_from_spec(&bad, bad_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad);
}

// ae67e6b: Enforce block stack floor in br/return validation.
static void test_br_block_floor() {
    printf("Testing br block stack floor enforcement (ae67e6b)...\n");

    const char *bad_spec2 = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            block i32 \
                block void \
                    br 1 \
                end \
                i32.const 1 \
            end \
        end } ]}";

    wah_module_t bad = {0};
    assert_err(wah_parse_module_from_spec(&bad, bad_spec2), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad);

    // Positive: br with proper value on stack
    const char *good_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            block i32 \
                block void \
                    i32.const 42 \
                    br 1 \
                end \
                i32.const 1 \
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
}

// f3d52e6: Validate stack height at else in if blocks.
static void test_else_stack_height() {
    printf("Testing else stack height validation (f3d52e6)...\n");

    // if (void->i32): then-branch leaves wrong stack (two values)
    const char *bad_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            i32.const 1 \
            if i32 \
                i32.const 1 \
                i32.const 2 \
            else \
                i32.const 3 \
            end \
        end } ]}";

    wah_module_t bad = {0};
    assert_err(wah_parse_module_from_spec(&bad, bad_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad);

    // Positive: correct stack at else
    const char *good_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            i32.const 1 \
            if i32 \
                i32.const 2 \
            else \
                i32.const 3 \
            end \
        end } ]}";

    wah_module_t good = {0};
    assert_ok(wah_parse_module_from_spec(&good, good_spec));
    wah_free_module(&good);
}

static void test_function_end_with_results() {
    printf("Testing function END with results validation...\n");
    // Positive: function returns i32, stack has exactly one i32 at END
    const char *spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] i32.const 42 end } ]}";
    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &module, NULL));
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);
    wah_free_exec_context(&ctx);
    wah_free_module(&module);

    // Negative: function returns i32 but stack has two i32s at END
    const char *bad_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] i32.const 1 i32.const 2 end } ]}";
    wah_module_t bad = {0};
    assert_err(wah_parse_module_from_spec(&bad, bad_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad);
}

static void test_unknown_opcode() {
    printf("Testing unknown opcode validation...\n");
    wah_module_t module = {0};
    // Use raw hex for code body: 0x06 is an unrecognized opcode
    assert_err(wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [] [] ]} funcs {[ 0 ]} \
        code {[ {[] %'06' end } ]}"),
        WAH_ERROR_MALFORMED);
    wah_free_module(&module);
}

static void test_ref_test_concrete_heap_type() {
    printf("Testing ref.test with concrete heap type...\n");
    // ref.test checks if a structref is actually type $0 (a struct type)
    const char *spec = "wasm \
        types {[ struct [i32 mut], fn [] [i32] ]} \
        funcs {[ 1 ]} \
        code {[ {[] \
            ref.null structref \
            ref.test 0 \
        end } ]}";
    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &module, NULL));
    assert_ok(wah_instantiate(&ctx));
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 0); // null ref fails ref.test
    wah_free_exec_context(&ctx);
    wah_free_module(&module);
}

static void test_nullref_subtype_check() {
    printf("Testing nullref subtype of struct type...\n");
    // A block expects structref on input, but we pass nullref (which is a subtype of struct types)
    // This exercises the NULLREF subtype path
    const char *spec = "wasm \
        types {[ struct [i32 mut], fn [] [i32] ]} \
        funcs {[ 1 ]} \
        code {[ {[] \
            ref.null nullref \
            ref.test 0 \
        end } ]}";
    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_exec_context_t ctx = {0};
    assert_ok(wah_new_exec_context(&ctx, &module, NULL));
    assert_ok(wah_instantiate(&ctx));
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 0);
    wah_free_exec_context(&ctx);
    wah_free_module(&module);
}

static void test_struct_new_default_non_defaultable() {
    printf("Testing struct.new_default rejects non-defaultable fields...\n");

    // struct { (ref 0) } -- non-nullable ref field is NOT defaultable
    // struct.new_default should be rejected by validation
    const char *bad_spec = "wasm \
        types {[ sub [] struct [type.ref 0 mut], fn [] [i32] ]} \
        funcs {[ 1 ]} \
        code {[ {[] struct.new_default 0 ref.is_null end } ]}";

    wah_module_t bad = {0};
    assert_err(wah_parse_module_from_spec(&bad, bad_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad);

    // struct { (ref null 0) } -- nullable ref field IS defaultable (should pass)
    const char *good_spec = "wasm \
        types {[ sub [] struct [type.ref.null 0 mut], fn [] [i32] ]} \
        funcs {[ 1 ]} \
        code {[ {[] struct.new_default 0 ref.is_null end } ]}";

    wah_module_t good = {0};
    assert_ok(wah_parse_module_from_spec(&good, good_spec));
    wah_free_module(&good);
}

static void test_array_new_default_non_defaultable() {
    printf("Testing array.new_default rejects non-defaultable element type...\n");

    // array (ref 0) -- non-nullable ref element is NOT defaultable
    const char *bad_spec = "wasm \
        types {[ sub [] array type.ref 0 mut, fn [] [i32] ]} \
        funcs {[ 1 ]} \
        code {[ {[] i32.const 3 array.new_default 0 ref.is_null end } ]}";

    wah_module_t bad = {0};
    assert_err(wah_parse_module_from_spec(&bad, bad_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad);

    // array (ref null 0) -- nullable ref element IS defaultable (should pass)
    const char *good_spec = "wasm \
        types {[ sub [] array type.ref.null 0 mut, fn [] [i32] ]} \
        funcs {[ 1 ]} \
        code {[ {[] i32.const 3 array.new_default 0 ref.is_null end } ]}";

    wah_module_t good = {0};
    assert_ok(wah_parse_module_from_spec(&good, good_spec));
    wah_free_module(&good);
}

static void test_non_nullable_field_store_validation() {
    printf("Testing struct.set/array.set reject nullable value for non-nullable field...\n");

    // struct { mut (ref 0) } -- non-nullable field
    // struct.set with ref.null should be rejected
    const char *bad_struct_set = "wasm \
        types {[ sub [] struct [type.ref 0 mut], fn [type.ref.null 0] [] ]} \
        funcs {[ 1 ]} \
        code {[ {[] \
            local.get 0 \
            ref.null 0 \
            struct.set 0 0 \
        end } ]}";

    wah_module_t bad1 = {0};
    assert_err(wah_parse_module_from_spec(&bad1, bad_struct_set), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad1);

    // struct.new with ref.null for non-nullable field should also be rejected
    const char *bad_struct_new = "wasm \
        types {[ sub [] struct [type.ref 0 mut], fn [] [type.ref.null 0] ]} \
        funcs {[ 1 ]} \
        code {[ {[] \
            ref.null 0 \
            struct.new 0 \
        end } ]}";

    wah_module_t bad2 = {0};
    assert_err(wah_parse_module_from_spec(&bad2, bad_struct_new), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad2);

    // array.set with ref.null for non-nullable element should be rejected
    const char *bad_array_set = "wasm \
        types {[ sub [] array type.ref 0 mut, fn [type.ref.null 0] [] ]} \
        funcs {[ 1 ]} \
        code {[ {[] \
            local.get 0 \
            i32.const 0 \
            ref.null 0 \
            array.set 0 \
        end } ]}";

    wah_module_t bad3 = {0};
    assert_err(wah_parse_module_from_spec(&bad3, bad_array_set), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad3);

    // Positive: struct.set with non-nullable value for non-nullable field (should pass)
    const char *good_struct_set = "wasm \
        types {[ sub [] struct [type.ref 0 mut], fn [type.ref 0] [] ]} \
        funcs {[ 1 ]} \
        code {[ {[] \
            local.get 0 \
            local.get 0 \
            struct.set 0 0 \
        end } ]}";

    wah_module_t good = {0};
    assert_ok(wah_parse_module_from_spec(&good, good_struct_set));
    wah_free_module(&good);
}

static void test_packed_field_get_validation() {
    printf("Testing struct.get_s/get_u require packed fields...\n");

    // struct.get_s on non-packed i32 field should be rejected
    const char *bad_get_s = "wasm \
        types {[ sub [] struct [i32 mut], fn [type.ref.null 0] [i32] ]} \
        funcs {[ 1 ]} \
        code {[ {[] local.get 0 struct.get_s 0 0 end } ]}";

    wah_module_t m1 = {0};
    assert_err(wah_parse_module_from_spec(&m1, bad_get_s), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&m1);

    // struct.get_u on non-packed i32 field should be rejected
    const char *bad_get_u = "wasm \
        types {[ sub [] struct [i32 mut], fn [type.ref.null 0] [i32] ]} \
        funcs {[ 1 ]} \
        code {[ {[] local.get 0 struct.get_u 0 0 end } ]}";

    wah_module_t m2 = {0};
    assert_err(wah_parse_module_from_spec(&m2, bad_get_u), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&m2);

    // struct.get on packed i8 field should be rejected
    // struct { i8 mut }
    const char *bad_get_packed = "wasm \
        types {[ sub [] struct [i8 mut], fn [type.ref.null 0] [i32] ]} \
        funcs {[ 1 ]} \
        code {[ {[] local.get 0 struct.get 0 0 end } ]}";

    wah_module_t m3 = {0};
    assert_err(wah_parse_module_from_spec(&m3, bad_get_packed), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&m3);

    // Positive: struct.get_s on packed i8 field should pass
    const char *good_get_s = "wasm \
        types {[ sub [] struct [i8 mut], fn [type.ref.null 0] [i32] ]} \
        funcs {[ 1 ]} \
        code {[ {[] local.get 0 struct.get_s 0 0 end } ]}";

    wah_module_t m4 = {0};
    assert_ok(wah_parse_module_from_spec(&m4, good_get_s));
    wah_free_module(&m4);

    // array.get_s on non-packed i32 array should be rejected
    const char *bad_array_get_s = "wasm \
        types {[ sub [] array i32 mut, fn [type.ref.null 0] [i32] ]} \
        funcs {[ 1 ]} \
        code {[ {[] local.get 0 i32.const 0 array.get_s 0 end } ]}";

    wah_module_t m5 = {0};
    assert_err(wah_parse_module_from_spec(&m5, bad_array_get_s), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&m5);
}

static void test_ref_cast_hierarchy_validation() {
    printf("Testing ref.cast/ref.test reject cross-hierarchy casts...\n");

    // ref.test structref on funcref (any vs func hierarchy)
    const char *bad1 = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] ref.null funcref ref.test structref end } ]}";
    wah_module_t m1 = {0};
    assert_err(wah_parse_module_from_spec(&m1, bad1), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&m1);

    // ref.cast externref on anyref (extern vs any hierarchy)
    const char *bad2 = "wasm \
        types {[ fn [] [externref] ]} \
        funcs {[ 0 ]} \
        code {[ {[] ref.null anyref ref.cast externref end } ]}";
    wah_module_t m2 = {0};
    assert_err(wah_parse_module_from_spec(&m2, bad2), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&m2);

    // ref.test i31ref on anyref (same any hierarchy -- valid)
    const char *good1 = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] ref.null anyref ref.test i31ref end } ]}";
    wah_module_t m3 = {0};
    assert_ok(wah_parse_module_from_spec(&m3, good1));
    wah_free_module(&m3);

    // ref.test concrete struct on anyref (same any hierarchy -- valid)
    const char *good2 = "wasm \
        types {[ struct [i32 mut], fn [] [i32] ]} \
        funcs {[ 1 ]} \
        code {[ {[] ref.null anyref ref.test 0 end } ]}";
    wah_module_t m4 = {0};
    assert_ok(wah_parse_module_from_spec(&m4, good2));
    wah_free_module(&m4);

    // ref.test funcref on (ref func_type) (same func hierarchy -- valid)
    const char *good3 = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        exports {[ {'f'} fn# 0 ]} \
        code {[ {[] ref.func 0 ref.test.null funcref end } ]}";
    wah_module_t m5 = {0};
    assert_ok(wah_parse_module_from_spec(&m5, good3));
    wah_free_module(&m5);
}

static void test_non_func_type_as_function_type() {
    printf("Testing non-func type index rejected in function contexts...\n");

    // Function section: struct type used as function type
    const char *func_section = "wasm \
        types {[ struct [i64 mut] ]} \
        funcs {[ 0 ]} \
        code {[{[] end}]}";
    wah_module_t m1 = {0};
    assert_err(wah_parse_module_from_spec(&m1, func_section), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&m1);

    // Function import: struct type used as function type
    const char *func_import = "wasm \
        types {[ struct [i64 mut] ]} \
        imports {[ {'m'} {'f'} fn# 0 ]}";
    wah_module_t m2 = {0};
    assert_err(wah_parse_module_from_spec(&m2, func_import), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&m2);

    // call_indirect: struct type used as call target type
    const char *call_indirect = "wasm \
        types {[ struct [i64 mut], fn [] [] ]} \
        funcs {[ 1 ]} \
        tables {[ funcref 0 1 ]} \
        code {[{[] i32.const 0 call_indirect 0 0 end}]}";
    wah_module_t m3 = {0};
    assert_err(wah_parse_module_from_spec(&m3, call_indirect), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&m3);

    // Tag section: struct type used as tag type
    const char *tag_section = "wasm \
        types {[ struct [i64 mut] ]} \
        tags {[ 0 0 ]}";
    wah_module_t m4 = {0};
    wah_parse_options_t opts = {0};
    opts.features = WAH_FEATURE_ALL;
    assert_err(wah_parse_module_from_spec_ex(&m4, &opts, tag_section), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&m4);
}

static void test_non_func_type_as_block_type() {
    printf("Testing non-func type index rejected in block type...\n");

    // block with struct type index
    const char *block_struct = "wasm \
        types {[ struct [i64 mut], fn [] [] ]} \
        funcs {[ 1 ]} \
        code {[{[] block 0 end end}]}";
    wah_module_t m1 = {0};
    assert_err(wah_parse_module_from_spec(&m1, block_struct), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&m1);

    // loop with struct type index
    const char *loop_struct = "wasm \
        types {[ struct [i32 mut], fn [] [] ]} \
        funcs {[ 1 ]} \
        code {[{[] loop 0 end end}]}";
    wah_module_t m2 = {0};
    assert_err(wah_parse_module_from_spec(&m2, loop_struct), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&m2);

    // if with struct type index
    const char *if_struct = "wasm \
        types {[ struct [i32 mut], fn [] [] ]} \
        funcs {[ 1 ]} \
        code {[{[] i32.const 1 if 0 else end end}]}";
    wah_module_t m3 = {0};
    assert_err(wah_parse_module_from_spec(&m3, if_struct), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&m3);

}

static void test_unreachable_end_restore() {
    printf("Testing END restores is_unreachable from the popped frame...\n");

    // (func (result i32) unreachable (block) i32.const 1 i32.add)
    // The outer unreachable state must survive the inner block's end.
    const char *spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            unreachable \
            block void \
            end \
            i32.const 1 \
            i32.add \
        end } ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    wah_free_module(&module);

    // Negative: nested end inside dead code must not keep polymorphic state
    // if the enclosing code was reachable when the block was entered.
    const char *bad_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            block void \
                unreachable \
            end \
            i32.add \
        end } ]}";

    wah_module_t bad = {0};
    assert_err(wah_parse_module_from_spec(&bad, bad_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad);
}

int main() {
    test_block_type_not_skipped();
    test_if_complex_block_type();
    test_if_no_else_complex_block_type();
    test_loop_complex_block_type();
    test_block_value_types();
    test_empty_stack_validation();
    test_unreachable_stack_reset();
    test_unreachable_not_retained();
    test_unreachable_stack_height();
    test_br_block_floor();
    test_else_stack_height();
    test_function_end_with_results();
    test_unknown_opcode();
    test_ref_test_concrete_heap_type();
    test_nullref_subtype_check();
    test_struct_new_default_non_defaultable();
    test_array_new_default_non_defaultable();
    test_non_nullable_field_store_validation();
    test_packed_field_get_validation();
    test_ref_cast_hierarchy_validation();
    test_non_func_type_as_function_type();
    test_non_func_type_as_block_type();
    test_unreachable_end_restore();
    printf("All validation tests passed!\n");
    return 0;
}
