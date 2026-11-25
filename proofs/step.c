// CBMC proofs that interpreter handlers move the value stack and the instruction pointer as the abstract machine of
// proofs/lean/WahProofs/Lowered.lean does for the instructions that they run (`make cbmc-step`). Each harness runs
// one instruction, the opcode STEP_OP given on the command line (by proofs/step_ops.c, which lists them), followed by
// UNREACHABLE, so that the interpreter stops right after it with the value stack and the instruction pointer stored.
// Stacks are allocated with exactly the values that instructions take, so any access beyond them fails.
#define WAH_CHECK_LOWERED // For wah_check_decode
#include "proofs.h"

#ifndef STEP_TAKEN // Only for harness_branch
#define STEP_TAKEN 0
#endif
#ifndef STEP_KEEP
#define STEP_KEEP 0
#endif
#ifndef STEP_DROP
#define STEP_DROP 0
#endif
#ifndef STEP_INDEX // Only for harness_br_table
#define STEP_INDEX 0
#endif
wah_value_t nondet_value(void);
uint32_t nondet_u32(void);

static wah_exec_context_t ctx;
static wah_call_frame_t frame;
static wah_code_body_t body;
static uint8_t code[64];

static void put16(uint8_t *p, uint16_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }
static void put32(uint8_t *p, uint32_t v) { for (int i = 0; i < 4; i++) p[i] = (uint8_t)(v >> (8 * i)); }

// The code: the instruction, then UNREACHABLE at the next even offsets, so that jumps to any of them stop there
static void setup_code(uint32_t len) {
    for (uint32_t off = len; off < len + 8; off += 2) put16(code + off, WAH_OP_UNREACHABLE);
    body.parsed_code.bytecode = code;
    body.parsed_code.bytecode_size = sizeof(code);
    frame.code = &body;
    frame.bytecode_ip = code;
    frame.frame_ctx = &ctx;
    ctx.frame_ptr = &frame;
    ctx.call_depth = 1;
}

// The length of the instruction as wah_check_lowered_code decodes it
static uint32_t decoded_len(void) {
    wah_check_instr_t in;
    __CPROVER_assert(wah_check_decode(code, sizeof(code), 0, &in) == WAH_OK, "decodes");
    return in.len;
}

static wah_value_t *new_stack(uint32_t n) {
    wah_value_t *stack = malloc((n ? n : 1) * sizeof(wah_value_t));
    __CPROVER_assume(stack != NULL);
    for (uint32_t i = 0; i < n; i++) stack[i] = nondet_value();
    return stack;
}

// A jump target other than the next instruction. Handlers only add it to the base of the code, and a symbolic one
// would make the next opcode symbolic, which CBMC can't follow into the interpreter in reasonable time.
static uint32_t target_after(uint32_t len) { return len + 2; }

// Values that numeric classes pop, by their names (`_<popped>_<pushed>`); each pushes one
static uint32_t class_pops(uint8_t cls) {
    switch (cls) {
        case WAH_OPCLASS__I_I: case WAH_OPCLASS__I_L: case WAH_OPCLASS__I_F: case WAH_OPCLASS__I_D:
        case WAH_OPCLASS__I_V: case WAH_OPCLASS__L_I: case WAH_OPCLASS__L_L: case WAH_OPCLASS__L_F:
        case WAH_OPCLASS__L_D: case WAH_OPCLASS__L_V: case WAH_OPCLASS__F_I: case WAH_OPCLASS__F_L:
        case WAH_OPCLASS__F_F: case WAH_OPCLASS__F_V: case WAH_OPCLASS__D_I: case WAH_OPCLASS__D_L:
        case WAH_OPCLASS__D_D: case WAH_OPCLASS__D_V: case WAH_OPCLASS__V_I: case WAH_OPCLASS__V_V: return 1;
        case WAH_OPCLASS__II_I: case WAH_OPCLASS__IV_V: case WAH_OPCLASS__LL_I: case WAH_OPCLASS__LL_L:
        case WAH_OPCLASS__LV_V: case WAH_OPCLASS__FF_I: case WAH_OPCLASS__FF_F: case WAH_OPCLASS__DD_I:
        case WAH_OPCLASS__DD_L: case WAH_OPCLASS__DD_D: case WAH_OPCLASS__VI_V: case WAH_OPCLASS__VV_V: return 2;
        case WAH_OPCLASS__VVV_V: return 3;
        default: return 0;
    }
}

