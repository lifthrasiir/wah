// CBMC proof that each step of the struct layout builder agrees with `place` of proofs/lean/WahProofs/Layout.lean,
// which proves that the builder aligns fields, never overlaps them and pads at most 15 bytes in total. Steps agree
// for every builder state that satisfies the invariant of the Lean model (`Inv`), so they agree for any field count.
#include "proofs.h"

bool nondet_bool(void);
uint32_t nondet_u32(void);

// `St` in Lean: slot i is free at val[i] if some[i] (i < 4, `Inv.slot_high`), with unbounded naturals
typedef struct {
    bool some[4];
    uint64_t val[4], frontier;
} model_st_t;

static uint32_t model_find_class(const model_st_t *st, uint32_t k) { // `findClass`
    return k < 4 ? (st->some[k] ? k : model_find_class(st, k + 1)) : 4;
}

static uint64_t model_place(model_st_t *st, uint32_t k) { // `place`, returning the offset
    uint32_t j = model_find_class(st, k);
    uint64_t o = j < 4 ? (st->some[j] ? st->val[j] : 0) : st->frontier;
    model_st_t next;
    for (uint32_t i = 0; i < 4; ++i) {
        if (k <= i && i < j) next.some[i] = true, next.val[i] = o + ((uint64_t)1 << i);
        else if (i == j) next.some[i] = false, next.val[i] = 0;
        else next.some[i] = st->some[i], next.val[i] = st->val[i];
    }
    next.frontier = j < 4 ? st->frontier : st->frontier + 16;
    *st = next;
    return o;
}

void harness_place(void) {
    uint32_t slots[4], free_mask = nondet_u32(), frontier = nondet_u32(), k = nondet_u32(), offset;
    __CPROVER_assume(free_mask < 16 && k <= 4);
    model_st_t st;
    st.frontier = frontier;
    for (uint32_t i = 0; i < 4; ++i) {
        slots[i] = nondet_u32();
        st.some[i] = (free_mask >> i) & 1;
        st.val[i] = slots[i];
        // `Inv.slot_bound`: free slots lie below the frontier
        if (st.some[i]) __CPROVER_assume((uint64_t)slots[i] + (1u << i) <= frontier);
    }
    uint32_t old_mask = free_mask, old_frontier = frontier, old_slots[4];
    for (uint32_t i = 0; i < 4; ++i) old_slots[i] = slots[i];
    uint32_t j = model_find_class(&st, k);

    wah_error_t err = wah_layout_place(slots, &free_mask, &frontier, k, &offset);
    uint64_t model_offset = model_place(&st, k);
    if (err != WAH_OK) {
        // The model has no bound, which C only reaches with a new chunk past 4 GiB
        __CPROVER_assert(err == WAH_ERROR_TOO_LARGE && j == 4 && st.frontier > UINT32_MAX,
                         "fails only when a new chunk overflows the frontier");
        __CPROVER_assert(free_mask == old_mask && frontier == old_frontier, "failure leaves the state intact");
        for (uint32_t i = 0; i < 4; ++i) __CPROVER_assert(slots[i] == old_slots[i], "failure leaves the slots intact");
        return;
    }
    __CPROVER_assert(offset == model_offset, "same offset");
    __CPROVER_assert(frontier == st.frontier, "same frontier");
    for (uint32_t i = 0; i < 4; ++i) {
        __CPROVER_assert(((free_mask >> i) & 1) == st.some[i], "same free slots");
        __CPROVER_assert(!st.some[i] || slots[i] == st.val[i], "same slot offsets");
    }
    __CPROVER_assert(free_mask < 16, "no other free slots");
}
