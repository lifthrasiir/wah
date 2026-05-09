#include <stdio.h>
#include <stdlib.h>
#include "../wah.h"
#include "common.h"

typedef struct {
    size_t allocs;
    size_t frees;
    size_t outstanding;
    uintptr_t sum;
    uintptr_t xorv;
} tracking_alloc_t;

static void track_alloc_ptr(tracking_alloc_t *t, void *ptr) {
    uintptr_t h = wah_test_perturb_ptr(ptr);
    t->allocs++;
    t->outstanding++;
    t->sum += h;
    t->xorv ^= h;
}

static void track_free_hash(tracking_alloc_t *t, uintptr_t h) {
    t->frees++;
    t->outstanding--;
    t->sum -= h;
    t->xorv ^= h;
}

static void track_free_ptr(tracking_alloc_t *t, void *ptr) {
    track_free_hash(t, wah_test_perturb_ptr(ptr));
}

static void *tracking_malloc(size_t size, void *userdata) {
    tracking_alloc_t *t = (tracking_alloc_t *)userdata;
    void *p = malloc(size ? size : 1);
    if (p) track_alloc_ptr(t, p);
    return p;
}

static void *tracking_realloc(void *ptr, size_t size, void *userdata) {
    tracking_alloc_t *t = (tracking_alloc_t *)userdata;
    if (!ptr) {
        return tracking_malloc(size, userdata);
    }
    uintptr_t old_hash = wah_test_perturb_ptr(ptr);
    if (size == 0) {
        track_free_hash(t, old_hash);
        free(ptr);
        return NULL;
    }
    void *new_ptr = realloc(ptr, size);
    if (new_ptr) {
        track_free_hash(t, old_hash);
        track_alloc_ptr(t, new_ptr);
    }
    return new_ptr;
}

static void tracking_free(void *ptr, void *userdata) {
    tracking_alloc_t *t = (tracking_alloc_t *)userdata;
    if (ptr) {
        track_free_ptr(t, ptr);
        free(ptr);
    }
}

static int tracking_ok(const char *name, const tracking_alloc_t *t) {
    if (t->outstanding == 0 && t->sum == 0 && t->xorv == 0) return 1;
    fprintf(stderr, "%s allocator mismatch: allocs=%zu frees=%zu outstanding=%zu sum=%zx xor=%zx\n",
            name, t->allocs, t->frees, t->outstanding, (size_t)t->sum, (size_t)t->xorv);
    return 0;
}

static void host_id(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    wah_result_i32(ctx, 0, wah_param_i32(ctx, 0));
}

int main(void) {
    tracking_alloc_t module_counts = {0};
    tracking_alloc_t context_counts = {0};
    wah_alloc_t module_alloc = { tracking_malloc, tracking_realloc, tracking_free, &module_counts };
    wah_alloc_t context_alloc = { tracking_malloc, tracking_realloc, tracking_free, &context_counts };

    wah_module_t module;
    wah_parse_options_t parse_opts = { .alloc = &module_alloc };
    const char *spec = "wasm \
        types {[ fn [i32] [i32] ]} \
        funcs {[ 0 ]} \
        exports {[ {'id'} fn# 0 ]} \
        code {[ {[] local.get 0 end } ]}";
    assert_ok(wah_parse_module_from_spec_ex(&module, &parse_opts, spec));
    if (module_counts.allocs == 0) return 1;

    wah_exec_context_t ctx;
    wah_exec_options_t exec_opts = { .alloc = &context_alloc };
    assert_ok(wah_new_exec_context(&ctx, &module, &exec_opts));
    if (context_counts.allocs == 0) return 1;

    wah_value_t arg = { .i32 = 42 };
    wah_value_t result = {0};
    assert_ok(wah_call(&ctx, 0, &arg, 1, &result));
    assert_eq_i32(result.i32, 42);

    wah_free_exec_context(&ctx);
    wah_free_module(&module);
    if (!tracking_ok("context", &context_counts) || !tracking_ok("module", &module_counts)) {
        return 1;
    }

    tracking_alloc_t builder_counts = {0};
    wah_alloc_t builder_alloc = { tracking_malloc, tracking_realloc, tracking_free, &builder_counts };
    wah_module_t host_module;
    assert_ok(wah_new_module(&host_module, &builder_alloc));
    assert_ok(wah_export_func(&host_module, "id", "(i32) -> i32", host_id, NULL, NULL));
    if (builder_counts.allocs == 0) return 1;
    wah_free_module(&host_module);
    if (!tracking_ok("builder", &builder_counts)) {
        return 1;
    }

    // Regression: partial allocator (missing function pointer) caused NULL call.
    printf("Testing partial allocator validation...\n");
    {
        wah_alloc_t bad = { tracking_malloc, NULL, tracking_free, NULL };
        wah_module_t m = {0};
        assert_err(wah_new_module(&m, &bad), WAH_ERROR_MISUSE);
    }

    // Regression: wah_free_exec_context did not free dropped_elem_segments /
    // dropped_data_segments on owned linked module contexts, leaking memory.
    printf("Testing linked module dropped segment cleanup...\n");
    {
        tracking_alloc_t mc = {0}, cc = {0};
        wah_alloc_t ma = { tracking_malloc, tracking_realloc, tracking_free, &mc };
        wah_alloc_t ca = { tracking_malloc, tracking_realloc, tracking_free, &cc };

        wah_module_t linked_mod = {0};
        wah_parse_options_t po = { .alloc = &ma };
        assert_ok(wah_parse_module_from_spec_ex(&linked_mod, &po, "wasm \
            types {[fn [] []]} \
            funcs {[0, 0]} \
            tables {[funcref limits.i32/2 10 10]} \
            memories {[limits.i32/1 1]} \
            exports {[{'drop_both'} fn# 0]} \
            elements {[elem.passive elem.funcref [1]]} \
            datacount {1} \
            code {[{[] elem.drop 0 data.drop 0 end}, {[] end}]} \
            data {[data.passive {%'AB'}]}"));

        wah_module_t primary_mod = {0};
        assert_ok(wah_parse_module_from_spec_ex(&primary_mod, &po, "wasm \
            types {[fn [] []]} \
            imports {[{'L'} {'drop_both'} fn# 0]} \
            funcs {[0]} \
            exports {[{'run'} fn# 1]} \
            code {[{[] call 0 end}]}"));

        wah_exec_context_t ectx = {0};
        wah_exec_options_t eo = { .alloc = &ca };
        assert_ok(wah_new_exec_context(&ectx, &primary_mod, &eo));
        assert_ok(wah_link_module(&ectx, "L", &linked_mod));
        assert_ok(wah_instantiate(&ectx));

        wah_value_t result;
        assert_ok(wah_call_by_name(&ectx, "run", NULL, 0, &result));

        wah_free_exec_context(&ectx);
        wah_free_module(&primary_mod);
        wah_free_module(&linked_mod);
        if (!tracking_ok("linked-drop-ctx", &cc) || !tracking_ok("linked-drop-mod", &mc)) {
            return 1;
        }
    }

    printf("custom allocator API tests passed\n");
    return 0;
}
