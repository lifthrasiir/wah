// CBMC proofs that types are compared as `typesEq` (with `canonRef` and `refEq`) and references are hashed as
// `hashRef` of proofs/lean/WahProofs/Types.lean, which proves that canonical_map maps types to the least equivalent
// type, and that subtypes are checked as `wahSub` of proofs/lean/WahProofs/Subtype.lean, which proves that it is the
// subtyping of the spec. Hashing whole rec groups (`groupHash`) is beyond CBMC even with one type, so it is only
// checked by review and by comparing canonical maps with the Lean model.
#include <stdint.h>
// The Lean model works for any mixer, so it is an uninterpreted function here
uint64_t __CPROVER_uninterpreted_mix(uint64_t h, uint64_t v);
#define WAH_REC_GROUP_HASH_MIX __CPROVER_uninterpreted_mix
#include "proofs.h"

uint32_t nondet_u32(void);
uint64_t nondet_u64(void);
int32_t nondet_i32(void);

#define N 4 // Types in the module

// `VT` in Lean: references to type x (nullable or not), and anything else as an atom
typedef struct {
    bool ref, null;
    uint32_t x; // Type index for references, (uint32_t)t for atoms
} vt_t;

static vt_t decode(wah_type_t t) {
    if (t < 0) return (vt_t){ .ref = false, .null = false, .x = (uint32_t)t };
    return (vt_t){ .ref = true, .null = t & 1, .x = (uint32_t)t >> 1 };
}

static bool vt_eq(vt_t a, vt_t b) { return a.ref == b.ref && a.x == b.x && (!a.ref || a.null == b.null); }

static bool in_group(uint32_t x, uint32_t s, uint32_t gs) { return s <= x && (uint64_t)x < (uint64_t)s + gs; }

static vt_t model_canon_ref(const uint32_t *canon, uint32_t n, uint32_t s, uint32_t gs, vt_t v) { // `canonRef`
    if (!v.ref) return v;
    if (in_group(v.x, s, gs)) return v;
    if (n <= v.x) return v;
    return (vt_t){ .ref = true, .null = v.null, .x = canon[v.x] };
}

static bool model_ref_eq(uint32_t s1, uint32_t g1, uint32_t s2, uint32_t g2, vt_t a, vt_t b) { // `refEq`
    if (a.ref && b.ref) {
        if (in_group(a.x, s1, g1)) return in_group(b.x, s2, g2) ? a.null == b.null && a.x - s1 == b.x - s2 : false;
        return in_group(b.x, s2, g2) ? false : a.null == b.null && a.x == b.x;
    }
    if (!a.ref && !b.ref) return a.x == b.x;
    return false;
}

static wah_type_def_t defs[N];
static uint32_t canon[N];
static wah_module_t module;

static uint64_t model_hash_ref(const uint64_t *gh, uint32_t s, uint32_t gs, uint64_t h, vt_t v) { // `hashRef`
    if (!v.ref) return wah_rec_group_hash_mix(wah_rec_group_hash_mix(h, 0), v.x);
    if (in_group(v.x, s, gs)) return wah_rec_group_hash_mix(wah_rec_group_hash_mix(h, 1), 2 * (uint64_t)(v.x - s) + v.null);
    uint32_t start = defs[v.x].rec_group_start;
    h = wah_rec_group_hash_mix(wah_rec_group_hash_mix(h, 2), gh[start]);
    return wah_rec_group_hash_mix(h, 2 * (uint64_t)(v.x - start) + v.null);
}

static wah_type_t nondet_type(uint32_t n) { // Any type, referring to types of the module
    wah_type_t t = nondet_i32();
    // WAH_TYPE_BOT_REF only comes from unreachable code in validation, never from types of modules
    __CPROVER_assume((t < 0 && t != WAH_TYPE_BOT_REF) || WAH_TYIDX(t) < n);
    return t;
}

void harness_ref_eq(void) {
    uint32_t n = nondet_u32(), s1 = nondet_u32(), g1 = nondet_u32(), s2 = nondet_u32(), g2 = nondet_u32();
    __CPROVER_assume(n <= N && s1 + g1 <= n && s2 + g2 <= n && s1 < s1 + g1 && s2 < s2 + g2);
    for (uint32_t i = 0; i < N; ++i) {
        canon[i] = nondet_u32();
        __CPROVER_assume(canon[i] < n);
    }
    wah_type_t ta = nondet_type(n), tb = nondet_type(n);
    wah_type_t ca = wah_canonicalize_type_ref(ta, s1, g1, canon, n), cb = wah_canonicalize_type_ref(tb, s2, g2, canon, n);
    vt_t ma = model_canon_ref(canon, n, s1, g1, decode(ta)), mb = model_canon_ref(canon, n, s2, g2, decode(tb));
    __CPROVER_assert(vt_eq(decode(ca), ma) && vt_eq(decode(cb), mb), "same canonicalized references");
    __CPROVER_assert(wah_canon_ref_eq(ca, s1, g1, cb, s2, g2) == model_ref_eq(s1, g1, s2, g2, ma, mb),
                     "same comparison");
}

