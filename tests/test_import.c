// Tests for import section parsing and function import resolution
#include <stdio.h>
#include "../wah.h"
#include "wah_impl.h"
#include "common.h"

static int g_add_called = 0;

static void host_add(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    wah_return_i32(ctx, wah_param_i32(ctx, 0) + wah_param_i32(ctx, 1));
    g_add_called = 1;
}

// WASM: imports "env"."add" (i32,i32)->i32, wraps it, exports as "callAdd" (global idx 1)
static const char *k_import_spec = "wasm \
    types {[ fn [i32, i32] [i32] ]} \
    imports {[ {'env'} {'add'} fn# 0 ]} \
    funcs {[ 0 ]} \
    exports {[ {'callAdd'} fn# 1 ]} \
    code {[ {[] local.get 0 local.get 1 call 0 end} ]}";

static wah_error_t make_env_module(wah_module_t *mod) {
    wah_error_t err = wah_new_module(mod);
    if (err != WAH_OK) return err;
    return wah_module_export_func(mod, "add", "(i32, i32) -> i32", host_add, NULL, NULL);
}

// ---------------------------------------------------------------------------

static void test_basic_import_resolution(void) {
    printf("Test 1: Basic import resolution (WASM calls host function)\n");
    wah_module_t wasm_mod = {0}, env_mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(make_env_module(&env_mod));
    assert_ok(wah_parse_module_from_spec(&wasm_mod, k_import_spec));
    assert_eq_u32(wasm_mod.import_function_count, 1);
    printf("  import_function_count=%u  wasm_function_count=%u\n",
           wasm_mod.import_function_count, wasm_mod.wasm_function_count);

    assert_ok(wah_exec_context_create(&ctx, &wasm_mod));
    assert_ok(wah_link_module(&ctx, "env", &env_mod));
    assert_ok(wah_instantiate(&ctx));

    wah_export_desc_t entry;
    assert_ok(wah_module_export_by_name(&wasm_mod, "callAdd", &entry));
    uint32_t export_idx = entry.index;
    assert_eq_u32(export_idx, 1);

    wah_value_t params[2] = {{ .i32 = 10 }, { .i32 = 20 }};
    wah_value_t result = {0};
    g_add_called = 0;
    assert_ok(wah_call(&ctx, entry.index, params, 2, &result));
    assert_true(g_add_called);
    assert_eq_i32(result.i32, 30);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&wasm_mod);
    wah_free_module(&env_mod);
}

static void test_import_not_found_bad_module(void) {
    printf("Test 2: WAH_ERROR_IMPORT_NOT_FOUND -- wrong module name\n");
    wah_module_t wasm_mod = {0}, env_mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(make_env_module(&env_mod));
    assert_ok(wah_parse_module_from_spec(&wasm_mod, k_import_spec));
    assert_ok(wah_exec_context_create(&ctx, &wasm_mod));

    wah_link_module(&ctx, "wrong_name", &env_mod);  // intentionally wrong

    assert_err(wah_instantiate(&ctx), WAH_ERROR_IMPORT_NOT_FOUND);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&wasm_mod);
    wah_free_module(&env_mod);
}

static void test_import_not_found_bad_field(void) {
    printf("Test 3: WAH_ERROR_IMPORT_NOT_FOUND -- wrong field name\n");
    wah_module_t wasm_mod = {0}, env_mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_new_module(&env_mod));

    wah_module_export_func(&env_mod, "wrong_func", "(i32, i32) -> i32", host_add, NULL, NULL);

    assert_ok(wah_parse_module_from_spec(&wasm_mod, k_import_spec));
    assert_ok(wah_exec_context_create(&ctx, &wasm_mod));

    wah_link_module(&ctx, "env", &env_mod);

    assert_err(wah_instantiate(&ctx), WAH_ERROR_IMPORT_NOT_FOUND);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&wasm_mod);
    wah_free_module(&env_mod);
}

