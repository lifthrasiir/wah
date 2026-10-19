// wahjit.h -- Optional experimental JIT compiler for WAH.
//
// Include this header instead of wah.h in the single file defining WAH_IMPLEMENTATION:
//
//     #define WAH_IMPLEMENTATION
//     #include "wahjit.h"
//
// Other files keep including wah.h, whose API is unchanged.
//
// Currently the following combinations of OS and architecture are supported:
// - x86-64 on Windows, macOS or Linux, with SSE4.1 and POPCNT (otherwise every module is interpreted)
// - AArch64 on macOS or Linux
// Elsewhere, or with WAH_NO_JIT defined, this is the same as including wah.h.
//
// This file works by enabling a few hooks defined in wah.h, only available when WAH_JIT is defined.
// `wah_alloc_t` is partly ignored by the JIT; some memory (namely executable code) is directly mapped from the OS.
// The user can opt out of JIT compilation by setting `wah_parse_options_t::disable_jit` to true.
// Native execution only differs from the interpreter by resource usage and polling behavior;
// otherwise they are identical.
//
// Only a subset of modules are compiled: those without fuel metering, using only numeric types (i32, i64, f32 and
// f64) and WASM 2.0 instructions except SIMD, reference types and table instructions other than `call_indirect`.
// Any number of 32-bit memories can be used. Any other feature (e.g. GC, exceptions, tail calls or memory64) leaves
// the whole module interpreted. Its activation runs natively when all of its function imports are host functions,
// it neither imports nor exports a table, and it imports no global of a reference type; otherwise it is interpreted.

#ifndef WAHJIT_H
#define WAHJIT_H

#if defined(WAH_IMPLEMENTATION) && !defined(WAH_NO_JIT) && \
    (((defined(__x86_64__) || defined(_M_X64)) && (defined(_WIN32) || defined(__APPLE__) || defined(__linux__))) || \
     (defined(__aarch64__) && (defined(__APPLE__) || defined(__linux__))))
#ifdef WAH_H
#error "wahjit.h should be included instead of wah.h, not after it"
#endif
#define WAH_JIT 1
#if defined(__x86_64__) || defined(_M_X64)
#define WAH_JIT_X86_64 1
#else
#define WAH_JIT_AARCH64 1
#endif
#endif

#include "wah.h"

#ifdef WAH_JIT

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <sys/mman.h>
#endif
#if defined(WAH_JIT_X86_64)
#ifdef _MSC_VER
#include <intrin.h>
#else
#include <cpuid.h>
#endif
#elif defined(__APPLE__)
#include <pthread.h>
#include <libkern/OSCacheControl.h>
#endif

////////////////////////////////////////////////////////////////////////////////
// Data structures /////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Native code of a module is a single buffer starting with the trampoline, which calls into or resumes the code.
// Wasm calls don't use the native stack, so traps and suspensions can exit straight to the trampoline,
// and a suspended activation can be resumed by any later trampoline call.
// Each function has a frame in the value stack: its locals, followed by the link slot holding the return address,
// its operand stack and scratch slots for host call results. Callees start at arguments on the operand stack,
// so they don't overwrite the link slot. See code generation sections for register usages of each architecture.
typedef wah_error_t (*wah_jit_entry_t)(wah_exec_context_t *ctx, const void *code, const void *resume_pc, wah_value_t *fp);

typedef struct { uint32_t pos, func; } wah_jit_call_fixup_t;

typedef struct wah_jit_module_s {
    uint8_t *buf;               // Code being generated; freed when finished
    uint32_t len, cap;          // In bytes, like all code offsets
    uint32_t *func_pos;         // Offset of each defined function
    wah_jit_call_fixup_t *calls; // Direct calls to be patched when all functions are compiled
    uint32_t call_count, call_cap;
    uint32_t exit_pos;          // Offset of the trampoline exit, where the return register holds the result
    uint32_t extra_slots;       // Slots above the operand stack for host call results
    bool failed;                // Some function couldn't be compiled, so the module is interpreted
#ifdef WAH_JIT_X86_64
    bool lzcnt, tzcnt;          // Optional instructions supported by the CPU
#endif
#ifdef _WIN32
    uint8_t unwind[64];         // UNWIND_INFO of the trampoline frame, which applies to the whole code
    void *func_table;           // RUNTIME_FUNCTION registered for the code, placed after it
#endif
    uint8_t *code;              // Executable code, when finished
    size_t code_size;
} wah_jit_module_t;

typedef struct wah_jit_ctx_s {
    const void *resume_pc;      // Where a suspended native activation continues, NULL if not suspended
    wah_value_t *resume_fp;
    wah_error_t err;            // Error from a helper returning a pointer
} wah_jit_ctx_t;

// A value on the operand stack being compiled
enum { WAH_JV_SLOT, WAH_JV_GPR, WAH_JV_FPR, WAH_JV_CONST, WAH_JV_FLAGS };
typedef struct {
    uint8_t kind;
    uint8_t reg; // Register for GPR and FPR, condition code for FLAGS
    int64_t imm; // Value for CONST
} wah_jit_val_t;

// Owners of registers, otherwise an index of the operand stack
#define WAH_JO_FREE (-1)
#define WAH_JO_TEMP (-2) // Popped but still in use by the instruction being compiled
#define WAH_JO_NONE (-3) // Not allocatable

typedef struct {
    uint16_t opcode;            // WAH_OP_BLOCK, WAH_OP_LOOP or WAH_OP_IF
    bool has_else;
    bool reachable;             // Some branch targets the end of a block or if
    uint32_t label, else_label; // End label (header label for loops), and the false edge of if
    uint32_t h_start, end_h;    // Stack heights at the start (including params) and the end (if reachable)
} wah_jit_ctl_t;

// TABLE is a 32-bit offset from `base`, others are branches specific to each architecture
enum { WAH_JP_TABLE, WAH_JP_X64_REL32, WAH_JP_A64_B26, WAH_JP_A64_B19, WAH_JP_A64_ADR };
typedef struct { uint32_t pos, label, base; uint8_t kind; } wah_jit_patch_t;

// Out-of-line code for suspension points: a POLL calling the poll handler first,
// or a helper call which may have returned a status in the return register
typedef struct { uint32_t stub, resume, cont; bool poll; } wah_jit_stub_t;

// Arguments of helper calls, where SLOT is the address of an operand stack slot and SLOT_U32 is its 32-bit value
enum { WAH_JA_CTX, WAH_JA_IMM, WAH_JA_SLOT, WAH_JA_SLOT_U32 };
typedef struct { uint8_t kind; uint32_t v; } wah_jit_arg_t;

// Kinds of operations, translated by each architecture
enum { WAH_JI_ADD, WAH_JI_SUB, WAH_JI_MUL, WAH_JI_AND, WAH_JI_OR, WAH_JI_XOR };
enum { WAH_JI_CLZ, WAH_JI_CTZ, WAH_JI_POPCNT };
enum { WAH_JS_SHL, WAH_JS_SHR_S, WAH_JS_SHR_U, WAH_JS_ROTL, WAH_JS_ROTR };
enum { WAH_JD_DIV_S, WAH_JD_DIV_U, WAH_JD_REM_S, WAH_JD_REM_U };
enum { WAH_JF_ADD, WAH_JF_SUB, WAH_JF_MUL, WAH_JF_DIV, WAH_JF_MIN, WAH_JF_MAX };
enum { WAH_JF_ABS, WAH_JF_NEG, WAH_JF_CEIL, WAH_JF_FLOOR, WAH_JF_TRUNC, WAH_JF_NEAREST, WAH_JF_SQRT };
enum { WAH_JFC_EQ, WAH_JFC_NE, WAH_JFC_LT, WAH_JFC_GT, WAH_JFC_LE, WAH_JFC_GE };

typedef struct {
    wah_jit_module_t *jm;
    const wah_alloc_t *alloc;
    const wah_module_t *module;
    const wah_func_type_t *ft;
    uint32_t nlocals, max_stack, frame_bytes, link_off;

    wah_jit_val_t *vals;
    uint32_t h;
    int32_t gpr[32], fpr[32];

    wah_jit_ctl_t *ctl;
    uint32_t nctl, ctl_cap;
    uint32_t *labels; // Code offsets, UINT32_MAX if not bound yet
    uint32_t nlabels, labels_cap;
    wah_jit_patch_t *patches;
    uint32_t npatches, patches_cap;
    wah_jit_stub_t *stubs;
    uint32_t nstubs, stubs_cap;
    uint32_t l_trap, l_oob, l_stack, l_err, l_err_jit; // Shared stubs exiting with the corresponding error

    bool dead;           // The current code is unreachable
    uint32_t dead_depth; // Number of blocks entered in unreachable code
    bool long_branches;  // Conditional branches go through unconditional ones, for AArch64 functions over 1MB
    bool out_of_range;   // Retry with long branches
    bool fail;
} wah_jit_fn_t;

////////////////////////////////////////////////////////////////////////////////
// Code buffer and labels //////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define WAH_JIT_FAIL(f, reason) do { WAH_LOG("JIT: %s", reason); (f)->fail = true; } while (0)

static void wah_jit_emit(wah_jit_fn_t *f, const void *bytes, uint32_t n) {
    wah_jit_module_t *jm = f->jm;
    if (jm->cap - jm->len < n) {
        void *p = jm->buf;
        if (wah_grow_array(f->alloc, (size_t)jm->len + n, 1, &p, &jm->cap) != WAH_OK) {
            WAH_JIT_FAIL(f, "out of memory");
            jm->len = 0; // Keep going without growing, the result is discarded anyway
            if (!jm->buf || jm->cap < n) return;
        } else {
            jm->buf = (uint8_t *)p;
        }
    }
    memcpy(jm->buf + jm->len, bytes, n);
    jm->len += n;
}

static uint32_t wah_jit_pos(const wah_jit_fn_t *f) { return f->jm->len; }

static void wah_jit_set32(wah_jit_module_t *jm, uint32_t pos, uint32_t v) { memcpy(jm->buf + pos, &v, 4); }

#define WAH_JIT_GROW(f, arr, count, cap) do { \
    void *p_ = (f)->arr; \
    if (wah_grow_array((f)->alloc, (size_t)(f)->count + 1, sizeof(*(f)->arr), &p_, &(f)->cap) != WAH_OK) { \
        WAH_JIT_FAIL((f), "out of memory"); \
    } else { \
        (f)->arr = p_; \
    } \
} while (0)

static uint32_t wah_jit_new_label(wah_jit_fn_t *f) {
    WAH_JIT_GROW(f, labels, nlabels, labels_cap);
    if (f->fail) return 0;
    f->labels[f->nlabels] = UINT32_MAX;
    return f->nlabels++;
}

static void wah_jit_bind(wah_jit_fn_t *f, uint32_t label) {
    if (!f->fail) f->labels[label] = wah_jit_pos(f);
}

static void wah_jit_add_patch(wah_jit_fn_t *f, uint8_t kind, uint32_t label, uint32_t base, uint32_t pos) {
    WAH_JIT_GROW(f, patches, npatches, patches_cap);
    if (f->fail) return;
    f->patches[f->npatches++] = (wah_jit_patch_t){ .pos = pos, .label = label, .base = base, .kind = kind };
}

static void wah_jit_patch_here(wah_jit_fn_t *f, uint8_t kind, uint32_t label, uint32_t base) {
    wah_jit_add_patch(f, kind, label, base, wah_jit_pos(f));
}

#if defined(WAH_JIT_X86_64)

////////////////////////////////////////////////////////////////////////////////
// x86-64 encoder //////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

enum {
    WAH_X64_RAX, WAH_X64_RCX, WAH_X64_RDX, WAH_X64_RBX, WAH_X64_RSP, WAH_X64_RBP, WAH_X64_RSI, WAH_X64_RDI,
    WAH_X64_R8, WAH_X64_R9, WAH_X64_R10, WAH_X64_R11, WAH_X64_R12, WAH_X64_R13, WAH_X64_R14, WAH_X64_R15,
};
enum {
    WAH_X64_O, WAH_X64_NO, WAH_X64_B, WAH_X64_AE, WAH_X64_E, WAH_X64_NE, WAH_X64_BE, WAH_X64_A,
    WAH_X64_S, WAH_X64_NS, WAH_X64_P, WAH_X64_NP, WAH_X64_L, WAH_X64_GE, WAH_X64_LE, WAH_X64_G,
};
#define WAH_X64_JMP (-1) // Unconditional, for wah_x64_jcc8
#define WAH_X64_RIP (-1) // RIP-relative base, for wah_x64_mem

#define WAH_JR_RET WAH_X64_RAX
#define WAH_JR_CTX WAH_X64_RBX
#define WAH_JR_FP WAH_X64_RBP
#define WAH_JR_MEM WAH_X64_R12
#define WAH_JR_MEMSZ WAH_X64_R13
#define WAH_JR_LIMIT WAH_X64_R14
#define WAH_JR_GLOBALS WAH_X64_R15
#define WAH_JR_T0 WAH_X64_R11
#define WAH_JR_T1 WAH_X64_R10
#define WAH_JR_VT0 15
#define WAH_JR_VT1 14

#define WAH_JC_EQ WAH_X64_E
#define WAH_JC_NE WAH_X64_NE
#define WAH_JC_LT_S WAH_X64_L
#define WAH_JC_LT_U WAH_X64_B
#define WAH_JC_GT_S WAH_X64_G
#define WAH_JC_GT_U WAH_X64_A
#define WAH_JC_LE_S WAH_X64_LE
#define WAH_JC_LE_U WAH_X64_BE
#define WAH_JC_GE_S WAH_X64_GE
#define WAH_JC_GE_U WAH_X64_AE

// Native stack frame of the trampoline, where rsp stays during the whole activation.
// It holds the MXCSR of the host followed by that of the JIT, and on Windows, the shadow space,
// a fifth argument (32) and saved xmm6-xmm15 (48-207) as well.
#ifdef _WIN32
#define WAH_X64_FRAME_SIZE 216
#define WAH_X64_MXCSR_OFF 40
#else
#define WAH_X64_FRAME_SIZE 8
#define WAH_X64_MXCSR_OFF 0
#endif

// Opcodes are up to 3 bytes, with an optional mandatory prefix (0x66, 0xF2 or 0xF3) in bits 24-31.
// Most instructions take a register in the reg field and another register or memory in the r/m field.
#define WAH_X64_W  1u // REX.W
#define WAH_X64_B8 2u // Byte registers, where 4-7 need REX to be SPL/BPL/SIL/DIL

#define WAH_X64_ADD 0x03u // reg += r/m, and so on
#define WAH_X64_OR  0x0Bu
#define WAH_X64_AND 0x23u
#define WAH_X64_SUB 0x2Bu
#define WAH_X64_XOR 0x33u
#define WAH_X64_CMP 0x3Bu
#define WAH_X64_TEST 0x85u
#define WAH_X64_MOV_STORE8 0x88u
#define WAH_X64_MOV_STORE 0x89u
#define WAH_X64_MOV_LOAD 0x8Bu
#define WAH_X64_LEA 0x8Du
#define WAH_X64_MOVSXD 0x63u
#define WAH_X64_IMUL 0x0FAFu
#define WAH_X64_MOVZX8 0x0FB6u
#define WAH_X64_MOVZX16 0x0FB7u
#define WAH_X64_MOVSX8 0x0FBEu
#define WAH_X64_MOVSX16 0x0FBFu
#define WAH_X64_BSF 0x0FBCu
#define WAH_X64_BSR 0x0FBDu
#define WAH_X64_TZCNT 0xF3000FBCu
#define WAH_X64_LZCNT 0xF3000FBDu
#define WAH_X64_POPCNT 0xF3000FB8u
#define WAH_X64_CMOV(cc) (0x0F40u + (uint32_t)(cc))
#define WAH_X64_SETCC(cc) (0x0F90u + (uint32_t)(cc))
// Opcodes with a digit in the reg field
#define WAH_X64_GRP1_IMM32 0x81u // add 0, or 1, and 4, sub 5, xor 6, cmp 7
#define WAH_X64_GRP1_IMM8 0x83u
#define WAH_X64_GRP2_IMM8 0xC1u  // rol 0, ror 1, shl 4, shr 5, sar 7
#define WAH_X64_GRP2_CL 0xD3u
#define WAH_X64_GRP3 0xF7u       // not 2, neg 3, div 6, idiv 7
#define WAH_X64_GRP5 0xFFu       // call 2, jmp 4, push 6
#define WAH_X64_GRP8 0x0FBAu     // bts 5, btc 7
#define WAH_X64_MOV_IMM 0xC7u    // 0
#define WAH_X64_MXCSR 0x0FAEu    // ldmxcsr 2, stmxcsr 3
// SSE, where scalar instructions take WAH_X64_SS or WAH_X64_SD as a prefix
#define WAH_X64_SS 0xF3000000u
#define WAH_X64_SD 0xF2000000u
#define WAH_X64_FP(type) ((type) ? WAH_X64_SD : WAH_X64_SS)
#define WAH_X64_MOVS_LOAD 0x0F10u
#define WAH_X64_MOVS_STORE 0x0F11u
#define WAH_X64_CVTSI2S 0x0F2Au
#define WAH_X64_CVTTS2SI 0x0F2Cu
#define WAH_X64_SQRTS 0x0F51u
#define WAH_X64_ADDS 0x0F58u
#define WAH_X64_MULS 0x0F59u
#define WAH_X64_CVTS 0x0F5Au // From the type of the prefix to the other
#define WAH_X64_SUBS 0x0F5Cu
#define WAH_X64_MINS 0x0F5Du
#define WAH_X64_DIVS 0x0F5Eu
#define WAH_X64_MAXS 0x0F5Fu
#define WAH_X64_MOVAPS_LOAD 0x0F28u
#define WAH_X64_MOVAPS_STORE 0x0F29u
#define WAH_X64_ANDPS 0x0F54u
#define WAH_X64_ANDNPS 0x0F55u
#define WAH_X64_ORPS 0x0F56u
#define WAH_X64_XORPS 0x0F57u
#define WAH_X64_MOVQ_TO_XMM 0x66000F6Eu   // With REX.W
#define WAH_X64_MOVQ_FROM_XMM 0x66000F7Eu // With REX.W, the XMM register in the reg field
#define WAH_X64_UCOMIS(type) ((type) ? 0x66000F2Eu : 0x0F2Eu)
#define WAH_X64_ROUNDS(type) ((type) ? 0x660F3A0Bu : 0x660F3A0Au) // SSE4.1

static void wah_x64_byte(wah_jit_fn_t *f, uint32_t b) {
    uint8_t v = (uint8_t)b;
    wah_jit_emit(f, &v, 1);
}

static void wah_x64_imm32(wah_jit_fn_t *f, uint32_t v) { wah_jit_emit(f, &v, 4); }

static void wah_x64_opcode(wah_jit_fn_t *f, uint32_t op, uint32_t flags, int reg, int index, int base, bool force_rex) {
    if (op >> 24) wah_x64_byte(f, op >> 24);
    uint32_t rex = (flags & WAH_X64_W ? 8u : 0u) | (reg & 8 ? 4u : 0u) | (index & 8 ? 2u : 0u) | (base & 8 ? 1u : 0u);
    if (rex || force_rex) wah_x64_byte(f, 0x40 | rex);
    if (op & 0xff0000u) wah_x64_byte(f, op >> 16);
    if (op & 0xff00u) wah_x64_byte(f, op >> 8);
    wah_x64_byte(f, op);
}

static bool wah_x64_byte_hi(int r) { return r >= 4 && r < 8; }

// Register operands
static void wah_x64_rr(wah_jit_fn_t *f, uint32_t op, uint32_t flags, int reg, int rm) {
    wah_x64_opcode(f, op, flags, reg, 0, rm, (flags & WAH_X64_B8) && (wah_x64_byte_hi(reg) || wah_x64_byte_hi(rm)));
    wah_x64_byte(f, 0xC0u | (uint32_t)(reg & 7) << 3 | (uint32_t)(rm & 7));
}

// A memory operand [base + index * (1 << scale) + disp], without index if negative.
// With WAH_X64_RIP as base, disp is the last 4 bytes so that it can be patched.
static void wah_x64_mem(wah_jit_fn_t *f, uint32_t op, uint32_t flags, int reg, int base, int index, int scale, int32_t disp) {
    wah_x64_opcode(f, op, flags, reg, index < 0 ? 0 : index, base < 0 ? 0 : base, (flags & WAH_X64_B8) && wah_x64_byte_hi(reg));
    uint32_t r = (uint32_t)(reg & 7) << 3;
    if (base < 0) {
        wah_x64_byte(f, 0x05 | r);
        wah_x64_imm32(f, (uint32_t)disp);
        return;
    }
    uint32_t mod = disp == 0 && (base & 7) != 5 ? 0x00 : disp >= -128 && disp < 128 ? 0x40 : 0x80;
    if (index >= 0 || (base & 7) == 4) {
        wah_x64_byte(f, mod | r | 4);
        wah_x64_byte(f, (uint32_t)scale << 6 | (uint32_t)((index < 0 ? 4 : index) & 7) << 3 | (uint32_t)(base & 7));
    } else {
        wah_x64_byte(f, mod | r | (uint32_t)(base & 7));
    }
    if (mod == 0x40) wah_x64_byte(f, (uint32_t)disp);
    else if (mod == 0x80) wah_x64_imm32(f, (uint32_t)disp);
}

static void wah_x64_mov(wah_jit_fn_t *f, bool w, int rd, int rs) {
    wah_x64_rr(f, WAH_X64_MOV_LOAD, w ? WAH_X64_W : 0, rd, rs);
}

// GRP1 with the shortest immediate
static void wah_x64_alu_imm(wah_jit_fn_t *f, int digit, uint32_t flags, int rm, int32_t imm) {
    if (imm >= -128 && imm < 128) {
        wah_x64_rr(f, WAH_X64_GRP1_IMM8, flags, digit, rm);
        wah_x64_byte(f, (uint32_t)imm);
    } else {
        wah_x64_rr(f, WAH_X64_GRP1_IMM32, flags, digit, rm);
        wah_x64_imm32(f, (uint32_t)imm);
    }
}

static void wah_x64_shift_imm(wah_jit_fn_t *f, int digit, uint32_t flags, int rm, uint32_t s) {
    wah_x64_rr(f, WAH_X64_GRP2_IMM8, flags, digit, rm);
    wah_x64_byte(f, s);
}

// Short forward jumps within an instruction, bound by wah_x64_bind8
static uint32_t wah_x64_jcc8(wah_jit_fn_t *f, int cc) {
    wah_x64_byte(f, cc < 0 ? 0xEBu : 0x70u + (uint32_t)cc);
    wah_x64_byte(f, 0);
    return wah_jit_pos(f);
}

static void wah_x64_bind8(wah_jit_fn_t *f, uint32_t pos) {
    uint32_t d = wah_jit_pos(f) - pos;
    if (f->fail) return;
    if (d > 127) { WAH_JIT_FAIL(f, "short jump out of range"); return; }
    f->jm->buf[pos - 1] = (uint8_t)d;
}

static void wah_x64_call_abs(wah_jit_fn_t *f, uintptr_t fn);

// Primitives used by the shared code ///////////////////////////////////////////

// rax, rcx, rdx, rsi, rdi, r8, r9 and xmm0-xmm13 are allocatable
static void wah_jit_init_regs(wah_jit_fn_t *f) {
    for (int r = 0; r < 32; r++) {
        f->gpr[r] = r <= WAH_X64_RDX || r == WAH_X64_RSI || r == WAH_X64_RDI || r == WAH_X64_R8 || r == WAH_X64_R9
                    ? WAH_JO_FREE : WAH_JO_NONE;
        f->fpr[r] = r < 14 ? WAH_JO_FREE : WAH_JO_NONE;
    }
}

// Loads or stores a 32-bit or 64-bit (wide) value at [base + off].
static void wah_jit_ldst(wah_jit_fn_t *f, bool store, bool fp, bool wide, int r, int base, uint64_t off) {
    if (off > INT32_MAX) { WAH_JIT_FAIL(f, "offset too large"); return; }
    if (fp) wah_x64_mem(f, WAH_X64_FP(wide) | (store ? WAH_X64_MOVS_STORE : WAH_X64_MOVS_LOAD), 0, r, base, -1, 0, (int32_t)off);
    else wah_x64_mem(f, store ? WAH_X64_MOV_STORE : WAH_X64_MOV_LOAD, wide ? WAH_X64_W : 0, r, base, -1, 0, (int32_t)off);
}

