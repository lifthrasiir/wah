// CBMC proofs of the allocation wrappers over custom allocators of any alignment: blocks are 16-byte aligned and
// writable for the whole size, reallocation keeps the contents (or the block on failure), the allocator always gets
// back its own blocks, and nothing leaks. Sizes are small or overflowing, which covers every branch.
#include "proofs.h"

bool nondet_bool(void);
uint8_t nondet_u8(void);

#define MAX_SIZE 24
#define MAX_REALLOC_SIZE 8 // Reallocations copy and compare contents, which are expensive

// CBMC's memmove copies through a VLA of the length, which the solver can't handle for unbounded lengths even when
// they are unreachable, so it's replaced with a loop. The unwinding bound still proves that no copy is longer.
void *memmove(void *dst, const void *src, size_t n) {
    uint8_t *d = dst;
    const uint8_t *s = src;
    if (d < s) {
        for (size_t i = 0; i < n; i++) d[i] = s[i];
    } else {
        for (size_t i = n; i > 0; i--) d[i - 1] = s[i - 1];
    }
    return dst;
}

// An allocator holding up to one block, which may fail anywhere. CBMC aligns every object to 16 bytes, so blocks are
// placed at any offset in a larger object that keeps the alignment of the allocator.
static uint8_t *g_raw, *g_base;
static size_t g_raw_size, g_align;
static int g_calls;

static uint8_t *mock_block(size_t size) {
    if (nondet_bool() || size > SIZE_MAX - 15) return NULL;
    uint8_t *base = malloc(size + 15), offset = nondet_u8();
    __CPROVER_assume(base && offset <= 15 && (offset & (g_align - 1)) == 0);
    g_base = base;
    return base + offset;
}

static void *mock_malloc(size_t size, void *ud) {
    __CPROVER_assert(ud == &g_raw && size > 0 && !g_raw, "called as documented");
    g_calls++;
    uint8_t *raw = mock_block(size);
    if (raw) g_raw = raw, g_raw_size = size;
    return raw;
}

static void *mock_realloc(void *ptr, size_t size, void *ud) {
    __CPROVER_assert(ud == &g_raw && size > 0 && ptr && ptr == g_raw, "called as documented with its own block");
    g_calls++;
    uint8_t *old_base = g_base, *raw = mock_block(size);
    if (!raw) return NULL;
    memcpy(raw, g_raw, size < g_raw_size ? size : g_raw_size);
    free(old_base);
    g_raw = raw, g_raw_size = size;
    return raw;
}

static void mock_free(void *ptr, void *ud) {
    __CPROVER_assert(ud == &g_raw && ptr && ptr == g_raw, "called as documented with its own block");
    free(g_base);
    g_raw = NULL;
}

static wah_alloc_t mock_alloc(void) {
    uint8_t shift = nondet_u8();
    __CPROVER_assume(shift <= 5);
    g_align = (size_t)1 << shift;
    return (wah_alloc_t){mock_malloc, mock_realloc, mock_free, &g_raw, g_align};
}

// Either at most max_size bytes or too large to allocate with padding
static size_t nondet_count(size_t elemsize, size_t max_size) {
    size_t count = nondet_size(), size;
    __CPROVER_assume(__builtin_mul_overflow(count, elemsize, &size) || size <= max_size ||
                     size > SIZE_MAX - WAH_BLOCK_ALIGN);
    return count;
}

// Zero bytes are no block, and sizes that overflow (with padding if any) are rejected without calling the allocator,
// which is documented to never get a zero size.
typedef enum { NO_BLOCK, TOO_LARGE, BLOCK } size_class_t;

static size_class_t classify(const wah_alloc_t *a, size_t count, size_t elemsize, size_t *size) {
    if (__builtin_mul_overflow(count, elemsize, size)) return TOO_LARGE;
    if (*size == 0) return NO_BLOCK;
    return a->min_align < WAH_BLOCK_ALIGN && *size > SIZE_MAX - WAH_BLOCK_ALIGN ? TOO_LARGE : BLOCK;
}

