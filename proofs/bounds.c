// CBMC proofs of range checks: for all operands, they agree with the same checks done in unbounded (128-bit)
// arithmetic, so that no wraparound can make an out-of-bounds range pass.
#include "proofs.h"

typedef unsigned __int128 u128;

uint32_t nondet_u32(void);
uint64_t nondet_u64(void);

void harness_effective_addr(void) {
    uint64_t addr = nondet_u64(), offset = nondet_u64(), size = nondet_u64(), memory_size = nondet_u64(), ea = 0;
    wah_error_t err = wah_check_effective_addr(addr, offset, size, memory_size, &ea);
    bool in_bounds = (u128)addr + offset + size <= memory_size;
    __CPROVER_assert(err == (in_bounds ? WAH_OK : WAH_ERROR_MEMORY_OUT_OF_BOUNDS), "accepts exactly in-bounds accesses");
    if (err == WAH_OK) __CPROVER_assert(ea == addr + offset, "returns the effective address");
}

void harness_range_in_bounds(void) {
    uint64_t offset = nondet_u64(), size = nondet_u64(), limit = nondet_u64();
    __CPROVER_assert(wah_u64_range_in_bounds(offset, size, limit) == ((u128)offset + size <= limit),
                     "accepts exactly in-bounds ranges");
}

// The count prefix is a LEB128, proven separately (leb128.c), so it's enough to cover lengths up to 5 bytes and
// a few remaining bytes.
#define MAX_LEN 8

void harness_count(void) {
    uint8_t buf[MAX_LEN];
    for (int i = 0; i < MAX_LEN; i++) buf[i] = nondet_u8();
    size_t len = nondet_size();
    __CPROVER_assume(len <= MAX_LEN);
    uint32_t min_bytes_per_item = nondet_u32(), count = 0;
    const uint8_t *p = buf, *end = buf + len;
    wah_error_t err = wah_decode_and_validate_count(&p, end, &count, min_bytes_per_item);
    __CPROVER_assert(p >= buf && p <= end, "the cursor stays within the input");

    const uint8_t *q = buf;
    uint32_t decoded = 0;
    if (wah_decode_uleb128(&q, end, &decoded) != WAH_OK) {
        __CPROVER_assert(err != WAH_OK, "rejects a malformed count");
        return;
    }
    bool fits = (uint64_t)decoded * min_bytes_per_item <= (uint64_t)(end - q);
    __CPROVER_assert((err == WAH_OK) == fits, "accepts exactly the counts whose items fit in the rest");
    __CPROVER_assert(err == WAH_OK || err == WAH_ERROR_TOO_LARGE || err == WAH_ERROR_MALFORMED, "reports the error");
    if (err == WAH_OK) __CPROVER_assert(count == decoded && p == q, "returns the count after its encoding");
}