void harness_hash_ref(void) {
    uint32_t n = nondet_u32(), s = nondet_u32(), gs = nondet_u32();
    uint64_t gh[N], h = nondet_u64();
    __CPROVER_assume(n <= N && s + gs <= n && s < s + gs);
    for (uint32_t i = 0; i < N; ++i) {
        defs[i].rec_group_start = nondet_u32();
        __CPROVER_assume(defs[i].rec_group_start <= i);
        gh[i] = nondet_u64();
    }
    module.type_defs = defs;
    module.type_count = n;
    wah_type_t t = nondet_type(n);
    __CPROVER_assert(wah_rec_group_hash_type(&module, h, t, s, gs, gh) == model_hash_ref(gh, s, gs, h, decode(t)),
                     "same hash");
}

// Types with at most F fields, or params and results each
#define F 2
static wah_func_type_t ftypes[N];
static wah_type_t field_types[N][F], param_types[N][F], result_types[N][F];
static bool field_mutables[N][F];

// `TD` in Lean: functions have params ++ results as immutable fields and (param_count, result_count) as counts,
// structs and arrays have (field_count, 0)
typedef struct {
    bool final, has_super, mut[2 * F];
    uint32_t kind, super, c1, c2, nf;
    vt_t ft[2 * F];
    wah_type_t raw[2 * F]; // ft as wah_type_t
} td_t;

static td_t encode(uint32_t i) {
    const wah_type_def_t *d = &defs[i];
    td_t td = { .final = d->is_final, .kind = d->kind, .has_super = d->supertype != WAH_NO_SUPERTYPE,
                .super = d->supertype };
    if (d->kind == WAH_COMP_FUNC) {
        td.c1 = ftypes[i].param_count, td.c2 = ftypes[i].result_count;
        for (uint32_t j = 0; j < td.c1; ++j) td.raw[td.nf] = ftypes[i].param_types[j], td.mut[td.nf++] = false;
        for (uint32_t j = 0; j < td.c2; ++j) td.raw[td.nf] = ftypes[i].result_types[j], td.mut[td.nf++] = false;
    } else {
        td.c1 = d->field_count, td.c2 = 0;
        for (uint32_t j = 0; j < td.c1; ++j) td.raw[td.nf] = d->field_types[j], td.mut[td.nf++] = d->field_mutables[j];
    }
    for (uint32_t j = 0; j < td.nf; ++j) td.ft[j] = decode(td.raw[j]);
    return td;
}

static uint32_t gstart(uint32_t i) { return defs[i].rec_group_start; }
static uint32_t gsize(uint32_t i) { return defs[i].rec_group_size; }

static bool model_ref_eq_c(uint32_t n, uint32_t s1, uint32_t s2, vt_t a, vt_t b) { // `refEqC`
    return model_ref_eq(s1, gsize(s1), s2, gsize(s2), model_canon_ref(canon, n, s1, gsize(s1), a),
                        model_canon_ref(canon, n, s2, gsize(s2), b));
}

static bool model_types_eq(uint32_t n, uint32_t a, uint32_t b) { // `typesEq` with `fieldsEq`
    td_t ta = encode(a), tb = encode(b);
    uint32_t s1 = gstart(a), s2 = gstart(b);
    if (ta.kind != tb.kind || ta.final != tb.final) return false;
    if (ta.has_super != tb.has_super) return false;
    if (ta.has_super && !model_ref_eq_c(n, s1, s2, (vt_t){ .ref = true, .x = ta.super },
                                        (vt_t){ .ref = true, .x = tb.super })) return false;
    if (ta.c1 != tb.c1 || ta.c2 != tb.c2 || ta.nf != tb.nf) return false;
    for (uint32_t j = 0; j < ta.nf; ++j) {
        if (!model_ref_eq_c(n, s1, s2, ta.ft[j], tb.ft[j]) || ta.mut[j] != tb.mut[j]) return false;
    }
    return true;
}