static void test_import_index_space(void) {
    printf("Test 4: Import + local index space -- callAdd export at global index 1\n");
    wah_module_t wasm_mod = {0};

    assert_ok(wah_parse_module_from_spec(&wasm_mod, k_import_spec));

    wah_export_desc_t entry;
    assert_ok(wah_module_export_by_name(&wasm_mod, "callAdd", &entry));
    uint32_t idx = entry.index;
    assert_eq_u32(idx, 1);

    assert_eq_i32(entry.kind, WAH_KIND_FUNCTION);
    assert_eq_u32(entry.u.func.param_count, 2);
    assert_eq_u32(entry.u.func.result_count, 1);
    assert_eq_i32(entry.u.func.param_types[0], WAH_TYPE_I32);
    assert_eq_i32(entry.u.func.param_types[1], WAH_TYPE_I32);
    assert_eq_i32(entry.u.func.result_types[0], WAH_TYPE_I32);

    wah_free_module(&wasm_mod);
}

static void test_no_imports_unchanged(void) {
    printf("Test 5: Module without imports works as before\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    const char* wasm_spec = "wasm \
        types {[ fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        exports {[ {'add'} fn# 0 ]} \
        code {[ {[] local.get 0 local.get 1 i32.add end} ]} \
    ";
    assert_ok(wah_parse_module_from_spec(&mod, wasm_spec));
    assert_eq_u32(mod.import_function_count, 0);
    assert_ok(wah_exec_context_create(&ctx, &mod));

    wah_export_desc_t entry;
    assert_ok(wah_module_export_by_name(&mod, "add", &entry));
    assert_eq_u32(entry.index, 0);

    wah_value_t params[2] = {{ .i32 = 7 }, { .i32 = 8 }};
    wah_value_t result = {0};
    assert_ok(wah_call(&ctx, entry.index, params, 2, &result));
    assert_eq_i32(result.i32, 15);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

// --- Global Import Tests ---

static void test_global_import_i32(void) {
    printf("Test 6: Global import (i32)\n");

    wah_module_t provider = {0};
    assert_ok(wah_new_module(&provider));
    assert_ok(wah_module_export_global_i32(&provider, "g", 1, 42));

    const char *spec = "wasm \
        types {[ fn [] [i32] ]} \
        imports {[ {'env'} {'g'} export.global i32 mut ]} \
        funcs {[ 0 ]} \
        exports {[ {'get'} fn# 0 ]} \
        code {[ {[] global.get 0 end} ]}";

    wah_module_t wasm_mod = {0};
    assert_ok(wah_parse_module_from_spec(&wasm_mod, spec));
    assert_eq_u32(wasm_mod.import_global_count, 1);
    assert_eq_u32(wasm_mod.global_count, 0);

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &wasm_mod));
    assert_ok(wah_link_module(&ctx, "env", &provider));
    assert_ok(wah_instantiate(&ctx));

    wah_export_desc_t entry;
    assert_ok(wah_module_export_by_name(&wasm_mod, "get", &entry));

    wah_value_t result = {0};
    assert_ok(wah_call(&ctx, entry.index, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&wasm_mod);
    wah_free_module(&provider);
}

static void test_global_import_set(void) {
    printf("Test 7: Global import set (mutable i32)\n");

    wah_module_t provider = {0};
    assert_ok(wah_new_module(&provider));
    assert_ok(wah_module_export_global_i32(&provider, "g", 1, 10));

    const char *spec = "wasm \
        types {[ fn [] [i32], fn [] [] ]} \
        imports {[ {'env'} {'g'} export.global i32 mut ]} \
        funcs {[ 0, 1 ]} \
        exports {[ {'get'} fn# 0, {'inc'} fn# 1 ]} \
        code {[ \
            {[] global.get 0 end}, \
            {[] global.get 0 i32.const 1 i32.add global.set 0 end} \
        ]}";

    wah_module_t wasm_mod = {0};
    assert_ok(wah_parse_module_from_spec(&wasm_mod, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &wasm_mod));
    assert_ok(wah_link_module(&ctx, "env", &provider));
    assert_ok(wah_instantiate(&ctx));

    wah_export_desc_t get_entry, inc_entry;
    assert_ok(wah_module_export_by_name(&wasm_mod, "get", &get_entry));
    assert_ok(wah_module_export_by_name(&wasm_mod, "inc", &inc_entry));

    wah_value_t result = {0};
    assert_ok(wah_call(&ctx, get_entry.index, NULL, 0, &result));
    assert_eq_i32(result.i32, 10);

    assert_ok(wah_call(&ctx, inc_entry.index, NULL, 0, NULL));
    assert_ok(wah_call(&ctx, get_entry.index, NULL, 0, &result));
    assert_eq_i32(result.i32, 11);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&wasm_mod);
    wah_free_module(&provider);
}

static void test_global_import_with_local_globals(void) {
    printf("Test 8: Global import + local globals\n");

    wah_module_t provider = {0};
    assert_ok(wah_new_module(&provider));
    assert_ok(wah_module_export_global_i32(&provider, "g", 0, 100));

    const char *spec = "wasm \
        types {[ fn [] [i32] ]} \
        imports {[ {'env'} {'g'} export.global i32 immut ]} \
        funcs {[ 0 ]} \
        globals {[ i32 0x00 i32.const 200 end ]} \
        exports {[ {'sum'} fn# 0 ]} \
        code {[ {[] global.get 0 global.get 1 i32.add end} ]}";

    wah_module_t wasm_mod = {0};
    assert_ok(wah_parse_module_from_spec(&wasm_mod, spec));
    assert_eq_u32(wasm_mod.import_global_count, 1);
    assert_eq_u32(wasm_mod.global_count, 1);

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &wasm_mod));
    assert_ok(wah_link_module(&ctx, "env", &provider));
    assert_ok(wah_instantiate(&ctx));

    wah_export_desc_t entry;
    assert_ok(wah_module_export_by_name(&wasm_mod, "sum", &entry));

    wah_value_t result = {0};
    assert_ok(wah_call(&ctx, entry.index, NULL, 0, &result));
    assert_eq_i32(result.i32, 300);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&wasm_mod);
    wah_free_module(&provider);
}

static void test_global_import_not_found(void) {
    printf("Test 9: Global import not found\n");

    wah_module_t provider = {0};
    assert_ok(wah_new_module(&provider));
    assert_ok(wah_module_export_global_i32(&provider, "other", 0, 0));

    const char *spec = "wasm \
        types {[ fn [] [i32] ]} \
        imports {[ {'env'} {'g'} export.global i32 immut ]} \
        funcs {[ 0 ]} \
        exports {[ {'get'} fn# 0 ]} \
        code {[ {[] global.get 0 end} ]}";

    wah_module_t wasm_mod = {0};
    assert_ok(wah_parse_module_from_spec(&wasm_mod, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &wasm_mod));
    assert_ok(wah_link_module(&ctx, "env", &provider));
    assert_err(wah_instantiate(&ctx), WAH_ERROR_IMPORT_NOT_FOUND);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&wasm_mod);
    wah_free_module(&provider);
}

