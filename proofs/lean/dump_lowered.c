// Dumps lowered functions of the modules in .wast files (or .wasm files) for wah_lowered_oracle, which checks the
// verdicts of wah_check_lowered_code against the model in WahProofs/Lowered.lean (`make lean-diff`). Only functions
// whose instructions are all in the model are dumped, and each also with mutants of its lowered code (a branch target,
// keep or drop, an immediate such as a lane, an opcode replaced by one with the same immediates, or a slot of a ref
// map), each as
//   F <max depth> <local count> <local>* <result count> <result>* <verdict: 1 if wah_check_lowered_code accepts it>
//   <one line per instruction, see LoweredOracle.lean>
//   M <instruction index> <type count> <type>*      for each stack map
//   L <type index> <1 if struct> <field count> (<offset> <type> <1 if mutable>)*   for each struct and array type
//   S <type count> <type>* <1 if type i is a subtype of type j, else 0, for i != j in order>
//   E
// with types as wah_type_t after wah_check_norm, and branch targets as instruction indices (offsets that are not
// instruction starts become indices beyond the end). S covers all types in the function and their non-null versions,
// and L all struct and array types among them, whose field types are in S as well. Module-level facts, such as
// signatures of callees and whether indices are valid, are resolved here, and instructions that the checker rejects for
// them become `X` (bad). Ref maps are printed as the positions they list, topmost first, or as -1 if they are
// malformed. Modules are parsed with and without fuel metering, to get METERs too.
#include <inttypes.h>
#include <stdint.h>
#define WAH_CHECK_LOWERED
#define WAH_IMPLEMENTATION
#include "../../wah.h"
#include "../../tests/wast.c"

#define MUTANTS 8
#define RARE_MUTANTS 16 // More of those that mutate GC, exception and lane instructions, which are rare
static int mutant_scale = 1; // -m <scale> multiplies them, for small inputs made for mutants
#define MAX_TYPES 256

// Output of one function, built in memory as it may turn out not to be in the model
static char *buf;
static size_t buf_len, buf_cap;
static void put(const char *fmt, ...) {
    va_list ap;
    for (;;) {
        va_start(ap, fmt);
        int n = vsnprintf(buf + buf_len, buf_cap - buf_len, fmt, ap);
        va_end(ap);
        if (n >= 0 && (size_t)n < buf_cap - buf_len) { buf_len += (size_t)n; return; }
        buf_cap = buf_cap ? buf_cap * 2 : 4096;
        buf = realloc(buf, buf_cap);
        if (!buf) abort();
    }
}

// Types of the function, for the subtyping table
static wah_type_t types[MAX_TYPES];
static uint32_t type_count;
static bool too_many_types;
static void add_type(wah_type_t t) {
    for (uint32_t i = 0; i < type_count; i++) if (types[i] == t) return;
    if (type_count == MAX_TYPES) { too_many_types = true; return; }
    types[type_count++] = t;
}

static const wah_module_t *cur;
static uint32_t max_depth; // Of the function

// A type in the model, or false for others (packed types, which never appear in lowered code)
static bool ty(wah_type_t t) {
    if (WAH_TYPE_IS_PACKED(t) || !wah_check_val_type(cur, t)) return false;
    t = wah_check_norm(t);
    add_type(t);
    if (WAH_TYPE_IS_REF(t)) add_type(WAH_TYPE_AS_NON_NULL(t));
    put(" %" PRId32, t);
    return true;
}

// A field type, which may be packed
static bool fty(wah_type_t t) {
    if (!WAH_TYPE_IS_PACKED(t)) return ty(t);
    add_type(t);
    put(" %" PRId32, t);
    return true;
}

static wah_type_t unpacked(wah_type_t t) { return WAH_TYPE_IS_PACKED(t) ? WAH_TYPE_I32 : t; }

static bool heap_ok(wah_type_t t) { return WAH_TYPE_IS_REF(t) && (t < 0 || WAH_TYIDX(t) < cur->type_count); }

static bool comp_ok(uint32_t t, int kind) { return t < cur->type_count && cur->type_defs[t].kind == kind; }

static bool tys(uint32_t n, const wah_type_t *t, wah_type_t extra, bool has_extra) {
    put(" %" PRIu32, n + has_extra);
    for (uint32_t i = 0; i < n; i++) if (!ty(t[i])) return false;
    if (has_extra && !ty(extra)) return false;
    return true;
}

static bool sig(const char *op, uint32_t na, const wah_type_t *a, uint32_t nr, const wah_type_t *r) {
    put("%s", op);
    if (!tys(na, a, 0, false) || !tys(nr, r, 0, false)) return false;
    put("\n");
    return true;
}
#define T(...) ((const wah_type_t[]){ __VA_ARGS__ })
#define SIG(op, na, a, nr, r) do { if (!sig(op, na, a, nr, r)) return false; } while (0)

static void put_map(const wah_parsed_code_t *pc, uint32_t idx) {
    uint32_t n = idx < pc->poll_count ? pc->poll_ref_tops[idx] : 0, count = 0;
    for (uint32_t m = n; m; m = pc->ref_nodes[m - 1].parent, count++) {
        if (m > pc->ref_node_count || pc->ref_nodes[m - 1].parent >= m) { put(" -1"); return; }
    }
    put(" %" PRIu32, count);
    for (uint32_t m = n; m; m = pc->ref_nodes[m - 1].parent) put(" %" PRIu32, pc->ref_nodes[m - 1].pos);
}

static bool opclass_sig(uint8_t cls) {
    enum { I = WAH_TYPE_I32, L = WAH_TYPE_I64, F = WAH_TYPE_F32, D = WAH_TYPE_F64, V = WAH_TYPE_V128 };
    switch (cls) {
        case WAH_OPCLASS__I_I: return sig("o", 1, T(I), 1, T(I));
        case WAH_OPCLASS__I_L: return sig("o", 1, T(I), 1, T(L));
        case WAH_OPCLASS__I_F: return sig("o", 1, T(I), 1, T(F));
        case WAH_OPCLASS__I_D: return sig("o", 1, T(I), 1, T(D));
        case WAH_OPCLASS__I_V: return sig("o", 1, T(I), 1, T(V));
        case WAH_OPCLASS__II_I: return sig("o", 2, T(I, I), 1, T(I));
        case WAH_OPCLASS__IV_V: return sig("o", 2, T(I, V), 1, T(V));
        case WAH_OPCLASS__L_I: return sig("o", 1, T(L), 1, T(I));
        case WAH_OPCLASS__L_L: return sig("o", 1, T(L), 1, T(L));
        case WAH_OPCLASS__L_F: return sig("o", 1, T(L), 1, T(F));
        case WAH_OPCLASS__L_D: return sig("o", 1, T(L), 1, T(D));
        case WAH_OPCLASS__L_V: return sig("o", 1, T(L), 1, T(V));
        case WAH_OPCLASS__LL_I: return sig("o", 2, T(L, L), 1, T(I));
        case WAH_OPCLASS__LL_L: return sig("o", 2, T(L, L), 1, T(L));
        case WAH_OPCLASS__LV_V: return sig("o", 2, T(L, V), 1, T(V));
        case WAH_OPCLASS__F_I: return sig("o", 1, T(F), 1, T(I));
        case WAH_OPCLASS__F_L: return sig("o", 1, T(F), 1, T(L));
        case WAH_OPCLASS__F_F: return sig("o", 1, T(F), 1, T(F));
        case WAH_OPCLASS__F_V: return sig("o", 1, T(F), 1, T(V));
        case WAH_OPCLASS__FF_I: return sig("o", 2, T(F, F), 1, T(I));
        case WAH_OPCLASS__FF_F: return sig("o", 2, T(F, F), 1, T(F));
        case WAH_OPCLASS__D_I: return sig("o", 1, T(D), 1, T(I));
        case WAH_OPCLASS__D_L: return sig("o", 1, T(D), 1, T(L));
        case WAH_OPCLASS__D_D: return sig("o", 1, T(D), 1, T(D));
        case WAH_OPCLASS__D_V: return sig("o", 1, T(D), 1, T(V));
        case WAH_OPCLASS__DD_I: return sig("o", 2, T(D, D), 1, T(I));
        case WAH_OPCLASS__DD_L: return sig("o", 2, T(D, D), 1, T(L));
        case WAH_OPCLASS__DD_D: return sig("o", 2, T(D, D), 1, T(D));
        case WAH_OPCLASS__V_I: return sig("o", 1, T(V), 1, T(I));
        case WAH_OPCLASS__V_V: return sig("o", 1, T(V), 1, T(V));
        case WAH_OPCLASS__VI_V: return sig("o", 2, T(V, I), 1, T(V));
        case WAH_OPCLASS__VV_V: return sig("o", 2, T(V, V), 1, T(V));
        case WAH_OPCLASS__VVV_V: return sig("o", 3, T(V, V, V), 1, T(V));
        default: return false;
    }
}