static void check_block(const wah_alloc_t *a, void *p, size_t size) {
    if (a->min_align < WAH_BLOCK_ALIGN) __CPROVER_assert((uintptr_t)p % WAH_BLOCK_ALIGN == 0, "is 16-byte aligned");
    __CPROVER_assert(__CPROVER_same_object(p, g_raw) && (uint8_t *)p >= g_raw &&
                     (uint8_t *)p - g_raw <= g_raw_size && size <= g_raw_size - ((uint8_t *)p - g_raw),
                     "is within the block");
}

static void check_malloc(size_t elemsize) {
    wah_alloc_t a = mock_alloc();
    size_t count = nondet_count(elemsize, MAX_SIZE), size;
    void *p = (void *)&g_calls;
    wah_error_t err = wah_malloc(&a, count, elemsize, &p);
    switch (classify(&a, count, elemsize, &size)) {
    case NO_BLOCK:
        __CPROVER_assert(err == WAH_OK && !p && g_calls == 0, "allocates nothing for zero bytes");
        break;
    case TOO_LARGE:
        __CPROVER_assert(err == WAH_ERROR_OUT_OF_MEMORY && !p && g_calls == 0, "rejects too large sizes");
        break;
    case BLOCK:
        if (!g_raw) {
            __CPROVER_assert(err == WAH_ERROR_OUT_OF_MEMORY && !p, "fails when the allocator fails");
        } else {
            __CPROVER_assert(err == WAH_OK, "succeeds when the allocator succeeds");
            check_block(&a, p, size);
        }
        break;
    }
    wah_free(&a, p);
    __CPROVER_assert(!g_raw, "frees the block");
}

static void check_realloc(size_t elemsize) {
    wah_alloc_t a = mock_alloc();
    size_t old_size = nondet_size();
    __CPROVER_assume(old_size <= MAX_REALLOC_SIZE);
    void *p = NULL;
    if (nondet_bool() && old_size > 0) { // Otherwise from NULL
        __CPROVER_assume(wah_malloc(&a, old_size, 1, &p) == WAH_OK);
        g_calls = 0;
    }
    uint8_t old[MAX_REALLOC_SIZE];
    for (size_t i = 0; i < MAX_REALLOC_SIZE; i++) {
        old[i] = nondet_u8();
        if (i < old_size && p) ((uint8_t *)p)[i] = old[i];
    }
    void *old_p = p;

    size_t count = nondet_count(elemsize, MAX_REALLOC_SIZE), size = 0, kept = old_p ? old_size : 0;
    wah_error_t err = wah_realloc(&a, count, elemsize, &p);
    switch (classify(&a, count, elemsize, &size)) {
    case NO_BLOCK:
        __CPROVER_assert(err == WAH_OK && !p && !g_raw && g_calls == 0, "frees the block for zero bytes");
        kept = 0;
        break;
    case TOO_LARGE:
        __CPROVER_assert(err == WAH_ERROR_OUT_OF_MEMORY && p == old_p && g_calls == 0, "rejects too large sizes");
        break;
    case BLOCK:
        if (err != WAH_OK) {
            __CPROVER_assert(err == WAH_ERROR_OUT_OF_MEMORY && p == old_p && g_calls == 1, "fails when the allocator fails");
        } else {
            check_block(&a, p, size);
            if (size < kept) kept = size;
        }
        break;
    }
    for (size_t i = 0; i < MAX_REALLOC_SIZE; i++) {
        if (i < kept) __CPROVER_assert(((uint8_t *)p)[i] == old[i], "keeps the contents");
    }
    wah_free(&a, p);
    __CPROVER_assert(!g_raw, "frees the block");
}

// Element sizes are constants, or the solver would face 64-bit divisions
#define ALLOC_HARNESSES(name, elemsize) \
    void harness_malloc_##name(void) { check_malloc(elemsize); } \
    void harness_realloc_##name(void) { check_realloc(elemsize); }

ALLOC_HARNESSES(0, 0)
ALLOC_HARNESSES(1, 1)
ALLOC_HARNESSES(3, 3)
ALLOC_HARNESSES(16, 16)
ALLOC_HARNESSES(huge, SIZE_MAX / 2 + 1)