// Runs a numeric instruction (STEP_OP) on a stack of exactly the values that its class pops, followed by
// UNREACHABLE, and checks that it pops them and pushes one and falls through, unless it traps.
void harness_numeric(void) {
    uint16_t op = STEP_OP;
    uint32_t pop = class_pops(wah_opclasses[op]), push = 1;
    __CPROVER_assert(pop > 0, "a numeric class");
    put16(code, op);
    put16(code + 2, WAH_OP_UNREACHABLE);
    body.parsed_code.bytecode = code;
    body.parsed_code.bytecode_size = 4;
    frame.code = &body;
    frame.bytecode_ip = code;
    frame.frame_ctx = &ctx;
    ctx.frame_ptr = &frame;
    ctx.call_depth = 1;
    uint32_t n = pop > push ? pop : push;
    wah_value_t *stack = malloc(n * sizeof(wah_value_t));
    __CPROVER_assume(stack != NULL);
    for (uint32_t i = 0; i < n; i++) stack[i] = nondet_value();
    ctx.sp = stack + pop;
    wah_error_t err = wah_run_interpreter(&ctx);
    __CPROVER_assert(err == WAH_ERROR_TRAP, "only traps");
    if (frame.bytecode_ip == code + 4) {
        __CPROVER_assert(ctx.sp == stack + push, "pops and pushes as the class says");
    } else {
        __CPROVER_assert(frame.bytecode_ip == code + 2, "traps in the instruction");
    }
}