// Scalar loads and stores: the type loaded (or 0 for stores) and the type stored (or 0 for loads)
static bool mem_sig(uint16_t base, wah_type_t *load, wah_type_t *store) {
    *load = *store = 0;
    switch (base) {
        case WAH_OP_I32_LOAD: case WAH_OP_I32_LOAD8_S: case WAH_OP_I32_LOAD8_U: case WAH_OP_I32_LOAD16_S:
        case WAH_OP_I32_LOAD16_U: *load = WAH_TYPE_I32; return true;
        case WAH_OP_I64_LOAD: case WAH_OP_I64_LOAD8_S: case WAH_OP_I64_LOAD8_U: case WAH_OP_I64_LOAD16_S:
        case WAH_OP_I64_LOAD16_U: case WAH_OP_I64_LOAD32_S: case WAH_OP_I64_LOAD32_U: *load = WAH_TYPE_I64; return true;
        case WAH_OP_F32_LOAD: *load = WAH_TYPE_F32; return true;
        case WAH_OP_F64_LOAD: *load = WAH_TYPE_F64; return true;
        case WAH_OP_I32_STORE: case WAH_OP_I32_STORE8: case WAH_OP_I32_STORE16: *store = WAH_TYPE_I32; return true;
        case WAH_OP_I64_STORE: case WAH_OP_I64_STORE8: case WAH_OP_I64_STORE16: case WAH_OP_I64_STORE32:
            *store = WAH_TYPE_I64; return true;
        case WAH_OP_F32_STORE: *store = WAH_TYPE_F32; return true;
        case WAH_OP_F64_STORE: *store = WAH_TYPE_F64; return true;
        case WAH_OP_V128_LOAD: case WAH_OP_V128_LOAD8X8_S: case WAH_OP_V128_LOAD8X8_U: case WAH_OP_V128_LOAD16X4_S:
        case WAH_OP_V128_LOAD16X4_U: case WAH_OP_V128_LOAD32X2_S: case WAH_OP_V128_LOAD32X2_U:
        case WAH_OP_V128_LOAD8_SPLAT: case WAH_OP_V128_LOAD16_SPLAT: case WAH_OP_V128_LOAD32_SPLAT:
        case WAH_OP_V128_LOAD64_SPLAT: case WAH_OP_V128_LOAD32_ZERO: case WAH_OP_V128_LOAD64_ZERO:
            *load = WAH_TYPE_V128; return true;
        case WAH_OP_V128_STORE: *store = WAH_TYPE_V128; return true;
        default: return false;
    }
}

// Lanes of SIMD loads and stores of lanes, or 0 for others
static uint32_t mem_lanes(uint16_t base, bool *load) {
    *load = true;
    switch (base) {
        case WAH_OP_V128_LOAD8_LANE: return 16;
        case WAH_OP_V128_LOAD16_LANE: return 8;
        case WAH_OP_V128_LOAD32_LANE: return 4;
        case WAH_OP_V128_LOAD64_LANE: return 2;
        default: break;
    }
    *load = false;
    switch (base) {
        case WAH_OP_V128_STORE8_LANE: return 16;
        case WAH_OP_V128_STORE16_LANE: return 8;
        case WAH_OP_V128_STORE32_LANE: return 4;
        case WAH_OP_V128_STORE64_LANE: return 2;
        default: return 0;
    }
}

// `l`: an instruction with lanes `ks` of `n` lanes
static bool lanes(uint32_t n, uint32_t count, const uint8_t *ks, uint32_t na, const wah_type_t *a, uint32_t nr,
                  const wah_type_t *r) {
    put("l %" PRIu32 " %" PRIu32, n, count);
    for (uint32_t i = 0; i < count; i++) put(" %u", ks[i]);
    return sig("", na, a, nr, r);
}

static bool mem_ok(uint32_t idx, wah_type_t at) {
    return idx < wah_memory_index_limit(cur) && wah_memory_type(cur, idx)->addr_type == at;
}

// Instruction indices of offsets, or beyond the end for others
static uint32_t *index_of;
static uint32_t index_cap, instr_count;
static uint64_t target(uint32_t off, uint32_t size) {
    return off < size && index_of[off] != UINT32_MAX ? index_of[off] : (uint64_t)instr_count + 1 + off;
}