// --- Memory Import Tests ---

static void test_memory_import(void) {
    printf("Test 10: Memory import\n");

    wah_module_t provider = {0};
    assert_ok(wah_new_module(&provider));
    assert_ok(wah_module_export_memory(&provider, "mem", 1, 1));

    const char *spec = "wasm \
        types {[ fn [] [i32] ]} \
        imports {[ {'env'} {'mem'} export.memory limits.i32/1 1 ]} \
        funcs {[ 0 ]} \
        exports {[ {'load'} fn# 0 ]} \
        code {[ {[] i32.const 0 i32.load align=4 offset=0 end} ]}";

    wah_module_t wasm_mod = {0};
    assert_ok(wah_parse_module_from_spec(&wasm_mod, spec));
    assert_eq_u32(wasm_mod.import_memory_count, 1);
    assert_eq_u32(wasm_mod.memory_count, 0);

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &wasm_mod));
    assert_ok(wah_link_module(&ctx, "env", &provider));
    assert_ok(wah_instantiate(&ctx));

    assert_true(wah_debug_memory_data(&ctx, 0) != NULL);
    assert_true(wah_debug_memory_size(&ctx, 0) == 65536);

    uint32_t val = 0x12345678;
    memcpy(wah_debug_memory_data(&ctx, 0), &val, 4);

    wah_export_desc_t entry;
    assert_ok(wah_module_export_by_name(&wasm_mod, "load", &entry));

    wah_value_t result = {0};
    assert_ok(wah_call(&ctx, entry.index, NULL, 0, &result));
    assert_eq_u32((uint32_t)result.i32, 0x12345678);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&wasm_mod);
    wah_free_module(&provider);
}