// Doesn't change flags.
static void wah_jit_mov_imm(wah_jit_fn_t *f, int rd, uint64_t v) {
    if (v <= UINT32_MAX) { // Zero-extended
        if (rd & 8) wah_x64_byte(f, 0x41);
        wah_x64_byte(f, 0xB8u + (uint32_t)(rd & 7));
        wah_x64_imm32(f, (uint32_t)v);
    } else if ((int64_t)v == (int32_t)v) { // Sign-extended
        wah_x64_rr(f, WAH_X64_MOV_IMM, WAH_X64_W, 0, rd);
        wah_x64_imm32(f, (uint32_t)v);
    } else {
        wah_x64_byte(f, 0x48u | (rd & 8 ? 1u : 0u));
        wah_x64_byte(f, 0xB8u + (uint32_t)(rd & 7));
        wah_jit_emit(f, &v, 8);
    }
}

static void wah_jit_store_imm(wah_jit_fn_t *f, int base, uint64_t off, int64_t imm) {
    if (imm == (int32_t)imm && off <= INT32_MAX) {
        wah_x64_mem(f, WAH_X64_MOV_IMM, WAH_X64_W, 0, base, -1, 0, (int32_t)off);
        wah_x64_imm32(f, (uint32_t)imm);
    } else {
        wah_jit_mov_imm(f, WAH_JR_T0, (uint64_t)imm);
        wah_jit_ldst(f, true, false, true, WAH_JR_T0, base, off);
    }
}

static void wah_jit_setcc(wah_jit_fn_t *f, int rd, int cond) {
    wah_x64_rr(f, WAH_X64_SETCC(cond), WAH_X64_B8, 0, rd);
    wah_x64_rr(f, WAH_X64_MOVZX8, WAH_X64_B8, rd, rd);
}

static void wah_jit_gpr_to_fpr(wah_jit_fn_t *f, int vd, int rn) { wah_x64_rr(f, WAH_X64_MOVQ_TO_XMM, WAH_X64_W, vd, rn); }
static void wah_jit_fpr_to_gpr(wah_jit_fn_t *f, int rd, int vn) { wah_x64_rr(f, WAH_X64_MOVQ_FROM_XMM, WAH_X64_W, vn, rd); }

static void wah_jit_b(wah_jit_fn_t *f, uint32_t label) {
    wah_x64_byte(f, 0xE9);
    wah_jit_patch_here(f, WAH_JP_X64_REL32, label, 0);
    wah_x64_imm32(f, 0);
}

static void wah_jit_bcond(wah_jit_fn_t *f, int cond, uint32_t label) {
    wah_x64_byte(f, 0x0F);
    wah_x64_byte(f, 0x80u + (uint32_t)cond);
    wah_jit_patch_here(f, WAH_JP_X64_REL32, label, 0);
    wah_x64_imm32(f, 0);
}

// Branches if rt is zero, or nonzero if given.
static void wah_jit_cbz(wah_jit_fn_t *f, bool nonzero, bool sf, int rt, uint32_t label) {
    wah_x64_rr(f, WAH_X64_TEST, sf ? WAH_X64_W : 0, rt, rt);
    wah_jit_bcond(f, nonzero ? WAH_X64_NE : WAH_X64_E, label);
}

static void wah_jit_adr(wah_jit_fn_t *f, int rd, uint32_t label) {
    wah_x64_mem(f, WAH_X64_LEA, WAH_X64_W, rd, WAH_X64_RIP, -1, 0, 0);
    if (!f->fail) wah_jit_add_patch(f, WAH_JP_X64_REL32, label, 0, wah_jit_pos(f) - 4);
}

static void wah_jit_exit(wah_jit_fn_t *f) {
    wah_x64_byte(f, 0xE9);
    wah_x64_imm32(f, f->jm->exit_pos - (wah_jit_pos(f) + 4));
}

static void wah_x64_call_abs(wah_jit_fn_t *f, uintptr_t fn) {
    wah_jit_mov_imm(f, WAH_X64_RAX, (uint64_t)fn);
    wah_x64_rr(f, WAH_X64_GRP5, 0, 2, WAH_X64_RAX);
}

#elif defined(WAH_JIT_AARCH64)

////////////////////////////////////////////////////////////////////////////////
// AArch64 encoder /////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

enum {
    WAH_A64_EQ, WAH_A64_NE, WAH_A64_HS, WAH_A64_LO, WAH_A64_MI, WAH_A64_PL, WAH_A64_VS, WAH_A64_VC,
    WAH_A64_HI, WAH_A64_LS, WAH_A64_GE, WAH_A64_LT, WAH_A64_GT, WAH_A64_LE,
};
#define WAH_A64_ZR 31
#define WAH_A64_SP 31

#define WAH_JR_RET 0
#define WAH_JR_CTX 19
#define WAH_JR_FP 20
#define WAH_JR_MEM 21
#define WAH_JR_MEMSZ 22
#define WAH_JR_LIMIT 23
#define WAH_JR_GLOBALS 24
#define WAH_JR_FPCR_HOST 25
#define WAH_JR_FPCR_JIT 26
#define WAH_JR_T0 16
#define WAH_JR_T1 17
#define WAH_JR_VT0 30
#define WAH_JR_VT1 31

#define WAH_JC_EQ WAH_A64_EQ
#define WAH_JC_NE WAH_A64_NE
#define WAH_JC_LT_S WAH_A64_LT
#define WAH_JC_LT_U WAH_A64_LO
#define WAH_JC_GT_S WAH_A64_GT
#define WAH_JC_GT_U WAH_A64_HI
#define WAH_JC_LE_S WAH_A64_LE
#define WAH_JC_LE_U WAH_A64_LS
#define WAH_JC_GE_S WAH_A64_GE
#define WAH_JC_GE_U WAH_A64_HS

// Load/store opcodes with an unsigned offset, and the log2 of their access size
#define WAH_A64_STRB   0x39000000u
#define WAH_A64_LDRB   0x39400000u
#define WAH_A64_LDRSBX 0x39800000u
#define WAH_A64_LDRSBW 0x39C00000u
#define WAH_A64_STRH   0x79000000u
#define WAH_A64_LDRH   0x79400000u
#define WAH_A64_LDRSHX 0x79800000u
#define WAH_A64_LDRSHW 0x79C00000u
#define WAH_A64_STRW   0xB9000000u
#define WAH_A64_LDRW   0xB9400000u
#define WAH_A64_LDRSW  0xB9800000u
#define WAH_A64_STRX   0xF9000000u
#define WAH_A64_LDRX   0xF9400000u
#define WAH_A64_STRS   0xBD000000u
#define WAH_A64_LDRS   0xBD400000u
#define WAH_A64_STRD   0xFD000000u
#define WAH_A64_LDRD   0xFD400000u
#define WAH_A64_LDST_SIZE(op) ((op) >> 30) // Except for 128-bit ones, which aren't used

static void wah_a64(wah_jit_fn_t *f, uint32_t insn) { wah_jit_emit(f, &insn, 4); }

static uint32_t wah_jit_get32(const wah_jit_module_t *jm, uint32_t pos) {
    uint32_t v;
    memcpy(&v, jm->buf + pos, 4);
    return v;
}

#define WAH_A64_SF(sf) ((uint32_t)!!(sf) << 31)

static void wah_a64_movw(wah_jit_fn_t *f, uint32_t base, bool sf, int rd, uint32_t imm16, int hw) {
    wah_a64(f, base | WAH_A64_SF(sf) | (uint32_t)hw << 21 | (imm16 & 0xffff) << 5 | (uint32_t)rd);
}
#define WAH_A64_MOVN 0x12800000u
#define WAH_A64_MOVZ 0x52800000u
#define WAH_A64_MOVK 0x72800000u

static void wah_a64_mov_imm(wah_jit_fn_t *f, bool sf, int rd, uint64_t v) {
    int n = sf ? 4 : 2, zeros = 0, ones = 0;
    if (!sf) v &= 0xffffffffu;
    for (int i = 0; i < n; i++) {
        uint32_t hw = (uint32_t)(v >> (16 * i)) & 0xffff;
        zeros += hw == 0;
        ones += hw == 0xffff;
    }
    bool inv = ones > zeros;
    uint32_t skip = inv ? 0xffff : 0;
    int first = 0;
    while (first < n - 1 && ((uint32_t)(v >> (16 * first)) & 0xffff) == skip) first++;
    if (((uint32_t)(v >> (16 * first)) & 0xffff) == skip) first = 0; // All halfwords are the same
    uint32_t hw0 = (uint32_t)(v >> (16 * first)) & 0xffff;
    wah_a64_movw(f, inv ? WAH_A64_MOVN : WAH_A64_MOVZ, sf, rd, inv ? ~hw0 : hw0, first);
    for (int i = first + 1; i < n; i++) {
        uint32_t hw = (uint32_t)(v >> (16 * i)) & 0xffff;
        if (hw != skip) wah_a64_movw(f, WAH_A64_MOVK, sf, rd, hw, i);
    }
}

// ADD/SUB (immediate): 0x11000000 add, 0x31000000 adds, 0x51000000 sub, 0x71000000 subs
static void wah_a64_addsub_imm(wah_jit_fn_t *f, uint32_t base, bool sf, int rd, int rn, uint32_t imm12, bool lsl12) {
    wah_a64(f, base | WAH_A64_SF(sf) | (uint32_t)lsl12 << 22 | (imm12 & 0xfff) << 10 | (uint32_t)rn << 5 | (uint32_t)rd);
}
#define WAH_A64_ADDI  0x11000000u
#define WAH_A64_ADDSI 0x31000000u
#define WAH_A64_SUBI  0x51000000u
#define WAH_A64_SUBSI 0x71000000u

// Register forms with a shift amount of zero
static void wah_a64_rrr(wah_jit_fn_t *f, uint32_t base, bool sf, int rd, int rn, int rm) {
    wah_a64(f, base | WAH_A64_SF(sf) | (uint32_t)rm << 16 | (uint32_t)rn << 5 | (uint32_t)rd);
}
#define WAH_A64_ADD  0x0B000000u
#define WAH_A64_ADDS 0x2B000000u
#define WAH_A64_SUB  0x4B000000u
#define WAH_A64_SUBS 0x6B000000u
#define WAH_A64_AND  0x0A000000u
#define WAH_A64_ORR  0x2A000000u
#define WAH_A64_EOR  0x4A000000u
#define WAH_A64_UDIV 0x1AC00800u
#define WAH_A64_SDIV 0x1AC00C00u
#define WAH_A64_LSLV 0x1AC02000u
#define WAH_A64_LSRV 0x1AC02400u
#define WAH_A64_ASRV 0x1AC02800u
#define WAH_A64_RORV 0x1AC02C00u
#define WAH_A64_MUL  0x1B007C00u // MADD with ra = zr
#define WAH_A64_RBIT 0x5AC00000u // rm is ignored
#define WAH_A64_CLZ  0x5AC01000u

static void wah_a64_mov(wah_jit_fn_t *f, bool sf, int rd, int rm) { wah_a64_rrr(f, WAH_A64_ORR, sf, rd, WAH_A64_ZR, rm); }

static void wah_a64_msub(wah_jit_fn_t *f, bool sf, int rd, int rn, int rm, int ra) {
    wah_a64(f, 0x1B008000u | WAH_A64_SF(sf) | (uint32_t)rm << 16 | (uint32_t)ra << 10 | (uint32_t)rn << 5 | (uint32_t)rd);
}

// rd = rn + imm, also for large immediates, using x16 if needed
static void wah_a64_add_imm(wah_jit_fn_t *f, int rd, int rn, uint64_t imm) {
    if (imm < 4096) {
        if (imm || rd != rn) wah_a64_addsub_imm(f, WAH_A64_ADDI, true, rd, rn, (uint32_t)imm, false);
    } else if (imm < ((uint64_t)1 << 24)) {
        wah_a64_addsub_imm(f, WAH_A64_ADDI, true, rd, rn, (uint32_t)(imm >> 12), true);
        if (imm & 0xfff) wah_a64_addsub_imm(f, WAH_A64_ADDI, true, rd, rd, (uint32_t)(imm & 0xfff), false);
    } else {
        wah_a64_mov_imm(f, true, WAH_JR_T0, imm);
        wah_a64_rrr(f, WAH_A64_ADD, true, rd, rn, WAH_JR_T0);
    }
}

static void wah_a64_sub_imm(wah_jit_fn_t *f, int rd, int rn, uint64_t imm) {
    if (imm < 4096) {
        if (imm || rd != rn) wah_a64_addsub_imm(f, WAH_A64_SUBI, true, rd, rn, (uint32_t)imm, false);
    } else if (imm < ((uint64_t)1 << 24)) {
        wah_a64_addsub_imm(f, WAH_A64_SUBI, true, rd, rn, (uint32_t)(imm >> 12), true);
        if (imm & 0xfff) wah_a64_addsub_imm(f, WAH_A64_SUBI, true, rd, rd, (uint32_t)(imm & 0xfff), false);
    } else {
        wah_a64_mov_imm(f, true, WAH_JR_T0, imm);
        wah_a64_rrr(f, WAH_A64_SUB, true, rd, rn, WAH_JR_T0);
    }
}

// SBFM/UBFM, with N = sf
static void wah_a64_bfm(wah_jit_fn_t *f, bool is_signed, bool sf, int rd, int rn, uint32_t immr, uint32_t imms) {
    wah_a64(f, (is_signed ? 0x13000000u : 0x53000000u) | WAH_A64_SF(sf) | (uint32_t)!!sf << 22 |
               immr << 16 | imms << 10 | (uint32_t)rn << 5 | (uint32_t)rd);
}

static void wah_a64_extr(wah_jit_fn_t *f, bool sf, int rd, int rn, int rm, uint32_t lsb) {
    wah_a64(f, 0x13800000u | WAH_A64_SF(sf) | (uint32_t)!!sf << 22 | (uint32_t)rm << 16 | lsb << 10 |
               (uint32_t)rn << 5 | (uint32_t)rd);
}

static void wah_a64_csel(wah_jit_fn_t *f, bool sf, int rd, int rn, int rm, int cond) {
    wah_a64(f, 0x1A800000u | WAH_A64_SF(sf) | (uint32_t)rm << 16 | (uint32_t)cond << 12 | (uint32_t)rn << 5 | (uint32_t)rd);
}

static void wah_a64_cset(wah_jit_fn_t *f, int rd, int cond) { // CSINC rd, wzr, wzr, !cond
    wah_a64(f, 0x1A9F07E0u | (uint32_t)(cond ^ 1) << 12 | (uint32_t)rd);
}

static void wah_a64_ldst(wah_jit_fn_t *f, uint32_t op, int rt, int rn, uint64_t off) {
    uint32_t sz = WAH_A64_LDST_SIZE(op);
    if ((off & ((1u << sz) - 1)) == 0 && (off >> sz) < 4096) {
        wah_a64(f, op | (uint32_t)(off >> sz) << 10 | (uint32_t)rn << 5 | (uint32_t)rt);
    } else {
        wah_a64_mov_imm(f, true, WAH_JR_T1, off);
        wah_a64_rrr(f, WAH_A64_ADD, true, WAH_JR_T1, rn, WAH_JR_T1);
        wah_a64(f, op | (uint32_t)WAH_JR_T1 << 5 | (uint32_t)rt);
    }
}

// [rn, xm]
static void wah_a64_ldst_reg(wah_jit_fn_t *f, uint32_t op, int rt, int rn, int rm) {
    wah_a64(f, (op & ~0x01000000u) | 0x00206800u | (uint32_t)rm << 16 | (uint32_t)rn << 5 | (uint32_t)rt);
}

// FP data processing, where type is 0 for single and 1 for double precision
static void wah_a64_fp2(wah_jit_fn_t *f, uint32_t base, int type, int rd, int rn, int rm) {
    wah_a64(f, base | (uint32_t)type << 22 | (uint32_t)rm << 16 | (uint32_t)rn << 5 | (uint32_t)rd);
}
#define WAH_A64_FMUL 0x1E200800u
#define WAH_A64_FDIV 0x1E201800u
#define WAH_A64_FADD 0x1E202800u
#define WAH_A64_FSUB 0x1E203800u
#define WAH_A64_FMAX 0x1E204800u
#define WAH_A64_FMIN 0x1E205800u
#define WAH_A64_FCMP 0x1E202000u // rd = 0
#define WAH_A64_FMOV   0x1E204000u // rm = 0 for the rest
#define WAH_A64_FABS   0x1E20C000u
#define WAH_A64_FNEG   0x1E214000u
#define WAH_A64_FSQRT  0x1E21C000u
#define WAH_A64_FRINTN 0x1E244000u
#define WAH_A64_FRINTP 0x1E24C000u
#define WAH_A64_FRINTM 0x1E254000u
#define WAH_A64_FRINTZ 0x1E25C000u

static void wah_a64_fcsel(wah_jit_fn_t *f, int type, int rd, int rn, int rm, int cond) {
    wah_a64(f, 0x1E200C00u | (uint32_t)type << 22 | (uint32_t)rm << 16 | (uint32_t)cond << 12 | (uint32_t)rn << 5 | (uint32_t)rd);
}

// Conversions between general and FP registers: no rm, sf and type as given
static void wah_a64_fcvt_gp(wah_jit_fn_t *f, uint32_t base, bool sf, int type, int rd, int rn) {
    wah_a64(f, base | WAH_A64_SF(sf) | (uint32_t)type << 22 | (uint32_t)rn << 5 | (uint32_t)rd);
}
#define WAH_A64_SCVTF  0x1E220000u
#define WAH_A64_UCVTF  0x1E230000u
#define WAH_A64_FCVTZS 0x1E380000u
#define WAH_A64_FCVTZU 0x1E390000u
#define WAH_A64_FMOV_TO_GP   0x1E260000u // Same size only: w <- s, x <- d
#define WAH_A64_FMOV_FROM_GP 0x1E270000u

static void wah_a64_fmov_to_gp(wah_jit_fn_t *f, bool sf, int rd, int vn) { wah_a64_fcvt_gp(f, WAH_A64_FMOV_TO_GP, sf, sf, rd, vn); }
static void wah_a64_fmov_from_gp(wah_jit_fn_t *f, bool sf, int vd, int rn) { wah_a64_fcvt_gp(f, WAH_A64_FMOV_FROM_GP, sf, sf, vd, rn); }

static void wah_a64_b_to(wah_jit_fn_t *f, uint32_t target, bool link) {
    int64_t d = ((int64_t)target - (int64_t)wah_jit_pos(f)) / 4;
    if (d < -(1 << 25) || d >= (1 << 25)) WAH_JIT_FAIL(f, "branch out of range");
    wah_a64(f, (link ? 0x94000000u : 0x14000000u) | ((uint32_t)d & 0x3ffffff));
}

static void wah_a64_call_abs(wah_jit_fn_t *f, uintptr_t fn) {
    wah_a64_mov_imm(f, true, WAH_JR_T0, (uint64_t)fn);
    wah_a64(f, 0xD63F0000u | (uint32_t)WAH_JR_T0 << 5); // BLR x16
}

// Primitives used by the shared code ///////////////////////////////////////////

// x0-x15, v0-v7 and v16-v29 are allocatable
static void wah_jit_init_regs(wah_jit_fn_t *f) {
    for (int r = 0; r < 32; r++) {
        f->gpr[r] = r < 16 ? WAH_JO_FREE : WAH_JO_NONE;
        f->fpr[r] = r < 8 || (r >= 16 && r < 30) ? WAH_JO_FREE : WAH_JO_NONE;
    }
}

// Loads or stores a 32-bit or 64-bit (wide) value at [base + off].
static void wah_jit_ldst(wah_jit_fn_t *f, bool store, bool fp, bool wide, int r, int base, uint64_t off) {
    uint32_t op = fp ? (wide ? (store ? WAH_A64_STRD : WAH_A64_LDRD) : (store ? WAH_A64_STRS : WAH_A64_LDRS))
                     : (wide ? (store ? WAH_A64_STRX : WAH_A64_LDRX) : (store ? WAH_A64_STRW : WAH_A64_LDRW));
    wah_a64_ldst(f, op, r, base, off);
}

static void wah_jit_mov_imm(wah_jit_fn_t *f, int rd, uint64_t v) { wah_a64_mov_imm(f, true, rd, v); }

static void wah_jit_store_imm(wah_jit_fn_t *f, int base, uint64_t off, int64_t imm) {
    if (imm == 0) {
        wah_a64_ldst(f, WAH_A64_STRX, WAH_A64_ZR, base, off);
    } else {
        wah_a64_mov_imm(f, true, WAH_JR_T0, (uint64_t)imm);
        wah_a64_ldst(f, WAH_A64_STRX, WAH_JR_T0, base, off);
    }
}

static void wah_jit_setcc(wah_jit_fn_t *f, int rd, int cond) { wah_a64_cset(f, rd, cond); }
static void wah_jit_gpr_to_fpr(wah_jit_fn_t *f, int vd, int rn) { wah_a64_fmov_from_gp(f, true, vd, rn); }
static void wah_jit_fpr_to_gpr(wah_jit_fn_t *f, int rd, int vn) { wah_a64_fmov_to_gp(f, true, rd, vn); }

static void wah_jit_b(wah_jit_fn_t *f, uint32_t label) {
    wah_jit_patch_here(f, WAH_JP_A64_B26, label, 0);
    wah_a64(f, 0x14000000u);
}

static void wah_jit_bcond(wah_jit_fn_t *f, int cond, uint32_t label) {
    if (f->long_branches) { // Skip the following B unless cond
        wah_a64(f, 0x54000000u | 2u << 5 | (uint32_t)(cond ^ 1));
        wah_jit_b(f, label);
        return;
    }
    wah_jit_patch_here(f, WAH_JP_A64_B19, label, 0);
    wah_a64(f, 0x54000000u | (uint32_t)cond);
}

// Branches if rt is zero, or nonzero if given.
static void wah_jit_cbz(wah_jit_fn_t *f, bool nonzero, bool sf, int rt, uint32_t label) {
    if (f->long_branches) {
        wah_a64(f, (nonzero ? 0x34000000u : 0x35000000u) | WAH_A64_SF(sf) | 2u << 5 | (uint32_t)rt);
        wah_jit_b(f, label);
        return;
    }
    wah_jit_patch_here(f, WAH_JP_A64_B19, label, 0);
    wah_a64(f, (nonzero ? 0x35000000u : 0x34000000u) | WAH_A64_SF(sf) | (uint32_t)rt);
}

static void wah_jit_adr(wah_jit_fn_t *f, int rd, uint32_t label) {
    wah_jit_patch_here(f, WAH_JP_A64_ADR, label, 0);
    wah_a64(f, 0x10000000u | (uint32_t)rd);
}

static void wah_jit_exit(wah_jit_fn_t *f) { wah_a64_b_to(f, f->jm->exit_pos, false); }

#endif // WAH_JIT_AARCH64

////////////////////////////////////////////////////////////////////////////////
// Stubs and patches ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static void wah_jit_emit_stub(wah_jit_fn_t *f, const wah_jit_stub_t *s);

// Stubs are emitted at the end of the function, or right here with long branches so that ADR can reach `resume`.
// Then the code before branches here if the stub is to be run.
static void wah_jit_add_stub(wah_jit_fn_t *f, uint32_t stub, uint32_t resume, uint32_t cont, bool poll) {
    wah_jit_stub_t s = { .stub = stub, .resume = resume, .cont = cont, .poll = poll };
    if (f->long_branches) {
        uint32_t skip = wah_jit_new_label(f);
        wah_jit_b(f, skip);
        wah_jit_emit_stub(f, &s);
        wah_jit_bind(f, skip);
        return;
    }
    WAH_JIT_GROW(f, stubs, nstubs, stubs_cap);
    if (f->fail) return;
    f->stubs[f->nstubs++] = s;
}

