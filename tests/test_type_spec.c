#include "../wah.h"
#include "wah_impl.h"
#include "common.h"
#include <stdio.h>

static void assert_func_counts(const wah_debug_type_spec_t *spec, uint32_t nparams, uint32_t nresults) {
    assert_eq_u32(spec->kind, WAH_DEBUG_TYPE_SPEC_FUNC);
    assert_eq_u32(spec->param_count, nparams);
    assert_eq_u32(spec->result_count, nresults);
}

static void assert_field(const wah_debug_type_spec_t *spec, uint32_t idx, wah_type_t type, bool is_mutable) {
    assert_true(idx < spec->field_count);
    assert_eq_i32(spec->field_types[idx], type);
    assert_eq_i32(spec->field_mutables[idx], is_mutable);
}

static void test_function_specs(void) {
    printf("Testing function type specs...\n");
    wah_module_t mod = {0};
    wah_debug_type_spec_t spec = {0};
    assert_ok(wah_new_module(&mod, NULL));

    assert_ok(wah_debug_parse_define_type_spec(&mod, &spec, "fn ()"));
    assert_func_counts(&spec, 0, 0);
    wah_debug_free_type_spec(&spec);

    assert_ok(wah_debug_parse_define_type_spec(&mod, &spec, "fn () -> ()"));
    assert_func_counts(&spec, 0, 0);
    wah_debug_free_type_spec(&spec);

    assert_ok(wah_debug_parse_define_type_spec(&mod, &spec,
        "fn (i32, i64, f32, f64, v128, funcref, externref, exnref, anyref, eqref, \
             i31ref, structref, arrayref, nullref, nullfuncref, nullexternref, nullexnref) -> (i32, funcref)"));
    assert_func_counts(&spec, 17, 2);
    assert_eq_i32(spec.param_types[0], WAH_TYPE_I32);
    assert_eq_i32(spec.param_types[4], WAH_TYPE_V128);
    assert_eq_i32(spec.param_types[5], WAH_TYPE_FUNCREF);
    assert_eq_i32(spec.param_types[16], WAH_TYPE_NULLEXNREF);
    assert_eq_i32(spec.result_types[0], WAH_TYPE_I32);
    assert_eq_i32(spec.result_types[1], WAH_TYPE_FUNCREF);
    wah_debug_free_type_spec(&spec);

    assert_err(wah_debug_parse_define_type_spec(&mod, &spec, "() -> ()"), WAH_ERROR_BAD_SPEC);
    assert_ok(wah_debug_parse_export_func_type_spec(&mod, &spec, "() -> ()"));
    assert_func_counts(&spec, 0, 0);
    wah_debug_free_type_spec(&spec);
    assert_ok(wah_debug_parse_export_func_type_spec(&mod, &spec, "fn () -> ()"));
    assert_func_counts(&spec, 0, 0);
    wah_debug_free_type_spec(&spec);
    assert_err(wah_debug_parse_export_func_type_spec(&mod, &spec, "struct { i32 }"), WAH_ERROR_BAD_SPEC);

    wah_free_module(&mod);
}

static void test_struct_and_array_specs(void) {
    printf("Testing struct and array type specs...\n");
    wah_module_t mod = {0};
    wah_debug_type_spec_t spec = {0};
    assert_ok(wah_new_module(&mod, NULL));

    assert_ok(wah_debug_parse_define_type_spec(&mod, &spec, "struct { i32, mut funcref, i8, mut i16 }"));
    assert_eq_u32(spec.kind, WAH_DEBUG_TYPE_SPEC_STRUCT);
    assert_eq_u32(spec.field_count, 4);
    assert_field(&spec, 0, WAH_TYPE_I32, false);
    assert_field(&spec, 1, WAH_TYPE_FUNCREF, true);
    assert_field(&spec, 2, WAH_TYPE_PACKED_I8, false);
    assert_field(&spec, 3, WAH_TYPE_PACKED_I16, true);
    wah_debug_free_type_spec(&spec);

    assert_ok(wah_debug_parse_define_type_spec(&mod, &spec, "array i8"));
    assert_eq_u32(spec.kind, WAH_DEBUG_TYPE_SPEC_ARRAY);
    assert_eq_u32(spec.field_count, 1);
    assert_field(&spec, 0, WAH_TYPE_PACKED_I8, false);
    wah_debug_free_type_spec(&spec);

    assert_ok(wah_debug_parse_define_type_spec(&mod, &spec, "array mut i16"));
    assert_eq_u32(spec.kind, WAH_DEBUG_TYPE_SPEC_ARRAY);
    assert_eq_u32(spec.field_count, 1);
    assert_field(&spec, 0, WAH_TYPE_PACKED_I16, true);
    wah_debug_free_type_spec(&spec);

    assert_err(wah_debug_parse_define_type_spec(&mod, &spec, "struct"), WAH_ERROR_BAD_SPEC);
    assert_err(wah_debug_parse_define_type_spec(&mod, &spec, "array"), WAH_ERROR_BAD_SPEC);

    wah_free_module(&mod);
}