static void test_memory_import_not_found(void) {
    printf("Test 11: Memory import not found\n");

    wah_module_t provider = {0};
    assert_ok(wah_new_module(&provider));

    const char *spec = "wasm \
        types {[ fn [] [i32] ]} \
        imports {[ {'env'} {'mem'} export.memory limits.i32/1 1 ]} \
        funcs {[ 0 ]} \
        exports {[ {'load'} fn# 0 ]} \
        code {[ {[] i32.const 0 i32.load align=4 offset=0 end} ]}";

    wah_module_t wasm_mod = {0};
    assert_ok(wah_parse_module_from_spec(&wasm_mod, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &wasm_mod));
    assert_ok(wah_link_module(&ctx, "env", &provider));
    assert_err(wah_instantiate(&ctx), WAH_ERROR_IMPORT_NOT_FOUND);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&wasm_mod);
    wah_free_module(&provider);
}

// --- Table Import Tests ---

static void test_table_import(void) {
    printf("Test 12: Table import\n");

    wah_module_t provider = {0};
    assert_ok(wah_new_module(&provider));
    assert_ok(wah_debug_module_export_table(&provider, "tbl",
        WAH_TYPE_FUNCREF, WAH_TYPE_I32, 1, 10));

    const char *spec = "wasm \
        types {[ fn [] [i32] ]} \
        imports {[ {'env'} {'tbl'} export.table funcref limits.i32/1 1 ]} \
        funcs {[ 0 ]} \
        exports {[ {'tsize'} fn# 0 ]} \
        code {[ {[] table.size 0 end} ]}";

    wah_module_t wasm_mod = {0};
    assert_ok(wah_parse_module_from_spec(&wasm_mod, spec));
    assert_eq_u32(wasm_mod.import_table_count, 1);
    assert_eq_u32(wasm_mod.table_count, 0);

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &wasm_mod));
    assert_ok(wah_link_module(&ctx, "env", &provider));
    assert_ok(wah_instantiate(&ctx));

    wah_export_desc_t entry;
    assert_ok(wah_module_export_by_name(&wasm_mod, "tsize", &entry));

    wah_value_t result = {0};
    assert_ok(wah_call(&ctx, entry.index, NULL, 0, &result));
    assert_eq_i32(result.i32, 1);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&wasm_mod);
    wah_free_module(&provider);
}

static void test_table_import_not_found(void) {
    printf("Test 13: Table import not found\n");

    wah_module_t provider = {0};
    assert_ok(wah_new_module(&provider));

    const char *spec = "wasm \
        types {[ fn [] [i32] ]} \
        imports {[ {'env'} {'tbl'} export.table funcref limits.i32/1 1 ]} \
        funcs {[ 0 ]} \
        exports {[ {'tsize'} fn# 0 ]} \
        code {[ {[] table.size 0 end} ]}";

    wah_module_t wasm_mod = {0};
    assert_ok(wah_parse_module_from_spec(&wasm_mod, spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &wasm_mod));
    assert_ok(wah_link_module(&ctx, "env", &provider));
    assert_err(wah_instantiate(&ctx), WAH_ERROR_IMPORT_NOT_FOUND);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&wasm_mod);
    wah_free_module(&provider);
}

