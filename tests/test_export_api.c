// Test for wah_module_export_memory and wah_module_export_global_* functions

#define WAH_IMPLEMENTATION
#include "../wah.h"
#include "common.c"
#include <stdio.h>
#include <string.h>

static bool flags_checked = false;
static void check_flags_host(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    assert_not_null(ctx->param_type_flags);
    assert_not_null(ctx->result_type_flags);
    assert_true(ctx->param_type_flags[0] & WAH_TYPE_FLAG_NULLABLE);
    assert_true(ctx->param_type_flags[1] == 0);
    assert_true(ctx->result_type_flags[0] == 0);
    flags_checked = true;
    wah_return_i32(ctx, 1);
}

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
        assert_eq_u64(mod.memories[0].min_pages, 1);
        assert_eq_u64(mod.memories[0].max_pages, 10);

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

    // Test: Host function type flags are exposed through wah_entry_t and wah_call_context_t
    printf("Testing host function type flags exposure...\n");
    {
        wah_module_t host_mod = {0};
        assert_ok(wah_new_module(&host_mod));
        wah_type_t ptypes[2] = { WAH_TYPE_FUNCREF, WAH_TYPE_I32 };
        wah_type_t rtypes[1] = { WAH_TYPE_I32 };
        assert_ok(wah_module_export_funcv(&host_mod, "check", 2, ptypes, 1, rtypes,
                                          check_flags_host, NULL, NULL));

        // Verify flags through wah_entry_t
        wah_entry_t entry;
        assert_ok(wah_module_export_by_name(&host_mod, "check", &entry));
        assert_not_null(entry.u.func.param_type_flags);
        assert_not_null(entry.u.func.result_type_flags);
        assert_true(entry.u.func.param_type_flags[0] & WAH_TYPE_FLAG_NULLABLE);
        assert_true(entry.u.func.param_type_flags[1] == 0);
        assert_true(entry.u.func.result_type_flags[0] == 0);

        // Call host function to verify flags in wah_call_context_t
        const char *spec = "wasm \
            types {[ fn [funcref, i32] [i32], fn [] [i32] ]} \
            imports {[ {'env'} {'check'} fn# 0 ]} \
            funcs {[ 1 ]} \
            exports {[ {'f'} fn# 1 ]} \
            code {[ {[] ref.null funcref i32.const 0 call 0 end } ]}";
        wah_module_t wasm_mod = {0};
        assert_ok(wah_parse_module_from_spec(&wasm_mod, spec));

        wah_exec_context_t ctx = {0};
        assert_ok(wah_exec_context_create(&ctx, &wasm_mod));
        assert_ok(wah_link_module(&ctx, "env", &host_mod));
        assert_ok(wah_instantiate(&ctx));

        wah_value_t result;
        assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
        assert_eq_i32(result.i32, 1);
        assert_true(flags_checked);

        wah_exec_context_destroy(&ctx);
        wah_free_module(&wasm_mod);
        wah_free_module(&host_mod);
    }

    printf("\n--- ALL TESTS PASSED ---\n");
    return 0;
}
