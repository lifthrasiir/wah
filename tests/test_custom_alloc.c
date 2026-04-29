#include <stdio.h>
#include <stdint.h>
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

static uintptr_t perturb_ptr(const void *ptr) {
    uintptr_t x = (uintptr_t)ptr;
#if UINTPTR_MAX > 0xffffffffu
    x ^= x >> 33;
    x *= (uintptr_t)0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= (uintptr_t)0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;
#else
    x ^= x >> 16;
    x *= (uintptr_t)0x7feb352dU;
    x ^= x >> 15;
    x *= (uintptr_t)0x846ca68bU;
    x ^= x >> 16;
#endif
    return x ? x : 1;
}

static void track_alloc_ptr(tracking_alloc_t *t, void *ptr) {
    uintptr_t h = perturb_ptr(ptr);
    t->allocs++;
    t->outstanding++;
    t->sum += h;
    t->xorv ^= h;
}

static void track_free_ptr(tracking_alloc_t *t, void *ptr) {
    uintptr_t h = perturb_ptr(ptr);
    t->frees++;
    t->outstanding--;
    t->sum -= h;
    t->xorv ^= h;
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
    if (size == 0) {
        free(ptr);
        track_free_ptr(t, ptr);
        return NULL;
    }
    void *new_ptr = realloc(ptr, size);
    if (new_ptr) {
        track_free_ptr(t, ptr);
        track_alloc_ptr(t, new_ptr);
    }
    return new_ptr;
}

static void tracking_free(void *ptr, void *userdata) {
    tracking_alloc_t *t = (tracking_alloc_t *)userdata;
    if (ptr) {
        free(ptr);
        track_free_ptr(t, ptr);
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
    assert_ok(wah_exec_context_create_ex(&ctx, &module, &exec_opts));
    if (context_counts.allocs == 0) return 1;

    wah_value_t arg = { .i32 = 42 };
    wah_value_t result = {0};
    assert_ok(wah_call(&ctx, 0, &arg, 1, &result));
    assert_eq_i32(result.i32, 42);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
    if (!tracking_ok("context", &context_counts) || !tracking_ok("module", &module_counts)) {
        return 1;
    }

    tracking_alloc_t builder_counts = {0};
    wah_alloc_t builder_alloc = { tracking_malloc, tracking_realloc, tracking_free, &builder_counts };
    wah_module_t host_module;
    assert_ok(wah_new_module_ex(&host_module, &builder_alloc));
    assert_ok(wah_module_export_func(&host_module, "id", "(i32) -> i32", host_id, NULL, NULL));
    if (builder_counts.allocs == 0) return 1;
    wah_free_module(&host_module);
    if (!tracking_ok("builder", &builder_counts)) {
        return 1;
    }

    printf("custom allocator API tests passed\n");
    return 0;
}