// --- Mixed Import Tests ---

static void host_get42(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    wah_return_i32(ctx, 42);
}

static void test_mixed_imports(void) {
    printf("Test 14: Mixed imports (func + global + memory)\n");

    wah_module_t provider = {0};
    assert_ok(wah_new_module(&provider));
    assert_ok(wah_module_export_func(&provider, "get42", "() -> i32", host_get42, NULL, NULL));
    assert_ok(wah_module_export_global_i32(&provider, "g", 0, 100));
    assert_ok(wah_module_export_memory(&provider, "mem", 1, 1));

    const char *spec = "wasm \
        types {[ fn [] [i32] ]} \
        imports {[ \
            {'env'} {'get42'} fn# 0, \
            {'env'} {'g'} export.global i32 immut, \
            {'env'} {'mem'} export.memory limits.i32/1 1 \
        ]} \
        funcs {[ 0 ]} \
        exports {[ {'test'} fn# 1 ]} \
        code {[ {[] call 0 global.get 0 i32.add end} ]}";

    wah_module_t wasm_mod = {0};
    assert_ok(wah_parse_module_from_spec(&wasm_mod, spec));
    assert_eq_u32(wasm_mod.import_function_count, 1);
    assert_eq_u32(wasm_mod.import_global_count, 1);
    assert_eq_u32(wasm_mod.import_memory_count, 1);

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &wasm_mod));
    assert_ok(wah_link_module(&ctx, "env", &provider));
    assert_ok(wah_instantiate(&ctx));

    wah_export_desc_t entry;
    assert_ok(wah_module_export_by_name(&wasm_mod, "test", &entry));

    wah_value_t result = {0};
    assert_ok(wah_call(&ctx, entry.index, NULL, 0, &result));
    assert_eq_i32(result.i32, 142);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&wasm_mod);
    wah_free_module(&provider);
}

// aefa56d: Fix global import resolution order in wah_instantiate.
// Globals imported after other import kinds must be resolved before local globals init.
static void test_global_import_order() {
    printf("Testing global import order (aefa56d)...\n");

    // Provider module: exports a global i32 with value 100.
    const char *provider_spec = "wasm \
        types {[]} \
        globals {[ i32 immut i32.const 100 end ]} \
        exports {[ {'g'} global# 0 ]}";

    // Consumer module: imports the global, then has a local global initialized via global.get 0
    // (the imported global). A function returns the local global.
    const char *consumer_spec = "wasm \
        types {[ fn [] [i32] ]} \
        imports {[ {'provider'} {'g'} global# i32 immut ]} \
        funcs {[ 0 ]} \
        globals {[ i32 immut global.get 0 end ]} \
        code {[ {[] global.get 1 end } ]}";

    wah_module_t provider = {0};
    assert_ok(wah_parse_module_from_spec(&provider, provider_spec));

    wah_module_t consumer = {0};
    assert_ok(wah_parse_module_from_spec(&consumer, consumer_spec));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &consumer));
    assert_ok(wah_link_module(&ctx, "provider", &provider));
    assert_ok(wah_instantiate(&ctx));

    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 100);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&consumer);
    wah_free_module(&provider);
}

