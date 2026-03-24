// Test for wah_module_export_memory and wah_module_export_global_* functions

#define WAH_IMPLEMENTATION
#include "wah.h"
#include "wah_testutils.c"
#include <stdio.h>
#include <string.h>

int main(void) {
    // Test 1: Export memory
    printf("Testing wah_module_export_memory...\n");
    {
        wah_module_t mod = {0};
        assert_ok(wah_new_module(&mod));
        assert_ok(wah_module_export_memory(&mod, "memory", 1, 10));

        // Verify export
        wah_entry_t entry = {0};
        assert_ok(wah_module_export_by_name(&mod, "memory", &entry));
        assert_true(WAH_TYPE_IS_MEMORY(entry.type));
        assert_eq_u32(mod.memories[0].min_pages, 1);
        assert_eq_u32(mod.memories[0].max_pages, 10);

        wah_free_module(&mod);
    }

    // Test 2: Export immutable i32 global
    printf("Testing wah_module_export_global_i32 (immutable)...\n");
    {
        wah_module_t mod = {0};
        assert_ok(wah_new_module(&mod));
        assert_ok(wah_module_export_global_i32(&mod, "const_i32", false, 42));

        // Verify export
        wah_entry_t entry = {0};
        assert_ok(wah_module_export_by_name(&mod, "const_i32", &entry));
        assert_true(WAH_TYPE_IS_GLOBAL(entry.type));
        assert_eq_i32(entry.type, WAH_TYPE_I32);
        assert_eq_i32(mod.globals[0].type, WAH_TYPE_I32);
        assert_false(mod.globals[0].is_mutable);

        // Verify initial value through execution context
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &mod));

        assert_ok(wah_instantiate(&ctx));
        assert_eq_i32(ctx.globals[0].i32, 42);

        wah_exec_context_destroy(&ctx);
        wah_free_module(&mod);
    }

    // Test 3: Export mutable i32 global
    printf("Testing wah_module_export_global_i32 (mutable)...\n");
    {
        wah_module_t mod = {0};
        assert_ok(wah_new_module(&mod));
        assert_ok(wah_module_export_global_i32(&mod, "mut_i32", true, -100));

        assert_true(mod.globals[0].is_mutable);

        // Verify initial value through execution context
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &mod));

        assert_ok(wah_instantiate(&ctx));
        assert_eq_i32(ctx.globals[0].i32, -100);

        wah_exec_context_destroy(&ctx);
        wah_free_module(&mod);
    }

    // Test 4: Export i64 global
    printf("Testing wah_module_export_global_i64...\n");
    {
        wah_module_t mod = {0};
        assert_ok(wah_new_module(&mod));

        int64_t test_value = 0x123456789ABCDEF0LL;
        assert_ok(wah_module_export_global_i64(&mod, "const_i64", false, test_value));
        assert_eq_i32(mod.globals[0].type, WAH_TYPE_I64);

        // Verify initial value through execution context
        wah_exec_context_t ctx;
        assert_ok(wah_exec_context_create(&ctx, &mod));

        assert_ok(wah_instantiate(&ctx));
        assert_eq_i64(ctx.globals[0].i64, test_value);

        wah_exec_context_destroy(&ctx);
        wah_free_module(&mod);
    }

    // Test 5: Export f32 global
    printf("Testing wah_module_export_global_f32...\n");
    {
        wah_module_t mod = {0};
        assert_ok(wah_new_module(&mod));

        float test_value = 3.14159f;
        assert_ok(wah_module_export_global_f32(&mod, "const_f32", false, test_value));
        assert_eq_i32(mod.globals[0].type, WAH_TYPE_F32);

        wah_free_module(&mod);
    }

    // Test 6: Export f64 global
    printf("Testing wah_module_export_global_f64...\n");
    {
        wah_module_t mod = {0};
        assert_ok(wah_new_module(&mod));

        double test_value = 2.718281828459045;
        assert_ok(wah_module_export_global_f64(&mod, "const_f64", false, test_value));
        assert_eq_i32(mod.globals[0].type, WAH_TYPE_F64);

        wah_free_module(&mod);
    }

    // Test 7: Export v128 global
    printf("Testing wah_module_export_global_v128...\n");
    {
        wah_module_t mod = {0};
        assert_ok(wah_new_module(&mod));

        wah_v128_t test_value = {{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}};
        assert_ok(wah_module_export_global_v128(&mod, "const_v128", false, &test_value));
        assert_eq_i32(mod.globals[0].type, WAH_TYPE_V128);

        wah_free_module(&mod);
    }

    // Test 8: Multiple exports
    printf("Testing multiple exports...\n");
    {
        wah_module_t mod = {0};
        assert_ok(wah_new_module(&mod));

        assert_ok(wah_module_export_memory(&mod, "mem1", 1, 1));
        assert_ok(wah_module_export_global_i32(&mod, "g1", false, 1));
        assert_ok(wah_module_export_global_i64(&mod, "g2", true, 2));

        assert_eq_u32(mod.export_count, 3);
        assert_eq_u32(mod.memory_count, 1);
        assert_eq_u32(mod.global_count, 2);

        wah_free_module(&mod);
    }

    // Test 9: Duplicate export name should fail
    printf("Testing duplicate export name...\n");
    {
        wah_module_t mod = {0};
        assert_ok(wah_new_module(&mod));

        assert_ok(wah_module_export_global_i32(&mod, "dup", false, 1));
        assert_err(wah_module_export_global_i32(&mod, "dup", false, 2), WAH_ERROR_VALIDATION_FAILED);

        wah_free_module(&mod);
    }

    printf("\n--- ALL TESTS PASSED ---\n");
    return 0;
}
