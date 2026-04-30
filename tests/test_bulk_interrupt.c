#define WAH_BULK_CHECK_INTERVAL 64
#define WAH_IMPLEMENTATION
#include "common.h"

#define REP8(x) x x x x x x x x
#define REP64(x) REP8(REP8(x))
#define REP128(x) REP64(x) REP64(x)

static void host_request_interrupt(wah_call_context_t *call, void *userdata) {
    (void)userdata;
    wah_request_interrupt(call->exec);
}

static void resume_to_ok(wah_exec_context_t *ctx) {
    wah_error_t err;
    while ((err = wah_resume(ctx)) > 0) {
    }
    assert_ok(err);
}

static void test_bulk_memory_fill_interrupt(void) {
    printf("Testing bulk memory.fill interrupt...\n");
    wah_module_t env = {0};
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_new_module(&env, NULL));
    assert_ok(wah_module_export_func(&env, "interrupt", "()", host_request_interrupt, NULL, NULL));
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32], fn [] []]} \
        imports {[{'env'} {'interrupt'} fn# 1]} \
        funcs {[0]} \
        memories {[limits.i32/1 1]} \
        code {[{[] \
            call 0 \
            i32.const 0 i32.const 0xAB i32.const 128 memory.fill 0 \
            i32.const 127 i32.load8_u 0 0 \
        end}]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_link_module(&ctx, "env", &env));
    assert_ok(wah_instantiate(&ctx));

    assert_ok(wah_start(&ctx, 1, NULL, 0));
    wah_error_t err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_STATUS_YIELDED);
    assert_eq_u32(ctx.memory_base[0], 0xAB);
    assert_eq_u32(ctx.memory_base[63], 0xAB);
    assert_eq_u32(ctx.memory_base[64], 0x00);

    resume_to_ok(&ctx);
    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0xAB);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
    wah_free_module(&env);
}

static void test_bulk_memory_copy_interrupt(void) {
    printf("Testing bulk memory.copy interrupt...\n");
    wah_module_t env = {0};
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_new_module(&env, NULL));
    assert_ok(wah_module_export_func(&env, "interrupt", "()", host_request_interrupt, NULL, NULL));
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32], fn [] []]} \
        imports {[{'env'} {'interrupt'} fn# 1]} \
        funcs {[0]} \
        memories {[limits.i32/1 1]} \
        code {[{[] \
            call 0 \
            i32.const 256 i32.const 0 i32.const 128 memory.copy 0 0 \
            i32.const 383 i32.load8_u 0 0 \
        end}]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_link_module(&ctx, "env", &env));
    assert_ok(wah_instantiate(&ctx));
    memset(ctx.memory_base, 0xCD, 128);

    assert_ok(wah_start(&ctx, 1, NULL, 0));
    wah_error_t err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_STATUS_YIELDED);
    assert_eq_u32(ctx.memory_base[256], 0xCD);
    assert_eq_u32(ctx.memory_base[319], 0xCD);
    assert_eq_u32(ctx.memory_base[320], 0x00);

    resume_to_ok(&ctx);
    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0xCD);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
    wah_free_module(&env);
}

static void test_bulk_memory_copy_backward_interrupt(void) {
    printf("Testing backward bulk memory.copy interrupt...\n");
    wah_module_t env = {0};
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_new_module(&env, NULL));
    assert_ok(wah_module_export_func(&env, "interrupt", "()", host_request_interrupt, NULL, NULL));
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32], fn [] []]} \
        imports {[{'env'} {'interrupt'} fn# 1]} \
        funcs {[0]} \
        memories {[limits.i32/1 1]} \
        code {[{[] \
            call 0 \
            i32.const 32 i32.const 0 i32.const 128 memory.copy 0 0 \
            i32.const 32 i32.load8_u 0 0 \
        end}]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_link_module(&ctx, "env", &env));
    assert_ok(wah_instantiate(&ctx));
    memset(ctx.memory_base, 0xA1, 64);
    memset(ctx.memory_base + 64, 0xB2, 64);

    assert_ok(wah_start(&ctx, 1, NULL, 0));
    wah_error_t err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_STATUS_YIELDED);
    assert_eq_u32(ctx.memory_base[96], 0xB2);
    assert_eq_u32(ctx.memory_base[159], 0xB2);

    resume_to_ok(&ctx);
    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0xA1);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
    wah_free_module(&env);
}

