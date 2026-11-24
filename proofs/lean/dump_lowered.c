// Dumps lowered functions of the modules in .wast files (or .wasm files) for wah_lowered_oracle, which checks the
// verdicts of wah_check_lowered_code against the model in WahProofs/Lowered.lean (`make lean-diff`). Only functions
// whose types and instructions are all in the model are dumped, and each also with mutants of its lowered code (a
// branch target, keep or drop, an immediate, or an opcode replaced by one with the same immediates), each as
//   F <max depth> <local count> <local>* <result count> <result>* <verdict: 1 if wah_check_lowered_code accepts it>
//   <one line per instruction, see Oracle of Lowered.lean>
//   E
// with value types encoded as 0 = i32, 1 = i64, 2 = f32, 3 = f64, 4 = v128, and branch targets as instruction indices
// (offsets that are not instruction starts become indices beyond the end). Module-level facts, such as signatures of
// callees and whether memory or global indices are valid, are resolved here, and instructions that the checker
// rejects for them become `X` (bad). Modules are parsed with and without fuel metering, to get METERs too.
#include <inttypes.h>
#include <stdint.h>
#define WAH_CHECK_LOWERED
#define WAH_IMPLEMENTATION
#include "../../wah.h"
#include "../../tests/wast.c"

#define MUTANTS 8
#define MAX_LOCALS 4096

static int vt(wah_type_t t) {
    switch (t) {
        case WAH_TYPE_I32: return 0;
        case WAH_TYPE_I64: return 1;
        case WAH_TYPE_F32: return 2;
        case WAH_TYPE_F64: return 3;
        case WAH_TYPE_V128: return 4;
        default: return -1;
    }
}

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

// Types popped (bottom first) and pushed by an instruction
typedef struct { int n, t[8]; } types_t;
static types_t T0(void) { return (types_t){0}; }
static types_t T1(int a) { return (types_t){1, {a}}; }
static types_t T2(int a, int b) { return (types_t){2, {a, b}}; }
static types_t T3(int a, int b, int c) { return (types_t){3, {a, b, c}}; }

static bool opclass_sig(uint8_t cls, types_t *a, types_t *r) {
    enum { I, L, F, D, V };
    switch (cls) {
        case WAH_OPCLASS__I_I: *a = T1(I), *r = T1(I); return true;
        case WAH_OPCLASS__I_L: *a = T1(I), *r = T1(L); return true;
        case WAH_OPCLASS__I_F: *a = T1(I), *r = T1(F); return true;
        case WAH_OPCLASS__I_D: *a = T1(I), *r = T1(D); return true;
        case WAH_OPCLASS__I_V: *a = T1(I), *r = T1(V); return true;
        case WAH_OPCLASS__II_I: *a = T2(I, I), *r = T1(I); return true;
        case WAH_OPCLASS__IV_V: *a = T2(I, V), *r = T1(V); return true;
        case WAH_OPCLASS__L_I: *a = T1(L), *r = T1(I); return true;
        case WAH_OPCLASS__L_L: *a = T1(L), *r = T1(L); return true;
        case WAH_OPCLASS__L_F: *a = T1(L), *r = T1(F); return true;
        case WAH_OPCLASS__L_D: *a = T1(L), *r = T1(D); return true;
        case WAH_OPCLASS__L_V: *a = T1(L), *r = T1(V); return true;
        case WAH_OPCLASS__LL_I: *a = T2(L, L), *r = T1(I); return true;
        case WAH_OPCLASS__LL_L: *a = T2(L, L), *r = T1(L); return true;
        case WAH_OPCLASS__LV_V: *a = T2(L, V), *r = T1(V); return true;
        case WAH_OPCLASS__F_I: *a = T1(F), *r = T1(I); return true;
        case WAH_OPCLASS__F_L: *a = T1(F), *r = T1(L); return true;
        case WAH_OPCLASS__F_F: *a = T1(F), *r = T1(F); return true;
        case WAH_OPCLASS__F_V: *a = T1(F), *r = T1(V); return true;
        case WAH_OPCLASS__FF_I: *a = T2(F, F), *r = T1(I); return true;
        case WAH_OPCLASS__FF_F: *a = T2(F, F), *r = T1(F); return true;
        case WAH_OPCLASS__D_I: *a = T1(D), *r = T1(I); return true;
        case WAH_OPCLASS__D_L: *a = T1(D), *r = T1(L); return true;
        case WAH_OPCLASS__D_D: *a = T1(D), *r = T1(D); return true;
        case WAH_OPCLASS__D_V: *a = T1(D), *r = T1(V); return true;
        case WAH_OPCLASS__DD_I: *a = T2(D, D), *r = T1(I); return true;
        case WAH_OPCLASS__DD_L: *a = T2(D, D), *r = T1(L); return true;
        case WAH_OPCLASS__DD_D: *a = T2(D, D), *r = T1(D); return true;
        case WAH_OPCLASS__V_I: *a = T1(V), *r = T1(I); return true;
        case WAH_OPCLASS__V_V: *a = T1(V), *r = T1(V); return true;
        case WAH_OPCLASS__VI_V: *a = T2(V, I), *r = T1(V); return true;
        case WAH_OPCLASS__VV_V: *a = T2(V, V), *r = T1(V); return true;
        case WAH_OPCLASS__VVV_V: *a = T3(V, V, V), *r = T1(V); return true;
        default: return false;
    }
}

