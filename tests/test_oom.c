#include "../wah.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    size_t fail_after;
    size_t attempts;
    size_t failed_size;
    int failed_realloc;
    size_t allocs;
    size_t frees;
    size_t outstanding;
    uintptr_t sum;
    uintptr_t xorv;
} oom_alloc_t;

typedef wah_error_t (*oom_op_t)(oom_alloc_t *state, void *userdata);
typedef void (*oom_cleanup_t)(void *userdata);

static void track_alloc(oom_alloc_t *state, void *ptr) {
    uintptr_t h = wah_test_perturb_ptr(ptr);
    state->allocs++;
    state->outstanding++;
    state->sum += h;
    state->xorv ^= h;
}

static void track_free_hash(oom_alloc_t *state, uintptr_t h) {
    state->frees++;
    state->outstanding--;
    state->sum -= h;
    state->xorv ^= h;
}

static void track_free(oom_alloc_t *state, void *ptr) {
    track_free_hash(state, wah_test_perturb_ptr(ptr));
}

static int should_fail(oom_alloc_t *state) {
    state->attempts++;
    return state->fail_after != 0 && state->attempts == state->fail_after;
}

static void *oom_malloc(size_t size, void *userdata) {
    oom_alloc_t *state = (oom_alloc_t *)userdata;
    if (size == 0) {
        void *ptr = malloc(1);
        if (ptr) track_alloc(state, ptr);
        return ptr;
    }
    if (should_fail(state)) {
        state->failed_size = size;
        state->failed_realloc = 0;
        return NULL;
    }
    void *ptr = malloc(size ? size : 1);
    if (ptr) track_alloc(state, ptr);
    return ptr;
}

static void *oom_realloc(void *ptr, size_t size, void *userdata) {
    oom_alloc_t *state = (oom_alloc_t *)userdata;
    if (!ptr) return oom_malloc(size, userdata);
    uintptr_t old_hash = wah_test_perturb_ptr(ptr);
    if (size == 0) {
        track_free_hash(state, old_hash);
        free(ptr);
        return NULL;
    }
    if (should_fail(state)) {
        state->failed_size = size;
        state->failed_realloc = 1;
        return NULL;
    }
    void *new_ptr = realloc(ptr, size);
    if (new_ptr) {
        track_free_hash(state, old_hash);
        track_alloc(state, new_ptr);
    }
    return new_ptr;
}

static void oom_free(void *ptr, void *userdata) {
    oom_alloc_t *state = (oom_alloc_t *)userdata;
    if (!ptr) return;
    track_free(state, ptr);
    free(ptr);
}

static wah_alloc_t make_oom_alloc(oom_alloc_t *state) {
    return (wah_alloc_t){ oom_malloc, oom_realloc, oom_free, state };
}

static void assert_no_leaks(const char *name, const oom_alloc_t *state) {
    if (state->outstanding == 0 && state->sum == 0 && state->xorv == 0) return;
    fprintf(stderr, "%s leaked allocator state: allocs=%zu frees=%zu outstanding=%zu sum=%zx xor=%zx\n",
            name, state->allocs, state->frees, state->outstanding,
            (size_t)state->sum, (size_t)state->xorv);
    exit(1);
}

static void run_oom_loop(const char *name, oom_op_t op, oom_cleanup_t cleanup, void *userdata) {
    printf("Testing OOM cleanup: %s...\n", name);
    fflush(stdout);
    for (size_t fail_at = 1; ; ++fail_at) {
        oom_alloc_t state = { .fail_after = fail_at };
        wah_error_t err = op(&state, userdata);
        if (err == WAH_OK) {
            if (cleanup) cleanup(userdata);
            assert_no_leaks(name, &state);
            printf("  success after %zu allocation attempts\n", fail_at - 1);
            return;
        }
        if (cleanup) cleanup(userdata);
        if (err != WAH_ERROR_OUT_OF_MEMORY) {
            fprintf(stderr, "%s fail_at=%zu: expected OOM or OK, got %s (failed %s size=%zu)\n",
                    name, fail_at, wah_strerror(err),
                    state.failed_realloc ? "realloc" : "malloc", state.failed_size);
            exit(1);
        }
        assert_no_leaks(name, &state);
        if (fail_at > 10000) {
            fprintf(stderr, "%s did not reach success\n", name);
            exit(1);
        }
    }
}

