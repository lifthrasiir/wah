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

void sink_i64(wah_call_context_t *ctx, void *userdata) {
    (void)ctx; (void)userdata;
}

int danger_called = 0;
void danger_host_func(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    danger_called = 1;
    wah_return_i32(ctx, -999);
}

int safe_called = 0;
void safe_host_func(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    safe_called = 1;
    wah_return_i32(ctx, 123);
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

    // Regression: linked module with imported globals had wrong slot offset for funcref
    // global conversion. The prefuncref sentinel was written at lg_offset + k instead of
    // lg_offset + import_global_count + k, leaving the funcref global unconverted.
    printf("Testing linked module funcref global with imported globals...\n");
    {
        // Module C (host): exports a dummy i32 global and "getConst" () -> i32 returning 42.
        wah_module_t mod_c = {0};
        assert_ok(wah_new_module(&mod_c, NULL));
        assert_ok(wah_export_global_i32(&mod_c, "dummyGlobal", 0, false));
        assert_ok(wah_export_func(&mod_c, "getConst", "() -> i32",
                                         simple_host_func, NULL, NULL));

        // Module B: imports "moduleC"."dummyGlobal" (global idx 0, i32 immut)
        //           and "moduleC"."getConst" (func idx 0).
        // local global[0] (global idx 1) = funcref mut ref.func 0 (= the imported getConst).
        // local func 0 (func idx 1): reads global[1], stores into table[0][0], call_indirect.
        const char *spec_b = "wasm \
            types {[ fn [] [i32] ]} \
            imports {[ {'moduleC'} {'dummyGlobal'} global# i32 immut, \
                       {'moduleC'} {'getConst'} fn# 0 ]} \
            funcs {[ 0 ]} \
            tables {[ funcref limits.i32/1 1 ]} \
            globals {[ funcref mut ref.func 0 end ]} \
            exports {[ {'callViaRef'} fn# 1 ]} \
            code {[ \
                {[] i32.const 0 global.get 1 table.set 0 i32.const 0 call_indirect 0 0 end} \
            ]}";

        // Module A: imports callViaRef from moduleB.
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

    // Bug: return_call to a host function from the outermost wasm frame
    // (call_depth == 1) caused RELOAD_FRAME to jump to cleanup because
    // call_depth decremented to 0, silently skipping the host function.
    printf("Test: return_call to host function from outermost frame\n");
    {
        wah_module_t host_mod = {0};
        assert_ok(wah_new_module(&host_mod, NULL));
        assert_ok(wah_export_func(&host_mod, "host_fn", "() -> i32", simple_host_func, NULL, NULL));

        wah_module_t wasm_mod = {0};
        assert_ok(wah_parse_module_from_spec(&wasm_mod, "wasm \
            types {[fn [] [i32]]} \
            imports {[{'host'} {'host_fn'} fn# 0]} \
            funcs {[0]} \
            exports {[{'go'} fn# 1]} \
            code {[{[] return_call 0 end}]}"));

        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &wasm_mod, NULL));
        assert_ok(wah_link_module(&ctx, "host", &host_mod));
        assert_ok(wah_instantiate(&ctx));

        host_func_called = 0;
        wah_value_t result = {0};
        assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
        assert_true(host_func_called);
        assert_eq_i32(result.i32, 42);

        wah_free_exec_context(&ctx);
        wah_free_module(&wasm_mod);
        wah_free_module(&host_mod);
    }

    // Regression: call_indirect on a host function whose type is a subtype of
    // the expected type (via contravariant params) should succeed.  The old code
    // used direct `!=` for host function param/result types, which traps when
    // structurally compatible but non-identical abstract ref types are involved.
    printf("Test: call_indirect host function subtype (contravariant params)\n");
    {
        wah_module_t host_mod = {0};
        assert_ok(wah_new_module(&host_mod, NULL));
        // Host fn accepts anyref (= ref null any) and returns i32.
        assert_ok(wah_export_func(&host_mod, "hostFn", "(anyref) -> i32", simple_host_func, NULL, NULL));

        // Consumer module:
        // type 0 = fn () -> (i32)             ;; exported function type
        // type 1 = fn (structref) -> (i32)    ;; call_indirect expected type
        // type 2 = fn (anyref) -> (i32)       ;; import type (host fn actual type)
        // fn(anyref)->i32 <: fn(structref)->i32  because structref <: anyref (contravariant params)
        const char *spec = "wasm \
            types {[ fn [] [i32], fn [structref] [i32], fn [anyref] [i32] ]} \
            imports {[ {'host'} {'hostFn'} fn# 2 ]} \
            funcs {[ 0 ]} \
            tables {[ funcref limits.i32/1 1 ]} \
            globals {[ funcref mut ref.func 0 end ]} \
            exports {[ {'test'} fn# 1 ]} \
            code {[ {[] i32.const 0 global.get 0 table.set 0 \
                        ref.null structref i32.const 0 call_indirect 1 0 end} ]}";

        wah_module_t consumer = {0};
        assert_ok(wah_parse_module_from_spec(&consumer, spec));

        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &consumer, NULL));
        assert_ok(wah_link_module(&ctx, "host", &host_mod));
        assert_ok(wah_instantiate(&ctx));

        wah_value_t result;
        assert_ok(wah_call(&ctx, 1, NULL, 0, &result));
        assert_eq_i32(result.i32, 42);

        wah_free_exec_context(&ctx);
        wah_free_module(&consumer);
        wah_free_module(&host_mod);
    }

    // Regression: importing the same memory twice via wah_link_context, then growing
    // one slot, left the alias slot with a stale pointer/size.
    printf("Test: duplicate memory import alias updated on grow (security regression)\n");
    {
        // Provider: 1 memory (1 page, max 10), exports it.
        wah_module_t provider = {0};
        assert_ok(wah_new_module(&provider, NULL));
        assert_ok(wah_export_memory(&provider, "mem", 1, 10));

        wah_exec_context_t pctx = {0};
        assert_ok(wah_new_exec_context(&pctx, &provider, NULL));
        assert_ok(wah_instantiate(&pctx));

        // Consumer: imports "mem" twice, exports grow0 (memory.grow 0) and
        // size1 (memory.size 1) to observe the alias.
        const char *consumer_spec = "wasm \
            types {[ fn [i32] [i32], fn [] [i32] ]} \
            imports {[ {'p'} {'mem'} mem# limits.i32/2 1 10, \
                       {'p'} {'mem'} mem# limits.i32/2 1 10 ]} \
            funcs {[ 0, 1 ]} \
            exports {[ {'grow0'} fn# 0, {'size1'} fn# 1 ]} \
            code {[ {[] local.get 0 memory.grow 0 end }, \
                    {[] memory.size 1 end } ]}";

        wah_module_t consumer = {0};
        assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

        wah_exec_context_t cctx = {0};
        assert_ok(wah_new_exec_context(&cctx, &consumer, NULL));
        assert_ok(wah_link_context(&cctx, "p", &pctx));
        assert_ok(wah_instantiate(&cctx));

        // Initial: both slots should report 1 page.
        wah_value_t result;
        assert_ok(wah_call_by_name(&cctx, "size1", NULL, 0, &result));
        assert_eq_i32(result.i32, 1);

        // Grow memory 0 by 2 pages.
        wah_value_t arg = {.i32 = 2};
        assert_ok(wah_call_by_name(&cctx, "grow0", &arg, 1, &result));
        assert_eq_i32(result.i32, 1);  // old page count

        // memory 1 (alias) must also see 3 pages now.
        assert_ok(wah_call_by_name(&cctx, "size1", NULL, 0, &result));
        assert_eq_i32(result.i32, 3);

        wah_free_exec_context(&cctx);
        wah_free_exec_context(&pctx);
        wah_free_module(&consumer);
        wah_free_module(&provider);
    }

    // Regression: same bug for tables — duplicate table import alias not updated on grow.
    printf("Test: duplicate table import alias updated on grow (security regression)\n");
    {
        // Provider: 1 table (funcref, min 1, max 10), exports it.
        wah_module_t provider = {0};
        assert_ok(wah_parse_module_from_spec(&provider, "wasm \
            tables {[ funcref limits.i32/2 1 10 ]} \
            exports {[ {'tbl'} table# 0 ]}"));

        wah_exec_context_t pctx = {0};
        assert_ok(wah_new_exec_context(&pctx, &provider, NULL));
        assert_ok(wah_instantiate(&pctx));

        // Consumer: imports "tbl" twice, exports grow0 (table.grow 0) and
        // size1 (table.size 1).
        const char *consumer_spec = "wasm \
            types {[ fn [i32] [i32], fn [] [i32] ]} \
            imports {[ {'p'} {'tbl'} table# funcref limits.i32/2 1 10, \
                       {'p'} {'tbl'} table# funcref limits.i32/2 1 10 ]} \
            funcs {[ 0, 1 ]} \
            exports {[ {'grow0'} fn# 0, {'size1'} fn# 1 ]} \
            code {[ {[] ref.null funcref local.get 0 table.grow 0 end }, \
                    {[] table.size 1 end } ]}";

        wah_module_t consumer = {0};
        assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

        wah_exec_context_t cctx = {0};
        assert_ok(wah_new_exec_context(&cctx, &consumer, NULL));
        assert_ok(wah_link_context(&cctx, "p", &pctx));
        assert_ok(wah_instantiate(&cctx));

        // Initial: both slots should report size 1.
        wah_value_t result;
        assert_ok(wah_call_by_name(&cctx, "size1", NULL, 0, &result));
        assert_eq_i32(result.i32, 1);

        // Grow table 0 by 2.
        wah_value_t arg = {.i32 = 2};
        assert_ok(wah_call_by_name(&cctx, "grow0", &arg, 1, &result));
        assert_eq_i32(result.i32, 1);  // old size

        // table 1 (alias) must also see size 3 now.
        assert_ok(wah_call_by_name(&cctx, "size1", NULL, 0, &result));
        assert_eq_i32(result.i32, 3);

        wah_free_exec_context(&cctx);
        wah_free_exec_context(&pctx);
        wah_free_module(&consumer);
        wah_free_module(&provider);
    }

    // Regression: linked module internal call must use the linked module's
    // function index space, not the primary module's function_table.
    printf("Test: linked module internal call uses correct function index space (security regression)\n");
    {
        // Provider module: func 0 returns 111, func 1 (exported as "safe") calls func 0.
        wah_module_t provider = {0};
        assert_ok(wah_parse_module_from_spec(&provider, "wasm \
            types {[ fn [i32] [i32] ]} \
            funcs {[0, 0]} \
            exports {[ {'safe'} fn# 1 ]} \
            code {[ \
                {[] i32.const 111 end }, \
                {[] local.get 0 call 0 end } \
            ]}"));

        // Primary module: imports host.danger as func 0, provider.safe as func 1.
        // Exports "run" = func 2 which calls func 1 (provider.safe).
        wah_module_t primary = {0};
        assert_ok(wah_parse_module_from_spec(&primary, "wasm \
            types {[ fn [i32] [i32] ]} \
            imports {[ \
                {'host'} {'danger'} fn# 0, \
                {'provider'} {'safe'} fn# 0 \
            ]} \
            funcs {[0]} \
            exports {[ {'run'} fn# 2 ]} \
            code {[ \
                {[] local.get 0 call 1 end } \
            ]}"));

        wah_module_t host_mod = {0};
        wah_new_module(&host_mod, NULL);
        wah_export_func(&host_mod, "danger", "(i32) -> (i32)", danger_host_func, NULL, NULL);

        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &primary, NULL));
        assert_ok(wah_link_module(&ctx, "host", &host_mod));
        assert_ok(wah_link_module(&ctx, "provider", &provider));
        assert_ok(wah_instantiate(&ctx));

        danger_called = 0;
        wah_value_t arg = {.i32 = 42};
        wah_value_t result;
        assert_ok(wah_call_by_name(&ctx, "run", &arg, 1, &result));

        // provider.safe calls provider.func0 which returns 111.
        // If the bug is present, it would call host.danger instead.
        assert_eq_i32(danger_called, 0);
        assert_eq_i32(result.i32, 111);

        wah_free_exec_context(&ctx);
        wah_free_module(&primary);
        wah_free_module(&provider);
        wah_free_module(&host_mod);
    }

    // Regression: linked module that imports a function and calls it directly
    // must have its import slot properly resolved, not left as a zeroed entry.
    printf("Test: linked module import function slot resolved correctly (security regression)\n");
    {
        // "impl" module: exports "get42" returning 42.
        wah_module_t impl_mod = {0};
        assert_ok(wah_parse_module_from_spec(&impl_mod, "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[0]} \
            exports {[ {'get42'} fn# 0 ]} \
            code {[ {[] i32.const 42 end } ]}"));

        // "middle" module: imports impl.get42 as func 0, has local func 1 that calls func 0.
        // Exports "call_import" = func 1.
        wah_module_t middle_mod = {0};
        assert_ok(wah_parse_module_from_spec(&middle_mod, "wasm \
            types {[ fn [] [i32] ]} \
            imports {[ {'impl'} {'get42'} fn# 0 ]} \
            funcs {[0]} \
            exports {[ {'call_import'} fn# 1 ]} \
            code {[ {[] call 0 end } ]}"));

        // Primary module: imports middle.call_import, exports "run" that calls it.
        wah_module_t primary2 = {0};
        assert_ok(wah_parse_module_from_spec(&primary2, "wasm \
            types {[ fn [] [i32] ]} \
            imports {[ {'middle'} {'call_import'} fn# 0 ]} \
            funcs {[0]} \
            exports {[ {'run'} fn# 1 ]} \
            code {[ {[] call 0 end } ]}"));

        wah_exec_context_t ctx2 = {0};
        assert_ok(wah_new_exec_context(&ctx2, &primary2, NULL));
        assert_ok(wah_link_module(&ctx2, "impl", &impl_mod));
        assert_ok(wah_link_module(&ctx2, "middle", &middle_mod));
        assert_ok(wah_instantiate(&ctx2));

        wah_value_t result2;
        assert_ok(wah_call_by_name(&ctx2, "run", NULL, 0, &result2));
        assert_eq_i32(result2.i32, 42);

        wah_free_exec_context(&ctx2);
        wah_free_module(&primary2);
        wah_free_module(&middle_mod);
        wah_free_module(&impl_mod);
    }

    // Regression: linked module with its own memory must use its own memory,
    // not the primary module's memory array.
    printf("Test: linked module uses own memory space (security regression)\n");
    {
        // "memmod" module: has 1 page memory, exports "store" that stores i32 at offset 0,
        // and "load" that loads from offset 0.
        wah_module_t memmod = {0};
        assert_ok(wah_parse_module_from_spec(&memmod, "wasm \
            types {[ fn [i32] [], fn [] [i32] ]} \
            funcs {[0, 1]} \
            memories {[ limits.i32/2 1 1 ]} \
            exports {[ {'store'} fn# 0, {'load'} fn# 1 ]} \
            code {[ \
                {[] i32.const 0 local.get 0 i32.store 0 0 end }, \
                {[] i32.const 0 i32.load 0 0 end } \
            ]}"));

        // Primary has no memory. Imports memmod.store and memmod.load.
        wah_module_t primary3 = {0};
        assert_ok(wah_parse_module_from_spec(&primary3, "wasm \
            types {[ fn [i32] [], fn [] [i32] ]} \
            imports {[ {'memmod'} {'store'} fn# 0, {'memmod'} {'load'} fn# 1 ]} \
            funcs {[0, 1]} \
            exports {[ {'do_store'} fn# 2, {'do_load'} fn# 3 ]} \
            code {[ \
                {[] local.get 0 call 0 end }, \
                {[] call 1 end } \
            ]}"));

        wah_exec_context_t ctx3 = {0};
        assert_ok(wah_new_exec_context(&ctx3, &primary3, NULL));
        assert_ok(wah_link_module(&ctx3, "memmod", &memmod));
        assert_ok(wah_instantiate(&ctx3));

        wah_value_t store_arg = {.i32 = 12345};
        assert_ok(wah_call_by_name(&ctx3, "do_store", &store_arg, 1, NULL));

        wah_value_t load_result;
        assert_ok(wah_call_by_name(&ctx3, "do_load", NULL, 0, &load_result));
        assert_eq_i32(load_result.i32, 12345);

        wah_free_exec_context(&ctx3);
        wah_free_module(&primary3);
        wah_free_module(&memmod);
    }

    // Regression: linked module with signature-mismatched imports must not
    // silently fall through to local function 0.
    printf("Test: linked module import with type mismatch rejected (security regression)\n");
    {
        // "provider" exports "fn_i64" with signature () -> i64
        wah_module_t prov = {0};
        assert_ok(wah_parse_module_from_spec(&prov, "wasm \
            types {[ fn [] [i64] ]} \
            funcs {[0]} \
            exports {[ {'fn_i64'} fn# 0 ]} \
            code {[ {[] i64.const 999 end } ]}"));

        // "consumer" imports provider.fn_i64 as () -> i32 (type mismatch)
        // Has local func 0 that calls import func 0.
        wah_module_t consumer = {0};
        assert_ok(wah_parse_module_from_spec(&consumer, "wasm \
            types {[ fn [] [i32] ]} \
            imports {[ {'provider'} {'fn_i64'} fn# 0 ]} \
            funcs {[0]} \
            exports {[ {'call_it'} fn# 1 ]} \
            code {[ {[] call 0 end } ]}"));

        // Primary just calls consumer.call_it
        wah_module_t primary4 = {0};
        assert_ok(wah_parse_module_from_spec(&primary4, "wasm \
            types {[ fn [] [i32] ]} \
            imports {[ {'consumer'} {'call_it'} fn# 0 ]} \
            funcs {[0]} \
            exports {[ {'run'} fn# 1 ]} \
            code {[ {[] call 0 end } ]}"));

        wah_exec_context_t ctx4 = {0};
        assert_ok(wah_new_exec_context(&ctx4, &primary4, NULL));
        assert_ok(wah_link_module(&ctx4, "provider", &prov));
        assert_ok(wah_link_module(&ctx4, "consumer", &consumer));
        // Instantiation should fail because consumer imports () -> i32
        // but provider exports () -> i64 -- type mismatch.
        wah_error_t err4 = wah_instantiate(&ctx4);
        assert(err4 == WAH_ERROR_LINK_FAILED);

        wah_free_exec_context(&ctx4);
        wah_free_module(&primary4);
        wah_free_module(&consumer);
        wah_free_module(&prov);
    }

    // Test: Linked module with imported mutable global
    // Regression: imported mutable globals in linked modules must have proper
    // slots in the global index space. Previously, only local globals were
    // allocated, causing GLOBAL_SET_INDIRECT to interpret a local global's value
    // as a pointer (arbitrary write).
    printf("Test: Linked module imported mutable global\n");
    {
        // Primary module: has a mutable i64 global, exports it as "g",
        // imports a function "write" from linked module "L",
        // and has a "read_g" function to read back the global.
        wah_module_t primary = {0};
        assert_ok(wah_parse_module_from_spec(&primary, "wasm \
            types {[ fn [i64] [], fn [] [i64] ]} \
            imports {[ {'L'} {'write'} fn# 0 ]} \
            funcs {[ 0, 1 ]} \
            globals {[ i64 mut i64.const 100 end ]} \
            exports {[ {'run'} fn# 1, {'g'} export.global 0, {'read_g'} fn# 2 ]} \
            code {[ {[] local.get 0 call 0 end}, {[] global.get 0 end} ]}"));

        // Linked module "L": imports mutable i64 global from "primary" (field "g"),
        // has a local i64 global (to verify no aliasing), exports "write" that
        // sets the imported global.
        wah_module_t linked = {0};
        assert_ok(wah_parse_module_from_spec(&linked, "wasm \
            types {[ fn [i64] [] ]} \
            imports {[ {'primary'} {'g'} export.global i64 mut ]} \
            funcs {[ 0 ]} \
            globals {[ i64 immut i64.const 999 end ]} \
            exports {[ {'write'} fn# 0 ]} \
            code {[ {[] local.get 0 global.set 0 end} ]}"));

        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &primary, NULL));
        assert_ok(wah_link_module(&ctx, "L", &linked));
        assert_ok(wah_link_module(&ctx, "primary", &primary));
        assert_ok(wah_instantiate(&ctx));

        // Call write(42) - should set primary's mutable global to 42
        wah_value_t arg = { .i64 = 42 };
        assert_ok(wah_call_by_name(&ctx, "run", &arg, 1, NULL));

        // Read back the global through primary's read_g function
        wah_value_t result;
        assert_ok(wah_call_by_name(&ctx, "read_g", NULL, 0, &result));
        assert_eq_i64(result.i64, 42);

        wah_free_exec_context(&ctx);
        wah_free_module(&primary);
        wah_free_module(&linked);
    }

    // Test: Linked module imported mutable global - unresolvable import should fail
    printf("Test: Linked module unresolvable global import\n");
    {
        wah_module_t primary = {0};
        assert_ok(wah_parse_module_from_spec(&primary, "wasm \
            types {[ fn [i64] [] ]} \
            imports {[ {'L'} {'write'} fn# 0 ]} \
            funcs {[ 0 ]} \
            exports {[ {'run'} fn# 1 ]} \
            code {[ {[] local.get 0 call 0 end} ]}"));

        // Linked module imports from "nowhere" which is not linked
        wah_module_t linked = {0};
        assert_ok(wah_parse_module_from_spec(&linked, "wasm \
            types {[ fn [i64] [] ]} \
            imports {[ {'nowhere'} {'g'} export.global i64 mut ]} \
            funcs {[ 0 ]} \
            exports {[ {'write'} fn# 0 ]} \
            code {[ {[] local.get 0 global.set 0 end} ]}"));

        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &primary, NULL));
        assert_ok(wah_link_module(&ctx, "L", &linked));
        wah_error_t err = wah_instantiate(&ctx);
        assert(err == WAH_ERROR_LINK_FAILED);

        wah_free_exec_context(&ctx);
        wah_free_module(&primary);
        wah_free_module(&linked);
    }

    // Regression: linked module global imports must be type-checked. Without
    // this, a linked module can import a primary funcref global as mutable i64,
    // write an arbitrary integer into it, and later make call_indirect treat
    // that integer as a function reference pointer.
    printf("Test: linked module global import type mismatch rejected (security regression)\n");
    {
        wah_module_t primary = {0};
        assert_ok(wah_parse_module_from_spec(&primary, "wasm \
            types {[ fn [] [] ]} \
            imports {[ {'L'} {'poison'} fn# 0 ]} \
            funcs {[ 0 ]} \
            globals {[ funcref mut ref.null funcref end ]} \
            exports {[ {'g'} export.global 0, {'run'} fn# 1 ]} \
            code {[ {[] call 0 end} ]}"));

        wah_module_t linked = {0};
        assert_ok(wah_parse_module_from_spec(&linked, "wasm \
            types {[ fn [] [] ]} \
            imports {[ {'primary'} {'g'} export.global i64 mut ]} \
            funcs {[ 0 ]} \
            exports {[ {'poison'} fn# 0 ]} \
            code {[ {[] i64.const 1048576 global.set 0 end} ]}"));

        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &primary, NULL));
        assert_ok(wah_link_module(&ctx, "L", &linked));
        assert_ok(wah_link_module(&ctx, "primary", &primary));
        assert_err(wah_instantiate(&ctx), WAH_ERROR_LINK_FAILED);

        wah_free_exec_context(&ctx);
        wah_free_module(&primary);
        wah_free_module(&linked);
    }

    // Test: Linked module with imported immutable global (value copy)
    printf("Test: Linked module imported immutable global\n");
    {
        wah_module_t primary = {0};
        assert_ok(wah_parse_module_from_spec(&primary, "wasm \
            types {[ fn [] [i64] ]} \
            imports {[ {'L'} {'read'} fn# 0 ]} \
            funcs {[ 0 ]} \
            globals {[ i64 immut i64.const 777 end ]} \
            exports {[ {'run'} fn# 1, {'g'} export.global 0 ]} \
            code {[ {[] call 0 end} ]}"));

        // Linked module: imports immutable i64 global, exports a function that reads it
        wah_module_t linked = {0};
        assert_ok(wah_parse_module_from_spec(&linked, "wasm \
            types {[ fn [] [i64] ]} \
            imports {[ {'primary'} {'g'} export.global i64 immut ]} \
            funcs {[ 0 ]} \
            exports {[ {'read'} fn# 0 ]} \
            code {[ {[] global.get 0 end} ]}"));

        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &primary, NULL));
        assert_ok(wah_link_module(&ctx, "L", &linked));
        assert_ok(wah_link_module(&ctx, "primary", &primary));
        assert_ok(wah_instantiate(&ctx));

        wah_value_t result;
        assert_ok(wah_call_by_name(&ctx, "run", NULL, 0, &result));
        assert_eq_i64(result.i64, 777);

        wah_free_exec_context(&ctx);
        wah_free_module(&primary);
        wah_free_module(&linked);
    }

    // Regression: linked module internal context must verify host import param/result
    // types, not just counts. Without this, a type-mismatched host import (e.g. i32
    // vs i64) would silently succeed and allow type confusion at runtime.
    printf("Test: linked module host import type mismatch rejected\n");
    {
        // Host module: exports sink with signature (i64) -> ()
        wah_module_t host_mod = {0};
        assert_ok(wah_new_module(&host_mod, NULL));
        assert_ok(wah_export_func(&host_mod, "sink", "(i64) -> ()", sink_i64, NULL, NULL));

        // Linked wasm module: imports host.sink as (i32) -> () -- type mismatch!
        // Exports "call_sink" that forwards an i32 arg to the import.
        wah_module_t linked = {0};
        assert_ok(wah_parse_module_from_spec(&linked, "wasm \
            types {[ fn [i32] [] ]} \
            imports {[ {'host'} {'sink'} fn# 0 ]} \
            funcs {[0]} \
            exports {[ {'call_sink'} fn# 1 ]} \
            code {[ {[] local.get 0 call 0 end } ]}"));

        // Primary module: imports linked.call_sink and exports "run".
        wah_module_t primary = {0};
        assert_ok(wah_parse_module_from_spec(&primary, "wasm \
            types {[ fn [i32] [] ]} \
            imports {[ {'linked'} {'call_sink'} fn# 0 ]} \
            funcs {[0]} \
            exports {[ {'run'} fn# 1 ]} \
            code {[ {[] local.get 0 call 0 end } ]}"));

        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &primary, NULL));
        assert_ok(wah_link_module(&ctx, "host", &host_mod));
        assert_ok(wah_link_module(&ctx, "linked", &linked));
        assert_err(wah_instantiate(&ctx), WAH_ERROR_LINK_FAILED);

        wah_free_exec_context(&ctx);
        wah_free_module(&primary);
        wah_free_module(&linked);
        wah_free_module(&host_mod);
    }

    // Bug: linked module with tags gets a partial ictx early (for tag instances),
    // causing later function import resolution to be skipped. Import slots remain
    // zeroed, bypassing type checks and dispatching to wrong functions.
    printf("Test: linked module with tag resolves function imports correctly\n");
    {
        // Host module exports sink: (i64) -> ()
        wah_module_t host_mod = {0};
        assert_ok(wah_new_module(&host_mod, NULL));
        assert_ok(wah_export_func(&host_mod, "sink", "(i64) -> ()", sink_i64, NULL, NULL));

        // Linked module: imports host.sink as (i32)->() [TYPE MISMATCH], has a tag,
        // exports call_sink which calls the import.
        wah_module_t linked = {0};
        assert_ok(wah_parse_module_from_spec(&linked, "wasm \
            types {[ fn [i32] [], fn [] [] ]} \
            imports {[ {'host'} {'sink'} fn# 0 ]} \
            funcs {[0]} \
            tags {[ tag.type# 1 ]} \
            exports {[ {'call_sink'} fn# 1 ]} \
            code {[ {[] local.get 0 call 0 end } ]}"));

        // Primary module: imports linked.call_sink.
        wah_module_t primary = {0};
        assert_ok(wah_parse_module_from_spec(&primary, "wasm \
            types {[ fn [i32] [] ]} \
            imports {[ {'linked'} {'call_sink'} fn# 0 ]} \
            funcs {[0]} \
            exports {[ {'run'} fn# 1 ]} \
            code {[ {[] local.get 0 call 0 end } ]}"));

        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &primary, NULL));
        assert_ok(wah_link_module(&ctx, "host", &host_mod));
        assert_ok(wah_link_module(&ctx, "linked", &linked));
        assert_err(wah_instantiate(&ctx), WAH_ERROR_LINK_FAILED);

        wah_free_exec_context(&ctx);
        wah_free_module(&primary);
        wah_free_module(&linked);
        wah_free_module(&host_mod);
    }

    // Same scenario but with matching types — should succeed and call through.
    printf("Test: linked module with tag + correct imports calls through\n");
    {
        wah_module_t host_mod = {0};
        assert_ok(wah_new_module(&host_mod, NULL));
        assert_ok(wah_export_func(&host_mod, "sink", "(i32) -> ()", sink_i64, NULL, NULL));

        // Linked module: imports host.sink as (i32)->() [matches], has a tag,
        // exports call_sink.
        wah_module_t linked = {0};
        assert_ok(wah_parse_module_from_spec(&linked, "wasm \
            types {[ fn [i32] [], fn [] [] ]} \
            imports {[ {'host'} {'sink'} fn# 0 ]} \
            funcs {[0]} \
            tags {[ tag.type# 1 ]} \
            exports {[ {'call_sink'} fn# 1 ]} \
            code {[ {[] local.get 0 call 0 end } ]}"));

        // Primary module: imports linked.call_sink and calls it.
        wah_module_t primary = {0};
        assert_ok(wah_parse_module_from_spec(&primary, "wasm \
            types {[ fn [i32] [] ]} \
            imports {[ {'linked'} {'call_sink'} fn# 0 ]} \
            funcs {[0]} \
            exports {[ {'run'} fn# 1 ]} \
            code {[ {[] local.get 0 call 0 end } ]}"));

        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &primary, NULL));
        assert_ok(wah_link_module(&ctx, "host", &host_mod));
        assert_ok(wah_link_module(&ctx, "linked", &linked));
        assert_ok(wah_instantiate(&ctx));

        wah_value_t arg = {.i32 = 123};
        assert_ok(wah_call_by_name(&ctx, "run", &arg, 1, NULL));

        wah_free_exec_context(&ctx);
        wah_free_module(&primary);
        wah_free_module(&linked);
        wah_free_module(&host_mod);
    }

    // Regression: linked module tag import must verify type compatibility.
    // Without this check, a linked module could import a tag with a mismatched type
    // (e.g., i64 vs funcref), causing type confusion when catching exceptions.
    printf("Test: linked module tag import type mismatch rejected\n");
    {
        // Provider: exports a tag with type (i64)->()
        wah_module_t provider = {0};
        assert_ok(wah_parse_module_from_spec(&provider, "wasm \
            types {[ fn [i64] [], fn [] [] ]} \
            funcs {[ 1 ]} \
            tags {[ tag.type# 0 ]} \
            exports {[ {'thrower'} fn# 0, {'tag'} export.tag 0 ]} \
            code {[ {[] i64.const 305419896 throw 0 end } ]}"));

        // Plugin: imports tag as (funcref)->() [TYPE MISMATCH]
        wah_module_t plugin = {0};
        assert_ok(wah_parse_module_from_spec(&plugin, "wasm \
            types {[ fn [type.ref.null.func] [], fn [] [i32], fn [] [], fn [] [type.ref.null.func] ]} \
            imports {[ {'P'} {'thrower'} fn# 2, {'P'} {'tag'} tag# tag.type# 0 ]} \
            funcs {[ 1 ]} \
            exports {[ {'run'} fn# 1 ]} \
            code {[ {[] \
                block 3 \
                    try_table void [catch 0 0] \
                        call 0 \
                    end \
                    ref.null funcref \
                end \
                ref.is_null \
            end } ]}"));

        // Primary: imports plugin.run
        wah_module_t primary = {0};
        assert_ok(wah_parse_module_from_spec(&primary, "wasm \
            types {[ fn [] [i32] ]} \
            imports {[ {'L'} {'run'} fn# 0 ]}"));

        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &primary, NULL));
        assert_ok(wah_link_module(&ctx, "P", &provider));
        assert_ok(wah_link_module(&ctx, "L", &plugin));
        assert_err(wah_instantiate(&ctx), WAH_ERROR_LINK_FAILED);

        wah_free_exec_context(&ctx);
        wah_free_module(&primary);
        wah_free_module(&plugin);
        wah_free_module(&provider);
    }

    // Regression: memory.grow on imported memory flipped is_imported flags, causing
    // UAF when the importer context was freed before the provider context.
    printf("Test: memory.grow on imported memory must not transfer ownership (UAF regression)\n");
    {
        // Provider exports a memory and a store function.
        const char *provider_spec = "wasm \
            types {[ fn [i32, i32] [] ]} \
            funcs {[ 0 ]} \
            memories {[ limits.i32/2 1 10 ]} \
            exports {[ {'mem'} mem# 0, {'store'} fn# 0 ]} \
            code {[ {[] local.get 0 local.get 1 i32.store 0 0 end } ]}";

        // Consumer imports memory and grows it.
        const char *consumer_spec = "wasm \
            types {[ fn [i32] [i32] ]} \
            imports {[ {'p'} {'mem'} mem# limits.i32/2 1 10 ]} \
            funcs {[ 0 ]} \
            exports {[ {'grow'} fn# 0 ]} \
            code {[ {[] local.get 0 memory.grow 0 end } ]}";

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

        // Consumer grows the imported memory.
        wah_value_t arg = {.i32 = 2};
        wah_value_t result;
        assert_ok(wah_call(&cctx, 0, &arg, 1, &result));
        assert_eq_i32(result.i32, 1);

        // Free consumer first (simulates per-request teardown).
        wah_free_exec_context(&cctx);

        // Provider must still be able to use its memory (would UAF before fix).
        wah_value_t store_args[2] = {{.i32 = 0}, {.i32 = 0xCAFE}};
        assert_ok(wah_call(&pctx, 0, store_args, 2, NULL));

        wah_free_exec_context(&pctx);
        wah_free_module(&consumer);
        wah_free_module(&provider);
    }

    // Regression: table.grow on imported table must not transfer ownership (UAF regression).
    printf("Test: table.grow on imported table must not transfer ownership (UAF regression)\n");
    {
        // Provider exports a table and a table.size function.
        const char *provider_spec = "wasm \
            types {[ fn [] [i32], fn [i32] [i32] ]} \
            funcs {[ 0, 1 ]} \
            tables {[ funcref limits.i32/2 2 10 ]} \
            exports {[ {'tbl'} table# 0, {'size'} fn# 0, {'grow'} fn# 1 ]} \
            code {[ \
                {[] table.size 0 end }, \
                {[] ref.null funcref local.get 0 table.grow 0 end } \
            ]}";

        // Consumer imports table and grows it.
        const char *consumer_spec = "wasm \
            types {[ fn [i32] [i32] ]} \
            imports {[ {'p'} {'tbl'} table# funcref limits.i32/2 2 10 ]} \
            funcs {[ 0 ]} \
            exports {[ {'grow'} fn# 0 ]} \
            code {[ {[] ref.null funcref local.get 0 table.grow 0 end } ]}";

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

        // Consumer grows the imported table.
        wah_value_t arg = {.i32 = 3};
        wah_value_t result;
        assert_ok(wah_call(&cctx, 0, &arg, 1, &result));
        assert_eq_i32(result.i32, 2);

        // Free consumer first.
        wah_free_exec_context(&cctx);

        // Provider must still be able to query its table (would UAF before fix).
        wah_value_t size_result;
        assert_ok(wah_call(&pctx, 0, NULL, 0, &size_result));
        assert_eq_i32(size_result.i32, 5);

        wah_free_exec_context(&pctx);
        wah_free_module(&consumer);
        wah_free_module(&provider);
    }

    // Regression: table.grow must propagate to linked module internal contexts (UAF).
    printf("Test: table.grow propagates to linked module internal contexts\n");
    {
        // Primary module: owns a table, exports it and a grow function.
        // Also imports "linked.size" to read the table size from the linked module.
        const char *primary_spec = "wasm \
            types {[ fn [i32] [i32], fn [] [i32] ]} \
            imports {[ {'linked'} {'size'} fn# 1 ]} \
            funcs {[ 0, 1 ]} \
            tables {[ funcref limits.i32/2 1 100 ]} \
            exports {[ {'tbl'} table# 0, {'grow'} fn# 1, {'linked_size'} fn# 2 ]} \
            code {[ \
                {[] ref.null funcref local.get 0 table.grow 0 end }, \
                {[] call 0 end } \
            ]}";

        // Linked WASM module: imports "primary.tbl", exports table.size.
        const char *linked_spec = "wasm \
            types {[ fn [] [i32] ]} \
            imports {[ {'primary'} {'tbl'} table# funcref limits.i32/2 1 100 ]} \
            funcs {[ 0 ]} \
            exports {[ {'size'} fn# 0 ]} \
            code {[ {[] table.size 0 end } ]}";

        wah_module_t primary_mod = {0}, linked_mod = {0};
        assert_ok(wah_parse_module_from_spec(&primary_mod, primary_spec));
        assert_ok(wah_parse_module_from_spec(&linked_mod, linked_spec));

        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &primary_mod, NULL));
        assert_ok(wah_link_module(&ctx, "linked", &linked_mod));
        assert_ok(wah_link_module(&ctx, "primary", &primary_mod));
        assert_ok(wah_instantiate(&ctx));

        // linked_size (calls into linked module) should report size 1.
        wah_value_t result;
        assert_ok(wah_call_by_name(&ctx, "linked_size", NULL, 0, &result));
        assert_eq_i32(result.i32, 1);

        // Grow the table by 5. Old size should be 1.
        wah_value_t grow_arg = {.i32 = 5};
        assert_ok(wah_call_by_name(&ctx, "grow", &grow_arg, 1, &result));
        assert_eq_i32(result.i32, 1);

        // Linked module must see new size = 6 (would UAF before fix).
        assert_ok(wah_call_by_name(&ctx, "linked_size", NULL, 0, &result));
        assert_eq_i32(result.i32, 6);

        wah_free_exec_context(&ctx);
        wah_free_module(&linked_mod);
        wah_free_module(&primary_mod);
    }

    // Regression: memory.grow on linked-owned memory must propagate to primary ctx.
    // Uses wah_link_context so the primary module holds an imported alias to the
    // linked module's local memory.
    printf("Test: memory.grow on linked-owned memory propagates to primary (link_context)\n");
    {
        const char *linked_spec = "wasm \
            types {[ fn [i32] [i32] ]} \
            funcs {[ 0 ]} \
            memories {[ limits.i32/2 1 100 ]} \
            exports {[ {'mem'} mem# 0, {'grow'} fn# 0 ]} \
            code {[ {[] local.get 0 memory.grow 0 end } ]}";

        const char *primary_spec = "wasm \
            types {[ fn [i32] [i32], fn [i32, i32] [], fn [i32] [i32] ]} \
            imports {[ {'linked'} {'mem'} mem# limits.i32/2 1 100, \
                       {'linked'} {'grow'} fn# 0 ]} \
            funcs {[ 1, 2 ]} \
            exports {[ {'store'} fn# 1, {'load'} fn# 2, {'do_grow'} fn# 0 ]} \
            code {[ \
                {[] local.get 0 local.get 1 i32.store 0 0 end }, \
                {[] local.get 0 i32.load 0 0 end } \
            ]}";

        wah_module_t primary_mod = {0}, linked_mod = {0};
        assert_ok(wah_parse_module_from_spec(&linked_mod, linked_spec));
        assert_ok(wah_parse_module_from_spec(&primary_mod, primary_spec));

        wah_exec_context_t linked_ctx = {0};
        assert_ok(wah_new_exec_context(&linked_ctx, &linked_mod, NULL));
        assert_ok(wah_instantiate(&linked_ctx));

        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &primary_mod, NULL));
        assert_ok(wah_link_context(&ctx, "linked", &linked_ctx));
        assert_ok(wah_instantiate(&ctx));

        wah_value_t store_args[2] = {{.i32 = 0}, {.i32 = 0x12345678}};
        assert_ok(wah_call_by_name(&ctx, "store", store_args, 2, NULL));

        wah_value_t grow_arg = {.i32 = 1};
        wah_value_t result;
        assert_ok(wah_call_by_name(&ctx, "do_grow", &grow_arg, 1, &result));
        assert_eq_i32(result.i32, 1);

        store_args[1].i32 = 0x87654321u;
        assert_ok(wah_call_by_name(&ctx, "store", store_args, 2, NULL));
        assert_ok(wah_call_by_name(&ctx, "load", &store_args[0], 1, &result));
        assert_eq_i32(result.i32, (int32_t)0x87654321u);

        wah_free_exec_context(&ctx);
        wah_free_exec_context(&linked_ctx);
        wah_free_module(&linked_mod);
        wah_free_module(&primary_mod);
    }

    // Regression: table.grow on linked-owned table must propagate to primary ctx.
    // Uses wah_link_context so linked module has its own instantiated ictx with local tables.
    printf("Test: table.grow on linked-owned table propagates to primary (link_context)\n");
    {
        // Linked module: owns a local table, exports it and a grow function.
        const char *linked_spec = "wasm \
            types {[ fn [i32] [i32] ]} \
            funcs {[ 0 ]} \
            tables {[ funcref limits.i32/2 1 100 ]} \
            exports {[ {'tbl'} table# 0, {'grow'} fn# 0 ]} \
            code {[ {[] ref.null funcref local.get 0 table.grow 0 end } ]}";

        // Primary module: imports the table and grow func from linked module.
        // Exports "size" (table.size on imported table) and "do_grow" (calls linked grow).
        const char *primary_spec = "wasm \
            types {[ fn [] [i32], fn [i32] [i32] ]} \
            imports {[ {'linked'} {'tbl'} table# funcref limits.i32/2 1 100, \
                       {'linked'} {'grow'} fn# 1 ]} \
            funcs {[ 0, 1 ]} \
            exports {[ {'size'} fn# 1, {'do_grow'} fn# 2 ]} \
            code {[ \
                {[] table.size 0 end }, \
                {[] local.get 0 call 0 end } \
            ]}";

        wah_module_t primary_mod = {0}, linked_mod = {0};
        assert_ok(wah_parse_module_from_spec(&linked_mod, linked_spec));
        assert_ok(wah_parse_module_from_spec(&primary_mod, primary_spec));

        // Instantiate the linked module separately, then link via wah_link_context.
        wah_exec_context_t linked_ctx = {0};
        assert_ok(wah_new_exec_context(&linked_ctx, &linked_mod, NULL));
        assert_ok(wah_instantiate(&linked_ctx));

        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &primary_mod, NULL));
        assert_ok(wah_link_context(&ctx, "linked", &linked_ctx));
        assert_ok(wah_instantiate(&ctx));

        wah_value_t result;
        assert_ok(wah_call_by_name(&ctx, "size", NULL, 0, &result));
        assert_eq_i32(result.i32, 1);

        // Grow via linked module. Old size should be 1.
        wah_value_t grow_arg = {.i32 = 5};
        assert_ok(wah_call_by_name(&ctx, "do_grow", &grow_arg, 1, &result));
        assert_eq_i32(result.i32, 1);

        // Primary must see new size = 6 (stale pointer before fix).
        assert_ok(wah_call_by_name(&ctx, "size", NULL, 0, &result));
        assert_eq_i32(result.i32, 6);

        wah_free_exec_context(&ctx);
        wah_free_exec_context(&linked_ctx);
        wah_free_module(&linked_mod);
        wah_free_module(&primary_mod);
    }

    // Regression: table import min_elements validation must reject undersized provider.
    printf("Test: table import rejects provider with smaller min_elements\n");
    {
        // Provider: table with min=1
        const char *linked_spec = "wasm \
            tables {[ funcref limits.i32/1 1 ]} \
            exports {[ {'t'} table# 0 ]}";
        // Consumer: imports table with min=5 -- must fail
        const char *primary_spec = "wasm \
            imports {[ {'linked'} {'t'} table# funcref limits.i32/1 5 ]}";

        wah_module_t primary_mod = {0}, linked_mod = {0};
        assert_ok(wah_parse_module_from_spec(&linked_mod, linked_spec));
        assert_ok(wah_parse_module_from_spec(&primary_mod, primary_spec));

        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &primary_mod, NULL));
        assert_ok(wah_link_module(&ctx, "linked", &linked_mod));
        assert_err(wah_instantiate(&ctx), WAH_ERROR_LINK_FAILED);

        wah_free_exec_context(&ctx);
        wah_free_module(&linked_mod);
        wah_free_module(&primary_mod);
    }

    // Regression: table import min_elements validation via wah_link_context.
    printf("Test: table import rejects undersized provider via link_context\n");
    {
        const char *linked_spec = "wasm \
            tables {[ funcref limits.i32/1 1 ]} \
            exports {[ {'t'} table# 0 ]}";
        const char *primary_spec = "wasm \
            imports {[ {'linked'} {'t'} table# funcref limits.i32/1 5 ]}";

        wah_module_t primary_mod = {0}, linked_mod = {0};
        assert_ok(wah_parse_module_from_spec(&linked_mod, linked_spec));
        assert_ok(wah_parse_module_from_spec(&primary_mod, primary_spec));

        wah_exec_context_t linked_ctx = {0};
        assert_ok(wah_new_exec_context(&linked_ctx, &linked_mod, NULL));
        assert_ok(wah_instantiate(&linked_ctx));

        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &primary_mod, NULL));
        assert_ok(wah_link_context(&ctx, "linked", &linked_ctx));
        assert_err(wah_instantiate(&ctx), WAH_ERROR_LINK_FAILED);

        wah_free_exec_context(&ctx);
        wah_free_exec_context(&linked_ctx);
        wah_free_module(&linked_mod);
        wah_free_module(&primary_mod);
    }

    // Regression: memory import min_pages validation must reject undersized provider.
    printf("Test: memory import rejects provider with smaller min_pages\n");
    {
        // Provider: memory with min=1
        const char *linked_spec = "wasm \
            memories {[ limits.i32/1 1 ]} \
            exports {[ {'m'} mem# 0 ]}";
        // Consumer: imports memory with min=5 -- must fail
        const char *primary_spec = "wasm \
            imports {[ {'linked'} {'m'} mem# limits.i32/1 5 ]}";

        wah_module_t primary_mod = {0}, linked_mod = {0};
        assert_ok(wah_parse_module_from_spec(&linked_mod, linked_spec));
        assert_ok(wah_parse_module_from_spec(&primary_mod, primary_spec));

        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &primary_mod, NULL));
        assert_ok(wah_link_module(&ctx, "linked", &linked_mod));
        assert_err(wah_instantiate(&ctx), WAH_ERROR_LINK_FAILED);

        wah_free_exec_context(&ctx);
        wah_free_module(&linked_mod);
        wah_free_module(&primary_mod);
    }

    // Regression: memory import min_pages validation via wah_link_context.
    printf("Test: memory import rejects undersized provider via link_context\n");
    {
        const char *linked_spec = "wasm \
            memories {[ limits.i32/1 1 ]} \
            exports {[ {'m'} mem# 0 ]}";
        const char *primary_spec = "wasm \
            imports {[ {'linked'} {'m'} mem# limits.i32/1 5 ]}";

        wah_module_t primary_mod = {0}, linked_mod = {0};
        assert_ok(wah_parse_module_from_spec(&linked_mod, linked_spec));
        assert_ok(wah_parse_module_from_spec(&primary_mod, primary_spec));

        wah_exec_context_t linked_ctx = {0};
        assert_ok(wah_new_exec_context(&linked_ctx, &linked_mod, NULL));
        assert_ok(wah_instantiate(&linked_ctx));

        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &primary_mod, NULL));
        assert_ok(wah_link_context(&ctx, "linked", &linked_ctx));
        assert_err(wah_instantiate(&ctx), WAH_ERROR_LINK_FAILED);

        wah_free_exec_context(&ctx);
        wah_free_exec_context(&linked_ctx);
        wah_free_module(&linked_mod);
        wah_free_module(&primary_mod);
    }

    // Regression: when multiple consumers share the same provider via
    // wah_link_context, memory.grow in one consumer did not propagate the new
    // base/size to sibling consumers, causing use-after-free on stale pointers.
    printf("Test: sibling consumer memory alias propagation after grow\n");
    {
        // Provider: 1 page memory (max 10), exports "grow" and "mem".
        const char *provider_spec = "wasm \
            types {[ fn [i32] [i32] ]} \
            funcs {[ 0 ]} \
            memories {[ limits.i32/2 1 10 ]} \
            exports {[ {'grow'} fn# 0, {'mem'} mem# 0 ]} \
            code {[ {[] local.get 0 memory.grow 0 end } ]}";

        // Grower: no memory, imports grow from provider, re-exports as call_grow.
        const char *grower_spec = "wasm \
            types {[ fn [i32] [i32] ]} \
            imports {[ {'p'} {'grow'} fn# 0 ]} \
            funcs {[ 0 ]} \
            exports {[ {'call_grow'} fn# 1 ]} \
            code {[ {[] local.get 0 call 0 end } ]}";

        // User: imports provider's memory, exports store(addr, val) and load(addr).
        const char *user_spec = "wasm \
            types {[ fn [i32, i32] [], fn [i32] [i32] ]} \
            imports {[ {'p'} {'mem'} mem# limits.i32/2 1 10 ]} \
            funcs {[ 0, 1 ]} \
            exports {[ {'store'} fn# 0, {'load'} fn# 1 ]} \
            code {[ \
                {[] local.get 0 local.get 1 i32.store 2 0 end }, \
                {[] local.get 0 i32.load 2 0 end } \
            ]}";

        wah_module_t pmod = {0}, gmod = {0}, umod = {0};
        assert_ok(wah_parse_module_from_spec(&pmod, provider_spec));
        assert_ok(wah_parse_module_from_spec(&gmod, grower_spec));
        assert_ok(wah_parse_module_from_spec(&umod, user_spec));

        wah_exec_context_t pctx = {0};
        assert_ok(wah_new_exec_context(&pctx, &pmod, NULL));
        assert_ok(wah_instantiate(&pctx));

        wah_exec_context_t gctx = {0};
        assert_ok(wah_new_exec_context(&gctx, &gmod, NULL));
        assert_ok(wah_link_context(&gctx, "p", &pctx));
        assert_ok(wah_instantiate(&gctx));

        wah_exec_context_t uctx = {0};
        assert_ok(wah_new_exec_context(&uctx, &umod, NULL));
        assert_ok(wah_link_context(&uctx, "p", &pctx));
        assert_ok(wah_instantiate(&uctx));

        // Store 42 at offset 0 via user context (within initial 1-page memory).
        wah_value_t store_args[2] = {{.i32 = 0}, {.i32 = 42}};
        assert_ok(wah_call_by_name(&uctx, "store", store_args, 2, NULL));

        // Grow by 2 pages via grower context (realloc may move the buffer).
        wah_value_t grow_arg = {.i32 = 2};
        wah_value_t grow_result;
        assert_ok(wah_call_by_name(&gctx, "call_grow", &grow_arg, 1, &grow_result));
        assert_eq_i32(grow_result.i32, 1);  // old page count

        // Load from offset 0 via user context -- must see the grown memory,
        // not a stale pointer to the freed old buffer.
        wah_value_t load_arg = {.i32 = 0};
        wah_value_t load_result;
        assert_ok(wah_call_by_name(&uctx, "load", &load_arg, 1, &load_result));
        assert_eq_i32(load_result.i32, 42);

        // Also verify writing into the grown region works.
        uint32_t addr_in_grown = 1 * 65536 + 100;  // past original 1 page
        store_args[0].i32 = (int32_t)addr_in_grown;
        store_args[1].i32 = 99;
        assert_ok(wah_call_by_name(&uctx, "store", store_args, 2, NULL));
        load_arg.i32 = (int32_t)addr_in_grown;
        assert_ok(wah_call_by_name(&uctx, "load", &load_arg, 1, &load_result));
        assert_eq_i32(load_result.i32, 99);

        wah_free_exec_context(&uctx);
        wah_free_exec_context(&gctx);
        wah_free_exec_context(&pctx);
        wah_free_module(&umod);
        wah_free_module(&gmod);
        wah_free_module(&pmod);
    }

    // Regression: same as above but for table.grow -- sibling consumer's table
    // pointers were not updated after a grow through a different consumer.
    printf("Test: sibling consumer table alias propagation after grow\n");
    {
        // Provider: 1 funcref table (min 1, max 10), exports "tbl_grow" and "tbl".
        const char *provider_spec = "wasm \
            types {[ fn [i32] [i32] ]} \
            funcs {[ 0 ]} \
            tables {[ funcref limits.i32/2 1 10 ]} \
            exports {[ {'tbl_grow'} fn# 0, {'tbl'} table# 0 ]} \
            code {[ {[] ref.null funcref local.get 0 table.grow 0 end } ]}";

        // Grower: imports tbl_grow from provider.
        const char *grower_spec = "wasm \
            types {[ fn [i32] [i32] ]} \
            imports {[ {'p'} {'tbl_grow'} fn# 0 ]} \
            funcs {[ 0 ]} \
            exports {[ {'call_tbl_grow'} fn# 1 ]} \
            code {[ {[] local.get 0 call 0 end } ]}";

        // User: imports provider's table, exports tbl_size() -> i32.
        const char *user_spec = "wasm \
            types {[ fn [] [i32] ]} \
            imports {[ {'p'} {'tbl'} table# funcref limits.i32/2 1 10 ]} \
            funcs {[ 0 ]} \
            exports {[ {'tbl_size'} fn# 0 ]} \
            code {[ {[] table.size 0 end } ]}";

        wah_module_t pmod = {0}, gmod = {0}, umod = {0};
        assert_ok(wah_parse_module_from_spec(&pmod, provider_spec));
        assert_ok(wah_parse_module_from_spec(&gmod, grower_spec));
        assert_ok(wah_parse_module_from_spec(&umod, user_spec));

        wah_exec_context_t pctx = {0};
        assert_ok(wah_new_exec_context(&pctx, &pmod, NULL));
        assert_ok(wah_instantiate(&pctx));

        wah_exec_context_t gctx = {0};
        assert_ok(wah_new_exec_context(&gctx, &gmod, NULL));
        assert_ok(wah_link_context(&gctx, "p", &pctx));
        assert_ok(wah_instantiate(&gctx));

        wah_exec_context_t uctx = {0};
        assert_ok(wah_new_exec_context(&uctx, &umod, NULL));
        assert_ok(wah_link_context(&uctx, "p", &pctx));
        assert_ok(wah_instantiate(&uctx));

        // Check initial table size from user.
        wah_value_t size_result;
        assert_ok(wah_call_by_name(&uctx, "tbl_size", NULL, 0, &size_result));
        assert_eq_i32(size_result.i32, 1);

        // Grow table by 3 via grower.
        wah_value_t grow_arg = {.i32 = 3};
        wah_value_t grow_result;
        assert_ok(wah_call_by_name(&gctx, "call_tbl_grow", &grow_arg, 1, &grow_result));
        assert_eq_i32(grow_result.i32, 1);  // old size

        // User must see the updated table size.
        assert_ok(wah_call_by_name(&uctx, "tbl_size", NULL, 0, &size_result));
        assert_eq_i32(size_result.i32, 4);

        wah_free_exec_context(&uctx);
        wah_free_exec_context(&gctx);
        wah_free_exec_context(&pctx);
        wah_free_module(&umod);
        wah_free_module(&gmod);
        wah_free_module(&pmod);
    }

    // Regression: wah_export_func on a module with imports must store a global
    // index (import_count + local_idx), not a bare local index.
    printf("Test: programmatic export index accounts for imports (security regression)\n");
    {
        // Provider module has one function import and one unexported local function
        // that calls host.danger. We then programmatically add a safe host export.
        // The consumer imports provider.safe. Before the fix, the bare local index
        // caused the consumer to call the unexported wasm function instead of the
        // safe host callback.
        wah_module_t provider = {0};
        assert_ok(wah_parse_module_from_spec(&provider, "wasm \
            types {[ fn [] [i32] ]} \
            imports {[ {'host'} {'danger'} fn# 0 ]} \
            funcs {[0]} \
            code {[ {[] call 0 end } ]}"));

        assert_ok(wah_export_func(&provider, "safe", "() -> i32", safe_host_func, NULL, NULL));

        // Consumer calls provider.safe.
        wah_module_t consumer = {0};
        assert_ok(wah_parse_module_from_spec(&consumer, "wasm \
            types {[ fn [] [i32] ]} \
            imports {[ {'provider'} {'safe'} fn# 0 ]} \
            funcs {[0]} \
            exports {[ {'run'} fn# 1 ]} \
            code {[ {[] call 0 end } ]}"));

        wah_module_t host_mod = {0};
        wah_new_module(&host_mod, NULL);
        wah_export_func(&host_mod, "danger", "() -> i32", danger_host_func, NULL, NULL);

        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_exec_context(&ctx, &consumer, NULL));
        assert_ok(wah_link_module(&ctx, "provider", &provider));
        assert_ok(wah_link_module(&ctx, "host", &host_mod));
        assert_ok(wah_instantiate(&ctx));

        danger_called = 0;
        safe_called = 0;
        wah_value_t result;
        assert_ok(wah_call_by_name(&ctx, "run", NULL, 0, &result));

        assert_eq_i32(safe_called, 1);
        assert_eq_i32(danger_called, 0);
        assert_eq_i32(result.i32, 123);

        wah_free_exec_context(&ctx);
        wah_free_module(&consumer);
        wah_free_module(&provider);
        wah_free_module(&host_mod);
    }

    // Regression: memory.grow alias must propagate through re-exported imports.
    // provider -> middle -> user chain; grow on provider must update user's pointer.
    printf("Test: memory.grow propagates through re-exported import chain (UAF regression)\n");
    {
        // Provider: owns memory, exports it and a grow function.
        const char *provider_spec = "wasm \
            types {[ fn [i32] [i32] ]} \
            funcs {[ 0 ]} \
            memories {[ limits.i32/2 1 100 ]} \
            exports {[ {'mem'} mem# 0, {'grow'} fn# 0 ]} \
            code {[ {[] local.get 0 memory.grow 0 end } ]}";

        // Middle: imports memory and grow from provider, re-exports memory
        // and wraps grow in a local function (re-exporting imported funcs is
        // not supported by the linker).
        const char *middle_spec = "wasm \
            types {[ fn [i32] [i32] ]} \
            imports {[ {'provider'} {'mem'} mem# limits.i32/2 1 100, \
                       {'provider'} {'grow'} fn# 0 ]} \
            funcs {[ 0 ]} \
            exports {[ {'mem'} mem# 0, {'grow'} fn# 1 ]} \
            code {[ {[] local.get 0 call 0 end } ]}";

        // User: imports memory and grow from middle, stores/loads/grows.
        const char *user_spec = "wasm \
            types {[ fn [i32] [i32], fn [i32, i32] [], fn [i32] [i32] ]} \
            imports {[ {'middle'} {'mem'} mem# limits.i32/2 1 100, \
                       {'middle'} {'grow'} fn# 0 ]} \
            funcs {[ 1, 2 ]} \
            exports {[ {'store'} fn# 1, {'load'} fn# 2, {'do_grow'} fn# 0 ]} \
            code {[ \
                {[] local.get 0 local.get 1 i32.store 0 0 end }, \
                {[] local.get 0 i32.load 0 0 end } \
            ]}";

        wah_module_t provider_mod = {0}, middle_mod = {0}, user_mod = {0};
        assert_ok(wah_parse_module_from_spec(&provider_mod, provider_spec));
        assert_ok(wah_parse_module_from_spec(&middle_mod, middle_spec));
        assert_ok(wah_parse_module_from_spec(&user_mod, user_spec));

        wah_exec_context_t provider_ctx = {0};
        assert_ok(wah_new_exec_context(&provider_ctx, &provider_mod, NULL));
        assert_ok(wah_instantiate(&provider_ctx));

        wah_exec_context_t middle_ctx = {0};
        assert_ok(wah_new_exec_context(&middle_ctx, &middle_mod, NULL));
        assert_ok(wah_link_context(&middle_ctx, "provider", &provider_ctx));
        assert_ok(wah_instantiate(&middle_ctx));

        wah_exec_context_t user_ctx = {0};
        assert_ok(wah_new_exec_context(&user_ctx, &user_mod, NULL));
        assert_ok(wah_link_context(&user_ctx, "middle", &middle_ctx));
        assert_ok(wah_instantiate(&user_ctx));

        // Store a value before grow.
        wah_value_t store_args[2] = {{.i32 = 0}, {.i32 = 0x12345678}};
        assert_ok(wah_call_by_name(&user_ctx, "store", store_args, 2, NULL));

        // Grow via provider's grow function (called through middle's re-export).
        wah_value_t grow_arg = {.i32 = 20};
        wah_value_t result;
        assert_ok(wah_call_by_name(&user_ctx, "do_grow", &grow_arg, 1, &result));
        assert_eq_i32(result.i32, 1);

        // After grow, user must see the new memory (not a stale freed pointer).
        store_args[1].i32 = (int32_t)0x87654321u;
        assert_ok(wah_call_by_name(&user_ctx, "store", store_args, 2, NULL));
        assert_ok(wah_call_by_name(&user_ctx, "load", &store_args[0], 1, &result));
        assert_eq_i32(result.i32, (int32_t)0x87654321u);

        wah_free_exec_context(&user_ctx);
        wah_free_exec_context(&middle_ctx);
        wah_free_exec_context(&provider_ctx);
        wah_free_module(&user_mod);
        wah_free_module(&middle_mod);
        wah_free_module(&provider_mod);
    }

    // Regression: table.grow alias must propagate through re-exported imports.
    printf("Test: table.grow propagates through re-exported import chain\n");
    {
        // Provider: owns table, exports it and a grow function.
        const char *provider_spec = "wasm \
            types {[ fn [i32] [i32] ]} \
            funcs {[ 0 ]} \
            tables {[ funcref limits.i32/2 1 100 ]} \
            exports {[ {'tbl'} table# 0, {'grow'} fn# 0 ]} \
            code {[ {[] ref.null funcref local.get 0 table.grow 0 end } ]}";

        // Middle: imports table and grow from provider, re-exports table
        // and wraps grow in a local function.
        const char *middle_spec = "wasm \
            types {[ fn [i32] [i32] ]} \
            imports {[ {'provider'} {'tbl'} table# funcref limits.i32/2 1 100, \
                       {'provider'} {'grow'} fn# 0 ]} \
            funcs {[ 0 ]} \
            exports {[ {'tbl'} table# 0, {'grow'} fn# 1 ]} \
            code {[ {[] local.get 0 call 0 end } ]}";

        // User: imports table and grow from middle, checks size after grow.
        const char *user_spec = "wasm \
            types {[ fn [] [i32], fn [i32] [i32] ]} \
            imports {[ {'middle'} {'tbl'} table# funcref limits.i32/2 1 100, \
                       {'middle'} {'grow'} fn# 1 ]} \
            funcs {[ 0 ]} \
            exports {[ {'size'} fn# 1, {'do_grow'} fn# 0 ]} \
            code {[ \
                {[] table.size 0 end } \
            ]}";

        wah_module_t provider_mod = {0}, middle_mod = {0}, user_mod = {0};
        assert_ok(wah_parse_module_from_spec(&provider_mod, provider_spec));
        assert_ok(wah_parse_module_from_spec(&middle_mod, middle_spec));
        assert_ok(wah_parse_module_from_spec(&user_mod, user_spec));

        wah_exec_context_t provider_ctx = {0};
        assert_ok(wah_new_exec_context(&provider_ctx, &provider_mod, NULL));
        assert_ok(wah_instantiate(&provider_ctx));

        wah_exec_context_t middle_ctx = {0};
        assert_ok(wah_new_exec_context(&middle_ctx, &middle_mod, NULL));
        assert_ok(wah_link_context(&middle_ctx, "provider", &provider_ctx));
        assert_ok(wah_instantiate(&middle_ctx));

        wah_exec_context_t user_ctx = {0};
        assert_ok(wah_new_exec_context(&user_ctx, &user_mod, NULL));
        assert_ok(wah_link_context(&user_ctx, "middle", &middle_ctx));
        assert_ok(wah_instantiate(&user_ctx));

        wah_value_t result;
        assert_ok(wah_call_by_name(&user_ctx, "size", NULL, 0, &result));
        assert_eq_i32(result.i32, 1);

        wah_value_t grow_arg = {.i32 = 5};
        assert_ok(wah_call_by_name(&user_ctx, "do_grow", &grow_arg, 1, &result));
        assert_eq_i32(result.i32, 1);

        // After grow, user must see updated table size.
        assert_ok(wah_call_by_name(&user_ctx, "size", NULL, 0, &result));
        assert_eq_i32(result.i32, 6);

        wah_free_exec_context(&user_ctx);
        wah_free_exec_context(&middle_ctx);
        wah_free_exec_context(&provider_ctx);
        wah_free_module(&user_mod);
        wah_free_module(&middle_mod);
        wah_free_module(&provider_mod);
    }

    printf("All linkage tests passed!\n");
    return 0;
}