static void wah_jit_resolve_patches(wah_jit_fn_t *f) {
    for (uint32_t i = 0; i < f->npatches && !f->fail; i++) {
        const wah_jit_patch_t *p = &f->patches[i];
        uint32_t target = f->labels[p->label];
        if (target == UINT32_MAX) { WAH_JIT_FAIL(f, "unbound label"); break; }
        int64_t d = (int64_t)target - (int64_t)p->pos;
        switch (p->kind) {
            case WAH_JP_TABLE:
                wah_jit_set32(f->jm, p->pos, (uint32_t)((int64_t)target - (int64_t)p->base));
                break;
#if defined(WAH_JIT_X86_64)
            case WAH_JP_X64_REL32: // Relative to the end of the field
                wah_jit_set32(f->jm, p->pos, (uint32_t)(d - 4));
                break;
#elif defined(WAH_JIT_AARCH64)
            case WAH_JP_A64_B26:
                d /= 4;
                if (d < -(1 << 25) || d >= (1 << 25)) WAH_JIT_FAIL(f, "branch out of range");
                wah_jit_set32(f->jm, p->pos, wah_jit_get32(f->jm, p->pos) | ((uint32_t)d & 0x3ffffff));
                break;
            case WAH_JP_A64_B19:
                d /= 4;
                if (d < -(1 << 18) || d >= (1 << 18)) { WAH_JIT_FAIL(f, "branch out of range"); f->out_of_range = true; }
                wah_jit_set32(f->jm, p->pos, wah_jit_get32(f->jm, p->pos) | ((uint32_t)d & 0x7ffff) << 5);
                break;
            case WAH_JP_A64_ADR:
                if (d < -(1 << 20) || d >= (1 << 20)) { WAH_JIT_FAIL(f, "adr out of range"); f->out_of_range = true; }
                wah_jit_set32(f->jm, p->pos, wah_jit_get32(f->jm, p->pos) |
                                             ((uint32_t)d & 3) << 29 | (((uint32_t)d >> 2) & 0x7ffff) << 5);
                break;
#endif
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// Operand stack ///////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static uint32_t wah_jit_slot_off(const wah_jit_fn_t *f, uint32_t i) { return (f->nlocals + 1 + i) * (uint32_t)sizeof(wah_value_t); }

// Stores the value at index i of the operand stack to [fp + off] without changing its state.
// This doesn't change flags.
static void wah_jit_copy_val(wah_jit_fn_t *f, uint32_t i, uint32_t off) {
    const wah_jit_val_t *v = &f->vals[i];
    switch (v->kind) {
        case WAH_JV_GPR: wah_jit_ldst(f, true, false, true, v->reg, WAH_JR_FP, off); break;
        case WAH_JV_FPR: wah_jit_ldst(f, true, true, true, v->reg, WAH_JR_FP, off); break;
        case WAH_JV_CONST: wah_jit_store_imm(f, WAH_JR_FP, off, v->imm); break;
        case WAH_JV_FLAGS:
            wah_jit_setcc(f, WAH_JR_T0, v->reg);
            wah_jit_ldst(f, true, false, true, WAH_JR_T0, WAH_JR_FP, off);
            break;
        case WAH_JV_SLOT:
            if (wah_jit_slot_off(f, i) != off) {
                wah_jit_ldst(f, false, false, true, WAH_JR_T0, WAH_JR_FP, wah_jit_slot_off(f, i));
                wah_jit_ldst(f, true, false, true, WAH_JR_T0, WAH_JR_FP, off);
            }
            break;
    }
}

static void wah_jit_spill(wah_jit_fn_t *f, uint32_t i) {
    wah_jit_val_t *v = &f->vals[i];
    if (v->kind == WAH_JV_SLOT) return;
    wah_jit_copy_val(f, i, wah_jit_slot_off(f, i));
    if (v->kind == WAH_JV_GPR) f->gpr[v->reg] = WAH_JO_FREE;
    if (v->kind == WAH_JV_FPR) f->fpr[v->reg] = WAH_JO_FREE;
    v->kind = WAH_JV_SLOT;
}

static void wah_jit_flush(wah_jit_fn_t *f) {
    for (uint32_t i = 0; i < f->h; i++) wah_jit_spill(f, i);
}

// Forgets register contents, as at a merge point where everything is in slots.
static void wah_jit_reset(wah_jit_fn_t *f, uint32_t h) {
    wah_jit_init_regs(f);
    for (uint32_t i = 0; i < h; i++) f->vals[i] = (wah_jit_val_t){ .kind = WAH_JV_SLOT };
    f->h = h;
}

// Returns a register reserved as WAH_JO_TEMP, spilling the deepest cached value if needed.
// Registers are tried from the highest, which on x86-64 avoids those with special uses.
static int wah_jit_alloc(wah_jit_fn_t *f, bool fp) {
    int32_t *owner = fp ? f->fpr : f->gpr;
    for (int r = 31; r >= 0; r--) {
        if (owner[r] == WAH_JO_FREE) { owner[r] = WAH_JO_TEMP; return r; }
    }
    for (uint32_t i = 0; i < f->h; i++) {
        wah_jit_val_t *v = &f->vals[i];
        if (v->kind == (fp ? WAH_JV_FPR : WAH_JV_GPR)) {
            int r = v->reg;
            wah_jit_spill(f, i);
            owner[r] = WAH_JO_TEMP;
            return r;
        }
    }
    WAH_JIT_FAIL(f, "out of registers"); // Too many temporaries, which shouldn't happen
    return fp ? WAH_JR_VT0 : WAH_JR_T0;
}

// Moves the value at index i into a general register, as raw 64 bits.
static int wah_jit_to_gpr(wah_jit_fn_t *f, uint32_t i) {
    wah_jit_val_t *v = &f->vals[i];
    if (v->kind == WAH_JV_GPR) return v->reg;
    int r = wah_jit_alloc(f, false);
    switch (v->kind) {
        case WAH_JV_FPR: wah_jit_fpr_to_gpr(f, r, v->reg); f->fpr[v->reg] = WAH_JO_FREE; break;
        case WAH_JV_CONST: wah_jit_mov_imm(f, r, (uint64_t)v->imm); break;
        case WAH_JV_FLAGS: wah_jit_setcc(f, r, v->reg); break;
        case WAH_JV_SLOT: wah_jit_ldst(f, false, false, true, r, WAH_JR_FP, wah_jit_slot_off(f, i)); break;
    }
    *v = (wah_jit_val_t){ .kind = WAH_JV_GPR, .reg = (uint8_t)r };
    f->gpr[r] = (int32_t)i;
    return r;
}

static int wah_jit_to_fpr(wah_jit_fn_t *f, uint32_t i) {
    wah_jit_val_t *v = &f->vals[i];
    if (v->kind == WAH_JV_FPR) return v->reg;
    int r = wah_jit_alloc(f, true);
    switch (v->kind) {
        case WAH_JV_GPR: wah_jit_gpr_to_fpr(f, r, v->reg); f->gpr[v->reg] = WAH_JO_FREE; break;
        case WAH_JV_CONST:
            wah_jit_mov_imm(f, WAH_JR_T0, (uint64_t)v->imm);
            wah_jit_gpr_to_fpr(f, r, WAH_JR_T0);
            break;
        case WAH_JV_FLAGS:
            wah_jit_setcc(f, WAH_JR_T0, v->reg);
            wah_jit_gpr_to_fpr(f, r, WAH_JR_T0);
            break;
        case WAH_JV_SLOT: wah_jit_ldst(f, false, true, true, r, WAH_JR_FP, wah_jit_slot_off(f, i)); break;
    }
    *v = (wah_jit_val_t){ .kind = WAH_JV_FPR, .reg = (uint8_t)r };
    f->fpr[r] = (int32_t)i;
    return r;
}

// Popped registers stay reserved until released, so that operands are not clobbered while being used.
static int wah_jit_pop_gpr(wah_jit_fn_t *f) {
    int r = wah_jit_to_gpr(f, f->h - 1);
    f->gpr[r] = WAH_JO_TEMP;
    f->h--;
    return r;
}

static int wah_jit_pop_fpr(wah_jit_fn_t *f) {
    int r = wah_jit_to_fpr(f, f->h - 1);
    f->fpr[r] = WAH_JO_TEMP;
    f->h--;
    return r;
}

static void wah_jit_free_gpr(wah_jit_fn_t *f, int r) { if (f->gpr[r] == WAH_JO_TEMP) f->gpr[r] = WAH_JO_FREE; }
static void wah_jit_free_fpr(wah_jit_fn_t *f, int r) { if (f->fpr[r] == WAH_JO_TEMP) f->fpr[r] = WAH_JO_FREE; }

static void wah_jit_push(wah_jit_fn_t *f, wah_jit_val_t v) {
    if (v.kind == WAH_JV_GPR) f->gpr[v.reg] = (int32_t)f->h;
    if (v.kind == WAH_JV_FPR) f->fpr[v.reg] = (int32_t)f->h;
    f->vals[f->h++] = v;
}
static void wah_jit_push_gpr(wah_jit_fn_t *f, int r) { wah_jit_push(f, (wah_jit_val_t){ .kind = WAH_JV_GPR, .reg = (uint8_t)r }); }
static void wah_jit_push_fpr(wah_jit_fn_t *f, int r) { wah_jit_push(f, (wah_jit_val_t){ .kind = WAH_JV_FPR, .reg = (uint8_t)r }); }

static void wah_jit_drop(wah_jit_fn_t *f) {
    wah_jit_val_t *v = &f->vals[--f->h];
    if (v->kind == WAH_JV_GPR) f->gpr[v->reg] = WAH_JO_FREE;
    if (v->kind == WAH_JV_FPR) f->fpr[v->reg] = WAH_JO_FREE;
}

// A condition for branches, either flags or a register to compare with zero.
typedef struct { bool flags; int cond, reg; } wah_jit_cond_t;

static wah_jit_cond_t wah_jit_pop_cond(wah_jit_fn_t *f) {
    wah_jit_val_t *v = &f->vals[f->h - 1];
    if (v->kind == WAH_JV_FLAGS) {
        f->h--;
        return (wah_jit_cond_t){ .flags = true, .cond = v->reg };
    }
    return (wah_jit_cond_t){ .flags = false, .reg = wah_jit_pop_gpr(f) };
}

static void wah_jit_branch_if(wah_jit_fn_t *f, wah_jit_cond_t c, bool when, uint32_t label) {
    if (c.flags) wah_jit_bcond(f, when ? c.cond : c.cond ^ 1, label);
    else wah_jit_cbz(f, when, false, c.reg, label);
}

static void wah_jit_release_cond(wah_jit_fn_t *f, wah_jit_cond_t c) { if (!c.flags) wah_jit_free_gpr(f, c.reg); }

////////////////////////////////////////////////////////////////////////////////
// Runtime helpers called from native code /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static wah_error_t wah_jit_poll(wah_exec_context_t *ctx) { return wah_poll_handler(ctx); }

static wah_error_t wah_jit_bulk_yield(wah_exec_context_t *ctx) {
    WAH_POLL_FLAG_STORE(ctx->interrupt_flag, 0);
    wah_recompute_poll_flag(ctx);
    return WAH_STATUS_YIELDED;
}

// Calls a host function with args in the value stack, where results replace them like the interpreter.
static wah_error_t wah_jit_host(wah_exec_context_t *ctx, const wah_function_t *fn, wah_value_t *args) {
    size_t nparams = fn->nparams, nresults = fn->nresults;
    wah_value_t *results = args + nparams;
    WAH_ENSURE((uint8_t *)(results + nresults) <= (uint8_t *)ctx->frame_ptr, WAH_ERROR_STACK_OVERFLOW);
    memset(results, 0, sizeof(wah_value_t) * nresults);
    ctx->sp = results;
    WAH_CHECK(wah_call_host_function_internal(ctx, fn, args, (uint32_t)nparams, results));
    if (nresults > 0) memmove(args, results, sizeof(wah_value_t) * nresults);
    return WAH_OK;
}

static wah_error_t wah_jit_call_host(wah_exec_context_t *ctx, uint32_t func_idx, wah_value_t *args) {
    return wah_jit_host(ctx, &ctx->function_table[func_idx].func, args);
}

// Returns the code of a wasm function to call, 1 if a host function was called, or 0 with ctx->jit->err on errors.
static uintptr_t wah_jit_call_indirect(wah_exec_context_t *ctx, uint32_t type_idx, uint32_t table_idx,
                                       uint32_t elem_idx, wah_value_t *args) {
    wah_error_t err = WAH_ERROR_TRAP;
    const wah_table_inst_t *table = &ctx->tables[table_idx];
    if (elem_idx >= table->size) goto fail;
    void *ref = table->entries[elem_idx].ref;
    if (!ref) goto fail;
    WAH_IF_GC(if (wah_ref_is_i31(ref) || wah_gc_header(ref)->repr_id != WAH_TYPE_FUNC) goto fail;)
    const wah_function_t *fn = wah_ref_to_func(ref);
    const wah_module_t *fn_module = fn->fn_module ? fn->fn_module : ctx->module;
    if (fn->is_host) {
        if (!wah_host_func_matches(ctx, fn, fn_module, ctx->module, type_idx)) goto fail;
        err = wah_jit_host(ctx, fn, args);
        if (err != WAH_OK) goto fail;
        return 1;
    }
    uint32_t actual_type_idx = fn_module->function_type_indices[fn->local_idx];
    if (!wah_cross_module_subtype_cached(ctx, fn_module, WAH_TYPE_FROM_IDX(actual_type_idx, 0),
                                         ctx->module, WAH_TYPE_FROM_IDX(type_idx, 0))) goto fail;
    // Other modules can't be reached without importing tables or functions from them
    err = WAH_ERROR_MISUSE;
    if (fn_module != ctx->module || (fn->fn_ctx && fn->fn_ctx != ctx)) goto fail;
    return (uintptr_t)(ctx->module->jit->code + ctx->module->jit->func_pos[fn->local_idx]);
fail:
    ctx->jit->err = err;
    return 0;
}

static int32_t wah_jit_memory_grow(wah_exec_context_t *ctx, uint32_t mem_idx, int32_t pages) {
    uint64_t old_pages;
    if (pages < 0 || !wah_memory_grow_internal(ctx, ctx, mem_idx, (uint64_t)pages, &old_pages)) return -1;
    return (int32_t)old_pages;
}

// Bulk operations take operands in the value stack, which are updated before yielding in the middle.
static wah_error_t wah_jit_memory_fill(wah_exec_context_t *ctx, uint32_t mem_idx, wah_value_t *ops) {
    if (wah_bulk_should_interrupt(ctx)) return wah_jit_bulk_yield(ctx);
    uint64_t dst = (uint32_t)ops[0].i32, size = (uint32_t)ops[2].i32;
    uint8_t val = (uint8_t)ops[1].i32;
    WAH_ENSURE(wah_u64_range_in_bounds(dst, size, ctx->memories[mem_idx].size), WAH_ERROR_MEMORY_OUT_OF_BOUNDS);
    uint64_t done = wah_memory_fill_internal(ctx, ctx->memories[mem_idx].data, dst, val, size);
    if (done == size) return WAH_OK;
    ops[0].i32 = (int32_t)(dst + done);
    ops[2].i32 = (int32_t)(size - done);
    return wah_jit_bulk_yield(ctx);
}

static wah_error_t wah_jit_memory_copy(wah_exec_context_t *ctx, uint32_t dst_idx, uint32_t src_idx, wah_value_t *ops) {
    if (wah_bulk_should_interrupt(ctx)) return wah_jit_bulk_yield(ctx);
    uint64_t dst = (uint32_t)ops[0].i32, src = (uint32_t)ops[1].i32, size = (uint32_t)ops[2].i32;
    const wah_memory_inst_t *dmem = &ctx->memories[dst_idx], *smem = &ctx->memories[src_idx];
    WAH_ENSURE(wah_u64_range_in_bounds(dst, size, dmem->size), WAH_ERROR_MEMORY_OUT_OF_BOUNDS);
    WAH_ENSURE(wah_u64_range_in_bounds(src, size, smem->size), WAH_ERROR_MEMORY_OUT_OF_BOUNDS);
    bool backward = dmem->data == smem->data && dst > src && dst < src + size;
    uint64_t done = wah_memory_copy_internal(ctx, dmem->data, dst, smem->data, src, size, backward);
    if (done == size) return WAH_OK;
    if (!backward) {
        ops[0].i32 = (int32_t)(dst + done);
        ops[1].i32 = (int32_t)(src + done);
    }
    ops[2].i32 = (int32_t)(size - done);
    return wah_jit_bulk_yield(ctx);
}

static wah_error_t wah_jit_memory_init(wah_exec_context_t *ctx, uint32_t data_idx, uint32_t mem_idx, wah_value_t *ops) {
    if (wah_bulk_should_interrupt(ctx)) return wah_jit_bulk_yield(ctx);
    uint64_t dst = (uint32_t)ops[0].i32;
    uint32_t src = (uint32_t)ops[1].i32, size = (uint32_t)ops[2].i32;
    uint32_t seg_len = wah_data_seg_data_len(ctx, data_idx);
    WAH_ENSURE(wah_u64_range_in_bounds(dst, size, ctx->memories[mem_idx].size), WAH_ERROR_MEMORY_OUT_OF_BOUNDS);
    WAH_ENSURE((uint64_t)src + size <= seg_len, WAH_ERROR_TRAP);
    uint32_t done = (uint32_t)wah_memory_init_internal(ctx, ctx->memories[mem_idx].data, dst,
                                                       ctx->module->data_segments[data_idx].data, src, size);
    if (done == size) return WAH_OK;
    ops[0].i32 = (int32_t)(dst + done);
    ops[1].i32 = (int32_t)(src + done);
    ops[2].i32 = (int32_t)(size - done);
    return wah_jit_bulk_yield(ctx);
}

static void wah_jit_data_drop(wah_exec_context_t *ctx, uint32_t data_idx) { wah_data_seg_mark_dropped(ctx, data_idx); }

#define WAH_JIT_CTX_OFF(field) ((uint32_t)offsetof(wah_exec_context_t, field))

#if defined(WAH_JIT_X86_64)

////////////////////////////////////////////////////////////////////////////////
// Code generation (x86-64) ////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Generated code uses the following registers:
//
// - rbx: exec context, rbp: frame base, r12/r13: base and size of memory 0 (reloaded after anything that may grow
//   memories), r14: limit of the value stack, r15: globals
// - rax, rcx, rdx, rsi, rdi, r8, r9 and xmm0-xmm13: operand stack cache, r10/r11 and xmm14/xmm15: scratch
//
// rsp stays at the trampoline frame, where helpers can be called right away. Wasm calls push return addresses
// as usual but callees immediately pop them into their link slots. MXCSR is set to the default during the execution,
// which doesn't canonicalize NaNs unlike AArch64, so every arithmetic operation that may return NaN checks it.

#define WAH_X64_CANONICAL_NAN(type) ((type) ? 0x7ff8000000000000ull : 0x7fc00000ull)

static void wah_x64_ucomis(wah_jit_fn_t *f, int type, int a, int b) { wah_x64_rr(f, WAH_X64_UCOMIS(type), 0, a, b); }

static void wah_x64_canonicalize(wah_jit_fn_t *f, int type, int x) {
    wah_x64_ucomis(f, type, x, x);
    uint32_t skip = wah_x64_jcc8(f, WAH_X64_NP);
    wah_jit_mov_imm(f, WAH_JR_T0, WAH_X64_CANONICAL_NAN(type));
    wah_jit_gpr_to_fpr(f, x, WAH_JR_T0);
    wah_x64_bind8(f, skip);
}

static void wah_x64_fconst(wah_jit_fn_t *f, int type, int x, double v) {
    union { double d; uint64_t u; } ud = { .d = v };
    union { float s; uint32_t u; } us = { .s = (float)v };
    wah_jit_mov_imm(f, WAH_JR_T0, type ? ud.u : us.u);
    wah_jit_gpr_to_fpr(f, x, WAH_JR_T0);
}

// Frees register r for the instruction being compiled, moving its value elsewhere.
static void wah_x64_take_gpr(wah_jit_fn_t *f, int r) {
    int32_t i = f->gpr[r];
    if (i >= 0) {
        int n = -1;
        for (int k = 31; k >= 0 && n < 0; k--) if (f->gpr[k] == WAH_JO_FREE) n = k;
        if (n >= 0) {
            wah_x64_mov(f, true, n, r);
            f->vals[i].reg = (uint8_t)n;
            f->gpr[n] = i;
        } else {
            wah_jit_spill(f, (uint32_t)i);
        }
    }
    f->gpr[r] = WAH_JO_TEMP;
}

static bool wah_x64_top_imm32(const wah_jit_fn_t *f, bool sf, int32_t *imm) {
    const wah_jit_val_t *v = &f->vals[f->h - 1];
    if (v->kind != WAH_JV_CONST || (sf && v->imm != (int32_t)v->imm)) return false;
    *imm = (int32_t)v->imm;
    return true;
}

// Returns with results on top of the stack. The compile-time state is not changed.
static void wah_jit_return(wah_jit_fn_t *f) {
    uint32_t n = f->ft->result_count;
    wah_x64_mem(f, WAH_X64_GRP5, 0, 6, WAH_JR_FP, -1, 0, (int32_t)f->link_off); // PUSH, before results may overwrite it
    for (uint32_t j = 0; j < n; j++) wah_jit_copy_val(f, f->h - n + j, j * (uint32_t)sizeof(wah_value_t));
    wah_x64_byte(f, 0xC3); // RET
}

static void wah_jit_br(wah_jit_fn_t *f, uint32_t depth, uint32_t keep, uint32_t drop, const wah_jit_cond_t *c);

static void wah_jit_br_table(wah_jit_fn_t *f, const wah_br_table_imm_t *bt) {
    int idx = wah_jit_pop_gpr(f);
    wah_jit_flush(f);
    uint32_t n = bt->target_count, table = wah_jit_new_label(f);
    uint32_t record_labels[WAH_MAX_CONTROL_DEPTH + 1];
    for (uint32_t r = 0; r < bt->record_count; r++) record_labels[r] = wah_jit_new_label(f);
    const uint32_t *symbols = wah_br_table_symbols((wah_br_table_imm_t *)bt);
    const uint32_t *drops = wah_br_table_drops((wah_br_table_imm_t *)bt);

    wah_x64_mov(f, false, idx, idx); // Zero-extended for the address
    wah_x64_rr(f, WAH_X64_GRP1_IMM32, 0, 7, idx);
    wah_x64_imm32(f, n);
    wah_jit_bcond(f, WAH_X64_AE, record_labels[wah_br_table_target(bt, n)]);
    wah_jit_adr(f, WAH_JR_T0, table);
    wah_x64_mem(f, WAH_X64_MOVSXD, WAH_X64_W, WAH_JR_T1, WAH_JR_T0, idx, 2, 0);
    wah_x64_rr(f, WAH_X64_ADD, WAH_X64_W, WAH_JR_T0, WAH_JR_T1);
    wah_x64_rr(f, WAH_X64_GRP5, 0, 4, WAH_JR_T0); // JMP r11
    wah_jit_free_gpr(f, idx);

    wah_jit_bind(f, table);
    uint32_t table_pos = wah_jit_pos(f);
    for (uint32_t i = 0; i < n; i++) {
        wah_jit_patch_here(f, WAH_JP_TABLE, record_labels[wah_br_table_target(bt, i)], table_pos);
        wah_x64_imm32(f, 0);
    }
    for (uint32_t r = 0; r < bt->record_count; r++) {
        wah_jit_bind(f, record_labels[r]);
        wah_jit_br(f, symbols[r], bt->keep, drops[r], NULL);
    }
}

// Calls a wasm function in this module with the frame base moved by delta.
static void wah_jit_call_wasm(wah_jit_fn_t *f, uint32_t delta, uint32_t func) {
    wah_x64_mem(f, WAH_X64_LEA, WAH_X64_W, WAH_JR_FP, WAH_JR_FP, -1, 0, (int32_t)delta);
    wah_x64_byte(f, 0xE8); // CALL rel32
    WAH_JIT_GROW(f, jm->calls, jm->call_count, jm->call_cap);
    if (f->fail) return;
    f->jm->calls[f->jm->call_count++] = (wah_jit_call_fixup_t){ .pos = wah_jit_pos(f), .func = func };
    wah_x64_imm32(f, 0);
    wah_x64_mem(f, WAH_X64_LEA, WAH_X64_W, WAH_JR_FP, WAH_JR_FP, -1, 0, -(int32_t)delta);
}

// Calls the code returned by wah_jit_call_indirect unless it is 1.
static void wah_jit_call_code(wah_jit_fn_t *f, uint32_t delta, uint32_t done) {
    wah_x64_alu_imm(f, 7, WAH_X64_W, WAH_JR_RET, 1);
    wah_jit_bcond(f, WAH_X64_E, done);
    wah_x64_mem(f, WAH_X64_LEA, WAH_X64_W, WAH_JR_FP, WAH_JR_FP, -1, 0, (int32_t)delta);
    wah_x64_rr(f, WAH_X64_GRP5, 0, 2, WAH_JR_RET); // CALL rax
    wah_x64_mem(f, WAH_X64_LEA, WAH_X64_W, WAH_JR_FP, WAH_JR_FP, -1, 0, -(int32_t)delta);
}

// Calls a C function, also switching MXCSR for host code if `host`. The result is in WAH_JR_RET.
static void wah_jit_call_helper(wah_jit_fn_t *f, uintptr_t fn, const wah_jit_arg_t *args, uint32_t nargs, bool host) {
#ifdef _WIN32
    static const int regs[] = { WAH_X64_RCX, WAH_X64_RDX, WAH_X64_R8, WAH_X64_R9 };
#else
    static const int regs[] = { WAH_X64_RDI, WAH_X64_RSI, WAH_X64_RDX, WAH_X64_RCX, WAH_X64_R8, WAH_X64_R9 };
#endif
    const uint32_t nregs = (uint32_t)(sizeof(regs) / sizeof(*regs));
    for (uint32_t i = 0; i < nargs; i++) {
        int r = i < nregs ? regs[i] : WAH_JR_T0;
        uint32_t off = args[i].kind == WAH_JA_SLOT || args[i].kind == WAH_JA_SLOT_U32 ? wah_jit_slot_off(f, args[i].v) : 0;
        switch (args[i].kind) {
            case WAH_JA_CTX: wah_x64_mov(f, true, r, WAH_JR_CTX); break;
            case WAH_JA_IMM: wah_jit_mov_imm(f, r, args[i].v); break;
            case WAH_JA_SLOT: wah_x64_mem(f, WAH_X64_LEA, WAH_X64_W, r, WAH_JR_FP, -1, 0, (int32_t)off); break;
            case WAH_JA_SLOT_U32: wah_jit_ldst(f, false, false, false, r, WAH_JR_FP, off); break;
        }
        if (i >= nregs) wah_jit_ldst(f, true, false, true, r, WAH_X64_RSP, 32 + 8 * (i - nregs));
    }
    if (host) wah_x64_mem(f, WAH_X64_MXCSR, 0, 2, WAH_X64_RSP, -1, 0, WAH_X64_MXCSR_OFF);
    wah_x64_call_abs(f, fn);
    if (host) wah_x64_mem(f, WAH_X64_MXCSR, 0, 2, WAH_X64_RSP, -1, 0, WAH_X64_MXCSR_OFF + 4);
}

static void wah_jit_memory_size(wah_jit_fn_t *f, uint32_t mem_idx) {
    int d = wah_jit_alloc(f, false);
    if (mem_idx != 0) {
        wah_jit_ldst(f, false, false, true, d, WAH_JR_CTX, WAH_JIT_CTX_OFF(memories));
        wah_jit_ldst(f, false, false, true, d, d, (uint64_t)mem_idx * sizeof(wah_memory_inst_t) +
                                                  offsetof(wah_memory_inst_t, size));
    } else {
        wah_x64_mov(f, true, d, WAH_JR_MEMSZ);
    }
    wah_x64_shift_imm(f, 5, WAH_X64_W, d, 16);
    wah_jit_push_gpr(f, d);
}

static void wah_jit_fconst(wah_jit_fn_t *f, int d, uint64_t bits) {
    if (bits == 0) {
        wah_x64_rr(f, WAH_X64_XORPS, 0, d, d);
    } else {
        wah_jit_mov_imm(f, WAH_JR_T0, bits);
        wah_jit_gpr_to_fpr(f, d, WAH_JR_T0);
    }
}

// Integer comparison producing flags.
static void wah_jit_icmp(wah_jit_fn_t *f, bool sf, int cond, bool fuse) {
    uint32_t w = sf ? WAH_X64_W : 0;
    int32_t imm;
    int a;
    if (wah_x64_top_imm32(f, sf, &imm)) {
        f->h--;
        a = wah_jit_pop_gpr(f);
        wah_x64_alu_imm(f, 7, w, a, imm);
    } else {
        int b = wah_jit_pop_gpr(f);
        a = wah_jit_pop_gpr(f);
        wah_x64_rr(f, WAH_X64_CMP, w, a, b);
        wah_jit_free_gpr(f, b);
    }
    if (fuse) {
        wah_jit_free_gpr(f, a);
        wah_jit_push(f, (wah_jit_val_t){ .kind = WAH_JV_FLAGS, .reg = (uint8_t)cond });
    } else {
        wah_jit_setcc(f, a, cond);
        wah_jit_push_gpr(f, a);
    }
}

// Unordered operands set ZF, PF and CF, so only ordered conditions can be flags.
static void wah_jit_fcmp(wah_jit_fn_t *f, int type, int kind, bool fuse) {
    int b = wah_jit_pop_fpr(f), a = wah_jit_pop_fpr(f);
    bool swap = kind == WAH_JFC_LT || kind == WAH_JFC_LE;
    wah_x64_ucomis(f, type, swap ? b : a, swap ? a : b);
    wah_jit_free_fpr(f, a);
    wah_jit_free_fpr(f, b);
    if (kind == WAH_JFC_EQ || kind == WAH_JFC_NE) {
        bool eq = kind == WAH_JFC_EQ;
        int d = wah_jit_alloc(f, false);
        wah_jit_setcc(f, d, eq ? WAH_X64_E : WAH_X64_NE);
        wah_jit_setcc(f, WAH_JR_T0, eq ? WAH_X64_NP : WAH_X64_P);
        wah_x64_rr(f, eq ? WAH_X64_AND : WAH_X64_OR, 0, d, WAH_JR_T0);
        wah_jit_push_gpr(f, d);
        return;
    }
    int cond = kind == WAH_JFC_LT || kind == WAH_JFC_GT ? WAH_X64_A : WAH_X64_AE;
    if (fuse) {
        wah_jit_push(f, (wah_jit_val_t){ .kind = WAH_JV_FLAGS, .reg = (uint8_t)cond });
    } else {
        int d = wah_jit_alloc(f, false);
        wah_jit_setcc(f, d, cond);
        wah_jit_push_gpr(f, d);
    }
}

static void wah_jit_ibinop(wah_jit_fn_t *f, bool sf, int kind) {
    static const uint32_t ops[] = { WAH_X64_ADD, WAH_X64_SUB, WAH_X64_IMUL, WAH_X64_AND, WAH_X64_OR, WAH_X64_XOR };
    static const int digits[] = { 0, 5, -1, 4, 1, 6 };
    uint32_t w = sf ? WAH_X64_W : 0;
    int32_t imm;
    if (wah_x64_top_imm32(f, sf, &imm)) {
        f->h--;
        int a = wah_jit_pop_gpr(f);
        if (kind == WAH_JI_MUL) {
            wah_x64_rr(f, 0x69, w, a, a); // IMUL r, r/m, imm32
            wah_x64_imm32(f, (uint32_t)imm);
        } else {
            wah_x64_alu_imm(f, digits[kind], w, a, imm);
        }
        wah_jit_push_gpr(f, a);
        return;
    }
    int b = wah_jit_pop_gpr(f), a = wah_jit_pop_gpr(f);
    wah_x64_rr(f, ops[kind], w, a, b);
    wah_jit_free_gpr(f, b);
    wah_jit_push_gpr(f, a);
}

// Without LZCNT or TZCNT, BSR and BSF leave flags for a zero operand.
static void wah_jit_iunop(wah_jit_fn_t *f, bool sf, int kind) {
    uint32_t w = sf ? WAH_X64_W : 0, bits = sf ? 64 : 32;
    int a = wah_jit_pop_gpr(f);
    switch (kind) {
        case WAH_JI_CLZ:
            if (f->jm->lzcnt) {
                wah_x64_rr(f, WAH_X64_LZCNT, w, a, a);
            } else { // bits - 1 - bsr(a), where bsr(0) is taken as -1
                wah_jit_mov_imm(f, WAH_JR_T0, UINT64_MAX);
                wah_x64_rr(f, WAH_X64_BSR, w, a, a);
                wah_x64_rr(f, WAH_X64_CMOV(WAH_X64_E), w, a, WAH_JR_T0);
                wah_x64_rr(f, WAH_X64_GRP3, w, 3, a);
                wah_x64_alu_imm(f, 0, w, a, (int32_t)bits - 1);
            }
            break;
        case WAH_JI_CTZ:
            if (f->jm->tzcnt) {
                wah_x64_rr(f, WAH_X64_TZCNT, w, a, a);
            } else {
                wah_jit_mov_imm(f, WAH_JR_T0, bits);
                wah_x64_rr(f, WAH_X64_BSF, w, a, a);
                wah_x64_rr(f, WAH_X64_CMOV(WAH_X64_E), w, a, WAH_JR_T0);
            }
            break;
        case WAH_JI_POPCNT: wah_x64_rr(f, WAH_X64_POPCNT, w, a, a); break;
    }
    wah_jit_push_gpr(f, a);
}

static void wah_jit_shift(wah_jit_fn_t *f, bool sf, int kind) {
    static const int digits[] = { 4, 7, 5, 0, 1 };
    uint32_t w = sf ? WAH_X64_W : 0, bits = sf ? 64 : 32;
    const wah_jit_val_t *top = &f->vals[f->h - 1];
    if (top->kind == WAH_JV_CONST) {
        uint32_t s = (uint32_t)top->imm & (bits - 1);
        f->h--;
        int a = wah_jit_pop_gpr(f);
        if (s != 0) wah_x64_shift_imm(f, digits[kind], w, a, s);
        wah_jit_push_gpr(f, a);
        return;
    }
    wah_x64_take_gpr(f, WAH_X64_RCX);
    int b = wah_jit_pop_gpr(f), a = wah_jit_pop_gpr(f);
    wah_x64_mov(f, false, WAH_X64_RCX, b);
    wah_x64_rr(f, WAH_X64_GRP2_CL, w, digits[kind], a);
    wah_jit_free_gpr(f, b);
    wah_jit_free_gpr(f, WAH_X64_RCX);
    wah_jit_push_gpr(f, a);
}

// Division takes rdx:rax, and also traps on INT_MIN % -1 which should be 0.
static void wah_jit_divrem(wah_jit_fn_t *f, bool sf, int kind) {
    const wah_jit_val_t *top = &f->vals[f->h - 1];
    bool is_signed = kind == WAH_JD_DIV_S || kind == WAH_JD_REM_S;
    int64_t c = top->kind == WAH_JV_CONST ? (sf ? top->imm : (int32_t)top->imm) : 0;
    bool nonzero = top->kind == WAH_JV_CONST && c != 0;
    bool not_neg1 = top->kind == WAH_JV_CONST && c != -1;
    uint32_t w = sf ? WAH_X64_W : 0;
    wah_x64_take_gpr(f, WAH_X64_RAX);
    wah_x64_take_gpr(f, WAH_X64_RDX);
    int b = wah_jit_pop_gpr(f), a = wah_jit_pop_gpr(f);
    if (!nonzero) wah_jit_cbz(f, false, sf, b, f->l_trap);
    uint32_t skip = 0, done = 0;
    if (is_signed && !not_neg1) {
        wah_x64_alu_imm(f, 7, w, b, -1);
        skip = wah_x64_jcc8(f, WAH_X64_NE);
        if (kind == WAH_JD_DIV_S) { // INT_MIN / -1 overflows
            wah_x64_alu_imm(f, 7, w, a, 1);
            wah_jit_bcond(f, WAH_X64_O, f->l_trap);
        } else {
            wah_jit_mov_imm(f, a, 0);
            done = wah_x64_jcc8(f, WAH_X64_JMP);
        }
        wah_x64_bind8(f, skip);
    }
    wah_x64_mov(f, sf, WAH_X64_RAX, a);
    if (is_signed) {
        if (sf) wah_x64_byte(f, 0x48);
        wah_x64_byte(f, 0x99); // CDQ/CQO
    } else {
        wah_x64_rr(f, WAH_X64_XOR, 0, WAH_X64_RDX, WAH_X64_RDX);
    }
    wah_x64_rr(f, WAH_X64_GRP3, w, is_signed ? 7 : 6, b);
    wah_x64_mov(f, sf, a, kind == WAH_JD_DIV_S || kind == WAH_JD_DIV_U ? WAH_X64_RAX : WAH_X64_RDX);
    if (done) wah_x64_bind8(f, done);
    wah_jit_free_gpr(f, b);
    wah_jit_free_gpr(f, WAH_X64_RAX);
    wah_jit_free_gpr(f, WAH_X64_RDX);
    wah_jit_push_gpr(f, a);
}

static void wah_jit_fbinop(wah_jit_fn_t *f, int type, int kind) {
    static const uint32_t ops[] = { WAH_X64_ADDS, WAH_X64_SUBS, WAH_X64_MULS, WAH_X64_DIVS };
    int b = wah_jit_pop_fpr(f), a = wah_jit_pop_fpr(f);
    if (kind == WAH_JF_MIN || kind == WAH_JF_MAX) { // MINSS/MAXSS don't follow wasm on NaN and zeros of both signs
        bool min = kind == WAH_JF_MIN;
        wah_x64_ucomis(f, type, a, b);
        uint32_t nan = wah_x64_jcc8(f, WAH_X64_P), ne = wah_x64_jcc8(f, WAH_X64_NE);
        wah_x64_rr(f, min ? WAH_X64_ORPS : WAH_X64_ANDPS, 0, a, b);
        uint32_t done1 = wah_x64_jcc8(f, WAH_X64_JMP);
        wah_x64_bind8(f, ne);
        wah_x64_rr(f, WAH_X64_FP(type) | (min ? WAH_X64_MINS : WAH_X64_MAXS), 0, a, b);
        uint32_t done2 = wah_x64_jcc8(f, WAH_X64_JMP);
        wah_x64_bind8(f, nan);
        wah_jit_mov_imm(f, WAH_JR_T0, WAH_X64_CANONICAL_NAN(type));
        wah_jit_gpr_to_fpr(f, a, WAH_JR_T0);
        wah_x64_bind8(f, done1);
        wah_x64_bind8(f, done2);
    } else {
        wah_x64_rr(f, WAH_X64_FP(type) | ops[kind], 0, a, b);
        wah_x64_canonicalize(f, type, a);
    }
    wah_jit_free_fpr(f, b);
    wah_jit_push_fpr(f, a);
}

static void wah_jit_funop(wah_jit_fn_t *f, int type, int kind) {
    int a = wah_jit_pop_fpr(f);
    switch (kind) {
        case WAH_JF_ABS: case WAH_JF_NEG: {
            uint64_t sign = type ? 0x8000000000000000ull : 0x80000000ull;
            wah_jit_mov_imm(f, WAH_JR_T0, kind == WAH_JF_ABS ? ~sign & (sign | (sign - 1)) : sign);
            wah_jit_gpr_to_fpr(f, WAH_JR_VT0, WAH_JR_T0);
            wah_x64_rr(f, kind == WAH_JF_ABS ? WAH_X64_ANDPS : WAH_X64_XORPS, 0, a, WAH_JR_VT0);
            break;
        }
        case WAH_JF_SQRT:
            wah_x64_rr(f, WAH_X64_FP(type) | WAH_X64_SQRTS, 0, a, a);
            wah_x64_canonicalize(f, type, a);
            break;
        default: { // ROUNDSS/ROUNDSD with the precision exception suppressed
            uint32_t mode = kind == WAH_JF_NEAREST ? 0 : kind == WAH_JF_FLOOR ? 1 : kind == WAH_JF_CEIL ? 2 : 3;
            wah_x64_rr(f, WAH_X64_ROUNDS(type), 0, a, a);
            wah_x64_byte(f, 8 | mode);
            wah_x64_canonicalize(f, type, a);
            break;
        }
    }
    wah_jit_push_fpr(f, a);
}

static void wah_jit_copysign(wah_jit_fn_t *f, int type) {
    int b = wah_jit_pop_fpr(f), a = wah_jit_pop_fpr(f);
    wah_jit_mov_imm(f, WAH_JR_T0, type ? 0x8000000000000000ull : 0x80000000ull);
    wah_jit_gpr_to_fpr(f, WAH_JR_VT0, WAH_JR_T0);
    wah_x64_rr(f, WAH_X64_ANDPS, 0, b, WAH_JR_VT0);
    wah_x64_rr(f, WAH_X64_ANDNPS, 0, WAH_JR_VT0, a);
    wah_x64_rr(f, WAH_X64_ORPS, 0, WAH_JR_VT0, b);
    wah_x64_rr(f, WAH_X64_MOVAPS_LOAD, 0, a, WAH_JR_VT0);
    wah_jit_free_fpr(f, b);
    wah_jit_push_fpr(f, a);
}

// f32.demote_f64 and f64.promote_f32
static void wah_jit_fcvt(wah_jit_fn_t *f, bool to_double) {
    int a = wah_jit_pop_fpr(f);
    wah_x64_rr(f, WAH_X64_FP(!to_double) | WAH_X64_CVTS, 0, a, a);
    wah_x64_canonicalize(f, to_double, a);
    wah_jit_push_fpr(f, a);
}

static void wah_jit_extend(wah_jit_fn_t *f, bool sf, int bits, bool is_signed) {
    int a = wah_jit_pop_gpr(f);
    if (!is_signed) wah_x64_mov(f, false, a, a);
    else if (bits == 32) wah_x64_rr(f, WAH_X64_MOVSXD, WAH_X64_W, a, a);
    else wah_x64_rr(f, bits == 8 ? WAH_X64_MOVSX8 : WAH_X64_MOVSX16, (sf ? WAH_X64_W : 0) | WAH_X64_B8, a, a);
    wah_jit_push_gpr(f, a);
}

static void wah_x64_cvtt(wah_jit_fn_t *f, int type, bool wide, int d, int a) {
    wah_x64_rr(f, WAH_X64_FP(type) | WAH_X64_CVTTS2SI, wide ? WAH_X64_W : 0, d, a);
}

// Trapping float to integer conversion: lo/hi are exclusive or inclusive bounds as described by lo_ge.
static void wah_jit_trunc(wah_jit_fn_t *f, int type, bool sf, bool is_signed, double lo, bool lo_ge, double hi) {
    int a = wah_jit_pop_fpr(f);
    for (int k = 0; k < 2; k++) {
        wah_x64_fconst(f, type, WAH_JR_VT0, k == 0 ? lo : hi);
        wah_x64_ucomis(f, type, a, WAH_JR_VT0);
        // Unordered sets CF, failing the first check which jumps when !(a >= lo) or !(a > lo)
        wah_jit_bcond(f, k == 0 ? (lo_ge ? WAH_X64_B : WAH_X64_BE) : WAH_X64_AE, f->l_trap);
    }
    int d = wah_jit_alloc(f, false);
    if (sf && !is_signed) { // Values from 2^63 are converted after subtracting it
        wah_x64_fconst(f, type, WAH_JR_VT0, 9223372036854775808.0);
        wah_x64_ucomis(f, type, a, WAH_JR_VT0);
        uint32_t big = wah_x64_jcc8(f, WAH_X64_AE);
        wah_x64_cvtt(f, type, true, d, a);
        uint32_t done = wah_x64_jcc8(f, WAH_X64_JMP);
        wah_x64_bind8(f, big);
        wah_x64_rr(f, WAH_X64_FP(type) | WAH_X64_SUBS, 0, a, WAH_JR_VT0);
        wah_x64_cvtt(f, type, true, d, a);
        wah_x64_rr(f, WAH_X64_GRP8, WAH_X64_W, 7, d); // BTC d, 63
        wah_x64_byte(f, 63);
        wah_x64_bind8(f, done);
    } else { // Unsigned 32-bit values are in the range of signed 64-bit ones
        wah_x64_cvtt(f, type, sf || !is_signed, d, a);
    }
    wah_jit_free_fpr(f, a);
    wah_jit_push_gpr(f, d);
}

// CVTTSS2SI returns INT_MIN for NaN and out-of-range values, which are fixed up.
static void wah_jit_trunc_sat(wah_jit_fn_t *f, int type, bool sf, bool is_signed) {
    int a = wah_jit_pop_fpr(f), d = wah_jit_alloc(f, false);
    if (is_signed) {
        uint32_t w = sf ? WAH_X64_W : 0;
        wah_x64_cvtt(f, type, sf, d, a);
        wah_x64_alu_imm(f, 7, w, d, 1); // Overflows only for INT_MIN
        uint32_t done1 = wah_x64_jcc8(f, WAH_X64_NO);
        wah_x64_ucomis(f, type, a, a);
        uint32_t nan = wah_x64_jcc8(f, WAH_X64_P);
        wah_x64_rr(f, WAH_X64_XORPS, 0, WAH_JR_VT0, WAH_JR_VT0);
        wah_x64_ucomis(f, type, a, WAH_JR_VT0);
        uint32_t done2 = wah_x64_jcc8(f, WAH_X64_B);
        wah_x64_rr(f, WAH_X64_GRP3, w, 2, d); // NOT gives INT_MAX
        uint32_t done3 = wah_x64_jcc8(f, WAH_X64_JMP);
        wah_x64_bind8(f, nan);
        wah_jit_mov_imm(f, d, 0);
        wah_x64_bind8(f, done1);
        wah_x64_bind8(f, done2);
        wah_x64_bind8(f, done3);
    } else if (!sf) { // Converted to i64, which is negative for NaN, negative values and those from 2^63
        wah_x64_cvtt(f, type, true, d, a);
        wah_x64_rr(f, WAH_X64_TEST, WAH_X64_W, d, d);
        uint32_t special = wah_x64_jcc8(f, WAH_X64_S);
        wah_jit_mov_imm(f, WAH_JR_T0, UINT32_MAX);
        wah_x64_rr(f, WAH_X64_CMP, WAH_X64_W, d, WAH_JR_T0);
        wah_x64_rr(f, WAH_X64_CMOV(WAH_X64_A), WAH_X64_W, d, WAH_JR_T0);
        uint32_t done1 = wah_x64_jcc8(f, WAH_X64_JMP);
        wah_x64_bind8(f, special);
        wah_x64_rr(f, WAH_X64_XORPS, 0, WAH_JR_VT0, WAH_JR_VT0);
        wah_x64_ucomis(f, type, a, WAH_JR_VT0);
        wah_jit_mov_imm(f, d, 0);
        uint32_t done2 = wah_x64_jcc8(f, WAH_X64_BE);
        wah_jit_mov_imm(f, d, UINT32_MAX);
        wah_x64_bind8(f, done1);
        wah_x64_bind8(f, done2);
    } else {
        wah_x64_fconst(f, type, WAH_JR_VT0, 9223372036854775808.0);
        wah_x64_ucomis(f, type, a, WAH_JR_VT0);
        uint32_t big = wah_x64_jcc8(f, WAH_X64_AE);
        wah_x64_cvtt(f, type, true, d, a);
        wah_x64_mov(f, true, WAH_JR_T0, d); // Negative results, including NaN, become 0
        wah_x64_shift_imm(f, 7, WAH_X64_W, WAH_JR_T0, 63);
        wah_x64_rr(f, WAH_X64_GRP3, WAH_X64_W, 2, WAH_JR_T0);
        wah_x64_rr(f, WAH_X64_AND, WAH_X64_W, d, WAH_JR_T0);
        uint32_t done = wah_x64_jcc8(f, WAH_X64_JMP);
        wah_x64_bind8(f, big);
        wah_x64_rr(f, WAH_X64_FP(type) | WAH_X64_SUBS, 0, a, WAH_JR_VT0);
        wah_x64_cvtt(f, type, true, d, a);
        wah_x64_mov(f, true, WAH_JR_T0, d); // Overflows become UINT64_MAX
        wah_x64_shift_imm(f, 7, WAH_X64_W, WAH_JR_T0, 63);
        wah_x64_rr(f, WAH_X64_GRP8, WAH_X64_W, 5, d); // BTS d, 63
        wah_x64_byte(f, 63);
        wah_x64_rr(f, WAH_X64_OR, WAH_X64_W, d, WAH_JR_T0);
        wah_x64_bind8(f, done);
    }
    wah_jit_free_fpr(f, a);
    wah_jit_push_gpr(f, d);
}

static void wah_jit_convert(wah_jit_fn_t *f, int type, bool sf, bool is_signed) {
    int a = wah_jit_pop_gpr(f), d = wah_jit_alloc(f, true);
    uint32_t op = WAH_X64_FP(type) | WAH_X64_CVTSI2S;
    wah_x64_rr(f, WAH_X64_XORPS, 0, d, d); // Breaks the dependency on d
    if (is_signed) {
        wah_x64_rr(f, op, sf ? WAH_X64_W : 0, d, a);
    } else if (!sf) {
        wah_x64_mov(f, false, WAH_JR_T0, a);
        wah_x64_rr(f, op, WAH_X64_W, d, WAH_JR_T0);
    } else { // Values from 2^63 are halved with the lowest bit kept for rounding, then doubled
        wah_x64_rr(f, WAH_X64_TEST, WAH_X64_W, a, a);
        uint32_t big = wah_x64_jcc8(f, WAH_X64_S);
        wah_x64_rr(f, op, WAH_X64_W, d, a);
        uint32_t done = wah_x64_jcc8(f, WAH_X64_JMP);
        wah_x64_bind8(f, big);
        wah_x64_mov(f, true, WAH_JR_T0, a);
        wah_x64_shift_imm(f, 5, WAH_X64_W, WAH_JR_T0, 1);
        wah_x64_alu_imm(f, 4, 0, a, 1);
        wah_x64_rr(f, WAH_X64_OR, WAH_X64_W, WAH_JR_T0, a);
        wah_x64_rr(f, op, WAH_X64_W, d, WAH_JR_T0);
        wah_x64_rr(f, WAH_X64_FP(type) | WAH_X64_ADDS, 0, d, d);
        wah_x64_bind8(f, done);
    }
    wah_jit_free_gpr(f, a);
    wah_jit_push_fpr(f, d);
}

// Bounds-checks the address on the stack, and returns the base with the address in T0 to be added with *disp.
static int wah_jit_address(wah_jit_fn_t *f, const wah_memarg_t *m, uint32_t size, int *size_reg, int32_t *disp) {
    int a = wah_jit_pop_gpr(f);
    int base = WAH_JR_MEM, limit = WAH_JR_MEMSZ;
    *size_reg = -1;
    if (m->memidx != 0) {
        base = wah_jit_alloc(f, false);
        limit = wah_jit_alloc(f, false);
        *size_reg = limit;
        uint64_t off = (uint64_t)m->memidx * sizeof(wah_memory_inst_t);
        wah_jit_ldst(f, false, false, true, base, WAH_JR_CTX, WAH_JIT_CTX_OFF(memories));
        wah_jit_ldst(f, false, false, true, limit, base, off + offsetof(wah_memory_inst_t, size));
        wah_jit_ldst(f, false, false, true, base, base, off + offsetof(wah_memory_inst_t, data));
    }
    wah_x64_mov(f, false, WAH_JR_T0, a); // Zero-extended
    wah_jit_free_gpr(f, a);
    uint64_t end = (uint64_t)m->offset + size; // Fits in 33 bits, so no overflow
    if (end <= INT32_MAX) {
        *disp = (int32_t)m->offset;
    } else {
        wah_jit_mov_imm(f, WAH_JR_T1, m->offset);
        wah_x64_rr(f, WAH_X64_ADD, WAH_X64_W, WAH_JR_T0, WAH_JR_T1);
        *disp = 0;
        end = size;
    }
    wah_x64_mem(f, WAH_X64_LEA, WAH_X64_W, WAH_JR_T1, WAH_JR_T0, -1, 0, (int32_t)end);
    wah_x64_rr(f, WAH_X64_CMP, WAH_X64_W, WAH_JR_T1, limit);
    wah_jit_bcond(f, WAH_X64_A, f->l_oob);
    return base;
}

static void wah_jit_load(wah_jit_fn_t *f, const wah_memarg_t *m, uint32_t size_log2, bool is_signed, bool sf, bool fp) {
    int size_reg;
    int32_t disp;
    int base = wah_jit_address(f, m, 1u << size_log2, &size_reg, &disp);
    if (size_reg >= 0) wah_jit_free_gpr(f, size_reg);
    if (base != WAH_JR_MEM) wah_jit_free_gpr(f, base);
    int d = wah_jit_alloc(f, fp);
    uint32_t op, flags = 0;
    if (fp) {
        op = WAH_X64_FP(size_log2 == 3) | WAH_X64_MOVS_LOAD;
    } else {
        switch (size_log2) {
            case 0: op = is_signed ? WAH_X64_MOVSX8 : WAH_X64_MOVZX8; break;
            case 1: op = is_signed ? WAH_X64_MOVSX16 : WAH_X64_MOVZX16; break;
            case 2: op = is_signed ? WAH_X64_MOVSXD : WAH_X64_MOV_LOAD; break;
            default: op = WAH_X64_MOV_LOAD; break;
        }
        if ((is_signed && sf) || size_log2 == 3) flags = WAH_X64_W;
    }
    wah_x64_mem(f, op, flags, d, base, WAH_JR_T0, 0, disp);
    if (fp) wah_jit_push_fpr(f, d);
    else wah_jit_push_gpr(f, d);
}

static void wah_jit_store(wah_jit_fn_t *f, const wah_memarg_t *m, uint32_t size_log2, bool fp_ok) {
    wah_jit_val_t *v = &f->vals[f->h - 1];
    int r = -1;
    bool fp = false;
    int64_t imm = v->imm;
    if (v->kind == WAH_JV_FPR && fp_ok) {
        r = wah_jit_pop_fpr(f);
        fp = true;
    } else if (v->kind == WAH_JV_CONST && (size_log2 < 3 || imm == (int32_t)imm)) {
        f->h--;
    } else {
        r = wah_jit_pop_gpr(f);
    }
    int size_reg;
    int32_t disp;
    int base = wah_jit_address(f, m, 1u << size_log2, &size_reg, &disp);
    if (fp) {
        wah_x64_mem(f, WAH_X64_FP(size_log2 == 3) | WAH_X64_MOVS_STORE, 0, r, base, WAH_JR_T0, 0, disp);
    } else if (r < 0) {
        static const uint32_t ops[] = { 0xC6u, 0x660000C7u, WAH_X64_MOV_IMM, WAH_X64_MOV_IMM };
        wah_x64_mem(f, ops[size_log2], size_log2 == 3 ? WAH_X64_W : 0, 0, base, WAH_JR_T0, 0, disp);
        uint32_t u = (uint32_t)imm;
        wah_jit_emit(f, &u, size_log2 == 0 ? 1 : size_log2 == 1 ? 2 : 4);
    } else {
        static const uint32_t ops[] = { WAH_X64_MOV_STORE8, 0x66000089u, WAH_X64_MOV_STORE, WAH_X64_MOV_STORE };
        wah_x64_mem(f, ops[size_log2], size_log2 == 3 ? WAH_X64_W : size_log2 == 0 ? WAH_X64_B8 : 0, r, base, WAH_JR_T0, 0, disp);
    }
    if (size_reg >= 0) wah_jit_free_gpr(f, size_reg);
    if (base != WAH_JR_MEM) wah_jit_free_gpr(f, base);
    if (fp) wah_jit_free_fpr(f, r);
    else if (r >= 0) wah_jit_free_gpr(f, r);
}

static void wah_jit_select(wah_jit_fn_t *f) {
    wah_jit_cond_t c = wah_jit_pop_cond(f);
    bool fp = f->vals[f->h - 1].kind == WAH_JV_FPR || f->vals[f->h - 2].kind == WAH_JV_FPR;
    int b, a;
    if (fp) { b = wah_jit_pop_fpr(f); a = wah_jit_pop_fpr(f); }
    else { b = wah_jit_pop_gpr(f); a = wah_jit_pop_gpr(f); }
    int cond = c.cond;
    if (!c.flags) {
        wah_x64_rr(f, WAH_X64_TEST, 0, c.reg, c.reg);
        cond = WAH_X64_NE;
    }
    if (fp) {
        uint32_t skip = wah_x64_jcc8(f, cond);
        wah_x64_rr(f, WAH_X64_MOVAPS_LOAD, 0, a, b);
        wah_x64_bind8(f, skip);
        wah_jit_free_fpr(f, b);
        wah_jit_push_fpr(f, a);
    } else {
        wah_x64_rr(f, WAH_X64_CMOV(cond ^ 1), WAH_X64_W, a, b);
        wah_jit_free_gpr(f, b);
        wah_jit_push_gpr(f, a);
    }
    wah_jit_release_cond(f, c);
}

static void wah_jit_emit_stub(wah_jit_fn_t *f, const wah_jit_stub_t *s) {
    wah_jit_bind(f, s->stub);
    if (s->poll) {
        wah_jit_call_helper(f, (uintptr_t)wah_jit_poll, (const wah_jit_arg_t[]){ { WAH_JA_CTX, 0 } }, 1, false);
        wah_jit_cbz(f, false, false, WAH_JR_RET, s->cont);
    }
    // eax is a status to suspend with, or an error
    wah_x64_rr(f, WAH_X64_TEST, 0, WAH_JR_RET, WAH_JR_RET);
    wah_jit_bcond(f, WAH_X64_S, f->l_err);
    wah_jit_ldst(f, false, false, true, WAH_JR_T0, WAH_JR_CTX, WAH_JIT_CTX_OFF(jit));
    wah_jit_adr(f, WAH_JR_T1, s->resume);
    wah_jit_ldst(f, true, false, true, WAH_JR_T1, WAH_JR_T0, offsetof(wah_jit_ctx_t, resume_pc));
    wah_jit_ldst(f, true, false, true, WAH_JR_FP, WAH_JR_T0, offsetof(wah_jit_ctx_t, resume_fp));
    wah_jit_exit(f);
}

// Checks the stack and saves the return address. Declared locals are zeroed by the caller.
static void wah_jit_prologue(wah_jit_fn_t *f) {
    wah_x64_byte(f, 0x41); // POP r11
    wah_x64_byte(f, 0x58u + (WAH_JR_T0 & 7));
    wah_x64_mem(f, WAH_X64_LEA, WAH_X64_W, WAH_JR_T1, WAH_JR_FP, -1, 0, (int32_t)f->frame_bytes);
    wah_x64_rr(f, WAH_X64_CMP, WAH_X64_W, WAH_JR_T1, WAH_JR_LIMIT);
    wah_jit_bcond(f, WAH_X64_A, f->l_stack);
    wah_jit_ldst(f, true, false, true, WAH_JR_T0, WAH_JR_FP, f->link_off);
}

// Zeroes n slots from [fp + off].
static void wah_jit_zero_loop(wah_jit_fn_t *f, uint32_t off, uint32_t n) {
    uint32_t loop = wah_jit_new_label(f);
    wah_x64_mem(f, WAH_X64_LEA, WAH_X64_W, WAH_JR_T0, WAH_JR_FP, -1, 0, (int32_t)off);
    wah_jit_mov_imm(f, WAH_JR_T1, n);
    wah_jit_bind(f, loop);
    wah_jit_store_imm(f, WAH_JR_T0, 0, 0);
    wah_x64_alu_imm(f, 0, WAH_X64_W, WAH_JR_T0, (int32_t)sizeof(wah_value_t));
    wah_x64_alu_imm(f, 5, 0, WAH_JR_T1, 1);
    wah_jit_bcond(f, WAH_X64_NE, loop);
}

#elif defined(WAH_JIT_AARCH64)

////////////////////////////////////////////////////////////////////////////////
// Code generation (AArch64) ///////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Generated code uses the following registers:
//
// - x19: exec context, x20: frame base
// - x21/x22: base and size of memory 0, reloaded after anything that may grow memories
// - x23: limit of the value stack, x24: globals, x25/x26: FPCR of the host and the JIT
// - x0-x15, v0-v7 and v16-v29: operand stack cache, x16/x17 and v30/v31: scratch
//
// Wasm calls are BL, whose callees save x30 into their link slots. FPCR is set to the default NaN mode,
// which makes every NaN result canonical.

// Whether the top of the stack is a constant usable as a 12-bit immediate, possibly negated.
static bool wah_a64_top_imm12(const wah_jit_fn_t *f, bool sf, uint32_t *imm, bool *negated) {
    const wah_jit_val_t *v = &f->vals[f->h - 1];
    if (v->kind != WAH_JV_CONST) return false;
    uint64_t c = sf ? (uint64_t)v->imm : (uint32_t)v->imm;
    uint64_t nc = sf ? (uint64_t)0 - c : (uint32_t)(0u - (uint32_t)c);
    if (c < 4096) { *imm = (uint32_t)c; *negated = false; return true; }
    if (nc < 4096) { *imm = (uint32_t)nc; *negated = true; return true; }
    return false;
}

// Returns with results on top of the stack. The compile-time state is not changed.
static void wah_jit_return(wah_jit_fn_t *f) {
    uint32_t n = f->ft->result_count;
    wah_a64_ldst(f, WAH_A64_LDRX, 30, WAH_JR_FP, f->link_off); // Before results may overwrite it
    for (uint32_t j = 0; j < n; j++) wah_jit_copy_val(f, f->h - n + j, j * (uint32_t)sizeof(wah_value_t));
    wah_a64(f, 0xD65F03C0u); // RET
}

static void wah_jit_br(wah_jit_fn_t *f, uint32_t depth, uint32_t keep, uint32_t drop, const wah_jit_cond_t *c);

static void wah_jit_br_table(wah_jit_fn_t *f, const wah_br_table_imm_t *bt) {
    int idx = wah_jit_pop_gpr(f);
    wah_jit_flush(f);
    uint32_t n = bt->target_count, table = wah_jit_new_label(f);
    uint32_t record_labels[WAH_MAX_CONTROL_DEPTH + 1];
    for (uint32_t r = 0; r < bt->record_count; r++) record_labels[r] = wah_jit_new_label(f);
    const uint32_t *symbols = wah_br_table_symbols((wah_br_table_imm_t *)bt);
    const uint32_t *drops = wah_br_table_drops((wah_br_table_imm_t *)bt);

    if (n < 4096) {
        wah_a64_addsub_imm(f, WAH_A64_SUBSI, false, WAH_A64_ZR, idx, n, false);
    } else {
        wah_a64_mov_imm(f, false, WAH_JR_T0, n);
        wah_a64_rrr(f, WAH_A64_SUBS, false, WAH_A64_ZR, idx, WAH_JR_T0);
    }
    wah_jit_bcond(f, WAH_A64_HS, record_labels[wah_br_table_target(bt, n)]);
    wah_jit_adr(f, WAH_JR_T0, table);
    // LDRSW x17, [x16, w_idx, UXTW #2]
    wah_a64(f, (WAH_A64_LDRSW & ~0x01000000u) | 0x00205800u | (uint32_t)idx << 16 | (uint32_t)WAH_JR_T0 << 5 | WAH_JR_T1);
    wah_a64_rrr(f, WAH_A64_ADD, true, WAH_JR_T0, WAH_JR_T0, WAH_JR_T1);
    wah_a64(f, 0xD61F0000u | (uint32_t)WAH_JR_T0 << 5); // BR x16
    wah_jit_free_gpr(f, idx);

    wah_jit_bind(f, table);
    uint32_t table_pos = wah_jit_pos(f);
    for (uint32_t i = 0; i < n; i++) {
        wah_jit_patch_here(f, WAH_JP_TABLE, record_labels[wah_br_table_target(bt, i)], table_pos);
        wah_a64(f, 0);
    }
    for (uint32_t r = 0; r < bt->record_count; r++) {
        wah_jit_bind(f, record_labels[r]);
        wah_jit_br(f, symbols[r], bt->keep, drops[r], NULL);
    }
}

// Calls a wasm function in this module with the frame base moved by delta.
static void wah_jit_call_wasm(wah_jit_fn_t *f, uint32_t delta, uint32_t func) {
    wah_a64_add_imm(f, WAH_JR_FP, WAH_JR_FP, delta);
    WAH_JIT_GROW(f, jm->calls, jm->call_count, jm->call_cap);
    if (f->fail) return;
    f->jm->calls[f->jm->call_count++] = (wah_jit_call_fixup_t){ .pos = wah_jit_pos(f), .func = func };
    wah_a64(f, 0x94000000u); // BL
    wah_a64_sub_imm(f, WAH_JR_FP, WAH_JR_FP, delta);
}

// Calls the code returned by wah_jit_call_indirect unless it is 1.
static void wah_jit_call_code(wah_jit_fn_t *f, uint32_t delta, uint32_t done) {
    wah_a64_addsub_imm(f, WAH_A64_SUBSI, true, WAH_A64_ZR, WAH_JR_RET, 1, false);
    wah_jit_bcond(f, WAH_A64_EQ, done);
    wah_a64_add_imm(f, WAH_JR_FP, WAH_JR_FP, delta);
    wah_a64(f, 0xD63F0000u); // BLR x0
    wah_a64_sub_imm(f, WAH_JR_FP, WAH_JR_FP, delta);
}

// Calls a C function, also switching FPCR for host code if `host`. The result is in WAH_JR_RET.
static void wah_jit_call_helper(wah_jit_fn_t *f, uintptr_t fn, const wah_jit_arg_t *args, uint32_t nargs, bool host) {
    for (uint32_t i = 0; i < nargs; i++) {
        int r = (int)i;
        switch (args[i].kind) {
            case WAH_JA_CTX: wah_a64_mov(f, true, r, WAH_JR_CTX); break;
            case WAH_JA_IMM: wah_a64_mov_imm(f, false, r, args[i].v); break;
            case WAH_JA_SLOT: wah_a64_add_imm(f, r, WAH_JR_FP, wah_jit_slot_off(f, args[i].v)); break;
            case WAH_JA_SLOT_U32: wah_a64_ldst(f, WAH_A64_LDRW, r, WAH_JR_FP, wah_jit_slot_off(f, args[i].v)); break;
        }
    }
    if (host) wah_a64(f, 0xD51B4400u | WAH_JR_FPCR_HOST); // MSR FPCR
    wah_a64_call_abs(f, fn);
    if (host) wah_a64(f, 0xD51B4400u | WAH_JR_FPCR_JIT);
}

static void wah_jit_memory_size(wah_jit_fn_t *f, uint32_t mem_idx) {
    int d = wah_jit_alloc(f, false);
    int src = WAH_JR_MEMSZ;
    if (mem_idx != 0) {
        wah_a64_ldst(f, WAH_A64_LDRX, d, WAH_JR_CTX, WAH_JIT_CTX_OFF(memories));
        wah_a64_ldst(f, WAH_A64_LDRX, d, d, (uint64_t)mem_idx * sizeof(wah_memory_inst_t) +
                                            offsetof(wah_memory_inst_t, size));
        src = d;
    }
    wah_a64_bfm(f, false, true, d, src, 16, 63); // LSR #16
    wah_jit_push_gpr(f, d);
}

static void wah_jit_fconst(wah_jit_fn_t *f, int d, uint64_t bits) {
    if (bits == 0) {
        wah_a64_fmov_from_gp(f, false, d, WAH_A64_ZR);
    } else {
        wah_a64_mov_imm(f, true, WAH_JR_T0, bits);
        wah_a64_fmov_from_gp(f, true, d, WAH_JR_T0);
    }
}

// Integer comparison producing flags.
static void wah_jit_icmp(wah_jit_fn_t *f, bool sf, int cond, bool fuse) {
    uint32_t imm;
    bool neg;
    int a;
    if (wah_a64_top_imm12(f, sf, &imm, &neg)) {
        f->h--;
        a = wah_jit_pop_gpr(f);
        wah_a64_addsub_imm(f, neg ? WAH_A64_ADDSI : WAH_A64_SUBSI, sf, WAH_A64_ZR, a, imm, false);
    } else {
        int b = wah_jit_pop_gpr(f);
        a = wah_jit_pop_gpr(f);
        wah_a64_rrr(f, WAH_A64_SUBS, sf, WAH_A64_ZR, a, b);
        wah_jit_free_gpr(f, b);
    }
    if (fuse) {
        wah_jit_free_gpr(f, a);
        wah_jit_push(f, (wah_jit_val_t){ .kind = WAH_JV_FLAGS, .reg = (uint8_t)cond });
    } else {
        wah_a64_cset(f, a, cond);
        wah_jit_push_gpr(f, a);
    }
}

static void wah_jit_fcmp(wah_jit_fn_t *f, int type, int kind, bool fuse) {
    static const int conds[] = { WAH_A64_EQ, WAH_A64_NE, WAH_A64_MI, WAH_A64_GT, WAH_A64_LS, WAH_A64_GE };
    int b = wah_jit_pop_fpr(f), a = wah_jit_pop_fpr(f);
    wah_a64_fp2(f, WAH_A64_FCMP, type, 0, a, b);
    wah_jit_free_fpr(f, a);
    wah_jit_free_fpr(f, b);
    if (fuse) {
        wah_jit_push(f, (wah_jit_val_t){ .kind = WAH_JV_FLAGS, .reg = (uint8_t)conds[kind] });
    } else {
        int d = wah_jit_alloc(f, false);
        wah_a64_cset(f, d, conds[kind]);
        wah_jit_push_gpr(f, d);
    }
}

static void wah_jit_ibinop(wah_jit_fn_t *f, bool sf, int kind) {
    static const uint32_t ops[] = { WAH_A64_ADD, WAH_A64_SUB, WAH_A64_MUL, WAH_A64_AND, WAH_A64_ORR, WAH_A64_EOR };
    uint32_t imm;
    bool neg;
    if ((kind == WAH_JI_ADD || kind == WAH_JI_SUB) && wah_a64_top_imm12(f, sf, &imm, &neg)) {
        f->h--;
        int a = wah_jit_pop_gpr(f);
        wah_a64_addsub_imm(f, (kind == WAH_JI_SUB) != neg ? WAH_A64_SUBI : WAH_A64_ADDI, sf, a, a, imm, false);
        wah_jit_push_gpr(f, a);
        return;
    }
    int b = wah_jit_pop_gpr(f), a = wah_jit_pop_gpr(f);
    wah_a64_rrr(f, ops[kind], sf, a, a, b);
    wah_jit_free_gpr(f, b);
    wah_jit_push_gpr(f, a);
}

static void wah_jit_iunop(wah_jit_fn_t *f, bool sf, int kind) {
    int a = wah_jit_pop_gpr(f);
    switch (kind) {
        case WAH_JI_CTZ: wah_a64_rrr(f, WAH_A64_RBIT, sf, a, a, 0); // Fall through
        case WAH_JI_CLZ: wah_a64_rrr(f, WAH_A64_CLZ, sf, a, a, 0); break;
        case WAH_JI_POPCNT:
            wah_a64_fmov_from_gp(f, sf, WAH_JR_VT1, a);
            wah_a64(f, 0x0E205800u | (uint32_t)WAH_JR_VT1 << 5 | WAH_JR_VT1); // CNT v31.8b
            wah_a64(f, 0x0E31B800u | (uint32_t)WAH_JR_VT1 << 5 | WAH_JR_VT1); // ADDV b31, v31.8b
            wah_a64_fmov_to_gp(f, false, a, WAH_JR_VT1);
            break;
    }
    wah_jit_push_gpr(f, a);
}

static void wah_jit_shift(wah_jit_fn_t *f, bool sf, int kind) {
    uint32_t bits = sf ? 64 : 32;
    const wah_jit_val_t *top = &f->vals[f->h - 1];
    if (top->kind == WAH_JV_CONST) {
        uint32_t s = (uint32_t)top->imm & (bits - 1);
        f->h--;
        int a = wah_jit_pop_gpr(f);
        if (s != 0) {
            switch (kind) {
                case WAH_JS_SHL: wah_a64_bfm(f, false, sf, a, a, (bits - s) & (bits - 1), bits - 1 - s); break;
                case WAH_JS_SHR_S: wah_a64_bfm(f, true, sf, a, a, s, bits - 1); break;
                case WAH_JS_SHR_U: wah_a64_bfm(f, false, sf, a, a, s, bits - 1); break;
                case WAH_JS_ROTL: wah_a64_extr(f, sf, a, a, a, bits - s); break;
                case WAH_JS_ROTR: wah_a64_extr(f, sf, a, a, a, s); break;
            }
        }
        wah_jit_push_gpr(f, a);
        return;
    }
    int b = wah_jit_pop_gpr(f), a = wah_jit_pop_gpr(f);
    switch (kind) {
        case WAH_JS_SHL: wah_a64_rrr(f, WAH_A64_LSLV, sf, a, a, b); break;
        case WAH_JS_SHR_S: wah_a64_rrr(f, WAH_A64_ASRV, sf, a, a, b); break;
        case WAH_JS_SHR_U: wah_a64_rrr(f, WAH_A64_LSRV, sf, a, a, b); break;
        case WAH_JS_ROTL:
            wah_a64_rrr(f, WAH_A64_SUB, false, WAH_JR_T0, WAH_A64_ZR, b);
            wah_a64_rrr(f, WAH_A64_RORV, sf, a, a, WAH_JR_T0);
            break;
        case WAH_JS_ROTR: wah_a64_rrr(f, WAH_A64_RORV, sf, a, a, b); break;
    }
    wah_jit_free_gpr(f, b);
    wah_jit_push_gpr(f, a);
}

static void wah_jit_divrem(wah_jit_fn_t *f, bool sf, int kind) {
    const wah_jit_val_t *top = &f->vals[f->h - 1];
    bool safe = top->kind == WAH_JV_CONST && (sf ? top->imm : (int32_t)top->imm) != 0 &&
                ((sf ? top->imm : (int32_t)top->imm) != -1 || kind != WAH_JD_DIV_S);
    int b = wah_jit_pop_gpr(f), a = wah_jit_pop_gpr(f);
    bool is_signed = kind == WAH_JD_DIV_S || kind == WAH_JD_REM_S;
    if (!safe) {
        wah_jit_cbz(f, false, sf, b, f->l_trap);
        if (kind == WAH_JD_DIV_S) { // INT_MIN / -1 overflows
            uint32_t ok = wah_jit_new_label(f);
            wah_a64_addsub_imm(f, WAH_A64_ADDSI, sf, WAH_A64_ZR, b, 1, false);
            wah_jit_bcond(f, WAH_A64_NE, ok);
            wah_a64_addsub_imm(f, WAH_A64_SUBSI, sf, WAH_A64_ZR, a, 1, false);
            wah_jit_bcond(f, WAH_A64_VS, f->l_trap);
            wah_jit_bind(f, ok);
        }
    }
    uint32_t div = is_signed ? WAH_A64_SDIV : WAH_A64_UDIV;
    if (kind == WAH_JD_DIV_S || kind == WAH_JD_DIV_U) {
        wah_a64_rrr(f, div, sf, a, a, b);
    } else { // INT_MIN % -1 is 0 as sdiv gives INT_MIN
        wah_a64_rrr(f, div, sf, WAH_JR_T0, a, b);
        wah_a64_msub(f, sf, a, WAH_JR_T0, b, a);
    }
    wah_jit_free_gpr(f, b);
    wah_jit_push_gpr(f, a);
}

static void wah_jit_fbinop(wah_jit_fn_t *f, int type, int kind) {
    static const uint32_t ops[] = { WAH_A64_FADD, WAH_A64_FSUB, WAH_A64_FMUL, WAH_A64_FDIV, WAH_A64_FMIN, WAH_A64_FMAX };
    int b = wah_jit_pop_fpr(f), a = wah_jit_pop_fpr(f);
    wah_a64_fp2(f, ops[kind], type, a, a, b);
    wah_jit_free_fpr(f, b);
    wah_jit_push_fpr(f, a);
}

static void wah_jit_funop(wah_jit_fn_t *f, int type, int kind) {
    static const uint32_t ops[] = {
        WAH_A64_FABS, WAH_A64_FNEG, WAH_A64_FRINTP, WAH_A64_FRINTM, WAH_A64_FRINTZ, WAH_A64_FRINTN, WAH_A64_FSQRT,
    };
    int a = wah_jit_pop_fpr(f);
    wah_a64_fp2(f, ops[kind], type, a, a, 0);
    wah_jit_push_fpr(f, a);
}

static void wah_jit_copysign(wah_jit_fn_t *f, int type) {
    int b = wah_jit_pop_fpr(f), a = wah_jit_pop_fpr(f);
    if (type) wah_a64_movw(f, WAH_A64_MOVZ, true, WAH_JR_T0, 0x8000, 3);
    else wah_a64_movw(f, WAH_A64_MOVZ, false, WAH_JR_T0, 0x8000, 1);
    wah_a64_fmov_from_gp(f, true, WAH_JR_VT1, WAH_JR_T0);
    wah_a64(f, 0x2EA01C00u | (uint32_t)WAH_JR_VT1 << 16 | (uint32_t)b << 5 | (uint32_t)a); // BIT a.8b, b.8b, mask.8b
    wah_jit_free_fpr(f, b);
    wah_jit_push_fpr(f, a);
}

// f32.demote_f64 and f64.promote_f32
static void wah_jit_fcvt(wah_jit_fn_t *f, bool to_double) {
    int a = wah_jit_pop_fpr(f);
    wah_a64(f, (to_double ? 0x1E22C000u : 0x1E624000u) | (uint32_t)a << 5 | (uint32_t)a);
    wah_jit_push_fpr(f, a);
}

static void wah_jit_extend(wah_jit_fn_t *f, bool sf, int bits, bool is_signed) {
    int a = wah_jit_pop_gpr(f);
    if (is_signed) wah_a64_bfm(f, true, sf, a, a, 0, (uint32_t)bits - 1);
    else wah_a64_mov(f, false, a, a);
    wah_jit_push_gpr(f, a);
}

// Trapping float to integer conversion: lo/hi are exclusive or inclusive bounds as described by lo_ge.
static void wah_jit_trunc(wah_jit_fn_t *f, int type, bool sf, bool is_signed, double lo, bool lo_ge, double hi) {
    int a = wah_jit_pop_fpr(f);
    union { double d; uint64_t u; } ud;
    union { float s; uint32_t u; } us;
    for (int k = 0; k < 2; k++) {
        double bound = k == 0 ? lo : hi;
        if (type) { ud.d = bound; wah_a64_mov_imm(f, true, WAH_JR_T0, ud.u); }
        else { us.s = (float)bound; wah_a64_mov_imm(f, false, WAH_JR_T0, us.u); }
        wah_a64_fmov_from_gp(f, type, WAH_JR_VT1, WAH_JR_T0);
        wah_a64_fp2(f, WAH_A64_FCMP, type, 0, a, WAH_JR_VT1);
        // Unordered fails the first check, which jumps when !(a >= lo) or !(a > lo)
        wah_jit_bcond(f, k == 0 ? (lo_ge ? WAH_A64_LT : WAH_A64_LE) : WAH_A64_GE, f->l_trap);
    }
    int d = wah_jit_alloc(f, false);
    wah_a64_fcvt_gp(f, is_signed ? WAH_A64_FCVTZS : WAH_A64_FCVTZU, sf, type, d, a);
    wah_jit_free_fpr(f, a);
    wah_jit_push_gpr(f, d);
}

static void wah_jit_trunc_sat(wah_jit_fn_t *f, int type, bool sf, bool is_signed) {
    int a = wah_jit_pop_fpr(f), d = wah_jit_alloc(f, false);
    wah_a64_fcvt_gp(f, is_signed ? WAH_A64_FCVTZS : WAH_A64_FCVTZU, sf, type, d, a);
    wah_jit_free_fpr(f, a);
    wah_jit_push_gpr(f, d);
}

static void wah_jit_convert(wah_jit_fn_t *f, int type, bool sf, bool is_signed) {
    int a = wah_jit_pop_gpr(f), d = wah_jit_alloc(f, true);
    wah_a64_fcvt_gp(f, is_signed ? WAH_A64_SCVTF : WAH_A64_UCVTF, sf, type, d, a);
    wah_jit_free_gpr(f, a);
    wah_jit_push_fpr(f, d);
}

// Bounds-checks the address on the stack and leaves base + x16 as the effective address.
static int wah_jit_address(wah_jit_fn_t *f, const wah_memarg_t *m, uint32_t size, int *size_reg) {
    int a = wah_jit_pop_gpr(f);
    int base = WAH_JR_MEM, limit = WAH_JR_MEMSZ;
    *size_reg = -1;
    if (m->memidx != 0) {
        base = wah_jit_alloc(f, false);
        limit = wah_jit_alloc(f, false);
        *size_reg = limit;
        uint64_t off = (uint64_t)m->memidx * sizeof(wah_memory_inst_t);
        wah_a64_ldst(f, WAH_A64_LDRX, base, WAH_JR_CTX, WAH_JIT_CTX_OFF(memories));
        wah_a64_ldst(f, WAH_A64_LDRX, limit, base, off + offsetof(wah_memory_inst_t, size));
        wah_a64_ldst(f, WAH_A64_LDRX, base, base, off + offsetof(wah_memory_inst_t, data));
    }
    wah_a64_mov(f, false, WAH_JR_T0, a); // Zero-extended
    wah_jit_free_gpr(f, a);
    wah_a64_add_imm(f, WAH_JR_T0, WAH_JR_T0, m->offset); // Fits in 33 bits, so no overflow
    wah_a64_addsub_imm(f, WAH_A64_ADDI, true, WAH_JR_T1, WAH_JR_T0, size, false);
    wah_a64_rrr(f, WAH_A64_SUBS, true, WAH_A64_ZR, WAH_JR_T1, limit);
    wah_jit_bcond(f, WAH_A64_HI, f->l_oob);
    return base;
}

static void wah_jit_load(wah_jit_fn_t *f, const wah_memarg_t *m, uint32_t size_log2, bool is_signed, bool sf, bool fp) {
    uint32_t op;
    if (fp) {
        op = size_log2 == 3 ? WAH_A64_LDRD : WAH_A64_LDRS;
    } else {
        switch (size_log2) {
            case 0: op = is_signed ? (sf ? WAH_A64_LDRSBX : WAH_A64_LDRSBW) : WAH_A64_LDRB; break;
            case 1: op = is_signed ? (sf ? WAH_A64_LDRSHX : WAH_A64_LDRSHW) : WAH_A64_LDRH; break;
            case 2: op = is_signed ? WAH_A64_LDRSW : WAH_A64_LDRW; break;
            default: op = WAH_A64_LDRX; break;
        }
    }
    int size_reg;
    int base = wah_jit_address(f, m, 1u << size_log2, &size_reg);
    if (size_reg >= 0) wah_jit_free_gpr(f, size_reg);
    if (base != WAH_JR_MEM) wah_jit_free_gpr(f, base);
    int d = wah_jit_alloc(f, fp);
    wah_a64_ldst_reg(f, op, d, base, WAH_JR_T0);
    if (fp) wah_jit_push_fpr(f, d);
    else wah_jit_push_gpr(f, d);
}

static void wah_jit_store(wah_jit_fn_t *f, const wah_memarg_t *m, uint32_t size_log2, bool fp_ok) {
    static const uint32_t ops[] = { WAH_A64_STRB, WAH_A64_STRH, WAH_A64_STRW, WAH_A64_STRX };
    wah_jit_val_t *v = &f->vals[f->h - 1];
    int r, fp = 0;
    if (v->kind == WAH_JV_FPR && fp_ok) {
        r = wah_jit_pop_fpr(f);
        fp = 1;
    } else if (v->kind == WAH_JV_CONST && v->imm == 0) {
        f->h--;
        r = WAH_A64_ZR;
    } else {
        r = wah_jit_pop_gpr(f);
    }
    int size_reg;
    int base = wah_jit_address(f, m, 1u << size_log2, &size_reg);
    wah_a64_ldst_reg(f, fp ? (size_log2 == 3 ? WAH_A64_STRD : WAH_A64_STRS) : ops[size_log2], r, base, WAH_JR_T0);
    if (size_reg >= 0) wah_jit_free_gpr(f, size_reg);
    if (base != WAH_JR_MEM) wah_jit_free_gpr(f, base);
    if (fp) wah_jit_free_fpr(f, r);
    else if (r != WAH_A64_ZR) wah_jit_free_gpr(f, r);
}

static void wah_jit_select(wah_jit_fn_t *f) {
    wah_jit_cond_t c = wah_jit_pop_cond(f);
    bool fp = f->vals[f->h - 1].kind == WAH_JV_FPR || f->vals[f->h - 2].kind == WAH_JV_FPR;
    int b, a;
    if (fp) { b = wah_jit_pop_fpr(f); a = wah_jit_pop_fpr(f); }
    else { b = wah_jit_pop_gpr(f); a = wah_jit_pop_gpr(f); }
    int cond = c.cond;
    if (!c.flags) {
        wah_a64_addsub_imm(f, WAH_A64_SUBSI, false, WAH_A64_ZR, c.reg, 0, false);
        cond = WAH_A64_NE;
    }
    if (fp) {
        wah_a64_fcsel(f, 1, a, a, b, cond);
        wah_jit_free_fpr(f, b);
        wah_jit_push_fpr(f, a);
    } else {
        wah_a64_csel(f, true, a, a, b, cond);
        wah_jit_free_gpr(f, b);
        wah_jit_push_gpr(f, a);
    }
    wah_jit_release_cond(f, c);
}

static void wah_jit_emit_stub(wah_jit_fn_t *f, const wah_jit_stub_t *s) {
    wah_jit_bind(f, s->stub);
    if (s->poll) {
        wah_jit_call_helper(f, (uintptr_t)wah_jit_poll, (const wah_jit_arg_t[]){ { WAH_JA_CTX, 0 } }, 1, false);
        wah_jit_cbz(f, false, false, WAH_JR_RET, s->cont);
    }
    // w0 is a status to suspend with, or an error
    wah_a64_addsub_imm(f, WAH_A64_SUBSI, false, WAH_A64_ZR, WAH_JR_RET, 0, false);
    wah_jit_bcond(f, WAH_A64_LT, f->l_err);
    wah_a64_ldst(f, WAH_A64_LDRX, WAH_JR_T0, WAH_JR_CTX, WAH_JIT_CTX_OFF(jit));
    wah_jit_adr(f, WAH_JR_T1, s->resume);
    wah_a64_ldst(f, WAH_A64_STRX, WAH_JR_T1, WAH_JR_T0, offsetof(wah_jit_ctx_t, resume_pc));
    wah_a64_ldst(f, WAH_A64_STRX, WAH_JR_FP, WAH_JR_T0, offsetof(wah_jit_ctx_t, resume_fp));
    wah_jit_exit(f);
}

// Checks the stack and saves the return address. Declared locals are zeroed by the caller.
static void wah_jit_prologue(wah_jit_fn_t *f) {
    wah_a64_add_imm(f, WAH_JR_T1, WAH_JR_FP, f->frame_bytes);
    wah_a64_rrr(f, WAH_A64_SUBS, true, WAH_A64_ZR, WAH_JR_T1, WAH_JR_LIMIT);
    wah_jit_bcond(f, WAH_A64_HI, f->l_stack);
    wah_a64_ldst(f, WAH_A64_STRX, 30, WAH_JR_FP, f->link_off);
}

// Zeroes n slots from [fp + off].
static void wah_jit_zero_loop(wah_jit_fn_t *f, uint32_t off, uint32_t n) {
    uint32_t loop = wah_jit_new_label(f);
    wah_a64_add_imm(f, WAH_JR_T0, WAH_JR_FP, off);
    wah_a64_mov_imm(f, false, WAH_JR_T1, n);
    wah_jit_bind(f, loop);
    wah_a64(f, 0xF8000400u | (uint32_t)sizeof(wah_value_t) << 12 | (uint32_t)WAH_JR_T0 << 5 | WAH_A64_ZR); // STR xzr, [x16], #16
    wah_a64_addsub_imm(f, WAH_A64_SUBSI, false, WAH_JR_T1, WAH_JR_T1, 1, false);
    wah_jit_bcond(f, WAH_A64_NE, loop);
}

#endif // WAH_JIT_AARCH64

////////////////////////////////////////////////////////////////////////////////
// Compiler ////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static void wah_jit_exit_with(wah_jit_fn_t *f, wah_error_t err) {
    wah_jit_mov_imm(f, WAH_JR_RET, (uint32_t)err);
    wah_jit_exit(f);
}

static void wah_jit_reload_memory(wah_jit_fn_t *f) {
    if (wah_memory_index_limit(f->module) == 0) return;
    wah_jit_ldst(f, false, false, true, WAH_JR_MEM, WAH_JR_CTX, WAH_JIT_CTX_OFF(memory_base));
    wah_jit_ldst(f, false, false, true, WAH_JR_MEMSZ, WAH_JR_CTX, WAH_JIT_CTX_OFF(memory_size));
}

// Checks the poll flag, and calls the poll handler if set, which may suspend here.
static void wah_jit_poll_check(wah_jit_fn_t *f) {
    uint32_t check = wah_jit_new_label(f), stub = wah_jit_new_label(f), cont = wah_jit_new_label(f);
    wah_jit_bind(f, check);
    wah_jit_ldst(f, false, false, false, WAH_JR_T0, WAH_JR_CTX, WAH_JIT_CTX_OFF(poll_flag));
    wah_jit_cbz(f, true, false, WAH_JR_T0, stub);
    wah_jit_bind(f, cont);
    wah_jit_add_stub(f, stub, check, cont, true);
}

// After a helper call returning a status, which suspends at `resume` if positive.
static void wah_jit_check_status(wah_jit_fn_t *f, uint32_t resume) {
    uint32_t stub = wah_jit_new_label(f);
    wah_jit_cbz(f, true, false, WAH_JR_RET, stub);
    wah_jit_add_stub(f, stub, resume, 0, false);
}

// Moves `keep` values on top of the stack below `drop` values and stores the rest, as a branch to a label expects.
// The compile-time state is not changed, so this can be used in a conditional path.
static void wah_jit_branch_moves(wah_jit_fn_t *f, uint32_t keep, uint32_t drop) {
    uint32_t base = f->h - drop - keep;
    for (uint32_t i = 0; i < base; i++) {
        if (f->vals[i].kind != WAH_JV_SLOT) wah_jit_copy_val(f, i, wah_jit_slot_off(f, i));
    }
    for (uint32_t j = 0; j < keep; j++) wah_jit_copy_val(f, f->h - keep + j, wah_jit_slot_off(f, base + j));
}

static bool wah_jit_branch_needs_moves(const wah_jit_fn_t *f, uint32_t keep, uint32_t drop) {
    if (keep > 0 && drop > 0) return true;
    for (uint32_t i = 0; i < f->h; i++) {
        if (f->vals[i].kind != WAH_JV_SLOT) return true;
    }
    return false;
}

// A branch by relative depth, conditional if c is given.
static void wah_jit_br(wah_jit_fn_t *f, uint32_t depth, uint32_t keep, uint32_t drop, const wah_jit_cond_t *c) {
    if (depth == f->nctl) { // To the function level
        uint32_t skip = 0;
        if (c) wah_jit_branch_if(f, *c, false, skip = wah_jit_new_label(f));
        wah_jit_return(f);
        if (c) wah_jit_bind(f, skip);
        return;
    }
    wah_jit_ctl_t *t = &f->ctl[f->nctl - 1 - depth];
    if (t->opcode != WAH_OP_LOOP) {
        t->reachable = true;
        t->end_h = f->h - drop;
    }
    if (c && !wah_jit_branch_needs_moves(f, keep, drop)) {
        wah_jit_branch_if(f, *c, true, t->label);
        return;
    }
    uint32_t skip = 0;
    if (c) wah_jit_branch_if(f, *c, false, skip = wah_jit_new_label(f));
    wah_jit_branch_moves(f, keep, drop);
    wah_jit_b(f, t->label);
    if (c) wah_jit_bind(f, skip);
}

static bool wah_jit_numeric_type(wah_type_t t) {
    return t == WAH_TYPE_I32 || t == WAH_TYPE_I64 || t == WAH_TYPE_F32 || t == WAH_TYPE_F64;
}

static bool wah_jit_numeric_func_type(const wah_func_type_t *ft) {
    for (uint32_t i = 0; i < ft->param_count; i++) if (!wah_jit_numeric_type(ft->param_types[i])) return false;
    for (uint32_t i = 0; i < ft->result_count; i++) if (!wah_jit_numeric_type(ft->result_types[i])) return false;
    return true;
}

static bool wah_jit_is_fp_type(wah_type_t t) { return t == WAH_TYPE_F32 || t == WAH_TYPE_F64; }

static wah_type_t wah_jit_local_type(const wah_jit_fn_t *f, const wah_code_body_t *body, uint32_t idx) {
    return idx < f->ft->param_count ? f->ft->param_types[idx] : wah_local_type(body, idx - f->ft->param_count);
}

// Whether an instruction can be compiled. Unsupported ones make the whole module interpreted.
static bool wah_jit_supported(const wah_module_t *module, const wah_decoded_instr_t *in) {
    switch (in->opcode) {
        case WAH_OP_CALL: return wah_jit_numeric_func_type(wah_resolve_func_type(module, in->imm.u32));
        case WAH_OP_CALL_INDIRECT: return wah_jit_numeric_func_type(&module->types[in->imm.u32_u32.a]);
        case WAH_OP_GLOBAL_GET: case WAH_OP_GLOBAL_SET: {
            uint32_t idx = in->imm.u32;
            wah_type_t t = idx < module->import_global_count ? module->global_imports[idx].type
                                                             : module->globals[idx - module->import_global_count].type;
            return wah_jit_numeric_type(t);
        }
        case WAH_OP_SELECT_T: return true; // Operands are numeric if their sources are
        case WAH_OP_UNREACHABLE: case WAH_OP_BLOCK: case WAH_OP_LOOP: case WAH_OP_IF: case WAH_OP_ELSE:
        case WAH_OP_END: case WAH_OP_BR: case WAH_OP_BR_IF: case WAH_OP_BR_TABLE: case WAH_OP_RETURN:
        case WAH_OP_DROP: case WAH_OP_SELECT: case WAH_OP_LOCAL_GET: case WAH_OP_LOCAL_SET: case WAH_OP_LOCAL_TEE:
        case WAH_OP_MEMORY_SIZE: case WAH_OP_MEMORY_GROW: case WAH_OP_MEMORY_INIT: case WAH_OP_DATA_DROP:
        case WAH_OP_MEMORY_COPY: case WAH_OP_MEMORY_FILL:
        case WAH_OP_I32_CONST: case WAH_OP_I64_CONST: case WAH_OP_F32_CONST: case WAH_OP_F64_CONST:
            return true;
        default:
            if (in->opcode >= WAH_OP_I32_LOAD && in->opcode <= WAH_OP_I64_STORE32) return true;
            if (in->opcode >= WAH_OP_I32_EQZ && in->opcode <= WAH_OP_I64_EXTEND32_S) return true;
            if (in->opcode >= WAH_OP_I32_TRUNC_SAT_F32_S && in->opcode <= WAH_OP_I64_TRUNC_SAT_F64_U) return true;
            return false;
    }
}

static void wah_jit_end(wah_jit_fn_t *f) {
    wah_jit_ctl_t t = f->ctl[--f->nctl];
    if (t.opcode == WAH_OP_LOOP) return; // Only reached by falling through
    if (t.opcode == WAH_OP_IF && !t.has_else) { // The false edge passes params as results
        if (!f->dead) wah_jit_flush(f);
        wah_jit_bind(f, t.else_label);
        wah_jit_bind(f, t.label);
        wah_jit_reset(f, t.h_start);
        f->dead = false;
        return;
    }
    if (!f->dead) {
        if (!t.reachable) return; // Nothing else reaches here, so the state is kept as is
        wah_jit_flush(f);
        t.end_h = f->h;
    }
    wah_jit_bind(f, t.label);
    if (t.reachable || !f->dead) {
        wah_jit_reset(f, t.end_h);
        f->dead = false;
    }
}

static void wah_jit_instr(wah_jit_fn_t *f, const wah_code_body_t *body, const wah_decoded_instr_t *in,
                          const wah_decoded_instr_t *next) {
    uint16_t op = in->opcode;
    bool fuse = next && (next->opcode == WAH_OP_BR_IF || next->opcode == WAH_OP_IF ||
                         next->opcode == WAH_OP_SELECT || next->opcode == WAH_OP_SELECT_T);

    if (f->dead) {
        switch (op) {
            case WAH_OP_BLOCK: case WAH_OP_LOOP: case WAH_OP_IF: f->dead_depth++; return;
            case WAH_OP_ELSE: if (f->dead_depth > 0) return; break;
            case WAH_OP_END: if (f->dead_depth > 0) { f->dead_depth--; return; } break;
            default: return;
        }
    }

    switch (op) {
        case WAH_OP_UNREACHABLE: wah_jit_b(f, f->l_trap); f->dead = true; break;

        case WAH_OP_BLOCK: case WAH_OP_LOOP: {
            WAH_JIT_GROW(f, ctl, nctl, ctl_cap);
            if (f->fail) return;
            wah_jit_ctl_t *t = &f->ctl[f->nctl++];
            *t = (wah_jit_ctl_t){ .opcode = op, .label = wah_jit_new_label(f), .h_start = f->h };
            if (op == WAH_OP_LOOP) {
                wah_jit_flush(f);
                wah_jit_reset(f, f->h);
                wah_jit_bind(f, t->label);
                wah_jit_poll_check(f);
            }
            break;
        }
        case WAH_OP_IF: {
            wah_jit_cond_t c = wah_jit_pop_cond(f);
            wah_jit_flush(f);
            WAH_JIT_GROW(f, ctl, nctl, ctl_cap);
            if (f->fail) return;
            wah_jit_ctl_t *t = &f->ctl[f->nctl++];
            *t = (wah_jit_ctl_t){ .opcode = op, .label = wah_jit_new_label(f), .else_label = wah_jit_new_label(f),
                                  .h_start = f->h };
            wah_jit_branch_if(f, c, false, t->else_label);
            wah_jit_release_cond(f, c);
            break;
        }
        case WAH_OP_ELSE: {
            wah_jit_ctl_t *t = &f->ctl[f->nctl - 1];
            if (!f->dead) {
                wah_jit_flush(f);
                t->reachable = true;
                t->end_h = f->h;
                wah_jit_b(f, t->label);
            }
            t->has_else = true;
            wah_jit_bind(f, t->else_label);
            wah_jit_reset(f, t->h_start);
            f->dead = false;
            break;
        }
        case WAH_OP_END:
            if (f->nctl == 0) {
                if (!f->dead) wah_jit_return(f);
                f->dead = true;
            } else {
                wah_jit_end(f);
            }
            break;
        case WAH_OP_BR:
            wah_jit_br(f, in->imm.branch.label_idx, in->imm.branch.keep, in->imm.branch.drop, NULL);
            f->dead = true;
            break;
        case WAH_OP_BR_IF: {
            wah_jit_cond_t c = wah_jit_pop_cond(f);
            wah_jit_br(f, in->imm.branch.label_idx, in->imm.branch.keep, in->imm.branch.drop, &c);
            wah_jit_release_cond(f, c);
            break;
        }
        case WAH_OP_BR_TABLE: wah_jit_br_table(f, in->imm.br_table); f->dead = true; break;
        case WAH_OP_RETURN: wah_jit_return(f); f->dead = true; break;

        case WAH_OP_CALL: {
            uint32_t idx = in->imm.u32;
            const wah_func_type_t *ft = wah_resolve_func_type(f->module, idx);
            wah_jit_flush(f);
            uint32_t args = f->h - ft->param_count;
            if (idx < f->module->import_function_count) {
                const wah_jit_arg_t a[] = { { WAH_JA_CTX, 0 }, { WAH_JA_IMM, idx }, { WAH_JA_SLOT, args } };
                wah_jit_call_helper(f, (uintptr_t)wah_jit_call_host, a, 3, true);
                wah_jit_cbz(f, true, false, WAH_JR_RET, f->l_err);
            } else {
                wah_jit_call_wasm(f, wah_jit_slot_off(f, args), idx - f->module->import_function_count);
            }
            wah_jit_reload_memory(f);
            wah_jit_reset(f, args + ft->result_count);
            break;
        }
        case WAH_OP_CALL_INDIRECT: {
            const wah_func_type_t *ft = &f->module->types[in->imm.u32_u32.a];
            wah_jit_flush(f);
            f->h--;
            uint32_t args = f->h - ft->param_count, done = wah_jit_new_label(f);
            const wah_jit_arg_t a[] = {
                { WAH_JA_CTX, 0 }, { WAH_JA_IMM, in->imm.u32_u32.a }, { WAH_JA_IMM, in->imm.u32_u32.b },
                { WAH_JA_SLOT_U32, f->h }, { WAH_JA_SLOT, args },
            };
            wah_jit_call_helper(f, (uintptr_t)wah_jit_call_indirect, a, 5, true);
            wah_jit_cbz(f, false, true, WAH_JR_RET, f->l_err_jit);
            wah_jit_call_code(f, wah_jit_slot_off(f, args), done);
            wah_jit_bind(f, done);
            wah_jit_reload_memory(f);
            wah_jit_reset(f, args + ft->result_count);
            break;
        }

        case WAH_OP_DROP: wah_jit_drop(f); break;
        case WAH_OP_SELECT: case WAH_OP_SELECT_T: wah_jit_select(f); break;

        case WAH_OP_LOCAL_GET: {
            uint32_t idx = in->imm.u32;
            bool fp = wah_jit_is_fp_type(wah_jit_local_type(f, body, idx));
            int r = wah_jit_alloc(f, fp);
            wah_jit_ldst(f, false, fp, true, r, WAH_JR_FP, idx * (uint32_t)sizeof(wah_value_t));
            if (fp) wah_jit_push_fpr(f, r);
            else wah_jit_push_gpr(f, r);
            break;
        }
        case WAH_OP_LOCAL_SET: case WAH_OP_LOCAL_TEE: {
            uint32_t off = in->imm.u32 * (uint32_t)sizeof(wah_value_t);
            if (op == WAH_OP_LOCAL_TEE && f->vals[f->h - 1].kind == WAH_JV_SLOT) {
                if (wah_jit_is_fp_type(wah_jit_local_type(f, body, in->imm.u32))) wah_jit_to_fpr(f, f->h - 1);
                else wah_jit_to_gpr(f, f->h - 1);
            }
            wah_jit_copy_val(f, f->h - 1, off);
            if (op == WAH_OP_LOCAL_SET) wah_jit_drop(f);
            break;
        }
        case WAH_OP_GLOBAL_GET: case WAH_OP_GLOBAL_SET: {
            uint32_t idx = in->imm.u32, off = idx * (uint32_t)sizeof(wah_value_t);
            const wah_module_t *m = f->module;
            bool indirect = idx < m->import_global_count && m->global_imports[idx].is_mutable;
            wah_type_t t = idx < m->import_global_count ? m->global_imports[idx].type
                                                        : m->globals[idx - m->import_global_count].type;
            bool fp = wah_jit_is_fp_type(t), wide = t == WAH_TYPE_I64 || t == WAH_TYPE_F64;
            int base = WAH_JR_GLOBALS;
            if (indirect) {
                wah_jit_ldst(f, false, false, true, WAH_JR_T1, WAH_JR_GLOBALS, off);
                base = WAH_JR_T1;
                off = 0;
            }
            if (op == WAH_OP_GLOBAL_GET) {
                int r = wah_jit_alloc(f, fp);
                wah_jit_ldst(f, false, fp, wide, r, base, off);
                if (fp) wah_jit_push_fpr(f, r);
                else wah_jit_push_gpr(f, r);
            } else {
                int r = fp ? wah_jit_pop_fpr(f) : wah_jit_pop_gpr(f);
                wah_jit_ldst(f, true, fp, wide, r, base, off);
                if (fp) wah_jit_free_fpr(f, r);
                else wah_jit_free_gpr(f, r);
            }
            break;
        }

#define WAH_JIT_LOAD(name, size, sgn, sf, fp) case WAH_OP_##name: wah_jit_load(f, &in->imm.memarg, size, sgn, sf, fp); break;
#define WAH_JIT_STORE(name, size, fp) case WAH_OP_##name: wah_jit_store(f, &in->imm.memarg, size, fp); break;
        WAH_JIT_LOAD(I32_LOAD, 2, false, false, false)
        WAH_JIT_LOAD(I64_LOAD, 3, false, true, false)
        WAH_JIT_LOAD(F32_LOAD, 2, false, false, true)
        WAH_JIT_LOAD(F64_LOAD, 3, false, true, true)
        WAH_JIT_LOAD(I32_LOAD8_S, 0, true, false, false)
        WAH_JIT_LOAD(I32_LOAD8_U, 0, false, false, false)
        WAH_JIT_LOAD(I32_LOAD16_S, 1, true, false, false)
        WAH_JIT_LOAD(I32_LOAD16_U, 1, false, false, false)
        WAH_JIT_LOAD(I64_LOAD8_S, 0, true, true, false)
        WAH_JIT_LOAD(I64_LOAD8_U, 0, false, true, false)
        WAH_JIT_LOAD(I64_LOAD16_S, 1, true, true, false)
        WAH_JIT_LOAD(I64_LOAD16_U, 1, false, true, false)
        WAH_JIT_LOAD(I64_LOAD32_S, 2, true, true, false)
        WAH_JIT_LOAD(I64_LOAD32_U, 2, false, true, false)
        WAH_JIT_STORE(I32_STORE, 2, false)
        WAH_JIT_STORE(I64_STORE, 3, false)
        WAH_JIT_STORE(F32_STORE, 2, true)
        WAH_JIT_STORE(F64_STORE, 3, true)
        WAH_JIT_STORE(I32_STORE8, 0, false)
        WAH_JIT_STORE(I32_STORE16, 1, false)
        WAH_JIT_STORE(I64_STORE8, 0, false)
        WAH_JIT_STORE(I64_STORE16, 1, false)
        WAH_JIT_STORE(I64_STORE32, 2, false)
#undef WAH_JIT_LOAD
#undef WAH_JIT_STORE

        case WAH_OP_MEMORY_SIZE: wah_jit_memory_size(f, in->imm.u32); break;
        case WAH_OP_MEMORY_GROW: {
            wah_jit_flush(f);
            const wah_jit_arg_t a[] = { { WAH_JA_CTX, 0 }, { WAH_JA_IMM, in->imm.u32 }, { WAH_JA_SLOT_U32, f->h - 1 } };
            wah_jit_call_helper(f, (uintptr_t)wah_jit_memory_grow, a, 3, false);
            wah_jit_reload_memory(f);
            wah_jit_reset(f, f->h - 1);
            f->gpr[WAH_JR_RET] = WAH_JO_TEMP;
            wah_jit_push_gpr(f, WAH_JR_RET);
            break;
        }
        case WAH_OP_MEMORY_FILL: case WAH_OP_MEMORY_COPY: case WAH_OP_MEMORY_INIT: {
            wah_jit_flush(f);
            uint32_t start = wah_jit_new_label(f);
            wah_jit_bind(f, start); // Resumes here with operands in slots
            wah_jit_arg_t a[4] = { { WAH_JA_CTX, 0 } };
            uintptr_t fn;
            uint32_t nargs = 3;
            if (op == WAH_OP_MEMORY_FILL) {
                a[1] = (wah_jit_arg_t){ WAH_JA_IMM, in->imm.u32 };
                fn = (uintptr_t)wah_jit_memory_fill;
            } else {
                a[1] = (wah_jit_arg_t){ WAH_JA_IMM, in->imm.u32_u32.a };
                a[2] = (wah_jit_arg_t){ WAH_JA_IMM, in->imm.u32_u32.b };
                fn = op == WAH_OP_MEMORY_COPY ? (uintptr_t)wah_jit_memory_copy : (uintptr_t)wah_jit_memory_init;
                nargs = 4;
            }
            a[nargs - 1] = (wah_jit_arg_t){ WAH_JA_SLOT, f->h - 3 };
            wah_jit_call_helper(f, fn, a, nargs, false);
            wah_jit_check_status(f, start);
            wah_jit_reload_memory(f);
            wah_jit_reset(f, f->h - 3);
            break;
        }
        case WAH_OP_DATA_DROP: {
            wah_jit_flush(f);
            const wah_jit_arg_t a[] = { { WAH_JA_CTX, 0 }, { WAH_JA_IMM, in->imm.u32 } };
            wah_jit_call_helper(f, (uintptr_t)wah_jit_data_drop, a, 2, false);
            wah_jit_reset(f, f->h);
            break;
        }

        case WAH_OP_I32_CONST: wah_jit_push(f, (wah_jit_val_t){ .kind = WAH_JV_CONST, .imm = in->imm.i32 }); break;
        case WAH_OP_I64_CONST: wah_jit_push(f, (wah_jit_val_t){ .kind = WAH_JV_CONST, .imm = in->imm.i64 }); break;
        case WAH_OP_F32_CONST: case WAH_OP_F64_CONST: {
            int d = wah_jit_alloc(f, true);
            wah_jit_fconst(f, d, op == WAH_OP_F64_CONST ? in->imm.f64_bits : in->imm.f32_bits);
            wah_jit_push_fpr(f, d);
            break;
        }

        case WAH_OP_I32_EQZ: case WAH_OP_I64_EQZ:
            wah_jit_push(f, (wah_jit_val_t){ .kind = WAH_JV_CONST, .imm = 0 });
            wah_jit_icmp(f, op == WAH_OP_I64_EQZ, WAH_JC_EQ, fuse);
            break;
#define WAH_JIT_ICMP(N, sf) \
        case WAH_OP_I##N##_EQ: wah_jit_icmp(f, sf, WAH_JC_EQ, fuse); break; \
        case WAH_OP_I##N##_NE: wah_jit_icmp(f, sf, WAH_JC_NE, fuse); break; \
        case WAH_OP_I##N##_LT_S: wah_jit_icmp(f, sf, WAH_JC_LT_S, fuse); break; \
        case WAH_OP_I##N##_LT_U: wah_jit_icmp(f, sf, WAH_JC_LT_U, fuse); break; \
        case WAH_OP_I##N##_GT_S: wah_jit_icmp(f, sf, WAH_JC_GT_S, fuse); break; \
        case WAH_OP_I##N##_GT_U: wah_jit_icmp(f, sf, WAH_JC_GT_U, fuse); break; \
        case WAH_OP_I##N##_LE_S: wah_jit_icmp(f, sf, WAH_JC_LE_S, fuse); break; \
        case WAH_OP_I##N##_LE_U: wah_jit_icmp(f, sf, WAH_JC_LE_U, fuse); break; \
        case WAH_OP_I##N##_GE_S: wah_jit_icmp(f, sf, WAH_JC_GE_S, fuse); break; \
        case WAH_OP_I##N##_GE_U: wah_jit_icmp(f, sf, WAH_JC_GE_U, fuse); break;
        WAH_JIT_ICMP(32, false)
        WAH_JIT_ICMP(64, true)
#undef WAH_JIT_ICMP
#define WAH_JIT_FCMP(N, type) \
        case WAH_OP_F##N##_EQ: wah_jit_fcmp(f, type, WAH_JFC_EQ, fuse); break; \
        case WAH_OP_F##N##_NE: wah_jit_fcmp(f, type, WAH_JFC_NE, fuse); break; \
        case WAH_OP_F##N##_LT: wah_jit_fcmp(f, type, WAH_JFC_LT, fuse); break; \
        case WAH_OP_F##N##_GT: wah_jit_fcmp(f, type, WAH_JFC_GT, fuse); break; \
        case WAH_OP_F##N##_LE: wah_jit_fcmp(f, type, WAH_JFC_LE, fuse); break; \
        case WAH_OP_F##N##_GE: wah_jit_fcmp(f, type, WAH_JFC_GE, fuse); break;
        WAH_JIT_FCMP(32, 0)
        WAH_JIT_FCMP(64, 1)
#undef WAH_JIT_FCMP

#define WAH_JIT_INUM(N, sf) \
        case WAH_OP_I##N##_CLZ: wah_jit_iunop(f, sf, WAH_JI_CLZ); break; \
        case WAH_OP_I##N##_CTZ: wah_jit_iunop(f, sf, WAH_JI_CTZ); break; \
        case WAH_OP_I##N##_POPCNT: wah_jit_iunop(f, sf, WAH_JI_POPCNT); break; \
        case WAH_OP_I##N##_ADD: wah_jit_ibinop(f, sf, WAH_JI_ADD); break; \
        case WAH_OP_I##N##_SUB: wah_jit_ibinop(f, sf, WAH_JI_SUB); break; \
        case WAH_OP_I##N##_MUL: wah_jit_ibinop(f, sf, WAH_JI_MUL); break; \
        case WAH_OP_I##N##_DIV_S: wah_jit_divrem(f, sf, WAH_JD_DIV_S); break; \
        case WAH_OP_I##N##_DIV_U: wah_jit_divrem(f, sf, WAH_JD_DIV_U); break; \
        case WAH_OP_I##N##_REM_S: wah_jit_divrem(f, sf, WAH_JD_REM_S); break; \
        case WAH_OP_I##N##_REM_U: wah_jit_divrem(f, sf, WAH_JD_REM_U); break; \
        case WAH_OP_I##N##_AND: wah_jit_ibinop(f, sf, WAH_JI_AND); break; \
        case WAH_OP_I##N##_OR: wah_jit_ibinop(f, sf, WAH_JI_OR); break; \
        case WAH_OP_I##N##_XOR: wah_jit_ibinop(f, sf, WAH_JI_XOR); break; \
        case WAH_OP_I##N##_SHL: wah_jit_shift(f, sf, WAH_JS_SHL); break; \
        case WAH_OP_I##N##_SHR_S: wah_jit_shift(f, sf, WAH_JS_SHR_S); break; \
        case WAH_OP_I##N##_SHR_U: wah_jit_shift(f, sf, WAH_JS_SHR_U); break; \
        case WAH_OP_I##N##_ROTL: wah_jit_shift(f, sf, WAH_JS_ROTL); break; \
        case WAH_OP_I##N##_ROTR: wah_jit_shift(f, sf, WAH_JS_ROTR); break;
        WAH_JIT_INUM(32, false)
        WAH_JIT_INUM(64, true)
#undef WAH_JIT_INUM
#define WAH_JIT_FNUM(N, type) \
        case WAH_OP_F##N##_ABS: wah_jit_funop(f, type, WAH_JF_ABS); break; \
        case WAH_OP_F##N##_NEG: wah_jit_funop(f, type, WAH_JF_NEG); break; \
        case WAH_OP_F##N##_CEIL: wah_jit_funop(f, type, WAH_JF_CEIL); break; \
        case WAH_OP_F##N##_FLOOR: wah_jit_funop(f, type, WAH_JF_FLOOR); break; \
        case WAH_OP_F##N##_TRUNC: wah_jit_funop(f, type, WAH_JF_TRUNC); break; \
        case WAH_OP_F##N##_NEAREST: wah_jit_funop(f, type, WAH_JF_NEAREST); break; \
        case WAH_OP_F##N##_SQRT: wah_jit_funop(f, type, WAH_JF_SQRT); break; \
        case WAH_OP_F##N##_ADD: wah_jit_fbinop(f, type, WAH_JF_ADD); break; \
        case WAH_OP_F##N##_SUB: wah_jit_fbinop(f, type, WAH_JF_SUB); break; \
        case WAH_OP_F##N##_MUL: wah_jit_fbinop(f, type, WAH_JF_MUL); break; \
        case WAH_OP_F##N##_DIV: wah_jit_fbinop(f, type, WAH_JF_DIV); break; \
        case WAH_OP_F##N##_MIN: wah_jit_fbinop(f, type, WAH_JF_MIN); break; \
        case WAH_OP_F##N##_MAX: wah_jit_fbinop(f, type, WAH_JF_MAX); break; \
        case WAH_OP_F##N##_COPYSIGN: wah_jit_copysign(f, type); break;
        WAH_JIT_FNUM(32, 0)
        WAH_JIT_FNUM(64, 1)
#undef WAH_JIT_FNUM

        case WAH_OP_I32_WRAP_I64: break; // i32 operations ignore the upper half
        case WAH_OP_I64_EXTEND_I32_S: case WAH_OP_I64_EXTEND32_S: wah_jit_extend(f, true, 32, true); break;
        case WAH_OP_I64_EXTEND_I32_U: wah_jit_extend(f, true, 32, false); break;
        case WAH_OP_I32_EXTEND8_S: wah_jit_extend(f, false, 8, true); break;
        case WAH_OP_I32_EXTEND16_S: wah_jit_extend(f, false, 16, true); break;
        case WAH_OP_I64_EXTEND8_S: wah_jit_extend(f, true, 8, true); break;
        case WAH_OP_I64_EXTEND16_S: wah_jit_extend(f, true, 16, true); break;

        case WAH_OP_I32_TRUNC_F32_S: wah_jit_trunc(f, 0, false, true, -2147483648.0, true, 2147483648.0); break;
        case WAH_OP_I32_TRUNC_F32_U: wah_jit_trunc(f, 0, false, false, -1.0, false, 4294967296.0); break;
        case WAH_OP_I32_TRUNC_F64_S: wah_jit_trunc(f, 1, false, true, -2147483649.0, false, 2147483648.0); break;
        case WAH_OP_I32_TRUNC_F64_U: wah_jit_trunc(f, 1, false, false, -1.0, false, 4294967296.0); break;
        case WAH_OP_I64_TRUNC_F32_S: wah_jit_trunc(f, 0, true, true, -9223372036854775808.0, true, 9223372036854775808.0); break;
        case WAH_OP_I64_TRUNC_F32_U: wah_jit_trunc(f, 0, true, false, -1.0, false, 18446744073709551616.0); break;
        case WAH_OP_I64_TRUNC_F64_S: wah_jit_trunc(f, 1, true, true, -9223372036854775808.0, true, 9223372036854775808.0); break;
        case WAH_OP_I64_TRUNC_F64_U: wah_jit_trunc(f, 1, true, false, -1.0, false, 18446744073709551616.0); break;
        case WAH_OP_I32_TRUNC_SAT_F32_S: wah_jit_trunc_sat(f, 0, false, true); break;
        case WAH_OP_I32_TRUNC_SAT_F32_U: wah_jit_trunc_sat(f, 0, false, false); break;
        case WAH_OP_I32_TRUNC_SAT_F64_S: wah_jit_trunc_sat(f, 1, false, true); break;
        case WAH_OP_I32_TRUNC_SAT_F64_U: wah_jit_trunc_sat(f, 1, false, false); break;
        case WAH_OP_I64_TRUNC_SAT_F32_S: wah_jit_trunc_sat(f, 0, true, true); break;
        case WAH_OP_I64_TRUNC_SAT_F32_U: wah_jit_trunc_sat(f, 0, true, false); break;
        case WAH_OP_I64_TRUNC_SAT_F64_S: wah_jit_trunc_sat(f, 1, true, true); break;
        case WAH_OP_I64_TRUNC_SAT_F64_U: wah_jit_trunc_sat(f, 1, true, false); break;
        case WAH_OP_F32_CONVERT_I32_S: wah_jit_convert(f, 0, false, true); break;
        case WAH_OP_F32_CONVERT_I32_U: wah_jit_convert(f, 0, false, false); break;
        case WAH_OP_F32_CONVERT_I64_S: wah_jit_convert(f, 0, true, true); break;
        case WAH_OP_F32_CONVERT_I64_U: wah_jit_convert(f, 0, true, false); break;
        case WAH_OP_F64_CONVERT_I32_S: wah_jit_convert(f, 1, false, true); break;
        case WAH_OP_F64_CONVERT_I32_U: wah_jit_convert(f, 1, false, false); break;
        case WAH_OP_F64_CONVERT_I64_S: wah_jit_convert(f, 1, true, true); break;
        case WAH_OP_F64_CONVERT_I64_U: wah_jit_convert(f, 1, true, false); break;
        case WAH_OP_F32_DEMOTE_F64: wah_jit_fcvt(f, false); break;
        case WAH_OP_F64_PROMOTE_F32: wah_jit_fcvt(f, true); break;
        // Values are kept as raw bits, so reinterpretation only moves them between register files if needed
        case WAH_OP_I32_REINTERPRET_F32: case WAH_OP_I64_REINTERPRET_F64:
            if (f->vals[f->h - 1].kind == WAH_JV_FPR) wah_jit_to_gpr(f, f->h - 1);
            break;
        case WAH_OP_F32_REINTERPRET_I32: case WAH_OP_F64_REINTERPRET_I64:
            if (f->vals[f->h - 1].kind == WAH_JV_GPR || f->vals[f->h - 1].kind == WAH_JV_FLAGS) wah_jit_to_fpr(f, f->h - 1);
            break;

        default: WAH_JIT_FAIL(f, "unhandled opcode"); break;
    }
}

static void wah_jit_emit_stubs(wah_jit_fn_t *f) {
    for (uint32_t i = 0; i < f->nstubs && !f->fail; i++) wah_jit_emit_stub(f, &f->stubs[i]);
    wah_jit_bind(f, f->l_trap);
    wah_jit_exit_with(f, WAH_ERROR_TRAP);
    wah_jit_bind(f, f->l_oob);
    wah_jit_exit_with(f, WAH_ERROR_MEMORY_OUT_OF_BOUNDS);
    wah_jit_bind(f, f->l_stack);
    wah_jit_exit_with(f, WAH_ERROR_STACK_OVERFLOW);
    wah_jit_bind(f, f->l_err_jit);
    wah_jit_ldst(f, false, false, true, WAH_JR_T0, WAH_JR_CTX, WAH_JIT_CTX_OFF(jit));
    wah_jit_ldst(f, false, false, false, WAH_JR_RET, WAH_JR_T0, offsetof(wah_jit_ctx_t, err));
    wah_jit_bind(f, f->l_err);
    wah_jit_exit(f);
}

static void wah_jit_free_fn(wah_jit_fn_t *f);
static void wah_jit_compile_body(wah_jit_fn_t *f, uint32_t local_idx, const wah_code_body_t *body,
                                 const wah_analyzed_code_t *ac);

static void wah_jit_compile_function(wah_module_t *module, uint32_t local_idx, const wah_analyzed_code_t *ac) {
    wah_jit_module_t *jm = module->jit;
    if (jm->failed) return;
    wah_jit_fn_t fs = { .jm = jm, .alloc = &module->alloc, .module = module }, *f = &fs;
    const wah_code_body_t *body = &module->code_bodies[local_idx];
    f->ft = &module->types[module->function_type_indices[local_idx]];

    if (!jm->func_pos) {
        void *p = NULL;
        if (wah_malloc(f->alloc, module->wasm_function_count, sizeof(uint32_t), &p) != WAH_OK) goto fail;
        jm->func_pos = (uint32_t *)p;
        for (uint32_t i = 0; i < module->type_count; i++) {
            if (module->types[i].result_count > jm->extra_slots) jm->extra_slots = module->types[i].result_count;
        }
    }
    if (!wah_jit_numeric_func_type(f->ft)) { WAH_LOG("JIT: function %u has a non-numeric type", local_idx); goto fail; }
    for (uint32_t r = 0; r < body->local_run_count; r++) {
        if (!wah_jit_numeric_type(body->local_runs[r].type)) { WAH_LOG("JIT: function %u has a non-numeric local", local_idx); goto fail; }
    }
    for (uint32_t i = 0; i < ac->instr_count; i++) {
        if (!wah_jit_supported(module, &ac->instrs[i])) {
            WAH_LOG("JIT: function %u has unsupported opcode %u", local_idx, ac->instrs[i].opcode);
            goto fail;
        }
    }
    for (uint32_t i = 0; i < wah_memory_index_limit(module); i++) {
        if (wah_memory_type(module, i)->addr_type != WAH_TYPE_I32) { WAH_LOG("JIT: %s", "memory64 is not supported"); goto fail; }
    }

    uint32_t start = jm->len, call_start = jm->call_count;
    for (int attempt = 0; attempt < 2; attempt++) {
        *f = (wah_jit_fn_t){ .jm = jm, .alloc = &module->alloc, .module = module, .ft = f->ft, .long_branches = attempt > 0 };
        jm->len = start;
        jm->call_count = call_start;
        wah_jit_compile_body(f, local_idx, body, ac);
        wah_jit_free_fn(f);
        if (!f->out_of_range) break;
    }
    if (f->fail) { WAH_LOG("JIT: function %u failed to compile", local_idx); goto fail; }
    return;
fail:
    jm->failed = true;
}

static void wah_jit_free_fn(wah_jit_fn_t *f) {
    wah_free(f->alloc, f->vals);
    wah_free(f->alloc, f->ctl);
    wah_free(f->alloc, f->labels);
    wah_free(f->alloc, f->patches);
    wah_free(f->alloc, f->stubs);
}

static void wah_jit_compile_body(wah_jit_fn_t *f, uint32_t local_idx, const wah_code_body_t *body,
                                 const wah_analyzed_code_t *ac) {
    wah_jit_module_t *jm = f->jm;
    f->nlocals = f->ft->param_count + body->local_count;
    f->max_stack = ac->max_stack_depth;
    f->link_off = f->nlocals * (uint32_t)sizeof(wah_value_t);
    f->frame_bytes = wah_jit_slot_off(f, f->max_stack + jm->extra_slots);
    {
        void *p = NULL;
        if (wah_malloc(f->alloc, (size_t)f->max_stack + 1, sizeof(wah_jit_val_t), &p) != WAH_OK) {
            WAH_JIT_FAIL(f, "out of memory");
            return;
        }
        f->vals = (wah_jit_val_t *)p;
    }
    wah_jit_reset(f, 0);
    f->l_trap = wah_jit_new_label(f);
    f->l_oob = wah_jit_new_label(f);
    f->l_stack = wah_jit_new_label(f);
    f->l_err = wah_jit_new_label(f);
    f->l_err_jit = wah_jit_new_label(f);
    if (f->fail) return;

    // Prologue: check the stack, save the return address, zero declared locals and poll
    jm->func_pos[local_idx] = wah_jit_pos(f);
    wah_jit_prologue(f);
    uint32_t first = f->ft->param_count, nzero = body->local_count;
    if (nzero <= 8) {
        for (uint32_t i = 0; i < nzero; i++) wah_jit_store_imm(f, WAH_JR_FP, (uint64_t)(first + i) * sizeof(wah_value_t), 0);
    } else {
        wah_jit_zero_loop(f, first * (uint32_t)sizeof(wah_value_t), nzero);
    }
    wah_jit_poll_check(f);

    for (uint32_t i = 0; i < ac->instr_count && !f->fail; i++) {
        wah_jit_instr(f, body, &ac->instrs[i], i + 1 < ac->instr_count ? &ac->instrs[i + 1] : NULL);
    }
    if (!f->dead || f->nctl != 0) WAH_JIT_FAIL(f, "unbalanced control"); // The function-level END should have been compiled
    wah_jit_emit_stubs(f);
    wah_jit_resolve_patches(f);
}

////////////////////////////////////////////////////////////////////////////////
// Modules and activations /////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#if defined(WAH_JIT_X86_64)

// Whether the CPU supports instructions always used by the JIT, also detecting optional ones.
static bool wah_jit_check_cpu(wah_jit_module_t *jm) {
    uint32_t ecx1, ebx7 = 0, ecx_ext = 0;
#ifdef _MSC_VER
    int r[4];
    __cpuid(r, 0);
    uint32_t max = (uint32_t)r[0];
    if (max < 1) return false;
    __cpuid(r, 1);
    ecx1 = (uint32_t)r[2];
    if (max >= 7) {
        __cpuidex(r, 7, 0);
        ebx7 = (uint32_t)r[1];
    }
    __cpuid(r, (int)0x80000000);
    if ((uint32_t)r[0] >= 0x80000001u) {
        __cpuid(r, (int)0x80000001);
        ecx_ext = (uint32_t)r[2];
    }
#else
    uint32_t eax, ebx, edx;
    if (!__get_cpuid(1, &eax, &ebx, &ecx1, &edx)) return false;
    if (__get_cpuid_max(0, NULL) >= 7) {
        uint32_t ecx;
        __cpuid_count(7, 0, eax, ebx7, ecx, edx);
    }
    if (!__get_cpuid(0x80000001u, &eax, &ebx, &ecx_ext, &edx)) ecx_ext = 0;
#endif
    jm->lzcnt = (ecx_ext >> 5) & 1;
    jm->tzcnt = (ebx7 >> 3) & 1; // BMI1
    return ((ecx1 >> 19) & 1) && ((ecx1 >> 23) & 1); // SSE4.1 and POPCNT
}

// Generates the trampoline: wah_error_t entry(ctx, code, resume_pc, fp) calls code or jumps to resume_pc.
// On Windows, its frame is described by unwind codes for the whole code.
static void wah_jit_trampoline(wah_jit_fn_t *f) {
#ifdef _WIN32
    static const int saved[] = {
        WAH_X64_RBX, WAH_X64_RBP, WAH_X64_RDI, WAH_X64_RSI, WAH_X64_R12, WAH_X64_R13, WAH_X64_R14, WAH_X64_R15,
    };
    const int a_ctx = WAH_X64_RCX, a_code = WAH_X64_RDX, a_resume = WAH_X64_R8, a_fp = WAH_X64_R9;
    struct { uint8_t off, op; int32_t extra; } ev[20]; // Prolog events: offset after it, operation and info, extra slot
    int nev = 0;
#else
    static const int saved[] = { WAH_X64_RBX, WAH_X64_RBP, WAH_X64_R12, WAH_X64_R13, WAH_X64_R14, WAH_X64_R15 };
    const int a_ctx = WAH_X64_RDI, a_code = WAH_X64_RSI, a_resume = WAH_X64_RDX, a_fp = WAH_X64_RCX;
#endif
    const int nsaved = (int)(sizeof(saved) / sizeof(*saved));
    for (int i = 0; i < nsaved; i++) {
        if (saved[i] & 8) wah_x64_byte(f, 0x41);
        wah_x64_byte(f, 0x50u + (uint32_t)(saved[i] & 7)); // PUSH
#ifdef _WIN32
        ev[nev].off = (uint8_t)wah_jit_pos(f); ev[nev].op = (uint8_t)(0 | saved[i] << 4); ev[nev++].extra = -1; // UWOP_PUSH_NONVOL
#endif
    }
    wah_x64_alu_imm(f, 5, WAH_X64_W, WAH_X64_RSP, WAH_X64_FRAME_SIZE);
#ifdef _WIN32
    ev[nev].off = (uint8_t)wah_jit_pos(f); ev[nev].op = 1; ev[nev++].extra = WAH_X64_FRAME_SIZE / 8; // UWOP_ALLOC_LARGE
    for (int i = 0; i < 10; i++) {
        wah_x64_mem(f, WAH_X64_MOVAPS_STORE, 0, 6 + i, WAH_X64_RSP, -1, 0, 48 + 16 * i);
        ev[nev].off = (uint8_t)wah_jit_pos(f); ev[nev].op = (uint8_t)(8 | (6 + i) << 4); ev[nev++].extra = 3 + i; // UWOP_SAVE_XMM128
    }
    {
        uint8_t *u = f->jm->unwind;
        uint32_t k = 4;
        u[0] = 1; // Version
        u[1] = (uint8_t)wah_jit_pos(f);
        u[3] = 0;
        for (int i = nev - 1; i >= 0; i--) {
            u[k++] = ev[i].off;
            u[k++] = ev[i].op;
            if (ev[i].extra >= 0) {
                u[k++] = (uint8_t)ev[i].extra;
                u[k++] = (uint8_t)(ev[i].extra >> 8);
            }
        }
        u[2] = (uint8_t)((k - 4) / 2);
    }
#endif
    wah_x64_mem(f, WAH_X64_MXCSR, 0, 3, WAH_X64_RSP, -1, 0, WAH_X64_MXCSR_OFF); // STMXCSR
    // The default MXCSR: round to nearest, no flush to zero, all exceptions masked
    wah_x64_mem(f, WAH_X64_MOV_IMM, 0, 0, WAH_X64_RSP, -1, 0, WAH_X64_MXCSR_OFF + 4);
    wah_x64_imm32(f, 0x1F80);
    wah_x64_mem(f, WAH_X64_MXCSR, 0, 2, WAH_X64_RSP, -1, 0, WAH_X64_MXCSR_OFF + 4); // LDMXCSR
    wah_x64_mov(f, true, WAH_JR_CTX, a_ctx);
    wah_x64_mov(f, true, WAH_JR_FP, a_fp);
    wah_jit_ldst(f, false, false, true, WAH_JR_MEM, WAH_JR_CTX, WAH_JIT_CTX_OFF(memory_base));
    wah_jit_ldst(f, false, false, true, WAH_JR_MEMSZ, WAH_JR_CTX, WAH_JIT_CTX_OFF(memory_size));
    wah_jit_ldst(f, false, false, true, WAH_JR_LIMIT, WAH_JR_CTX, WAH_JIT_CTX_OFF(frame_ptr));
    wah_jit_ldst(f, false, false, true, WAH_JR_GLOBALS, WAH_JR_CTX, WAH_JIT_CTX_OFF(globals));
    wah_x64_rr(f, WAH_X64_TEST, WAH_X64_W, a_resume, a_resume);
    uint32_t resume = wah_x64_jcc8(f, WAH_X64_NE);
    wah_x64_rr(f, WAH_X64_GRP5, 0, 2, a_code); // CALL, returning here
    wah_jit_mov_imm(f, WAH_X64_RAX, 0);
    uint32_t done = wah_x64_jcc8(f, WAH_X64_JMP);
    wah_x64_bind8(f, resume);
    wah_x64_rr(f, WAH_X64_GRP5, 0, 4, a_resume); // JMP
    wah_x64_bind8(f, done);
    f->jm->exit_pos = wah_jit_pos(f);
    wah_x64_mem(f, WAH_X64_MXCSR, 0, 2, WAH_X64_RSP, -1, 0, WAH_X64_MXCSR_OFF);
#ifdef _WIN32
    for (int i = 0; i < 10; i++) wah_x64_mem(f, WAH_X64_MOVAPS_LOAD, 0, 6 + i, WAH_X64_RSP, -1, 0, 48 + 16 * i);
#endif
    wah_x64_alu_imm(f, 0, WAH_X64_W, WAH_X64_RSP, WAH_X64_FRAME_SIZE);
    for (int i = nsaved - 1; i >= 0; i--) {
        if (saved[i] & 8) wah_x64_byte(f, 0x41);
        wah_x64_byte(f, 0x58u + (uint32_t)(saved[i] & 7)); // POP
    }
    wah_x64_byte(f, 0xC3);
}

#elif defined(WAH_JIT_AARCH64)

// Generates the trampoline: wah_error_t entry(ctx, code, resume_pc, fp) calls code or jumps to resume_pc.
static void wah_jit_trampoline(wah_jit_fn_t *f) {
    wah_a64(f, 0xA9800000u | (uint32_t)(-12 & 0x7f) << 15 | 30u << 10 | WAH_A64_SP << 5 | 29u); // STP x29, x30, [sp, #-96]!
    wah_a64_addsub_imm(f, WAH_A64_ADDI, true, 29, WAH_A64_SP, 0, false);
    for (int i = 0; i < 5; i++) { // STP x19+2i, x20+2i, [sp, #16+16i]
        wah_a64(f, 0xA9000000u | (uint32_t)(2 + 2 * i) << 15 | (uint32_t)(20 + 2 * i) << 10 | WAH_A64_SP << 5 | (uint32_t)(19 + 2 * i));
    }
    wah_a64_mov(f, true, WAH_JR_CTX, 0);
    wah_a64_mov(f, true, WAH_JR_FP, 3);
    wah_a64(f, 0xD53B4400u | WAH_JR_FPCR_HOST); // MRS x25, FPCR
    // Default NaN mode makes every NaN result canonical, and everything else stays at the IEEE defaults
    wah_a64_movw(f, WAH_A64_MOVZ, true, WAH_JR_FPCR_JIT, 0x200, 1);
    wah_a64(f, 0xD51B4400u | WAH_JR_FPCR_JIT);
    wah_a64_ldst(f, WAH_A64_LDRX, WAH_JR_MEM, WAH_JR_CTX, WAH_JIT_CTX_OFF(memory_base));
    wah_a64_ldst(f, WAH_A64_LDRX, WAH_JR_MEMSZ, WAH_JR_CTX, WAH_JIT_CTX_OFF(memory_size));
    wah_a64_ldst(f, WAH_A64_LDRX, WAH_JR_LIMIT, WAH_JR_CTX, WAH_JIT_CTX_OFF(frame_ptr));
    wah_a64_ldst(f, WAH_A64_LDRX, WAH_JR_GLOBALS, WAH_JR_CTX, WAH_JIT_CTX_OFF(globals));
    wah_a64(f, 0xB5000000u | 4u << 5 | 2u); // CBNZ x2, BR x2 below
    wah_a64(f, 0xD63F0000u | 1u << 5);      // BLR x1, returning here
    wah_a64_movw(f, WAH_A64_MOVZ, false, 0, 0, 0);
    wah_a64(f, 0x14000002u);                // B exit
    wah_a64(f, 0xD61F0000u | 2u << 5);      // BR x2
    f->jm->exit_pos = wah_jit_pos(f);
    wah_a64(f, 0xD51B4400u | WAH_JR_FPCR_HOST);
    for (int i = 4; i >= 0; i--) { // LDP
        wah_a64(f, 0xA9400000u | (uint32_t)(2 + 2 * i) << 15 | (uint32_t)(20 + 2 * i) << 10 | WAH_A64_SP << 5 | (uint32_t)(19 + 2 * i));
    }
    wah_a64(f, 0xA8C00000u | 12u << 15 | 30u << 10 | WAH_A64_SP << 5 | 29u); // LDP x29, x30, [sp], #96
    wah_a64(f, 0xD65F03C0u);
}

#endif // WAH_JIT_AARCH64

static void wah_jit_new_module(wah_module_t *module) {
    void *p = NULL;
    if (wah_malloc(&module->alloc, 1, sizeof(wah_jit_module_t), &p) != WAH_OK) return;
    wah_jit_module_t *jm = (wah_jit_module_t *)p;
    *jm = (wah_jit_module_t){0};
#if defined(WAH_JIT_X86_64)
    if (!wah_jit_check_cpu(jm)) {
        WAH_LOG("JIT: %s", "the CPU lacks SSE4.1 or POPCNT");
        wah_free(&module->alloc, jm);
        return;
    }
#endif
    module->jit = jm;
    wah_jit_fn_t f = { .jm = jm, .alloc = &module->alloc };
    wah_jit_trampoline(&f);
    if (f.fail) jm->failed = true;
}

static void wah_jit_free_module(wah_module_t *module) {
    wah_jit_module_t *jm = module->jit;
    if (!jm) return;
    if (jm->code) {
#ifdef _WIN32
        RtlDeleteFunctionTable((PRUNTIME_FUNCTION)jm->func_table);
        VirtualFree(jm->code, 0, MEM_RELEASE);
#else
        munmap(jm->code, jm->code_size);
#endif
    }
    wah_free(&module->alloc, jm->buf);
    wah_free(&module->alloc, jm->func_pos);
    wah_free(&module->alloc, jm->calls);
    wah_free(&module->alloc, jm);
    module->jit = NULL;
}

static void wah_jit_finish_module(wah_module_t *module) {
    wah_jit_module_t *jm = module->jit;
    if (jm->failed || module->wasm_function_count == 0) goto fail;
    for (uint32_t i = 0; i < jm->call_count; i++) {
        uint32_t pos = jm->calls[i].pos, target = jm->func_pos[jm->calls[i].func];
#if defined(WAH_JIT_X86_64)
        wah_jit_set32(jm, pos, target - (pos + 4));
#elif defined(WAH_JIT_AARCH64)
        int64_t d = ((int64_t)target - (int64_t)pos) / 4;
        if (d < -(1 << 25) || d >= (1 << 25)) goto fail;
        wah_jit_set32(jm, pos, wah_jit_get32(jm, pos) | ((uint32_t)d & 0x3ffffff));
#endif
    }
#ifdef _WIN32
    // Unwind data follows the code
    uint32_t code_end = jm->len, func_table_off;
    {
        static const uint8_t pad[4] = { 0xCC, 0xCC, 0xCC, 0xCC };
        wah_jit_fn_t fs = { .jm = jm, .alloc = &module->alloc };
        wah_jit_emit(&fs, pad, (4 - code_end % 4) % 4);
        uint32_t entry[3] = { 0, code_end, jm->len }; // RUNTIME_FUNCTION
        wah_jit_emit(&fs, jm->unwind, sizeof(jm->unwind));
        func_table_off = jm->len;
        wah_jit_emit(&fs, entry, sizeof(entry));
        if (fs.fail) goto fail;
    }
#endif
    jm->code_size = jm->len;
#ifdef _WIN32
    void *code = VirtualAlloc(NULL, jm->code_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!code) goto fail;
    memcpy(code, jm->buf, jm->code_size);
    DWORD old_protect;
    if (!VirtualProtect(code, jm->code_size, PAGE_EXECUTE_READ, &old_protect)) { VirtualFree(code, 0, MEM_RELEASE); goto fail; }
    FlushInstructionCache(GetCurrentProcess(), code, jm->code_size);
    jm->func_table = (uint8_t *)code + func_table_off;
    if (!RtlAddFunctionTable((PRUNTIME_FUNCTION)jm->func_table, 1, (DWORD64)(uintptr_t)code)) {
        VirtualFree(code, 0, MEM_RELEASE);
        goto fail;
    }
#elif defined(__APPLE__) && defined(WAH_JIT_AARCH64)
    void *code = mmap(NULL, jm->code_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANON | MAP_JIT, -1, 0);
    if (code == MAP_FAILED) goto fail;
    pthread_jit_write_protect_np(0);
    memcpy(code, jm->buf, jm->code_size);
    pthread_jit_write_protect_np(1);
    sys_icache_invalidate(code, jm->code_size);
#else
    void *code = mmap(NULL, jm->code_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (code == MAP_FAILED) goto fail;
    memcpy(code, jm->buf, jm->code_size);
    if (mprotect(code, jm->code_size, PROT_READ | PROT_EXEC) != 0) { munmap(code, jm->code_size); goto fail; }
#ifdef WAH_JIT_AARCH64
    __builtin___clear_cache((char *)code, (char *)code + jm->code_size);
#endif
#endif
    jm->code = (uint8_t *)code;
    wah_free(&module->alloc, jm->buf);
    wah_free(&module->alloc, jm->calls);
    jm->buf = NULL;
    jm->calls = NULL;
    return;
fail:
    wah_jit_free_module(module);
}

// Native code can run an activation only when it never leaves the module except for host functions.
// So functions of other modules shouldn't get into tables, by exporting them or importing references.
static bool wah_jit_ctx_usable(const wah_exec_context_t *ctx) {
    const wah_module_t *m = ctx->module;
    if (m->import_table_count > 0) return false;
    for (uint32_t i = 0; i < m->import_global_count; i++) {
        if (WAH_TYPE_IS_REF(m->global_imports[i].type)) return false;
    }
    for (uint32_t i = 0; i < m->export_count; i++) {
        if (m->exports[i].kind == WAH_KIND_TABLE) return false;
    }
    for (uint32_t i = 0; i < m->import_function_count; i++) {
        if (!ctx->function_table[i].func.is_host) return false;
    }
    return true;
}

static void wah_jit_instantiate(wah_exec_context_t *ctx) {
    const wah_module_t *m = ctx->module;
    void *p = NULL;
    if (!m->jit || !m->jit->code || !wah_jit_ctx_usable(ctx)) return;
    if (wah_malloc(&ctx->alloc, 1, sizeof(wah_jit_ctx_t), &p) != WAH_OK) return; // Interpreted then
    ctx->jit = (wah_jit_ctx_t *)p;
    *ctx->jit = (wah_jit_ctx_t){0};
}

static bool wah_jit_run(wah_exec_context_t *ctx, wah_error_t *out) {
    const wah_module_t *m = ctx->module;
    wah_jit_module_t *jm = m->jit;
    if (!ctx->jit) return false;
    wah_jit_ctx_t *jc = ctx->jit;
    wah_call_frame_t *frame = ctx->frame_ptr;
    const void *code = NULL, *resume = jc->resume_pc;
    wah_value_t *fp;
    if (resume) {
        fp = jc->resume_fp;
        jc->resume_pc = NULL;
    } else {
        // Only fresh activations of this module, as wah_start leaves them
        if (ctx->call_depth != ctx->lifecycle.base_call_depth + 1) return false;
        if (frame->module != m || frame->frame_ctx != ctx || frame->bytecode_ip != frame->code->parsed_code.bytecode) return false;
        code = jm->code + jm->func_pos[frame->func_idx];
        fp = frame->locals;
    }
    wah_error_t err = ((wah_jit_entry_t)(uintptr_t)jm->code)(ctx, code, resume, fp);
    if (err == WAH_OK) { // Pop the frame like the interpreter, leaving results at its locals
        ctx->sp = frame->locals + frame->result_count;
        ctx->call_depth--;
        ctx->frame_ptr++;
    }
    *out = err;
    return true;
}

static void wah_jit_cancel(wah_exec_context_t *ctx) {
    if (ctx->jit) ctx->jit->resume_pc = NULL;
}

static void wah_jit_free_context(wah_exec_context_t *ctx) {
    wah_alloc_t alloc = wah_resolve_alloc(&ctx->alloc);
    wah_free(&alloc, ctx->jit);
    ctx->jit = NULL;
}

#endif // WAH_JIT
#endif // WAHJIT_H