// One instruction, or false if it isn't in the model
static bool dump_instr(const wah_parsed_code_t *pc, uint32_t off, const wah_check_instr_t *ip) {
    const wah_module_t *m = cur;
    const uint8_t *code = pc->bytecode;
    uint32_t size = pc->bytecode_size;
    wah_check_instr_t in = *ip;
    wah_type_t load, store;
    switch (in.op) {
        case WAH_OP_I32_CONST: SIG("o", 0, NULL, 1, T(WAH_TYPE_I32)); return true;
        case WAH_OP_I64_CONST: SIG("o", 0, NULL, 1, T(WAH_TYPE_I64)); return true;
        case WAH_OP_F32_CONST: SIG("o", 0, NULL, 1, T(WAH_TYPE_F32)); return true;
        case WAH_OP_F64_CONST: SIG("o", 0, NULL, 1, T(WAH_TYPE_F64)); return true;
        case WAH_OP_V128_CONST: SIG("o", 0, NULL, 1, T(WAH_TYPE_V128)); return true;
        case WAH_OP_I8X16_SHUFFLE: { // Lanes index both operands
            enum { V = WAH_TYPE_V128 };
            return lanes(32, 16, code + off + sizeof(uint16_t), 2, T(V, V), 1, T(V));
        }
        case WAH_OP_I8X16_EXTRACT_LANE_S: case WAH_OP_I8X16_EXTRACT_LANE_U: case WAH_OP_I16X8_EXTRACT_LANE_S:
        case WAH_OP_I16X8_EXTRACT_LANE_U: case WAH_OP_I32X4_EXTRACT_LANE: case WAH_OP_I64X2_EXTRACT_LANE:
        case WAH_OP_F32X4_EXTRACT_LANE: case WAH_OP_F64X2_EXTRACT_LANE: case WAH_OP_I8X16_REPLACE_LANE:
        case WAH_OP_I16X8_REPLACE_LANE: case WAH_OP_I32X4_REPLACE_LANE: case WAH_OP_I64X2_REPLACE_LANE:
        case WAH_OP_F32X4_REPLACE_LANE: case WAH_OP_F64X2_REPLACE_LANE: {
            enum { I = WAH_TYPE_I32, L = WAH_TYPE_I64, F = WAH_TYPE_F32, D = WAH_TYPE_F64, V = WAH_TYPE_V128 };
            uint32_t n; wah_type_t t; bool extract = true;
            switch (in.op) {
                case WAH_OP_I8X16_REPLACE_LANE: extract = false; // fallthrough
                case WAH_OP_I8X16_EXTRACT_LANE_S: case WAH_OP_I8X16_EXTRACT_LANE_U: n = 16; t = I; break;
                case WAH_OP_I16X8_REPLACE_LANE: extract = false; // fallthrough
                case WAH_OP_I16X8_EXTRACT_LANE_S: case WAH_OP_I16X8_EXTRACT_LANE_U: n = 8; t = I; break;
                case WAH_OP_I32X4_REPLACE_LANE: extract = false; // fallthrough
                case WAH_OP_I32X4_EXTRACT_LANE: n = 4; t = I; break;
                case WAH_OP_I64X2_REPLACE_LANE: extract = false; // fallthrough
                case WAH_OP_I64X2_EXTRACT_LANE: n = 2; t = L; break;
                case WAH_OP_F32X4_REPLACE_LANE: extract = false; // fallthrough
                case WAH_OP_F32X4_EXTRACT_LANE: n = 4; t = F; break;
                case WAH_OP_F64X2_REPLACE_LANE: extract = false; // fallthrough
                default: n = 2; t = D; break;
            }
            uint8_t k = in.lane;
            return extract ? lanes(n, 1, &k, 1, T(V), 1, T(t)) : lanes(n, 1, &k, 2, T(V, t), 1, T(V));
        }
        case WAH_OP_F32_DEMOTE_F64: SIG("o", 1, T(WAH_TYPE_F64), 1, T(WAH_TYPE_F32)); return true;
        case WAH_OP_F64_PROMOTE_F32: SIG("o", 1, T(WAH_TYPE_F32), 1, T(WAH_TYPE_F64)); return true;
        case WAH_OP_REF_I31: SIG("o", 1, T(WAH_TYPE_I32), 1, T(WAH_TYPE_I31)); return true;
        case WAH_OP_I31_GET_S: case WAH_OP_I31_GET_U: SIG("o", 1, T(WAH_TYPE_I31REF), 1, T(WAH_TYPE_I32)); return true;
        case WAH_OP_DROP: put("d\n"); return true;
        case WAH_OP_SELECT: put("s\n"); return true;
        case WAH_OP_SELECT_T: {
            wah_type_t t = (wah_type_t)(int32_t)in.a;
            if (!wah_check_val_type(m, t)) { put("X\n"); return true; }
            SIG("o", 3, T(t, t, WAH_TYPE_I32), 1, T(t));
            return true;
        }
        case WAH_OP_REF_NULL: {
            wah_type_t ht = (wah_type_t)(int32_t)in.a;
            if (!heap_ok(ht)) { put("X\n"); return true; }
            SIG("o", 0, NULL, 1, T(WAH_TYPE_AS_NULLABLE(ht)));
            return true;
        }
        case WAH_OP_REF_FUNC: {
            if (in.a >= wah_func_index_limit(m)) { put("X\n"); return true; }
            uint32_t idx = (uint32_t)(wah_resolve_func_type(m, in.a) - m->types);
            SIG("o", 0, NULL, 1, T(WAH_TYPE_FROM_IDX(idx, 0)));
            return true;
        }
        case WAH_OP_REF_IS_NULL: put("z\n"); return true;
        case WAH_OP_REF_AS_NON_NULL: put("a\n"); return true;
        case WAH_OP_LOCAL_GET: put("g %" PRIu32 "\n", in.a); return true;
        case WAH_OP_LOCAL_SET: put("x %" PRIu32 "\n", in.a); return true;
        case WAH_OP_LOCAL_TEE: put("t %" PRIu32 "\n", in.a); return true;
        case WAH_OP_GLOBAL_GET: case WAH_OP_GLOBAL_SET: case WAH_OP_GLOBAL_GET_INDIRECT: case WAH_OP_GLOBAL_SET_INDIRECT: {
            if (in.a >= wah_global_index_limit(m)) { put("X\n"); return true; }
            bool indirect = in.a < m->import_global_count && m->global_imports[in.a].is_mutable;
            bool get = in.op == WAH_OP_GLOBAL_GET || in.op == WAH_OP_GLOBAL_GET_INDIRECT;
            wah_type_t t = wah_global_type(m, in.a);
            if (indirect != (in.op == WAH_OP_GLOBAL_GET_INDIRECT || in.op == WAH_OP_GLOBAL_SET_INDIRECT) ||
                (!get && !wah_global_is_mutable(m, in.a))) put("X\n");
            else if (get) SIG("o", 0, NULL, 1, T(t));
            else SIG("o", 1, T(t), 0, NULL);
            return true;
        }
        case WAH_OP_MEMORY_SIZE: case WAH_OP_MEMORY_GROW: case WAH_OP_MEMORY_SIZE_i64: case WAH_OP_MEMORY_GROW_i64: {
            wah_type_t at = in.op == WAH_OP_MEMORY_SIZE || in.op == WAH_OP_MEMORY_GROW ? WAH_TYPE_I32 : WAH_TYPE_I64;
            if (!mem_ok(in.a, at)) put("X\n");
            else if (in.op == WAH_OP_MEMORY_SIZE || in.op == WAH_OP_MEMORY_SIZE_i64) SIG("o", 0, NULL, 1, T(at));
            else SIG("o", 1, T(at), 1, T(at));
            return true;
        }
        case WAH_OP_DATA_DROP: if (in.a < m->data_segment_count) put("o 0 0\n"); else put("X\n"); return true;
        case WAH_OP_ELEM_DROP: if (in.a < m->element_segment_count) put("o 0 0\n"); else put("X\n"); return true;
        case WAH_OP_MEMORY_FILL: case WAH_OP_MEMORY_FILL_i64: {
            wah_type_t at = in.op == WAH_OP_MEMORY_FILL ? WAH_TYPE_I32 : WAH_TYPE_I64;
            if (!mem_ok(in.a, at)) { put("X\n"); return true; }
            put("y"), put_map(pc, in.b);
            return sig("", 3, T(at, WAH_TYPE_I32, at), 0, NULL);
        }
        case WAH_OP_MEMORY_INIT: case WAH_OP_TABLE_INIT: case WAH_OP_MEMORY_INIT_i64: case WAH_OP_TABLE_INIT_i64: {
            wah_type_t at = in.op == WAH_OP_MEMORY_INIT || in.op == WAH_OP_TABLE_INIT ? WAH_TYPE_I32 : WAH_TYPE_I64;
            bool ok;
            if (in.op == WAH_OP_MEMORY_INIT || in.op == WAH_OP_MEMORY_INIT_i64) {
                ok = in.a < m->data_segment_count && mem_ok(in.b, at);
            } else {
                ok = in.a < m->element_segment_count && in.b < wah_table_index_limit(m) &&
                     wah_table_type(m, in.b)->addr_type == at &&
                     wah_type_is_subtype(m->element_segments[in.a].elem_type, wah_table_type(m, in.b)->elem_type, m);
            }
            if (!ok) { put("X\n"); return true; }
            put("y"), put_map(pc, in.c);
            return sig("", 3, T(at, WAH_TYPE_I32, WAH_TYPE_I32), 0, NULL);
        }
        case WAH_OP_MEMORY_COPY: case WAH_OP_TABLE_COPY:
        case WAH_OP_MEMORY_COPY_i32_to_i64: case WAH_OP_MEMORY_COPY_i64_to_i32: case WAH_OP_MEMORY_COPY_i64_to_i64:
        case WAH_OP_TABLE_COPY_i32_to_i64: case WAH_OP_TABLE_COPY_i64_to_i32: case WAH_OP_TABLE_COPY_i64_to_i64: {
            wah_type_t dst_at = WAH_TYPE_I32, src_at = WAH_TYPE_I32;
            bool table = in.op == WAH_OP_TABLE_COPY || in.op == WAH_OP_TABLE_COPY_i32_to_i64 ||
                         in.op == WAH_OP_TABLE_COPY_i64_to_i32 || in.op == WAH_OP_TABLE_COPY_i64_to_i64;
            switch (in.op) {
                case WAH_OP_MEMORY_COPY_i32_to_i64: case WAH_OP_TABLE_COPY_i32_to_i64: dst_at = WAH_TYPE_I64; break;
                case WAH_OP_MEMORY_COPY_i64_to_i32: case WAH_OP_TABLE_COPY_i64_to_i32: src_at = WAH_TYPE_I64; break;
                case WAH_OP_MEMORY_COPY_i64_to_i64: case WAH_OP_TABLE_COPY_i64_to_i64: dst_at = src_at = WAH_TYPE_I64; break;
                default: break;
            }
            bool ok;
            if (table) {
                ok = in.a < wah_table_index_limit(m) && in.b < wah_table_index_limit(m);
                if (ok) {
                    const wah_table_type_t *dst = wah_table_type(m, in.a), *src = wah_table_type(m, in.b);
                    ok = dst->addr_type == dst_at && src->addr_type == src_at &&
                         wah_type_is_subtype(src->elem_type, dst->elem_type, m);
                }
            } else {
                ok = mem_ok(in.a, dst_at) && mem_ok(in.b, src_at);
            }
            if (!ok) { put("X\n"); return true; }
            put("y"), put_map(pc, in.c);
            wah_type_t len = dst_at == WAH_TYPE_I64 && src_at == WAH_TYPE_I64 ? WAH_TYPE_I64 : WAH_TYPE_I32;
            return sig("", 3, T(dst_at, src_at, len), 0, NULL);
        }
        case WAH_OP_TABLE_GET: case WAH_OP_TABLE_SET: case WAH_OP_TABLE_SIZE: case WAH_OP_TABLE_GROW: case WAH_OP_TABLE_FILL:
        case WAH_OP_TABLE_GET_i64: case WAH_OP_TABLE_SET_i64: case WAH_OP_TABLE_SIZE_i64: case WAH_OP_TABLE_GROW_i64:
        case WAH_OP_TABLE_FILL_i64: {
            bool i32 = in.op == WAH_OP_TABLE_GET || in.op == WAH_OP_TABLE_SET || in.op == WAH_OP_TABLE_SIZE ||
                       in.op == WAH_OP_TABLE_GROW || in.op == WAH_OP_TABLE_FILL;
            wah_type_t at = i32 ? WAH_TYPE_I32 : WAH_TYPE_I64;
            if (in.a >= wah_table_index_limit(m) || wah_table_type(m, in.a)->addr_type != at) { put("X\n"); return true; }
            wah_type_t et = wah_table_type(m, in.a)->elem_type;
            switch (in.op) {
                case WAH_OP_TABLE_GET: case WAH_OP_TABLE_GET_i64: return sig("o", 1, T(at), 1, T(et));
                case WAH_OP_TABLE_SET: case WAH_OP_TABLE_SET_i64: return sig("o", 2, T(at, et), 0, NULL);
                case WAH_OP_TABLE_SIZE: case WAH_OP_TABLE_SIZE_i64: return sig("o", 0, NULL, 1, T(at));
                case WAH_OP_TABLE_GROW: case WAH_OP_TABLE_GROW_i64: return sig("o", 2, T(et, at), 1, T(at));
                default: put("y"), put_map(pc, in.b); return sig("", 3, T(at, et, at), 0, NULL);
            }
        }
        case WAH_OP_CALL: case WAH_OP_RETURN_CALL: {
            if (in.a >= wah_func_index_limit(m)) { put("X\n"); return true; }
            const wah_func_type_t *ft = wah_resolve_func_type(m, in.a);
            return sig(in.op == WAH_OP_CALL ? "k" : "r", ft->param_count, ft->param_types, ft->result_count, ft->result_types);
        }
        case WAH_OP_CALL_REF: case WAH_OP_RETURN_CALL_REF:
        case WAH_OP_CALL_INDIRECT: case WAH_OP_CALL_INDIRECT_i64:
        case WAH_OP_RETURN_CALL_INDIRECT: case WAH_OP_RETURN_CALL_INDIRECT_i64: {
            bool tail = in.op == WAH_OP_RETURN_CALL_REF || in.op == WAH_OP_RETURN_CALL_INDIRECT ||
                        in.op == WAH_OP_RETURN_CALL_INDIRECT_i64;
            bool by_ref = in.op == WAH_OP_CALL_REF || in.op == WAH_OP_RETURN_CALL_REF;
            if (in.a >= m->type_count || m->type_defs[in.a].kind != WAH_COMP_FUNC) { put("X\n"); return true; }
            wah_type_t extra = WAH_TYPE_FROM_IDX(in.a, 1);
            if (!by_ref) {
                extra = in.op == WAH_OP_CALL_INDIRECT || in.op == WAH_OP_RETURN_CALL_INDIRECT ? WAH_TYPE_I32 : WAH_TYPE_I64;
                if (in.b >= wah_table_index_limit(m) ||
                    !wah_type_is_subtype(wah_table_type(m, in.b)->elem_type, WAH_TYPE_FUNCREF, m) ||
                    wah_table_type(m, in.b)->addr_type != extra) { put("X\n"); return true; }
            }
            const wah_func_type_t *ft = &m->types[in.a];
            put(tail ? "r" : "k");
            if (!tys(ft->param_count, ft->param_types, extra, true) || !tys(ft->result_count, ft->result_types, 0, false))
                return false;
            put("\n");
            return true;
        }
        case WAH_OP_BR: case WAH_OP_BR_IF: case WAH_OP_BR_ON_NULL: case WAH_OP_BR_ON_NON_NULL:
            put("%c %" PRIu64 " %" PRIu32 " %" PRIu32 "\n", in.op == WAH_OP_BR ? 'b' : in.op == WAH_OP_BR_IF ? 'B' :
                in.op == WAH_OP_BR_ON_NULL ? 'N' : 'O', target(in.a, size), in.b, in.c);
            return true;
        case WAH_OP_BR_TABLE: {
            const uint8_t *records = code + off + sizeof(uint16_t) + 2 * sizeof(uint32_t) + ((size_t)in.a + 1) * sizeof(uint16_t);
            put("T %" PRIu32 " %" PRIu32, in.b, in.c);
            for (uint32_t i = 0; i < in.c; i++) {
                put(" %" PRIu64 " %" PRIu32, target(wah_read_u32_le(records + (size_t)i * 8), size),
                    wah_read_u32_le(records + (size_t)i * 8 + 4));
            }
            put("\n");
            return true;
        }
        case WAH_OP_IF: put("i %" PRIu64 "\n", target(in.a, size)); return true;
        case WAH_OP_ELSE: put("e %" PRIu64 "\n", target(in.a, size)); return true;
        case WAH_OP_METER: put("m %" PRIu64 "\n", target(in.b, size)); return true;
        case WAH_OP_POLL: case WAH_OP_TICK:
            put(in.op == WAH_OP_POLL ? "p" : "q"), put_map(pc, in.a), put("\n");
            return true;
        case WAH_OP_REF_EQ: put("Q\n"); return true;
        case WAH_OP_ARRAY_LEN: SIG("o", 1, T(WAH_TYPE_ARRAYREF), 1, T(WAH_TYPE_I32)); return true;
        case WAH_OP_REF_TEST: case WAH_OP_REF_TEST_NULL: case WAH_OP_REF_CAST: case WAH_OP_REF_CAST_NULL: {
            wah_type_t ht = (wah_type_t)(int32_t)in.a;
            if (!heap_ok(ht)) put("X\n");
            else if (in.op == WAH_OP_REF_TEST || in.op == WAH_OP_REF_TEST_NULL) put("z\n");
            else if (put("c"), !ty(in.op == WAH_OP_REF_CAST ? WAH_TYPE_AS_NON_NULL(ht) : WAH_TYPE_AS_NULLABLE(ht))) return false;
            else put("\n");
            return true;
        }
        case WAH_OP_BR_ON_CAST: case WAH_OP_BR_ON_CAST_FAIL: {
            wah_type_t ht = (wah_type_t)(int32_t)in.b;
            bool nul = (in.lane & 0x02) != 0;
            if (!heap_ok(ht)) { put("X\n"); return true; }
            put("C %d %d", in.op == WAH_OP_BR_ON_CAST_FAIL, nul);
            if (!ty(nul ? WAH_TYPE_AS_NULLABLE(ht) : WAH_TYPE_AS_NON_NULL(ht))) return false;
            put(" %" PRIu64 " %" PRIu32 " %" PRIu32 "\n", target(in.a, size), in.c, (uint32_t)in.x);
            return true;
        }
        case WAH_OP_STRUCT_NEW: case WAH_OP_STRUCT_NEW_DEFAULT: {
            if (!comp_ok(in.a, WAH_COMP_STRUCT)) { put("X\n"); return true; }
            const wah_type_def_t *td = &m->type_defs[in.a];
            if (in.op == WAH_OP_STRUCT_NEW_DEFAULT) {
                if (td->non_defaultable) put("X\n");
                else SIG("o", 0, NULL, 1, T(WAH_TYPE_FROM_IDX(in.a, 0)));
                return true;
            }
            put("o %" PRIu32, td->field_count);
            for (uint32_t i = 0; i < td->field_count; i++) if (!ty(unpacked(td->field_types[i]))) return false;
            put(" 1");
            if (!ty(WAH_TYPE_FROM_IDX(in.a, 0))) return false;
            put("\n");
            return true;
        }
        case WAH_OP_ARRAY_NEW: case WAH_OP_ARRAY_NEW_DEFAULT: case WAH_OP_ARRAY_NEW_FIXED: case WAH_OP_ARRAY_NEW_DATA:
        case WAH_OP_ARRAY_NEW_ELEM: case WAH_OP_ARRAY_FILL: case WAH_OP_ARRAY_INIT_DATA: {
            if (!comp_ok(in.a, WAH_COMP_ARRAY)) { put("X\n"); return true; }
            const wah_type_def_t *td = &m->type_defs[in.a];
            wah_type_t et = td->field_types[0], ut = unpacked(et), r = WAH_TYPE_FROM_IDX(in.a, 0);
            bool ref = (wah_check_storage(et) & 0x200) != 0;
            switch (in.op) {
                case WAH_OP_ARRAY_NEW: put("y"), put_map(pc, in.b); return sig("", 2, T(ut, WAH_TYPE_I32), 1, T(r));
                case WAH_OP_ARRAY_NEW_DEFAULT:
                    if (td->non_defaultable) { put("X\n"); return true; }
                    put("y"), put_map(pc, in.b);
                    return sig("", 1, T(WAH_TYPE_I32), 1, T(r));
                case WAH_OP_ARRAY_NEW_FIXED:
                    // Popping more than the stack can hold always fails, and mutants may have huge lengths
                    if (in.b > max_depth) { put("X\n"); return true; }
                    put("o %" PRIu32, in.b);
                    for (uint32_t i = 0; i < in.b; i++) if (!ty(ut)) return false;
                    put(" 1");
                    if (!ty(r)) return false;
                    put("\n");
                    return true;
                case WAH_OP_ARRAY_NEW_DATA: case WAH_OP_ARRAY_NEW_ELEM:
                    if (in.op == WAH_OP_ARRAY_NEW_DATA ? in.b >= m->data_segment_count || ref :
                        in.b >= m->element_segment_count || !ref ||
                        !wah_check_sub(m, m->element_segments[in.b].elem_type, et)) { put("X\n"); return true; }
                    put("y"), put_map(pc, in.c);
                    return sig("", 2, T(WAH_TYPE_I32, WAH_TYPE_I32), 1, T(r));
                default: // array.fill and array.init_data
                    if (!td->field_mutables[0] ||
                        (in.op == WAH_OP_ARRAY_INIT_DATA && (in.b >= m->data_segment_count || ref))) {
                        put("X\n");
                        return true;
                    }
                    put("y"), put_map(pc, in.op == WAH_OP_ARRAY_FILL ? in.b : in.c);
                    return sig("", 4, T(WAH_TYPE_FROM_IDX(in.a, 1), WAH_TYPE_I32,
                                        in.op == WAH_OP_ARRAY_FILL ? ut : WAH_TYPE_I32, WAH_TYPE_I32), 0, NULL);
            }
        }
        case WAH_OP_ARRAY_COPY: put("P"), put_map(pc, in.a), put("\n"); return true;
        case WAH_OP_ARRAY_INIT_ELEM:
            if (in.a >= m->element_segment_count) { put("X\n"); return true; }
            put("I"), put_map(pc, in.b);
            if (!ty(m->element_segments[in.a].elem_type)) return false;
            put("\n");
            return true;
        case WAH_OP_STRUCT_GET_S8: case WAH_OP_STRUCT_GET_S16: case WAH_OP_STRUCT_GET_U8: case WAH_OP_STRUCT_GET_U16:
        case WAH_OP_STRUCT_GET_32: case WAH_OP_STRUCT_GET_64: case WAH_OP_STRUCT_GET_128:
            put("G %" PRIu32 " %" PRIu32 "\n", wah_check_access(in.op), in.a);
            return true;
        case WAH_OP_STRUCT_SET_8: case WAH_OP_STRUCT_SET_16: case WAH_OP_STRUCT_SET_32: case WAH_OP_STRUCT_SET_64:
        case WAH_OP_STRUCT_SET_128:
            put("H %" PRIu32 " %" PRIu32 "\n", wah_check_access(in.op), in.a);
            return true;
        case WAH_OP_ARRAY_GET_S8: case WAH_OP_ARRAY_GET_S16: case WAH_OP_ARRAY_GET_U8: case WAH_OP_ARRAY_GET_U16:
        case WAH_OP_ARRAY_GET_32: case WAH_OP_ARRAY_GET_64: case WAH_OP_ARRAY_GET_128:
            put("A %" PRIu32 "\n", wah_check_access(in.op));
            return true;
        case WAH_OP_ARRAY_SET_8: case WAH_OP_ARRAY_SET_16: case WAH_OP_ARRAY_SET_32: case WAH_OP_ARRAY_SET_64:
        case WAH_OP_ARRAY_SET_128:
            put("Y %" PRIu32 "\n", wah_check_access(in.op));
            return true;
        case WAH_OP_TRY_TABLE: {
            const uint8_t *c = code + off + sizeof(uint16_t) + sizeof(uint32_t);
            for (uint32_t i = 0; i < in.a; i++, c += 1 + 3 * sizeof(uint32_t)) {
                bool tagged = c[0] == WAH_CATCH_KIND_CATCH || c[0] == WAH_CATCH_KIND_CATCH_REF;
                if (c[0] > WAH_CATCH_KIND_CATCH_ALL_REF ||
                    (tagged && wah_read_u32_le(c + 1) >= m->import_tag_count + m->tag_count)) { put("X\n"); return true; }
            }
            put("W %" PRIu32, in.a);
            c = code + off + sizeof(uint16_t) + sizeof(uint32_t);
            for (uint32_t i = 0; i < in.a; i++, c += 1 + 3 * sizeof(uint32_t)) {
                uint32_t tag = wah_read_u32_le(c + 1);
                bool tagged = c[0] == WAH_CATCH_KIND_CATCH || c[0] == WAH_CATCH_KIND_CATCH_REF;
                bool ref = c[0] == WAH_CATCH_KIND_CATCH_REF || c[0] == WAH_CATCH_KIND_CATCH_ALL_REF;
                put(" %u %" PRIu32 " %" PRIu64 " %" PRIu32, c[0], tag,
                    target(wah_read_u32_le(c + 1 + sizeof(uint32_t)), size), wah_read_u32_le(c + 1 + 2 * sizeof(uint32_t)));
                const wah_func_type_t *tt = tagged ? &m->types[wah_tag_type_index(m, tag)] : NULL;
                if (!tys(tt ? tt->param_count : 0, tt ? tt->param_types : NULL, WAH_TYPE_EXN, ref)) return false;
            }
            put("\n");
            return true;
        }
        case WAH_OP_END_TRY_TABLE: put("U\n"); return true;
        case WAH_OP_TRIM_HANDLERS: put("V %" PRIu32 "\n", in.a); return true;
        case WAH_OP_THROW: {
            if (in.a >= m->import_tag_count + m->tag_count) { put("X\n"); return true; }
            const wah_func_type_t *tt = &m->types[wah_tag_type_index(m, in.a)];
            put("Z");
            if (!tys(tt->param_count, tt->param_types, 0, false)) return false;
            put("\n");
            return true;
        }
        case WAH_OP_THROW_REF:
            put("Z");
            if (!tys(1, T(WAH_TYPE_EXNREF), 0, false)) return false;
            put("\n");
            return true;
        case WAH_OP_END: put("n\n"); return true;
        case WAH_OP_RETURN: put("R\n"); return true;
        case WAH_OP_UNREACHABLE: put("u\n"); return true;
        default:
            if (in.mem != WAH_CHECK_MEM_NONE) {
                bool lane_load;
                uint32_t n = mem_lanes(in.base, &lane_load);
                if (!n && !mem_sig(in.base, &load, &store)) return false;
                bool mem0 = in.mem == WAH_CHECK_MEM_I32_MEM0 || in.mem == WAH_CHECK_MEM_I64_MEM0;
                wah_type_t at = in.mem == WAH_CHECK_MEM_I32 || in.mem == WAH_CHECK_MEM_I32_MEM0 ? WAH_TYPE_I32 : WAH_TYPE_I64;
                if (!mem_ok(mem0 ? 0 : in.a, at)) { put("X\n"); return true; }
                uint8_t k = in.lane;
                if (n) return lanes(n, 1, &k, 2, T(at, WAH_TYPE_V128), lane_load, T(WAH_TYPE_V128));
                if (load) return sig("o", 1, T(at), 1, T(load));
                return sig("o", 2, T(at, store), 0, NULL);
            }
            return in.op < WAH_FE && opclass_sig(wah_opclasses[in.op]);
    }
}