static void test_bulk_memory_init_interrupt(void) {
    printf("Testing bulk memory.init interrupt...\n");
    wah_module_t env = {0};
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_new_module(&env, NULL));
    assert_ok(wah_module_export_func(&env, "interrupt", "()", host_request_interrupt, NULL, NULL));
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32], fn [] []]} \
        imports {[{'env'} {'interrupt'} fn# 1]} \
        funcs {[0]} \
        memories {[limits.i32/1 1]} \
        datacount {1} \
        code {[{[] \
            call 0 \
            i32.const 0 i32.const 0 i32.const 128 memory.init 0 0 \
            i32.const 127 i32.load8_u 0 0 \
        end}]} \
        data {[data.passive {%'" REP128("5A") "'}]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_link_module(&ctx, "env", &env));
    assert_ok(wah_instantiate(&ctx));

    assert_ok(wah_start(&ctx, 1, NULL, 0));
    wah_error_t err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_STATUS_YIELDED);
    assert_eq_u32(ctx.memory_base[0], 0x5A);
    assert_eq_u32(ctx.memory_base[63], 0x5A);
    assert_eq_u32(ctx.memory_base[64], 0x00);

    resume_to_ok(&ctx);
    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0x5A);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
    wah_free_module(&env);
}

static void test_bulk_memory64_copy_interrupt(void) {
    printf("Testing memory64 memory.copy interrupt...\n");
    wah_module_t env = {0};
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_new_module(&env, NULL));
    assert_ok(wah_module_export_func(&env, "interrupt", "()", host_request_interrupt, NULL, NULL));
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32], fn [] []]} \
        imports {[{'env'} {'interrupt'} fn# 1]} \
        funcs {[0]} \
        memories {[limits.i64/2 1 1]} \
        code {[{[] \
            call 0 \
            i64.const 256 i64.const 0 i64.const 128 memory.copy 0 0 \
            i64.const 383 i32.load8_u 0 0 \
        end}]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_link_module(&ctx, "env", &env));
    assert_ok(wah_instantiate(&ctx));
    memset(ctx.memory_base, 0xD4, 128);

    assert_ok(wah_start(&ctx, 1, NULL, 0));
    wah_error_t err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_STATUS_YIELDED);
    assert_eq_u32(ctx.memory_base[256], 0xD4);
    assert_eq_u32(ctx.memory_base[319], 0xD4);
    assert_eq_u32(ctx.memory_base[320], 0x00);

    resume_to_ok(&ctx);
    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0xD4);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
    wah_free_module(&env);
}

#define FUNC0_REF "0, "

static void test_bulk_table_copy_interrupt(void) {
    printf("Testing table.copy interrupt...\n");
    wah_module_t env = {0};
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_new_module(&env, NULL));
    assert_ok(wah_module_export_func(&env, "interrupt", "()", host_request_interrupt, NULL, NULL));
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32], fn [] []]} \
        imports {[{'env'} {'interrupt'} fn# 1]} \
        funcs {[0]} \
        tables {[funcref limits.i32/2 300 300]} \
        elements {[elem.passive elem.funcref [" REP128(FUNC0_REF) "]]} \
        code {[{[] \
            i32.const 0 i32.const 0 i32.const 128 table.init 0 0 \
            call 0 \
            i32.const 128 i32.const 0 i32.const 128 table.copy 0 0 \
            i32.const 255 table.get 0 ref.is_null \
        end}]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_link_module(&ctx, "env", &env));
    assert_ok(wah_instantiate(&ctx));

    assert_ok(wah_start(&ctx, 1, NULL, 0));
    wah_error_t err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_STATUS_YIELDED);
    assert_not_null(ctx.tables[0].entries[128].ref);
    assert_not_null(ctx.tables[0].entries[191].ref);
    assert_null(ctx.tables[0].entries[192].ref);

    resume_to_ok(&ctx);
    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
    wah_free_module(&env);
}

static void test_bulk_table_copy_backward_interrupt(void) {
    printf("Testing backward table.copy interrupt...\n");
    wah_module_t env = {0};
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_new_module(&env, NULL));
    assert_ok(wah_module_export_func(&env, "interrupt", "()", host_request_interrupt, NULL, NULL));
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32], fn [] []]} \
        imports {[{'env'} {'interrupt'} fn# 1]} \
        funcs {[0]} \
        tables {[funcref limits.i32/2 300 300]} \
        elements {[elem.passive elem.funcref [" REP128(FUNC0_REF) "]]} \
        code {[{[] \
            i32.const 0 i32.const 0 i32.const 128 table.init 0 0 \
            call 0 \
            i32.const 32 i32.const 0 i32.const 128 table.copy 0 0 \
            i32.const 32 table.get 0 ref.is_null \
        end}]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_link_module(&ctx, "env", &env));
    assert_ok(wah_instantiate(&ctx));

    assert_ok(wah_start(&ctx, 1, NULL, 0));
    wah_error_t err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_STATUS_YIELDED);
    assert_not_null(ctx.tables[0].entries[96].ref);
    assert_not_null(ctx.tables[0].entries[159].ref);

    resume_to_ok(&ctx);
    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
    wah_free_module(&env);
}

