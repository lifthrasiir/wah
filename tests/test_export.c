#include "../wah.h"
#include "wah_impl.h"
#include "common.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

// --- Test Cases ---

void test_basic_exports() {
    printf("Running test_basic_exports...\n");

    const char *module_spec = "wasm \
        types {[ fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        memories {[ limits.i32/1 1 ]} \
        globals {[ i32 mut i32.const 0 end ]} \
        exports {[ \
            {'add'} fn# 0, \
            {'g'} global# 0, \
            {'mem'} mem# 0, \
            {'tbl'} table# 0, \
        ]} \
        code {[ {[] local.get 0 local.get 1 i32.add end } ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, module_spec));

    assert_eq_u32(wah_module_export_count(&module), 4);
    assert_eq_u32(wah_module_type_count(&module), 1);
    assert_eq_u32(wah_module_function_count(&module), 1);
    assert_eq_u32(wah_module_table_count(&module), 1);
    assert_eq_u32(wah_module_memory_count(&module), 1);
    assert_eq_u32(wah_module_global_count(&module), 1);

    wah_export_desc_t entry;
    wah_type_desc_t type_desc;
    assert_ok(wah_module_type(&module, 0, &type_desc));
    assert_eq_i32(type_desc.kind, WAH_COMP_FUNC);
    assert_eq_u32(type_desc.param_count, 2);
    assert_eq_i32(type_desc.param_types[0], WAH_TYPE_I32);
    assert_eq_i32(type_desc.param_types[1], WAH_TYPE_I32);
    assert_eq_u32(type_desc.result_count, 1);
    assert_eq_i32(type_desc.result_types[0], WAH_TYPE_I32);

    wah_func_desc_t func_desc;
    assert_ok(wah_module_function(&module, 0, &func_desc));
    assert_false(func_desc.is_import);
    assert_false(func_desc.is_host);
    assert_eq_u32(func_desc.type_index, 0);
    assert_eq_u32(func_desc.param_count, 2);

    wah_table_desc_t table_desc;
    assert_ok(wah_module_table(&module, 0, &table_desc));
    assert_eq_i32(table_desc.elem_type, WAH_TYPE_FUNCREF);
    assert_eq_u64(table_desc.min_elements, 1);
    assert_eq_u64(table_desc.max_elements, UINT64_MAX);

    wah_memory_desc_t memory_desc;
    assert_ok(wah_module_memory(&module, 0, &memory_desc));
    assert_eq_i32(memory_desc.addr_type, WAH_TYPE_I32);
    assert_eq_u64(memory_desc.min_pages, 1);
    assert_eq_u64(memory_desc.max_pages, UINT64_MAX);

    wah_global_desc_t global_desc;
    assert_ok(wah_module_global(&module, 0, &global_desc));
    assert_eq_i32(global_desc.type, WAH_TYPE_I32);
    assert_true(global_desc.is_mutable);

    // Test export by index
    assert_ok(wah_module_export(&module, 0, &entry));
    assert_eq_i32(entry.kind, WAH_KIND_FUNCTION);
    assert_eq_u32(entry.index, 0);
    assert_eq_str(entry.name, "add");
    assert_eq_u64(entry.name_len, 3);

    assert_ok(wah_module_export(&module, 1, &entry));
    assert_eq_i32(entry.kind, WAH_KIND_GLOBAL);
    assert_eq_i32(entry.u.global.type, WAH_TYPE_I32);
    assert_eq_u32(entry.index, 0);
    assert_eq_str(entry.name, "g");
    assert_eq_u64(entry.name_len, 1);

    assert_ok(wah_module_export(&module, 2, &entry));
    assert_eq_i32(entry.kind, WAH_KIND_MEMORY);
    assert_eq_u32(entry.index, 0);
    assert_eq_str(entry.name, "mem");
    assert_eq_u64(entry.name_len, 3);

    assert_ok(wah_module_export(&module, 3, &entry));
    assert_eq_i32(entry.kind, WAH_KIND_TABLE);
    assert_eq_u32(entry.index, 0);
    assert_eq_str(entry.name, "tbl");
    assert_eq_u64(entry.name_len, 3);

    // Test export by name
    assert_ok(wah_module_export_by_name(&module, "add", &entry));
    assert_eq_i32(entry.kind, WAH_KIND_FUNCTION);
    assert_eq_str(entry.name, "add");

    assert_ok(wah_module_export_by_name(&module, "g", &entry));
    assert_eq_i32(entry.kind, WAH_KIND_GLOBAL);
    assert_eq_i32(entry.u.global.type, WAH_TYPE_I32);
    assert_eq_str(entry.name, "g");

    assert_err(wah_module_export_by_name(&module, "nonexistent", &entry), WAH_ERROR_NOT_FOUND);

    // Test per-kind descriptor for exported function
    wah_func_desc_t func;
    assert_ok(wah_module_function(&module, 0, &func));
    assert_eq_u32(func.param_count, 2);
    assert_eq_u32(func.result_count, 1);

    wah_free_module(&module);
}