// Struct and array types among the types of the function, which grow by their field types, or false if they lack
// layouts. wah_check_lowered_code finds fields by offsets in wah_repr_info_t, with types in wah_type_def_t.
static bool dump_layouts(void) {
    uint32_t done[MAX_TYPES], done_count = 0;
    for (uint32_t i = 0; i < type_count; i++) {
        wah_type_t t = types[i];
        if (t < 0 || WAH_TYIDX(t) >= cur->type_count) continue;
        uint32_t idx = WAH_TYIDX(t);
        const wah_type_def_t *td = &cur->type_defs[idx];
        bool seen = false;
        for (uint32_t j = 0; j < done_count; j++) seen |= done[j] == idx;
        if (td->kind == WAH_COMP_FUNC || seen) continue;
        done[done_count++] = idx;
        const wah_repr_info_t *ri = wah_repr_info_get(cur, wah_module_typeidx_to_repr(cur, idx));
        bool st = td->kind == WAH_COMP_STRUCT;
        if (!ri || (st && ri->count != td->field_count)) return false;
        put("L %" PRIu32 " %d %" PRIu32, idx, st, td->field_count);
        for (uint32_t f = 0; f < td->field_count; f++) {
            put(" %" PRIu32, st ? ri->fields[f].offset : 0);
            if (!fty(td->field_types[f])) return false;
            put(" %d", td->field_mutables[f]);
        }
        put("\n");
    }
    return true;
}

