// Parsed modules should not keep memory far beyond their sizes, as in production (without WAH_CHECK_LOWERED).
#undef WAH_CHECK_LOWERED
#define WAH_IMPLEMENTATION
#include "../wah.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>

static size_t cur_bytes, peak_bytes;
static void *count_malloc(size_t n, void *u) {
    (void)u;
    size_t *p = malloc(n + 16);
    if (!p) return NULL;
    p[0] = n, cur_bytes += n;
    if (cur_bytes > peak_bytes) peak_bytes = cur_bytes;
    return (char *)p + 16;
}
static void *count_realloc(void *q, size_t n, void *u) {
    (void)u;
    size_t *p = q ? (size_t *)((char *)q - 16) : NULL, old = p ? p[0] : 0;
    p = realloc(p, n + 16);
    if (!p) return NULL;
    p[0] = n, cur_bytes += n - old;
    if (cur_bytes > peak_bytes) peak_bytes = cur_bytes;
    return (char *)p + 16;
}
static void count_free(void *q, void *u) {
    (void)u;
    if (!q) return;
    size_t *p = (size_t *)((char *)q - 16);
    cur_bytes -= p[0];
    free(p);
}

static uint8_t *buf;
static size_t len, cap;
static void b(uint8_t x) {
    if (len == cap) buf = realloc(buf, cap = cap ? cap * 2 : 1024);
    buf[len++] = x;
}
static void leb(uint32_t v) { do { uint8_t x = v & 0x7f; v >>= 7; b(v ? x | 0x80 : x); } while (v); }
static size_t mark(void) { size_t m = len; for (int i = 0; i < 5; i++) b(0); return m; } // Fixed 5-byte LEB128
static void patch(size_t m) {
    uint32_t v = (uint32_t)(len - m - 5);
    for (int i = 0; i < 5; i++, v >>= 7) buf[m + i] = (uint8_t)((v & 0x7f) | (i < 4 ? 0x80 : 0));
}

enum { K = 1000, N = 2000 };
enum { EXACT_TYPES, REF_NESS, LABELS };

// Types t0: (c*K) -> (a*K), t1: (a*K) -> (c*K) and t2: () -> (c*K), whose third function alternates calls
// so that all K slots change at every call, or only has repeated blocks of type t2.
static void build(int kind) {
    len = 0;
    static const uint8_t header[] = { 0, 0x61, 0x73, 0x6d, 1, 0, 0, 0 };
    for (size_t i = 0; i < sizeof(header); i++) b(header[i]);
    // eqref vs anyref only differ by exact types, i32 vs externref by reference-ness
    uint8_t a = kind == REF_NESS ? 0x7f : 0x6d, c = kind == REF_NESS ? 0x6f : 0x6e;
    b(1); size_t s = mark(); leb(3);
    b(0x60); leb(K); for (int i = 0; i < K; i++) b(c); leb(K); for (int i = 0; i < K; i++) b(a);
    b(0x60); leb(K); for (int i = 0; i < K; i++) b(a); leb(K); for (int i = 0; i < K; i++) b(c);
    b(0x60); leb(0); leb(K); for (int i = 0; i < K; i++) b(c);
    patch(s);
    b(3); s = mark(); leb(3); leb(0); leb(1); leb(2); patch(s);
    b(10); s = mark(); leb(3);
    for (int f = 0; f < 3; f++) {
        size_t bs = mark();
        leb(0);
        uint8_t t = f == 0 ? a : c;
        if (f == 2 && kind == LABELS) { // unreachable (block (type 2) unreachable end unreachable)*
            b(0x00);
            for (int i = 0; i < N; i++) { b(0x02); b(0x02); b(0x00); b(0x0b); b(0x00); }
        } else {
            for (int i = 0; i < K; i++) { if (t == 0x7f) { b(0x41); b(0); } else { b(0xd0); b(t); } }
            if (f == 2) for (int i = 0; i < N; i++) { b(0x10); b(0); b(0x10); b(1); }
        }
        b(0x0b);
        patch(bs);
    }
    patch(s);
}

static void check(int kind, const char *name) {
    printf("Testing memory kept by parsing %s...\n", name);
    build(kind);
    wah_alloc_t alloc = { count_malloc, count_realloc, count_free, NULL, 0 };
    wah_parse_options_t opts = { .features = WAH_FEATURE_ALL, .alloc = &alloc };
    wah_module_t mod;
    wah_error_t err = wah_parse_module(&mod, buf, len, &opts);
    size_t kept = cur_bytes;
    printf("  %zu bytes: %s, %zu bytes kept\n", len, wah_strerror(err), kept);
    assert_true(err == WAH_OK || err == WAH_ERROR_TOO_LARGE);
    assert_true(kept <= 64 * len + 256 * 1024);
    if (err == WAH_OK) wah_free_module(&mod);
    assert_eq_u64(cur_bytes, 0);
}

// Sections claiming as many entries as they could fit, but only with zeros
static void check_section_counts(void) {
    static const uint8_t sections[] = { 2 /* import */, 4 /* table */, 5 /* memory */, 6 /* global */, 7 /* export */,
                                        9 /* element */, 10 /* code */, 11 /* data */, 13 /* tag */ };
    enum { SIZE = 1 << 20 };
    for (size_t k = 0; k < sizeof(sections); k++) {
        printf("Testing memory allocated for section %u with too many entries...\n", sections[k]);
        size_t peak = 0;
        for (uint32_t per_entry = 1; per_entry <= 8; per_entry++) {
            len = 0;
            static const uint8_t header[] = { 0, 0x61, 0x73, 0x6d, 1, 0, 0, 0 };
            for (size_t i = 0; i < sizeof(header); i++) b(header[i]);
            if (sections[k] == 10) { // Functions should be declared as many
                b(1); size_t s = mark(); leb(1); b(0x60); leb(0); leb(0); patch(s);
                b(3); s = mark(); leb(SIZE / per_entry); for (uint32_t i = 0; i < SIZE / per_entry; i++) b(0); patch(s);
            }
            b(sections[k]);
            size_t s = mark();
            leb(SIZE / per_entry);
            for (int i = 0; i < SIZE; i++) b(0);
            patch(s);
            wah_alloc_t alloc = { count_malloc, count_realloc, count_free, NULL, 0 };
            wah_parse_options_t opts = { .features = WAH_FEATURE_ALL, .alloc = &alloc };
            wah_module_t mod;
            peak_bytes = 0;
            wah_error_t err = wah_parse_module(&mod, buf, len, &opts);
            if (peak_bytes > peak) peak = peak_bytes;
            if (err == WAH_OK) wah_free_module(&mod); // Such as empty memories
            assert_eq_u64(cur_bytes, 0);
        }
        printf("  %zu bytes at peak\n", peak);
        assert_true(peak <= 64 * len + 256 * 1024);
    }
}

int main(void) {
    check(EXACT_TYPES, "ref maps of slots changing only exact types");
    check(REF_NESS, "ref maps of slots changing reference-ness");
    check(LABELS, "labels with reference types");
    check_section_counts();
    free(buf);
    printf("All parse amplification tests passed!\n");
    return 0;
}
