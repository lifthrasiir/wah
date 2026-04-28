#include "common.h"
#include <stdio.h>

static void test_local_tag_introspection(void) {
    printf("Testing local tag introspection...\n");

    const char *spec = "wasm \
        types {[ fn [i32, funcref] [] ]} \
        tags {[ tag.type# 0 ]} \
        exports {[ {'tag'} export.tag 0 ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));

    assert_eq_u32(wah_module_tag_count(NULL), 0);
    assert_eq_u32(wah_module_tag_count(&module), 1);

    wah_tag_desc_t tag = {0};
    assert_err(wah_module_tag(NULL, 0, &tag), WAH_ERROR_MISUSE);
    assert_err(wah_module_tag(&module, 0, NULL), WAH_ERROR_MISUSE);
    assert_err(wah_module_tag(&module, 1, &tag), WAH_ERROR_NOT_FOUND);

    assert_ok(wah_module_tag(&module, 0, &tag));
    assert_eq_u32(tag.type_index, 0);
    assert_eq_u32(tag.param_count, 2);
    assert_eq_i32(tag.param_types[0], WAH_TYPE_I32);
    assert_eq_i32(tag.param_types[1], WAH_TYPE_FUNCREF);

    wah_export_desc_t export_desc = {0};
    assert_ok(wah_module_export(&module, 0, &export_desc));
    assert_eq_i32(export_desc.kind, WAH_KIND_TAG);
    assert_eq_u32(export_desc.index, 0);
    assert_eq_str(export_desc.name, "tag");
    assert_eq_u32((uint32_t)export_desc.name_len, 3);
    assert_eq_u32(export_desc.u.tag.type_index, 0);
    assert_eq_u32(export_desc.u.tag.param_count, 2);
    assert_eq_i32(export_desc.u.tag.param_types[0], WAH_TYPE_I32);
    assert_eq_i32(export_desc.u.tag.param_types[1], WAH_TYPE_FUNCREF);

    wah_export_desc_t named_export = {0};
    assert_ok(wah_module_export_by_name(&module, "tag", &named_export));
    assert_eq_i32(named_export.kind, WAH_KIND_TAG);
    assert_eq_u32(named_export.u.tag.type_index, 0);

    wah_free_module(&module);
}

static void test_imported_and_local_tag_introspection(void) {
    printf("Testing imported and local tag introspection...\n");

    const char *spec = "wasm \
        types {[ fn [i32] [], fn [i64] [] ]} \
        imports {[ {'env'} {'imported'} tag# tag.type# 0 ]} \
        tags {[ tag.type# 1 ]} \
        exports {[ {'imported'} export.tag 0, {'local'} export.tag 1 ]}";

    wah_module_t module = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));

    assert_eq_u32(wah_module_import_count(&module), 1);
    assert_eq_u32(wah_module_export_count(&module), 2);
    assert_eq_u32(wah_module_tag_count(&module), 2);

    wah_import_desc_t import_desc = {0};
    assert_ok(wah_module_import(&module, 0, &import_desc));
    assert_eq_i32(import_desc.kind, WAH_KIND_TAG);
    assert_eq_u32(import_desc.index, 0);
    assert_eq_str(import_desc.module, "env");
    assert_eq_u32((uint32_t)import_desc.module_len, 3);
    assert_eq_str(import_desc.name, "imported");
    assert_eq_u32((uint32_t)import_desc.name_len, 8);
    assert_eq_u32(import_desc.u.tag.type_index, 0);
    assert_eq_u32(import_desc.u.tag.param_count, 1);
    assert_eq_i32(import_desc.u.tag.param_types[0], WAH_TYPE_I32);

    wah_tag_desc_t imported_tag = {0};
    assert_ok(wah_module_tag(&module, 0, &imported_tag));
    assert_eq_u32(imported_tag.type_index, 0);
    assert_eq_u32(imported_tag.param_count, 1);
    assert_eq_i32(imported_tag.param_types[0], WAH_TYPE_I32);

    wah_tag_desc_t local_tag = {0};
    assert_ok(wah_module_tag(&module, 1, &local_tag));
    assert_eq_u32(local_tag.type_index, 1);
    assert_eq_u32(local_tag.param_count, 1);
    assert_eq_i32(local_tag.param_types[0], WAH_TYPE_I64);

    wah_export_desc_t imported_export = {0};
    assert_ok(wah_module_export_by_name(&module, "imported", &imported_export));
    assert_eq_i32(imported_export.kind, WAH_KIND_TAG);
    assert_eq_u32(imported_export.index, 0);
    assert_eq_u32(imported_export.u.tag.type_index, 0);
    assert_eq_i32(imported_export.u.tag.param_types[0], WAH_TYPE_I32);

    wah_export_desc_t local_export = {0};
    assert_ok(wah_module_export_by_name(&module, "local", &local_export));
    assert_eq_i32(local_export.kind, WAH_KIND_TAG);
    assert_eq_u32(local_export.index, 1);
    assert_eq_u32(local_export.u.tag.type_index, 1);
    assert_eq_i32(local_export.u.tag.param_types[0], WAH_TYPE_I64);

    wah_free_module(&module);
}

int main(void) {
    test_local_tag_introspection();
    test_imported_and_local_tag_introspection();

    printf("\n--- ALL TESTS PASSED ---\n");
    return 0;
}