// Constants, drop, select, locals, globals and reference instructions: pop and push as the abstract machine,
// accessing only the local or global that they name, or trap where they may
void harness_basic(void) {
    uint16_t op = STEP_OP;
    put16(code, op);
    uint32_t pop = 0, push = 0, imm = 0, nl = 0, ng = 0, nf = 0, idx = 0;
    bool may_trap = false, named = false;
    switch (op) {
        case WAH_OP_I32_CONST: case WAH_OP_F32_CONST: imm = 4; push = 1; break;
        case WAH_OP_I64_CONST: case WAH_OP_F64_CONST: imm = 8; push = 1; break;
        case WAH_OP_V128_CONST: imm = 16; push = 1; break;
        case WAH_OP_DROP: pop = 1; break;
        case WAH_OP_SELECT: pop = 3; push = 1; break;
        case WAH_OP_SELECT_T: imm = 4; pop = 3; push = 1; break;
        case WAH_OP_LOCAL_GET: case WAH_OP_LOCAL_SET: case WAH_OP_LOCAL_TEE:
            imm = 4; nl = nondet_u32() % 4 + 1; named = true;
            pop = op == WAH_OP_LOCAL_GET ? 0 : 1;
            push = op == WAH_OP_LOCAL_SET ? 0 : 1;
            break;
        case WAH_OP_GLOBAL_GET: case WAH_OP_GLOBAL_SET: case WAH_OP_GLOBAL_GET_INDIRECT: case WAH_OP_GLOBAL_SET_INDIRECT:
            imm = 4; ng = nondet_u32() % 4 + 1; named = true;
            pop = op == WAH_OP_GLOBAL_SET || op == WAH_OP_GLOBAL_SET_INDIRECT;
            push = !pop;
            break;
        case WAH_OP_REF_NULL: imm = 4; push = 1; break;
        case WAH_OP_REF_FUNC: imm = 4; nf = nondet_u32() % 4 + 1; named = true; push = 1; break;
        case WAH_OP_REF_IS_NULL: case WAH_OP_REF_I31: pop = 1; push = 1; break;
        case WAH_OP_REF_AS_NON_NULL: case WAH_OP_I31_GET_S: case WAH_OP_I31_GET_U: pop = 1; push = 1; may_trap = true; break;
        case WAH_OP_REF_EQ: pop = 2; push = 1; break;
        default: __CPROVER_assert(false, "a basic instruction"); return;
    }
    if (named) {
        idx = nondet_u32();
        __CPROVER_assume(idx < (nl ? nl : ng ? ng : nf)); // Checked by wah_check_lowered_code
        put32(code + 2, idx);
    } else {
        for (uint32_t i = 0; i < imm; i++) code[2 + i] = (uint8_t)nondet_u32();
    }
    uint32_t len = 2 + imm;
    setup_code(len);
    __CPROVER_assert(decoded_len() == len, "decoded as long as it runs");
    wah_value_t *locals = new_stack(nl);
    frame.locals = locals;
    // Globals, which imported mutable ones refer to by pointers to their cells
    bool indirect = op == WAH_OP_GLOBAL_GET_INDIRECT || op == WAH_OP_GLOBAL_SET_INDIRECT;
    wah_value_t *globals = new_stack(ng), *cell = new_stack(1);
    if (indirect) globals[idx].ref = cell;
    frame.frame_globals = globals;
    wah_function_holder_t *funcs = malloc((nf ? nf : 1) * sizeof(wah_function_holder_t));
    __CPROVER_assume(funcs != NULL);
    frame.frame_function_table = funcs;
    frame.frame_function_table_count = nf;
    uint32_t n = pop > push ? pop : push;
    wah_value_t *stack = new_stack(n);
    wah_value_t top = pop ? stack[pop - 1] : nondet_value(), cond = stack[pop ? pop - 1 : 0];
    wah_value_t a = stack[0], b = stack[pop > 1 ? 1 : 0];
    wah_value_t old = nl ? locals[idx] : ng ? (indirect ? *cell : globals[idx]) : top;
    ctx.sp = stack + pop;
    wah_error_t err = wah_run_interpreter(&ctx);
    __CPROVER_assert(err == WAH_ERROR_TRAP, "only traps");
    if (frame.bytecode_ip != code + len + 2) {
        __CPROVER_assert(may_trap && frame.bytecode_ip == code + len, "traps where it may");
        return;
    }
    __CPROVER_assert(ctx.sp == stack + push, "pops and pushes as the abstract machine");
    if (op == WAH_OP_SELECT || op == WAH_OP_SELECT_T)
        __CPROVER_assert(memcmp(&stack[0], cond.i32 ? &a : &b, sizeof(wah_value_t)) == 0, "selects");
    if (op == WAH_OP_LOCAL_GET || op == WAH_OP_GLOBAL_GET || op == WAH_OP_GLOBAL_GET_INDIRECT)
        __CPROVER_assert(memcmp(&stack[0], &old, sizeof(wah_value_t)) == 0, "gets");
    if (op == WAH_OP_LOCAL_SET || op == WAH_OP_LOCAL_TEE)
        __CPROVER_assert(memcmp(&locals[idx], &top, sizeof(wah_value_t)) == 0, "sets");
    if (op == WAH_OP_GLOBAL_SET || op == WAH_OP_GLOBAL_SET_INDIRECT)
        __CPROVER_assert(memcmp(indirect ? cell : &globals[idx], &top, sizeof(wah_value_t)) == 0, "sets");
    if (op == WAH_OP_REF_NULL) __CPROVER_assert(stack[0].ref == NULL, "pushes null");
    if (op == WAH_OP_REF_FUNC) __CPROVER_assert(stack[0].ref == wah_func_to_ref(&funcs[idx].func), "pushes the function");
    if (op == WAH_OP_REF_IS_NULL) __CPROVER_assert(stack[0].i32 == (top.ref == NULL), "tests null");
    if (op == WAH_OP_REF_EQ) __CPROVER_assert(stack[0].i32 == (a.ref == b.ref), "compares");
    if (op == WAH_OP_REF_AS_NON_NULL) __CPROVER_assert(stack[0].ref == top.ref && top.ref != NULL, "keeps non-null");
}

