// CBMC proofs of the decoders of instruction and type immediates: for every input of up to MAX_LEN bytes, they read
// nothing out of bounds, and accept exactly the encodings of the spec's grammar with the same result and length.
// The references use the LEB128 decoders, which are proven to follow the grammar of uN and sN (leb128.c).
#include "proofs.h"

uint64_t nondet_u64(void);

#define MAX_LEN 21 // One more than the longest memarg

#define INPUT() \
    uint8_t buf[MAX_LEN]; \
    for (int i = 0; i < MAX_LEN; i++) buf[i] = nondet_u8(); \
    size_t len = nondet_size(); \
    __CPROVER_assume(len <= MAX_LEN); \
    const uint8_t *p = buf, *end = buf + len

#define CHECK_CURSOR() __CPROVER_assert(p >= buf && p <= end, "the cursor stays within the input")

static bool has_feature(const wah_module_t *m, int shift) { return (m->enabled_features >> shift) & 1; }
static bool requires_feature(const wah_module_t *m, int shift) { return (m->required_features >> shift) & 1; }

// Opcodes are a byte, or a prefix byte and a u32. WAH remaps them to disjoint ranges below WAH_FE, so the decoded
// opcode tells the encoding. Unassigned opcodes may be accepted here, which validation rejects later.
void harness_opcode(void) {
    INPUT();
    uint16_t op = 0;
    wah_error_t err = wah_decode_opcode(&p, end, &op);
    CHECK_CURSOR();

    size_t used = 0;
    uint32_t expected = 0;
    if (len > 0 && buf[0] < WAH_FB) {
        used = 1, expected = buf[0];
    } else if (len > 0 && buf[0] >= 0xFB && buf[0] <= 0xFD) {
        static const uint32_t base[] = {WAH_FB, WAH_FC, WAH_FD, WAH_FE};
        const uint8_t *q = buf + 1;
        uint32_t x;
        int i = buf[0] - 0xFB;
        if (wah_decode_uleb128(&q, end, &x) == WAH_OK && x < base[i + 1] - base[i]) used = q - buf, expected = base[i] + x;
    }
    __CPROVER_assert((err == WAH_OK) == (used > 0), "accepts exactly opcodes in the remapped ranges");
    if (err == WAH_OK) __CPROVER_assert((size_t)(p - buf) == used && op == expected, "decodes the remapped opcode");
}

// memarg ::= n:u32 o:u64 (n < 64) | n:u32 x:memidx o:u64 (64 <= n < 128), the latter for multiple memories.
void harness_memarg(void) {
    INPUT();
    wah_module_t m = {0};
    m.enabled_features = nondet_u64();
    wah_memarg_t memarg = {0};
    wah_error_t err = wah_decode_memarg(&p, end, &m, &memarg);
    CHECK_CURSOR();

    const uint8_t *q = buf;
    uint32_t n, x = 0;
    uint64_t o;
    bool ok = wah_decode_uleb128(&q, end, &n) == WAH_OK && n < 128;
    bool multi = ok && n >= 64;
    if (multi) ok = has_feature(&m, WAH_FEATURE_SHIFT_MULTI_MEMORY) && wah_decode_uleb128(&q, end, &x) == WAH_OK;
    ok = ok && wah_decode_uleb128_64(&q, end, &o) == WAH_OK;
    __CPROVER_assert((err == WAH_OK) == ok, "accepts exactly the encodings of memarg");
    if (multi) __CPROVER_assert(requires_feature(&m, WAH_FEATURE_SHIFT_MULTI_MEMORY), "requires multiple memories");
    if (err == WAH_OK) {
        __CPROVER_assert(p == q, "consumes the whole memarg");
        __CPROVER_assert(memarg.align == n % 64 && memarg.memidx == x && memarg.offset == o, "decodes the memarg");
    }
}