// Dumps code (which may be a mutant) of a function, or returns false if it isn't in the model
static bool dump_code(const wah_func_type_t *ft, const wah_code_body_t *body, const wah_parsed_code_t *pc) {
    const uint8_t *code = pc->bytecode;
    uint32_t size = pc->bytecode_size;
    wah_check_instr_t in;
    if (size > index_cap) {
        index_cap = size;
        index_of = realloc(index_of, (size_t)size * sizeof(uint32_t));
        if (!index_of) abort();
    }
    for (uint32_t i = 0; i < size; i++) index_of[i] = UINT32_MAX;
    instr_count = 0;
    for (uint32_t off = 0; off < size; off += in.len) {
        if (wah_check_decode(code, size, off, &in) != WAH_OK) return false; // Pass 1 of the checker fails
        index_of[off] = instr_count++;
    }
    bool ok = wah_check_lowered_code(cur, pc, ft, body, body->max_stack_depth, WAH_TYPE_BOT) == WAH_OK;
    max_depth = body->max_stack_depth;
    type_count = 0;
    too_many_types = false;
    put("F %" PRIu32 " %" PRIu32, body->max_stack_depth, ft->param_count + body->local_count);
    for (uint32_t i = 0; i < ft->param_count + body->local_count; i++) {
        if (!ty(i < ft->param_count ? ft->param_types[i] : wah_local_type(body, i - ft->param_count))) return false;
    }
    if (!tys(ft->result_count, ft->result_types, 0, false)) return false;
    put(" %d\n", ok);
    for (uint32_t off = 0; off < size; off += in.len) {
        wah_check_decode(code, size, off, &in);
        if (!dump_instr(pc, off, &in)) return false;
    }
    for (uint32_t i = 0; i < pc->stack_map_count; i++) {
        const wah_stack_map_t *sm = &pc->stack_maps[i];
        if (sm->start > pc->stack_map_type_count || sm->count > pc->stack_map_type_count - sm->start) return false;
        put("M %" PRIu64, target(sm->offset, size));
        if (!tys(sm->count, pc->stack_map_types + sm->start, 0, false)) return false;
        put("\n");
    }
    if (!dump_layouts() || too_many_types) return false;
    put("S %" PRIu32, type_count);
    for (uint32_t i = 0; i < type_count; i++) put(" %" PRId32, types[i]);
    put(" ");
    for (uint32_t i = 0; i < type_count; i++) {
        for (uint32_t j = 0; j < type_count; j++) {
            if (i != j) put("%d", wah_check_sub(cur, types[i], types[j]));
        }
    }
    put("\nE\n");
    return true;
}

