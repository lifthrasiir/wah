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
        assert_ok(wah_new_module(&host_mod));

        assert_ok(wah_module_export_funcv(&host_mod, "testFunc", 0, NULL, 1, (wah_type_t[]){WAH_TYPE_I32}, simple_host_func, NULL, NULL));

        // Create execution context with host module as primary
        wah_exec_context_t ctx = {0};
        assert_ok(wah_exec_context_create(&ctx, &host_mod));

        // Link the same module (should work as no-op)
        assert_ok(wah_link_module(&ctx, "host", &host_mod));

        // Instantiate
        assert_ok(wah_instantiate(&ctx));

        // Try to call the exported function
        wah_value_t result;
        host_func_called = 0;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
        assert_true(host_func_called);
        assert_eq_i32(result.i32, 42);

        wah_exec_context_destroy(&ctx);
        wah_free_module(&host_mod);
    }

    // Test 2: Link multiple modules
    printf("Test 2: Link multiple modules\n");
    {
        // Create first host module
        wah_module_t mod1 = {0};
        wah_new_module(&mod1);
        assert_ok(wah_module_export_funcv(&mod1, "func1", 0, NULL, 1, (wah_type_t[]){WAH_TYPE_I32}, simple_host_func, NULL, NULL));

        // Create second host module
        wah_module_t mod2 = {0};
        assert_ok(wah_new_module(&mod2));
        assert_ok(wah_module_export_funcv(&mod2, "func2", 0, NULL, 1, (wah_type_t[]){WAH_TYPE_I32}, simple_host_func, NULL, NULL));

        // Create execution context
        wah_exec_context_t ctx = {0};
        assert_ok(wah_exec_context_create(&ctx, &mod1));

        // Link second module
        assert_ok(wah_link_module(&ctx, "mod2", &mod2));

        // Instantiate
        assert_ok(wah_instantiate(&ctx));

        // Call function from first module
        wah_value_t result;
        host_func_called = 0;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
        assert_true(host_func_called);
        assert_eq_i32(result.i32, 42);

        wah_exec_context_destroy(&ctx);
        wah_free_module(&mod1);
        wah_free_module(&mod2);
    }

    // Test 3: Duplicate export names should fail
    printf("Test 3: Duplicate export names should fail\n");
    {
        wah_module_t mod = {0};
        assert_ok(wah_new_module(&mod));

        assert_ok(wah_module_export_funcv(&mod, "dup", 0, NULL, 1, (wah_type_t[]){WAH_TYPE_I32}, simple_host_func, NULL, NULL));
        assert_err(wah_module_export_funcv(&mod, "dup", 0, NULL, 1, (wah_type_t[]){WAH_TYPE_I32}, simple_host_func, NULL, NULL), WAH_ERROR_VALIDATION_FAILED);

        wah_free_module(&mod);
    }

    // Test 4: Instantiate without linking should work for single module
    printf("Test 4: Instantiate without linking\n");
    {
        wah_module_t mod = {0};
        assert_ok(wah_new_module(&mod));
        assert_ok(wah_module_export_funcv(&mod, "func", 0, NULL, 1, (wah_type_t[]){WAH_TYPE_I32}, simple_host_func, NULL, NULL));

        wah_exec_context_t ctx = {0};
        assert_ok(wah_exec_context_create(&ctx, &mod));

        // Instantiate directly without linking
        assert_ok(wah_instantiate(&ctx));

        wah_exec_context_destroy(&ctx);
        wah_free_module(&mod);
    }

    // Test 5: Wasm-to-wasm cross-module call
    printf("Test 5: Wasm-to-wasm cross-module call\n");
    {
        wah_module_t module_b = {0};
        wah_module_t module_a = {0};
        wah_exec_context_t ctx = {0};

        // Module B: exports a simple add function
        // func add(i32, i32) -> i32: returns a + b
        const char *module_b_wasm = "wasm \
            types {[ fn [i32, i32] [i32] ]} \
            funcs {[ 0 ]} \
            exports {[ {'add'} fn# 0 ]} \
            code {[ {[] local.get 0 local.get 1 i32.add end } ]}";
        assert_ok(wah_parse_module_from_spec(&module_b, module_b_wasm));

        // Module A: imports add from moduleB and exports callAdd
        // func callAdd(i32, i32) -> i32: calls add(a, b) and returns result
        const char *module_a_wasm = "wasm \
            types {[ fn [i32, i32] [i32] ]} \
            imports {[ {'moduleB'} {'add'} fn# 0 ]} \
            funcs {[ 0 ]} \
            exports {[ {'callAdd'} fn# 1 ]} \
            code {[ {[] local.get 0 local.get 1 call 0 end } ]}";
        assert_ok(wah_parse_module_from_spec(&module_a, module_a_wasm));

        // Create execution context with module_a as primary
        assert_ok(wah_exec_context_create(&ctx, &module_a));

        // Link module_b
        assert_ok(wah_link_module(&ctx, "moduleB", &module_b));

        // Instantiate
        assert_ok(wah_instantiate(&ctx));

        // Call callAdd(10, 32) - should call add(10, 32) = 42
        wah_value_t params[2] = {{.i32 = 10}, {.i32 = 32}};
        wah_value_t result;
        assert_ok(wah_call(&ctx, 0, params, 2, &result));
        assert_eq_i32(result.i32, 42);

        wah_exec_context_destroy(&ctx);
        wah_free_module(&module_a);
        wah_free_module(&module_b);
    }

    // Test 6: Linked module global initializer using global.get
    printf("Test 6: Linked module global initializer using global.get\n");
    {
        // Module A: global[0]=100, imports getGlobal1 from moduleB, wraps and exports it
        const char *spec_a = "wasm \
            types {[ fn [] [i32] ]} \
            imports {[ {'moduleB'} {'getGlobal1'} fn# 0 ]} \
            funcs {[ 0 ]} \
            globals {[ i32 mut i32.const 100 end ]} \
            exports {[ {'callGetGlobal1'} fn# 1 ]} \
            code {[ {[] call 0 end} ]}";

        // Module B: global[0]=42, global[1]=global.get 0 (must resolve to B's own global[0]=42,
        //           not A's global[0]=100)
        const char *spec_b = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0 ]} \
            globals {[ \
                i32 mut i32.const 42 end, \
                i32 mut global.get 0 end \
            ]} \
            exports {[ {'getGlobal1'} fn# 0 ]} \
            code {[ {[] global.get 1 end} ]}";

        wah_module_t mod_a = {0}, mod_b = {0};
        wah_exec_context_t ctx = {0};

        assert_ok(wah_parse_module_from_spec(&mod_a, spec_a));
        assert_ok(wah_parse_module_from_spec(&mod_b, spec_b));
        assert_ok(wah_exec_context_create(&ctx, &mod_a));
        assert_ok(wah_link_module(&ctx, "moduleB", &mod_b));
        assert_ok(wah_instantiate(&ctx));

        wah_value_t result;
        assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
        assert_eq_i32(result.i32, 42);

        wah_exec_context_destroy(&ctx);
        wah_free_module(&mod_a);
        wah_free_module(&mod_b);
    }

    printf("All linkage tests passed!\n");
    return 0;
}
