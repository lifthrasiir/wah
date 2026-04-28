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
    assert_ok(wah_new_module(&mod));

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
    assert_ok(wah_new_module(&mod));

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
    assert_ok(wah_new_module(&mod));

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
    assert_ok(wah_new_module(&mod));

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

    assert_err(wah_debug_parse_define_type_spec(&mod, &spec, "fn (%T)", WAH_TYPE_FROM_IDX(0, false)), WAH_ERROR_BAD_SPEC);
    assert_ok(wah_module_define_type(&mod, &struct_type, "struct { i32 }"));
    assert_ok(wah_debug_parse_define_type_spec(&mod, &spec, "fn (%T) -> ref null %T", struct_type, struct_type));
    assert_func_counts(&spec, 1, 1);
    assert_eq_i32(spec.param_types[0], struct_type);
    assert_eq_i32(spec.result_types[0], WAH_TYPE_FROM_IDX(WAH_TYIDX(struct_type), true));
    wah_debug_free_type_spec(&spec);

    wah_free_module(&mod);
}

int main(void) {
    test_function_specs();
    test_struct_and_array_specs();
    test_edge_cases();
    test_placeholders();
    printf("All type spec tests passed!\n");
    return 0;
}
