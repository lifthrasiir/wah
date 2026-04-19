// Tests for import section parsing and function import resolution
#define WAH_IMPLEMENTATION
#include <stdio.h>
#include "../wah.h"
#include "common.c"

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
    wah_type_t params[2] = { WAH_TYPE_I32, WAH_TYPE_I32 };
    wah_type_t results[1] = { WAH_TYPE_I32 };
    return wah_module_export_funcv(mod, "add", 2, params, 1, results, host_add, NULL, NULL);
}

// ---------------------------------------------------------------------------

static void test_basic_import_resolution(void) {
    printf("Test 1: Basic import resolution (WASM calls host function)\n");
    wah_module_t wasm_mod = {0}, env_mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(make_env_module(&env_mod));
    assert_ok(wah_parse_module_from_spec(&wasm_mod, k_import_spec));
    assert_eq_u32(wasm_mod.import_function_count, 1);
    printf("  import_function_count=%u  function_count=%u\n",
           wasm_mod.import_function_count, wasm_mod.function_count);

    assert_ok(wah_exec_context_create(&ctx, &wasm_mod));
    assert_ok(wah_link_module(&ctx, "env", &env_mod));
    assert_ok(wah_instantiate(&ctx));

    wah_entry_t entry;
    assert_ok(wah_module_export_by_name(&wasm_mod, "callAdd", &entry));
    uint32_t export_idx = WAH_GET_ENTRY_INDEX(entry.id);
    assert_eq_u32(export_idx, 1);

    wah_value_t params[2] = {{ .i32 = 10 }, { .i32 = 20 }};
    wah_value_t result = {0};
    g_add_called = 0;
    assert_ok(wah_call(&ctx, entry.id, params, 2, &result));
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

    wah_type_t params[2] = { WAH_TYPE_I32, WAH_TYPE_I32 };
    wah_type_t results[1] = { WAH_TYPE_I32 };
    wah_module_export_funcv(&env_mod, "wrong_func", 2, params, 1, results, host_add, NULL, NULL);

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

    wah_entry_t entry;
    assert_ok(wah_module_export_by_name(&wasm_mod, "callAdd", &entry));
    uint32_t idx = WAH_GET_ENTRY_INDEX(entry.id);
    assert_eq_u32(idx, 1);

    uint32_t nargs, nrets;
    const wah_type_t *args, *rets;
    assert_ok(wah_entry_func(&entry, &nargs, &args, &nrets, &rets));
    assert_eq_u32(nargs, 2);
    assert_eq_u32(nrets, 1);
    assert_eq_u32(args[0], WAH_TYPE_I32);
    assert_eq_u32(args[1], WAH_TYPE_I32);
    assert_eq_u32(rets[0], WAH_TYPE_I32);

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

    wah_entry_t entry;
    assert_ok(wah_module_export_by_name(&mod, "add", &entry));
    assert_eq_u32(WAH_GET_ENTRY_INDEX(entry.id), 0);

    wah_value_t params[2] = {{ .i32 = 7 }, { .i32 = 8 }};
    wah_value_t result = {0};
    assert_ok(wah_call(&ctx, entry.id, params, 2, &result));
    assert_eq_i32(result.i32, 15);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

int main(void) {
    printf("=== Import Section & Import Resolution Tests ===\n\n");

    test_basic_import_resolution();
    test_import_not_found_bad_module();
    test_import_not_found_bad_field();
    test_import_index_space();
    test_no_imports_unchanged();

    printf("All import tests passed!\n");
    return 0;
}
