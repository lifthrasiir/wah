// CBMC proofs of tagged pointers: i31 references follow the spec (exec/instructions.html#ref-i31) and are told from
// aligned pointers, and the tags of GC object links are kept apart from the links.
#include "proofs.h"

bool nondet_bool(void);
int32_t nondet_i32(void);
uintptr_t nondet_uintptr(void);

// ref.i31 keeps the low 31 bits, i31.get_u zero-extends them and i31.get_s sign-extends them
void harness_i31(void) {
    int32_t v = nondet_i32();
    void *ref = wah_ref_make_i31(v);
    uint32_t low = (uint32_t)v & 0x7FFFFFFF;
    __CPROVER_assert(ref && wah_ref_is_i31(ref), "is an i31 reference");
    __CPROVER_assert((uint32_t)wah_ref_i31_get_u(ref) == low, "i31.get_u");
    __CPROVER_assert(wah_ref_i31_get_s(ref) == (low < 0x40000000 ? (int64_t)low : (int64_t)low - 0x80000000),
                     "i31.get_s");
    int32_t w = nondet_i32();
    __CPROVER_assert((wah_ref_make_i31(w) == ref) == ((((uint32_t)w ^ (uint32_t)v) & 0x7FFFFFFF) == 0),
                     "ref.eq compares the low 31 bits");

    uintptr_t p = nondet_uintptr();
    __CPROVER_assume(p % 2 == 0);
    __CPROVER_assert(!wah_ref_is_i31((void *)p), "aligned pointers aren't i31 references");
}

// Objects are aligned to at least 4 bytes, leaving two tag bits in links
void harness_gc_link(void) {
    wah_gc_object_t obj, *next = (wah_gc_object_t *)nondet_uintptr(), *next2 = (wah_gc_object_t *)nondet_uintptr();
    __CPROVER_assume(((uintptr_t)next & WAH_GC_TAG_MASK) == 0 && ((uintptr_t)next2 & WAH_GC_TAG_MASK) == 0);
    uintptr_t tags = nondet_uintptr() & WAH_GC_TAG_MASK;
    obj.next_tagged = (wah_gc_object_t *)((uintptr_t)next | tags);
    __CPROVER_assert(wah_gc_next(&obj) == next, "reads the link");
    __CPROVER_assert(wah_gc_marked(&obj) == ((tags & WAH_GC_TAG_MARK) != 0), "reads the mark");

    bool mark = nondet_bool();
    wah_gc_set_mark(&obj, mark);
    __CPROVER_assert(wah_gc_next(&obj) == next && wah_gc_marked(&obj) == mark, "marking keeps the link");
    __CPROVER_assert(((uintptr_t)obj.next_tagged & WAH_GC_TAG_AUX) == (tags & WAH_GC_TAG_AUX), "marking keeps aux");

    wah_gc_set_next(&obj, next2);
    __CPROVER_assert(wah_gc_next(&obj) == next2 && wah_gc_marked(&obj) == mark, "linking keeps the mark");
    __CPROVER_assert(((uintptr_t)obj.next_tagged & WAH_GC_TAG_AUX) == (tags & WAH_GC_TAG_AUX), "linking keeps aux");
}