// Branches: jump to their targets keeping the top `keep` values and dropping `drop` below them, or fall through.
// The condition (STEP_TAKEN) and the values kept and dropped (STEP_KEEP, STEP_DROP) are constant, as symbolic ones
// would make the stack pointer and the next opcode symbolic, which CBMC can't follow in reasonable time.
void harness_branch(void) {
    uint16_t op = STEP_OP;
    put16(code, op);
    uint32_t keep = STEP_KEEP, drop = STEP_DROP, len, cond = 0;
    uint32_t t;
    switch (op) {
        case WAH_OP_BR: case WAH_OP_BR_IF:
            len = 14; t = target_after(len);
            put32(code + 2, t); put32(code + 6, keep); put32(code + 10, drop);
            cond = op == WAH_OP_BR_IF;
            break;
        case WAH_OP_BR_ON_NULL: case WAH_OP_BR_ON_NON_NULL:
            len = 14; t = target_after(len);
            if (op == WAH_OP_BR_ON_NON_NULL) __CPROVER_assume(keep > 0); // The reference is kept
            put32(code + 2, t); put32(code + 6, keep); put32(code + 10, drop);
            cond = op == WAH_OP_BR_ON_NULL;
            break;
        case WAH_OP_IF: case WAH_OP_ELSE:
            len = 6; t = target_after(len); keep = drop = 0;
            put32(code + 2, t);
            cond = op == WAH_OP_IF;
            break;
        default: __CPROVER_assert(false, "a branch"); return;
    }
    setup_code(len);
    __CPROVER_assert(decoded_len() == len, "decoded as long as it runs");
    uint32_t n = drop + keep + cond;
    wah_value_t *stack = new_stack(n);
    static uint8_t object;
    if (op == WAH_OP_BR_ON_NULL || op == WAH_OP_BR_ON_NON_NULL) {
        stack[n - 1].ref = (op == WAH_OP_BR_ON_NULL) == STEP_TAKEN ? NULL : &object;
    } else if (cond) {
        stack[n - 1].i32 = (op == WAH_OP_IF) != STEP_TAKEN;
    }
    wah_value_t kept[2];
    for (uint32_t i = 0; i < keep; i++) kept[i] = stack[drop + i];
    bool taken = (!cond && op != WAH_OP_BR_ON_NON_NULL) || STEP_TAKEN;
    ctx.sp = stack + n;
    wah_error_t err = wah_run_interpreter(&ctx);
    __CPROVER_assert(err == WAH_ERROR_TRAP, "stops at unreachable");
    if (taken) {
        __CPROVER_assert(frame.bytecode_ip == code + t + 2, "jumps to the target");
        __CPROVER_assert(ctx.sp == stack + keep, "drops below the kept values");
        for (uint32_t i = 0; i < keep; i++)
            __CPROVER_assert(memcmp(&stack[i], &kept[i], sizeof(wah_value_t)) == 0, "keeps the top values");
    } else {
        __CPROVER_assert(frame.bytecode_ip == code + len + 2, "falls through");
        // The condition is popped, and the reference of br_on_null kept, that of br_on_non_null not
        uint32_t left = op == WAH_OP_BR_ON_NULL ? n : op == WAH_OP_BR_ON_NON_NULL ? n - 1 : n - cond;
        __CPROVER_assert(ctx.sp == stack + left, "pops the condition");
    }
}