static uint64_t rng_state;
static uint32_t rng(uint32_t n) {
    rng_state = rng_state * UINT64_C(6364136223846793005) + UINT64_C(1442695040888963407);
    return n ? (uint32_t)(rng_state >> 33) % n : 0;
}

// Replacements with the same immediates, so that the code still decodes
static const uint16_t no_imm[] = { WAH_OP_I32_ADD, WAH_OP_I64_ADD, WAH_OP_F32_ADD, WAH_OP_F64_ADD, WAH_OP_I32_EQZ,
                                   WAH_OP_I64_EQZ, WAH_OP_I32_WRAP_I64, WAH_OP_I64_EXTEND_I32_U, WAH_OP_DROP,
                                   WAH_OP_SELECT, WAH_OP_RETURN, WAH_OP_UNREACHABLE, WAH_OP_END, WAH_OP_F32_DEMOTE_F64,
                                   WAH_OP_REF_IS_NULL, WAH_OP_REF_AS_NON_NULL, WAH_OP_REF_I31, WAH_OP_I31_GET_S,
                                   WAH_OP_REF_EQ, WAH_OP_ARRAY_LEN, WAH_OP_ARRAY_GET_32, WAH_OP_ARRAY_GET_64,
                                   WAH_OP_ARRAY_GET_U8, WAH_OP_ARRAY_GET_S16, WAH_OP_ARRAY_SET_8, WAH_OP_ARRAY_SET_32,
                                   WAH_OP_ARRAY_SET_64, WAH_OP_END_TRY_TABLE, WAH_OP_THROW_REF };
static const uint16_t one_imm[] = { WAH_OP_LOCAL_GET, WAH_OP_LOCAL_SET, WAH_OP_LOCAL_TEE, WAH_OP_I32_CONST,
                                    WAH_OP_F32_CONST, WAH_OP_GLOBAL_GET, WAH_OP_GLOBAL_SET, WAH_OP_CALL,
                                    WAH_OP_RETURN_CALL, WAH_OP_IF, WAH_OP_ELSE, WAH_OP_POLL, WAH_OP_TICK,
                                    WAH_OP_MEMORY_SIZE, WAH_OP_MEMORY_GROW, WAH_OP_REF_NULL, WAH_OP_REF_FUNC,
                                    WAH_OP_SELECT_T, WAH_OP_TABLE_GET, WAH_OP_TABLE_SET, WAH_OP_TABLE_SIZE,
                                    WAH_OP_TABLE_GROW, WAH_OP_CALL_REF, WAH_OP_STRUCT_GET_32, WAH_OP_STRUCT_GET_64,
                                    WAH_OP_STRUCT_GET_S8, WAH_OP_STRUCT_GET_U16, WAH_OP_STRUCT_SET_8,
                                    WAH_OP_STRUCT_SET_32, WAH_OP_STRUCT_SET_64, WAH_OP_REF_TEST, WAH_OP_REF_CAST,
                                    WAH_OP_REF_CAST_NULL, WAH_OP_STRUCT_NEW, WAH_OP_STRUCT_NEW_DEFAULT,
                                    WAH_OP_ARRAY_COPY, WAH_OP_TRIM_HANDLERS, WAH_OP_THROW };
static const uint16_t two_imm[] = { WAH_OP_ARRAY_NEW, WAH_OP_ARRAY_NEW_DEFAULT, WAH_OP_ARRAY_NEW_FIXED,
                                    WAH_OP_ARRAY_FILL, WAH_OP_ARRAY_INIT_ELEM };
static const uint16_t cast_imm[] = { WAH_OP_BR_ON_CAST, WAH_OP_BR_ON_CAST_FAIL };
static const uint16_t three_imm[] = { WAH_OP_BR, WAH_OP_BR_IF, WAH_OP_BR_ON_NULL, WAH_OP_BR_ON_NON_NULL };

// A type, often one of the module
static int32_t random_type(void) {
    static const int32_t ts[] = { WAH_TYPE_I32, WAH_TYPE_I64, WAH_TYPE_FUNCREF, WAH_TYPE_EXTERNREF, WAH_TYPE_ANYREF,
                                  WAH_TYPE_FUNC, WAH_TYPE_NULLREF, WAH_TYPE_NONE, WAH_TYPE_STRUCT, WAH_TYPE_EQ,
                                  WAH_TYPE_I31, 0, 1, 2, 3, 4, 5 };
    if (cur->type_count > 0 && rng(2)) return WAH_TYPE_FROM_IDX(rng(cur->type_count), rng(2));
    return ts[rng(sizeof(ts) / sizeof(*ts))];
}

// An offset of a field of some struct type of the module, or a nearby one
static uint32_t random_offset(uint32_t v) {
    static const int32_t ds[] = { 1, -1, 2, -2, 4, -4, 8, -8 };
    uint32_t t = cur->type_count ? rng(cur->type_count) : 0;
    if (t < cur->type_count && cur->type_defs[t].kind == WAH_COMP_STRUCT && rng(4)) {
        const wah_repr_info_t *ri = wah_repr_info_get(cur, wah_module_typeidx_to_repr(cur, t));
        if (ri && ri->count) return ri->fields[rng(ri->count)].offset;
    }
    return v + (uint32_t)ds[rng(8)];
}

// Opcodes that differ only in how they treat types, so that swapping them often keeps code valid but for one rule
static const uint16_t siblings[][16] = {
    { WAH_OP_REF_CAST, WAH_OP_REF_CAST_NULL, WAH_OP_REF_TEST, WAH_OP_REF_TEST_NULL },
    { WAH_OP_BR_ON_CAST, WAH_OP_BR_ON_CAST_FAIL }, { WAH_OP_BR_ON_NULL, WAH_OP_BR_ON_NON_NULL },
    { WAH_OP_STRUCT_GET_S8, WAH_OP_STRUCT_GET_S16, WAH_OP_STRUCT_GET_U8, WAH_OP_STRUCT_GET_U16, WAH_OP_STRUCT_GET_32,
      WAH_OP_STRUCT_GET_64, WAH_OP_STRUCT_GET_128 },
    { WAH_OP_STRUCT_SET_8, WAH_OP_STRUCT_SET_16, WAH_OP_STRUCT_SET_32, WAH_OP_STRUCT_SET_64, WAH_OP_STRUCT_SET_128 },
    { WAH_OP_ARRAY_GET_S8, WAH_OP_ARRAY_GET_S16, WAH_OP_ARRAY_GET_U8, WAH_OP_ARRAY_GET_U16, WAH_OP_ARRAY_GET_32,
      WAH_OP_ARRAY_GET_64, WAH_OP_ARRAY_GET_128 },
    { WAH_OP_ARRAY_SET_8, WAH_OP_ARRAY_SET_16, WAH_OP_ARRAY_SET_32, WAH_OP_ARRAY_SET_64, WAH_OP_ARRAY_SET_128 },
    { WAH_OP_STRUCT_NEW, WAH_OP_STRUCT_NEW_DEFAULT }, { WAH_OP_ARRAY_NEW, WAH_OP_ARRAY_NEW_DEFAULT },
    { WAH_OP_ARRAY_NEW_DATA, WAH_OP_ARRAY_NEW_ELEM },
    { WAH_OP_I8X16_EXTRACT_LANE_S, WAH_OP_I8X16_EXTRACT_LANE_U, WAH_OP_I16X8_EXTRACT_LANE_S,
      WAH_OP_I16X8_EXTRACT_LANE_U, WAH_OP_I32X4_EXTRACT_LANE, WAH_OP_I64X2_EXTRACT_LANE, WAH_OP_F32X4_EXTRACT_LANE,
      WAH_OP_F64X2_EXTRACT_LANE, WAH_OP_I8X16_REPLACE_LANE, WAH_OP_I16X8_REPLACE_LANE, WAH_OP_I32X4_REPLACE_LANE,
      WAH_OP_I64X2_REPLACE_LANE, WAH_OP_F32X4_REPLACE_LANE, WAH_OP_F64X2_REPLACE_LANE },
    { WAH_OP_V128_LOAD8_LANE_i32_mem0, WAH_OP_V128_LOAD16_LANE_i32_mem0, WAH_OP_V128_LOAD32_LANE_i32_mem0,
      WAH_OP_V128_LOAD64_LANE_i32_mem0, WAH_OP_V128_STORE8_LANE_i32_mem0, WAH_OP_V128_STORE16_LANE_i32_mem0,
      WAH_OP_V128_STORE32_LANE_i32_mem0, WAH_OP_V128_STORE64_LANE_i32_mem0 },
    { WAH_OP_V128_LOAD_i32_mem0, WAH_OP_V128_STORE_i32_mem0, WAH_OP_V128_LOAD8_SPLAT_i32_mem0,
      WAH_OP_V128_LOAD64_ZERO_i32_mem0, WAH_OP_I32_LOAD_i32_mem0, WAH_OP_I64_STORE_i32_mem0 },
};