static bool spec_abs_heap_type(uint8_t b, wah_type_t *t) {
    switch (b) {
        case 0x74: *t = WAH_TYPE_NOEXN; return true;
        case 0x73: *t = WAH_TYPE_NOFUNC; return true;
        case 0x72: *t = WAH_TYPE_NOEXTERN; return true;
        case 0x71: *t = WAH_TYPE_NONE; return true;
        case 0x70: *t = WAH_TYPE_FUNC; return true;
        case 0x6F: *t = WAH_TYPE_EXTERN; return true;
        case 0x6E: *t = WAH_TYPE_ANY; return true;
        case 0x6D: *t = WAH_TYPE_EQ; return true;
        case 0x6C: *t = WAH_TYPE_I31; return true;
        case 0x6B: *t = WAH_TYPE_STRUCT; return true;
        case 0x6A: *t = WAH_TYPE_ARRAY; return true;
        case 0x69: *t = WAH_TYPE_EXN; return true;
        default: return false;
    }
}

// heaptype ::= absheaptype | x:s33 (x >= 0). Indices beyond WAH_MAX_TYPE_INDEX < 2^31 are rejected, and s33 and s32
// have the same encodings of 0..2^31-1, so s32 is enough. Returns the length, or 0 if there is no encoding.
static size_t spec_heap_type(const uint8_t *buf, const uint8_t *end, wah_type_t *t) {
    if (buf == end) return 0;
    if (spec_abs_heap_type(buf[0], t)) return 1;
    const uint8_t *q = buf;
    int32_t x;
    if (wah_decode_sleb128_32(&q, end, &x) != WAH_OK || x < 0 || (uint32_t)x > WAH_MAX_TYPE_INDEX) return 0;
    *t = WAH_TYPE_FROM_IDX(x, false);
    return q - buf;
}

// valtype ::= numtype | vectype | reftype, reftype ::= 0x64 ht | 0x63 ht | absheaptype (short for 0x63 ht),
// storagetype ::= valtype | packedtype. Long forms of reftypes need typed function references.
static size_t spec_type(const uint8_t *buf, const uint8_t *end, bool storage, wah_type_t *t, bool *typed) {
    if (buf == end) return 0;
    size_t n;
    switch (buf[0]) {
        case 0x7F: *t = WAH_TYPE_I32; return 1;
        case 0x7E: *t = WAH_TYPE_I64; return 1;
        case 0x7D: *t = WAH_TYPE_F32; return 1;
        case 0x7C: *t = WAH_TYPE_F64; return 1;
        case 0x7B: *t = WAH_TYPE_V128; return 1;
        case 0x78: *t = WAH_TYPE_PACKED_I8; return storage;
        case 0x77: *t = WAH_TYPE_PACKED_I16; return storage;
        case 0x64: *typed = true; n = spec_heap_type(buf + 1, end, t); return n ? n + 1 : 0;
        case 0x63: *typed = true; n = spec_heap_type(buf + 1, end, t); *t = WAH_TYPE_AS_NULLABLE(*t); return n ? n + 1 : 0;
        default: n = spec_abs_heap_type(buf[0], t); *t = WAH_TYPE_AS_NULLABLE(*t); return n;
    }
}

static void check_type(bool storage) {
    INPUT();
    wah_module_t m = {0};
    m.enabled_features = nondet_u64();
    wah_type_t got = 0;
    wah_error_t err = storage ? wah_decode_storage_type(&p, end, &m, &got) : wah_decode_val_type(&p, end, &m, &got);
    CHECK_CURSOR();

    wah_type_t expected = 0;
    bool typed = false;
    size_t used = spec_type(buf, end, storage, &expected, &typed);
    bool ok = used > 0 && (!typed || has_feature(&m, WAH_FEATURE_SHIFT_TYPED_FUNCREF));
    __CPROVER_assert((err == WAH_OK) == ok, "accepts exactly the encodings of the type");
    if (typed) __CPROVER_assert(requires_feature(&m, WAH_FEATURE_SHIFT_TYPED_FUNCREF), "requires typed funcrefs");
    if (err == WAH_OK) __CPROVER_assert((size_t)(p - buf) == used && got == expected, "decodes the type");
}

void harness_val_type(void) { check_type(false); }
void harness_storage_type(void) { check_type(true); }