void test_import_descriptors() {
    printf("Running test_import_descriptors...\n");

    const char *module_spec = "wasm \
        types {[ fn [] [i32] ]} \
        imports {[ \
            {'env'} {'get'} fn# 0, \
            {'env'} {'tbl'} export.table funcref limits.i32/1 1, \
            {'env'} {'mem'} export.memory limits.i32/2 1 2, \
            {'env'} {'g'} export.global i32 immut \
        ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, module_spec));

    assert_eq_u32(wah_module_import_count(&module), 4);
    assert_eq_u32(wah_module_function_count(&module), 1);
    assert_eq_u32(wah_module_table_count(&module), 1);
    assert_eq_u32(wah_module_memory_count(&module), 1);
    assert_eq_u32(wah_module_global_count(&module), 1);

    wah_import_desc_t import_desc;
    assert_ok(wah_module_import(&module, 0, &import_desc));
    assert_eq_i32(import_desc.kind, 0);
    assert_eq_u32(import_desc.index, 0);
    assert_eq_u64(import_desc.module_len, 3);
    assert_true(memcmp(import_desc.module, "env", 3) == 0);
    assert_eq_u64(import_desc.name_len, 3);
    assert_true(memcmp(import_desc.name, "get", 3) == 0);
    assert_eq_u32(import_desc.u.func.result_count, 1);
    assert_eq_i32(import_desc.u.func.result_types[0], WAH_TYPE_I32);

    assert_ok(wah_module_import(&module, 1, &import_desc));
    assert_eq_i32(import_desc.kind, 1);
    assert_eq_i32(import_desc.u.table.elem_type, WAH_TYPE_FUNCREF);

    assert_ok(wah_module_import(&module, 2, &import_desc));
    assert_eq_i32(import_desc.kind, 2);
    assert_eq_u64(import_desc.u.memory.min_pages, 1);
    assert_eq_u64(import_desc.u.memory.max_pages, 2);

    assert_ok(wah_module_import(&module, 3, &import_desc));
    assert_eq_i32(import_desc.kind, 3);
    assert_eq_i32(import_desc.u.global.type, WAH_TYPE_I32);
    assert_false(import_desc.u.global.is_mutable);

    wah_free_module(&module);
}

void test_duplicate_export_names() {
    printf("Running test_duplicate_export_names...\n");

    const char *module_spec = "wasm \
        types {[ fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        exports {[ \
            {'add'} fn# 0, \
            {'add'} fn# 0, \
        ]} \
        code {[ {[] local.get 0 local.get 1 i32.add end } ]}";

    // Export "add" twice
    wah_module_t module;
    assert_err(wah_parse_module_from_spec(&module, module_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module); // Should be safe to call even if parsing failed
}

void test_invalid_export_kind_or_index() {
    printf("Running test_invalid_export_kind_or_index...\n");

    // Test 1: Invalid export kind (e.g., 0x04 which is not defined)
    const char *module_spec_invalid_kind = "wasm \
        types {[ fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        exports {[ {'bad'} 4 0 ]} \
        code {[ {[] local.get 0 local.get 1 i32.add end } ]}";
    wah_module_t module_invalid_kind;
    assert_err(wah_parse_module_from_spec(&module_invalid_kind, module_spec_invalid_kind), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module_invalid_kind);

    // Test 2: Export function with out-of-bounds index
    const char *module_spec_invalid_func_idx = "wasm \
        types {[ fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        exports {[ {'bad_func'} fn# 1 ]} \
        code {[ {[] local.get 0 local.get 1 i32.add end } ]}";
    wah_module_t module_invalid_func_idx;
    assert_err(wah_parse_module_from_spec(&module_invalid_func_idx, module_spec_invalid_func_idx), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&module_invalid_func_idx);
}

void test_non_utf8_export_name() {
    printf("Running test_non_utf8_export_name...\n");

    // Export name with invalid UTF-8 sequence (e.g., 0xFF)
    const char *module_spec_non_utf8 = "wasm \
        types {[ fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        exports {[ {'bad' %'ff' 'name'} fn# 0 ]} \
        code {[ {[] local.get 0 local.get 1 i32.add end } ]}";
    wah_module_t module;
    assert_err(wah_parse_module_from_spec(&module, module_spec_non_utf8), WAH_ERROR_MALFORMED);
    wah_free_module(&module);
}

void test_module_no_exports() {
    printf("Running test_module_no_exports...\n");

    const char *module_spec_no_exports = "wasm \
        types {[ fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] local.get 0 local.get 1 i32.add end } ]}";

    // Module with no export section
    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, module_spec_no_exports));

    assert_eq_u32(wah_module_export_count(&module), 0);

    wah_export_desc_t entry;
    assert_err(wah_module_export(&module, 0, &entry), WAH_ERROR_NOT_FOUND);
    assert_err(wah_module_export_by_name(&module, "any", &entry), WAH_ERROR_NOT_FOUND);

    wah_free_module(&module);
}

