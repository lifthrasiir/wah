#include "common.h"
#include <assert.h>

// return_call: func 0 (entry) calls func 1 via return_call
// func 0: [] -> [i32]   return_call 1
// func 1: [] -> [i32]   i32.const 42
static void test_return_call_basic(void) {
    printf("Test: return_call basic\n");
    const char *spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0, 0 ]} \
        code {[ \
            {[] return_call 1 end}, \
            {[] i32.const 42 end} \
        ]}";

    wah_module_t module = {0};
    wah_exec_context_t ctx = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    assert_ok(wah_exec_context_create(&ctx, &module));
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);
    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// return_call with parameters: func 0 takes (i32,i32), passes to func 1 via return_call
// func 0: [i32,i32] -> [i32]  local.get 0  local.get 1  return_call 1
// func 1: [i32,i32] -> [i32]  local.get 0  local.get 1  i32.add
static void test_return_call_with_params(void) {
    printf("Test: return_call with params\n");
    const char *spec = "wasm \
        types {[ fn [i32, i32] [i32], fn [i32, i32] [i32] ]} \
        funcs {[ 0, 1 ]} \
        code {[ \
            {[] local.get 0 local.get 1 return_call 1 end}, \
            {[] local.get 0 local.get 1 i32.add end} \
        ]}";

    wah_module_t module = {0};
    wah_exec_context_t ctx = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    assert_ok(wah_exec_context_create(&ctx, &module));
    wah_value_t result;
    wah_value_t params[2] = {{.i32 = 10}, {.i32 = 32}};
    assert_ok(wah_call(&ctx, 0, params, 2, &result));
    assert_eq_i32(result.i32, 42);
    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// return_call does not consume call stack: deep tail-call recursion
// func 0: [i32] -> [i32]
//   local.get 0  i32.eqz  if [i32]  i32.const 0
//   else  local.get 0  i32.const 1  i32.sub  return_call 0  end
static void test_return_call_deep_recursion(void) {
    printf("Test: return_call deep recursion (no stack overflow)\n");
    const char *spec = "wasm \
        types {[ fn [i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ \
            {[] \
                local.get 0 i32.eqz \
                if i32 \
                    i32.const 0 \
                else \
                    local.get 0 i32.const 1 i32.sub return_call 0 \
                end \
            end} \
        ]}";

    wah_module_t module = {0};
    wah_exec_context_t ctx = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    assert_ok(wah_exec_context_create(&ctx, &module));
    wah_value_t result;
    wah_value_t params[1] = {{.i32 = 100000}};
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 0);
    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// return_call_indirect basic
// Table with one function (func 1), func 0 calls it via return_call_indirect
static void test_return_call_indirect_basic(void) {
    printf("Test: return_call_indirect basic\n");
    const char *spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0, 0 ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        elements {[ elem.active.table#0 i32.const 0 end [ 1 ] ]} \
        code {[ \
            {[] i32.const 0 return_call_indirect 0 0 end}, \
            {[] i32.const 99 end} \
        ]}";

    wah_module_t module = {0};
    wah_exec_context_t ctx = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_instantiate(&ctx));
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 99);
    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// return_call_indirect through a table64 table.
static void test_return_call_indirect_table64(void) {
    printf("Test: return_call_indirect table64\n");
    const char *spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0, 0 ]} \
        tables {[ funcref limits.i64/1 1 ]} \
        elements {[ elem.active.table#0 i64.const 0 end [ 1 ] ]} \
        code {[ \
            {[] i64.const 0 return_call_indirect 0 0 end}, \
            {[] i32.const 123 end} \
        ]}";

    wah_module_t module = {0};
    wah_exec_context_t ctx = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_instantiate(&ctx));
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 123);
    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// return_call_indirect deep recursion
// func 0: [i32] -> [i32], table[0] = func 0
// if n == 0: return 0, else: return_call_indirect(n-1) through table[0]
static void test_return_call_indirect_deep_recursion(void) {
    printf("Test: return_call_indirect deep recursion (no stack overflow)\n");
    const char *spec = "wasm \
        types {[ fn [i32] [i32] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 1 ]} \
        elements {[ elem.active.table#0 i32.const 0 end [ 0 ] ]} \
        code {[ \
            {[] \
                local.get 0 i32.eqz \
                if i32 \
                    i32.const 0 \
                else \
                    local.get 0 i32.const 1 i32.sub \
                    i32.const 0 return_call_indirect 0 0 \
                end \
            end} \
        ]}";

    wah_module_t module = {0};
    wah_exec_context_t ctx = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_instantiate(&ctx));
    wah_value_t result;
    wah_value_t params[1] = {{.i32 = 100000}};
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 0);
    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// return_call_ref basic
static void test_return_call_ref_basic(void) {
    printf("Test: return_call_ref basic\n");
    // type 0: [] -> [i32]
    // func 0: ref.func 1, return_call_ref 0
    // func 1: i32.const 77
    const char *spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0, 0 ]} \
        elements {[ elem.declarative elem.funcref [ 1 ] ]} \
        code {[ \
            {[] ref.func 1 return_call_ref 0 end}, \
            {[] i32.const 77 end} \
        ]}";

    wah_module_t module = {0};
    wah_exec_context_t ctx = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_instantiate(&ctx));
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 77);
    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// return_call_ref null trap
static void test_return_call_ref_null_trap(void) {
    printf("Test: return_call_ref null trap\n");
    const char *spec = "wasm \
        types {[ fn [] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ \
            {[] ref.null 0 return_call_ref 0 end} \
        ]}";

    wah_module_t module = {0};
    wah_exec_context_t ctx = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_instantiate(&ctx));
    wah_value_t result;
    wah_error_t err = wah_call(&ctx, 0, NULL, 0, &result);
    assert(err == WAH_ERROR_TRAP && "expected trap for null call_ref");
    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// Mutual tail-call recursion: even/odd