static void host_id(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    wah_return_i32(ctx, wah_param_i32(ctx, 0));
}

typedef struct {
    wah_module_t module;
} module_case_t;

static void cleanup_module_case(void *userdata) {
    module_case_t *c = (module_case_t *)userdata;
    wah_free_module(&c->module);
    c->module = (wah_module_t){0};
}

static wah_error_t op_parse_complex_module(oom_alloc_t *state, void *userdata) {
    module_case_t *c = (module_case_t *)userdata;
    wah_alloc_t alloc = make_oom_alloc(state);
    wah_parse_options_t opts = { .alloc = &alloc, .features = WAH_FEATURE_ALL };
    const char *spec = "wasm \
        types {[ \
            struct [i32 mut, funcref mut], \
            array i32 mut, \
            fn [i32] [i32], \
            fn [i32] [] \
        ]} \
        imports {[ {'env'} {'host'} fn# 2, {'env'} {'tag'} tag# tag.type# 3 ]} \
        funcs {[ 2 ]} \
        tables {[ funcref limits.i32/1 2 ]} \
        memories {[ limits.i32/1 1 ]} \
        tags {[ tag.type# 3 ]} \
        globals {[ i32 mut i32.const 7 end ]} \
        exports {[ {'id'} fn# 1, {'tbl'} table# 0, {'mem'} mem# 0, {'tag'} export.tag 1, {'g'} global# 0 ]} \
        datacount { 2 } \
        code {[ {[] local.get 0 end} ]} \
        data {[ data.active.table#0 i32.const 0 end {'abc'}, data.passive {'xyz'} ]}";
    return wah_parse_module_from_spec_ex(&c->module, &opts, spec);
}

static wah_error_t op_define_type(oom_alloc_t *state, void *userdata) {
    module_case_t *c = (module_case_t *)userdata;
    wah_alloc_t alloc = make_oom_alloc(state);
    wah_error_t err = wah_new_module_ex(&c->module, &alloc);
    wah_type_t first, second;
    if (err != WAH_OK) return err;
    err = wah_module_define_type(&c->module, &first,
        "fresh struct { i64, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, mut funcref, externref }");
    if (err != WAH_OK) return err;
    return wah_module_define_type(&c->module, &second,
        "fresh struct { i64, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, mut funcref, externref }");
}

typedef struct {
    wah_module_t module;
    wah_exec_context_t ctx;
} context_case_t;

static void cleanup_context_case(void *userdata) {
    context_case_t *c = (context_case_t *)userdata;
    wah_exec_context_destroy(&c->ctx);
    c->ctx = (wah_exec_context_t){0};
}

static wah_error_t op_context_create_deadline(oom_alloc_t *state, void *userdata) {
    context_case_t *c = (context_case_t *)userdata;
    wah_alloc_t alloc = make_oom_alloc(state);
    wah_exec_options_t opts = { .alloc = &alloc, .limits = { .deadline = 1000000 } };
    return wah_exec_context_create_ex(&c->ctx, &c->module, &opts);
}

typedef struct {
    wah_module_t provider;
    wah_module_t consumer;
    wah_exec_context_t ctx;
} instantiate_case_t;

static void cleanup_instantiate_case(void *userdata) {
    instantiate_case_t *c = (instantiate_case_t *)userdata;
    wah_exec_context_destroy(&c->ctx);
    c->ctx = (wah_exec_context_t){0};
}

static wah_error_t op_instantiate_linked(oom_alloc_t *state, void *userdata) {
    instantiate_case_t *c = (instantiate_case_t *)userdata;
    wah_alloc_t alloc = make_oom_alloc(state);
    wah_exec_options_t opts = { .alloc = &alloc };
    wah_error_t err = wah_exec_context_create_ex(&c->ctx, &c->consumer, &opts);
    if (err != WAH_OK) return err;
    err = wah_link_module(&c->ctx, "provider", &c->provider);
    if (err != WAH_OK) return err;
    err = wah_gc_start(&c->ctx);
    if (err != WAH_OK) return err;
    return wah_instantiate(&c->ctx);
}

