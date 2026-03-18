// Test for linkage

#define WAH_IMPLEMENTATION
#include "wah.h"
#include "wah_testutils.c"
#include <stdio.h>
#include <string.h>

// Simple host function
int host_func_called = 0;
void simple_host_func(wah_call_context_t *ctx, void *userdata) {
    (void)ctx;
    (void)userdata;
    printf("  Host function called\n");
    host_func_called = 1;
    wah_return_i32(ctx, 42);
}

int main() {
    printf("Testing linkage...\n\n");

    // Test 1: Create and link host module
    printf("Test 1: Create and link host module\n");
    {
        // Create host module
        wah_module_t host_mod = {0};
        wah_error_t err = wah_new_module(&host_mod);
        if (err != WAH_OK) {
            printf("FAIL: wah_new_module returned %s\n", wah_strerror(err));
            return 1;
        }

        err = wah_module_export_funcv(&host_mod, "testFunc", 0, NULL, 1, (wah_type_t[]){WAH_TYPE_I32}, simple_host_func, NULL, NULL);
        if (err != WAH_OK) {
            printf("FAIL: wah_module_export_func returned %s\n", wah_strerror(err));
            wah_free_module(&host_mod);
            return 1;
        }

        // Create execution context with host module as primary
        wah_exec_context_t ctx = {0};
        err = wah_exec_context_create(&ctx, &host_mod);
        if (err != WAH_OK) {
            printf("FAIL: wah_exec_context_create returned %s\n", wah_strerror(err));
            wah_free_module(&host_mod);
            return 1;
        }

        // Link the same module (should work as no-op)
        err = wah_link_module(&ctx, "host", &host_mod);
        if (err != WAH_OK) {
            printf("FAIL: wah_link_module returned %s\n", wah_strerror(err));
            wah_exec_context_destroy(&ctx);
            wah_free_module(&host_mod);
            return 1;
        }

        // Instantiate
        err = wah_instantiate(&ctx);
        if (err != WAH_OK) {
            printf("FAIL: wah_instantiate returned %s\n", wah_strerror(err));
            wah_exec_context_destroy(&ctx);
            wah_free_module(&host_mod);
            return 1;
        }

        // Try to call the exported function
        wah_value_t result;
        host_func_called = 0;
        err = wah_call(&ctx, 0, NULL, 0, &result);
        if (err != WAH_OK) {
            printf("FAIL: wah_call returned %s\n", wah_strerror(err));
            wah_exec_context_destroy(&ctx);
            wah_free_module(&host_mod);
            return 1;
        }

        if (!host_func_called) {
            printf("FAIL: Host function was not called\n");
            wah_exec_context_destroy(&ctx);
            wah_free_module(&host_mod);
            return 1;
        }

        if (result.i32 != 42) {
            printf("FAIL: Expected result 42, got %d\n", result.i32);
            wah_exec_context_destroy(&ctx);
            wah_free_module(&host_mod);
            return 1;
        }

        wah_exec_context_destroy(&ctx);
        wah_free_module(&host_mod);

        printf("PASS\n\n");
    }

    // Test 2: Link multiple modules
    printf("Test 2: Link multiple modules\n");
    {
        // Create first host module
        wah_module_t mod1 = {0};
        wah_new_module(&mod1);
        wah_module_export_funcv(&mod1, "func1", 0, NULL, 1, (wah_type_t[]){WAH_TYPE_I32}, simple_host_func, NULL, NULL);

        // Create second host module
        wah_module_t mod2 = {0};
        wah_new_module(&mod2);
        wah_module_export_funcv(&mod2, "func2", 0, NULL, 1, (wah_type_t[]){WAH_TYPE_I32}, simple_host_func, NULL, NULL);

        // Create execution context
        wah_exec_context_t ctx = {0};
        wah_error_t err = wah_exec_context_create(&ctx, &mod1);
        if (err != WAH_OK) {
            printf("FAIL: wah_exec_context_create returned %s\n", wah_strerror(err));
            wah_free_module(&mod1);
            wah_free_module(&mod2);
            return 1;
        }

        // Link second module
        err = wah_link_module(&ctx, "mod2", &mod2);
        if (err != WAH_OK) {
            printf("FAIL: wah_link_module returned %s\n", wah_strerror(err));
            wah_exec_context_destroy(&ctx);
            wah_free_module(&mod1);
            wah_free_module(&mod2);
            return 1;
        }

        // Instantiate
        err = wah_instantiate(&ctx);
        if (err != WAH_OK) {
            printf("FAIL: wah_instantiate returned %s\n", wah_strerror(err));
            wah_exec_context_destroy(&ctx);
            wah_free_module(&mod1);
            wah_free_module(&mod2);
            return 1;
        }

        // Call function from first module
        wah_value_t result;
        host_func_called = 0;
        err = wah_call(&ctx, 0, NULL, 0, &result);
        if (err != WAH_OK) {
            printf("FAIL: wah_call returned %s\n", wah_strerror(err));
            wah_exec_context_destroy(&ctx);
            wah_free_module(&mod1);
            wah_free_module(&mod2);
            return 1;
        }

        if (!host_func_called) {
            printf("FAIL: Host function was not called\n");
            wah_exec_context_destroy(&ctx);
            wah_free_module(&mod1);
            wah_free_module(&mod2);
            return 1;
        }

        wah_exec_context_destroy(&ctx);
        wah_free_module(&mod1);
        wah_free_module(&mod2);

        printf("PASS\n\n");
    }

    // Test 3: Duplicate export names should fail
    printf("Test 3: Duplicate export names should fail\n");
    {
        wah_module_t mod = {0};
        wah_new_module(&mod);

        wah_error_t err1 = wah_module_export_funcv(&mod, "dup", 0, NULL, 1, (wah_type_t[]){WAH_TYPE_I32}, simple_host_func, NULL, NULL);
        wah_error_t err2 = wah_module_export_funcv(&mod, "dup", 0, NULL, 1, (wah_type_t[]){WAH_TYPE_I32}, simple_host_func, NULL, NULL);

        if (err1 != WAH_OK) {
            printf("FAIL: First export should succeed\n");
            wah_free_module(&mod);
            return 1;
        }

        if (err2 == WAH_OK) {
            printf("FAIL: Duplicate export should fail\n");
            wah_free_module(&mod);
            return 1;
        }

        wah_free_module(&mod);
        printf("PASS\n\n");
    }

    // Test 4: Instantiate without linking should work for single module
    printf("Test 4: Instantiate without linking\n");
    {
        wah_module_t mod = {0};
        wah_new_module(&mod);
        wah_module_export_funcv(&mod, "func", 0, NULL, 1, (wah_type_t[]){WAH_TYPE_I32}, simple_host_func, NULL, NULL);

        wah_exec_context_t ctx = {0};
        wah_error_t err = wah_exec_context_create(&ctx, &mod);
        if (err != WAH_OK) {
            printf("FAIL: wah_exec_context_create returned %s\n", wah_strerror(err));
            wah_free_module(&mod);
            return 1;
        }

        // Instantiate directly without linking
        err = wah_instantiate(&ctx);
        if (err != WAH_OK) {
            printf("FAIL: wah_instantiate returned %s\n", wah_strerror(err));
            wah_exec_context_destroy(&ctx);
            wah_free_module(&mod);
            return 1;
        }

        wah_exec_context_destroy(&ctx);
        wah_free_module(&mod);
        printf("PASS\n\n");
    }

    // Test 5: Wasm-to-wasm cross-module call
    printf("Test 5: Wasm-to-wasm cross-module call\n");
    {
        wah_module_t module_b = {0};
        wah_module_t module_a = {0};
        wah_exec_context_t ctx = {0};
        wah_error_t err;

        // Module B: exports a simple add function
        // func add(i32, i32) -> i32: returns a + b
        err = wah_parse_module_from_spec(&module_b, "wasm \
            types {[ fn [i32, i32] [i32] ]} \
            funcs {[ 0 ]} \
            exports {[ {'add'} fn# 0 ]} \
            code {[ {[] local.get 0 local.get 1 i32.add end } ]}"
        );
        if (err != WAH_OK) {
            printf("FAIL: Failed to create module_b: %s\n", wah_strerror(err));
            goto cleanup;
        }

        // Module A: imports add from moduleB and exports callAdd
        // func callAdd(i32, i32) -> i32: calls add(a, b) and returns result
        err = wah_parse_module_from_spec(&module_a, "wasm \
            types {[ fn [i32, i32] [i32] ]} \
            imports {[ {'moduleB'} {'add'} fn# 0 ]} \
            funcs {[ 0 ]} \
            exports {[ {'callAdd'} fn# 1 ]} \
            code {[ {[] local.get 0 local.get 1 call 0 end } ]}"
        );
        if (err != WAH_OK) {
            printf("FAIL: Failed to create module_a: %s\n", wah_strerror(err));
            goto cleanup;
        }

        // Create execution context with module_a as primary
        err = wah_exec_context_create(&ctx, &module_a);
        if (err != WAH_OK) {
            printf("FAIL: wah_exec_context_create returned %s\n", wah_strerror(err));
            goto cleanup;
        }

        // Link module_b
        err = wah_link_module(&ctx, "moduleB", &module_b);
        if (err != WAH_OK) {
            printf("FAIL: wah_link_module returned %s\n", wah_strerror(err));
            goto cleanup;
        }

        // Instantiate
        err = wah_instantiate(&ctx);
        if (err != WAH_OK) {
            printf("FAIL: wah_instantiate returned %s\n", wah_strerror(err));
            goto cleanup;
        }

        // Call callAdd(10, 32) - should call add(10, 32) = 42
        wah_value_t params[2] = {{.i32 = 10}, {.i32 = 32}};
        wah_value_t result;
        err = wah_call(&ctx, 0, params, 2, &result);
        if (err != WAH_OK) {
            printf("FAIL: wah_call returned %s\n", wah_strerror(err));
            goto cleanup;
        }

        if (result.i32 != 42) {
            printf("FAIL: Expected result 42, got %d\n", result.i32);
            goto cleanup;
        }

        printf("PASS\n\n");

cleanup:
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module_a);
        wah_free_module(&module_b);
        if (err != WAH_OK) return 1;
    }

    printf("All linkage tests passed!\n");
    return 0;
}
