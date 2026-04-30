#include "../wah.h"
#include "common.h"
#include <stdio.h>

// 1721112: Implement typed select instruction (0x1C).
// Runtime test: typed select on funcref values
static void test_typed_select_funcref() {
    printf("Testing typed select on funcref (1721112)...\n");

    // func takes i32 condition, uses typed select on two funcref values,
    // then ref.is_null to check which was selected.
    const char *spec = "wasm \
        types {[ fn [i32] [i32] ]} \
        funcs {[ 0 ]} \
        elements {[ elem.declarative.expr funcref [ ref.func 0 end ] ]} \
        code {[ {[] \
            ref.func 0 \
            ref.null funcref \
            local.get 0 \
            select.typed 1 funcref \
            ref.is_null \
        end } ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t params[1], result;

    params[0].i32 = 1;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 0);

    params[0].i32 = 0;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 1);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// 1721112: Typed select on i32 (basic positive)
static void test_typed_select_i32() {
    printf("Testing typed select on i32 (1721112)...\n");

    // select_t with i32: same as regular select but uses typed encoding
    const char *spec = "wasm \
        types {[ fn [i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            i32.const 42 \
            i32.const 99 \
            local.get 0 \
            select.typed 1 i32 \
        end } ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t params[1], result;

    params[0].i32 = 1;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 42);

    params[0].i32 = 0;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 99);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// c14877f: Reject untyped select instruction with reference type operands.