static bool in_list(uint16_t op, const uint16_t *list, size_t n) {
    for (size_t i = 0; i < n; i++) if (list[i] == op) return true;
    return false;
}

// Mutates one instruction of `code`, returning false if it found nothing to mutate
static bool mutate(uint8_t *code, uint32_t size, const uint32_t *starts, uint32_t count) {
    wah_check_instr_t in;
    for (int tries = 0; tries < 16; tries++) {
        uint32_t off = starts[rng(count)];
        if (wah_check_decode(code, size, off, &in) != WAH_OK) return false;
        uint8_t *imm = code + off + sizeof(uint16_t);
        uint32_t random_target = rng(4) ? starts[rng(count)] : rng(size);
        switch (rng(3)) {
            case 0: // Immediates
                switch (in.op) {
                    case WAH_OP_BR: case WAH_OP_BR_IF: case WAH_OP_BR_ON_NULL: case WAH_OP_BR_ON_NON_NULL: {
                        uint32_t which = rng(3), v = wah_read_u32_le(imm + which * 4);
                        v = which == 0 ? random_target : rng(2) ? v + 1 : v - 1;
                        memcpy(imm + which * 4, &v, 4);
                        return true;
                    }
                    case WAH_OP_IF: case WAH_OP_ELSE: memcpy(imm, &random_target, 4); return true;
                    case WAH_OP_METER: memcpy(imm + 2, &random_target, 4); return true;
                    case WAH_OP_BR_TABLE: {
                        uint8_t *rec = imm + 8 + ((size_t)in.a + 1) * 2 + (size_t)rng(in.c) * 8;
                        uint32_t v;
                        if (rng(2)) { memcpy(rec, &random_target, 4); return true; }
                        if (rng(2)) { memcpy(&v, rec + 4, 4); v += rng(2) ? 1 : -1; memcpy(rec + 4, &v, 4); return true; }
                        memcpy(&v, imm + 4, 4); v += rng(2) ? 1 : -1; memcpy(imm + 4, &v, 4); // Keep
                        return true;
                    }
                    case WAH_OP_LOCAL_GET: case WAH_OP_LOCAL_SET: case WAH_OP_LOCAL_TEE: case WAH_OP_GLOBAL_GET:
                    case WAH_OP_GLOBAL_SET: case WAH_OP_CALL: case WAH_OP_POLL: case WAH_OP_TICK: case WAH_OP_REF_FUNC:
                    case WAH_OP_TABLE_GET: case WAH_OP_TABLE_SET: case WAH_OP_CALL_REF: {
                        uint32_t v = wah_read_u32_le(imm) + (rng(2) ? 1 : -1);
                        memcpy(imm, &v, 4);
                        return true;
                    }
                    case WAH_OP_REF_NULL: case WAH_OP_SELECT_T: case WAH_OP_REF_TEST: case WAH_OP_REF_CAST:
                    case WAH_OP_REF_CAST_NULL: { // Types
                        int32_t v = random_type();
                        memcpy(imm, &v, 4);
                        return true;
                    }
                    case WAH_OP_BR_ON_CAST: case WAH_OP_BR_ON_CAST_FAIL: { // Flags, target, type, keep, drop
                        uint32_t which = rng(5), v;
                        if (which == 0) { imm[0] ^= (uint8_t)(1 + rng(2)); return true; }
                        memcpy(&v, imm + 1 + (which - 1) * 4, 4);
                        v = which == 1 ? random_target : which == 2 ? (uint32_t)random_type() : rng(2) ? v + 1 : v - 1;
                        memcpy(imm + 1 + (which - 1) * 4, &v, 4);
                        return true;
                    }
                    case WAH_OP_STRUCT_GET_S8: case WAH_OP_STRUCT_GET_S16: case WAH_OP_STRUCT_GET_U8:
                    case WAH_OP_STRUCT_GET_U16: case WAH_OP_STRUCT_GET_32: case WAH_OP_STRUCT_GET_64:
                    case WAH_OP_STRUCT_GET_128: case WAH_OP_STRUCT_SET_8: case WAH_OP_STRUCT_SET_16:
                    case WAH_OP_STRUCT_SET_32: case WAH_OP_STRUCT_SET_64: case WAH_OP_STRUCT_SET_128: { // Offsets
                        uint32_t v = random_offset(wah_read_u32_le(imm));
                        memcpy(imm, &v, 4);
                        return true;
                    }
                    case WAH_OP_MEMORY_FILL: case WAH_OP_TABLE_FILL: { // Ref maps
                        uint32_t v = wah_read_u32_le(imm + 4) + (rng(2) ? 1 : -1);
                        memcpy(imm + 4, &v, 4);
                        return true;
                    }
                    case WAH_OP_TRY_TABLE: { // Kind, tag, target or drop of a clause
                        if (in.a == 0) break;
                        uint8_t *c = imm + 4 + (size_t)rng(in.a) * 13;
                        uint32_t which = rng(4), v;
                        if (which == 0) { c[0] = (uint8_t)rng(5); return true; }
                        memcpy(&v, c + 1 + (which - 1) * 4, 4);
                        v = which == 2 ? random_target : rng(2) ? v + 1 : v - 1;
                        memcpy(c + 1 + (which - 1) * 4, &v, 4);
                        return true;
                    }
                    case WAH_OP_I8X16_SHUFFLE: imm[rng(16)] = (uint8_t)(rng(2) ? 31 + rng(2) : rng(256)); return true;
                    case WAH_OP_I8X16_EXTRACT_LANE_S: case WAH_OP_I8X16_EXTRACT_LANE_U: case WAH_OP_I16X8_EXTRACT_LANE_S:
                    case WAH_OP_I16X8_EXTRACT_LANE_U: case WAH_OP_I32X4_EXTRACT_LANE: case WAH_OP_I64X2_EXTRACT_LANE:
                    case WAH_OP_F32X4_EXTRACT_LANE: case WAH_OP_F64X2_EXTRACT_LANE: case WAH_OP_I8X16_REPLACE_LANE:
                    case WAH_OP_I16X8_REPLACE_LANE: case WAH_OP_I32X4_REPLACE_LANE: case WAH_OP_I64X2_REPLACE_LANE:
                    case WAH_OP_F32X4_REPLACE_LANE: case WAH_OP_F64X2_REPLACE_LANE: // Lanes
                        imm[0] = (uint8_t)(rng(4) ? rng(17) : rng(256));
                        return true;
                    case WAH_OP_TRIM_HANDLERS: case WAH_OP_THROW: {
                        uint32_t v = wah_read_u32_le(imm) + (rng(2) ? 1 : -1);
                        memcpy(imm, &v, 4);
                        return true;
                    }
                    case WAH_OP_STRUCT_NEW: case WAH_OP_STRUCT_NEW_DEFAULT: case WAH_OP_ARRAY_COPY: // Types, ref maps
                    case WAH_OP_ARRAY_NEW: case WAH_OP_ARRAY_NEW_DEFAULT: case WAH_OP_ARRAY_NEW_FIXED: // and segments
                    case WAH_OP_ARRAY_FILL: case WAH_OP_ARRAY_INIT_ELEM: case WAH_OP_ARRAY_NEW_DATA:
                    case WAH_OP_ARRAY_NEW_ELEM: case WAH_OP_ARRAY_INIT_DATA: {
                        uint32_t n = (in.len - 2) / 4, which = rng(n), v = wah_read_u32_le(imm + which * 4);
                        bool type = which == 0 && in.op != WAH_OP_ARRAY_COPY && in.op != WAH_OP_ARRAY_INIT_ELEM;
                        v = type && cur->type_count && rng(2) ? rng(cur->type_count) : rng(2) ? v + 1 : v - 1;
                        memcpy(imm + which * 4, &v, 4);
                        return true;
                    }
                    default:
                        if (in.mem != WAH_CHECK_MEM_NONE && wah_opclasses[in.base] == WAH_OPCLASS_MB) { // Lanes
                            code[off + in.len - 1] = (uint8_t)(rng(4) ? rng(17) : rng(256));
                            return true;
                        }
                        break;
                }
                break;
            default: { // Opcodes
                const uint16_t *list = NULL;
                size_t n = 0;
                for (size_t g = 0; g < sizeof(siblings) / sizeof(*siblings); g++) {
                    for (n = 0; n < 16 && siblings[g][n]; n++) {}
                    if (!in_list(in.op, siblings[g], n) || rng(2)) continue;
                    uint16_t op = siblings[g][rng((uint32_t)n)];
                    memcpy(code + off, &op, 2);
                    return true;
                }
                n = 0;
                if (in_list(in.op, no_imm, sizeof(no_imm) / 2) || (in.op < WAH_FE && in.mem == WAH_CHECK_MEM_NONE &&
                    in.len == 2 && wah_opclasses[in.op] >= WAH_OPCLASS__I_I && wah_opclasses[in.op] <= WAH_OPCLASS__VVV_V)) {
                    list = no_imm, n = sizeof(no_imm) / 2;
                } else if (in_list(in.op, one_imm, sizeof(one_imm) / 2)) {
                    list = one_imm, n = sizeof(one_imm) / 2;
                } else if (in_list(in.op, three_imm, sizeof(three_imm) / 2)) {
                    list = three_imm, n = sizeof(three_imm) / 2;
                } else if (in_list(in.op, two_imm, sizeof(two_imm) / 2)) {
                    list = two_imm, n = sizeof(two_imm) / 2;
                } else if (in_list(in.op, cast_imm, sizeof(cast_imm) / 2)) {
                    list = cast_imm, n = sizeof(cast_imm) / 2;
                }
                if (!list) break;
                uint16_t op = list[rng((uint32_t)n)];
                memcpy(code + off, &op, 2);
                return true;
            }
        }
    }
    return false;
}