// Lanes of SIMD loads and stores of lanes, or 0 for other memory instructions
static uint32_t mem_lanes(uint16_t base, bool *load) {
    *load = base == WAH_OP_V128_LOAD8_LANE || base == WAH_OP_V128_LOAD16_LANE || base == WAH_OP_V128_LOAD32_LANE ||
            base == WAH_OP_V128_LOAD64_LANE;
    switch (base) {
        case WAH_OP_V128_LOAD8_LANE: case WAH_OP_V128_STORE8_LANE: return 16;
        case WAH_OP_V128_LOAD16_LANE: case WAH_OP_V128_STORE16_LANE: return 8;
        case WAH_OP_V128_LOAD32_LANE: case WAH_OP_V128_STORE32_LANE: return 4;
        case WAH_OP_V128_LOAD64_LANE: case WAH_OP_V128_STORE64_LANE: return 2;
        default: return 0;
    }
}

static bool mem_store(uint16_t base) {
    switch (base) {
        case WAH_OP_I32_STORE: case WAH_OP_I32_STORE8: case WAH_OP_I32_STORE16: case WAH_OP_I64_STORE:
        case WAH_OP_I64_STORE8: case WAH_OP_I64_STORE16: case WAH_OP_I64_STORE32: case WAH_OP_F32_STORE:
        case WAH_OP_F64_STORE: case WAH_OP_V128_STORE: return true;
        default: return false;
    }
}

// Loads and stores (of all memory and address kinds): pop the address (and the value), push the value loaded, and
// access only the memory that they name, within its size, or trap
void harness_memory(void) {
    uint16_t op = STEP_OP;
    put16(code, op);
    memset(code + 2, 0, 22);
    wah_check_instr_t in;
    __CPROVER_assert(wah_check_decode(code, sizeof(code), 0, &in) == WAH_OK && in.mem != WAH_CHECK_MEM_NONE,
                     "a memory instruction");
    bool mem0 = in.mem == WAH_CHECK_MEM_I32_MEM0 || in.mem == WAH_CHECK_MEM_I64_MEM0, lane_load;
    uint32_t lanes = mem_lanes(in.base, &lane_load), len = in.len;
    uint32_t count = nondet_u32() % 2 + 1, memidx = mem0 ? 0 : nondet_u32() % count;
    uint32_t p = 2;
    if (!mem0) { put32(code + p, memidx); p += 4; }
    for (uint32_t i = 0; i < 8; i++) code[p + i] = (uint8_t)nondet_u32();
    p += 8;
    if (lanes) code[p++] = (uint8_t)(nondet_u32() % lanes); // Checked by wah_check_lowered_code
    __CPROVER_assert(p == len, "immediates as decoded");
    setup_code(len);
    __CPROVER_assert(decoded_len() == len, "decoded as long as it runs");
    wah_memory_inst_t *mems = malloc(count * sizeof(wah_memory_inst_t));
    __CPROVER_assume(mems != NULL);
    for (uint32_t i = 0; i < count; i++) {
        uint64_t size = nondet_u32() % 17;
        mems[i].size = size;
        mems[i].data = malloc(size ? size : 1);
        __CPROVER_assume(mems[i].data != NULL);
    }
    ctx.memories = mems;
    ctx.memory_count = count;
    ctx.memory_base = mems[0].data;
    ctx.memory_size = mems[0].size;
    uint32_t pop = lanes || mem_store(in.base) ? 2 : 1, push = lanes ? lane_load : !mem_store(in.base);
    uint32_t n = pop > push ? pop : push;
    wah_value_t *stack = new_stack(n);
    ctx.sp = stack + pop;
    wah_error_t err = wah_run_interpreter(&ctx);
    if (frame.bytecode_ip == code + len + 2) {
        __CPROVER_assert(err == WAH_ERROR_TRAP, "stops at unreachable");
        __CPROVER_assert(ctx.sp == stack + push, "pops and pushes as the abstract machine");
    } else {
        __CPROVER_assert(err == WAH_ERROR_TRAP || err == WAH_ERROR_MEMORY_OUT_OF_BOUNDS, "only traps");
    }
}