static void test_bulk_table_init_interrupt(void) {
    printf("Testing table.init interrupt...\n");
    wah_module_t env = {0};
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_new_module(&env, NULL));
    assert_ok(wah_module_export_func(&env, "interrupt", "()", host_request_interrupt, NULL, NULL));
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32], fn [] []]} \
        imports {[{'env'} {'interrupt'} fn# 1]} \
        funcs {[0]} \
        tables {[funcref limits.i32/2 300 300]} \
        elements {[elem.passive elem.funcref [" REP128(FUNC0_REF) "]]} \
        code {[{[] \
            call 0 \
            i32.const 0 i32.const 0 i32.const 128 table.init 0 0 \
            i32.const 127 table.get 0 ref.is_null \
        end}]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_link_module(&ctx, "env", &env));
    assert_ok(wah_instantiate(&ctx));

    assert_ok(wah_start(&ctx, 1, NULL, 0));
    wah_error_t err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_STATUS_YIELDED);
    assert_not_null(ctx.tables[0].entries[0].ref);
    assert_not_null(ctx.tables[0].entries[63].ref);
    assert_null(ctx.tables[0].entries[64].ref);

    resume_to_ok(&ctx);
    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
    wah_free_module(&env);
}

static void test_bulk_table64_copy_interrupt(void) {
    printf("Testing table64 table.copy interrupt...\n");
    wah_module_t env = {0};
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_new_module(&env, NULL));
    assert_ok(wah_module_export_func(&env, "interrupt", "()", host_request_interrupt, NULL, NULL));
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32], fn [] []]} \
        imports {[{'env'} {'interrupt'} fn# 1]} \
        funcs {[0]} \
        tables {[funcref limits.i64/2 300 300]} \
        elements {[elem.passive elem.funcref [" REP128(FUNC0_REF) "]]} \
        code {[{[] \
            i64.const 0 i32.const 0 i32.const 128 table.init 0 0 \
            call 0 \
            i64.const 128 i64.const 0 i64.const 128 table.copy 0 0 \
            i64.const 255 table.get 0 ref.is_null \
        end}]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_link_module(&ctx, "env", &env));
    assert_ok(wah_instantiate(&ctx));

    assert_ok(wah_start(&ctx, 1, NULL, 0));
    wah_error_t err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_STATUS_YIELDED);
    assert_not_null(ctx.tables[0].entries[128].ref);
    assert_not_null(ctx.tables[0].entries[191].ref);
    assert_null(ctx.tables[0].entries[192].ref);

    resume_to_ok(&ctx);
    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
    wah_free_module(&env);
}

static void test_bulk_table64_init_interrupt(void) {
    printf("Testing table64 table.init interrupt...\n");
    wah_module_t env = {0};
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_new_module(&env, NULL));
    assert_ok(wah_module_export_func(&env, "interrupt", "()", host_request_interrupt, NULL, NULL));
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32], fn [] []]} \
        imports {[{'env'} {'interrupt'} fn# 1]} \
        funcs {[0]} \
        tables {[funcref limits.i64/2 300 300]} \
        elements {[elem.passive elem.funcref [" REP128(FUNC0_REF) "]]} \
        code {[{[] \
            call 0 \
            i64.const 0 i32.const 0 i32.const 128 table.init 0 0 \
            i64.const 127 table.get 0 ref.is_null \
        end}]}"));
    assert_ok(wah_exec_context_create(&ctx, &mod, NULL));
    assert_ok(wah_link_module(&ctx, "env", &env));
    assert_ok(wah_instantiate(&ctx));

    assert_ok(wah_start(&ctx, 1, NULL, 0));
    wah_error_t err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_STATUS_YIELDED);
    assert_not_null(ctx.tables[0].entries[0].ref);
    assert_not_null(ctx.tables[0].entries[63].ref);
    assert_null(ctx.tables[0].entries[64].ref);

    resume_to_ok(&ctx);
    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&mod);
    wah_free_module(&env);
}

int main(void) {
    test_bulk_memory_fill_interrupt();
    test_bulk_memory_copy_interrupt();
    test_bulk_memory_copy_backward_interrupt();
    test_bulk_memory_init_interrupt();
    test_bulk_memory64_copy_interrupt();
    test_bulk_table_copy_interrupt();
    test_bulk_table_copy_backward_interrupt();
    test_bulk_table_init_interrupt();
    test_bulk_table64_copy_interrupt();
    test_bulk_table64_init_interrupt();

    printf("\n=== All bulk interrupt tests passed ===\n");
    return 0;
}
