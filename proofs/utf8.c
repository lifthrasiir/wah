// CBMC proof of the UTF-8 validator: for every input of up to MAX_LEN bytes, it reads nothing out of bounds and
// accepts exactly the concatenations of UTF-8 encodings of scalar values (binary/values.html#names).
#include "proofs.h"

#define MAX_LEN 8

// The encoding of a code point c in n bytes: the first byte carries the top bits after n ones (none for n = 1) and
// a zero, and the others carry 6 bits each after 10. It must be the shortest one and c must be a scalar value.
// Returns the length of the encoding at the start of the input, or 0 if there is none. The encoding is prefix-free,
// so at most one n can match.
static size_t spec_utf8_char(const uint8_t *buf, size_t len) {
    static const uint32_t min_cp[5] = {0, 0, 0x80, 0x800, 0x10000};
    for (size_t n = 1; n <= 4 && n <= len; n++) {
        uint8_t lead_mask = n == 1 ? 0x80 : (uint8_t)(0xFF00 >> (n + 1)), lead = (uint8_t)(0xFF00 >> n);
        if ((buf[0] & lead_mask) != (n == 1 ? 0 : lead)) continue;
        uint32_t c = buf[0] & (uint8_t)~lead_mask;
        bool ok = true;
        for (size_t i = 1; i < n; i++) {
            if ((buf[i] & 0xC0) != 0x80) ok = false;
            c = c << 6 | (buf[i] & 0x3F);
        }
        if (ok && c >= min_cp[n] && c <= 0x10FFFF && !(c >= 0xD800 && c <= 0xDFFF)) return n;
    }
    return 0;
}

static bool spec_utf8(const uint8_t *buf, size_t len) {
    for (size_t i = 0; i < len;) {
        size_t n = spec_utf8_char(buf + i, len - i);
        if (n == 0) return false;
        i += n;
    }
    return true;
}

void harness_valid(void) {
    uint8_t buf[MAX_LEN];
    for (int i = 0; i < MAX_LEN; i++) buf[i] = nondet_u8();
    size_t len = nondet_size();
    __CPROVER_assume(len <= MAX_LEN);
    bool got = wah_is_valid_utf8((const char *)buf, len);
    __CPROVER_assert(got == spec_utf8(buf, len), "accepts exactly the UTF-8 encodings of scalar values");
}