static wah_error_t op_host_builder(oom_alloc_t *state, void *userdata) {
    module_case_t *c = (module_case_t *)userdata;
    wah_alloc_t alloc = make_oom_alloc(state);
    wah_error_t err = wah_new_module_ex(&c->module, &alloc);
    wah_v128_t v = {{0}};
    if (err != WAH_OK) return err;
    err = wah_module_export_func(&c->module, "id",
                                 "(i32, i64, f32, f64, funcref, externref, v128, i32, i64) -> (i32, i64, funcref)",
                                 host_id, NULL, NULL);
    if (err != WAH_OK) return err;
    err = wah_module_export_memory(&c->module, "memory", 1, 1);
    if (err != WAH_OK) return err;
    err = wah_module_export_global_i32(&c->module, "g_i32", true, 7);
    if (err != WAH_OK) return err;
    err = wah_module_export_global_i64(&c->module, "g_i64", false, 8);
    if (err != WAH_OK) return err;
    err = wah_module_export_global_f32(&c->module, "g_f32", false, 1.25f);
    if (err != WAH_OK) return err;
    err = wah_module_export_global_f64(&c->module, "g_f64", false, 2.5);
    if (err != WAH_OK) return err;
    return wah_module_export_global_v128(&c->module, "g_v128", false, &v);
}

typedef struct {
    wah_module_t module;
    wah_exec_context_t ctx;
    wah_alloc_t saved_alloc;
} runtime_case_t;

static void cleanup_runtime_call(void *userdata) {
    runtime_case_t *c = (runtime_case_t *)userdata;
    wah_cancel(&c->ctx);
    c->ctx.alloc = c->saved_alloc;
}

static wah_error_t op_runtime_throw(oom_alloc_t *state, void *userdata) {
    runtime_case_t *c = (runtime_case_t *)userdata;
    c->ctx.alloc = make_oom_alloc(state);
    wah_value_t result = {0};
    return wah_call(&c->ctx, 0, NULL, 0, &result);
}