// A module of n types in rec groups, referring to earlier groups or their own (`WF` in Lean)
static uint32_t setup_module(void) {
    uint32_t n = nondet_u32();
    __CPROVER_assume(n > 0 && n <= N);
    for (uint32_t i = 0; i < N; ++i) {
        wah_type_def_t *d = &defs[i];
        d->rec_group_start = nondet_u32(), d->rec_group_size = nondet_u32();
        __CPROVER_assume(d->rec_group_start <= i && i < d->rec_group_start + d->rec_group_size &&
                         d->rec_group_start + d->rec_group_size <= n);
        if (i > 0 && d->rec_group_start < i)
            __CPROVER_assume(d->rec_group_start == defs[i - 1].rec_group_start &&
                             d->rec_group_size == defs[i - 1].rec_group_size);
    }
    for (uint32_t i = 0; i < N; ++i) {
        wah_type_def_t *d = &defs[i];
        uint32_t end = d->rec_group_start + d->rec_group_size;
        uint8_t kind = nondet_u8();
        __CPROVER_assume(kind == WAH_COMP_FUNC || kind == WAH_COMP_STRUCT || kind == WAH_COMP_ARRAY);
        d->kind = (wah_comp_type_kind_t)kind;
        d->is_final = nondet_u8() & 1;
        d->supertype = nondet_u32();
        __CPROVER_assume(d->supertype == WAH_NO_SUPERTYPE || d->supertype < end);
        d->field_count = nondet_u32(), ftypes[i].param_count = nondet_u32(), ftypes[i].result_count = nondet_u32();
        __CPROVER_assume(d->field_count <= F && ftypes[i].param_count <= F && ftypes[i].result_count <= F);
        for (uint32_t j = 0; j < F; ++j) {
            field_types[i][j] = nondet_type(end), param_types[i][j] = nondet_type(end);
            result_types[i][j] = nondet_type(end), field_mutables[i][j] = nondet_u8() & 1;
        }
        d->field_types = field_types[i], d->field_mutables = field_mutables[i];
        ftypes[i].param_types = param_types[i], ftypes[i].result_types = result_types[i];
        canon[i] = nondet_u32();
        __CPROVER_assume(canon[i] < n);
    }
    module.type_defs = defs;
    module.types = ftypes;
    module.type_count = n;
    return n;
}

void harness_types_eq(void) {
    uint32_t n = setup_module(), a = nondet_u32(), b = nondet_u32();
    __CPROVER_assume(a < n && b < n);
    __CPROVER_assert(wah_types_structurally_equal(&module, a, b, canon) == model_types_eq(n, a, b), "same equality");
}


// `wahSub` of proofs/lean/WahProofs/Subtype.lean, which proves that it is the subtyping of the spec
static int64_t emod2(int64_t t) { return ((t % 2) + 2) % 2; } // Int.emod of Lean

static bool model_walk(uint32_t pi, uint32_t fuel, uint32_t t) { // `walk`
    for (; fuel > 0; --fuel) {
        uint32_t st = defs[t].supertype;
        if (st == WAH_NO_SUPERTYPE) return false;
        if (st == pi || canon[st] == canon[pi]) return true;
        t = st;
    }
    return false;
}

static uint32_t model_kind(uint32_t n, int64_t t) { // `wahKind`
    return t >= 0 && t / 2 < n ? (uint32_t)defs[t / 2].kind : 0;
}

static bool model_abs_table(int64_t sub, int64_t sup) { // `absTable`
    bool sub_ref = (-54 <= sub && sub <= -31) || sub >= 0, sup_ref = (-54 <= sup && sup <= -31) || sup >= 0;
    if (!sub_ref || !sup_ref) return false;
    if (sup == -36) return !(sub == -34 || sub == -46 || sub == -50 || sub == -54 || sub == -32 || sub == -48);
    if (sup == -38) return sub == -40 || sub == -42 || sub == -44 || sub == -52;
    if (sup == -42 || sup == -44 || sup == -40) return sub == -52;
    if (sup == -32) return sub == -48;
    if (sup == -34) return sub == -50;
    if (sup == -46) return sub == -54;
    return false;
}

static bool model_is_subtype(uint32_t n, int64_t sub, int64_t sup) { // `wahSub`
    if (sub == sup || sub == -99) return true;
    if (emod2(sub) == 1 && emod2(sup) != 1) return false;
    sub -= emod2(sub), sup -= emod2(sup);
    if (sub == sup) return true;
    if (sub >= 0 && sup >= 0) {
        uint32_t si = (uint32_t)(sub / 2), pi = (uint32_t)(sup / 2);
        return si < n && pi < n && (canon[si] == canon[pi] || model_walk(pi, n, si));
    }
    if (sub >= 0) {
        uint32_t k = model_kind(n, sub);
        if (sup == -36 || sup == -38) return k == WAH_COMP_STRUCT || k == WAH_COMP_ARRAY;
        if (sup == -42) return k == WAH_COMP_STRUCT;
        if (sup == -44) return k == WAH_COMP_ARRAY;
        if (sup == -32) return k == WAH_COMP_FUNC;
        return false;
    }
    if (sup >= 0) {
        uint32_t k = model_kind(n, sup);
        if (sub == -52) return k == WAH_COMP_STRUCT || k == WAH_COMP_ARRAY;
        if (sub == -48) return k == WAH_COMP_FUNC;
        return false;
    }
    return model_abs_table(sub, sup);
}