static void test_untyped_select_rejects_refs() {
    printf("Testing untyped select rejects refs (c14877f)...\n");

    // Untyped select (0x1B) with funcref operands should fail validation
    const char *bad_spec = "wasm \
        types {[ fn [] [funcref] ]} \
        funcs {[ 0 ]} \
        elements {[ elem.declarative.expr funcref [ ref.func 0 end ] ]} \
        code {[ {[] \
            ref.func 0 \
            ref.null funcref \
            i32.const 1 \
            select \
        end } ]}";

    wah_module_t module = {0};
    wah_error_t err = wah_parse_module_from_spec(&module, bad_spec);
    assert_err(err, WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module);

    // Positive: typed select (0x1C) with funcref should succeed
    const char *good_spec = "wasm \
        types {[ fn [] [funcref] ]} \
        funcs {[ 0 ]} \
        elements {[ elem.declarative.expr funcref [ ref.func 0 end ] ]} \
        code {[ {[] \
            ref.func 0 \
            ref.null funcref \
            i32.const 1 \
            select.typed 1 funcref \
        end } ]}";

    wah_module_t good_module = {0};
    assert_ok(wah_parse_module_from_spec(&good_module, good_spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &good_module, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_not_null(result.ref);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&good_module);
}

// 06349ef: Validate heap type indices across all module sections.
static void test_oob_heap_type_indices() {
    printf("Testing OOB heap type indices (06349ef)...\n");

    // OOB type index in global section: global of type (ref null 99) where type 99 doesn't exist
    const char *bad_global = "wasm \
        types {[ fn [] [] ]} \
        globals {[ type.ref.null 99 mut ref.null 99 end ]}";
    wah_module_t m1 = {0};
    assert_err(wah_parse_module_from_spec(&m1, bad_global), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&m1);

    // OOB type index in table section: table of (ref null 99)
    const char *bad_table = "wasm \
        types {[ fn [] [] ]} \
        tables {[ type.ref.null 99 limits.i32/1 0 ]}";
    wah_module_t m2 = {0};
    assert_err(wah_parse_module_from_spec(&m2, bad_table), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&m2);

    // OOB type index in element segment
    const char *bad_elem = "wasm \
        types {[ fn [] [] ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        elements {[ elem.passive.expr type.ref.null 99 [ ref.null 99 end ] ]}";
    wah_module_t m3 = {0};
    assert_err(wah_parse_module_from_spec(&m3, bad_elem), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&m3);
}

// 328399d: Validate ref.func "declared" requirement per WebAssembly spec.
static void test_ref_func_declared() {
    printf("Testing ref.func declared requirement (328399d)...\n");

    // Positive: ref.func with function declared in an element segment
    const char *good_spec = "wasm \
        types {[ fn [] [funcref] ]} \
        funcs {[ 0 ]} \
        elements {[ elem.declarative.expr funcref [ ref.func 0 end ] ]} \
        code {[ {[] ref.func 0 end } ]}";
    wah_module_t good = {0};
    assert_ok(wah_parse_module_from_spec(&good, good_spec));
    wah_free_module(&good);

    // Negative: ref.func with undeclared function
    const char *bad_spec = "wasm \
        types {[ fn [] [funcref], fn [] [] ]} \
        funcs {[ 0, 1 ]} \
        code {[ {[] ref.func 1 end }, {[] end } ]}";
    wah_module_t bad = {0};
    assert_err(wah_parse_module_from_spec(&bad, bad_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad);

    // Positive: ref.func with function declared via export
    const char *export_spec = "wasm \
        types {[ fn [] [funcref] ]} \
        funcs {[ 0 ]} \
        exports {[ {'f'} fn# 0 ]} \
        code {[ {[] ref.func 0 end } ]}";
    wah_module_t exported = {0};
    assert_ok(wah_parse_module_from_spec(&exported, export_spec));
    wah_free_module(&exported);
}

// 9f6d790: Fix br_on_null / br_on_non_null label type validation.
static void test_br_on_null_label_types() {
    printf("Testing br_on_null label type validation (9f6d790)...\n");

    // Positive: br_on_null with matching label type.
    // block (void -> i32) { funcref on stack; br_on_null 0 pops ref, branches with i32 }
    // Actually, br_on_null branches when null, and the label must match the values below the ref.
    // Simpler: br_on_null 0 where label expects no results.
    const char *good_spec = "wasm \
        types {[ fn [funcref] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            block void \
                local.get 0 \
                br_on_null 0 \
                drop \
            end \
            i32.const 1 \
        end } ]}";

    wah_module_t good = {0};
    assert_ok(wah_parse_module_from_spec(&good, good_spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &good, NULL));
    assert_ok(wah_instantiate(&ctx));

    // null input -> branches, returns 1
    wah_value_t params[1] = {{.ref = NULL}};
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 1);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&good);

    // Negative: br_on_null targeting label with wrong result types
    // br_on_null targets block(void->i64) but we only have i32 below
    const char *bad_spec = "wasm \
        types {[ fn [funcref] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            block i64 \
                i32.const 1 \
                local.get 0 \
                br_on_null 0 \
                drop \
            end \
            drop \
            i32.const 0 \
        end } ]}";

    wah_module_t bad = {0};
    assert_err(wah_parse_module_from_spec(&bad, bad_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad);
}

// d3668a2: Implement call_ref.
static void test_call_ref() {
    printf("Testing call_ref (d3668a2)...\n");

    // Successful call_ref: ref.func 0 then call_ref 0
    const char *spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0, 0 ]} \
        elements {[ elem.declarative.expr funcref [ ref.func 0 end ] ]} \
        code {[ \
            {[] i32.const 42 end }, \
            {[] ref.func 0 call_ref 0 end } \
        ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &module, NULL));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);

    // Null call_ref should trap
    // ref.null <type_idx> to get a nullable concrete type for call_ref
    const char *null_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] ref.null 0 call_ref 0 end } ]}";

    wah_module_t null_mod = {0};
    assert_ok(wah_parse_module_from_spec(&null_mod, null_spec));

    wah_exec_context_t ctx2 = {0};
    assert_ok(wah_exec_context_create(&ctx2, &null_mod, NULL));
    assert_ok(wah_instantiate(&ctx2));

    assert_err(wah_call(&ctx2, 0, NULL, 0, &result), WAH_ERROR_TRAP);

    wah_exec_context_destroy(&ctx2);
    wah_free_module(&null_mod);
}