// e445f7a: Verify import type compatibility during module linking.
static void test_import_type_mismatch() {
    printf("Testing import type mismatch (e445f7a)...\n");

    // Provider exports fn ()->(i32)
    const char *provider_spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        exports {[ {'f'} fn# 0 ]} \
        code {[ {[] i32.const 42 end } ]}";

    // Consumer imports fn ()->(i64) -- mismatched result type
    const char *consumer_bad = "wasm \
        types {[ fn [] [i64] ]} \
        imports {[ {'provider'} {'f'} fn# 0 ]} \
        funcs {[ 0 ]} \
        code {[ {[] call 0 end } ]}";

    wah_module_t provider = {0}, consumer = {0};
    assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
    assert_ok(wah_parse_module_from_spec(&consumer, consumer_bad));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &consumer));
    assert_ok(wah_link_module(&ctx, "provider", &provider));
    assert_err(wah_instantiate(&ctx), WAH_ERROR_IMPORT_NOT_FOUND);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&consumer);
    wah_free_module(&provider);

    // Provider exports fn (i32)->(i32), consumer imports fn ()->(i32) -- param count mismatch
    const char *provider_spec2 = "wasm \
        types {[ fn [i32] [i32] ]} \
        funcs {[ 0 ]} \
        exports {[ {'f'} fn# 0 ]} \
        code {[ {[] local.get 0 end } ]}";

    const char *consumer_bad2 = "wasm \
        types {[ fn [] [i32] ]} \
        imports {[ {'provider'} {'f'} fn# 0 ]}";

    wah_module_t prov2 = {0}, cons2 = {0};
    assert_ok(wah_parse_module_from_spec(&prov2, provider_spec2));
    assert_ok(wah_parse_module_from_spec(&cons2, consumer_bad2));

    wah_exec_context_t ctx2 = {0};
    assert_ok(wah_exec_context_create(&ctx2, &cons2));
    assert_ok(wah_link_module(&ctx2, "provider", &prov2));
    assert_err(wah_instantiate(&ctx2), WAH_ERROR_IMPORT_NOT_FOUND);

    wah_exec_context_destroy(&ctx2);
    wah_free_module(&cons2);
    wah_free_module(&prov2);
}

// 2830538: Fix immutable global import nullability check for cross-type subtyping.
static void test_global_import_nullability() {
    printf("Testing global import nullability (2830538)...\n");

    // Provider exports immutable global of type (ref null func) = nullable funcref
    const char *provider_spec = "wasm \
        types {[]} \
        globals {[ funcref immut ref.null funcref end ]} \
        exports {[ {'g'} global# 0 ]}";

    // Negative: consumer imports as (ref func) = non-nullable. Should fail.
    const char *consumer_bad = "wasm \
        types {[]} \
        imports {[ {'provider'} {'g'} global# type.ref.func immut ]}";

    wah_module_t provider = {0}, consumer_b = {0};
    assert_ok(wah_parse_module_from_spec(&provider, provider_spec));
    assert_ok(wah_parse_module_from_spec(&consumer_b, consumer_bad));

    wah_exec_context_t ctx = {0};
    assert_ok(wah_exec_context_create(&ctx, &consumer_b));
    assert_ok(wah_link_module(&ctx, "provider", &provider));
    assert_err(wah_instantiate(&ctx), WAH_ERROR_IMPORT_NOT_FOUND);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&consumer_b);

    // Positive: consumer imports as (ref null func) = nullable. Should succeed.
    const char *consumer_good = "wasm \
        types {[]} \
        imports {[ {'provider'} {'g'} global# funcref immut ]}";

    wah_module_t consumer_g = {0};
    assert_ok(wah_parse_module_from_spec(&consumer_g, consumer_good));

    wah_exec_context_t ctx2 = {0};
    assert_ok(wah_exec_context_create(&ctx2, &consumer_g));
    assert_ok(wah_link_module(&ctx2, "provider", &provider));
    assert_ok(wah_instantiate(&ctx2));

    wah_exec_context_destroy(&ctx2);
    wah_free_module(&consumer_g);
    wah_free_module(&provider);
}

int main(void) {
    printf("=== Import Section & Import Resolution Tests ===\n\n");

    test_basic_import_resolution();
    test_import_not_found_bad_module();
    test_import_not_found_bad_field();
    test_import_index_space();
    test_no_imports_unchanged();
    test_global_import_i32();
    test_global_import_set();
    test_global_import_with_local_globals();
    test_global_import_not_found();
    test_memory_import();
    test_memory_import_not_found();
    test_table_import();
    test_table_import_not_found();
    test_mixed_imports();
    test_global_import_order();
    test_import_type_mismatch();
    test_global_import_nullability();

    printf("All import tests passed!\n");
    return 0;
}
