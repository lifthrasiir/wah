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
wah_value_t nondet_value(void);
uint32_t nondet_u32(void);

static wah_exec_context_t ctx;
static wah_call_frame_t frame;
static wah_code_body_t body;
static uint8_t code[32];

static void put16(uint8_t *p, uint16_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }
static void put32(uint8_t *p, uint32_t v) { for (int i = 0; i < 4; i++) p[i] = (uint8_t)(v >> (8 * i)); }

// The code: the instruction, then UNREACHABLE at every even offset after it, so that jumps to any of them stop there
static void setup_code(uint32_t len) {
    for (uint32_t off = len; off + 2 <= sizeof(code); off += 2) put16(code + off, WAH_OP_UNREACHABLE);
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

// Constants, drop, select and locals: pops and pushes as the abstract machine, accessing only the local it names
void harness_basic(void) {
    uint16_t op = STEP_OP;
    put16(code, op);
    uint32_t pop = 0, push = 0, imm = 0, nl = 0, idx = 0;
    switch (op) {
        case WAH_OP_I32_CONST: case WAH_OP_F32_CONST: imm = 4; push = 1; break;
        case WAH_OP_I64_CONST: case WAH_OP_F64_CONST: imm = 8; push = 1; break;
        case WAH_OP_V128_CONST: imm = 16; push = 1; break;
        case WAH_OP_DROP: pop = 1; break;
        case WAH_OP_SELECT: pop = 3; push = 1; break;
        case WAH_OP_SELECT_T: imm = 4; pop = 3; push = 1; break;
        case WAH_OP_LOCAL_GET: case WAH_OP_LOCAL_SET: case WAH_OP_LOCAL_TEE:
            imm = 4; nl = nondet_u32() % 4 + 1; idx = nondet_u32();
            __CPROVER_assume(idx < nl); // Checked by wah_check_lowered_code
            put32(code + 2, idx);
            pop = op == WAH_OP_LOCAL_GET ? 0 : 1;
            push = op == WAH_OP_LOCAL_SET ? 0 : 1;
            break;
        default: __CPROVER_assert(false, "a basic instruction"); return;
    }
    for (uint32_t i = 0; i < imm && op != WAH_OP_LOCAL_GET && op != WAH_OP_LOCAL_SET && op != WAH_OP_LOCAL_TEE; i++)
        code[2 + i] = (uint8_t)nondet_u32();
    uint32_t len = 2 + imm;
    setup_code(len);
    __CPROVER_assert(decoded_len() == len, "decoded as long as it runs");
    wah_value_t *locals = new_stack(nl);
    frame.locals = locals;
    uint32_t n = pop > push ? pop : push;
    wah_value_t *stack = new_stack(n);
    wah_value_t top = pop ? stack[pop - 1] : nondet_value(), cond = stack[pop ? pop - 1 : 0];
    wah_value_t a = stack[0], b = stack[pop > 1 ? 1 : 0], old = nl ? locals[idx] : top;
    ctx.sp = stack + pop;
    wah_error_t err = wah_run_interpreter(&ctx);
    __CPROVER_assert(err == WAH_ERROR_TRAP && frame.bytecode_ip == code + len + 2, "falls through");
    __CPROVER_assert(ctx.sp == stack + push, "pops and pushes as the abstract machine");
    if (op == WAH_OP_SELECT || op == WAH_OP_SELECT_T)
        __CPROVER_assert(memcmp(&stack[0], cond.i32 ? &a : &b, sizeof(wah_value_t)) == 0, "selects");
    if (op == WAH_OP_LOCAL_GET) __CPROVER_assert(memcmp(&stack[0], &old, sizeof(wah_value_t)) == 0, "gets");
    if (op == WAH_OP_LOCAL_SET || op == WAH_OP_LOCAL_TEE)
        __CPROVER_assert(memcmp(&locals[idx], &top, sizeof(wah_value_t)) == 0, "sets");
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
    if (cond) stack[n - 1].i32 = (op == WAH_OP_IF) != STEP_TAKEN;
    wah_value_t kept[2];
    for (uint32_t i = 0; i < keep; i++) kept[i] = stack[drop + i];
    bool taken = !cond || STEP_TAKEN;
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
        __CPROVER_assert(ctx.sp == stack + n - cond, "pops the condition");
    }
}