// Scalar loads and stores: the type loaded (or -1 for stores) and the type stored (or -1 for loads)
static bool mem_sig(uint16_t base, int *load, int *store) {
    *load = *store = -1;
    switch (base) {
        case WAH_OP_I32_LOAD: case WAH_OP_I32_LOAD8_S: case WAH_OP_I32_LOAD8_U: case WAH_OP_I32_LOAD16_S:
        case WAH_OP_I32_LOAD16_U: *load = 0; return true;
        case WAH_OP_I64_LOAD: case WAH_OP_I64_LOAD8_S: case WAH_OP_I64_LOAD8_U: case WAH_OP_I64_LOAD16_S:
        case WAH_OP_I64_LOAD16_U: case WAH_OP_I64_LOAD32_S: case WAH_OP_I64_LOAD32_U: *load = 1; return true;
        case WAH_OP_F32_LOAD: *load = 2; return true;
        case WAH_OP_F64_LOAD: *load = 3; return true;
        case WAH_OP_I32_STORE: case WAH_OP_I32_STORE8: case WAH_OP_I32_STORE16: *store = 0; return true;
        case WAH_OP_I64_STORE: case WAH_OP_I64_STORE8: case WAH_OP_I64_STORE16: case WAH_OP_I64_STORE32: *store = 1; return true;
        case WAH_OP_F32_STORE: *store = 2; return true;
        case WAH_OP_F64_STORE: *store = 3; return true;
        default: return false;
    }
}

static void put_types(types_t t) {
    put(" %d", t.n);
    for (int i = 0; i < t.n; i++) put(" %d", t.t[i]);
}

static bool put_func_types(const wah_func_type_t *ft, int extra) {
    put(" %" PRIu32, ft->param_count + (extra >= 0));
    for (uint32_t i = 0; i < ft->param_count; i++) {
        int t = vt(ft->param_types[i]);
        if (t < 0) return false;
        put(" %d", t);
    }
    if (extra >= 0) put(" %d", extra);
    put(" %" PRIu32, ft->result_count);
    for (uint32_t i = 0; i < ft->result_count; i++) {
        int t = vt(ft->result_types[i]);
        if (t < 0) return false;
        put(" %d", t);
    }
    return true;
}

// Instruction indices of offsets, or beyond the end for others
static uint32_t *index_of;
static uint32_t index_cap, instr_count;
static uint64_t target(uint32_t off, uint32_t size) {
    return off < size && index_of[off] != UINT32_MAX ? index_of[off] : (uint64_t)instr_count + 1 + off;
}