static void test_edge_cases(void) {
    printf("Testing type spec edge cases...\n");
    wah_module_t mod = {0};
    wah_debug_type_spec_t spec = {0};
    assert_ok(wah_new_module(&mod, NULL));

    assert_err(wah_debug_parse_define_type_spec(&mod, &spec, "fn (i8)"), WAH_ERROR_BAD_SPEC);
    assert_err(wah_debug_parse_define_type_spec(&mod, &spec, "fn () -> i16"), WAH_ERROR_BAD_SPEC);

    assert_err(wah_debug_parse_define_type_spec(&mod, &spec, "fn (func)"), WAH_ERROR_BAD_SPEC);
    assert_err(wah_debug_parse_define_type_spec(&mod, &spec, "fn (extern)"), WAH_ERROR_BAD_SPEC);
    assert_ok(wah_debug_parse_define_type_spec(&mod, &spec, "fn (ref func, ref null func)"));
    assert_func_counts(&spec, 2, 0);
    assert_eq_i32(spec.param_types[0], WAH_TYPE_FUNC);
    assert_eq_i32(spec.param_types[1], WAH_TYPE_FUNCREF);
    wah_debug_free_type_spec(&spec);

    assert_err(wah_debug_parse_define_type_spec(&mod, &spec, "fn (ref funcref)"), WAH_ERROR_BAD_SPEC);
    assert_err(wah_debug_parse_define_type_spec(&mod, &spec, "fn (ref null funcref)"), WAH_ERROR_BAD_SPEC);
    assert_err(wah_debug_parse_define_type_spec(&mod, &spec, "fn (ref nullfuncref)"), WAH_ERROR_BAD_SPEC);

    assert_err(wah_debug_parse_define_type_spec(&mod, &spec, "fn (struct)"), WAH_ERROR_BAD_SPEC);
    assert_err(wah_debug_parse_define_type_spec(&mod, &spec, "fn (array)"), WAH_ERROR_BAD_SPEC);
    assert_ok(wah_debug_parse_define_type_spec(&mod, &spec, "fn (ref struct, ref array)"));
    assert_func_counts(&spec, 2, 0);
    assert_eq_i32(spec.param_types[0], WAH_TYPE_STRUCT);
    assert_eq_i32(spec.param_types[1], WAH_TYPE_ARRAY);
    wah_debug_free_type_spec(&spec);

    wah_free_module(&mod);
}