// br_table: takes the record of the index (STEP_INDEX, or the last one beyond the targets), keeping STEP_KEEP values
// and dropping those of the record
void harness_br_table(void) {
    uint32_t targets = 2, keep = STEP_KEEP, drop = STEP_DROP, index = STEP_INDEX;
    uint32_t len = 2 + 4 + 4 + (targets + 1) * 2 + (targets + 1) * 8;
    put16(code, WAH_OP_BR_TABLE);
    put32(code + 2, targets);
    put32(code + 6, keep);
    for (uint32_t r = 0; r <= targets; r++) {
        put16(code + 10 + 2 * r, (uint16_t)r);
        put32(code + 10 + (targets + 1) * 2 + 8 * r, len + 2 * r);
        put32(code + 10 + (targets + 1) * 2 + 8 * r + 4, drop);
    }
    setup_code(len);
    __CPROVER_assert(decoded_len() == len, "decoded as long as it runs");
    uint32_t n = drop + keep + 1, taken = index < targets ? index : targets;
    wah_value_t *stack = new_stack(n);
    stack[n - 1].i32 = (int32_t)index;
    wah_value_t kept[2];
    for (uint32_t i = 0; i < keep; i++) kept[i] = stack[drop + i];
    ctx.sp = stack + n;
    wah_error_t err = wah_run_interpreter(&ctx);
    __CPROVER_assert(err == WAH_ERROR_TRAP, "stops at unreachable");
    __CPROVER_assert(frame.bytecode_ip == code + len + 2 * taken + 2, "jumps to the target of the record");
    __CPROVER_assert(ctx.sp == stack + keep, "drops below the kept values");
    for (uint32_t i = 0; i < keep; i++)
        __CPROVER_assert(memcmp(&stack[i], &kept[i], sizeof(wah_value_t)) == 0, "keeps the top values");
}

#ifndef STEP_PARAMS // Only for harness_call and harness_return
#define STEP_PARAMS 0
#endif
#ifndef STEP_RESULTS
#define STEP_RESULTS 0
#endif
#ifndef STEP_LOCALS
#define STEP_LOCALS 0
#endif
#ifndef STEP_SLACK // Operands of the callee beyond its locals, constant as the stack overflows or not
#define STEP_SLACK 0
#endif

// The unified stack: values grow upward from vals, frames downward from the end, one for the caller already there.
// Values beyond vals run into the frames, which the interpreter checks before pushing a frame.
typedef struct { wah_value_t vals[8]; wah_call_frame_t frames[2]; } step_stack_t;
static wah_module_t mod;
static uint8_t callee_code[8];

