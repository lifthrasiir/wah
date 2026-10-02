// Interrupts requested before a bulk op yield at its start without doing any work, then the op runs to completion.
// Yields in the middle of bulk ops share their code with fuel exhaustion, which test_bulk_fuel covers.
#define WAH_BULK_CHECK_INTERVAL 64
#define WAH_IMPLEMENTATION
#include "common.h"

#define REP8(x) x x x x x x x x
#define REP64(x) REP8(REP8(x))
#define REP128(x) REP64(x) REP64(x)

static void host_request_interrupt(wah_call_context_t *call, void *userdata) {
    (void)userdata;
    wah_request_interrupt_from_host(call);
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
    assert_ok(wah_export_func(&env, "interrupt", "()", host_request_interrupt, NULL, NULL));
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
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_link_module(&ctx, "env", &env));
    assert_ok(wah_instantiate(&ctx));

    assert_ok(wah_start(&ctx, 1, NULL, 0));
    wah_error_t err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_STATUS_YIELDED);
    assert_eq_u32(ctx.memory_base[0], 0x00);
    assert_eq_u32(ctx.memory_base[127], 0x00);

    resume_to_ok(&ctx);
    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0xAB);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
    wah_free_module(&env);
}

static void test_bulk_memory_copy_interrupt(void) {
    printf("Testing bulk memory.copy interrupt...\n");
    wah_module_t env = {0};
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_new_module(&env, NULL));
    assert_ok(wah_export_func(&env, "interrupt", "()", host_request_interrupt, NULL, NULL));
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
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_link_module(&ctx, "env", &env));
    assert_ok(wah_instantiate(&ctx));
    memset(ctx.memory_base, 0xCD, 128);

    assert_ok(wah_start(&ctx, 1, NULL, 0));
    wah_error_t err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_STATUS_YIELDED);
    assert_eq_u32(ctx.memory_base[256], 0x00);
    assert_eq_u32(ctx.memory_base[383], 0x00);

    resume_to_ok(&ctx);
    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0xCD);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
    wah_free_module(&env);
}

static void test_bulk_memory_copy_backward_interrupt(void) {
    printf("Testing backward bulk memory.copy interrupt...\n");
    wah_module_t env = {0};
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_new_module(&env, NULL));
    assert_ok(wah_export_func(&env, "interrupt", "()", host_request_interrupt, NULL, NULL));
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
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_link_module(&ctx, "env", &env));
    assert_ok(wah_instantiate(&ctx));
    memset(ctx.memory_base, 0xA1, 64);
    memset(ctx.memory_base + 64, 0xB2, 64);

    assert_ok(wah_start(&ctx, 1, NULL, 0));
    wah_error_t err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_STATUS_YIELDED);
    assert_eq_u32(ctx.memory_base[32], 0xA1);
    assert_eq_u32(ctx.memory_base[159], 0x00);

    resume_to_ok(&ctx);
    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0xA1);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
    wah_free_module(&env);
}

static void test_bulk_memory_init_interrupt(void) {
    printf("Testing bulk memory.init interrupt...\n");
    wah_module_t env = {0};
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_new_module(&env, NULL));
    assert_ok(wah_export_func(&env, "interrupt", "()", host_request_interrupt, NULL, NULL));
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
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_link_module(&ctx, "env", &env));
    assert_ok(wah_instantiate(&ctx));

    assert_ok(wah_start(&ctx, 1, NULL, 0));
    wah_error_t err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_STATUS_YIELDED);
    assert_eq_u32(ctx.memory_base[0], 0x00);
    assert_eq_u32(ctx.memory_base[127], 0x00);

    resume_to_ok(&ctx);
    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0x5A);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
    wah_free_module(&env);
}

static void test_bulk_memory64_copy_interrupt(void) {
    printf("Testing memory64 memory.copy interrupt...\n");
    wah_module_t env = {0};
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_new_module(&env, NULL));
    assert_ok(wah_export_func(&env, "interrupt", "()", host_request_interrupt, NULL, NULL));
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
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_link_module(&ctx, "env", &env));
    assert_ok(wah_instantiate(&ctx));
    memset(ctx.memory_base, 0xD4, 128);

    assert_ok(wah_start(&ctx, 1, NULL, 0));
    wah_error_t err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_STATUS_YIELDED);
    assert_eq_u32(ctx.memory_base[256], 0x00);
    assert_eq_u32(ctx.memory_base[383], 0x00);

    resume_to_ok(&ctx);
    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0xD4);

    wah_free_exec_context(&ctx);
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
    assert_ok(wah_export_func(&env, "interrupt", "()", host_request_interrupt, NULL, NULL));
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
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_link_module(&ctx, "env", &env));
    assert_ok(wah_instantiate(&ctx));

    assert_ok(wah_start(&ctx, 1, NULL, 0));
    wah_error_t err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_STATUS_YIELDED);
    assert_null(ctx.tables[0].entries[128].ref);
    assert_null(ctx.tables[0].entries[255].ref);

    resume_to_ok(&ctx);
    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
    wah_free_module(&env);
}

static void test_bulk_table_copy_backward_interrupt(void) {
    printf("Testing backward table.copy interrupt...\n");
    wah_module_t env = {0};
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_new_module(&env, NULL));
    assert_ok(wah_export_func(&env, "interrupt", "()", host_request_interrupt, NULL, NULL));
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
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_link_module(&ctx, "env", &env));
    assert_ok(wah_instantiate(&ctx));

    assert_ok(wah_start(&ctx, 1, NULL, 0));
    wah_error_t err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_STATUS_YIELDED);
    assert_not_null(ctx.tables[0].entries[32].ref);
    assert_null(ctx.tables[0].entries[159].ref);

    resume_to_ok(&ctx);
    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
    wah_free_module(&env);
}