// func 0 (is_even): [i32] -> [i32]
//   if n == 0: return 1, else: return_call 1 (n-1)
// func 1 (is_odd): [i32] -> [i32]
//   if n == 0: return 0, else: return_call 0 (n-1)
static void test_mutual_tail_recursion(void) {
    printf("Test: mutual tail recursion (even/odd)\n");
    const char *spec = "wasm \
        types {[ fn [i32] [i32] ]} \
        funcs {[ 0, 0 ]} \
        code {[ \
            {[] \
                local.get 0 i32.eqz \
                if i32 \
                    i32.const 1 \
                else \
                    local.get 0 i32.const 1 i32.sub return_call 1 \
                end \
            end}, \
            {[] \
                local.get 0 i32.eqz \
                if i32 \
                    i32.const 0 \
                else \
                    local.get 0 i32.const 1 i32.sub return_call 0 \
                end \
            end} \
        ]}";

    wah_module_t module = {0};
    wah_exec_context_t ctx = {0};
    assert_ok(wah_parse_module_from_spec(&module, spec));
    assert_ok(wah_exec_context_create(&ctx, &module));
    wah_value_t result;
    wah_value_t params[1];

    params[0].i32 = 100000;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 1);  // 100000 is even

    params[0].i32 = 100001;
    assert_ok(wah_call(&ctx, 0, params, 1, &result));
    assert_eq_i32(result.i32, 0);  // 100001 is odd

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
}

// 4283e74: Check nullability flags in return_call_ref return-type validation.
static void test_return_call_ref_nullability() {
    printf("Testing return_call_ref nullability validation (4283e74)...\n");

    // Negative: return_call_ref where callee returns (ref null func) but
    // the calling function's return type is (ref func) (non-nullable).
    // Type 0: fn [] -> [(ref func)] (non-nullable result)
    // Type 1: fn [] -> [(ref null func)] (nullable result)
    // Function 0 (type 0) does return_call_ref on a ref of type 1 - mismatch.
    const char *bad_spec = "wasm \
        types {[ sub [] fn [] [type.ref.func], sub [] fn [] [funcref] ]} \
        funcs {[ 0 ]} \
        elements {[ elem.declarative.expr funcref [ ref.func 0 end ] ]} \
        code {[ {[] ref.func 0 return_call_ref 1 end } ]}";

    wah_module_t bad = {0};
    assert_err(wah_parse_module_from_spec(&bad, bad_spec), WAH_ERROR_VALIDATION_FAILED);
    wah_free_module(&bad);
}

// Callee does return_call_indirect while caller has an active try_table handler.
// The handler must be unwound before the tail call reuses the frame.
static void test_return_call_indirect_exception_cleanup(void) {
    printf("Test: return_call_indirect exception handler cleanup\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32]]} \
        funcs {[0, 0, 0]} \
        tables {[funcref limits.i32/1 1]} \
        elements {[elem.active.table#0 i32.const 0 end [0]]} \
        code {[ \
            {[] i32.const 77 end}, \
            {[] i32.const 0 return_call_indirect 0 0 end}, \
            {[] \
                block void \
                    try_table i32 [catch_all 0] \
                        call 1 \
                    end \
                    return \
                end \
                i32.const -1 \
            end} \
        ]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod));
    assert_ok(wah_instantiate(&ctx));
    wah_value_t result;
    assert_ok(wah_call(&ctx, 2, NULL, 0, &result));
    assert_eq_i32(result.i32, 77);
    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_return_call_ref_exception_cleanup(void) {
    printf("Test: return_call_ref exception handler cleanup\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32]]} \
        funcs {[0, 0, 0]} \
        exports {[{'f0'} fn# 0]} \
        code {[ \
            {[] i32.const 88 end}, \
            {[] ref.func 0 return_call_ref 0 end}, \
            {[] \
                block void \
                    try_table i32 [catch_all 0] \
                        call 1 \
                    end \
                    return \
                end \
                i32.const -1 \
            end} \
        ]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod));
    assert_ok(wah_instantiate(&ctx));
    wah_value_t result;
    assert_ok(wah_call(&ctx, 2, NULL, 0, &result));
    assert_eq_i32(result.i32, 88);
    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

static void test_return_call_indirect_i64_exception_cleanup(void) {
    printf("Test: return_call_indirect_i64 exception handler cleanup\n");
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32]]} \
        funcs {[0, 0, 0]} \
        tables {[funcref limits.i64/1 1]} \
        elements {[elem.active.table#0 i64.const 0 end [0]]} \
        code {[ \
            {[] i32.const 99 end}, \
            {[] i64.const 0 return_call_indirect 0 0 end}, \
            {[] \
                block void \
                    try_table i32 [catch_all 0] \
                        call 1 \
                    end \
                    return \
                end \
                i32.const -1 \
            end} \
        ]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod));
    assert_ok(wah_instantiate(&ctx));
    wah_value_t result;
    assert_ok(wah_call(&ctx, 2, NULL, 0, &result));
    assert_eq_i32(result.i32, 99);
    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
}

int main(void) {
    test_return_call_basic();
    test_return_call_with_params();
    test_return_call_deep_recursion();
    test_return_call_indirect_basic();
    test_return_call_indirect_table64();
    test_return_call_indirect_deep_recursion();
    test_return_call_ref_basic();
    test_return_call_ref_null_trap();
    test_mutual_tail_recursion();
    test_return_call_ref_nullability();
    test_return_call_indirect_exception_cleanup();
    test_return_call_ref_exception_cleanup();
    test_return_call_indirect_i64_exception_cleanup();
    printf("All tail-call tests passed.\n");
    return 0;
}
