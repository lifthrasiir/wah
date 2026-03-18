// Tests for import section parsing and function import resolution
#define WAH_IMPLEMENTATION
#include <stdio.h>
#include "wah.h"
#include "wah_testutils.c"

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

static int test_basic_import_resolution(void) {
    printf("Test 1: Basic import resolution (WASM calls host function)\n");
    wah_module_t wasm_mod = {0}, env_mod = {0};
    wah_exec_context_t ctx = {0};
    int fail = 0;

    if (make_env_module(&env_mod) != WAH_OK) {
        printf("  FAIL: make_env_module\n"); fail = 1; goto done;
    }
    if (wah_parse_module_from_spec(&wasm_mod, k_import_spec) != WAH_OK) {
        printf("  FAIL: wah_parse_module_from_spec\n"); fail = 1; goto done;
    }
    if (wasm_mod.import_function_count != 1) {
        printf("  FAIL: import_function_count=%u (expected 1)\n",
               wasm_mod.import_function_count);
        fail = 1; goto done;
    }
    printf("  import_function_count=%u  function_count=%u\n",
           wasm_mod.import_function_count, wasm_mod.function_count);

    if (wah_exec_context_create(&ctx, &wasm_mod) != WAH_OK) {
        printf("  FAIL: wah_exec_context_create\n"); fail = 1; goto done;
    }
    if (wah_link_module(&ctx, "env", &env_mod) != WAH_OK) {
        printf("  FAIL: wah_link_module\n"); fail = 1; goto done;
    }
    if (wah_instantiate(&ctx) != WAH_OK) {
        printf("  FAIL: wah_instantiate\n"); fail = 1; goto done;
    }

    wah_entry_t entry;
    if (wah_module_export_by_name(&wasm_mod, "callAdd", &entry) != WAH_OK) {
        printf("  FAIL: export 'callAdd' not found\n"); fail = 1; goto done;
    }
    uint32_t export_idx = WAH_GET_ENTRY_INDEX(entry.id);
    if (export_idx != 1) {
        printf("  FAIL: callAdd global index=%u (expected 1)\n", export_idx);
        fail = 1; goto done;
    }
    printf("  callAdd global index=%u (expected 1)\n", export_idx);

    wah_value_t params[2] = {{ .i32 = 10 }, { .i32 = 20 }};
    wah_value_t result = {0};
    g_add_called = 0;
    if (wah_call(&ctx, entry.id, params, 2, &result) != WAH_OK) {
        printf("  FAIL: wah_call\n"); fail = 1; goto done;
    }
    if (!g_add_called) {
        printf("  FAIL: host_add was not called\n"); fail = 1; goto done;
    }
    if (result.i32 != 30) {
        printf("  FAIL: result=%d (expected 30)\n", result.i32); fail = 1; goto done;
    }
    printf("  callAdd(10, 20) = %d (expected 30)\n", result.i32);
    printf("  PASS\n\n");

done:
    wah_exec_context_destroy(&ctx);
    wah_free_module(&wasm_mod);
    wah_free_module(&env_mod);
    return fail;
}

static int test_import_not_found_bad_module(void) {
    printf("Test 2: WAH_ERROR_IMPORT_NOT_FOUND -- wrong module name\n");
    wah_module_t wasm_mod = {0}, env_mod = {0};
    wah_exec_context_t ctx = {0};
    int fail = 0;

    if (make_env_module(&env_mod) != WAH_OK) {
        printf("  FAIL: make_env_module\n"); fail = 1; goto done;
    }
    if (wah_parse_module_from_spec(&wasm_mod, k_import_spec) != WAH_OK) {
        printf("  FAIL: wah_parse_module_from_spec\n"); fail = 1; goto done;
    }
    if (wah_exec_context_create(&ctx, &wasm_mod) != WAH_OK) {
        printf("  FAIL: wah_exec_context_create\n"); fail = 1; goto done;
    }

    wah_link_module(&ctx, "wrong_name", &env_mod);  // intentionally wrong

    wah_error_t err = wah_instantiate(&ctx);
    if (err != WAH_ERROR_IMPORT_NOT_FOUND) {
        printf("  FAIL: expected WAH_ERROR_IMPORT_NOT_FOUND, got %s\n", wah_strerror(err));
        fail = 1; goto done;
    }
    printf("  PASS\n\n");

done:
    wah_exec_context_destroy(&ctx);
    wah_free_module(&wasm_mod);
    wah_free_module(&env_mod);
    return fail;
}

