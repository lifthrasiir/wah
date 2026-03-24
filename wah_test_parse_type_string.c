// Test for wah_parse_func_type

#define WAH_IMPLEMENTATION
#include "wah.h"
#include "wah_testutils.c"
#include <stdio.h>
#include <string.h>
#include <assert.h>

void test_host_func(wah_call_context_t *ctx, void *userdata) {
    (void)ctx;
    (void)userdata;
    wah_return_i32(ctx, 42);
}

int main() {
    // Test 1: Simple i32 -> i32
    printf("Test 1: i32 -> i32\n");
    {
        const char *types = "i32 -> i32";
        size_t nparams, nresults;
        wah_type_t *param_types, *result_types;
        assert_ok(wah_parse_func_type(types, &nparams, &param_types, &nresults, &result_types));
        assert_eq_u64(nparams, 1);
        assert_eq_u64(nresults, 1);
        assert_eq_i32(param_types[0], WAH_TYPE_I32);
        assert_eq_i32(result_types[0], WAH_TYPE_I32);
        free(param_types);
        free(result_types);
    }

    // Test 2: Multiple types with commas
    printf("Test 2: i32, i64 -> f32, f64\n");
    {
        const char *types = "i32, i64 -> f32, f64";
        size_t nparams, nresults;
        wah_type_t *param_types, *result_types;
        assert_ok(wah_parse_func_type(types, &nparams, &param_types, &nresults, &result_types));
        assert_eq_u64(nparams, 2);
        assert_eq_u64(nresults, 2);
        assert_eq_i32(param_types[0], WAH_TYPE_I32);
        assert_eq_i32(param_types[1], WAH_TYPE_I64);
        assert_eq_i32(result_types[0], WAH_TYPE_F32);
        assert_eq_i32(result_types[1], WAH_TYPE_F64);
        free(param_types);
        free(result_types);
    }

    // Test 3: No parameters
    printf("Test 3: -> i32\n");
    {
        const char *types = "-> i32";
        size_t nparams, nresults;
        wah_type_t *param_types, *result_types;
        assert_ok(wah_parse_func_type(types, &nparams, &param_types, &nresults, &result_types));
        assert_eq_u64(nparams, 0);
        assert_eq_u64(nresults, 1);
        assert_eq_i32(result_types[0], WAH_TYPE_I32);
        free(param_types);
        free(result_types);
    }

    // Test 4: No results
    printf("Test 4: i32 ->\n");
    {
        const char *types = "i32 ->";
        size_t nparams, nresults;
        wah_type_t *param_types, *result_types;
        assert_ok(wah_parse_func_type(types, &nparams, &param_types, &nresults, &result_types));
        assert_eq_u64(nparams, 1);
        assert_eq_u64(nresults, 0);
        assert_eq_i32(param_types[0], WAH_TYPE_I32);
        free(param_types);
        free(result_types);
    }

    // Test 5: No parameters and no results
    printf("Test 5: ->\n");
    {
        const char *types = "->";
        size_t nparams, nresults;
        wah_type_t *param_types, *result_types;
        assert_ok(wah_parse_func_type(types, &nparams, &param_types, &nresults, &result_types));
        assert_eq_u64(nparams, 0);
        assert_eq_u64(nresults, 0);
        free(param_types);
        free(result_types);
    }

    // Test 6: Use with wah_module_export_func
    printf("Test 6: Use with wah_module_export_func\n");
    {
        wah_module_t mod = {0};
        assert_ok(wah_new_module(&mod));

        assert_ok(wah_module_export_func(&mod, "add", "i32, i32 -> i32", test_host_func, NULL, NULL));

        // Verify the export
        wah_entry_t entry;
        assert_ok(wah_module_export_by_name(&mod, "add", &entry));

        wah_free_module(&mod);
    }

    // Test 7: Invalid types should fail
    printf("Test 7: Invalid types should fail\n");
    {
        const char *invalid_types[] = {
            "i364 -> i32",    // Invalid type width
            "i128 -> i32",    // Invalid type width
            "i33 -> i32",     // Invalid type width
            "u32 -> i32",     // Invalid type prefix
            "i32 -> f128",    // Invalid result type
            "i3 -> i32",      // Incomplete type
            "i -> i32",       // Incomplete type
            "32 -> i32",      // Missing prefix
        };

        for (size_t i = 0; i < sizeof(invalid_types) / sizeof(invalid_types[0]); i++) {
            const char *types = invalid_types[i];
            size_t nparams, nresults;
            wah_type_t *param_types = NULL, *result_types = NULL;
            printf("  %s\n", types);
            assert_err(wah_parse_func_type(types, &nparams, &param_types, &nresults, &result_types), WAH_ERROR_BAD_SPEC);
            free(param_types);
            free(result_types);
        }
    }

    // Test 8: v128 type parsing
    printf("Test 8: v128 -> v128\n");
    {
        const char *types = "v128 -> v128";
        size_t nparams, nresults;
        wah_type_t *param_types, *result_types;
        assert_ok(wah_parse_func_type(types, &nparams, &param_types, &nresults, &result_types));
        assert_eq_u64(nparams, 1);
        assert_eq_u64(nresults, 1);
        assert_eq_i32(param_types[0], WAH_TYPE_V128);
        assert_eq_i32(result_types[0], WAH_TYPE_V128);
        free(param_types);
        free(result_types);
    }

    // Test 9: v128 mixed with other types
    printf("Test 9: i32, v128, f64 -> v128, i32\n");
    {
        const char *types = "i32, v128, f64 -> v128, i32";
        size_t nparams, nresults;
        wah_type_t *param_types, *result_types;
        assert_ok(wah_parse_func_type(types, &nparams, &param_types, &nresults, &result_types));
        assert_eq_u64(nparams, 3);
        assert_eq_u64(nresults, 2);
        assert_eq_i32(param_types[0], WAH_TYPE_I32);
        assert_eq_i32(param_types[1], WAH_TYPE_V128);
        assert_eq_i32(param_types[2], WAH_TYPE_F64);
        assert_eq_i32(result_types[0], WAH_TYPE_V128);
        assert_eq_i32(result_types[1], WAH_TYPE_I32);
        free(param_types);
        free(result_types);
    }

    // Test 10: Multiple v128 types
    printf("Test 10: v128, v128 -> v128\n");
    {
        const char *types = "v128, v128 -> v128";
        size_t nparams, nresults;
        wah_type_t *param_types, *result_types;
        assert_ok(wah_parse_func_type(types, &nparams, &param_types, &nresults, &result_types));
        assert_eq_u64(nparams, 2);
        assert_eq_u64(nresults, 1);
        assert_eq_i32(param_types[0], WAH_TYPE_V128);
        assert_eq_i32(param_types[1], WAH_TYPE_V128);
        assert_eq_i32(result_types[0], WAH_TYPE_V128);
        free(param_types);
        free(result_types);
    }

    // Test 11: v128 with no parameters
    printf("Test 11: -> v128\n");
    {
        const char *types = "-> v128";
        size_t nparams, nresults;
        wah_type_t *param_types, *result_types;
        assert_ok(wah_parse_func_type(types, &nparams, &param_types, &nresults, &result_types));
        assert_eq_u64(nparams, 0);
        assert_eq_u64(nresults, 1);
        assert_eq_i32(result_types[0], WAH_TYPE_V128);
        free(param_types);
        free(result_types);
    }

    // Test 12: v128 with no results
    printf("Test 12: v128 ->\n");
    {
        const char *types = "v128 ->";
        size_t nparams, nresults;
        wah_type_t *param_types, *result_types;
        assert_ok(wah_parse_func_type(types, &nparams, &param_types, &nresults, &result_types));
        assert_eq_u64(nparams, 1);
        assert_eq_u64(nresults, 0);
        assert_eq_i32(param_types[0], WAH_TYPE_V128);
        free(param_types);
        free(result_types);
    }

    printf("\nAll tests passed!\n");
    return 0;
}
