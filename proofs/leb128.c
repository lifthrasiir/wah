// CBMC proofs of the LEB128 decoders: for every input of up to MAX_LEN bytes, they read nothing out of bounds, and
// accept exactly the encodings of the spec's grammar (binary/values.html#integers) with the same value and length.
#include "proofs.h"

#define MAX_LEN 11 // One more than the longest encoding

// The grammar of uN and sN: a byte below 128 ends the encoding and must fit in the N bits left, otherwise it carries
// 7 bits and N-7 bits are left. Returns the length, or 0 if the input doesn't start with an encoding.
static size_t spec_leb128(const uint8_t *buf, size_t len, int n, bool is_signed, uint64_t *value) {
    uint64_t v = 0;
    for (size_t i = 0, shift = 0; i < len; i++, shift += 7, n -= 7) {
        uint8_t b = buf[i];
        if (b >= 128) {
            if (n <= 7) return 0;
            v |= (uint64_t)(b - 128) << shift;
            continue;
        }
        if (!is_signed) {
            if (n < 7 && b >= (1u << n)) return 0;
            v |= (uint64_t)b << shift;
        } else if (b < 64) {
            if (n < 7 && b >= (1u << (n - 1))) return 0;
            v |= (uint64_t)b << shift;
        } else {
            if (n < 7 && b < 128 - (1u << (n - 1))) return 0;
            v |= (uint64_t)(b - 128) << shift; // Negative, so it sign-extends over the higher bits
        }
        *value = v;
        return i + 1;
    }
    return 0;
}

static void check(wah_error_t err, const uint8_t *buf, const uint8_t *p, size_t len, uint64_t got, int n, bool is_signed) {
    __CPROVER_assert(p >= buf && p <= buf + len, "the cursor stays within the input");
    uint64_t expected = 0;
    size_t used = spec_leb128(buf, len, n, is_signed, &expected);
    __CPROVER_assert((err == WAH_OK) == (used > 0), "accepts exactly the encodings of the grammar");
    if (err == WAH_OK) {
        __CPROVER_assert((size_t)(p - buf) == used, "consumes the whole encoding");
        __CPROVER_assert(got == expected, "decodes the value of the grammar");
    }
}

#define INPUT() \
    uint8_t buf[MAX_LEN]; \
    for (int i = 0; i < MAX_LEN; i++) buf[i] = nondet_u8(); \
    size_t len = nondet_size(); \
    __CPROVER_assume(len <= MAX_LEN); \
    const uint8_t *p = buf

void harness_uleb128(void) {
    INPUT();
    uint32_t v = 0;
    wah_error_t err = wah_decode_uleb128(&p, buf + len, &v);
    check(err, buf, p, len, v, 32, false);
}

void harness_uleb128_64(void) {
    INPUT();
    uint64_t v = 0;
    wah_error_t err = wah_decode_uleb128_64(&p, buf + len, &v);
    check(err, buf, p, len, v, 64, false);
}

void harness_sleb128_32(void) {
    INPUT();
    int32_t v = 0;
    wah_error_t err = wah_decode_sleb128_32(&p, buf + len, &v);
    check(err, buf, p, len, (uint64_t)(int64_t)v, 32, true);
}

void harness_sleb128_64(void) {
    INPUT();
    int64_t v = 0;
    wah_error_t err = wah_decode_sleb128_64(&p, buf + len, &v);
    check(err, buf, p, len, (uint64_t)v, 64, true);
}