// GC, exception and lane instructions for RARE_MUTANTS
static bool is_rare(const wah_check_instr_t *in) {
    if (in->mem != WAH_CHECK_MEM_NONE) return wah_opclasses[in->base] == WAH_OPCLASS_MB;
    switch (in->op) {
        case WAH_OP_TRY_TABLE: case WAH_OP_END_TRY_TABLE: case WAH_OP_TRIM_HANDLERS: case WAH_OP_THROW:
        case WAH_OP_THROW_REF:
        case WAH_OP_REF_EQ: case WAH_OP_REF_TEST: case WAH_OP_REF_TEST_NULL: case WAH_OP_REF_CAST:
        case WAH_OP_REF_CAST_NULL: case WAH_OP_BR_ON_CAST: case WAH_OP_BR_ON_CAST_FAIL: case WAH_OP_STRUCT_NEW:
        case WAH_OP_STRUCT_NEW_DEFAULT: case WAH_OP_ARRAY_NEW: case WAH_OP_ARRAY_NEW_DEFAULT: case WAH_OP_ARRAY_NEW_FIXED:
        case WAH_OP_ARRAY_NEW_DATA: case WAH_OP_ARRAY_NEW_ELEM: case WAH_OP_ARRAY_FILL: case WAH_OP_ARRAY_INIT_DATA:
        case WAH_OP_ARRAY_COPY: case WAH_OP_ARRAY_INIT_ELEM: case WAH_OP_ARRAY_LEN:
        case WAH_OP_STRUCT_GET_S8: case WAH_OP_STRUCT_GET_S16: case WAH_OP_STRUCT_GET_U8: case WAH_OP_STRUCT_GET_U16:
        case WAH_OP_STRUCT_GET_32: case WAH_OP_STRUCT_GET_64: case WAH_OP_STRUCT_GET_128: case WAH_OP_STRUCT_SET_8:
        case WAH_OP_STRUCT_SET_16: case WAH_OP_STRUCT_SET_32: case WAH_OP_STRUCT_SET_64: case WAH_OP_STRUCT_SET_128:
        case WAH_OP_ARRAY_GET_S8: case WAH_OP_ARRAY_GET_S16: case WAH_OP_ARRAY_GET_U8: case WAH_OP_ARRAY_GET_U16:
        case WAH_OP_ARRAY_GET_32: case WAH_OP_ARRAY_GET_64: case WAH_OP_ARRAY_GET_128: case WAH_OP_ARRAY_SET_8:
        case WAH_OP_ARRAY_SET_16: case WAH_OP_ARRAY_SET_32: case WAH_OP_ARRAY_SET_64: case WAH_OP_ARRAY_SET_128:
        case WAH_OP_I8X16_SHUFFLE: case WAH_OP_I8X16_EXTRACT_LANE_S: case WAH_OP_I8X16_EXTRACT_LANE_U:
        case WAH_OP_I16X8_EXTRACT_LANE_S: case WAH_OP_I16X8_EXTRACT_LANE_U: case WAH_OP_I32X4_EXTRACT_LANE:
        case WAH_OP_I64X2_EXTRACT_LANE: case WAH_OP_F32X4_EXTRACT_LANE: case WAH_OP_F64X2_EXTRACT_LANE:
        case WAH_OP_I8X16_REPLACE_LANE: case WAH_OP_I16X8_REPLACE_LANE: case WAH_OP_I32X4_REPLACE_LANE:
        case WAH_OP_I64X2_REPLACE_LANE: case WAH_OP_F32X4_REPLACE_LANE: case WAH_OP_F64X2_REPLACE_LANE:
            return true;
        default: return false;
    }
}

static uint64_t total, dumped, mutants;

static void dump_function(uint32_t i) {
    const wah_code_body_t *body = &cur->code_bodies[i];
    const wah_func_type_t *ft = &cur->types[cur->function_type_indices[i]];
    const wah_parsed_code_t *pc = &body->parsed_code;
    buf_len = 0;
    total++;
    if (!dump_code(ft, body, pc)) return;
    fwrite(buf, 1, buf_len, stdout);
    dumped++;

    uint32_t size = pc->bytecode_size, count = instr_count, rare_count = 0;
    uint32_t *starts = malloc((size_t)count * sizeof(uint32_t)), *rare_starts = malloc((size_t)count * sizeof(uint32_t));
    uint8_t *copy = malloc(size);
    if (!starts || !rare_starts || !copy) abort();
    for (uint32_t off = 0, k = 0; off < size; k++) {
        starts[k] = off;
        wah_check_instr_t in;
        wah_check_decode(pc->bytecode, size, off, &in);
        if (is_rare(&in)) rare_starts[rare_count++] = off;
        off += in.len;
    }
    wah_ref_node_t *nodes = malloc((size_t)pc->ref_node_count * sizeof(wah_ref_node_t) + 1);
    if (!nodes) abort();
    rng_state = (uint64_t)size * 31 + i;
    for (int k = 0; k < (MUTANTS + (rare_count ? RARE_MUTANTS : 0)) * mutant_scale; k++) {
        bool rare = k >= MUTANTS * mutant_scale;
        wah_parsed_code_t mpc = *pc;
        memcpy(copy, pc->bytecode, size);
        if (pc->ref_node_count > 0 && !rare && rng(4) == 0) { // A slot of a ref map
            memcpy(nodes, pc->ref_nodes, (size_t)pc->ref_node_count * sizeof(wah_ref_node_t));
            nodes[rng(pc->ref_node_count)].pos += rng(2) ? 1 : -1;
            mpc.ref_nodes = nodes;
        } else if (!mutate(copy, size, rare ? rare_starts : starts, rare ? rare_count : count)) {
            continue;
        }
        mpc.bytecode = copy;
        buf_len = 0;
        if (!dump_code(ft, body, &mpc)) continue;
        fwrite(buf, 1, buf_len, stdout);
        mutants++;
    }
    free(starts);
    free(rare_starts);
    free(copy);
    free(nodes);
}

static void dump_binary(const uint8_t *bytes, size_t len) {
    static const uint8_t empty = 0;
    for (int metered = 0; metered < 2; metered++) {
        wah_module_t m = {0};
        wah_parse_options_t options = { .enable_fuel_metering = metered };
        if (wah_parse_module(&m, len ? bytes : &empty, len, &options) == WAH_OK) {
            cur = &m;
            for (uint32_t i = 0; i < m.code_count; i++) dump_function(i);
        }
        wah_free_module(&m);
    }
}

int main(int argc, char **argv) {
    for (int a = 1; a < argc; ++a) {
        if (strcmp(argv[a], "-m") == 0 && a + 1 < argc) { mutant_scale = atoi(argv[++a]); continue; }
        size_t size;
        char *data = wast_read_file(argv[a], &size);
        if (!data) { fprintf(stderr, "cannot read %s\n", argv[a]); return 1; }
        size_t n = strlen(argv[a]);
        if (n < 5 || strcmp(argv[a] + n - 5, ".wast") != 0) {
            dump_binary((const uint8_t *)data, size);
            free(data);
            continue;
        }
        wast_parser_t parser;
        wast_parser_init(&parser, data, size);
        wast_node_t *node;
        while ((node = wast_parse_next(&parser)) != NULL) {
            uint8_t *bytes = NULL;
            size_t len = 0;
            if (!node->is_atom && node->child_count >= 2 && wast_atom_eq(node->children[0], "module") &&
                wast_gather_binary_module(node, &bytes, &len)) {
                dump_binary(bytes, len);
                free(bytes);
            }
            wast_node_free(node);
        }
        free(data);
    }
    fprintf(stderr, "dump_lowered: %" PRIu64 " of %" PRIu64 " functions in the model, %" PRIu64 " mutants\n", dumped,
            total, mutants);
    return 0;
}