// Calls of a wasm function of STEP_PARAMS params, STEP_RESULTS results and STEP_LOCALS locals: the params become the
// first locals of a new frame, followed by zeroed locals, which starts at the beginning of the callee with the return
// address stored in the caller; or the stack overflows when the callee may not have room for its locals and operands
void harness_call(void) {
    put16(code, WAH_OP_CALL);
    put32(code + 2, 0);
    uint32_t len = 6;
    setup_code(len);
    __CPROVER_assert(decoded_len() == len, "decoded as long as it runs");
    for (uint32_t off = 0; off < sizeof(callee_code); off += 2) put16(callee_code + off, WAH_OP_UNREACHABLE);
    static wah_func_type_t type;
    static uint32_t type_idx;
    static wah_code_body_t callee;
    static wah_function_holder_t fn;
    type.param_count = STEP_PARAMS;
    type.result_count = STEP_RESULTS;
    uint32_t slack = STEP_SLACK;
    callee.local_count = STEP_LOCALS;
    callee.max_frame_slots = STEP_LOCALS + slack;
    callee.parsed_code.bytecode = callee_code;
    callee.parsed_code.bytecode_size = sizeof(callee_code);
    mod.types = &type;
    mod.function_type_indices = &type_idx;
    mod.code_bodies = &callee;
    fn.func.is_host = false;
    fn.func.local_idx = 0;
    ctx.module = &mod;
    ctx.function_table = &fn;
    ctx.function_table_count = 1;
    static step_stack_t st_; // Static, since CBMC loses frame pointers stored in dynamic objects
    step_stack_t *st = &st_;
    for (uint32_t i = 0; i < 8; i++) st->vals[i] = nondet_value();
    wah_value_t args[STEP_PARAMS + 1];
    for (uint32_t i = 0; i < STEP_PARAMS; i++) args[i] = st->vals[1 + i];
    wah_call_frame_t *caller = &st->frames[1];
    *caller = frame;
    caller->module = &mod;
    caller->frame_function_table = &fn;
    caller->frame_function_table_count = 1;
    ctx.frame_ptr = caller;
    ctx.sp = st->vals + 1 + STEP_PARAMS; // One value of the caller below the arguments
    wah_error_t err = wah_run_interpreter(&ctx);
    if (1 + STEP_PARAMS + STEP_LOCALS + slack > 8) {
        __CPROVER_assert(err == WAH_ERROR_STACK_OVERFLOW, "overflows without room");
        __CPROVER_assert(ctx.call_depth == 1 && ctx.frame_ptr == caller, "pushes no frame");
        return;
    }
    __CPROVER_assert(err == WAH_ERROR_TRAP, "stops at unreachable");
    __CPROVER_assert(ctx.call_depth == 2 && ctx.frame_ptr == &st->frames[0], "pushes a frame");
    wah_call_frame_t *f = &st->frames[0];
    __CPROVER_assert(caller->bytecode_ip == code + len, "stores the return address");
    __CPROVER_assert(f->bytecode_ip == callee_code + 2, "starts at the callee");
    __CPROVER_assert(f->locals == st->vals + 1 && f->result_count == STEP_RESULTS, "takes the arguments as locals");
    __CPROVER_assert(f->code == &callee && f->frame_ctx == &ctx && f->frame_function_table == &fn, "binds the callee");
    __CPROVER_assert(ctx.sp == f->locals + STEP_PARAMS + STEP_LOCALS, "pushes the locals");
    for (uint32_t i = 0; i < STEP_PARAMS; i++)
        __CPROVER_assert(memcmp(&f->locals[i], &args[i], sizeof(wah_value_t)) == 0, "keeps the arguments");
    wah_value_t zero;
    memset(&zero, 0, sizeof(zero));
    for (uint32_t i = STEP_PARAMS; i < STEP_PARAMS + STEP_LOCALS; i++)
        __CPROVER_assert(memcmp(&f->locals[i], &zero, sizeof(wah_value_t)) == 0, "zeroes the other locals");
}

// return and the end of a function: move STEP_RESULTS results from the top of the stack to the locals of the frame,
// dropping STEP_DROP values below them, pop the frame and continue at the return address of the caller
void harness_return(void) {
    uint16_t op = STEP_OP;
    put16(code, op);
    uint32_t len = 2;
    setup_code(len);
    __CPROVER_assert(decoded_len() == len, "decoded as long as it runs");
    for (uint32_t off = 0; off < sizeof(callee_code); off += 2) put16(callee_code + off, WAH_OP_UNREACHABLE);
    step_stack_t *st = malloc(sizeof(step_stack_t));
    __CPROVER_assume(st != NULL);
    for (uint32_t i = 0; i < 8; i++) st->vals[i] = nondet_value();
    wah_value_t below = st->vals[0], results[STEP_RESULTS + 1];
    for (uint32_t i = 0; i < STEP_RESULTS; i++) results[i] = st->vals[1 + STEP_DROP + i];
    wah_call_frame_t *caller = &st->frames[1], *f = &st->frames[0];
    *caller = frame;
    caller->bytecode_ip = callee_code; // The return address
    *f = frame;
    f->locals = st->vals + 1;
    f->result_count = STEP_RESULTS;
    ctx.frame_ptr = f;
    ctx.call_depth = 2;
    ctx.sp = st->vals + 1 + STEP_DROP + STEP_RESULTS;
    wah_error_t err = wah_run_interpreter(&ctx);
    __CPROVER_assert(err == WAH_ERROR_TRAP, "stops at unreachable");
    __CPROVER_assert(ctx.call_depth == 1 && ctx.frame_ptr == caller, "pops the frame");
    __CPROVER_assert(caller->bytecode_ip == callee_code + 2, "continues at the return address");
    __CPROVER_assert(ctx.sp == st->vals + 1 + STEP_RESULTS, "leaves the results in place of the locals");
    __CPROVER_assert(memcmp(&st->vals[0], &below, sizeof(wah_value_t)) == 0, "keeps the caller's values");
    for (uint32_t i = 0; i < STEP_RESULTS; i++)
        __CPROVER_assert(memcmp(&st->vals[1 + i], &results[i], sizeof(wah_value_t)) == 0, "moves the results");
}

