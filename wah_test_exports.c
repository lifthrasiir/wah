// Test for wah_module_export_memory and wah_module_export_global_* functions

#define WAH_IMPLEMENTATION
#include "wah.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    printf("Testing module export functionality...\n\n");

    // Test 1: Export memory
    printf("--- Testing wah_module_export_memory ---\n");
    {
        wah_module_t mod = {0};
        wah_error_t err = wah_new_module(&mod);
        if (err != WAH_OK) {
            printf("FAIL: wah_new_module returned %s\n", wah_strerror(err));
            return 1;
        }

        err = wah_module_export_memory(&mod, "memory", 1, 10);
        if (err != WAH_OK) {
            printf("FAIL: wah_module_export_memory returned %s\n", wah_strerror(err));
            wah_free_module(&mod);
            return 1;
        }

        // Verify export
        wah_entry_t entry = {0};
        err = wah_module_export_by_name(&mod, "memory", &entry);
        if (err != WAH_OK) {
            printf("FAIL: wah_module_export_by_name returned %s\n", wah_strerror(err));
            wah_free_module(&mod);
            return 1;
        }

        if (!WAH_TYPE_IS_MEMORY(entry.type)) {
            printf("FAIL: Expected type to be memory, got %d\n", entry.type);
            wah_free_module(&mod);
            return 1;
        }

        if (mod.memories[0].min_pages != 1 || mod.memories[0].max_pages != 10) {
            printf("FAIL: Memory bounds not set correctly\n");
            wah_free_module(&mod);
            return 1;
        }

        printf("PASS: wah_module_export_memory\n");
        wah_free_module(&mod);
    }

    // Test 2: Export immutable i32 global
    printf("\n--- Testing wah_module_export_global_i32 (immutable) ---\n");
    {
        wah_module_t mod = {0};
        wah_error_t err = wah_new_module(&mod);
        if (err != WAH_OK) {
            printf("FAIL: wah_new_module returned %s\n", wah_strerror(err));
            return 1;
        }

        err = wah_module_export_global_i32(&mod, "const_i32", false, 42);
        if (err != WAH_OK) {
            printf("FAIL: wah_module_export_global_i32 returned %s\n", wah_strerror(err));
            wah_free_module(&mod);
            return 1;
        }

        // Verify export
        wah_entry_t entry = {0};
        err = wah_module_export_by_name(&mod, "const_i32", &entry);
        if (err != WAH_OK) {
            printf("FAIL: wah_module_export_by_name returned %s\n", wah_strerror(err));
            wah_free_module(&mod);
            return 1;
        }

        if (!WAH_TYPE_IS_GLOBAL(entry.type)) {
            printf("FAIL: Expected type to be global, got %d\n", entry.type);
            wah_free_module(&mod);
            return 1;
        }

        if (mod.globals[0].type != WAH_TYPE_I32 || mod.globals[0].is_mutable) {
            printf("FAIL: Global type or mutability not set correctly\n");
            wah_free_module(&mod);
            return 1;
        }

        if (mod.globals[0].initial_value.i32 != 42) {
            printf("FAIL: Global initial value not set correctly\n");
            wah_free_module(&mod);
            return 1;
        }

        printf("PASS: wah_module_export_global_i32 (immutable)\n");
        wah_free_module(&mod);
    }

    // Test 3: Export mutable i32 global
    printf("\n--- Testing wah_module_export_global_i32 (mutable) ---\n");
    {
        wah_module_t mod = {0};
        wah_error_t err = wah_new_module(&mod);
        if (err != WAH_OK) {
            printf("FAIL: wah_new_module returned %s\n", wah_strerror(err));
            return 1;
        }

        err = wah_module_export_global_i32(&mod, "mut_i32", true, -100);
        if (err != WAH_OK) {
            printf("FAIL: wah_module_export_global_i32 returned %s\n", wah_strerror(err));
            wah_free_module(&mod);
            return 1;
        }

        if (mod.globals[0].is_mutable != true) {
            printf("FAIL: Global mutability not set correctly\n");
            wah_free_module(&mod);
            return 1;
        }

        if (mod.globals[0].initial_value.i32 != -100) {
            printf("FAIL: Global initial value not set correctly\n");
            wah_free_module(&mod);
            return 1;
        }

        printf("PASS: wah_module_export_global_i32 (mutable)\n");
        wah_free_module(&mod);
    }

    // Test 4: Export i64 global
    printf("\n--- Testing wah_module_export_global_i64 ---\n");
    {
        wah_module_t mod = {0};
        wah_error_t err = wah_new_module(&mod);
        if (err != WAH_OK) {
            printf("FAIL: wah_new_module returned %s\n", wah_strerror(err));
            return 1;
        }

        int64_t test_value = 0x123456789ABCDEF0LL;
        err = wah_module_export_global_i64(&mod, "const_i64", false, test_value);
        if (err != WAH_OK) {
            printf("FAIL: wah_module_export_global_i64 returned %s\n", wah_strerror(err));
            wah_free_module(&mod);
            return 1;
        }

        if (mod.globals[0].type != WAH_TYPE_I64) {
            printf("FAIL: Global type not set correctly\n");
            wah_free_module(&mod);
            return 1;
        }

        if (mod.globals[0].initial_value.i64 != test_value) {
            printf("FAIL: Global initial value not set correctly\n");
            wah_free_module(&mod);
            return 1;
        }

        printf("PASS: wah_module_export_global_i64\n");
        wah_free_module(&mod);
    }

    // Test 5: Export f32 global
    printf("\n--- Testing wah_module_export_global_f32 ---\n");
    {
        wah_module_t mod = {0};
        wah_error_t err = wah_new_module(&mod);
        if (err != WAH_OK) {
            printf("FAIL: wah_new_module returned %s\n", wah_strerror(err));
            return 1;
        }

        float test_value = 3.14159f;
        err = wah_module_export_global_f32(&mod, "const_f32", false, test_value);
        if (err != WAH_OK) {
            printf("FAIL: wah_module_export_global_f32 returned %s\n", wah_strerror(err));
            wah_free_module(&mod);
            return 1;
        }

        if (mod.globals[0].type != WAH_TYPE_F32) {
            printf("FAIL: Global type not set correctly\n");
            wah_free_module(&mod);
            return 1;
        }

        printf("PASS: wah_module_export_global_f32\n");
        wah_free_module(&mod);
    }

    // Test 6: Export f64 global
    printf("\n--- Testing wah_module_export_global_f64 ---\n");
    {
        wah_module_t mod = {0};
        wah_error_t err = wah_new_module(&mod);
        if (err != WAH_OK) {
            printf("FAIL: wah_new_module returned %s\n", wah_strerror(err));
            return 1;
        }

        double test_value = 2.718281828459045;
        err = wah_module_export_global_f64(&mod, "const_f64", false, test_value);
        if (err != WAH_OK) {
            printf("FAIL: wah_module_export_global_f64 returned %s\n", wah_strerror(err));
            wah_free_module(&mod);
            return 1;
        }

        if (mod.globals[0].type != WAH_TYPE_F64) {
            printf("FAIL: Global type not set correctly\n");
            wah_free_module(&mod);
            return 1;
        }

        printf("PASS: wah_module_export_global_f64\n");
        wah_free_module(&mod);
    }

    // Test 7: Export v128 global
    printf("\n--- Testing wah_module_export_global_v128 ---\n");
    {
        wah_module_t mod = {0};
        wah_error_t err = wah_new_module(&mod);
        if (err != WAH_OK) {
            printf("FAIL: wah_new_module returned %s\n", wah_strerror(err));
            return 1;
        }

        wah_v128_t test_value = {{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}};
        err = wah_module_export_global_v128(&mod, "const_v128", false, &test_value);
        if (err != WAH_OK) {
            printf("FAIL: wah_module_export_global_v128 returned %s\n", wah_strerror(err));
            wah_free_module(&mod);
            return 1;
        }

        if (mod.globals[0].type != WAH_TYPE_V128) {
            printf("FAIL: Global type not set correctly\n");
            wah_free_module(&mod);
            return 1;
        }

        printf("PASS: wah_module_export_global_v128\n");
        wah_free_module(&mod);
    }

    // Test 8: Multiple exports
    printf("\n--- Testing multiple exports ---\n");
    {
        wah_module_t mod = {0};
        wah_error_t err = wah_new_module(&mod);
        if (err != WAH_OK) {
            printf("FAIL: wah_new_module returned %s\n", wah_strerror(err));
            return 1;
        }

        err = wah_module_export_memory(&mod, "mem1", 1, 0);
        if (err != WAH_OK) {
            printf("FAIL: wah_module_export_memory returned %s\n", wah_strerror(err));
            wah_free_module(&mod);
            return 1;
        }

        err = wah_module_export_global_i32(&mod, "g1", false, 1);
        if (err != WAH_OK) {
            printf("FAIL: wah_module_export_global_i32 returned %s\n", wah_strerror(err));
            wah_free_module(&mod);
            return 1;
        }

        err = wah_module_export_global_i64(&mod, "g2", true, 2);
        if (err != WAH_OK) {
            printf("FAIL: wah_module_export_global_i64 returned %s\n", wah_strerror(err));
            wah_free_module(&mod);
            return 1;
        }

        if (mod.export_count != 3) {
            printf("FAIL: Expected 3 exports, got %d\n", mod.export_count);
            wah_free_module(&mod);
            return 1;
        }

        if (mod.memory_count != 1) {
            printf("FAIL: Expected 1 memory, got %d\n", mod.memory_count);
            wah_free_module(&mod);
            return 1;
        }

        if (mod.global_count != 2) {
            printf("FAIL: Expected 2 globals, got %d\n", mod.global_count);
            wah_free_module(&mod);
            return 1;
        }

        printf("PASS: Multiple exports\n");
        wah_free_module(&mod);
    }

    // Test 9: Duplicate export name should fail
    printf("\n--- Testing duplicate export name ---\n");
    {
        wah_module_t mod = {0};
        wah_error_t err = wah_new_module(&mod);
        if (err != WAH_OK) {
            printf("FAIL: wah_new_module returned %s\n", wah_strerror(err));
            return 1;
        }

        err = wah_module_export_global_i32(&mod, "dup", false, 1);
        if (err != WAH_OK) {
            printf("FAIL: wah_module_export_global_i32 returned %s\n", wah_strerror(err));
            wah_free_module(&mod);
            return 1;
        }

        err = wah_module_export_global_i32(&mod, "dup", false, 2);
        if (err != WAH_ERROR_VALIDATION_FAILED) {
            printf("FAIL: Expected WAH_ERROR_VALIDATION_FAILED for duplicate name, got %s\n", wah_strerror(err));
            wah_free_module(&mod);
            return 1;
        }

        printf("PASS: Duplicate export name rejected\n");
        wah_free_module(&mod);
    }

    printf("\n--- ALL TESTS PASSED ---\n");
    return 0;
}