static void test_bulk_table_init_interrupt(void) {
    printf("Testing table.init interrupt...\n");
    wah_module_t env = {0};
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_new_module(&env, NULL));
    assert_ok(wah_export_func(&env, "interrupt", "()", host_request_interrupt, NULL, NULL));
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
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_link_module(&ctx, "env", &env));
    assert_ok(wah_instantiate(&ctx));

    assert_ok(wah_start(&ctx, 1, NULL, 0));
    wah_error_t err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_STATUS_YIELDED);
    assert_null(ctx.tables[0].entries[0].ref);
    assert_null(ctx.tables[0].entries[127].ref);

    resume_to_ok(&ctx);
    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
    wah_free_module(&env);
}

static void test_bulk_table64_copy_interrupt(void) {
    printf("Testing table64 table.copy interrupt...\n");
    wah_module_t env = {0};
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_new_module(&env, NULL));
    assert_ok(wah_export_func(&env, "interrupt", "()", host_request_interrupt, NULL, NULL));
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
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_link_module(&ctx, "env", &env));
    assert_ok(wah_instantiate(&ctx));

    assert_ok(wah_start(&ctx, 1, NULL, 0));
    wah_error_t err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_STATUS_YIELDED);
    assert_null(ctx.tables[0].entries[128].ref);
    assert_null(ctx.tables[0].entries[255].ref);

    resume_to_ok(&ctx);
    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
    wah_free_module(&env);
}

static void test_bulk_table64_init_interrupt(void) {
    printf("Testing table64 table.init interrupt...\n");
    wah_module_t env = {0};
    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};

    assert_ok(wah_new_module(&env, NULL));
    assert_ok(wah_export_func(&env, "interrupt", "()", host_request_interrupt, NULL, NULL));
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
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_link_module(&ctx, "env", &env));
    assert_ok(wah_instantiate(&ctx));

    assert_ok(wah_start(&ctx, 1, NULL, 0));
    wah_error_t err = wah_resume(&ctx);
    assert_eq_i32(err, WAH_STATUS_YIELDED);
    assert_null(ctx.tables[0].entries[0].ref);
    assert_null(ctx.tables[0].entries[127].ref);

    resume_to_ok(&ctx);
    wah_value_t result;
    uint32_t actual;
    assert_ok(wah_finish(&ctx, &result, 1, &actual));
    assert_eq_i32(result.i32, 0);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
    wah_free_module(&env);
}

// Regression: bulk ops checked interrupts only between chunks, so that straight-line bulk ops smaller than a chunk
// were never interrupted, however many there were. They now yield at their start without doing any work.
static void test_bulk_small_ops_interrupt(void) {
    static const struct { const char *name, *body; } cases[] = {
        { "memory.fill", "i32.const 0 i32.const 0xAB i32.const 32 memory.fill 0" },
        { "memory.copy", "i32.const 0 i32.const 64 i32.const 32 memory.copy 0 0" },
        { "memory.init", "i32.const 0 i32.const 0 i32.const 32 memory.init 0 0" },
        { "table.fill", "i32.const 0 ref.null funcref i32.const 2 table.fill 0" },
        { "table.copy", "i32.const 0 i32.const 2 i32.const 2 table.copy 0 0" },
        { "table.init", "i32.const 0 i32.const 0 i32.const 1 table.init 0 0" },
        { "array.fill", "i32.const 4 array.new_default 0 i32.const 0 i32.const 1 i32.const 4 array.fill 0" },
        { "array.copy", "i32.const 4 array.new_default 0 i32.const 0 i32.const 4 array.new_default 0 i32.const 0 \
                         i32.const 4 array.copy 0 0" },
        { "array.init_data", "i32.const 4 array.new_default 0 i32.const 0 i32.const 0 i32.const 4 array.init_data 0 0" },
        { "array.init_elem", "i32.const 1 array.new_default 1 i32.const 0 i32.const 0 i32.const 1 array.init_elem 1 0" },
    };
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        printf("Testing small bulk %s interrupt...\n", cases[i].name);
        wah_module_t env = {0}, mod = {0};
        wah_exec_context_t ctx = {0};
        assert_ok(wah_new_module(&env, NULL));
        assert_ok(wah_export_func(&env, "interrupt", "()", host_request_interrupt, NULL, NULL));
        assert_ok(wah_parse_module_from_spec(&mod, "wasm \
            types {[ array i8 mut, array funcref mut, fn [] [], fn [] [i32] ]} \
            imports {[ {'env'} {'interrupt'} fn# 2 ]} funcs {[ 3 ]} \
            tables {[ funcref limits.i32/1 4 ]} memories {[ limits.i32/1 1 ]} \
            elements {[ elem.passive elem.funcref [1] ]} \
            datacount {1} \
            code {[ {[] call 0 %t i32.const 7 end} ]} \
            data {[ data.passive {%'" REP64("5A") "'} ]}", cases[i].body));
        assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
        assert_ok(wah_link_module(&ctx, "env", &env));
        assert_ok(wah_instantiate(&ctx));
        assert_ok(wah_start(&ctx, 1, NULL, 0));
        assert_eq_i32(wah_resume(&ctx), WAH_STATUS_YIELDED);
        assert_eq_u32(ctx.memory_base[0], 0x00); // memory.fill and memory.init haven't started
        resume_to_ok(&ctx);
        wah_value_t result;
        assert_ok(wah_finish(&ctx, &result, 1, NULL));
        assert_eq_i32(result.i32, 7);
        wah_free_exec_context(&ctx);
        wah_free_module(&mod);
        wah_free_module(&env);
    }
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
    test_bulk_small_ops_interrupt();

    printf("\n=== All bulk interrupt tests passed ===\n");
    return 0;
}
