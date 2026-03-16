// Test for wah_parse_func_type

#define WAH_IMPLEMENTATION
#include "wah.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

void test_host_func(wah_call_context_t *ctx, void *userdata) {
    (void)ctx;
    (void)userdata;
    wah_return_i32(ctx, 42);
}

int main() {
    printf("Testing wah_parse_func_type...\n\n");

    // Test 1: Simple i32 -> i32
    printf("Test 1: i32 -> i32\n");
    {
        const char *types = "i32 -> i32";
        size_t nparams, nresults;
        wah_type_t *param_types, *result_types;
        wah_error_t err = wah_parse_func_type(types, &nparams, &param_types, &nresults, &result_types);
        if (err != WAH_OK) {
            printf("FAIL: wah_parse_func_type returned %s\n", wah_strerror(err));
            return 1;
        }
        if (nparams != 1 || nresults != 1) {
            printf("FAIL: Expected 1 param and 1 result, got %zu params and %zu results\n", nparams, nresults);
            return 1;
        }
        if (param_types[0] != WAH_TYPE_I32 || result_types[0] != WAH_TYPE_I32) {
            printf("FAIL: Expected i32 -> i32\n");
            return 1;
        }
        free(param_types);
        free(result_types);
        printf("PASS\n\n");
    }

    // Test 2: Multiple types with commas
    printf("Test 2: i32, i64 -> f32, f64\n");
    {
        const char *types = "i32, i64 -> f32, f64";
        size_t nparams, nresults;
        wah_type_t *param_types, *result_types;
        wah_error_t err = wah_parse_func_type(types, &nparams, &param_types, &nresults, &result_types);
        if (err != WAH_OK) {
            printf("FAIL: wah_parse_func_type returned %s\n", wah_strerror(err));
            return 1;
        }
        if (nparams != 2 || nresults != 2) {
            printf("FAIL: Expected 2 params and 2 results, got %zu params and %zu results\n", nparams, nresults);
            return 1;
        }
        if (param_types[0] != WAH_TYPE_I32 || param_types[1] != WAH_TYPE_I64) {
            printf("FAIL: Expected i32, i64 params\n");
            return 1;
        }
        if (result_types[0] != WAH_TYPE_F32 || result_types[1] != WAH_TYPE_F64) {
            printf("FAIL: Expected f32, f64 results\n");
            return 1;
        }
        free(param_types);
        free(result_types);
        printf("PASS\n\n");
    }

    // Test 3: No parameters
    printf("Test 3: -> i32\n");
    {
        const char *types = "-> i32";
        size_t nparams, nresults;
        wah_type_t *param_types, *result_types;
        wah_error_t err = wah_parse_func_type(types, &nparams, &param_types, &nresults, &result_types);
        if (err != WAH_OK) {
            printf("FAIL: wah_parse_func_type returned %s\n", wah_strerror(err));
            return 1;
        }
        if (nparams != 0 || nresults != 1) {
            printf("FAIL: Expected 0 params and 1 result, got %zu params and %zu results\n", nparams, nresults);
            return 1;
        }
        if (result_types[0] != WAH_TYPE_I32) {
            printf("FAIL: Expected i32 result\n");
            return 1;
        }
        free(param_types);
        free(result_types);
        printf("PASS\n\n");
    }

    // Test 4: No results
    printf("Test 4: i32 ->\n");
    {
        const char *types = "i32 ->";
        size_t nparams, nresults;
        wah_type_t *param_types, *result_types;
        wah_error_t err = wah_parse_func_type(types, &nparams, &param_types, &nresults, &result_types);
        if (err != WAH_OK) {
            printf("FAIL: wah_parse_func_type returned %s\n", wah_strerror(err));
            return 1;
        }
        if (nparams != 1 || nresults != 0) {
            printf("FAIL: Expected 1 param and 0 results, got %zu params and %zu results\n", nparams, nresults);
            return 1;
        }
        if (param_types[0] != WAH_TYPE_I32) {
            printf("FAIL: Expected i32 param\n");
            return 1;
        }
        free(param_types);
        free(result_types);
        printf("PASS\n\n");
    }

    // Test 5: No parameters and no results
    printf("Test 5: ->\n");
    {
        const char *types = "->";
        size_t nparams, nresults;
        wah_type_t *param_types, *result_types;
        wah_error_t err = wah_parse_func_type(types, &nparams, &param_types, &nresults, &result_types);
        if (err != WAH_OK) {
            printf("FAIL: wah_parse_func_type returned %s\n", wah_strerror(err));
            return 1;
        }
        if (nparams != 0 || nresults != 0) {
            printf("FAIL: Expected 0 params and 0 results, got %zu params and %zu results\n", nparams, nresults);
            return 1;
        }
        free(param_types);
        free(result_types);
        printf("PASS\n\n");
    }

    // Test 6: Use with wah_module_export_func
    printf("Test 6: Use with wah_module_export_func\n");
    {
        wah_module_t mod = {0};
        wah_error_t err = wah_new_module(&mod);
        if (err != WAH_OK) {
            printf("FAIL: wah_new_module returned %s\n", wah_strerror(err));
            return 1;
        }

        err = wah_module_export_func(&mod, "add", "i32, i32 -> i32", test_host_func, NULL, NULL);
        if (err != WAH_OK) {
            printf("FAIL: wah_module_export_func returned %s\n", wah_strerror(err));
            wah_free_module(&mod);
            return 1;
        }

        // Verify the export
        wah_entry_t entry;
        err = wah_module_export_by_name(&mod, "add", &entry);
        if (err != WAH_OK) {
            printf("FAIL: wah_module_export_by_name returned %s\n", wah_strerror(err));
            wah_free_module(&mod);
            return 1;
        }

        wah_free_module(&mod);
        printf("PASS\n\n");
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
            wah_error_t err = wah_parse_func_type(types, &nparams, &param_types, &nresults, &result_types);
            if (err == WAH_OK) {
                printf("FAIL: Expected error for '%s', but got success\n", types);
                free(param_types);
                free(result_types);
                return 1;
            }
            free(param_types);
            free(result_types);
        }
        printf("PASS\n\n");
    }

    printf("All tests passed!\n");
    return 0;
}