static wah_type_t nondet_valid_type(uint32_t n) { // `ValidCode`
    wah_type_t t = nondet_i32();
    __CPROVER_assume((t >= -18 && t <= -2 && t % 2 == 0 && t != -12 && t != -14) || t == -99 ||
                     (t >= -54 && t <= -31) || (t >= 0 && WAH_TYIDX(t) < n));
    return t;
}

void harness_is_subtype(void) {
    uint32_t n = setup_module();
    for (uint32_t i = 0; i < N; ++i) // Supertypes are earlier types (`SubWF`)
        __CPROVER_assume(defs[i].supertype == WAH_NO_SUPERTYPE || defs[i].supertype < i);
    module.canonical_map = canon;
    wah_type_t sub = nondet_valid_type(n), sup = nondet_valid_type(n);
    __CPROVER_assert(wah_type_is_subtype(sub, sup, &module) == model_is_subtype(n, sub, sup), "same subtyping");
}

static bool model_accepts(uint32_t target, uint32_t fuel, uint32_t t) { // `acceptsWalk`
    for (; fuel > 0; --fuel) {
        if (canon[t] == canon[target]) return true;
        if (defs[t].supertype == WAH_NO_SUPERTYPE) return false;
        t = defs[t].supertype;
    }
    return false;
}

static wah_repr_info_t repr_store[N], *reprs[N];

void harness_accepts_repr(void) {
    uint32_t n = setup_module();
    for (uint32_t i = 0; i < N; ++i)
        __CPROVER_assume(defs[i].supertype == WAH_NO_SUPERTYPE || defs[i].supertype < i);
    module.canonical_map = canon;
    uint32_t repr_count = nondet_u32();
    __CPROVER_assume(repr_count <= N);
    for (uint32_t r = 0; r < N; ++r) {
        repr_store[r].typeidx = nondet_u32();
        __CPROVER_assume(repr_store[r].typeidx < n);
        reprs[r] = &repr_store[r];
    }
    module.repr_infos = reprs;
    module.repr_count = repr_count;
    uint32_t target = nondet_u32();
    wah_repr_t repr_id = (wah_repr_t)nondet_i32();
    bool model = repr_id >= 0 && (uint32_t)repr_id < repr_count && target < n &&
                 model_accepts(target, n, repr_store[repr_id].typeidx);
    __CPROVER_assert(wah_type_accepts_repr(&module, target, repr_id) == model, "same cast check");
}

// `declCheck` of proofs/lean/WahProofs/SubDecl.lean, which proves that it is the subtyping of the spec
static bool model_mut_eq(int64_t st, int64_t tt) { // `mutEq`
    return st == tt || (st >= 0 && tt >= 0 && emod2(st) == emod2(tt) && canon[st / 2] == canon[tt / 2]);
}

static bool model_field_check(uint32_t n, const td_t *a, const td_t *b, uint32_t j) { // `fieldCheck`
    if (b->mut[j]) return a->mut[j] && model_mut_eq(b->raw[j], a->raw[j]);
    return !a->mut[j] && model_is_subtype(n, a->raw[j], b->raw[j]);
}

static bool model_decl_check(uint32_t n, uint32_t t, uint32_t s) { // `declCheck`
    td_t a = encode(t), b = encode(s);
    if (a.kind == WAH_COMP_FUNC) {
        if (a.c1 != b.c1 || a.c2 != b.c2) return false;
        for (uint32_t j = 0; j < a.c1; ++j) if (!model_is_subtype(n, b.raw[j], a.raw[j])) return false;
        for (uint32_t j = 0; j < a.c2; ++j) if (!model_is_subtype(n, a.raw[a.c1 + j], b.raw[a.c1 + j])) return false;
        return true;
    }
    if (a.kind == WAH_COMP_STRUCT) {
        if (b.nf > a.nf) return false;
        for (uint32_t j = 0; j < b.nf; ++j) if (!model_field_check(n, &a, &b, j)) return false;
        return true;
    }
    if (a.kind == WAH_COMP_ARRAY) return a.nf == 1 && b.nf == 1 && model_field_check(n, &a, &b, 0);
    return true;
}

void harness_matches_supertype(void) {
    uint32_t n = setup_module();
    for (uint32_t i = 0; i < N; ++i) { // `SubWF`
        __CPROVER_assume(defs[i].supertype == WAH_NO_SUPERTYPE ||
                         (defs[i].supertype < i && defs[defs[i].supertype].kind == defs[i].kind));
    }
    module.canonical_map = canon;
    uint32_t t = nondet_u32();
    __CPROVER_assume(t < n);
    bool model = defs[t].supertype == WAH_NO_SUPERTYPE || model_decl_check(n, t, defs[t].supertype);
    __CPROVER_assert(wah_type_matches_supertype(&module, t) == model, "same check of the supertype");
}
