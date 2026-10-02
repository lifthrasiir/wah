#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../wah.h"
#include "common.h"

typedef struct {
    size_t allocs;
    size_t frees;
    size_t outstanding;
    uintptr_t sum;
    uintptr_t xorv;
} tracking_alloc_t;

typedef struct {
    tracking_alloc_t tracking;
    int null_reallocs;
} strict_alloc_t;

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

static void *strict_malloc(size_t size, void *userdata) {
    strict_alloc_t *s = (strict_alloc_t *)userdata;
    return tracking_malloc(size, &s->tracking);
}

static void *strict_realloc(void *ptr, size_t size, void *userdata) {
    strict_alloc_t *s = (strict_alloc_t *)userdata;
    if (!ptr) {
        s->null_reallocs++;
        return NULL;
    }
    return tracking_realloc(ptr, size, &s->tracking);
}

static void strict_free(void *ptr, void *userdata) {
    strict_alloc_t *s = (strict_alloc_t *)userdata;
    tracking_free(ptr, &s->tracking);
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

// Tracks the peak of live bytes, with the size stored before each block.
typedef struct {
    size_t live;
    size_t peak;
} peak_alloc_t;

#define PEAK_HEADER 16

static void *peak_malloc(size_t size, void *userdata) {
    peak_alloc_t *t = (peak_alloc_t *)userdata;
    unsigned char *p = (unsigned char *)malloc(PEAK_HEADER + size);
    if (!p) return NULL;
    memcpy(p, &size, sizeof(size));
    t->live += size;
    if (t->live > t->peak) t->peak = t->live;
    return p + PEAK_HEADER;
}

static void peak_free(void *ptr, void *userdata) {
    peak_alloc_t *t = (peak_alloc_t *)userdata;
    if (!ptr) return;
    unsigned char *p = (unsigned char *)ptr - PEAK_HEADER;
    size_t size;
    memcpy(&size, p, sizeof(size));
    t->live -= size;
    free(p);
}

static void *peak_realloc(void *ptr, size_t size, void *userdata) {
    if (!ptr) return peak_malloc(size, userdata);
    size_t old;
    memcpy(&old, (unsigned char *)ptr - PEAK_HEADER, sizeof(old));
    void *q = peak_malloc(size, userdata);
    if (!q) return NULL;
    memcpy(q, ptr, old < size ? old : size);
    peak_free(ptr, userdata);
    return q;
}

// Arrays grown by n appends from external inputs should only take O(log n) reallocations.
static size_t growth_reallocs;
static void *growth_malloc(size_t size, void *userdata) { (void)userdata; return malloc(size); }
static void *growth_realloc(void *ptr, size_t size, void *userdata) {
    (void)userdata;
    if (ptr) growth_reallocs++;
    return realloc(ptr, size);
}
static void growth_free(void *ptr, void *userdata) { (void)userdata; free(ptr); }

static int test_logarithmic_growth(void) {
    enum { N = 1024 };
    wah_alloc_t alloc = { growth_malloc, growth_realloc, growth_free, NULL };
    wah_module_t mod;
    assert_ok(wah_new_module(&mod, &alloc));
    growth_reallocs = 0;
    for (int i = 0; i < N; i++) {
        char name[32];
        wah_type_t t;
        assert_ok(wah_define_type(&mod, &t, "fresh struct { i32 }"));
        snprintf(name, sizeof(name), "g%d", i);
        assert_ok(wah_export_global_i32(&mod, name, false, i));
        snprintf(name, sizeof(name), "m%d", i);
        assert_ok(wah_export_memory(&mod, name, 1, 1));
    }
    char spec[8 * N + 16] = "fn (i32";
    for (int i = 1; i < N; i++) strcat(spec, ", i32");
    strcat(spec, ")");
    wah_type_t t;
    assert_ok(wah_define_type(&mod, &t, spec));
    printf("  %zu reallocations for %d types, globals and memories\n", growth_reallocs, N);
    wah_free_module(&mod);
    return growth_reallocs < N / 4;
}

// Tracking initialization of non-defaultable locals should cost in proportion to the code, not to the
// number of declared locals, which can be large for a few bytes.
static size_t parse_alloc_bytes;
static void *bytes_malloc(size_t size, void *userdata) { (void)userdata; parse_alloc_bytes += size; return malloc(size); }
static void *bytes_realloc(void *ptr, size_t size, void *userdata) {
    (void)userdata;
    parse_alloc_bytes += size;
    return realloc(ptr, size);
}

static int test_non_defaultable_locals_cost(void) {
    enum { FUNCS = 64 };
    wah_alloc_t alloc = { bytes_malloc, bytes_realloc, growth_free, NULL };
    wah_parse_options_t opts = { .alloc = &alloc };
    char spec[64 * FUNCS + 256];
    strcpy(spec, "wasm types {[ struct [i32 mut], fn [] [] ]} funcs {[ 1");
    for (int i = 1; i < FUNCS; i++) strcat(spec, ", 1");
    strcat(spec, " ]} code {[ {[60000 type.ref 0] end}");
    for (int i = 1; i < FUNCS; i++) strcat(spec, ", {[60000 type.ref 0] end}");
    strcat(spec, " ]}");
    wah_module_t mod;
    parse_alloc_bytes = 0;
    assert_ok(wah_parse_module_from_spec_ex(&mod, &opts, spec));
    printf("  %zu bytes allocated for %d functions with 60000 non-defaultable locals\n", parse_alloc_bytes, FUNCS);
    wah_free_module(&mod);
    return parse_alloc_bytes < 1024 * 1024;
}

// Expressions of element segments used to be allocated one by one, taking tens of bytes for 3-byte elements.
static int test_element_exprs_allocations(void) {
    enum { N = 100000 };
    tracking_alloc_t counts = {0};
    wah_alloc_t alloc = { tracking_malloc, tracking_realloc, tracking_free, &counts };
    wah_parse_options_t opts = { .alloc = &alloc };
    static const char elem[] = "ref.func 0 end, ";
    char *spec = malloc(N * (sizeof(elem) - 1) + 256);
    if (!spec) return 0;
    char *p = spec + snprintf(spec, 256, "wasm types {[fn [] []]} funcs {[0]} elements {[ elem.passive.expr funcref [");
    for (int i = 0; i < N; i++) { memcpy(p, elem, sizeof(elem) - 1); p += sizeof(elem) - 1; }
    strcpy(p - 2, "] ]} code {[{[] end}]}");
    wah_module_t mod;
    assert_ok(wah_parse_module_from_spec_ex(&mod, &opts, spec));
    free(spec);
    size_t outstanding = counts.outstanding;
    printf("  %zu allocations for %d element expressions\n", outstanding, N);
    wah_free_module(&mod);
    return outstanding < 100 && tracking_ok("element exprs", &counts);
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
    if (!test_logarithmic_growth()) return 1;
    if (!test_non_defaultable_locals_cost()) return 1;
    if (!test_element_exprs_allocations()) return 1;

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

    printf("Testing cross-module type checks don't use the parse allocator...\n");
    {
        tracking_alloc_t mc = {0};
        wah_alloc_t ma = { tracking_malloc, tracking_realloc, tracking_free, &mc };
        wah_parse_options_t po = { .alloc = &ma };
        wah_module_t provider = {0}, user = {0};
        // Comparing types referring to other types needs a memo
        assert_ok(wah_parse_module_from_spec_ex(&provider, &po, "wasm \
            types {[ fn [] [], fn [type.ref.null 0] [i32] ]} funcs {[ 1 ]} \
            tables {[ funcref limits.i32/1 1 ]} exports {[ {'t'} table# 0 ]} \
            elements {[ elem.active.table#0 i32.const 0 end [ 0 ] ]} code {[ {[] i32.const 42 end} ]}"));
        assert_ok(wah_parse_module_from_spec_ex(&user, &po, "wasm \
            types {[ fn [] [], fn [type.ref.null 0] [i32], fn [] [i32] ]} \
            imports {[ {'P'} {'t'} table# funcref limits.i32/1 1 ]} funcs {[ 2 ]} \
            code {[ {[] ref.null 0 i32.const 0 call_indirect 1 0 end} ]}"));
        size_t allocs = mc.allocs;
        wah_exec_context_t ectx = {0};
        assert_ok(wah_new_exec_context(&ectx, &user, NULL));
        assert_ok(wah_link_module(&ectx, "P", &provider));
        assert_ok(wah_instantiate(&ectx));
        wah_value_t r;
        assert_ok(wah_call(&ectx, 0, NULL, 0, &r));
        assert_eq_i32(r.i32, 42);
        assert_eq_u64(mc.allocs, allocs);
        wah_free_exec_context(&ectx);
        wah_free_module(&user);
        wah_free_module(&provider);
        if (!tracking_ok("cross-module-type-check", &mc)) return 1;
    }

    printf("Testing memory.grow 0 doesn't reallocate the memory...\n");
    {
        tracking_alloc_t cc = {0};
        wah_alloc_t ca = { tracking_malloc, tracking_realloc, tracking_free, &cc };
        wah_module_t m = {0};
        assert_ok(wah_parse_module_from_spec(&m, "wasm types {[ fn [] [i32] ]} funcs {[ 0 ]} \
            memories {[ limits.i32/1 1 ]} code {[ {[] i32.const 0 memory.grow 0 end} ]}"));
        wah_exec_context_t ectx = {0};
        wah_exec_options_t eo = { .alloc = &ca };
        assert_ok(wah_new_exec_context(&ectx, &m, &eo));
        assert_ok(wah_instantiate(&ectx));
        size_t allocs = cc.allocs;
        wah_value_t r;
        assert_ok(wah_call(&ectx, 0, NULL, 0, &r));
        assert_eq_i32(r.i32, 1);
        assert_eq_u64(cc.allocs, allocs);
        wah_free_exec_context(&ectx);
        wah_free_module(&m);
        if (!tracking_ok("memory-grow-0", &cc)) return 1;
    }

    // Regression: partial allocator (missing function pointer) caused NULL call.
    printf("Testing partial allocator validation...\n");
    {
        wah_alloc_t bad = { tracking_malloc, NULL, tracking_free, NULL };
        wah_module_t m = {0};
        assert_err(wah_new_module(&m, &bad), WAH_ERROR_MISUSE);
    }

    // Regression: wah_realloc must honor the public allocator contract and use
    // malloc for initial allocation instead of calling realloc with NULL.
    printf("Testing realloc(NULL) avoidance...\n");
    {
        strict_alloc_t sc = {0};
        wah_alloc_t strict_alloc = { strict_malloc, strict_realloc, strict_free, &sc };
        wah_module_t m = {0};
        wah_type_t t;
        assert_ok(wah_new_module(&m, &strict_alloc));
        assert_ok(wah_define_type(&m, &t, "fn (i32) -> i32"));
        wah_free_module(&m);
        if (sc.null_reallocs != 0 || !tracking_ok("strict-realloc", &sc.tracking)) {
            return 1;
        }
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

        assert_ok(wah_call_by_name(&ectx, "run", NULL, 0, &result));

        wah_free_exec_context(&ectx);
        wah_free_module(&primary_mod);
        wah_free_module(&linked_mod);
        if (!tracking_ok("linked-drop-ctx", &cc) || !tracking_ok("linked-drop-mod", &mc)) {
            return 1;
        }
    }

    // Regression: a tag import rejected because of disabled exceptions leaked its names.
    printf("Testing rejected tag import cleanup...\n");
    {
        tracking_alloc_t mc = {0};
        wah_alloc_t ma = { tracking_malloc, tracking_realloc, tracking_free, &mc };
        wah_parse_options_t po = { .features = WAH_FEATURE_WASM_V2, .alloc = &ma };
        wah_module_t m = {0};
        assert_err(wah_parse_module_from_spec_ex(&m, &po, "wasm \
            types {[fn [] []]} \
            imports {[{'env'} {'tag'} tag# 0 0]}"), WAH_ERROR_DISABLED_FEATURE);
        wah_free_module(&m);
        if (!tracking_ok("rejected-tag", &mc)) {
            return 1;
        }
    }

    // Regression: each import took an entry of every import kind.
    printf("Testing parse memory of many imports...\n");
    {
        enum { N = 1000 };
        static uint8_t bin[16 + N * 7];
        size_t n = 0, body = 2 + N * 7;
        memcpy(bin, "\0asm\1\0\0\0", 8); n = 8;
        bin[n++] = 2; // Import section
        bin[n++] = (uint8_t)(0x80 | (body & 0x7f)); bin[n++] = (uint8_t)(body >> 7);
        bin[n++] = (uint8_t)(0x80 | (N & 0x7f)); bin[n++] = (uint8_t)(N >> 7);
        for (int i = 0; i < N; i++) {
            static const uint8_t entry[] = { 1, 'm', 1, 'g', 3 /* global */, 0x7f /* i32 */, 0 };
            memcpy(bin + n, entry, sizeof(entry));
            n += sizeof(entry);
        }
        peak_alloc_t pc = {0};
        wah_alloc_t pa = { peak_malloc, peak_realloc, peak_free, &pc };
        wah_parse_options_t po = { .alloc = &pa };
        wah_module_t m = {0};
        assert_ok(wah_parse_module(&m, bin, n, &po));
        wah_free_module(&m);
        printf("  input %zu bytes, peak %zu bytes\n", n, pc.peak);
        assert_true(pc.peak < 32 * n);
    }

    printf("custom allocator API tests passed\n");
    return 0;
}