void test_wah_module_function_non_exported() {
    printf("Running test_wah_module_function_non_exported...\n");
    // Test per-kind descriptor for a function that is not exported

    const char *module_spec_no_exports = "wasm \
        types {[ fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] local.get 0 local.get 1 i32.add end } ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, module_spec_no_exports));

    wah_func_desc_t func;
    assert_ok(wah_module_function(&module, 0, &func));
    assert_eq_u32(func.param_count, 2);
    assert_eq_u32(func.result_count, 1);

    wah_free_module(&module);
}

void test_wah_module_function_invalid_id() {
    printf("Running test_wah_module_function_invalid_id...\n");

    const char *module_spec_invalid_id = "wasm \
        types {[ fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] local.get 0 local.get 1 i32.add end } ]}";

    // Test wah_module_entry with invalid entry IDs
    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, module_spec_invalid_id));

    wah_func_desc_t func;
    assert_err(wah_module_function(&module, 999, &func), WAH_ERROR_NOT_FOUND);

    wah_free_module(&module);
}

void test_export_name_with_null_byte() {
    printf("Running test_export_name_with_null_byte...\n");

    const char *module_spec_null_byte = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        exports {[ {'bad' %'00' 'name'} fn# 0 ]} \
        code {[ {[] end } ]}";

    // Export name: "bad\x00name" (length 8)
    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, module_spec_null_byte));

    assert_eq_u32(wah_module_export_count(&module), 1);

    wah_export_desc_t entry;

    // Verify by index
    assert_ok(wah_module_export(&module, 0, &entry));
    assert_eq_i32(entry.kind, WAH_KIND_FUNCTION);
    assert_eq_u32(entry.index, 0);
    assert_eq_u64(entry.name_len, 8);
    assert_true(memcmp(entry.name, "bad\0name", 8) == 0);

    // Attempt lookup by "bad" (shorter, stops at null)
    assert_err(wah_module_export_by_name(&module, "bad", &entry), WAH_ERROR_NOT_FOUND);

    // Attempt lookup by "bad\0name" (exact length, but strlen stops at null)
    char lookup_name_with_null[] = {'b', 'a', 'd', 0x00, 'n', 'a', 'm', 'e', '\0'};
    assert_err(wah_module_export_by_name(&module, lookup_name_with_null, &entry), WAH_ERROR_NOT_FOUND);

    // ec03097: wah_module_export_by_name_len should find it with explicit length
    assert_ok(wah_module_export_by_name_len(&module, "bad\0name", 8, &entry));
    assert_eq_i32(entry.kind, WAH_KIND_FUNCTION);
    assert_eq_u64(entry.name_len, 8);

    // Wrong length should not match
    assert_err(wah_module_export_by_name_len(&module, "bad", 3, &entry), WAH_ERROR_NOT_FOUND);

    wah_free_module(&module);
}

int main() {
    test_basic_exports();
    test_import_descriptors();
    test_duplicate_export_names();
    test_invalid_export_kind_or_index();
    test_non_utf8_export_name();
    test_module_no_exports();
    test_wah_module_function_non_exported();
    test_wah_module_function_invalid_id();
    test_export_name_with_null_byte();

    printf("All export tests passed!\n");
    return 0;
}