static void test_placeholders(void) {
    printf("Testing %%T placeholders...\n");
    wah_module_t mod = {0};
    wah_debug_type_spec_t spec = {0};
    wah_type_t struct_type;
    assert_ok(wah_new_module(&mod, NULL));

    assert_ok(wah_debug_parse_define_type_spec(&mod, &spec, "fn (%T, %T)", WAH_TYPE_I32, WAH_TYPE_FUNCREF));
    assert_func_counts(&spec, 2, 0);
    assert_eq_i32(spec.param_types[0], WAH_TYPE_I32);
    assert_eq_i32(spec.param_types[1], WAH_TYPE_FUNCREF);
    wah_debug_free_type_spec(&spec);

    assert_err(wah_debug_parse_define_type_spec(&mod, &spec, "fn (%T)", WAH_TYPE_PACKED_I8), WAH_ERROR_BAD_SPEC);
    assert_ok(wah_debug_parse_define_type_spec(&mod, &spec, "struct { %T, mut %T }", WAH_TYPE_PACKED_I8, WAH_TYPE_PACKED_I16));
    assert_eq_u32(spec.kind, WAH_DEBUG_TYPE_SPEC_STRUCT);
    assert_eq_u32(spec.field_count, 2);
    assert_field(&spec, 0, WAH_TYPE_PACKED_I8, false);
    assert_field(&spec, 1, WAH_TYPE_PACKED_I16, true);
    wah_debug_free_type_spec(&spec);

    assert_err(wah_debug_parse_define_type_spec(&mod, &spec, "fn (%T)", WAH_TYPE_FUNC), WAH_ERROR_BAD_SPEC);
    assert_err(wah_debug_parse_define_type_spec(&mod, &spec, "fn (ref %T)", WAH_TYPE_FUNCREF), WAH_ERROR_BAD_SPEC);
    assert_ok(wah_debug_parse_define_type_spec(&mod, &spec, "fn (ref %T, ref null %T)", WAH_TYPE_FUNC, WAH_TYPE_FUNC));
    assert_func_counts(&spec, 2, 0);
    assert_eq_i32(spec.param_types[0], WAH_TYPE_FUNC);
    assert_eq_i32(spec.param_types[1], WAH_TYPE_FUNCREF);
    wah_debug_free_type_spec(&spec);

    assert_err(wah_debug_parse_export_func_type_spec(&mod, &spec, "fn (%T)"), WAH_ERROR_BAD_SPEC);
    assert_err(wah_debug_parse_export_func_type_spec(&mod, &spec, "fn (ref %T)"), WAH_ERROR_BAD_SPEC);

    assert_err(wah_debug_parse_define_type_spec(&mod, &spec, "fn (%T)", wah_debug_type_from_idx(0, false)), WAH_ERROR_BAD_SPEC);
    assert_ok(wah_define_type(&mod, &struct_type, "struct { i32 }"));
    assert_ok(wah_debug_parse_define_type_spec(&mod, &spec, "fn (%T) -> ref null %T", struct_type, struct_type));
    assert_func_counts(&spec, 1, 1);
    assert_eq_i32(spec.param_types[0], struct_type);
    assert_eq_i32(spec.result_types[0], wah_debug_type_from_idx(wah_debug_typidx(struct_type), true));
    wah_debug_free_type_spec(&spec);

    wah_free_module(&mod);
}

static void test_type_reuse(void) {
    printf("Testing type reuse and fresh...\n");
    wah_module_t mod = {0};
    wah_type_t t1, t2, t3, t4;
    assert_ok(wah_new_module(&mod, NULL));

    // Same function type should reuse
    assert_ok(wah_define_type(&mod, &t1, "fn (i32, i32) -> (i32)"));
    assert_ok(wah_define_type(&mod, &t2, "fn (i32, i32) -> (i32)"));
    assert_eq_i32(t1, t2);

    // Different function type should not reuse
    assert_ok(wah_define_type(&mod, &t3, "fn (i32) -> (i32)"));
    assert_true(t1 != t3);

    // fresh should always create a new type
    assert_ok(wah_define_type(&mod, &t4, "fresh fn (i32, i32) -> (i32)"));
    assert_true(t1 != t4);

    // Reuse should still find t1, not t4, since t1 comes first
    wah_type_t t5;
    assert_ok(wah_define_type(&mod, &t5, "fn (i32, i32) -> (i32)"));
    assert_eq_i32(t1, t5);

    // fresh struct
    wah_type_t s1, s2, s3;
    assert_ok(wah_define_type(&mod, &s1, "struct { i32, i64 }"));
    assert_ok(wah_define_type(&mod, &s2, "struct { i32, i64 }"));
    assert_eq_i32(s1, s2);
    assert_ok(wah_define_type(&mod, &s3, "fresh struct { i32, i64 }"));
    assert_true(s1 != s3);

    // Struct with different mutability should not reuse
    wah_type_t s4;
    assert_ok(wah_define_type(&mod, &s4, "struct { mut i32, i64 }"));
    assert_true(s1 != s4);

    // fresh array
    wah_type_t a1, a2, a3;
    assert_ok(wah_define_type(&mod, &a1, "array i32"));
    assert_ok(wah_define_type(&mod, &a2, "array i32"));
    assert_eq_i32(a1, a2);
    assert_ok(wah_define_type(&mod, &a3, "fresh array i32"));
    assert_true(a1 != a3);

    // Type referring to another type via %T should reuse correctly
    wah_type_t r1, r2, r3;
    assert_ok(wah_define_type(&mod, &r1, "fn (ref %T) -> ()", s1));
    assert_ok(wah_define_type(&mod, &r2, "fn (ref %T) -> ()", s1));
    assert_eq_i32(r1, r2);
    // Same shape but different referenced type should not reuse
    assert_ok(wah_define_type(&mod, &r3, "fn (ref %T) -> ()", s3));
    assert_true(r1 != r3);

    wah_free_module(&mod);
}

int main(void) {
    test_function_specs();
    test_struct_and_array_specs();
    test_edge_cases();
    test_placeholders();
    test_type_reuse();
    printf("All type spec tests passed!\n");
    return 0;
}