// Dumps code (which may be a mutant) of a function, or returns false if it isn't in the model
static bool dump_code(const wah_module_t *m, const wah_func_type_t *ft, const wah_code_body_t *body,
                      const wah_parsed_code_t *pc) {
    const uint8_t *code = pc->bytecode;
    uint32_t size = pc->bytecode_size;
    wah_check_instr_t in;
    if (pc->stack_map_count > 0) return false; // Only at labels with references
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
    bool ok = wah_check_lowered_code(m, pc, ft, body, body->max_stack_depth, WAH_TYPE_BOT) == WAH_OK;
    if (ft->param_count + body->local_count > MAX_LOCALS) return false;
    put("F %" PRIu32 " %" PRIu32, body->max_stack_depth, ft->param_count + body->local_count);
    for (uint32_t i = 0; i < ft->param_count + body->local_count; i++) {
        int t = vt(i < ft->param_count ? ft->param_types[i] : wah_local_type(body, i - ft->param_count));
        if (t < 0) return false;
        put(" %d", t);
    }
    put(" %" PRIu32, ft->result_count);
    for (uint32_t i = 0; i < ft->result_count; i++) {
        int t = vt(ft->result_types[i]);
        if (t < 0) return false;
        put(" %d", t);
    }
    put(" %d\n", ok);
    for (uint32_t off = 0; off < size; off += in.len) {
        wah_check_decode(code, size, off, &in);
        types_t a, r;
        int load, store;
        switch (in.op) {
            case WAH_OP_I32_CONST: put("c 0\n"); break;
            case WAH_OP_I64_CONST: put("c 1\n"); break;
            case WAH_OP_F32_CONST: put("c 2\n"); break;
            case WAH_OP_F64_CONST: put("c 3\n"); break;
            case WAH_OP_V128_CONST: put("c 4\n"); break;
            case WAH_OP_F32_DEMOTE_F64: put("o 1 3 1 2\n"); break;
            case WAH_OP_F64_PROMOTE_F32: put("o 1 2 1 3\n"); break;
            case WAH_OP_DROP: put("d\n"); break;
            case WAH_OP_SELECT: put("s\n"); break;
            case WAH_OP_SELECT_T: {
                int t = vt((wah_type_t)(int32_t)in.a);
                if (t < 0) return false;
                put("o 3 %d %d 0 1 %d\n", t, t, t);
                break;
            }
            case WAH_OP_LOCAL_GET: put("g %" PRIu32 "\n", in.a); break;
            case WAH_OP_LOCAL_SET: put("x %" PRIu32 "\n", in.a); break;
            case WAH_OP_LOCAL_TEE: put("t %" PRIu32 "\n", in.a); break;
            case WAH_OP_GLOBAL_GET: case WAH_OP_GLOBAL_SET: case WAH_OP_GLOBAL_GET_INDIRECT: case WAH_OP_GLOBAL_SET_INDIRECT: {
                if (in.a >= wah_global_index_limit(m)) { put("X\n"); break; }
                bool indirect = in.a < m->import_global_count && m->global_imports[in.a].is_mutable;
                bool get = in.op == WAH_OP_GLOBAL_GET || in.op == WAH_OP_GLOBAL_GET_INDIRECT;
                int t = vt(wah_global_type(m, in.a));
                if (t < 0) return false;
                if (indirect != (in.op == WAH_OP_GLOBAL_GET_INDIRECT || in.op == WAH_OP_GLOBAL_SET_INDIRECT) ||
                    (!get && !wah_global_is_mutable(m, in.a))) put("X\n");
                else if (get) put("o 0 1 %d\n", t);
                else put("o 1 %d 0\n", t);
                break;
            }
            case WAH_OP_MEMORY_SIZE: case WAH_OP_MEMORY_GROW: case WAH_OP_MEMORY_SIZE_i64: case WAH_OP_MEMORY_GROW_i64: {
                int at = in.op == WAH_OP_MEMORY_SIZE || in.op == WAH_OP_MEMORY_GROW ? 0 : 1;
                if (in.a >= wah_memory_index_limit(m) || vt(wah_memory_type(m, in.a)->addr_type) != at) put("X\n");
                else if (in.op == WAH_OP_MEMORY_SIZE || in.op == WAH_OP_MEMORY_SIZE_i64) put("o 0 1 %d\n", at);
                else put("o 1 %d 1 %d\n", at, at);
                break;
            }
            case WAH_OP_CALL: case WAH_OP_RETURN_CALL:
                if (in.a >= wah_func_index_limit(m)) { put("X\n"); break; }
                put(in.op == WAH_OP_CALL ? "k" : "r");
                if (!put_func_types(wah_resolve_func_type(m, in.a), -1)) return false;
                put("\n");
                break;
            case WAH_OP_CALL_INDIRECT: case WAH_OP_CALL_INDIRECT_i64:
            case WAH_OP_RETURN_CALL_INDIRECT: case WAH_OP_RETURN_CALL_INDIRECT_i64: {
                bool tail = in.op == WAH_OP_RETURN_CALL_INDIRECT || in.op == WAH_OP_RETURN_CALL_INDIRECT_i64;
                int at = in.op == WAH_OP_CALL_INDIRECT || in.op == WAH_OP_RETURN_CALL_INDIRECT ? 0 : 1;
                if (in.a >= m->type_count || m->type_defs[in.a].kind != WAH_COMP_FUNC || in.b >= wah_table_index_limit(m)) {
                    put("X\n");
                    break;
                }
                const wah_table_type_t *tt = wah_table_type(m, in.b);
                if (!wah_type_is_subtype(tt->elem_type, WAH_TYPE_FUNCREF, m) || vt(tt->addr_type) != at) {
                    put("X\n");
                    break;
                }
                put(tail ? "r" : "k");
                if (!put_func_types(&m->types[in.a], at)) return false;
                put("\n");
                break;
            }
            case WAH_OP_BR: put("b %" PRIu64 " %" PRIu32 " %" PRIu32 "\n", target(in.a, size), in.b, in.c); break;
            case WAH_OP_BR_IF: put("B %" PRIu64 " %" PRIu32 " %" PRIu32 "\n", target(in.a, size), in.b, in.c); break;
            case WAH_OP_BR_TABLE: {
                const uint8_t *records = code + off + sizeof(uint16_t) + 2 * sizeof(uint32_t) + ((size_t)in.a + 1) * sizeof(uint16_t);
                put("T %" PRIu32 " %" PRIu32, in.b, in.c);
                for (uint32_t i = 0; i < in.c; i++) {
                    put(" %" PRIu64 " %" PRIu32, target(wah_read_u32_le(records + (size_t)i * 8), size),
                        wah_read_u32_le(records + (size_t)i * 8 + 4));
                }
                put("\n");
                break;
            }
            case WAH_OP_IF: put("i %" PRIu64 "\n", target(in.a, size)); break;
            case WAH_OP_ELSE: put("e %" PRIu64 "\n", target(in.a, size)); break;
            case WAH_OP_METER: put("m %" PRIu64 "\n", target(in.b, size)); break;
            // Without references on the stack, ref maps are exact if they are empty
            case WAH_OP_POLL: case WAH_OP_TICK:
                put("%c %d\n", in.op == WAH_OP_POLL ? 'p' : 'q', in.a >= pc->poll_count || pc->poll_ref_tops[in.a] == 0);
                break;
            case WAH_OP_END: put("n\n"); break;
            case WAH_OP_RETURN: put("R\n"); break;
            case WAH_OP_UNREACHABLE: put("u\n"); break;
            default:
                if (in.mem != WAH_CHECK_MEM_NONE) {
                    if (!mem_sig(in.base, &load, &store)) return false;
                    bool mem0 = in.mem == WAH_CHECK_MEM_I32_MEM0 || in.mem == WAH_CHECK_MEM_I64_MEM0;
                    int at = in.mem == WAH_CHECK_MEM_I32 || in.mem == WAH_CHECK_MEM_I32_MEM0 ? 0 : 1;
                    uint32_t memidx = mem0 ? 0 : in.a;
                    if (memidx >= wah_memory_index_limit(m) || vt(wah_memory_type(m, memidx)->addr_type) != at) put("X\n");
                    else if (load >= 0) put("o 1 %d 1 %d\n", at, load);
                    else put("o 2 %d %d 0\n", at, store);
                    break;
                }
                if (in.op >= WAH_FE || !opclass_sig(wah_opclasses[in.op], &a, &r)) return false;
                put("o"), put_types(a), put_types(r), put("\n");
        }
    }
    put("E\n");
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
                                   WAH_OP_SELECT, WAH_OP_RETURN, WAH_OP_UNREACHABLE, WAH_OP_END, WAH_OP_F32_DEMOTE_F64 };