static void prepare_context_case(context_case_t *c) {
    const char *spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 2 ]} \
        memories {[ limits.i32/1 1 ]} \
        tags {[ tag.type# 0 ]} \
        globals {[ i32 mut i32.const 3 end ]} \
        exports {[ {'f'} fn# 0, {'tag'} export.tag 0 ]} \
        code {[ {[] end} ]}";
    assert_ok(wah_parse_module_from_spec(&c->module, spec));
}

static void prepare_instantiate_case(instantiate_case_t *c) {
    const char *provider_spec = "wasm \
        types {[ sub [] struct [i32 mut], fn [i32] [], fn [] [i32] ]} \
        funcs {[ 2 ]} \
        tags {[ tag.type# 1 ]} \
        globals {[ \
            i32 immut i32.const 10 end, \
            type.ref.null 0 immut i32.const 1 struct.new 0 end, \
            type.ref.null 0 immut i32.const 2 struct.new 0 end, \
            type.ref.null 0 immut i32.const 3 struct.new 0 end, \
            type.ref.null 0 immut i32.const 4 struct.new 0 end, \
            type.ref.null 0 immut i32.const 5 struct.new 0 end, \
            type.ref.null 0 immut i32.const 6 struct.new 0 end, \
            type.ref.null 0 immut i32.const 7 struct.new 0 end, \
            i32 immut global.get 0 end \
        ]} \
        exports {[ {'tag'} export.tag 0, {'g'} global# 0, {'get'} fn# 0 ]} \
        code {[ {[] global.get 0 end} ]}";
    const char *consumer_spec = "wasm \
        types {[ sub [] struct [i32 mut], fn [i32] [], fn [] [i32] ]} \
        imports {[ {'provider'} {'tag'} tag# tag.type# 1, {'provider'} {'g'} global# i32 immut ]} \
        funcs {[ 2, 2, 2 ]} \
        tables {[ funcref limits.i32/1 3, funcref limits.i32/1 2 ]} \
        memories {[ limits.i32/1 1, limits.i32/1 1 ]} \
        tags {[ tag.type# 1 ]} \
        globals {[ \
            i32 immut global.get 0 end, \
            i32 immut global.get 1 end, \
            i32 immut global.get 2 end, \
            i32 immut global.get 3 end \
        ]} \
        exports {[ {'f'} fn# 0, {'tag'} export.tag 1 ]} \
        elements {[ elem.active.table#0 i32.const 0 end [ 0, 1, 2 ] ]} \
        code {[ {[] global.get 1 end}, {[] global.get 2 end}, {[] global.get 3 end} ]} \
        data {[ \
            data.active.table#0 i32.const 0 end {'hi'}, \
            data.active.table# 1 i32.const 0 end {'ok'}, \
            data.passive {'passive'} \
        ]}";
    assert_ok(wah_parse_module_from_spec(&c->provider, provider_spec));
    assert_ok(wah_parse_module_from_spec(&c->consumer, consumer_spec));
}

static void prepare_runtime_case(runtime_case_t *c) {
    const char *spec = "wasm \
        types {[ fn [] [i32], fn [i32] [] ]} \
        funcs {[ 0 ]} \
        tags {[ tag.type# 1 ]} \
        code {[ {[] \
            block i32 \
                try_table void [catch 0 1] \
                    i32.const 11 \
                    throw 0 \
                end \
                i32.const 0 \
            end \
        end } ]}";
    assert_ok(wah_parse_module_from_spec(&c->module, spec));
    assert_ok(wah_exec_context_create(&c->ctx, &c->module));
    assert_ok(wah_instantiate(&c->ctx));
    c->saved_alloc = c->ctx.alloc;
}

static void prepare_runtime_throw_ref_case(runtime_case_t *c) {
    const char *spec = "wasm \
        types {[ fn [] [i32], fn [i32] [], fn [] [i32, exnref] ]} \
        funcs {[ 0 ]} \
        tags {[ tag.type# 1 ]} \
        code {[ {[] \
            block i32 \
                try_table void [catch 0 1] \
                    block 2 \
                        try_table void [catch_ref 0 0] \
                            i32.const 42 \
                            throw 0 \
                        end \
                        i32.const 0 \
                        ref.null exnref \
                    end \
                    throw_ref \
                    unreachable \
                end \
                i32.const -1 \
            end \
        end } ]}";
    assert_ok(wah_parse_module_from_spec(&c->module, spec));
    assert_ok(wah_exec_context_create(&c->ctx, &c->module));
    assert_ok(wah_instantiate(&c->ctx));
    c->saved_alloc = c->ctx.alloc;
}

int main(void) {
    module_case_t parse_case = {0};
    run_oom_loop("wah_parse_module", op_parse_complex_module, cleanup_module_case, &parse_case);

    module_case_t define_case = {0};
    run_oom_loop("wah_module_define_type", op_define_type, cleanup_module_case, &define_case);

    context_case_t context_case = {0};
    prepare_context_case(&context_case);
    run_oom_loop("wah_exec_context_create deadline", op_context_create_deadline, cleanup_context_case, &context_case);
    wah_free_module(&context_case.module);

    instantiate_case_t instantiate_case = {0};
    prepare_instantiate_case(&instantiate_case);
    run_oom_loop("wah_instantiate linked modules", op_instantiate_linked, cleanup_instantiate_case, &instantiate_case);
    wah_free_module(&instantiate_case.consumer);
    wah_free_module(&instantiate_case.provider);

    module_case_t builder_case = {0};
    run_oom_loop("host module builder APIs", op_host_builder, cleanup_module_case, &builder_case);

    runtime_case_t runtime_case = {0};
    prepare_runtime_case(&runtime_case);
    run_oom_loop("runtime throw", op_runtime_throw, cleanup_runtime_call, &runtime_case);
    wah_exec_context_destroy(&runtime_case.ctx);
    wah_free_module(&runtime_case.module);

    runtime_case_t runtime_throw_ref_case = {0};
    prepare_runtime_throw_ref_case(&runtime_throw_ref_case);
    run_oom_loop("runtime throw_ref", op_runtime_throw, cleanup_runtime_call, &runtime_throw_ref_case);
    wah_exec_context_destroy(&runtime_throw_ref_case.ctx);
    wah_free_module(&runtime_throw_ref_case.module);

    printf("\n--- ALL OOM TESTS PASSED ---\n");
    return 0;
}
