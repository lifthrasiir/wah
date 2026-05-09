// Test for linkage

#include "../wah.h"
#include "common.h"
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

// Host function for imported start function test
int imported_start_called = 0;
void imported_start_host_func(wah_call_context_t *ctx, void *userdata) {
    (void)ctx;
    (void)userdata;
    imported_start_called = 1;
}

int main() {
    printf("Testing linkage...\n\n");

    // Test 1: Create and link host module
    printf("Test 1: Create and link host module\n");
    {
        // Create host module
        wah_module_t host_mod = {0};
        assert_ok(wah_new_module(&host_mod, NULL));

        assert_ok(wah_export_func(&host_mod, "testFunc", "() -> i32", simple_host_func, NULL, NULL));

        // Create execution context with host module as primary
        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &host_mod, NULL));

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

        wah_free_exec_context(&ctx);
        wah_free_module(&host_mod);
    }

    // Test 2: Link multiple modules
    printf("Test 2: Link multiple modules\n");
    {
        // Create first host module
        wah_module_t mod1 = {0};
        wah_new_module(&mod1, NULL);
        assert_ok(wah_export_func(&mod1, "func1", "() -> i32", simple_host_func, NULL, NULL));

        // Create second host module
        wah_module_t mod2 = {0};
        assert_ok(wah_new_module(&mod2, NULL));
        assert_ok(wah_export_func(&mod2, "func2", "() -> i32", simple_host_func, NULL, NULL));

        // Create execution context
        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &mod1, NULL));

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

        wah_free_exec_context(&ctx);
        wah_free_module(&mod1);
        wah_free_module(&mod2);
    }

    // Test 3: Duplicate export names should fail
    printf("Test 3: Duplicate export names should fail\n");
    {
        wah_module_t mod = {0};
        assert_ok(wah_new_module(&mod, NULL));

        assert_ok(wah_export_func(&mod, "dup", "() -> i32", simple_host_func, NULL, NULL));
        assert_err(wah_export_func(&mod, "dup", "() -> i32", simple_host_func, NULL, NULL), WAH_ERROR_VALIDATION_FAILED);

        wah_free_module(&mod);
    }

    // Test 4: Instantiate without linking should work for single module
    printf("Test 4: Instantiate without linking\n");
    {
        wah_module_t mod = {0};
        assert_ok(wah_new_module(&mod, NULL));
        assert_ok(wah_export_func(&mod, "func", "() -> i32", simple_host_func, NULL, NULL));

        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &mod, NULL));

        // Instantiate directly without linking
        assert_ok(wah_instantiate(&ctx));

        wah_free_exec_context(&ctx);
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
        assert_ok(wah_new_exec_context(&ctx, &module_a, NULL));

        // Link module_b
        assert_ok(wah_link_module(&ctx, "moduleB", &module_b));

        // Instantiate
        assert_ok(wah_instantiate(&ctx));

        // Call callAdd(10, 32) - should call add(10, 32) = 42
        wah_value_t params[2] = {{.i32 = 10}, {.i32 = 32}};
        wah_value_t result;
        assert_ok(wah_call(&ctx, 0, params, 2, &result));
        assert_eq_i32(result.i32, 42);

        wah_free_exec_context(&ctx);
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
                i32 immut i32.const 42 end, \
                i32 mut global.get 0 end \
            ]} \
            exports {[ {'getGlobal1'} fn# 0 ]} \
            code {[ {[] global.get 1 end} ]}";

        wah_module_t mod_a = {0}, mod_b = {0};
        wah_exec_context_t ctx = {0};

        assert_ok(wah_parse_module_from_spec(&mod_a, spec_a));
        assert_ok(wah_parse_module_from_spec(&mod_b, spec_b));
        assert_ok(wah_new_exec_context(&ctx, &mod_a, NULL));
        assert_ok(wah_link_module(&ctx, "moduleB", &mod_b));
        assert_ok(wah_instantiate(&ctx));

        wah_value_t result;
        assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
        assert_eq_i32(result.i32, 42);

        wah_free_exec_context(&ctx);
        wah_free_module(&mod_a);
        wah_free_module(&mod_b);
    }

    // Test 7: Call imported linked-wasm function directly via wah_call (exported imported function call)
    // Regression: wah_call_module_multi used caller module's function_type_indices instead of callee's,
    // causing out-of-bounds access when the primary module has no local functions.
    printf("Test 7: Call imported linked-wasm function directly\n");
    {
        // Module B: exports getValue () -> i32 returning 42
        const char *spec_b = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0 ]} \
            exports {[ {'getValue'} fn# 0 ]} \
            code {[ {[] i32.const 42 end} ]}";

        // Module A: imports getValue from moduleB, NO local functions
        const char *spec_a = "wasm \
            types {[ fn [] [i32] ]} \
            imports {[ {'moduleB'} {'getValue'} fn# 0 ]}";

        wah_module_t mod_a = {0}, mod_b = {0};
        wah_exec_context_t ctx = {0};

        assert_ok(wah_parse_module_from_spec(&mod_b, spec_b));
        assert_ok(wah_parse_module_from_spec(&mod_a, spec_a));
        assert_ok(wah_new_exec_context(&ctx, &mod_a, NULL));
        assert_ok(wah_link_module(&ctx, "moduleB", &mod_b));
        assert_ok(wah_instantiate(&ctx));

        // Call the imported function at global index 0 directly
        wah_value_t result;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
        assert_eq_i32(result.i32, 42);

        wah_free_exec_context(&ctx);
        wah_free_module(&mod_a);
        wah_free_module(&mod_b);
    }

    // Test 8: Imported start function is called during instantiation (not before import resolution)
    // Regression: wah_new_exec_context called start function before wah_instantiate resolved
    // imports, so function_table import slots were zero-initialized, causing wrong dispatch/crash.
    printf("Test 8: Imported start function\n");
    {
        wah_module_t host_mod = {0};
        assert_ok(wah_new_module(&host_mod, NULL));
        assert_ok(wah_export_func(&host_mod, "initFunc", "()",
                                         imported_start_host_func, NULL, NULL));

        // Module imports initFunc from 'host' and declares it (index 0) as start function.
        const char *spec_a = "wasm \
            types {[ fn [] [] ]} \
            imports {[ {'host'} {'initFunc'} fn# 0 ]} \
            start { 0 }";

        wah_module_t mod_a = {0};
        wah_exec_context_t ctx = {0};

        imported_start_called = 0;
        assert_ok(wah_parse_module_from_spec(&mod_a, spec_a));
        assert_ok(wah_new_exec_context(&ctx, &mod_a, NULL));
        assert_ok(wah_link_module(&ctx, "host", &host_mod));
        assert_ok(wah_instantiate(&ctx));
        assert_true(imported_start_called);

        wah_free_exec_context(&ctx);
        wah_free_module(&mod_a);
        wah_free_module(&host_mod);
    }

    // Test 9: Linked module global initializer using ref.func
    // Regression: linked module funcref globals were left as prefuncref after instantiation
    // (missing conversion step), causing call_indirect to dispatch through garbage pointers.
    printf("Test 9: Linked module global initializer using ref.func\n");
    {
        // Module B: func 0 returns 42; func 1 reads funcref global 0 (= ref.func 0),
        // stores it in table[0][0], then calls via call_indirect.
        // Global 0 must resolve to B's own func 0, not the primary module's func 0.
        const char *spec_b = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0, 0 ]} \
            tables {[ funcref limits.i32/1 1 ]} \
            globals {[ funcref mut ref.func 0 end ]} \
            exports {[ {'callViaRef'} fn# 1 ]} \
            code {[ \
                {[] i32.const 42 end}, \
                {[] i32.const 0 global.get 0 table.set 0 i32.const 0 call_indirect 0 0 end} \
            ]}";

        // Module A: imports callViaRef from B; provides table[0] used at runtime.
        const char *spec_a = "wasm \
            types {[ fn [] [i32] ]} \
            imports {[ {'moduleB'} {'callViaRef'} fn# 0 ]} \
            tables {[ funcref limits.i32/1 1 ]}";

        wah_module_t mod_a = {0}, mod_b = {0};
        wah_exec_context_t ctx = {0};

        assert_ok(wah_parse_module_from_spec(&mod_b, spec_b));
        assert_ok(wah_parse_module_from_spec(&mod_a, spec_a));
        assert_ok(wah_new_exec_context(&ctx, &mod_a, NULL));
        assert_ok(wah_link_module(&ctx, "moduleB", &mod_b));
        assert_ok(wah_instantiate(&ctx));

        // Calling import 0 (B's callViaRef) should return 42 via the funcref global.
        wah_value_t result;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
        assert_eq_i32(result.i32, 42);

        wah_free_exec_context(&ctx);
        wah_free_module(&mod_a);
        wah_free_module(&mod_b);
    }

    // Test 10: Linked module global initializer using ref.func referencing the linked module's own import.
    // Regression: ref.func <N> in a linked module's global, where N < import_function_count,
    // was left as a prefuncref sentinel instead of being resolved to the actual imported function.
    printf("Test 10: Linked module global uses ref.func pointing at its own import\n");
    {
        // Module C (host): exports "getConst" () -> i32 returning 42.
        wah_module_t mod_c = {0};
        assert_ok(wah_new_module(&mod_c, NULL));
        assert_ok(wah_export_func(&mod_c, "getConst", "() -> i32",
                                         simple_host_func, NULL, NULL));

        // Module B: imports "moduleC"."getConst" (func idx 0).
        // global[0] = funcref mut ref.func 0 (= the IMPORT, not a local).
        // local func 0 (global idx 1): stores global[0] into table[0][0], then call_indirect.
        const char *spec_b = "wasm \
            types {[ fn [] [i32] ]} \
            imports {[ {'moduleC'} {'getConst'} fn# 0 ]} \
            funcs {[ 0 ]} \
            tables {[ funcref limits.i32/1 1 ]} \
            globals {[ funcref mut ref.func 0 end ]} \
            exports {[ {'callViaRef'} fn# 1 ]} \
            code {[ \
                {[] i32.const 0 global.get 0 table.set 0 i32.const 0 call_indirect 0 0 end} \
            ]}";

        // Module A: imports callViaRef from moduleB; provides table[0] used at runtime.
        const char *spec_a = "wasm \
            types {[ fn [] [i32] ]} \
            imports {[ {'moduleB'} {'callViaRef'} fn# 0 ]} \
            tables {[ funcref limits.i32/1 1 ]}";

        wah_module_t mod_a = {0}, mod_b = {0};
        wah_exec_context_t ctx = {0};

        assert_ok(wah_parse_module_from_spec(&mod_b, spec_b));
        assert_ok(wah_parse_module_from_spec(&mod_a, spec_a));
        assert_ok(wah_new_exec_context(&ctx, &mod_a, NULL));
        assert_ok(wah_link_module(&ctx, "moduleB", &mod_b));
        assert_ok(wah_link_module(&ctx, "moduleC", &mod_c));
        assert_ok(wah_instantiate(&ctx));

        wah_value_t result;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
        assert_eq_i32(result.i32, 42);

        wah_free_exec_context(&ctx);
        wah_free_module(&mod_a);
        wah_free_module(&mod_b);
        wah_free_module(&mod_c);
    }

    // 2cc74f5: Fix cross-module CALL using wrong function table.
    {
        printf("Testing cross-module call uses correct function (2cc74f5)...\n");

        // Provider has 2 functions: func0 returns 10, func1 returns 20.
        // It exports func1 (not func0).
        const char *provider_spec = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0, 0 ]} \
            exports {[ {'get20'} fn# 1 ]} \
            code {[ {[] i32.const 10 end }, {[] i32.const 20 end } ]}";

        // Consumer imports get20 and calls it.
        // Consumer also has a local func0 that returns 99.
        // The bug was that the call would use the consumer's func table slot,
        // calling 99 instead of 20.
        const char *consumer_spec = "wasm \
            types {[ fn [] [i32] ]} \
            imports {[ {'provider'} {'get20'} fn# 0 ]} \
            funcs {[ 0, 0 ]} \
            code {[ {[] i32.const 99 end }, {[] call 0 end } ]}";

        wah_module_t provider = {0}, consumer = {0};
        assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
        assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &consumer, NULL));
        assert_ok(wah_link_module(&ctx, "provider", &provider));
        assert_ok(wah_instantiate(&ctx));

        // Call func2 (index 2 = import + local 1) which calls import 0
        wah_value_t result;
        assert_ok(wah_call(&ctx, 2, NULL, 0, &result));
        assert_eq_i32(result.i32, 20);

        wah_free_exec_context(&ctx);
        wah_free_module(&consumer);
        wah_free_module(&provider);
    }

    // 7e69288: Share mutable globals across linked modules via indirect opcodes.
    {
        printf("Testing shared mutable globals across modules (7e69288)...\n");

        // Provider exports a mutable global and a setter function.
        const char *provider_spec = "wasm \
            types {[ fn [i32] [], fn [] [i32] ]} \
            funcs {[ 0, 1 ]} \
            globals {[ i32 mut i32.const 0 end ]} \
            exports {[ {'g'} global# 0, {'set'} fn# 0, {'get'} fn# 1 ]} \
            code {[ \
                {[] local.get 0 global.set 0 end }, \
                {[] global.get 0 end } \
            ]}";

        // Consumer imports the global and the setter.
        // Consumer reads the global after the provider sets it.
        const char *consumer_spec = "wasm \
            types {[ fn [i32] [], fn [] [i32] ]} \
            imports {[ {'provider'} {'set'} fn# 0, {'provider'} {'g'} global# i32 mut ]} \
            funcs {[ 1 ]} \
            code {[ {[] global.get 0 end } ]}";

        wah_module_t provider = {0}, consumer = {0};
        assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
        assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &consumer, NULL));
        assert_ok(wah_link_module(&ctx, "provider", &provider));
        assert_ok(wah_instantiate(&ctx));

        // Set via imported function (provider's set)
        wah_value_t params[1] = {{.i32 = 555}};
        assert_ok(wah_call(&ctx, 0, params, 1, NULL));

        // Read via consumer's read (which reads the imported global)
        wah_value_t result;
        assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
        assert_eq_i32(result.i32, 555);

        wah_free_exec_context(&ctx);
        wah_free_module(&consumer);
        wah_free_module(&provider);
    }

    // 0a4153b: Fix linked module globals offset for multi-instance same-module linking.
    {
        printf("Testing multi-instance same-module globals offset (0a4153b)...\n");

        // Provider exports a mutable i32 global, setter and getter.
        const char *provider_spec = "wasm \
            types {[ fn [i32] [], fn [] [i32] ]} \
            funcs {[ 0, 1 ]} \
            globals {[ i32 mut i32.const 0 end ]} \
            exports {[ {'set'} fn# 0, {'get'} fn# 1 ]} \
            code {[ \
                {[] local.get 0 global.set 0 end }, \
                {[] global.get 0 end } \
            ]}";

        // Consumer imports set/get from two instances: 'a' and 'b'.
        const char *consumer_spec = "wasm \
            types {[ fn [i32] [], fn [] [i32] ]} \
            imports {[ \
                {'a'} {'set'} fn# 0, \
                {'a'} {'get'} fn# 1, \
                {'b'} {'set'} fn# 0, \
                {'b'} {'get'} fn# 1 \
            ]}";

        wah_module_t provider_a = {0}, provider_b = {0}, consumer = {0};
        assert_ok(wah_parse_module_from_spec(&provider_a, provider_spec));
        assert_ok(wah_parse_module_from_spec(&provider_b, provider_spec));
        assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &consumer, NULL));
        assert_ok(wah_link_module(&ctx, "a", &provider_a));
        assert_ok(wah_link_module(&ctx, "b", &provider_b));
        assert_ok(wah_instantiate(&ctx));

        // Set 'a' global to 111
        wah_value_t params[1] = {{.i32 = 111}};
        assert_ok(wah_call(&ctx, 0, params, 1, NULL));

        // Set 'b' global to 222
        params[0].i32 = 222;
        assert_ok(wah_call(&ctx, 2, params, 1, NULL));

        // Get 'a' should be 111 (not aliased by 'b')
        wah_value_t result;
        assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
        assert_eq_i32(result.i32, 111);

        // Get 'b' should be 222
        assert_ok(wah_call(&ctx, 3, NULL, 0, &result));
        assert_eq_i32(result.i32, 222);

        wah_free_exec_context(&ctx);
        wah_free_module(&consumer);
        wah_free_module(&provider_a);
        wah_free_module(&provider_b);
    }

    printf("Testing cross-module funcref table roundtrip...\n");
    {
        const char *provider_spec = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0, 0 ]} \
            exports {[ {'f10'} fn# 0, {'f20'} fn# 1 ]} \
            code {[ {[] i32.const 10 end }, {[] i32.const 20 end } ]}";

        const char *consumer_spec = "wasm \
            types {[ fn [] [i32], fn [i32] [i32] ]} \
            imports {[ {'provider'} {'f10'} fn# 0, {'provider'} {'f20'} fn# 0 ]} \
            funcs {[ 1 ]} \
            tables {[ funcref limits.i32/1 2 ]} \
            elements {[ elem.active.table#0 i32.const 0 end [ 0, 1 ] ]} \
            code {[ {[] local.get 0 call_indirect 0 0 end } ]}";

        wah_module_t provider = {0}, consumer = {0};
        assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
        assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &consumer, NULL));
        assert_ok(wah_link_module(&ctx, "provider", &provider));
        assert_ok(wah_instantiate(&ctx));

        wah_value_t result;
        wah_value_t param;

        param.i32 = 0;
        assert_ok(wah_call(&ctx, 2, &param, 1, &result));
        assert_eq_i32(result.i32, 10);

        param.i32 = 1;
        assert_ok(wah_call(&ctx, 2, &param, 1, &result));
        assert_eq_i32(result.i32, 20);

        wah_free_exec_context(&ctx);
        wah_free_module(&consumer);
        wah_free_module(&provider);
    }

    printf("Testing cross-module funcref global call...\n");
    {
        const char *provider_spec = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[] i32.const 99 end } ]}";

        const char *consumer_spec = "wasm \
            types {[ fn [] [i32] ]} \
            imports {[ {'provider'} {'f'} fn# 0 ]} \
            funcs {[ 0 ]} \
            tables {[ funcref limits.i32/1 1 ]} \
            globals {[ funcref mut ref.func 0 end ]} \
            code {[ {[] \
                i32.const 0 global.get 0 table.set 0 \
                i32.const 0 call_indirect 0 0 \
            end } ]}";

        wah_module_t provider = {0}, consumer = {0};
        assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
        assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &consumer, NULL));
        assert_ok(wah_link_module(&ctx, "provider", &provider));
        assert_ok(wah_instantiate(&ctx));

        wah_value_t result;
        assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
        assert_eq_i32(result.i32, 99);

        wah_free_exec_context(&ctx);
        wah_free_module(&consumer);
        wah_free_module(&provider);
    }

    printf("Testing duplicate module name...\n");
    {
        wah_module_t mod = {0};
        assert_ok(wah_new_module(&mod, NULL));

        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
        assert_ok(wah_link_module(&ctx, "mymod", &mod));
        assert_err(wah_link_module(&ctx, "mymod", &mod), WAH_ERROR_VALIDATION_FAILED);

        wah_free_exec_context(&ctx);
        wah_free_module(&mod);
    }

    printf("Testing duplicate name in wah_link_context...\n");
    {
        const char *spec = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0 ]} \
            exports {[ {'f'} fn# 0 ]} \
            code {[ {[] i32.const 1 end } ]}";

        wah_module_t pmod = {0};
        assert_ok(wah_parse_module_from_spec(&pmod, spec));
        wah_exec_context_t pctx = {0};
        assert_ok(wah_new_exec_context(&pctx, &pmod, NULL));
        assert_ok(wah_instantiate(&pctx));

        wah_module_t cmod = {0};
        assert_ok(wah_new_module(&cmod, NULL));
        wah_exec_context_t cctx = {0};
        assert_ok(wah_new_exec_context(&cctx, &cmod, NULL));

        assert_ok(wah_link_context(&cctx, "provider", &pctx));
        assert_err(wah_link_context(&cctx, "provider", &pctx), WAH_ERROR_VALIDATION_FAILED);

        wah_free_exec_context(&cctx);
        wah_free_exec_context(&pctx);
        wah_free_module(&cmod);
        wah_free_module(&pmod);
    }

    printf("Testing immutable global import type match...\n");
    {
        const char *provider_spec = "wasm \
            types {[]} \
            globals {[ funcref immut ref.null funcref end ]} \
            exports {[ {'g'} global# 0 ]}";

        const char *consumer_spec = "wasm \
            types {[ fn [] [i32] ]} \
            imports {[ {'provider'} {'g'} global# funcref immut ]} \
            funcs {[ 0 ]} \
            code {[ {[] global.get 0 ref.is_null end } ]}";

        wah_module_t provider = {0}, consumer = {0};
        assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
        assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &consumer, NULL));
        assert_ok(wah_link_module(&ctx, "provider", &provider));
        assert_ok(wah_instantiate(&ctx));

        wah_value_t result;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
        assert_eq_i32(result.i32, 1);

        wah_free_exec_context(&ctx);
        wah_free_module(&consumer);
        wah_free_module(&provider);
    }

    // Regression: call_indirect on host function with mismatched type must trap,
    // not bypass type checks via assert-only guards in WAH_REF_BODY.
    printf("Test: Host function call_indirect type mismatch traps\n");
    {
        wah_module_t host_mod = {0};
        assert_ok(wah_new_module(&host_mod, NULL));
        assert_ok(wah_export_func(&host_mod, "hostFn", "(i32) -> i32", simple_host_func, NULL, NULL));

        // Consumer imports hostFn, stores it in table[0], then call_indirect with
        // type 0 = fn [] [i32] (no params), which mismatches (i32) -> i32.
        const char *spec = "wasm \
            types {[ fn [] [i32], fn [i32] [i32] ]} \
            imports {[ {'host'} {'hostFn'} fn# 1 ]} \
            funcs {[ 0 ]} \
            tables {[ funcref limits.i32/1 1 ]} \
            globals {[ funcref mut ref.func 0 end ]} \
            exports {[ {'test'} fn# 1 ]} \
            code {[ {[] i32.const 0 global.get 0 table.set 0 \
                        i32.const 0 call_indirect 0 0 end} ]}";

        wah_module_t consumer = {0};
        assert_ok(wah_parse_module_from_spec(&consumer, spec));

        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &consumer, NULL));
        assert_ok(wah_link_module(&ctx, "host", &host_mod));
        assert_ok(wah_instantiate(&ctx));

        wah_value_t result;
        assert_err(wah_call(&ctx, 1, NULL, 0, &result), WAH_ERROR_TRAP);

        wah_free_exec_context(&ctx);
        wah_free_module(&consumer);
        wah_free_module(&host_mod);
    }

    // Regression: wah_link_module/wah_link_context dereferenced ctx before NULL check.
    printf("Test: wah_link_module/wah_link_context NULL ctx returns MISUSE\n");
    {
        wah_module_t dummy = {0};
        assert_err(wah_link_module(NULL, "x", &dummy), WAH_ERROR_MISUSE);

        wah_exec_context_t dummy_ctx = {0};
        assert_err(wah_link_context(NULL, "x", &dummy_ctx), WAH_ERROR_MISUSE);
    }

    // Cross-module data.drop: linked module's function does data.drop on its own
    // data segment. Primary module has 0 data segments. If the interpreter uses
    // ctx->module (primary) instead of fctx->module (linked), data_idx 0 is OOB.
    printf("Testing cross-module data.drop uses linked module's segments...\n");
    {
        wah_module_t linked_mod = {0};
        assert_ok(wah_parse_module_from_spec(&linked_mod, "wasm \
            types {[fn [] []]} \
            funcs {[0]} \
            memories {[limits.i32/1 1]} \
            exports {[{'do_drop'} fn# 0, {'mem'} mem# 0]} \
            datacount {1} \
            code {[{[] data.drop 0 end}]} \
            data {[data.passive {%'AABB'}]}"));

        wah_module_t primary_mod = {0};
        assert_ok(wah_parse_module_from_spec(&primary_mod, "wasm \
            types {[fn [] []]} \
            imports {[{'linked'} {'do_drop'} fn# 0, {'linked'} {'mem'} mem# limits.i32/1 1]} \
            funcs {[0]} \
            exports {[{'run'} fn# 1]} \
            code {[{[] call 0 end}]}"));

        wah_exec_context_t ectx = {0};
        assert_ok(wah_new_exec_context(&ectx, &primary_mod, NULL));
        assert_ok(wah_link_module(&ectx, "linked", &linked_mod));
        assert_ok(wah_instantiate(&ectx));

        wah_value_t result;
        assert_ok(wah_call_by_name(&ectx, "run", NULL, 0, &result));

        wah_free_exec_context(&ectx);
        wah_free_module(&primary_mod);
        wah_free_module(&linked_mod);
    }

    // Cross-module elem.drop: linked module's function does elem.drop on its own
    // element segment. Primary module has 0 element segments.
    printf("Testing cross-module elem.drop uses linked module's segments...\n");
    {
        wah_module_t linked_mod = {0};
        assert_ok(wah_parse_module_from_spec(&linked_mod, "wasm \
            types {[fn [] []]} \
            funcs {[0, 0]} \
            tables {[funcref limits.i32/2 10 10]} \
            exports {[{'do_drop'} fn# 0]} \
            elements {[elem.passive elem.funcref [0]]} \
            code {[\
              {[] elem.drop 0 end}, \
              {[] end} \
            ]}"));

        wah_module_t primary_mod = {0};
        assert_ok(wah_parse_module_from_spec(&primary_mod, "wasm \
            types {[fn [] []]} \
            imports {[{'linked'} {'do_drop'} fn# 0]} \
            funcs {[0]} \
            exports {[{'run'} fn# 1]} \
            code {[{[] call 0 end}]}"));

        wah_exec_context_t ectx = {0};
        assert_ok(wah_new_exec_context(&ectx, &primary_mod, NULL));
        assert_ok(wah_link_module(&ectx, "linked", &linked_mod));
        assert_ok(wah_instantiate(&ectx));

        wah_value_t result;
        assert_ok(wah_call_by_name(&ectx, "run", NULL, 0, &result));

        wah_free_exec_context(&ectx);
        wah_free_module(&primary_mod);
        wah_free_module(&linked_mod);
    }

    // Regression: wah_memory_grow_internal used ctx->memories[mem_idx] instead of
    // fctx->memories[mem_idx] for max_pages / import checks, causing OOB access
    // when the linked context has memories that the primary context does not.
    printf("Test: cross-module memory.grow via wah_link_context (security regression)\n");
    {
        // Provider: has 1 memory (1 page, max 10 pages), exports grow_and_size.
        // grow_and_size(pages_to_grow) -> old_page_count
        const char *provider_spec = "wasm \
            types {[ fn [i32] [i32] ]} \
            funcs {[ 0 ]} \
            memories {[ limits.i32/2 1 10 ]} \
            exports {[ {'grow'} fn# 0 ]} \
            code {[ {[] local.get 0 memory.grow 0 end } ]}";

        // Consumer: NO memory, imports grow from provider, re-exports as wrapper.
        const char *consumer_spec = "wasm \
            types {[ fn [i32] [i32] ]} \
            imports {[ {'p'} {'grow'} fn# 0 ]} \
            funcs {[ 0 ]} \
            exports {[ {'call_grow'} fn# 1 ]} \
            code {[ {[] local.get 0 call 0 end } ]}";

        wah_module_t provider = {0}, consumer = {0};
        assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
        assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

        wah_exec_context_t pctx = {0};
        assert_ok(wah_new_exec_context(&pctx, &provider, NULL));
        assert_ok(wah_instantiate(&pctx));

        wah_exec_context_t cctx = {0};
        assert_ok(wah_new_exec_context(&cctx, &consumer, NULL));
        assert_ok(wah_link_context(&cctx, "p", &pctx));
        assert_ok(wah_instantiate(&cctx));

        // Grow by 1 page. Should return old page count (1) without OOB access.
        wah_value_t arg = {.i32 = 1};
        wah_value_t result;
        assert_ok(wah_call(&cctx, 0, &arg, 1, &result));
        assert_eq_i32(result.i32, 1);  // old page count was 1

        // Grow again by 2 pages. Should return 2 (now 2 pages).
        arg.i32 = 2;
        assert_ok(wah_call(&cctx, 0, &arg, 1, &result));
        assert_eq_i32(result.i32, 2);  // old page count was 2

        wah_free_exec_context(&cctx);
        wah_free_exec_context(&pctx);
        wah_free_module(&consumer);
        wah_free_module(&provider);
    }

    // Regression: TABLE_*_IMPL macros used ctx->tables instead of fctx->tables,
    // causing wrong-table access when the linked context has its own tables.
    printf("Test: cross-module table ops via wah_link_context (security regression)\n");
    {
        // Provider: has 1 table (funcref, min 2, max 10), exports table.size and table.grow.
        // size() -> i32 via table.size
        // grow(delta) -> old_size via table.grow
        const char *provider_spec = "wasm \
            types {[ fn [] [i32], fn [i32] [i32] ]} \
            funcs {[ 0, 1 ]} \
            tables {[ funcref limits.i32/2 2 10 ]} \
            exports {[ {'size'} fn# 0, {'grow'} fn# 1 ]} \
            code {[ \
                {[] table.size 0 end }, \
                {[] ref.null funcref local.get 0 table.grow 0 end } \
            ]}";

        // Consumer: NO table, imports from provider, re-exports as wrappers.
        const char *consumer_spec = "wasm \
            types {[ fn [] [i32], fn [i32] [i32] ]} \
            imports {[ {'p'} {'size'} fn# 0, {'p'} {'grow'} fn# 1 ]} \
            funcs {[ 0, 1 ]} \
            exports {[ {'size'} fn# 2, {'grow'} fn# 3 ]} \
            code {[ \
                {[] call 0 end }, \
                {[] local.get 0 call 1 end } \
            ]}";

        wah_module_t provider = {0}, consumer = {0};
        assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
        assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

        wah_exec_context_t pctx = {0};
        assert_ok(wah_new_exec_context(&pctx, &provider, NULL));
        assert_ok(wah_instantiate(&pctx));

        wah_exec_context_t cctx = {0};
        assert_ok(wah_new_exec_context(&cctx, &consumer, NULL));
        assert_ok(wah_link_context(&cctx, "p", &pctx));
        assert_ok(wah_instantiate(&cctx));

        // table.size should return 2
        wah_value_t result;
        assert_ok(wah_call_by_name(&cctx, "size", NULL, 0, &result));
        assert_eq_i32(result.i32, 2);

        // table.grow by 3 should return old size (2)
        wah_value_t grow_arg = {.i32 = 3};
        assert_ok(wah_call_by_name(&cctx, "grow", &grow_arg, 1, &result));
        assert_eq_i32(result.i32, 2);

        // table.size should now return 5
        assert_ok(wah_call_by_name(&cctx, "size", NULL, 0, &result));
        assert_eq_i32(result.i32, 5);

        wah_free_exec_context(&cctx);
        wah_free_exec_context(&pctx);
        wah_free_module(&consumer);
        wah_free_module(&provider);
    }

    // Regression: wah_bind_frame_module falls back to caller's context when
    // a transitive module (linked into a linked context) is not found in
    // the primary context's linked_modules. The function then executes with
    // wrong globals.
    printf("Test: transitive cross-module call uses correct globals (security regression)\n");
    {
        // Module C: global g=100, exports getG() -> i32 = global.get 0
        const char *spec_c = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0 ]} \
            globals {[ i32 immut i32.const 100 end ]} \
            exports {[ {'getG'} fn# 0 ]} \
            code {[ {[] global.get 0 end } ]}";

        // Module B: imports getG from C, exports wrapG() -> i32 = call getG
        // B has its own global b=200 to verify offset computation.
        const char *spec_b = "wasm \
            types {[ fn [] [i32] ]} \
            imports {[ {'C'} {'getG'} fn# 0 ]} \
            funcs {[ 0 ]} \
            globals {[ i32 immut i32.const 200 end ]} \
            exports {[ {'wrapG'} fn# 1 ]} \
            code {[ {[] call 0 end } ]}";

        // Module A: imports wrapG from B, has its own global a=999.
        // Exports run() -> i32 = call wrapG
        const char *spec_a = "wasm \
            types {[ fn [] [i32] ]} \
            imports {[ {'B'} {'wrapG'} fn# 0 ]} \
            funcs {[ 0 ]} \
            globals {[ i32 immut i32.const 999 end ]} \
            exports {[ {'run'} fn# 1 ]} \
            code {[ {[] call 0 end } ]}";

        wah_module_t mod_c = {0}, mod_b = {0}, mod_a = {0};
        assert_ok(wah_parse_module_from_spec(&mod_c, spec_c));
        assert_ok(wah_parse_module_from_spec(&mod_b, spec_b));
        assert_ok(wah_parse_module_from_spec(&mod_a, spec_a));

        // B instantiated with C linked via wah_link_module.
        wah_exec_context_t bctx = {0};
        assert_ok(wah_new_exec_context(&bctx, &mod_b, NULL));
        assert_ok(wah_link_module(&bctx, "C", &mod_c));
        assert_ok(wah_instantiate(&bctx));

        // A links B_ctx via wah_link_context (B is already instantiated).
        wah_exec_context_t actx = {0};
        assert_ok(wah_new_exec_context(&actx, &mod_a, NULL));
        assert_ok(wah_link_context(&actx, "B", &bctx));
        assert_ok(wah_instantiate(&actx));

        // run() -> wrapG() -> getG() should return 100 (C's global).
        // Before fix: returns 999 (A's global) or crashes due to wrong context.
        wah_value_t result;
        assert_ok(wah_call_by_name(&actx, "run", NULL, 0, &result));
        assert_eq_i32(result.i32, 100);

        wah_free_exec_context(&actx);
        wah_free_exec_context(&bctx);
        wah_free_module(&mod_a);
        wah_free_module(&mod_b);
        wah_free_module(&mod_c);
    }

    // Bug: global import subtype check uses wah_type_is_subtype with the linked
    // module only, so the importer's concrete type index is resolved in the
    // wrong module.  struct{i32} vs struct{i64} at the same type index passes.
    printf("Test: cross-module global import rejects incompatible GC types\n");
    {
        // exporter: type 0 = struct{i32 mut}, mutable global of type (ref null 0)
        wah_module_t mod_exp = {0};
        assert_ok(wah_parse_module_from_spec(&mod_exp, "wasm \
            types {[struct [i32 mut]]} \
            globals {[type.ref.null 0 mut ref.null 0 end]} \
            exports {[{'g'} global# 0]}"));

        // importer: type 0 = struct{i64 mut}, imports mutable global of type (ref null 0)
        wah_module_t mod_imp = {0};
        assert_ok(wah_parse_module_from_spec(&mod_imp, "wasm \
            types {[struct [i64 mut], fn [] []]} \
            imports {[{'E'} {'g'} export.global type.ref.null 0 mut]} \
            funcs {[1]} \
            exports {[{'run'} fn# 0]} \
            code {[{[] end}]}"));

        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &mod_imp, NULL));
        wah_link_module(&ctx, "E", &mod_exp);
        wah_error_t err = wah_instantiate(&ctx);
        assert_err(err, WAH_ERROR_LINK_FAILED);

        wah_free_exec_context(&ctx);
        wah_free_module(&mod_imp);
        wah_free_module(&mod_exp);
    }

    // Bug: table import element type uses == instead of cross-module comparison.
    // struct{i32} vs struct{i64} at the same type index passes with ==.
    printf("Test: cross-module table import rejects incompatible element types\n");
    {
        // exporter: type 0 = struct{i32 mut}, table of (ref null 0)
        wah_module_t mod_exp = {0};
        assert_ok(wah_parse_module_from_spec(&mod_exp, "wasm \
            types {[struct [i32 mut]]} \
            tables {[type.ref.null 0 limits.i32/1 10]} \
            exports {[{'t'} table# 0]}"));

        // importer: type 0 = struct{i64 mut}, imports table of (ref null 0)
        wah_module_t mod_imp = {0};
        assert_ok(wah_parse_module_from_spec(&mod_imp, "wasm \
            types {[struct [i64 mut], fn [] []]} \
            imports {[{'E'} {'t'} table# type.ref.null 0 limits.i32/1 5]} \
            funcs {[1]} \
            exports {[{'run'} fn# 0]} \
            code {[{[] end}]}"));

        wah_exec_context_t ectx = {0};
        assert_ok(wah_new_exec_context(&ectx, &mod_exp, NULL));
        assert_ok(wah_instantiate(&ectx));

        wah_exec_context_t ictx = {0};
        assert_ok(wah_new_exec_context(&ictx, &mod_imp, NULL));
        assert_ok(wah_link_context(&ictx, "E", &ectx));
        wah_error_t err = wah_instantiate(&ictx);
        assert_err(err, WAH_ERROR_LINK_FAILED);

        wah_free_exec_context(&ictx);
        wah_free_exec_context(&ectx);
        wah_free_module(&mod_imp);
        wah_free_module(&mod_exp);
    }

    // Bug: re-exported table imports skip element type checking.
    // When a linked module re-exports an imported table, the type compatibility
    // check was only performed for locally-defined tables, allowing type confusion.
    printf("Test: re-exported table import checks element type\n");
    {
        // Module A: imports funcref table from env, re-exports it
        wah_module_t mod_a = {0};
        assert_ok(wah_parse_module_from_spec(&mod_a, "wasm \
            imports {[{'env'} {'t'} table# funcref limits.i32/1 1]} \
            exports {[{'t'} table# 0]}"));

        // Module B: imports table from A as externref (type mismatch!)
        wah_module_t mod_b = {0};
        assert_ok(wah_parse_module_from_spec(&mod_b, "wasm \
            types {[fn [] [i32]]} \
            imports {[{'A'} {'t'} table# externref limits.i32/1 1]} \
            funcs {[0]} \
            exports {[{'run'} fn# 0]} \
            code {[{[] i32.const 0 table.get 0 ref.is_null end}]}"));

        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &mod_b, NULL));
        assert_ok(wah_link_module(&ctx, "A", &mod_a));
        wah_error_t err = wah_instantiate(&ctx);
        assert_err(err, WAH_ERROR_LINK_FAILED);

        wah_free_exec_context(&ctx);
        wah_free_module(&mod_b);
        wah_free_module(&mod_a);
    }

    // Bug: re-exported memory imports skip addr_type checking.
    printf("Test: re-exported memory import checks addr_type\n");
    {
        // Module A: imports i32-addressed memory from env, re-exports it
        wah_module_t mod_a = {0};
        assert_ok(wah_parse_module_from_spec(&mod_a, "wasm \
            imports {[{'env'} {'m'} mem# limits.i32/1 1]} \
            exports {[{'m'} mem# 0]}"));

        // Module B: imports memory from A as memory64 (addr_type mismatch!)
        wah_module_t mod_b = {0};
        assert_ok(wah_parse_module_from_spec(&mod_b, "wasm \
            types {[fn [] [i32]]} \
            imports {[{'A'} {'m'} mem# limits.i64/1 1]} \
            funcs {[0]} \
            exports {[{'run'} fn# 0]} \
            code {[{[] i32.const 42 end}]}"));

        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &mod_b, NULL));
        assert_ok(wah_link_module(&ctx, "A", &mod_a));
        wah_error_t err = wah_instantiate(&ctx);
        assert_err(err, WAH_ERROR_LINK_FAILED);

        wah_free_exec_context(&ctx);
        wah_free_module(&mod_b);
        wah_free_module(&mod_a);
    }

    // Bug: wah_eval_const_expr used ctx->module (primary) when evaluating
    // linked module global init expressions, so global.get in a linked module's
    // init expr could reference the wrong module's global index space.
    printf("Test: linked module const expr uses correct module context\n");
    {
        // lib: global 0 = 200, global 1 = global.get 0 (should be 200)
        // main: global 0 = 999, imports lib.getg1
        // If wah_eval_const_expr uses main's module, global.get 0 in lib's
        // init expr would still work (globals are swapped), but ctx->module
        // must be restored after the loop to avoid corrupting later steps.
        wah_module_t lib = {0};
        assert_ok(wah_parse_module_from_spec(&lib, "wasm \
            types {[fn [] [i32]]} \
            funcs {[0]} \
            globals {[ \
                i32 immut i32.const 200 end, \
                i32 immut global.get 0 end \
            ]} \
            exports {[{'getg1'} fn# 0]} \
            code {[{[] global.get 1 end}]}"));

        wah_module_t main_mod = {0};
        assert_ok(wah_parse_module_from_spec(&main_mod, "wasm \
            types {[fn [] [i32]]} \
            imports {[{'lib'} {'getg1'} fn# 0]} \
            funcs {[0]} \
            globals {[i32 immut i32.const 999 end]} \
            exports {[{'run'} fn# 1]} \
            code {[{[] call 0 end}]}"));

        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &main_mod, NULL));
        wah_link_module(&ctx, "lib", &lib);
        assert_ok(wah_instantiate(&ctx));

        wah_value_t result;
        assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
        assert_eq_i32(result.i32, 200);

        wah_free_exec_context(&ctx);
        wah_free_module(&main_mod);
        wah_free_module(&lib);
    }

    printf("All linkage tests passed!\n");
    return 0;
}