static const uint16_t one_imm[] = { WAH_OP_LOCAL_GET, WAH_OP_LOCAL_SET, WAH_OP_LOCAL_TEE, WAH_OP_I32_CONST,
                                    WAH_OP_F32_CONST, WAH_OP_GLOBAL_GET, WAH_OP_GLOBAL_SET, WAH_OP_CALL,
                                    WAH_OP_RETURN_CALL, WAH_OP_IF, WAH_OP_ELSE, WAH_OP_POLL, WAH_OP_TICK,
                                    WAH_OP_MEMORY_SIZE, WAH_OP_MEMORY_GROW };
static const uint16_t three_imm[] = { WAH_OP_BR, WAH_OP_BR_IF };

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
                    case WAH_OP_BR: case WAH_OP_BR_IF: {
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
                    case WAH_OP_GLOBAL_SET: case WAH_OP_CALL: case WAH_OP_POLL: case WAH_OP_TICK: {
                        uint32_t v = wah_read_u32_le(imm) + (rng(2) ? 1 : -1);
                        memcpy(imm, &v, 4);
                        return true;
                    }
                    default: break;
                }
                break;
            default: { // Opcodes
                const uint16_t *list = NULL;
                size_t n = 0;
                if (in_list(in.op, no_imm, sizeof(no_imm) / 2) || (in.op < WAH_FE && in.mem == WAH_CHECK_MEM_NONE &&
                    in.len == 2 && wah_opclasses[in.op] >= WAH_OPCLASS__I_I && wah_opclasses[in.op] <= WAH_OPCLASS__VVV_V)) {
                    list = no_imm, n = sizeof(no_imm) / 2;
                } else if (in_list(in.op, one_imm, sizeof(one_imm) / 2)) {
                    list = one_imm, n = sizeof(one_imm) / 2;
                } else if (in_list(in.op, three_imm, sizeof(three_imm) / 2)) {
                    list = three_imm, n = sizeof(three_imm) / 2;
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

static uint64_t total, dumped, mutants;

static void dump_function(const wah_module_t *m, uint32_t i) {
    const wah_code_body_t *body = &m->code_bodies[i];
    const wah_func_type_t *ft = &m->types[m->function_type_indices[i]];
    const wah_parsed_code_t *pc = &body->parsed_code;
    buf_len = 0;
    total++;
    if (!dump_code(m, ft, body, pc)) return;
    fwrite(buf, 1, buf_len, stdout);
    dumped++;

    uint32_t size = pc->bytecode_size, count = instr_count;
    uint32_t *starts = malloc((size_t)count * sizeof(uint32_t));
    uint8_t *copy = malloc(size);
    if (!starts || !copy) abort();
    for (uint32_t off = 0, k = 0; off < size; k++) {
        starts[k] = off;
        wah_check_instr_t in;
        wah_check_decode(pc->bytecode, size, off, &in);
        off += in.len;
    }
    rng_state = (uint64_t)size * 31 + i;
    for (int k = 0; k < MUTANTS; k++) {
        memcpy(copy, pc->bytecode, size);
        if (!mutate(copy, size, starts, count)) continue;
        wah_parsed_code_t mpc = *pc;
        mpc.bytecode = copy;
        buf_len = 0;
        if (!dump_code(m, ft, body, &mpc)) continue;
        fwrite(buf, 1, buf_len, stdout);
        mutants++;
    }
    free(starts);
    free(copy);
}

static void dump_binary(const uint8_t *bytes, size_t len) {
    static const uint8_t empty = 0;
    for (int metered = 0; metered < 2; metered++) {
        wah_module_t m = {0};
        wah_parse_options_t options = { .enable_fuel_metering = metered };
        if (wah_parse_module(&m, len ? bytes : &empty, len, &options) == WAH_OK) {
            for (uint32_t i = 0; i < m.code_count; i++) dump_function(&m, i);
        }
        wah_free_module(&m);
    }
}

int main(int argc, char **argv) {
    for (int a = 1; a < argc; ++a) {
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