// dbea808: Implement uninitialized local tracking in validator for non-defaultable types.
static void test_uninit_local_tracking() {
    printf("Testing uninitialized local tracking (dbea808)...\n");

    // Negative: use non-defaultable local before init
    // Local 0 (param) is implicit. Local 1 is (ref func) non-defaultable.
    // local.get 1 before local.set should fail.
    const char *bad_spec = "wasm \
        types {[ fn [] [funcref] ]} \
        funcs {[ 0 ]} \
        elements {[ elem.declarative.expr funcref [ ref.func 0 end ] ]} \
        code {[ {[1 type.ref.func] local.get 0 end } ]}";

    wah_module_t bad = {0};
    assert_err(wah_parse_module_from_spec(&bad, bad_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad);

    // Positive: init before use
    const char *good_spec = "wasm \
        types {[ fn [] [funcref] ]} \
        funcs {[ 0 ]} \
        elements {[ elem.declarative.expr funcref [ ref.func 0 end ] ]} \
        code {[ {[1 type.ref.func] ref.func 0 local.set 0 local.get 0 end } ]}";

    wah_module_t good = {0};
    assert_ok(wah_parse_module_from_spec(&good, good_spec));
    wah_free_module(&good);
}

// 43b4fce: Add subtype validation: finality check and structural compatibility.
static void test_subtype_validation() {
    printf("Testing subtype validation (43b4fce)...\n");

    // Negative: subtype of implicitly-final type (sub.final is the default)
    // Type 0 is implicitly final (fn []->[i32]), type 1 tries to be its subtype.
    const char *bad_final = "wasm \
        types {[ sub.final [] fn [] [i32], sub [0] fn [] [i32] ]}";

    wah_module_t bad1 = {0};
    assert_err(wah_parse_module_from_spec(&bad1, bad_final), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad1);

    // Negative: structural mismatch (different param count)
    // Type 0 is non-final fn []->[i32], type 1 is fn [i32]->[i32] subtype of 0
    const char *bad_struct = "wasm \
        types {[ sub [] fn [] [i32], sub [0] fn [i32] [i32] ]}";

    wah_module_t bad2 = {0};
    assert_err(wah_parse_module_from_spec(&bad2, bad_struct), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad2);

    // Positive: structurally compatible subtype
    // Type 0 is non-final fn []->[i32], type 1 is fn []->[i32] subtype of 0
    const char *good = "wasm \
        types {[ sub [] fn [] [i32], sub [0] fn [] [i32] ]}";

    wah_module_t good_mod = {0};
    assert_ok(wah_parse_module_from_spec(&good_mod, good));
    wah_free_module(&good_mod);
}

// 5922606: Validate type reference scope: reject forward refs across rec groups.
static void test_forward_ref_across_rec_groups() {
    printf("Testing forward ref across rec groups (5922606)...\n");

    // Negative: type 0 references type 1, but they're in different rec groups (implicit single-type groups)
    // This means type 0's body uses a forward reference to type 1.
    // The spec requires refs to only reference types within the same or earlier rec groups.
    // For a func type with result type.ref.null 1 where type 1 doesn't exist yet:
    const char *bad_spec = "wasm \
        types {[ fn [] [type.ref.null 1], fn [] [] ]}";

    wah_module_t bad = {0};
    assert_err(wah_parse_module_from_spec(&bad, bad_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad);

    // Positive: same rec group (both types in one rec group)
    const char *good_spec = "wasm \
        types {[ rec [ fn [] [type.ref.null 1], fn [] [] ] ]}";

    wah_module_t good = {0};
    assert_ok(wah_parse_module_from_spec(&good, good_spec));
    wah_free_module(&good);
}

// 1556aa2: Add mutability and type-compatibility checks for GC array/struct ops.
static void test_gc_mutability_check() {
    printf("Testing GC mutability checks (1556aa2)...\n");

    // Negative: array.set on immutable array type
    const char *bad_arr = "wasm \
        types {[ sub [] array i32 immut, fn [] [] ]} \
        funcs {[ 1 ]} \
        code {[ {[] \
            i32.const 0 \
            i32.const 1 \
            array.new 0 \
            i32.const 0 \
            i32.const 99 \
            array.set 0 \
        end } ]}";

    wah_module_t bad1 = {0};
    assert_err(wah_parse_module_from_spec(&bad1, bad_arr), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad1);

    // Negative: struct.set on immutable field
    // struct type: { immut i32 }
    const char *bad_struct = "wasm \
        types {[ sub [] struct [i32 immut], fn [] [] ]} \
        funcs {[ 1 ]} \
        code {[ {[] \
            i32.const 0 \
            struct.new 0 \
            i32.const 99 \
            struct.set 0 0 \
        end } ]}";

    wah_module_t bad2 = {0};
    assert_err(wah_parse_module_from_spec(&bad2, bad_struct), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad2);

    // Positive: struct.set on mutable field
    const char *good_struct = "wasm \
        types {[ sub [] struct [i32 mut], fn [] [] ]} \
        funcs {[ 1 ]} \
        code {[ {[] \
            i32.const 0 \
            struct.new 0 \
            i32.const 99 \
            struct.set 0 0 \
        end } ]}";

    wah_module_t good = {0};
    assert_ok(wah_parse_module_from_spec(&good, good_struct));
    wah_free_module(&good);
}

static void test_gc_array_numeric_operands() {
    printf("Testing GC array numeric operand validation...\n");

    const char *bad_array_new = "wasm \
        types {[ array i8 mut, fn [] [i32] ]} \
        funcs {[ 1 ]} \
        exports {[ {'f'} fn# 0 ]} \
        elements {[ elem.declarative.expr funcref [ ref.func 0 end ] ]} \
        code {[ {[] \
            i32.const 0 \
            ref.func 0 \
            array.new 0 \
            i32.const 0 \
            array.get 0 \
        end } ]}";

    wah_module_t bad = {0};
    assert_err(wah_parse_module_from_spec(&bad, bad_array_new), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad);
}

// 755875e: Validate br_on_cast branch type against target label.
// 777f003: Fix br_on_cast/br_on_cast_fail validation and null handling.
static void test_br_on_cast_label_type() {
    printf("Testing br_on_cast label type validation (755875e, 777f003)...\n");

    // Positive: br_on_cast where branch type matches label.
    // block (void -> anyref) { anyref on stack; br_on_cast 0 flags=0x03 anyref eqref }
    // cast_flags=0x03 means src nullable, dst nullable.
    // Branch type is (ref null eq) = eqref, label expects anyref.
    // eqref <: anyref so it should match.
    // br_on_cast: on success, branches with eqref (subtype of anyref -> matches label).
    // On fallthrough, pushes (ref any) which is subtype of anyref -> matches block end.
    const char *good_spec = "wasm \
        types {[ fn [] [anyref] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            block anyref \
                ref.null anyref \
                br_on_cast.null.null 0 anyref eqref \
            end \
        end } ]}";

    wah_module_t good = {0};
    assert_ok(wah_parse_module_from_spec(&good, good_spec));
    wah_free_module(&good);

    // Negative: br_on_cast where dst type doesn't match label.
    // block (void -> funcref) targets funcref, but cast dst is eqref.
    // eqref is NOT subtype of funcref.
    const char *bad_spec = "wasm \
        types {[ fn [] [funcref] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            block funcref \
                ref.null anyref \
                br_on_cast.null.null 0 anyref eqref \
                drop \
            end \
        end } ]}";

    wah_module_t bad = {0};
    assert_err(wah_parse_module_from_spec(&bad, bad_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad);
}

// 7f7884b: br_on_cast multi-value branch target validation.
// dabfef6: Fix br_on_cast exact t* match & fallthrough type.
static void test_br_on_cast_multi_value() {
    printf("Testing br_on_cast multi-value and fallthrough (7f7884b, dabfef6)...\n");

    // Negative: br_on_cast targeting a block with 0 results (must have >= 1)
    const char *bad_no_result = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            block void \
                ref.null anyref \
                br_on_cast.null.null 0 anyref eqref \
                drop \
            end \
        end } ]}";

    wah_module_t bad1 = {0};
    assert_err(wah_parse_module_from_spec(&bad1, bad_no_result), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad1);

    // Positive: multi-value target: block (i32, anyref) -> (i32, anyref).
    // br_on_cast success sends (i32, eqref) to the label.
    // The i32 on stack must exactly match the label's first result.
    const char *good_multi = "wasm \
        types {[ fn [] [i32, anyref], fn [i32, anyref] [i32, anyref] ]} \
        funcs {[ 0 ]} \
        code {[ {[] \
            i32.const 1 \
            ref.null anyref \
            block 1 \
                br_on_cast.null.null 0 anyref eqref \
            end \
        end } ]}";

    wah_module_t good = {0};
    assert_ok(wah_parse_module_from_spec(&good, good_multi));
    wah_free_module(&good);
}

int main() {
    test_typed_select_i32();
    test_typed_select_funcref();
    test_untyped_select_rejects_refs();
    test_oob_heap_type_indices();
    test_ref_func_declared();
    test_br_on_null_label_types();
    test_call_ref();
    test_uninit_local_tracking();
    test_subtype_validation();
    test_forward_ref_across_rec_groups();
    test_gc_mutability_check();
    test_gc_array_numeric_operands();
    test_br_on_cast_label_type();
    test_br_on_cast_multi_value();
    printf("All references_validation tests passed!\n");
    return 0;
}
