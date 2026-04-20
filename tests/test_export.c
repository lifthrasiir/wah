#define WAH_IMPLEMENTATION
#include "../wah.h"
#include "common.c"
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

    assert_eq_u64(wah_module_num_exports(&module), 4);

    wah_entry_t entry;

    // Test export by index
    assert_ok(wah_module_export(&module, 0, &entry));
    assert_true(WAH_TYPE_IS_FUNCTION(entry.type));
    assert_eq_i32(WAH_GET_ENTRY_KIND(entry.id), WAH_ENTRY_KIND_FUNCTION);
    assert_eq_i32(WAH_GET_ENTRY_INDEX(entry.id), 0);
    assert_eq_str(entry.name, "add");
    assert_eq_u64(entry.name_len, 3);

    assert_ok(wah_module_export(&module, 1, &entry));
    assert_eq_i32(entry.type, WAH_TYPE_I32);
    assert_true(WAH_TYPE_IS_GLOBAL(entry.type));
    assert_eq_i32(WAH_GET_ENTRY_KIND(entry.id), WAH_ENTRY_KIND_GLOBAL);
    assert_eq_i32(WAH_GET_ENTRY_INDEX(entry.id), 0);
    assert_eq_str(entry.name, "g");
    assert_eq_u64(entry.name_len, 1);

    assert_ok(wah_module_export(&module, 2, &entry));
    assert_true(WAH_TYPE_IS_MEMORY(entry.type));
    assert_eq_i32(WAH_GET_ENTRY_KIND(entry.id), WAH_ENTRY_KIND_MEMORY);
    assert_eq_i32(WAH_GET_ENTRY_INDEX(entry.id), 0);
    assert_eq_str(entry.name, "mem");
    assert_eq_u64(entry.name_len, 3);

    assert_ok(wah_module_export(&module, 3, &entry));
    assert_true(WAH_TYPE_IS_TABLE(entry.type));
    assert_eq_i32(WAH_GET_ENTRY_KIND(entry.id), WAH_ENTRY_KIND_TABLE);
    assert_eq_i32(WAH_GET_ENTRY_INDEX(entry.id), 0);
    assert_eq_str(entry.name, "tbl");
    assert_eq_u64(entry.name_len, 3);

    // Test export by name
    assert_ok(wah_module_export_by_name(&module, "add", &entry));
    assert_true(WAH_TYPE_IS_FUNCTION(entry.type));
    assert_eq_str(entry.name, "add");

    assert_ok(wah_module_export_by_name(&module, "g", &entry));
    assert_eq_i32(entry.type, WAH_TYPE_I32);
    assert_true(WAH_TYPE_IS_GLOBAL(entry.type));
    assert_eq_str(entry.name, "g");

    assert_err(wah_module_export_by_name(&module, "nonexistent", &entry), WAH_ERROR_NOT_FOUND);

    // Test wah_module_entry for exported function
    wah_entry_id_t func_id = WAH_MAKE_ENTRY_ID(WAH_ENTRY_KIND_FUNCTION, 0);
    assert_ok(wah_module_entry(&module, func_id, &entry));
    assert_true(WAH_TYPE_IS_FUNCTION(entry.type));
    assert_eq_u64(entry.id, func_id);
    assert_null(entry.name); // wah_module_entry doesn't return name for non-exported

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
    assert_err(wah_parse_module_from_spec(&module, module_spec_non_utf8), WAH_ERROR_MALFORMED_UTF8);
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

    assert_eq_u64(wah_module_num_exports(&module), 0);

    wah_entry_t entry;
    assert_err(wah_module_export(&module, 0, &entry), WAH_ERROR_NOT_FOUND);
    assert_err(wah_module_export_by_name(&module, "any", &entry), WAH_ERROR_NOT_FOUND);

    wah_free_module(&module);
}

void test_wah_module_entry_non_exported() {
    printf("Running test_wah_module_entry_non_exported...\n");
    // Test wah_module_entry for a function that is not exported

    const char *module_spec_no_exports = "wasm \
        types {[ fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] local.get 0 local.get 1 i32.add end } ]}";

    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, module_spec_no_exports));

    wah_entry_t entry;
    wah_entry_id_t func_id = WAH_MAKE_ENTRY_ID(WAH_ENTRY_KIND_FUNCTION, 0);
    assert_ok(wah_module_entry(&module, func_id, &entry));
    assert_true(WAH_TYPE_IS_FUNCTION(entry.type));
    assert_eq_u64(entry.id, func_id);
    assert_null(entry.name);
    assert_eq_u64(entry.name_len, 0);

    wah_free_module(&module);
}

void test_wah_module_entry_invalid_id() {
    printf("Running test_wah_module_entry_invalid_id...\n");

    const char *module_spec_invalid_id = "wasm \
        types {[ fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] local.get 0 local.get 1 i32.add end } ]}";

    // Test wah_module_entry with invalid entry IDs
    wah_module_t module;
    assert_ok(wah_parse_module_from_spec(&module, module_spec_invalid_id));

    wah_entry_t entry;

    // Invalid function index
    wah_entry_id_t invalid_func_id = WAH_MAKE_ENTRY_ID(WAH_ENTRY_KIND_FUNCTION, 999);
    assert_err(wah_module_entry(&module, invalid_func_id, &entry), WAH_ERROR_NOT_FOUND);

    // Invalid kind
    wah_entry_id_t invalid_kind_id = WAH_MAKE_ENTRY_ID(0xFF, 0); // 0xFF is an unknown kind
    assert_err(wah_module_entry(&module, invalid_kind_id, &entry), WAH_ERROR_NOT_FOUND);

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

    assert_eq_u64(wah_module_num_exports(&module), 1);

    wah_entry_t entry;

    // Verify by index
    assert_ok(wah_module_export(&module, 0, &entry));
    assert_true(WAH_TYPE_IS_FUNCTION(entry.type));
    assert_eq_i32(WAH_GET_ENTRY_KIND(entry.id), WAH_ENTRY_KIND_FUNCTION);
    assert_eq_i32(WAH_GET_ENTRY_INDEX(entry.id), 0);
    assert_eq_u64(entry.name_len, 8);
    assert_true(memcmp(entry.name, "bad\0name", 8) == 0);

    // Attempt lookup by "bad" (shorter, stops at null)
    assert_err(wah_module_export_by_name(&module, "bad", &entry), WAH_ERROR_NOT_FOUND);

    // Attempt lookup by "bad\0name" (exact length, but strlen stops at null)
    char lookup_name_with_null[] = {'b', 'a', 'd', 0x00, 'n', 'a', 'm', 'e', '\0'}; // Ensure it's null-terminated for strlen
    assert_err(wah_module_export_by_name(&module, lookup_name_with_null, &entry), WAH_ERROR_NOT_FOUND);

    wah_free_module(&module);
}

int main() {
    test_basic_exports();
    test_duplicate_export_names();
    test_invalid_export_kind_or_index();
    test_non_utf8_export_name();
    test_module_no_exports();
    test_wah_module_entry_non_exported();
    test_wah_module_entry_invalid_id();
    test_export_name_with_null_byte();

    printf("All export tests passed!\n");
    return 0;
}