static int test_import_not_found_bad_field(void) {
    printf("Test 3: WAH_ERROR_IMPORT_NOT_FOUND -- wrong field name\n");
    wah_module_t wasm_mod = {0}, env_mod = {0};
    wah_exec_context_t ctx = {0};
    int fail = 0;

    if (wah_new_module(&env_mod) != WAH_OK) {
        printf("  FAIL: wah_new_module\n"); fail = 1; goto done;
    }
    wah_type_t params[2] = { WAH_TYPE_I32, WAH_TYPE_I32 };
    wah_type_t results[1] = { WAH_TYPE_I32 };
    wah_module_export_funcv(&env_mod, "wrong_func", 2, params, 1, results, host_add, NULL, NULL);

    if (wah_parse_module_from_spec(&wasm_mod, k_import_spec) != WAH_OK) {
        printf("  FAIL: wah_parse_module_from_spec\n"); fail = 1; goto done;
    }
    if (wah_exec_context_create(&ctx, &wasm_mod) != WAH_OK) {
        printf("  FAIL: wah_exec_context_create\n"); fail = 1; goto done;
    }

    wah_link_module(&ctx, "env", &env_mod);

    wah_error_t err = wah_instantiate(&ctx);
    if (err != WAH_ERROR_IMPORT_NOT_FOUND) {
        printf("  FAIL: expected WAH_ERROR_IMPORT_NOT_FOUND, got %s\n", wah_strerror(err));
        fail = 1; goto done;
    }
    printf("  PASS\n\n");

done:
    wah_exec_context_destroy(&ctx);
    wah_free_module(&wasm_mod);
    wah_free_module(&env_mod);
    return fail;
}

static int test_import_index_space(void) {
    printf("Test 4: Import + local index space -- callAdd export at global index 1\n");
    wah_module_t wasm_mod = {0};
    int fail = 0;

    if (wah_parse_module_from_spec(&wasm_mod, k_import_spec) != WAH_OK) {
        printf("  FAIL: wah_parse_module_from_spec\n"); fail = 1; goto done;
    }

    wah_entry_t entry;
    if (wah_module_export_by_name(&wasm_mod, "callAdd", &entry) != WAH_OK) {
        printf("  FAIL: export 'callAdd' not found\n"); fail = 1; goto done;
    }
    uint32_t idx = WAH_GET_ENTRY_INDEX(entry.id);
    if (idx != 1) {
        printf("  FAIL: expected global index 1, got %u\n", idx); fail = 1; goto done;
    }
    printf("  callAdd export global index = %u (expected 1)\n", idx);

    uint32_t nargs, nrets;
    const wah_type_t *args, *rets;
    if (wah_entry_func(&entry, &nargs, &args, &nrets, &rets) != WAH_OK) {
        printf("  FAIL: wah_entry_func\n"); fail = 1; goto done;
    }
    if (nargs != 2 || nrets != 1 ||
        args[0] != WAH_TYPE_I32 || args[1] != WAH_TYPE_I32 || rets[0] != WAH_TYPE_I32) {
        printf("  FAIL: unexpected type nargs=%u nrets=%u\n", nargs, nrets);
        fail = 1; goto done;
    }
    printf("  type: (i32,i32)->i32 -- correct\n");
    printf("  PASS\n\n");

done:
    wah_free_module(&wasm_mod);
    return fail;
}

static int test_no_imports_unchanged(void) {
    printf("Test 5: Module without imports works as before\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};
    int fail = 0;

    if (wah_parse_module_from_spec(&mod, "wasm \
        types {[ fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        exports {[ {'add'} fn# 0 ]} \
        code {[ {[] local.get 0 local.get 1 i32.add end} ]} \
    ") != WAH_OK) {
        printf("  FAIL: wah_parse_module_from_spec\n"); fail = 1; goto done;
    }
    if (mod.import_function_count != 0) {
        printf("  FAIL: import_function_count=%u (expected 0)\n",
               mod.import_function_count);
        fail = 1; goto done;
    }
    if (wah_exec_context_create(&ctx, &mod) != WAH_OK) {
        printf("  FAIL: wah_exec_context_create\n"); fail = 1; goto done;
    }

    wah_entry_t entry;
    if (wah_module_export_by_name(&mod, "add", &entry) != WAH_OK) {
        printf("  FAIL: export 'add' not found\n"); fail = 1; goto done;
    }
    if (WAH_GET_ENTRY_INDEX(entry.id) != 0) {
        printf("  FAIL: expected global index 0, got %u\n",
               (unsigned)WAH_GET_ENTRY_INDEX(entry.id));
        fail = 1; goto done;
    }

    wah_value_t params[2] = {{ .i32 = 7 }, { .i32 = 8 }};
    wah_value_t result = {0};
    if (wah_call(&ctx, entry.id, params, 2, &result) != WAH_OK) {
        printf("  FAIL: wah_call\n"); fail = 1; goto done;
    }
    if (result.i32 != 15) {
        printf("  FAIL: result=%d (expected 15)\n", result.i32); fail = 1; goto done;
    }
    printf("  add(7, 8) = %d (expected 15)\n", result.i32);
    printf("  PASS\n\n");

done:
    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
    return fail;
}

int main(void) {
    printf("=== Import Section & Import Resolution Tests ===\n\n");

    if (test_basic_import_resolution())    return 1;
    if (test_import_not_found_bad_module()) return 1;
    if (test_import_not_found_bad_field()) return 1;
    if (test_import_index_space())         return 1;
    if (test_no_imports_unchanged())       return 1;

    printf("All import tests passed!\n");
    return 0;
}