// table.get, table.set and table.size (of 32- and 64-bit tables): access only the table that they name, within its
// size, or trap
void harness_table(void) {
    uint16_t op = STEP_OP;
    put16(code, op);
    bool get = op == WAH_OP_TABLE_GET || op == WAH_OP_TABLE_GET_i64;
    bool set = op == WAH_OP_TABLE_SET || op == WAH_OP_TABLE_SET_i64;
    bool i64 = op == WAH_OP_TABLE_GET_i64 || op == WAH_OP_TABLE_SET_i64 || op == WAH_OP_TABLE_SIZE_i64;
    __CPROVER_assert(get || set || op == WAH_OP_TABLE_SIZE || op == WAH_OP_TABLE_SIZE_i64, "a table instruction");
    uint32_t count = nondet_u32() % 2 + 1, tidx = nondet_u32() % count, len = 6;
    put32(code + 2, tidx);
    setup_code(len);
    __CPROVER_assert(decoded_len() == len, "decoded as long as it runs");
    wah_table_inst_t *tables = malloc(count * sizeof(wah_table_inst_t));
    __CPROVER_assume(tables != NULL);
    for (uint32_t i = 0; i < count; i++) {
        tables[i].size = nondet_u32() % 5;
        tables[i].entries = malloc(tables[i].size ? tables[i].size * sizeof(wah_value_t) : 1);
        __CPROVER_assume(tables[i].entries != NULL);
    }
    ctx.tables = tables;
    ctx.table_count = count;
    uint32_t pop = get ? 1 : set ? 2 : 0, push = set ? 0 : 1, n = pop > push ? pop : push;
    wah_value_t *stack = new_stack(n);
    wah_value_t val = stack[set ? 1 : 0];
    uint64_t elem = i64 ? (uint64_t)stack[0].i64 : (uint32_t)stack[0].i32;
    wah_value_t old = elem < tables[tidx].size ? tables[tidx].entries[elem] : val;
    ctx.sp = stack + pop;
    wah_error_t err = wah_run_interpreter(&ctx);
    __CPROVER_assert(err == WAH_ERROR_TRAP, "only traps");
    if (frame.bytecode_ip != code + len + 2) {
        __CPROVER_assert((get || set) && elem >= tables[tidx].size && frame.bytecode_ip == code + len,
                         "traps out of bounds");
        return;
    }
    __CPROVER_assert(ctx.sp == stack + push, "pops and pushes as the abstract machine");
    if (get) __CPROVER_assert(memcmp(&stack[0], &old, sizeof(wah_value_t)) == 0, "gets the element");
    if (set) __CPROVER_assert(memcmp(&tables[tidx].entries[elem], &val, sizeof(wah_value_t)) == 0, "sets the element");
    if (!get && !set) {
        if (i64) __CPROVER_assert((uint64_t)stack[0].i64 == tables[tidx].size, "pushes the size");
        else __CPROVER_assert((uint32_t)stack[0].i32 == tables[tidx].size, "pushes the size");
    }
}
