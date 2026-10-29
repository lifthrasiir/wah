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
// - x86-64 on Windows, macOS or Linux, with SSE4.1 and POPCNT (otherwise every module is interpreted),
//   and SSE4.2 for SIMD (otherwise modules using SIMD are interpreted)
// - AArch64 on macOS or Linux
// Elsewhere, or with WAH_NO_JIT defined, this is the same as including wah.h.
//
// This file works by enabling a few hooks defined in wah.h, only available when WAH_JIT is defined.
// `wah_alloc_t` is partly ignored by the JIT; some memory (namely executable code) is directly mapped from the OS.
// The user can opt out of JIT compilation by setting `wah_parse_options_t::disable_jit` to true.
// Native execution only differs from the interpreter by resource usage and polling behavior;
// otherwise they are identical.
// `wah_enable_guard_pages` lets native code skip explicit bounds checks for most memory accesses.
// Out-of-bounds stores straddling the end of memory are assumed to write nothing before faulting,
// which holds on x86-64 and was confirmed on Apple CPUs but not on other AArch64 implementations.
//
// Only a subset of modules are compiled: those without fuel metering, using only numeric and vector types (i32, i64,
// f32, f64 and v128) and WASM 2.0 instructions plus relaxed SIMD, except reference types and table instructions other
// than `call_indirect`.
// Any number of memories can be used, but memory64 accesses are always bounds-checked even with guard pages.
// Any other feature (e.g. GC, exceptions or tail calls) leaves the whole module interpreted. Its activation runs natively when all of its function imports are host functions,
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
#include <signal.h>
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

// A guarded memory reserves 4 GiB for any 32-bit address, followed by guard pages for offsets up to WAH_JIT_GUARD_SIZE.
// Accesses with larger offsets are still checked.
#define WAH_JIT_GUARD_SIZE ((uint64_t)1 << 31)
#define WAH_JIT_GUARD_RESERVE (((uint64_t)1 << 32) + WAH_JIT_GUARD_SIZE)

typedef struct wah_jit_module_s {
    uint8_t *buf;               // Code being generated; freed when finished
    uint32_t len, cap;          // In bytes, like all code offsets
    uint32_t *func_pos;         // Offset of each defined function
    wah_jit_call_fixup_t *calls; // Direct calls to be patched when all functions are compiled
    uint32_t call_count, call_cap;
    uint32_t exit_pos;          // Offset of the trampoline exit, where the return register holds the result
    uint32_t oob_pos;           // Offset of the exit with WAH_ERROR_MEMORY_OUT_OF_BOUNDS, for faults on guard pages
    uint32_t extra_slots;       // Slots above the operand stack for host call results
    bool failed;                // Some function couldn't be compiled, so the module is interpreted
    bool guarded;               // Relies on guard pages, so every memory of the activation should be guarded
#ifdef WAH_JIT_X86_64
    bool lzcnt, tzcnt, sse42;   // Optional instructions supported by the CPU
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

// A value on the operand stack being compiled. LGPR, LFPR and LVEC are values of locals pinned to registers,
// read from them until the local is set. FPR and LFPR only have 64 bits; v128 values are in VEC or LVEC instead,
// which use all 128 bits of FP registers.
enum { WAH_JV_SLOT, WAH_JV_GPR, WAH_JV_FPR, WAH_JV_CONST, WAH_JV_FLAGS, WAH_JV_LGPR, WAH_JV_LFPR, WAH_JV_VEC, WAH_JV_LVEC };
typedef struct {
    uint8_t kind;
    uint8_t reg; // Register for GPR, FPR, VEC, LGPR, LFPR and LVEC, condition code for FLAGS
    int64_t imm; // Value for CONST, local index for LGPR, LFPR and LVEC
} wah_jit_val_t;

// A local living in a register during the whole function, except around calls and suspensions
typedef struct { uint32_t idx; uint8_t reg; bool fp, vec; } wah_jit_pin_t;
#define WAH_JIT_MAX_PINS 32
#define WAH_JIT_PIN_FP 32
#define WAH_JIT_PIN_VEC 64 // Along with WAH_JIT_PIN_FP

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

// A 16-byte constant placed after the function, loaded by a label
typedef struct { uint8_t bytes[16]; uint32_t label; } wah_jit_vconst_t;

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
    const wah_decoded_instr_t *instrs; // Instructions of the function, and the index of the one being compiled
    uint32_t ninstrs, instr_idx;
    int8_t *pin_of;          // Register of each local if pinned, plus WAH_JIT_PIN_FP for FPRs, -1 otherwise
    wah_jit_pin_t pins[WAH_JIT_MAX_PINS];
    uint32_t npins, gpr_pins, fpr_pins; // Masks of pinned registers

    wah_jit_val_t *vals;
    uint32_t h;
    int32_t gpr[32], fpr[32];
    int32_t gpr_local[32], fpr_local[32]; // Unpinned local whose value a free register still holds, -1 if none
    int hint;                // Pinned register for the result of the instruction, which then sets it; -1 if none
    uint32_t hint_idx;       // Local of the hint

    wah_jit_ctl_t *ctl;
    uint32_t nctl, ctl_cap;
    uint32_t *labels; // Code offsets, UINT32_MAX if not bound yet
    uint32_t nlabels, labels_cap;
    wah_jit_patch_t *patches;
    uint32_t npatches, patches_cap;
    wah_jit_stub_t *stubs;
    uint32_t nstubs, stubs_cap;
    wah_jit_vconst_t *vconsts;
    uint32_t nvconsts, vconsts_cap;
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

#if ((WAH_COMPILED_FEATURES) & WAH_FEATURE_SIMD)
// Returns the label of a 16-byte constant, which is emitted after the function.
static uint32_t wah_jit_vconst_label(wah_jit_fn_t *f, const uint8_t *bytes) {
    for (uint32_t i = 0; i < f->nvconsts; i++) {
        if (memcmp(f->vconsts[i].bytes, bytes, 16) == 0) return f->vconsts[i].label;
    }
    uint32_t label = wah_jit_new_label(f);
    WAH_JIT_GROW(f, vconsts, nvconsts, vconsts_cap);
    if (f->fail) return label;
    wah_jit_vconst_t *c = &f->vconsts[f->nvconsts++];
    memcpy(c->bytes, bytes, 16);
    c->label = label;
    return label;
}

// Constant lanes of 1 << size_log2 bytes, all set to v
typedef struct { uint8_t b[16]; } wah_jit_v16_t;
static wah_jit_v16_t wah_jit_splat_const(uint64_t v, uint32_t size_log2) {
    wah_jit_v16_t c;
    for (uint32_t i = 0; i < 16; i++) c.b[i] = (uint8_t)(v >> (8 * (i & ((1u << size_log2) - 1))));
    return c;
}
#endif

static void wah_jit_emit_vconsts(wah_jit_fn_t *f) {
    static const uint8_t pad[16] = {0};
    if (f->nvconsts == 0) return;
    wah_jit_emit(f, pad, (16 - wah_jit_pos(f) % 16) % 16); // Code is mapped at page boundaries
    for (uint32_t i = 0; i < f->nvconsts; i++) {
        wah_jit_bind(f, f->vconsts[i].label);
        wah_jit_emit(f, f->vconsts[i].bytes, 16);
    }
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

// Registers for pinned locals, in the order of preference. The operand stack cache keeps at least rax, rcx, rdx, rsi
// and rdi, as division and shifts need rax, rdx and rcx.
// The last one is the memory size register, only available when guard pages make it unnecessary.
static const uint8_t wah_jit_pin_gprs[] = { WAH_X64_R15, WAH_X64_R14, WAH_X64_R9, WAH_X64_R8, WAH_JR_MEMSZ };
static const uint8_t wah_jit_pin_fprs[] = { 13, 12, 11, 10, 9, 8 };
#define WAH_JIT_PIN_GPR_COUNT 5
#define WAH_JIT_PIN_FPR_COUNT 6

// Weight of uses of locals at a loop depth for pinning. The few registers go to innermost loops.
#define WAH_JIT_USE_WEIGHT(depth) ((uint64_t)1 << (3 * ((depth) < 8 ? (depth) : 8)))

// Operands of instructions whose result can be computed right into a pinned register, 0 if not supported
static uint32_t wah_jit_hint_arity(uint16_t op) {
    switch (op) {
        case WAH_OP_I32_ADD: case WAH_OP_I32_SUB: case WAH_OP_I32_MUL: case WAH_OP_I32_AND: case WAH_OP_I32_OR:
        case WAH_OP_I32_XOR: case WAH_OP_I64_ADD: case WAH_OP_I64_SUB: case WAH_OP_I64_MUL: case WAH_OP_I64_AND:
        case WAH_OP_I64_OR: case WAH_OP_I64_XOR:
            return 2;
        case WAH_OP_I32_LOAD: case WAH_OP_I64_LOAD: case WAH_OP_I32_LOAD8_S: case WAH_OP_I32_LOAD8_U:
        case WAH_OP_I32_LOAD16_S: case WAH_OP_I32_LOAD16_U: case WAH_OP_I64_LOAD8_S: case WAH_OP_I64_LOAD8_U:
        case WAH_OP_I64_LOAD16_S: case WAH_OP_I64_LOAD16_U: case WAH_OP_I64_LOAD32_S: case WAH_OP_I64_LOAD32_U:
            return 1;
        default: return 0;
    }
}

// i32 values are kept zero-extended in registers and slots, so that addresses need no extension.
// Values from outside (params of the activation and results of helpers) are zero-extended as they come in.
#define WAH_JIT_ZEXT_I32 1

// rax, rcx, rdx, rsi, rdi, r8, r9 and xmm0-xmm13 are allocatable unless pinned
static void wah_jit_init_regs(wah_jit_fn_t *f) {
    for (int r = 0; r < 32; r++) {
        f->gpr[r] = (r <= WAH_X64_RDX || r == WAH_X64_RSI || r == WAH_X64_RDI || r == WAH_X64_R8 || r == WAH_X64_R9) &&
                    !(f->gpr_pins >> r & 1) ? WAH_JO_FREE : WAH_JO_NONE;
        f->fpr[r] = r < 14 && !(f->fpr_pins >> r & 1) ? WAH_JO_FREE : WAH_JO_NONE;
    }
}

static void wah_jit_mov64(wah_jit_fn_t *f, int rd, int rs) { wah_x64_mov(f, true, rd, rs); }
static void wah_jit_fmov(wah_jit_fn_t *f, int vd, int vs) { wah_x64_rr(f, WAH_X64_MOVAPS_LOAD, 0, vd, vs); }
static void wah_jit_vmov(wah_jit_fn_t *f, int vd, int vs) { wah_jit_fmov(f, vd, vs); }

// Loads or stores 128 bits at [base + off], which may be unaligned.
static void wah_jit_vldst(wah_jit_fn_t *f, bool store, int v, int base, uint64_t off) {
    if (off > INT32_MAX) { WAH_JIT_FAIL(f, "offset too large"); return; }
    wah_x64_mem(f, store ? WAH_X64_MOVS_STORE : WAH_X64_MOVS_LOAD, 0, v, base, -1, 0, (int32_t)off); // MOVUPS
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
#define WAH_JR_T0 16
#define WAH_JR_T1 17
#define WAH_JR_VT0 30
#define WAH_JR_VT1 31

// The trampoline frame at sp, which has to be 16-byte aligned
typedef struct { uint64_t x29, x30, x19[10], d8[8], fpcr_host, pad; } wah_a64_frame_t;
#define WAH_A64_FRAME_OFF(field) ((uint32_t)offsetof(wah_a64_frame_t, field))

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
#define WAH_A64_STRQ   0x3D800000u
#define WAH_A64_LDRQ   0x3DC00000u
#define WAH_A64_LDST_SIZE(op) (((op) & 0xC4800000u) == 0x04800000u ? 4u : (op) >> 30)

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
    } else if (rn == WAH_JR_T1) { // Such as globals, using x16 for offsets over 24 bits
        wah_a64_add_imm(f, WAH_JR_T1, WAH_JR_T1, off);
        wah_a64(f, op | (uint32_t)WAH_JR_T1 << 5 | (uint32_t)rt);
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

// Sets FPCR for the JIT, using x16. Default NaN mode makes every NaN result canonical, and everything else stays at
// the IEEE defaults.
static void wah_a64_set_fpcr(wah_jit_fn_t *f) {
    wah_a64_movw(f, WAH_A64_MOVZ, true, WAH_JR_T0, 0x200, 1);
    wah_a64(f, 0xD51B4400u | WAH_JR_T0); // MSR FPCR, x16
}

// Primitives used by the shared code ///////////////////////////////////////////

// Registers for pinned locals, in the order of preference. The operand stack cache keeps at least x0-x5.
// The last one is the memory size register, only available when guard pages make it unnecessary.
static const uint8_t wah_jit_pin_gprs[] = { 28, 27, 26, 25, 24, 23, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, WAH_JR_MEMSZ };
static const uint8_t wah_jit_pin_fprs[] = { 15, 14, 13, 12, 11, 10, 9, 8 };
#define WAH_JIT_PIN_GPR_COUNT 17
#define WAH_JIT_PIN_FPR_COUNT 8

// Weight of uses of locals at a loop depth for pinning. Small inner loops often run only a few iterations,
// so exponential weights would give them all registers at the expense of their enclosing loops.
#define WAH_JIT_USE_WEIGHT(depth) ((uint64_t)(4 * (uint64_t)(depth) + 1))

// Operands of instructions whose result can be computed right into a pinned register, 0 if not supported.
// They write the result only after reading operands, which may be in the same register.
static uint32_t wah_jit_hint_arity(uint16_t op) {
    switch (op) {
        case WAH_OP_I32_ADD: case WAH_OP_I32_SUB: case WAH_OP_I32_MUL: case WAH_OP_I32_AND: case WAH_OP_I32_OR:
        case WAH_OP_I32_XOR: case WAH_OP_I32_SHL: case WAH_OP_I32_SHR_S: case WAH_OP_I32_SHR_U: case WAH_OP_I32_ROTL:
        case WAH_OP_I32_ROTR: case WAH_OP_I64_ADD: case WAH_OP_I64_SUB: case WAH_OP_I64_MUL: case WAH_OP_I64_AND:
        case WAH_OP_I64_OR: case WAH_OP_I64_XOR: case WAH_OP_I64_SHL: case WAH_OP_I64_SHR_S: case WAH_OP_I64_SHR_U:
        case WAH_OP_I64_ROTL: case WAH_OP_I64_ROTR:
            return 2;
        case WAH_OP_I32_LOAD: case WAH_OP_I64_LOAD: case WAH_OP_I32_LOAD8_S: case WAH_OP_I32_LOAD8_U:
        case WAH_OP_I32_LOAD16_S: case WAH_OP_I32_LOAD16_U: case WAH_OP_I64_LOAD8_S: case WAH_OP_I64_LOAD8_U:
        case WAH_OP_I64_LOAD16_S: case WAH_OP_I64_LOAD16_U: case WAH_OP_I64_LOAD32_S: case WAH_OP_I64_LOAD32_U:
            return 1;
        default: return 0;
    }
}

// The upper half of i32 values is undefined, and 32-bit operations ignore it. Addresses are zero-extended when used.
#define WAH_JIT_ZEXT_I32 0

// x0-x15 and v0-v29 are allocatable unless pinned
static void wah_jit_init_regs(wah_jit_fn_t *f) {
    for (int r = 0; r < 32; r++) {
        f->gpr[r] = r < 16 && !(f->gpr_pins >> r & 1) ? WAH_JO_FREE : WAH_JO_NONE;
        f->fpr[r] = r < 30 && !(f->fpr_pins >> r & 1) ? WAH_JO_FREE : WAH_JO_NONE;
    }
}

static void wah_jit_mov64(wah_jit_fn_t *f, int rd, int rs) { wah_a64_mov(f, true, rd, rs); }
static void wah_jit_fmov(wah_jit_fn_t *f, int vd, int vs) { wah_a64(f, 0x1E604000u | (uint32_t)vs << 5 | (uint32_t)vd); } // FMOV d, d
static void wah_jit_vmov(wah_jit_fn_t *f, int vd, int vs) { // MOV v.16b
    wah_a64(f, 0x4EA01C00u | (uint32_t)vs << 16 | (uint32_t)vs << 5 | (uint32_t)vd);
}

// Loads or stores 128 bits at [base + off].
static void wah_jit_vldst(wah_jit_fn_t *f, bool store, int v, int base, uint64_t off) {
    wah_a64_ldst(f, store ? WAH_A64_STRQ : WAH_A64_LDRQ, v, base, off);
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

// Whether a value of the kind owns its FP register
static bool wah_jit_owns_fpr(uint8_t kind) { return kind == WAH_JV_FPR || kind == WAH_JV_VEC; }

// Stores the value at index i of the operand stack to [fp + off] without changing its state.
// Slots are copied as a whole, as they may hold v128 values. This doesn't change flags.
static void wah_jit_copy_val(wah_jit_fn_t *f, uint32_t i, uint32_t off) {
    const wah_jit_val_t *v = &f->vals[i];
    switch (v->kind) {
        case WAH_JV_GPR: case WAH_JV_LGPR: wah_jit_ldst(f, true, false, true, v->reg, WAH_JR_FP, off); break;
        case WAH_JV_FPR: case WAH_JV_LFPR: wah_jit_ldst(f, true, true, true, v->reg, WAH_JR_FP, off); break;
        case WAH_JV_VEC: case WAH_JV_LVEC: wah_jit_vldst(f, true, v->reg, WAH_JR_FP, off); break;
        case WAH_JV_CONST: wah_jit_store_imm(f, WAH_JR_FP, off, v->imm); break;
        case WAH_JV_FLAGS:
            wah_jit_setcc(f, WAH_JR_T0, v->reg);
            wah_jit_ldst(f, true, false, true, WAH_JR_T0, WAH_JR_FP, off);
            break;
        case WAH_JV_SLOT:
            if (wah_jit_slot_off(f, i) != off) {
                wah_jit_vldst(f, false, WAH_JR_VT1, WAH_JR_FP, wah_jit_slot_off(f, i));
                wah_jit_vldst(f, true, WAH_JR_VT1, WAH_JR_FP, off);
            }
            break;
    }
}

static void wah_jit_spill(wah_jit_fn_t *f, uint32_t i) {
    wah_jit_val_t *v = &f->vals[i];
    if (v->kind == WAH_JV_SLOT) return;
    wah_jit_copy_val(f, i, wah_jit_slot_off(f, i));
    if (v->kind == WAH_JV_GPR) f->gpr[v->reg] = WAH_JO_FREE;
    if (wah_jit_owns_fpr(v->kind)) f->fpr[v->reg] = WAH_JO_FREE;
    v->kind = WAH_JV_SLOT;
}

static void wah_jit_flush(wah_jit_fn_t *f) {
    for (uint32_t i = 0; i < f->h; i++) wah_jit_spill(f, i);
}

// Forgets register contents, as at a merge point where everything is in slots.
static void wah_jit_reset(wah_jit_fn_t *f, uint32_t h) {
    wah_jit_init_regs(f);
    for (int r = 0; r < 32; r++) f->gpr_local[r] = f->fpr_local[r] = -1;
    for (uint32_t i = 0; i < h; i++) f->vals[i] = (wah_jit_val_t){ .kind = WAH_JV_SLOT };
    f->h = h;
}

// Returns a free register, preferring those not holding values of locals, or -1 if none.
static int wah_jit_free_reg(wah_jit_fn_t *f, bool fp) {
    const int32_t *owner = fp ? f->fpr : f->gpr, *local = fp ? f->fpr_local : f->gpr_local;
    int any = -1;
    for (int r = 31; r >= 0; r--) {
        if (owner[r] != WAH_JO_FREE) continue;
        if (local[r] < 0) return r;
        if (any < 0) any = r;
    }
    return any;
}

// Returns a register reserved as WAH_JO_TEMP, spilling the deepest cached value if needed.
// Registers are tried from the highest, which on x86-64 avoids those with special uses,
// and those holding values of locals are taken last.
static int wah_jit_alloc(wah_jit_fn_t *f, bool fp) {
    int32_t *owner = fp ? f->fpr : f->gpr;
    int r = wah_jit_free_reg(f, fp);
    if (r >= 0) {
        owner[r] = WAH_JO_TEMP;
        (fp ? f->fpr_local : f->gpr_local)[r] = -1;
        return r;
    }
    for (uint32_t i = 0; i < f->h; i++) {
        wah_jit_val_t *v = &f->vals[i];
        if (fp ? wah_jit_owns_fpr(v->kind) : v->kind == WAH_JV_GPR) {
            r = v->reg;
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
        case WAH_JV_LGPR: wah_jit_mov64(f, r, v->reg); break;
        case WAH_JV_LFPR: wah_jit_fpr_to_gpr(f, r, v->reg); break;
        case WAH_JV_CONST: wah_jit_mov_imm(f, r, (uint64_t)v->imm); break;
        case WAH_JV_FLAGS: wah_jit_setcc(f, r, v->reg); break;
        case WAH_JV_SLOT: wah_jit_ldst(f, false, false, true, r, WAH_JR_FP, wah_jit_slot_off(f, i)); break;
        default: WAH_JIT_FAIL(f, "not a scalar value"); break;
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
        case WAH_JV_LGPR: wah_jit_gpr_to_fpr(f, r, v->reg); break;
        case WAH_JV_LFPR: wah_jit_fmov(f, r, v->reg); break;
        case WAH_JV_CONST:
            wah_jit_mov_imm(f, WAH_JR_T0, (uint64_t)v->imm);
            wah_jit_gpr_to_fpr(f, r, WAH_JR_T0);
            break;
        case WAH_JV_FLAGS:
            wah_jit_setcc(f, WAH_JR_T0, v->reg);
            wah_jit_gpr_to_fpr(f, r, WAH_JR_T0);
            break;
        case WAH_JV_SLOT: wah_jit_ldst(f, false, true, true, r, WAH_JR_FP, wah_jit_slot_off(f, i)); break;
        default: WAH_JIT_FAIL(f, "not a scalar value"); break;
    }
    *v = (wah_jit_val_t){ .kind = WAH_JV_FPR, .reg = (uint8_t)r };
    f->fpr[r] = (int32_t)i;
    return r;
}

// Moves the v128 value at index i into an FP register.
static int wah_jit_to_vec(wah_jit_fn_t *f, uint32_t i) {
    wah_jit_val_t *v = &f->vals[i];
    if (v->kind == WAH_JV_VEC) return v->reg;
    int r = wah_jit_alloc(f, true);
    switch (v->kind) {
        case WAH_JV_LVEC: wah_jit_vmov(f, r, v->reg); break;
        case WAH_JV_SLOT: wah_jit_vldst(f, false, r, WAH_JR_FP, wah_jit_slot_off(f, i)); break;
        default: WAH_JIT_FAIL(f, "not a v128 value"); break;
    }
    *v = (wah_jit_val_t){ .kind = WAH_JV_VEC, .reg = (uint8_t)r };
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

static int wah_jit_pop_vec(wah_jit_fn_t *f) {
    int r = wah_jit_to_vec(f, f->h - 1);
    f->fpr[r] = WAH_JO_TEMP;
    f->h--;
    return r;
}

// Pops a value only to be read, which may be a pinned register.
static int wah_jit_pop_gpr_ro(wah_jit_fn_t *f) {
    const wah_jit_val_t *v = &f->vals[f->h - 1];
    if (v->kind != WAH_JV_LGPR) return wah_jit_pop_gpr(f);
    f->h--;
    return v->reg;
}

static int wah_jit_pop_fpr_ro(wah_jit_fn_t *f) {
    const wah_jit_val_t *v = &f->vals[f->h - 1];
    if (v->kind != WAH_JV_LFPR) return wah_jit_pop_fpr(f);
    f->h--;
    return v->reg;
}

static int wah_jit_pop_vec_ro(wah_jit_fn_t *f) {
    const wah_jit_val_t *v = &f->vals[f->h - 1];
    if (v->kind != WAH_JV_LVEC) return wah_jit_pop_vec(f);
    f->h--;
    return v->reg;
}

// A destination register for operands a and b (-1 if none), reusing them if they are temporaries.
// Operands other than the destination should be freed afterwards.
static int wah_jit_dest(wah_jit_fn_t *f, bool fp, int a, int b) {
    const int32_t *owner = fp ? f->fpr : f->gpr;
    if (!fp && f->hint >= 0) return f->hint;
    if (owner[a] == WAH_JO_TEMP) return a;
    if (b >= 0 && owner[b] == WAH_JO_TEMP) return b;
    return wah_jit_alloc(f, fp);
}

static void wah_jit_free_gpr(wah_jit_fn_t *f, int r) { if (f->gpr[r] == WAH_JO_TEMP) f->gpr[r] = WAH_JO_FREE; }

// Guarded memories never move, so free registers keep data pointers of memories other than 0 like values of locals,
// with nlocals + memidx in gpr_local. Returns such a register reserved as WAH_JO_TEMP, or -1 if none.
static int wah_jit_cached_base(wah_jit_fn_t *f, uint32_t memidx) {
    for (int r = 31; f->jm->guarded && r >= 0; r--) {
        if (f->gpr[r] == WAH_JO_FREE && f->gpr_local[r] == (int32_t)(f->nlocals + memidx)) {
            f->gpr[r] = WAH_JO_TEMP;
            return r;
        }
    }
    return -1;
}
// memory64 is never guarded, as addresses can be anything, so its accesses are always checked.
static bool wah_jit_mem64(const wah_jit_fn_t *f, uint32_t memidx) {
    return wah_memory_type(f->module, memidx)->addr_type == WAH_TYPE_I64;
}

// End of an access relative to the address, saturated as it is out of bounds anyway if overflown
static uint64_t wah_jit_access_end(const wah_memarg_t *m, uint32_t size) {
    return m->offset + size < m->offset ? UINT64_MAX : m->offset + size;
}

static void wah_jit_free_fpr(wah_jit_fn_t *f, int r) { if (f->fpr[r] == WAH_JO_TEMP) f->fpr[r] = WAH_JO_FREE; }

static bool wah_jit_is_fpr(uint8_t kind) { return kind == WAH_JV_FPR || kind == WAH_JV_LFPR; }

static void wah_jit_push(wah_jit_fn_t *f, wah_jit_val_t v) {
    if (v.kind == WAH_JV_GPR) f->gpr[v.reg] = (int32_t)f->h;
    if (wah_jit_owns_fpr(v.kind)) f->fpr[v.reg] = (int32_t)f->h;
    f->vals[f->h++] = v;
}
// The hinted register is pushed as the value of its local, which is about to be set
static void wah_jit_push_gpr(wah_jit_fn_t *f, int r) {
    bool pinned = r == f->hint;
    wah_jit_push(f, (wah_jit_val_t){ .kind = pinned ? WAH_JV_LGPR : WAH_JV_GPR, .reg = (uint8_t)r, .imm = pinned ? f->hint_idx : 0 });
}
static void wah_jit_push_fpr(wah_jit_fn_t *f, int r) { wah_jit_push(f, (wah_jit_val_t){ .kind = WAH_JV_FPR, .reg = (uint8_t)r }); }
static void wah_jit_push_vec(wah_jit_fn_t *f, int r) { wah_jit_push(f, (wah_jit_val_t){ .kind = WAH_JV_VEC, .reg = (uint8_t)r }); }

static void wah_jit_drop(wah_jit_fn_t *f) {
    wah_jit_val_t *v = &f->vals[--f->h];
    if (v->kind == WAH_JV_GPR) f->gpr[v->reg] = WAH_JO_FREE;
    if (wah_jit_owns_fpr(v->kind)) f->fpr[v->reg] = WAH_JO_FREE;
}

// Frees operands a and b (-1 if none) except d, then pushes d.
static void wah_jit_finish_gpr(wah_jit_fn_t *f, int d, int a, int b) {
    if (a != d) wah_jit_free_gpr(f, a);
    if (b >= 0 && b != d) wah_jit_free_gpr(f, b);
    wah_jit_push_gpr(f, d);
}

static void wah_jit_finish_fpr(wah_jit_fn_t *f, int d, int a, int b) {
    if (a != d) wah_jit_free_fpr(f, a);
    if (b >= 0 && b != d) wah_jit_free_fpr(f, b);
    wah_jit_push_fpr(f, d);
}

#if ((WAH_COMPILED_FEATURES) & WAH_FEATURE_RELAXED_SIMD)
// A destination register for v128 operands, preferring those which are temporaries in the given order.
static int wah_jit_vdest3(wah_jit_fn_t *f, int a, int b, int c) {
    if (f->fpr[a] == WAH_JO_TEMP) return a;
    if (b >= 0 && f->fpr[b] == WAH_JO_TEMP) return b;
    if (c >= 0 && f->fpr[c] == WAH_JO_TEMP) return c;
    return wah_jit_alloc(f, true);
}
#endif

#if ((WAH_COMPILED_FEATURES) & WAH_FEATURE_SIMD)
// Frees operands a, b and c (-1 if none) except d, then pushes d as v128.
static void wah_jit_finish_vec(wah_jit_fn_t *f, int d, int a, int b, int c) {
    if (a >= 0 && a != d) wah_jit_free_fpr(f, a);
    if (b >= 0 && b != d) wah_jit_free_fpr(f, b);
    if (c >= 0 && c != d) wah_jit_free_fpr(f, c);
    wah_jit_push_vec(f, d);
}
#endif

// A condition for branches, either flags or a register to compare with zero.
typedef struct { bool flags; int cond, reg; } wah_jit_cond_t;

static wah_jit_cond_t wah_jit_pop_cond(wah_jit_fn_t *f) {
    wah_jit_val_t *v = &f->vals[f->h - 1];
    if (v->kind == WAH_JV_FLAGS) {
        f->h--;
        return (wah_jit_cond_t){ .flags = true, .cond = v->reg };
    }
    return (wah_jit_cond_t){ .flags = false, .reg = wah_jit_pop_gpr_ro(f) };
}

static void wah_jit_branch_if(wah_jit_fn_t *f, wah_jit_cond_t c, bool when, uint32_t label) {
    if (c.flags) wah_jit_bcond(f, when ? c.cond : c.cond ^ 1, label);
    else wah_jit_cbz(f, when, false, c.reg, label);
}

static void wah_jit_release_cond(wah_jit_fn_t *f, wah_jit_cond_t c) { if (!c.flags) wah_jit_free_gpr(f, c.reg); }

// Stores pinned locals to their slots, or loads them back.
static void wah_jit_sync_pins(wah_jit_fn_t *f, bool store) {
    for (uint32_t i = 0; i < f->npins; i++) {
        const wah_jit_pin_t *p = &f->pins[i];
        uint32_t off = p->idx * (uint32_t)sizeof(wah_value_t);
        if (p->vec) wah_jit_vldst(f, store, p->reg, WAH_JR_FP, off);
        else wah_jit_ldst(f, store, p->fp, true, p->reg, WAH_JR_FP, off);
    }
}

// The value of local idx pinned to a register, which is read from it until set
static wah_jit_val_t wah_jit_pinned_val(const wah_jit_fn_t *f, uint32_t idx) {
    int8_t p = f->pin_of[idx];
    uint8_t kind = p & WAH_JIT_PIN_VEC ? WAH_JV_LVEC : p & WAH_JIT_PIN_FP ? WAH_JV_LFPR : WAH_JV_LGPR;
    return (wah_jit_val_t){ .kind = kind, .reg = (uint8_t)(p & (WAH_JIT_PIN_FP - 1)), .imm = idx };
}

// Sets the pinned local at idx to the top of the stack, which is kept.
static void wah_jit_set_pinned(wah_jit_fn_t *f, uint32_t idx) {
    int d = f->pin_of[idx] & (WAH_JIT_PIN_FP - 1);
    bool fp = (f->pin_of[idx] & WAH_JIT_PIN_FP) != 0;
    for (uint32_t i = 0; i + 1 < f->h; i++) { // Old values of the local are copied first
        wah_jit_val_t *v = &f->vals[i];
        if ((v->kind == WAH_JV_LGPR || v->kind == WAH_JV_LFPR || v->kind == WAH_JV_LVEC) && v->imm == (int64_t)idx) {
            if (v->kind == WAH_JV_LVEC) wah_jit_to_vec(f, i);
            else if (v->kind == WAH_JV_LFPR) wah_jit_to_fpr(f, i);
            else wah_jit_to_gpr(f, i);
        }
    }
    const wah_jit_val_t *v = &f->vals[f->h - 1];
    if (f->pin_of[idx] & WAH_JIT_PIN_VEC) {
        if (v->kind == WAH_JV_SLOT) wah_jit_vldst(f, false, d, WAH_JR_FP, wah_jit_slot_off(f, f->h - 1));
        else if (v->kind != WAH_JV_VEC && v->kind != WAH_JV_LVEC) WAH_JIT_FAIL(f, "not a v128 value");
        else if (v->reg != d) wah_jit_vmov(f, d, v->reg);
        return;
    }
    switch (v->kind) {
        case WAH_JV_GPR: case WAH_JV_LGPR:
            if (fp) wah_jit_gpr_to_fpr(f, d, v->reg);
            else if (v->reg != d) wah_jit_mov64(f, d, v->reg);
            break;
        case WAH_JV_FPR: case WAH_JV_LFPR:
            if (!fp) wah_jit_fpr_to_gpr(f, d, v->reg);
            else if (v->reg != d) wah_jit_fmov(f, d, v->reg);
            break;
        case WAH_JV_CONST:
            if (fp) {
                wah_jit_mov_imm(f, WAH_JR_T0, (uint64_t)v->imm);
                wah_jit_gpr_to_fpr(f, d, WAH_JR_T0);
            } else {
                wah_jit_mov_imm(f, d, (uint64_t)v->imm);
            }
            break;
        case WAH_JV_FLAGS:
            wah_jit_setcc(f, fp ? WAH_JR_T0 : d, v->reg);
            if (fp) wah_jit_gpr_to_fpr(f, d, WAH_JR_T0);
            break;
        case WAH_JV_SLOT: wah_jit_ldst(f, false, fp, true, d, WAH_JR_FP, wah_jit_slot_off(f, f->h - 1)); break;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Runtime helpers called from native code /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static wah_error_t wah_jit_poll(wah_exec_context_t *ctx) { return wah_poll_handler(ctx); }

#if ((WAH_COMPILED_FEATURES) & WAH_FEATURE_BULK_MEMORY)
static wah_error_t wah_jit_bulk_yield(wah_exec_context_t *ctx) {
    WAH_POLL_FLAG_STORE(ctx->interrupt_flag, 0);
    wah_recompute_poll_flag(ctx);
    return WAH_STATUS_YIELDED;
}
#endif

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

// Addresses and sizes of memory operations, which are i64 for memory64 and zero-extended i32 otherwise
static bool wah_jit_ctx_mem64(const wah_exec_context_t *ctx, uint32_t mem_idx) {
    return wah_memory_type(ctx->module, mem_idx)->addr_type == WAH_TYPE_I64;
}
#if ((WAH_COMPILED_FEATURES) & WAH_FEATURE_BULK_MEMORY)
static uint64_t wah_jit_get_addr(const wah_value_t *v, bool mem64) { return mem64 ? (uint64_t)v->i64 : (uint32_t)v->i32; }
static void wah_jit_set_addr(wah_value_t *v, bool mem64, uint64_t a) {
    if (mem64) v->i64 = (int64_t)a;
    else v->i32 = (int32_t)a;
}
#endif

// Returns the old size of the memory in pages or -1 (truncated to i32 if not memory64), growing by *pages.
static uint64_t wah_jit_memory_grow(wah_exec_context_t *ctx, uint32_t mem_idx, const wah_value_t *pages) {
    bool mem64 = wah_jit_ctx_mem64(ctx, mem_idx);
    int64_t delta = mem64 ? pages->i64 : pages->i32;
    uint64_t old_pages;
    if (delta < 0 || !wah_memory_grow_internal(ctx, ctx, mem_idx, (uint64_t)delta, &old_pages)) old_pages = UINT64_MAX;
    return mem64 ? old_pages : (uint32_t)old_pages;
}

#if ((WAH_COMPILED_FEATURES) & WAH_FEATURE_BULK_MEMORY)
// Bulk operations take operands in the value stack, which are updated before yielding in the middle.
static wah_error_t wah_jit_memory_fill(wah_exec_context_t *ctx, uint32_t mem_idx, wah_value_t *ops) {
    if (wah_bulk_should_interrupt(ctx)) return wah_jit_bulk_yield(ctx);
    bool mem64 = wah_jit_ctx_mem64(ctx, mem_idx);
    uint64_t dst = wah_jit_get_addr(&ops[0], mem64), size = wah_jit_get_addr(&ops[2], mem64);
    uint8_t val = (uint8_t)ops[1].i32;
    WAH_ENSURE(wah_u64_range_in_bounds(dst, size, ctx->memories[mem_idx].size), WAH_ERROR_MEMORY_OUT_OF_BOUNDS);
    uint64_t done = wah_memory_fill_internal(ctx, ctx->memories[mem_idx].data, dst, val, size);
    if (done == size) return WAH_OK;
    wah_jit_set_addr(&ops[0], mem64, dst + done);
    wah_jit_set_addr(&ops[2], mem64, size - done);
    return wah_jit_bulk_yield(ctx);
}

static wah_error_t wah_jit_memory_copy(wah_exec_context_t *ctx, uint32_t dst_idx, uint32_t src_idx, wah_value_t *ops) {
    if (wah_bulk_should_interrupt(ctx)) return wah_jit_bulk_yield(ctx);
    bool dst64 = wah_jit_ctx_mem64(ctx, dst_idx), src64 = wah_jit_ctx_mem64(ctx, src_idx);
    uint64_t dst = wah_jit_get_addr(&ops[0], dst64), src = wah_jit_get_addr(&ops[1], src64);
    uint64_t size = wah_jit_get_addr(&ops[2], dst64 && src64);
    const wah_memory_inst_t *dmem = &ctx->memories[dst_idx], *smem = &ctx->memories[src_idx];
    WAH_ENSURE(wah_u64_range_in_bounds(dst, size, dmem->size), WAH_ERROR_MEMORY_OUT_OF_BOUNDS);
    WAH_ENSURE(wah_u64_range_in_bounds(src, size, smem->size), WAH_ERROR_MEMORY_OUT_OF_BOUNDS);
    bool backward = dmem->data == smem->data && dst > src && dst < src + size;
    uint64_t done = wah_memory_copy_internal(ctx, dmem->data, dst, smem->data, src, size, backward);
    if (done == size) return WAH_OK;
    if (!backward) {
        wah_jit_set_addr(&ops[0], dst64, dst + done);
        wah_jit_set_addr(&ops[1], src64, src + done);
    }
    wah_jit_set_addr(&ops[2], dst64 && src64, size - done);
    return wah_jit_bulk_yield(ctx);
}

static wah_error_t wah_jit_memory_init(wah_exec_context_t *ctx, uint32_t data_idx, uint32_t mem_idx, wah_value_t *ops) {
    if (wah_bulk_should_interrupt(ctx)) return wah_jit_bulk_yield(ctx);
    bool mem64 = wah_jit_ctx_mem64(ctx, mem_idx);
    uint64_t dst = wah_jit_get_addr(&ops[0], mem64);
    uint32_t src = (uint32_t)ops[1].i32, size = (uint32_t)ops[2].i32;
    uint32_t seg_len = wah_data_seg_data_len(ctx, data_idx);
    WAH_ENSURE(wah_u64_range_in_bounds(dst, size, ctx->memories[mem_idx].size), WAH_ERROR_MEMORY_OUT_OF_BOUNDS);
    WAH_ENSURE((uint64_t)src + size <= seg_len, WAH_ERROR_TRAP);
    uint32_t done = (uint32_t)wah_memory_init_internal(ctx, ctx->memories[mem_idx].data, dst,
                                                       ctx->module->data_segments[data_idx].data, src, size);
    if (done == size) return WAH_OK;
    wah_jit_set_addr(&ops[0], mem64, dst + done);
    ops[1].i32 = (int32_t)(src + done);
    ops[2].i32 = (int32_t)(size - done);
    return wah_jit_bulk_yield(ctx);
}

static void wah_jit_data_drop(wah_exec_context_t *ctx, uint32_t data_idx) { wah_data_seg_mark_dropped(ctx, data_idx); }
#endif

#define WAH_JIT_CTX_OFF(field) ((uint32_t)offsetof(wah_exec_context_t, field))

#if defined(WAH_JIT_X86_64)

////////////////////////////////////////////////////////////////////////////////
// Code generation (x86-64) ////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Generated code uses the following registers:
//
// - rbx: exec context, rbp: frame base, r12/r13: base and size of memory 0 (reloaded after anything that may grow
//   memories)
// - r15, r14, r9, r8 and xmm13-xmm8: pinned locals in this order, otherwise the operand stack cache
// - rax, rcx, rdx, rsi, rdi and xmm0-xmm7: operand stack cache, r10/r11 and xmm14/xmm15: scratch
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
    f->gpr_local[r] = -1;
    if (i >= 0) {
        int n = -1;
        for (int k = 31; k >= 0 && n < 0; k--) if (f->gpr[k] == WAH_JO_FREE) n = k;
        if (n >= 0) {
            wah_x64_mov(f, true, n, r);
            f->vals[i].reg = (uint8_t)n;
            f->gpr[n] = i;
            f->gpr_local[n] = -1;
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

// Returns a register holding the address of globals, which is loaded into T1.
static int wah_jit_globals(wah_jit_fn_t *f) {
    wah_jit_ldst(f, false, false, true, WAH_JR_T1, WAH_JR_CTX, WAH_JIT_CTX_OFF(globals));
    return WAH_JR_T1;
}

static void wah_jit_memory_size(wah_jit_fn_t *f, uint32_t mem_idx) {
    int d = wah_jit_alloc(f, false);
    if (mem_idx != 0) {
        wah_jit_ldst(f, false, false, true, d, WAH_JR_CTX, WAH_JIT_CTX_OFF(memories));
        wah_jit_ldst(f, false, false, true, d, d, (uint64_t)mem_idx * sizeof(wah_memory_inst_t) +
                                                  offsetof(wah_memory_inst_t, size));
    } else if (f->jm->guarded) {
        wah_jit_ldst(f, false, false, true, d, WAH_JR_CTX, WAH_JIT_CTX_OFF(memory_size));
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
    int b = -1;
    if (wah_x64_top_imm32(f, sf, &imm)) {
        f->h--;
        a = wah_jit_pop_gpr_ro(f);
        wah_x64_alu_imm(f, 7, w, a, imm);
    } else {
        b = wah_jit_pop_gpr_ro(f);
        a = wah_jit_pop_gpr_ro(f);
        wah_x64_rr(f, WAH_X64_CMP, w, a, b);
    }
    if (fuse) {
        wah_jit_free_gpr(f, a);
        if (b >= 0) wah_jit_free_gpr(f, b);
        wah_jit_push(f, (wah_jit_val_t){ .kind = WAH_JV_FLAGS, .reg = (uint8_t)cond });
    } else {
        int d = wah_jit_dest(f, false, a, b); // MOV doesn't change flags
        wah_jit_setcc(f, d, cond);
        wah_jit_finish_gpr(f, d, a, b);
    }
}

// Unordered operands set ZF, PF and CF, so only ordered conditions can be flags.
static void wah_jit_fcmp(wah_jit_fn_t *f, int type, int kind, bool fuse) {
    int b = wah_jit_pop_fpr_ro(f), a = wah_jit_pop_fpr_ro(f);
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
    int d = f->hint;
    if (d >= 0) { // d = a op b, where a or b may be d
        bool is_imm = wah_x64_top_imm32(f, sf, &imm);
        if (is_imm) f->h--;
        int b = is_imm ? -1 : wah_jit_pop_gpr_ro(f), a = wah_jit_pop_gpr_ro(f), r = d;
        if (b == d && a != d) {
            if (kind == WAH_JI_SUB) r = WAH_JR_T0;
            else b = a, a = d;
        }
        if (is_imm && kind == WAH_JI_MUL) {
            wah_x64_rr(f, 0x69, w, r, a); // IMUL r, r/m, imm32
            wah_x64_imm32(f, (uint32_t)imm);
        } else {
            if (a != r) wah_jit_mov64(f, r, a);
            if (is_imm) wah_x64_alu_imm(f, digits[kind], w, r, imm);
            else wah_x64_rr(f, ops[kind], w, r, b);
        }
        if (r != d) wah_jit_mov64(f, d, r);
        wah_jit_free_gpr(f, a);
        if (b >= 0) wah_jit_free_gpr(f, b);
        wah_jit_push_gpr(f, d);
        return;
    }
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
    int b = wah_jit_pop_gpr_ro(f), a = wah_jit_pop_gpr(f);
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
    int b = wah_jit_pop_gpr_ro(f), a = wah_jit_pop_gpr(f);
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
    int b = wah_jit_pop_gpr_ro(f), a = wah_jit_pop_gpr(f);
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
    int b = wah_jit_pop_fpr_ro(f), a = wah_jit_pop_fpr(f);
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
    wah_jit_finish_fpr(f, a, a, b);
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
    if (!is_signed) return; // i64.extend_i32_u, which i32 values already are
    int a = wah_jit_pop_gpr(f);
    if (bits == 32) wah_x64_rr(f, WAH_X64_MOVSXD, WAH_X64_W, a, a);
    else wah_x64_rr(f, bits == 8 ? WAH_X64_MOVSX8 : WAH_X64_MOVSX16, (sf ? WAH_X64_W : 0) | WAH_X64_B8, a, a);
    wah_jit_push_gpr(f, a);
}

// Zero-extends the i32 value on the top of the stack, whose upper half may be anything.
static void wah_jit_zext_top(wah_jit_fn_t *f) {
    wah_jit_val_t *v = &f->vals[f->h - 1];
    if (v->kind == WAH_JV_CONST) {
        v->imm = (int64_t)(uint32_t)v->imm;
        return;
    }
    int a = wah_jit_pop_gpr_ro(f), d = wah_jit_dest(f, false, a, -1);
    wah_x64_mov(f, false, d, a);
    wah_jit_finish_gpr(f, d, a, -1);
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

#if ((WAH_COMPILED_FEATURES) & WAH_FEATURE_NONTRAPPING_F2I)
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
#endif

static void wah_jit_convert(wah_jit_fn_t *f, int type, bool sf, bool is_signed) {
    int a = wah_jit_pop_gpr(f), d = wah_jit_alloc(f, true);
    uint32_t op = WAH_X64_FP(type) | WAH_X64_CVTSI2S;
    wah_x64_rr(f, WAH_X64_XORPS, 0, d, d); // Breaks the dependency on d
    if (is_signed) {
        wah_x64_rr(f, op, sf ? WAH_X64_W : 0, d, a);
    } else if (!sf) { // Zero-extended
        wah_x64_rr(f, op, WAH_X64_W, d, a);
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

// Bounds-checks the address on the stack, and returns the base with the address in *index to be added with *disp.
// The index register is already freed, so it may be the destination of the access as long as it is read first.
static int wah_jit_address(wah_jit_fn_t *f, const wah_memarg_t *m, uint32_t size, int *size_reg, int *index,
                           int32_t *disp) {
    int a = wah_jit_pop_gpr_ro(f); // Zero-extended unless 64-bit
    int base = WAH_JR_MEM, limit = WAH_JR_MEMSZ;
    bool mem64 = wah_jit_mem64(f, m->memidx);
    uint64_t end = wah_jit_access_end(m, size); // Fits in 33 bits unless 64-bit, so a + end doesn't overflow then
    bool check = mem64 || !f->jm->guarded || end > WAH_JIT_GUARD_SIZE;
    *size_reg = -1;
    if (m->memidx != 0 && (base = wah_jit_cached_base(f, m->memidx)) >= 0) {
        if (check) {
            limit = wah_jit_alloc(f, false);
            *size_reg = limit;
            wah_jit_ldst(f, false, false, true, limit, WAH_JR_CTX, WAH_JIT_CTX_OFF(memories));
            wah_jit_ldst(f, false, false, true, limit, limit, (uint64_t)m->memidx * sizeof(wah_memory_inst_t) +
                                                              offsetof(wah_memory_inst_t, size));
        }
    } else if (m->memidx != 0) {
        base = wah_jit_alloc(f, false);
        uint64_t off = (uint64_t)m->memidx * sizeof(wah_memory_inst_t);
        wah_jit_ldst(f, false, false, true, base, WAH_JR_CTX, WAH_JIT_CTX_OFF(memories));
        if (check) {
            limit = wah_jit_alloc(f, false);
            *size_reg = limit;
            wah_jit_ldst(f, false, false, true, limit, base, off + offsetof(wah_memory_inst_t, size));
        }
        wah_jit_ldst(f, false, false, true, base, base, off + offsetof(wah_memory_inst_t, data));
        if (f->jm->guarded && !mem64) f->gpr_local[base] = (int32_t)(f->nlocals + m->memidx);
    } else if (check && f->jm->guarded) { // The size register may be pinned
        limit = wah_jit_alloc(f, false);
        *size_reg = limit;
        wah_jit_ldst(f, false, false, true, limit, WAH_JR_CTX, WAH_JIT_CTX_OFF(memory_size));
    }
    if (mem64) { // a + end may wrap around, which sets the carry
        if (end <= INT32_MAX) {
            wah_x64_mov(f, true, WAH_JR_T1, a);
            wah_x64_alu_imm(f, 0, WAH_X64_W, WAH_JR_T1, (int32_t)end); // ADD
        } else {
            wah_jit_mov_imm(f, WAH_JR_T1, end);
            wah_x64_rr(f, WAH_X64_ADD, WAH_X64_W, WAH_JR_T1, a);
        }
        wah_jit_bcond(f, WAH_X64_B, f->l_oob);
        wah_x64_rr(f, WAH_X64_CMP, WAH_X64_W, WAH_JR_T1, limit);
        wah_jit_bcond(f, WAH_X64_A, f->l_oob);
        check = false;
    }
    *index = a;
    if (end <= INT32_MAX) {
        *disp = (int32_t)m->offset;
    } else {
        wah_jit_mov_imm(f, WAH_JR_T0, m->offset);
        wah_x64_rr(f, WAH_X64_ADD, WAH_X64_W, WAH_JR_T0, a);
        *index = WAH_JR_T0;
        *disp = 0;
        end = size;
    }
    wah_jit_free_gpr(f, a);
    if (check) {
        wah_x64_mem(f, WAH_X64_LEA, WAH_X64_W, WAH_JR_T1, *index, -1, 0, (int32_t)end);
        wah_x64_rr(f, WAH_X64_CMP, WAH_X64_W, WAH_JR_T1, limit);
        wah_jit_bcond(f, WAH_X64_A, f->l_oob);
    }
    return base;
}

static void wah_jit_load(wah_jit_fn_t *f, const wah_memarg_t *m, uint32_t size_log2, bool is_signed, bool sf, bool fp) {
    int size_reg, index;
    int32_t disp;
    int base = wah_jit_address(f, m, 1u << size_log2, &size_reg, &index, &disp);
    if (size_reg >= 0) wah_jit_free_gpr(f, size_reg);
    if (base != WAH_JR_MEM) wah_jit_free_gpr(f, base);
    int d = !fp && f->hint >= 0 ? f->hint : wah_jit_alloc(f, fp); // May be the index, which the load reads first
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
    wah_x64_mem(f, op, flags, d, base, index, 0, disp);
    if (fp) wah_jit_push_fpr(f, d);
    else wah_jit_push_gpr(f, d);
}

static void wah_jit_store(wah_jit_fn_t *f, const wah_memarg_t *m, uint32_t size_log2, bool fp_ok) {
    wah_jit_val_t *v = &f->vals[f->h - 1];
    int r = -1;
    bool fp = false;
    int64_t imm = v->imm;
    if ((v->kind == WAH_JV_FPR || v->kind == WAH_JV_LFPR) && fp_ok) {
        r = wah_jit_pop_fpr_ro(f);
        fp = true;
    } else if (v->kind == WAH_JV_CONST && (size_log2 < 3 || imm == (int32_t)imm)) {
        f->h--;
    } else {
        r = wah_jit_pop_gpr_ro(f);
    }
    int size_reg, index;
    int32_t disp;
    int base = wah_jit_address(f, m, 1u << size_log2, &size_reg, &index, &disp);
    if (fp) {
        wah_x64_mem(f, WAH_X64_FP(size_log2 == 3) | WAH_X64_MOVS_STORE, 0, r, base, index, 0, disp);
    } else if (r < 0) {
        static const uint32_t ops[] = { 0xC6u, 0x660000C7u, WAH_X64_MOV_IMM, WAH_X64_MOV_IMM };
        wah_x64_mem(f, ops[size_log2], size_log2 == 3 ? WAH_X64_W : 0, 0, base, index, 0, disp);
        uint32_t u = (uint32_t)imm;
        wah_jit_emit(f, &u, size_log2 == 0 ? 1 : size_log2 == 1 ? 2 : 4);
    } else {
        static const uint32_t ops[] = { WAH_X64_MOV_STORE8, 0x66000089u, WAH_X64_MOV_STORE, WAH_X64_MOV_STORE };
        wah_x64_mem(f, ops[size_log2], size_log2 == 3 ? WAH_X64_W : size_log2 == 0 ? WAH_X64_B8 : 0, r, base, index, 0, disp);
    }
    if (size_reg >= 0) wah_jit_free_gpr(f, size_reg);
    if (base != WAH_JR_MEM) wah_jit_free_gpr(f, base);
    if (fp) wah_jit_free_fpr(f, r);
    else if (r >= 0) wah_jit_free_gpr(f, r);
}

static void wah_jit_select(wah_jit_fn_t *f) {
    wah_jit_cond_t c = wah_jit_pop_cond(f);
    bool fp = wah_jit_is_fpr(f->vals[f->h - 1].kind) || wah_jit_is_fpr(f->vals[f->h - 2].kind);
    int b, a;
    if (fp) { b = wah_jit_pop_fpr_ro(f); a = wah_jit_pop_fpr(f); }
    else { b = wah_jit_pop_gpr_ro(f); a = wah_jit_pop_gpr(f); }
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
    uint32_t resume = s->resume;
    if (s->poll) { // Pinned locals are kept in slots while suspended, and restored when resumed or continued
        uint32_t suspend = wah_jit_new_label(f);
        wah_jit_sync_pins(f, true);
        wah_jit_call_helper(f, (uintptr_t)wah_jit_poll, (const wah_jit_arg_t[]){ { WAH_JA_CTX, 0 } }, 1, false);
        wah_jit_cbz(f, true, false, WAH_JR_RET, suspend);
        wah_jit_sync_pins(f, false);
        wah_jit_b(f, s->cont);
        if (f->npins > 0) {
            resume = wah_jit_new_label(f);
            wah_jit_bind(f, resume);
            wah_jit_sync_pins(f, false);
            wah_jit_b(f, s->resume);
        }
        wah_jit_bind(f, suspend);
    }
    // eax is a status to suspend with, or an error
    wah_x64_rr(f, WAH_X64_TEST, 0, WAH_JR_RET, WAH_JR_RET);
    wah_jit_bcond(f, WAH_X64_S, f->l_err);
    wah_jit_ldst(f, false, false, true, WAH_JR_T0, WAH_JR_CTX, WAH_JIT_CTX_OFF(jit));
    wah_jit_adr(f, WAH_JR_T1, resume);
    wah_jit_ldst(f, true, false, true, WAH_JR_T1, WAH_JR_T0, offsetof(wah_jit_ctx_t, resume_pc));
    wah_jit_ldst(f, true, false, true, WAH_JR_FP, WAH_JR_T0, offsetof(wah_jit_ctx_t, resume_fp));
    wah_jit_exit(f);
}

// Checks the stack and saves the return address. Declared locals are zeroed by the caller.
static void wah_jit_prologue(wah_jit_fn_t *f) {
    wah_x64_byte(f, 0x41); // POP r11
    wah_x64_byte(f, 0x58u + (WAH_JR_T0 & 7));
    wah_x64_mem(f, WAH_X64_LEA, WAH_X64_W, WAH_JR_T1, WAH_JR_FP, -1, 0, (int32_t)f->frame_bytes);
    wah_x64_mem(f, WAH_X64_CMP, WAH_X64_W, WAH_JR_T1, WAH_JR_CTX, -1, 0, (int32_t)WAH_JIT_CTX_OFF(frame_ptr)); // The limit
    wah_jit_bcond(f, WAH_X64_A, f->l_stack);
    wah_jit_ldst(f, true, false, true, WAH_JR_T0, WAH_JR_FP, f->link_off);
}

// Zeroes n slots from [fp + off].
static void wah_jit_zero_loop(wah_jit_fn_t *f, uint32_t off, uint32_t n) {
    uint32_t loop = wah_jit_new_label(f);
    wah_x64_mem(f, WAH_X64_LEA, WAH_X64_W, WAH_JR_T0, WAH_JR_FP, -1, 0, (int32_t)off);
    wah_jit_mov_imm(f, WAH_JR_T1, n);
    wah_x64_rr(f, WAH_X64_XORPS, 0, WAH_JR_VT0, WAH_JR_VT0);
    wah_jit_bind(f, loop);
    wah_jit_vldst(f, true, WAH_JR_VT0, WAH_JR_T0, 0);
    wah_x64_alu_imm(f, 0, WAH_X64_W, WAH_JR_T0, (int32_t)sizeof(wah_value_t));
    wah_x64_alu_imm(f, 5, 0, WAH_JR_T1, 1);
    wah_jit_bcond(f, WAH_X64_NE, loop);
}


#if ((WAH_COMPILED_FEATURES) & WAH_FEATURE_SIMD)

// SIMD (x86-64) ////////////////////////////////////////////////////////////////

// SSE instructions up to SSE4.2 on xmm registers, which are destructive: the reg field is both the destination and
// the first operand, unless noted. Packed FP instructions take WAH_X64_PFP(type) as a prefix.
#define WAH_X64_PFP(type) ((type) ? 0x66000000u : 0u)
#define WAH_X64_PUNPCKLBW  0x66000F60u
#define WAH_X64_PUNPCKLWD  0x66000F61u
#define WAH_X64_PACKSSWB   0x66000F63u
#define WAH_X64_PCMPGTB    0x66000F64u // Plus 0, 1, 2 for the lane size, also for PCMPEQ*
#define WAH_X64_PACKUSWB   0x66000F67u
#define WAH_X64_PUNPCKHBW  0x66000F68u
#define WAH_X64_PUNPCKHWD  0x66000F69u
#define WAH_X64_PACKSSDW   0x66000F6Bu
#define WAH_X64_PUNPCKLQDQ 0x66000F6Cu
#define WAH_X64_MOVD_TO_XMM 0x66000F6Eu   // REX.W for MOVQ
#define WAH_X64_PSHUFD     0x66000F70u   // d = shuffle(rm), with imm8
#define WAH_X64_PSHUFLW    0xF2000F70u
#define WAH_X64_PSHIFT_IMM 0x66000F71u   // Plus 0, 1, 2 for the lane size; psrl 2, psra 4, psll 6, with imm8
#define WAH_X64_PCMPEQB    0x66000F74u
#define WAH_X64_MOVD_FROM_XMM 0x66000F7Eu // The xmm register in the reg field, REX.W for MOVQ
#define WAH_X64_PINSRW     0x66000FC4u   // With imm8
#define WAH_X64_PEXTRW     0x66000FC5u   // To the general register in the reg field, with imm8
#define WAH_X64_PSRLW      0x66000FD1u   // Plus 0, 1, 2 for the lane size, by the count in xmm
#define WAH_X64_PADDQ      0x66000FD4u
#define WAH_X64_PMULLW     0x66000FD5u
#define WAH_X64_PMOVMSKB   0x66000FD7u   // To the general register in the reg field
#define WAH_X64_PSUBUSB    0x66000FD8u
#define WAH_X64_PSUBUSW    0x66000FD9u
#define WAH_X64_PMINUB     0x66000FDAu
#define WAH_X64_PAND       0x66000FDBu
#define WAH_X64_PADDUSB    0x66000FDCu
#define WAH_X64_PADDUSW    0x66000FDDu
#define WAH_X64_PMAXUB     0x66000FDEu
#define WAH_X64_PANDN      0x66000FDFu
#define WAH_X64_PAVGB      0x66000FE0u
#define WAH_X64_PSRAW      0x66000FE1u
#define WAH_X64_PAVGW      0x66000FE3u
#define WAH_X64_PMULHUW    0x66000FE4u
#define WAH_X64_PMULHW     0x66000FE5u
#define WAH_X64_CVTTPD2DQ  0x66000FE6u
#define WAH_X64_PSUBSB     0x66000FE8u
#define WAH_X64_PSUBSW     0x66000FE9u
#define WAH_X64_PMINSW     0x66000FEAu
#define WAH_X64_POR        0x66000FEBu
#define WAH_X64_PADDSB     0x66000FECu
#define WAH_X64_PADDSW     0x66000FEDu
#define WAH_X64_PMAXSW     0x66000FEEu
#define WAH_X64_PXOR       0x66000FEFu
#define WAH_X64_PSLLW      0x66000FF1u
#define WAH_X64_PMULUDQ    0x66000FF4u
#define WAH_X64_PMADDWD    0x66000FF5u
#define WAH_X64_PSUBB      0x66000FF8u // Plus 0, 1, 2, 3 for the lane size, also for PADD* except PADDQ
#define WAH_X64_PADDB      0x66000FFCu
#define WAH_X64_PSHUFB     0x660F3800u
#define WAH_X64_PHADDD     0x660F3802u
#define WAH_X64_PMADDUBSW  0x660F3804u // Unsigned bytes of the reg operand, signed bytes of rm
#define WAH_X64_PMULHRSW   0x660F380Bu
#define WAH_X64_PTEST      0x660F3817u
#define WAH_X64_PABSB      0x660F381Cu // Plus 0, 1, 2 for the lane size
#define WAH_X64_PMOVSXBW   0x660F3820u // d = extend(rm), also PMOVSXWD (23), PMOVSXDQ (25) and PMOVZX* (+0x10)
#define WAH_X64_PMULDQ     0x660F3828u
#define WAH_X64_PCMPEQQ    0x660F3829u
#define WAH_X64_PACKUSDW   0x660F382Bu
#define WAH_X64_PCMPGTQ    0x660F3837u
#define WAH_X64_PMINSB     0x660F3838u
#define WAH_X64_PMINSD     0x660F3839u
#define WAH_X64_PMINUW     0x660F383Au
#define WAH_X64_PMINUD     0x660F383Bu
#define WAH_X64_PMAXSB     0x660F383Cu
#define WAH_X64_PMAXSD     0x660F383Du
#define WAH_X64_PMAXUW     0x660F383Eu
#define WAH_X64_PMAXUD     0x660F383Fu
#define WAH_X64_PMULLD     0x660F3840u
#define WAH_X64_ROUNDPS    0x660F3A08u // Plus 1 for doubles, with imm8
#define WAH_X64_PBLENDW    0x660F3A0Eu
#define WAH_X64_PEXTRB     0x660F3A14u // From the xmm register in the reg field, with imm8; PEXTRW (15) to memory,
                                       // PEXTRD and PEXTRQ (16, REX.W)
#define WAH_X64_PINSRB     0x660F3A20u // With imm8; PINSRD and PINSRQ (22, REX.W)
#define WAH_X64_INSERTPS   0x660F3A21u
#define WAH_X64_UNPCKLPS   0x0F14u     // 66 for UNPCKLPD
#define WAH_X64_MOVMSKPS   0x0F50u     // To the general register in the reg field, 66 for MOVMSKPD
#define WAH_X64_CVTPS2PD   0x0F5Au     // 66 for CVTPD2PS
#define WAH_X64_CVTDQ2PS   0x0F5Bu
#define WAH_X64_CVTTPS2DQ  0xF3000F5Bu
#define WAH_X64_CVTDQ2PD   0xF3000FE6u
#define WAH_X64_CMPPS      0x0FC2u     // With an imm8 predicate: eq 0, lt 1, le 2, unord 3, neq 4
#define WAH_X64_SHUFPS     0x0FC6u
#define WAH_X64_MOVQ_LOAD  0xF3000F7Eu // Zeroes the upper half

// Uses of WAH_X64_PCMPGTB and so on with the lane size
#define WAH_X64_LANE(op, size_log2) ((op) + (uint32_t)(size_log2))

static bool wah_jit_simd_supported(const wah_jit_module_t *jm) { return jm->sse42; }

static void wah_x64_rr_imm(wah_jit_fn_t *f, uint32_t op, uint32_t flags, int reg, int rm, uint32_t imm) {
    wah_x64_rr(f, op, flags, reg, rm);
    wah_x64_byte(f, imm);
}

// op d, [rip + constant], for instructions without an immediate
static void wah_x64_vconst_op(wah_jit_fn_t *f, uint32_t op, int d, const uint8_t *bytes) {
    uint32_t label = wah_jit_vconst_label(f, bytes);
    wah_x64_mem(f, op, 0, d, WAH_X64_RIP, -1, 0, 0);
    if (!f->fail) wah_jit_add_patch(f, WAH_JP_X64_REL32, label, 0, wah_jit_pos(f) - 4);
}

static void wah_x64_vsplat_op(wah_jit_fn_t *f, uint32_t op, int d, uint64_t v, uint32_t size_log2) {
    wah_jit_v16_t c = wah_jit_splat_const(v, size_log2);
    wah_x64_vconst_op(f, op, d, c.b);
}

static void wah_jit_load_vconst(wah_jit_fn_t *f, int d, const uint8_t *bytes) {
    static const uint8_t zeroes[16] = { 0 }, ones[16] = { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 };
    if (memcmp(bytes, zeroes, 16) == 0) wah_x64_rr(f, WAH_X64_PXOR, 0, d, d);
    else if (memcmp(bytes, ones, 16) == 0) wah_x64_rr(f, WAH_X64_LANE(WAH_X64_PCMPEQB, 2), 0, d, d);
    else wah_x64_vconst_op(f, WAH_X64_MOVAPS_LOAD, d, bytes); // The constants are aligned
}

static void wah_x64_vnot(wah_jit_fn_t *f, int d) {
    wah_x64_rr(f, WAH_X64_LANE(WAH_X64_PCMPEQB, 2), 0, WAH_JR_VT1, WAH_JR_VT1);
    wah_x64_rr(f, WAH_X64_PXOR, 0, d, WAH_JR_VT1);
}

// A destination register initialized with a, which is reused if it is a temporary.
static int wah_x64_vcopy(wah_jit_fn_t *f, int a) {
    if (f->fpr[a] == WAH_JO_TEMP) return a;
    int d = wah_jit_alloc(f, true);
    wah_jit_vmov(f, d, a);
    return d;
}

static void wah_jit_vselect(wah_jit_fn_t *f) {
    wah_jit_cond_t c = wah_jit_pop_cond(f);
    int b = wah_jit_pop_vec_ro(f), a = wah_jit_pop_vec(f);
    int cond = c.cond;
    if (!c.flags) {
        wah_x64_rr(f, WAH_X64_TEST, 0, c.reg, c.reg);
        cond = WAH_X64_NE;
    }
    uint32_t skip = wah_x64_jcc8(f, cond);
    wah_jit_vmov(f, a, b);
    wah_x64_bind8(f, skip);
    wah_jit_release_cond(f, c);
    wah_jit_finish_vec(f, a, a, b, -1);
}

// Replaces NaN lanes of d with the canonical NaN.
static void wah_x64_vcanonicalize(wah_jit_fn_t *f, int type, int d) {
    wah_jit_vmov(f, WAH_JR_VT0, d);
    wah_x64_rr_imm(f, WAH_X64_PFP(type) | WAH_X64_CMPPS, 0, WAH_JR_VT0, WAH_JR_VT0, 3);
    wah_x64_rr(f, WAH_X64_PTEST, 0, WAH_JR_VT0, WAH_JR_VT0);
    uint32_t skip = wah_x64_jcc8(f, WAH_X64_E);
    wah_jit_vmov(f, WAH_JR_VT1, WAH_JR_VT0);
    wah_x64_rr(f, WAH_X64_ANDNPS, 0, WAH_JR_VT1, d);
    wah_x64_vsplat_op(f, WAH_X64_ANDPS, WAH_JR_VT0, WAH_X64_CANONICAL_NAN(type), 2 + (uint32_t)type);
    wah_x64_rr(f, WAH_X64_ORPS, 0, WAH_JR_VT0, WAH_JR_VT1);
    wah_jit_vmov(f, d, WAH_JR_VT0);
    wah_x64_bind8(f, skip);
}

// Binary operation: d = a op b, or b op a if swapped, with an optional imm8 (>= 0), then negated or canonicalized
// as an FP type (>= 0) if given.
static void wah_x64_vbin(wah_jit_fn_t *f, uint32_t op, bool swap, bool commutative, int imm, bool not, int canon) {
    int b = wah_jit_pop_vec_ro(f), a = wah_jit_pop_vec_ro(f);
    if (swap) { int t = a; a = b; b = t; }
    int d, src = b;
    if (f->fpr[a] == WAH_JO_TEMP) d = a;
    else if (commutative && f->fpr[b] == WAH_JO_TEMP) { d = b; src = a; }
    else wah_jit_vmov(f, d = wah_jit_alloc(f, true), a);
    wah_x64_rr(f, op, 0, d, src);
    if (imm >= 0) wah_x64_byte(f, (uint32_t)imm);
    if (not) wah_x64_vnot(f, d);
    if (canon >= 0) wah_x64_vcanonicalize(f, canon, d);
    wah_jit_finish_vec(f, d, a, b, -1);
}
#define WAH_X64_VBIN(f, op) wah_x64_vbin(f, op, false, false, -1, false, -1)
#define WAH_X64_VBIN_C(f, op) wah_x64_vbin(f, op, false, true, -1, false, -1)

// Unary operation d = op(a) writing d without reading it, with an optional imm8
static void wah_x64_vun(wah_jit_fn_t *f, uint32_t op, int imm, int canon) {
    int a = wah_jit_pop_vec_ro(f), d = wah_jit_dest(f, true, a, -1);
    wah_x64_rr(f, op, 0, d, a);
    if (imm >= 0) wah_x64_byte(f, (uint32_t)imm);
    if (canon >= 0) wah_x64_vcanonicalize(f, canon, d);
    wah_jit_finish_vec(f, d, a, -1, -1);
}

// d = a op constant, like a bitwise operation with a splatted constant
static void wah_x64_vconst_bin(wah_jit_fn_t *f, uint32_t op, uint64_t v, uint32_t size_log2) {
    int a = wah_jit_pop_vec_ro(f), d = wah_x64_vcopy(f, a);
    wah_x64_vsplat_op(f, op, d, v, size_log2);
    wah_jit_finish_vec(f, d, a, -1, -1);
}

// Unsigned comparisons: a >= b iff max(a, b) == a, and a <= b iff min(a, b) == a
static void wah_x64_vcmp_u(wah_jit_fn_t *f, uint32_t size_log2, bool ge, bool not) {
    static const uint32_t maxs[] = { WAH_X64_PMAXUB, WAH_X64_PMAXUW, WAH_X64_PMAXUD };
    static const uint32_t mins[] = { WAH_X64_PMINUB, WAH_X64_PMINUW, WAH_X64_PMINUD };
    int b = wah_jit_pop_vec_ro(f), a = wah_jit_pop_vec_ro(f);
    wah_jit_vmov(f, WAH_JR_VT0, a);
    wah_x64_rr(f, (ge ? maxs : mins)[size_log2], 0, WAH_JR_VT0, b);
    wah_x64_rr(f, WAH_X64_LANE(WAH_X64_PCMPEQB, size_log2), 0, WAH_JR_VT0, a);
    if (not) wah_x64_vnot(f, WAH_JR_VT0);
    int d = wah_jit_dest(f, true, a, b);
    wah_jit_vmov(f, d, WAH_JR_VT0);
    wah_jit_finish_vec(f, d, a, b, -1);
}

static void wah_x64_vneg(wah_jit_fn_t *f, uint32_t size_log2) {
    int a = wah_jit_pop_vec_ro(f);
    wah_x64_rr(f, WAH_X64_PXOR, 0, WAH_JR_VT0, WAH_JR_VT0);
    wah_x64_rr(f, WAH_X64_LANE(WAH_X64_PSUBB, size_log2), 0, WAH_JR_VT0, a);
    int d = wah_jit_dest(f, true, a, -1);
    wah_jit_vmov(f, d, WAH_JR_VT0);
    wah_jit_finish_vec(f, d, a, -1, -1);
}

// FP min and max, which are NaN if either operand is, and order zeroes by sign
static void wah_x64_vminmax(wah_jit_fn_t *f, int type, bool max) {
    uint32_t op = WAH_X64_PFP(type) | (max ? WAH_X64_MAXS : WAH_X64_MINS);
    int b = wah_jit_pop_vec_ro(f), a = wah_jit_pop_vec_ro(f);
    wah_jit_vmov(f, WAH_JR_VT1, a);
    wah_x64_rr_imm(f, WAH_X64_PFP(type) | WAH_X64_CMPPS, 0, WAH_JR_VT1, b, 3); // Unordered
    wah_jit_vmov(f, WAH_JR_VT0, b);
    wah_x64_rr(f, op, 0, WAH_JR_VT0, a); // MINPS returns the second operand if either is NaN or both are zero
    int d = wah_x64_vcopy(f, a);
    wah_x64_rr(f, op, 0, d, b);
    wah_x64_rr(f, max ? WAH_X64_ANDPS : WAH_X64_ORPS, 0, d, WAH_JR_VT0);
    wah_x64_rr(f, WAH_X64_PTEST, 0, WAH_JR_VT1, WAH_JR_VT1);
    uint32_t skip = wah_x64_jcc8(f, WAH_X64_E);
    wah_jit_vmov(f, WAH_JR_VT0, WAH_JR_VT1);
    wah_x64_rr(f, WAH_X64_ANDNPS, 0, WAH_JR_VT0, d);
    wah_x64_vsplat_op(f, WAH_X64_ANDPS, WAH_JR_VT1, WAH_X64_CANONICAL_NAN(type), 2 + (uint32_t)type);
    wah_x64_rr(f, WAH_X64_ORPS, 0, WAH_JR_VT1, WAH_JR_VT0);
    wah_jit_vmov(f, d, WAH_JR_VT1);
    wah_x64_bind8(f, skip);
    wah_jit_finish_vec(f, d, a, b, -1);
}

// Shifts by the i32 on top of the stack, modulo the lane size; kind is WAH_JS_SHL, WAH_JS_SHR_S or WAH_JS_SHR_U.
static void wah_x64_vshift(wah_jit_fn_t *f, uint32_t size_log2, int kind) {
    uint32_t bits = 8u << size_log2;
    int digit = kind == WAH_JS_SHL ? 6 : kind == WAH_JS_SHR_S ? 4 : 2;
    uint32_t by_xmm = kind == WAH_JS_SHL ? WAH_X64_PSLLW : kind == WAH_JS_SHR_S ? WAH_X64_PSRAW : WAH_X64_PSRLW;
    const wah_jit_val_t *top = &f->vals[f->h - 1];
    bool is_const = top->kind == WAH_JV_CONST;
    uint32_t s = (uint32_t)top->imm & (bits - 1);
    int n = -1;
    if (is_const) f->h--;
    else n = wah_jit_pop_gpr_ro(f);
    int a = wah_jit_pop_vec_ro(f);
    if (is_const && s == 0) {
        int d = wah_jit_dest(f, true, a, -1);
        if (d != a) wah_jit_vmov(f, d, a);
        wah_jit_finish_vec(f, d, a, -1, -1);
        return;
    }
    if (!is_const) { // VT1 = count, T0 = count & (bits - 1) for 8-bit lanes
        wah_x64_mov(f, false, WAH_JR_T0, n);
        wah_x64_alu_imm(f, 4, 0, WAH_JR_T0, (int32_t)bits - 1);
        wah_x64_rr(f, WAH_X64_MOVD_TO_XMM, 0, WAH_JR_VT1, WAH_JR_T0);
        wah_jit_free_gpr(f, n);
    }
    int d;
    if (size_log2 == 0 && kind == WAH_JS_SHR_S) { // Bytes in upper halves of 16-bit lanes are shifted by 8 more
        wah_jit_vmov(f, WAH_JR_VT0, a);
        wah_x64_rr(f, WAH_X64_PUNPCKHBW, 0, WAH_JR_VT0, a);
        d = wah_x64_vcopy(f, a);
        wah_x64_rr(f, WAH_X64_PUNPCKLBW, 0, d, d);
        if (is_const) {
            wah_x64_rr_imm(f, WAH_X64_PSHIFT_IMM, 0, 4, WAH_JR_VT0, 8 + s);
            wah_x64_rr_imm(f, WAH_X64_PSHIFT_IMM, 0, 4, d, 8 + s);
        } else {
            wah_x64_alu_imm(f, 0, 0, WAH_JR_T0, 8);
            wah_x64_rr(f, WAH_X64_MOVD_TO_XMM, 0, WAH_JR_VT1, WAH_JR_T0);
            wah_x64_rr(f, WAH_X64_PSRAW, 0, WAH_JR_VT0, WAH_JR_VT1);
            wah_x64_rr(f, WAH_X64_PSRAW, 0, d, WAH_JR_VT1);
        }
        wah_x64_rr(f, WAH_X64_PACKSSWB, 0, d, WAH_JR_VT0);
    } else if (size_log2 == 0) { // 16-bit shifts, masking bits moved across bytes
        d = wah_x64_vcopy(f, a);
        if (is_const) {
            wah_x64_rr_imm(f, WAH_X64_PSHIFT_IMM, 0, digit, d, s);
            wah_x64_vsplat_op(f, WAH_X64_PAND, d, kind == WAH_JS_SHL ? (0xffu << s) & 0xff : 0xffu >> s, 0);
        } else { // m = 0xff >> count in all bytes, which masks bytes before shifting left or after shifting right
            int m = wah_jit_alloc(f, true);
            wah_x64_alu_imm(f, 0, 0, WAH_JR_T0, 8);
            wah_x64_rr(f, WAH_X64_MOVD_TO_XMM, 0, WAH_JR_VT0, WAH_JR_T0);
            wah_x64_rr(f, WAH_X64_LANE(WAH_X64_PCMPEQB, 2), 0, m, m);
            wah_x64_rr(f, WAH_X64_PSRLW, 0, m, WAH_JR_VT0);
            wah_x64_rr(f, WAH_X64_PACKUSWB, 0, m, m);
            if (kind == WAH_JS_SHL) wah_x64_rr(f, WAH_X64_PAND, 0, d, m);
            wah_x64_rr(f, by_xmm, 0, d, WAH_JR_VT1);
            if (kind != WAH_JS_SHL) wah_x64_rr(f, WAH_X64_PAND, 0, d, m);
            wah_jit_free_fpr(f, m);
        }
    } else if (size_log2 == 3 && kind == WAH_JS_SHR_S) { // ((a >> s) ^ m) - m with m = sign bit >> s
        wah_x64_vsplat_op(f, WAH_X64_MOVAPS_LOAD, WAH_JR_VT0, 0x8000000000000000ull, 3);
        d = wah_x64_vcopy(f, a);
        if (is_const) {
            wah_x64_rr_imm(f, WAH_X64_PSHIFT_IMM + 2, 0, 2, WAH_JR_VT0, s);
            wah_x64_rr_imm(f, WAH_X64_PSHIFT_IMM + 2, 0, 2, d, s);
        } else {
            wah_x64_rr(f, WAH_X64_PSRLW + 2, 0, WAH_JR_VT0, WAH_JR_VT1);
            wah_x64_rr(f, WAH_X64_PSRLW + 2, 0, d, WAH_JR_VT1);
        }
        wah_x64_rr(f, WAH_X64_PXOR, 0, d, WAH_JR_VT0);
        wah_x64_rr(f, WAH_X64_LANE(WAH_X64_PSUBB, 3), 0, d, WAH_JR_VT0);
    } else {
        d = wah_x64_vcopy(f, a);
        if (is_const) wah_x64_rr_imm(f, WAH_X64_PSHIFT_IMM + size_log2 - 1, 0, digit, d, s);
        else wah_x64_rr(f, by_xmm + size_log2 - 1, 0, d, WAH_JR_VT1);
    }
    wah_jit_finish_vec(f, d, a, -1, -1);
}

// any_true (size_log2 < 0) and all_true, which compares lanes with zero
static void wah_x64_vtest(wah_jit_fn_t *f, int size_log2, bool fuse) {
    static const uint32_t cmpeq[] = { WAH_X64_PCMPEQB, WAH_X64_PCMPEQB + 1, WAH_X64_PCMPEQB + 2, WAH_X64_PCMPEQQ };
    int a = wah_jit_pop_vec_ro(f), cond = WAH_X64_NE;
    if (size_log2 < 0) {
        wah_x64_rr(f, WAH_X64_PTEST, 0, a, a);
    } else {
        wah_x64_rr(f, WAH_X64_PXOR, 0, WAH_JR_VT0, WAH_JR_VT0);
        wah_x64_rr(f, cmpeq[size_log2], 0, WAH_JR_VT0, a);
        wah_x64_rr(f, WAH_X64_PTEST, 0, WAH_JR_VT0, WAH_JR_VT0);
        cond = WAH_X64_E;
    }
    wah_jit_free_fpr(f, a);
    if (fuse) {
        wah_jit_push(f, (wah_jit_val_t){ .kind = WAH_JV_FLAGS, .reg = (uint8_t)cond });
    } else {
        int d = wah_jit_alloc(f, false);
        wah_jit_setcc(f, d, cond);
        wah_jit_push_gpr(f, d);
    }
}

static void wah_x64_vbitmask(wah_jit_fn_t *f, uint32_t size_log2) {
    int a = wah_jit_pop_vec_ro(f), d = wah_jit_alloc(f, false);
    switch (size_log2) {
        case 0: wah_x64_rr(f, WAH_X64_PMOVMSKB, 0, d, a); break;
        case 1: // Saturated to bytes, which keeps signs
            wah_jit_vmov(f, WAH_JR_VT0, a);
            wah_x64_rr(f, WAH_X64_PACKSSWB, 0, WAH_JR_VT0, WAH_JR_VT0);
            wah_x64_rr(f, WAH_X64_PMOVMSKB, 0, d, WAH_JR_VT0);
            wah_x64_rr(f, WAH_X64_MOVZX8, WAH_X64_B8, d, d);
            break;
        default: wah_x64_rr(f, WAH_X64_PFP(size_log2 == 3) | WAH_X64_MOVMSKPS, 0, d, a); break;
    }
    wah_jit_free_fpr(f, a);
    wah_jit_push_gpr(f, d);
}

static void wah_x64_vextract(wah_jit_fn_t *f, uint32_t size_log2, uint32_t lane, bool is_signed, bool fp) {
    int a = wah_jit_pop_vec_ro(f);
    int d = wah_jit_alloc(f, fp);
    if (fp) { // The lane is moved to the lowest bits
        if (lane == 0) wah_jit_vmov(f, d, a);
        else wah_x64_rr_imm(f, WAH_X64_PSHUFD, 0, d, a, size_log2 == 3 ? 0xEE : lane);
    } else if (size_log2 == 1) {
        wah_x64_rr_imm(f, WAH_X64_PEXTRW, 0, d, a, lane);
        if (is_signed) wah_x64_rr(f, WAH_X64_MOVSX16, 0, d, d);
    } else if (size_log2 == 0) {
        wah_x64_rr_imm(f, WAH_X64_PEXTRB, 0, a, d, lane);
        if (is_signed) wah_x64_rr(f, WAH_X64_MOVSX8, WAH_X64_B8, d, d);
    } else if (lane == 0) {
        wah_x64_rr(f, WAH_X64_MOVD_FROM_XMM, size_log2 == 3 ? WAH_X64_W : 0, a, d);
    } else {
        wah_x64_rr_imm(f, WAH_X64_PEXTRB + 2, size_log2 == 3 ? WAH_X64_W : 0, a, d, lane);
    }
    wah_jit_free_fpr(f, a);
    if (fp) wah_jit_push_fpr(f, d);
    else wah_jit_push_gpr(f, d);
}

static void wah_x64_vreplace(wah_jit_fn_t *f, uint32_t size_log2, uint32_t lane, bool fp) {
    const wah_jit_val_t *top = &f->vals[f->h - 1];
    int s;
    bool is_const = !fp && top->kind == WAH_JV_CONST;
    int64_t imm = top->imm;
    if (fp) s = wah_jit_pop_fpr_ro(f);
    else if (is_const) { f->h--; s = WAH_JR_T0; }
    else s = wah_jit_pop_gpr_ro(f);
    int d = wah_jit_pop_vec(f);
    if (is_const) wah_jit_mov_imm(f, WAH_JR_T0, (uint64_t)imm); // Spills above may have used T0
    if (fp && size_log2 == 2) wah_x64_rr_imm(f, WAH_X64_INSERTPS, 0, d, s, lane << 4);
    else if (fp && lane == 0) wah_x64_rr(f, WAH_X64_SD | WAH_X64_MOVS_LOAD, 0, d, s); // MOVSD keeps the upper half
    else if (fp) wah_x64_rr(f, WAH_X64_PFP(1) | WAH_X64_UNPCKLPS, 0, d, s);
    else if (size_log2 == 1) wah_x64_rr_imm(f, WAH_X64_PINSRW, 0, d, s, lane);
    else if (size_log2 == 0) wah_x64_rr_imm(f, WAH_X64_PINSRB, 0, d, s, lane);
    else wah_x64_rr_imm(f, WAH_X64_PINSRB + 2, size_log2 == 3 ? WAH_X64_W : 0, d, s, lane);
    if (fp) wah_jit_free_fpr(f, s);
    else if (s != WAH_JR_T0) wah_jit_free_gpr(f, s);
    wah_jit_push_vec(f, d);
}

static void wah_x64_vsplat(wah_jit_fn_t *f, uint32_t size_log2, bool fp) {
    const wah_jit_val_t *top = &f->vals[f->h - 1];
    if (top->kind == WAH_JV_CONST) {
        wah_jit_v16_t c = wah_jit_splat_const((uint64_t)top->imm, size_log2);
        f->h--;
        int d = wah_jit_alloc(f, true);
        wah_jit_load_vconst(f, d, c.b);
        wah_jit_push_vec(f, d);
        return;
    }
    if (fp) {
        int s = wah_jit_pop_fpr_ro(f), d = wah_jit_dest(f, true, s, -1);
        wah_x64_rr_imm(f, WAH_X64_PSHUFD, 0, d, s, size_log2 == 3 ? 0x44 : 0);
        wah_jit_finish_vec(f, d, s, -1, -1);
        return;
    }
    int s = wah_jit_pop_gpr_ro(f), d = wah_jit_alloc(f, true);
    wah_x64_rr(f, WAH_X64_MOVD_TO_XMM, size_log2 == 3 ? WAH_X64_W : 0, d, s);
    switch (size_log2) {
        case 0:
            wah_x64_rr(f, WAH_X64_PXOR, 0, WAH_JR_VT0, WAH_JR_VT0);
            wah_x64_rr(f, WAH_X64_PSHUFB, 0, d, WAH_JR_VT0);
            break;
        case 1:
            wah_x64_rr_imm(f, WAH_X64_PSHUFLW, 0, d, d, 0);
            wah_x64_rr_imm(f, WAH_X64_PSHUFD, 0, d, d, 0);
            break;
        case 2: wah_x64_rr_imm(f, WAH_X64_PSHUFD, 0, d, d, 0); break;
        default: wah_x64_rr(f, WAH_X64_PUNPCKLQDQ, 0, d, d); break;
    }
    wah_jit_free_gpr(f, s);
    wah_jit_push_vec(f, d);
}

// Loads with an access of 1 << size_log2 bytes, as a whole (kind 0), extended from 64 bits by op (1), splatted (3),
// zero-extended (4) or into a lane (5)
static void wah_x64_vload(wah_jit_fn_t *f, const wah_decoded_instr_t *in, uint32_t size_log2, int kind, uint32_t op) {
    const wah_memarg_t *m = kind == 5 ? &in->imm.memarg_lane.memarg : &in->imm.memarg;
    int d = kind == 5 ? wah_jit_pop_vec(f) : -1, size_reg, index;
    int32_t disp;
    int base = wah_jit_address(f, m, 1u << size_log2, &size_reg, &index, &disp);
    if (size_reg >= 0) wah_jit_free_gpr(f, size_reg);
    if (base != WAH_JR_MEM) wah_jit_free_gpr(f, base);
    if (d < 0) d = wah_jit_alloc(f, true);
    uint32_t w = size_log2 == 3 ? WAH_X64_W : 0;
    switch (kind) {
        case 0: wah_x64_mem(f, WAH_X64_MOVS_LOAD, 0, d, base, index, 0, disp); break; // MOVUPS
        case 1: wah_x64_mem(f, op, 0, d, base, index, 0, disp); break;
        case 3:
            if (size_log2 == 3) {
                wah_x64_mem(f, 0xF2000F12u, 0, d, base, index, 0, disp); // MOVDDUP
                break;
            }
            if (size_log2 == 2) wah_x64_mem(f, WAH_X64_SS | WAH_X64_MOVS_LOAD, 0, d, base, index, 0, disp);
            else wah_x64_mem(f, size_log2 == 0 ? WAH_X64_PINSRB : WAH_X64_PINSRW, 0, d, base, index, 0, disp);
            if (size_log2 != 2) wah_x64_byte(f, 0);
            if (size_log2 == 0) {
                wah_x64_rr(f, WAH_X64_PXOR, 0, WAH_JR_VT0, WAH_JR_VT0);
                wah_x64_rr(f, WAH_X64_PSHUFB, 0, d, WAH_JR_VT0);
            } else {
                if (size_log2 == 1) wah_x64_rr_imm(f, WAH_X64_PSHUFLW, 0, d, d, 0);
                wah_x64_rr_imm(f, WAH_X64_PSHUFD, 0, d, d, 0);
            }
            break;
        case 4:
            wah_x64_mem(f, size_log2 == 3 ? WAH_X64_MOVQ_LOAD : WAH_X64_SS | WAH_X64_MOVS_LOAD, 0, d, base, index, 0, disp);
            break;
        default:
            wah_x64_mem(f, size_log2 == 0 ? WAH_X64_PINSRB : size_log2 == 1 ? WAH_X64_PINSRW : WAH_X64_PINSRB + 2, w,
                        d, base, index, 0, disp);
            wah_x64_byte(f, in->imm.memarg_lane.lane);
            break;
    }
    wah_jit_push_vec(f, d);
}

static void wah_x64_vstore(wah_jit_fn_t *f, const wah_decoded_instr_t *in, uint32_t size_log2, bool lane) {
    const wah_memarg_t *m = lane ? &in->imm.memarg_lane.memarg : &in->imm.memarg;
    int v = wah_jit_pop_vec_ro(f), size_reg, index;
    int32_t disp;
    int base = wah_jit_address(f, m, 1u << size_log2, &size_reg, &index, &disp);
    if (lane) { // PEXTRB, PEXTRW, PEXTRD and PEXTRQ to memory
        wah_x64_mem(f, WAH_X64_PEXTRB + (size_log2 < 2 ? size_log2 : 2), size_log2 == 3 ? WAH_X64_W : 0,
                    v, base, index, 0, disp);
        wah_x64_byte(f, in->imm.memarg_lane.lane);
    } else {
        wah_x64_mem(f, WAH_X64_MOVS_STORE, 0, v, base, index, 0, disp); // MOVUPS
    }
    if (size_reg >= 0) wah_jit_free_gpr(f, size_reg);
    if (base != WAH_JR_MEM) wah_jit_free_gpr(f, base);
    wah_jit_free_fpr(f, v);
}

// PSHUFB of each operand, with lanes of the other one zeroed by indices with the top bit set
static void wah_x64_vshuffle(wah_jit_fn_t *f, const uint8_t *lanes) {
    uint8_t ma[16], mb[16];
    bool all_a = true, all_b = true;
    for (int i = 0; i < 16; i++) {
        ma[i] = lanes[i] < 16 ? lanes[i] : 0x80;
        mb[i] = lanes[i] >= 16 ? lanes[i] - 16 : 0x80;
        all_a = all_a && lanes[i] < 16;
        all_b = all_b && lanes[i] >= 16;
    }
    int b = wah_jit_pop_vec_ro(f), a = wah_jit_pop_vec_ro(f), d;
    if (all_b) {
        wah_x64_vconst_op(f, WAH_X64_PSHUFB, d = wah_x64_vcopy(f, b), mb);
    } else {
        if (!all_a) {
            wah_jit_vmov(f, WAH_JR_VT0, b);
            wah_x64_vconst_op(f, WAH_X64_PSHUFB, WAH_JR_VT0, mb);
        }
        wah_x64_vconst_op(f, WAH_X64_PSHUFB, d = wah_x64_vcopy(f, a), ma);
        if (!all_a) wah_x64_rr(f, WAH_X64_POR, 0, d, WAH_JR_VT0);
    }
    wah_jit_finish_vec(f, d, a, b, -1);
}

static void wah_x64_vswizzle(wah_jit_fn_t *f) {
    int b = wah_jit_pop_vec_ro(f), a = wah_jit_pop_vec_ro(f);
    wah_jit_vmov(f, WAH_JR_VT0, b);
    wah_x64_vsplat_op(f, WAH_X64_PADDUSB, WAH_JR_VT0, 0x70, 0); // Indices from 16 get the top bit
    int d = wah_x64_vcopy(f, a);
    wah_x64_rr(f, WAH_X64_PSHUFB, 0, d, WAH_JR_VT0);
    wah_jit_finish_vec(f, d, a, b, -1);
}

static void wah_x64_vpopcnt(wah_jit_fn_t *f) {
    static const uint8_t lut[16] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };
    int a = wah_jit_pop_vec_ro(f), d = wah_x64_vcopy(f, a);
    wah_jit_vmov(f, WAH_JR_VT0, d);
    wah_x64_rr_imm(f, WAH_X64_PSHIFT_IMM, 0, 2, WAH_JR_VT0, 4);
    wah_x64_vsplat_op(f, WAH_X64_PAND, WAH_JR_VT0, 0x0f, 0);
    wah_x64_vsplat_op(f, WAH_X64_PAND, d, 0x0f, 0);
    wah_jit_load_vconst(f, WAH_JR_VT1, lut);
    wah_x64_rr(f, WAH_X64_PSHUFB, 0, WAH_JR_VT1, d);
    wah_jit_load_vconst(f, d, lut);
    wah_x64_rr(f, WAH_X64_PSHUFB, 0, d, WAH_JR_VT0);
    wah_x64_rr(f, WAH_X64_LANE(WAH_X64_PADDB, 0), 0, d, WAH_JR_VT1);
    wah_jit_finish_vec(f, d, a, -1, -1);
}

// Extension of the upper half by PMOVSX* or PMOVZX*
static void wah_x64_vextend_high(wah_jit_fn_t *f, uint32_t op) {
    int a = wah_jit_pop_vec_ro(f), d = wah_jit_dest(f, true, a, -1);
    wah_x64_rr_imm(f, WAH_X64_PSHUFD, 0, d, a, 0xEE);
    wah_x64_rr(f, op, 0, d, d);
    wah_jit_finish_vec(f, d, a, -1, -1);
}

// Extended multiplication of halves of 1 << size_log2 byte lanes
static void wah_x64_vextmul(wah_jit_fn_t *f, uint32_t size_log2, bool high, bool is_signed) {
    int b = wah_jit_pop_vec_ro(f), a = wah_jit_pop_vec_ro(f), d;
    if (size_log2 == 0) { // Extended to 16 bits first
        uint32_t ext = is_signed ? WAH_X64_PMOVSXBW : WAH_X64_PMOVSXBW + 0x10;
        if (high) wah_x64_rr_imm(f, WAH_X64_PSHUFD, 0, WAH_JR_VT0, b, 0xEE);
        wah_x64_rr(f, ext, 0, WAH_JR_VT0, high ? WAH_JR_VT0 : b);
        d = wah_jit_dest(f, true, a, b);
        if (high) wah_x64_rr_imm(f, WAH_X64_PSHUFD, 0, d, a, 0xEE);
        wah_x64_rr(f, ext, 0, d, high ? d : a);
        wah_x64_rr(f, WAH_X64_PMULLW, 0, d, WAH_JR_VT0);
    } else if (size_log2 == 1) { // Lower and upper halves of 32-bit products interleaved
        wah_jit_vmov(f, WAH_JR_VT0, a);
        wah_x64_rr(f, is_signed ? WAH_X64_PMULHW : WAH_X64_PMULHUW, 0, WAH_JR_VT0, b);
        d = wah_x64_vcopy(f, a);
        wah_x64_rr(f, WAH_X64_PMULLW, 0, d, b);
        wah_x64_rr(f, high ? WAH_X64_PUNPCKHWD : WAH_X64_PUNPCKLWD, 0, d, WAH_JR_VT0);
    } else { // Lanes 0 and 2 are multiplied
        uint32_t order = high ? 0xFA : 0x50;
        wah_x64_rr_imm(f, WAH_X64_PSHUFD, 0, WAH_JR_VT0, b, order);
        d = wah_jit_dest(f, true, a, b);
        wah_x64_rr_imm(f, WAH_X64_PSHUFD, 0, d, a, order);
        wah_x64_rr(f, is_signed ? WAH_X64_PMULDQ : WAH_X64_PMULUDQ, 0, d, WAH_JR_VT0);
    }
    wah_jit_finish_vec(f, d, a, b, -1);
}

static void wah_x64_vextadd(wah_jit_fn_t *f, uint32_t size_log2, bool is_signed) {
    int a = wah_jit_pop_vec_ro(f), d;
    if (size_log2 == 0 && is_signed) { // PMADDUBSW takes unsigned bytes in the reg operand
        wah_jit_v16_t ones = wah_jit_splat_const(1, 0);
        wah_jit_load_vconst(f, WAH_JR_VT0, ones.b);
        wah_x64_rr(f, WAH_X64_PMADDUBSW, 0, WAH_JR_VT0, a);
        d = wah_jit_dest(f, true, a, -1);
        wah_jit_vmov(f, d, WAH_JR_VT0);
    } else if (size_log2 == 0) {
        wah_x64_vsplat_op(f, WAH_X64_PMADDUBSW, d = wah_x64_vcopy(f, a), 1, 0);
    } else { // Unsigned lanes are biased to signed ones, then 2 * 32768 is added back
        d = wah_x64_vcopy(f, a);
        if (!is_signed) wah_x64_vsplat_op(f, WAH_X64_PXOR, d, 0x8000, 1);
        wah_x64_vsplat_op(f, WAH_X64_PMADDWD, d, 1, 1);
        if (!is_signed) wah_x64_vsplat_op(f, WAH_X64_LANE(WAH_X64_PADDB, 2), d, 0x10000, 2);
    }
    wah_jit_finish_vec(f, d, a, -1, -1);
}

// PMULHRSW gives -32768 for -32768 * -32768, which saturates to 32767
static void wah_x64_vq15mulr(wah_jit_fn_t *f) {
    wah_x64_vbin(f, WAH_X64_PMULHRSW, false, true, -1, false, -1);
    int d = f->vals[f->h - 1].reg;
    wah_x64_vsplat_op(f, WAH_X64_MOVAPS_LOAD, WAH_JR_VT0, 0x8000, 1);
    wah_x64_rr(f, WAH_X64_LANE(WAH_X64_PCMPEQB, 1), 0, WAH_JR_VT0, d);
    wah_x64_rr(f, WAH_X64_PXOR, 0, d, WAH_JR_VT0);
}

// Lower halves of 64-bit products, from three 32-bit multiplications
static void wah_x64_vi64_mul(wah_jit_fn_t *f) {
    int b = wah_jit_pop_vec_ro(f), a = wah_jit_pop_vec_ro(f);
    wah_jit_vmov(f, WAH_JR_VT0, a);
    wah_x64_rr_imm(f, WAH_X64_PSHIFT_IMM + 2, 0, 2, WAH_JR_VT0, 32);
    wah_x64_rr(f, WAH_X64_PMULUDQ, 0, WAH_JR_VT0, b);
    wah_jit_vmov(f, WAH_JR_VT1, b);
    wah_x64_rr_imm(f, WAH_X64_PSHIFT_IMM + 2, 0, 2, WAH_JR_VT1, 32);
    wah_x64_rr(f, WAH_X64_PMULUDQ, 0, WAH_JR_VT1, a);
    wah_x64_rr(f, WAH_X64_PADDQ, 0, WAH_JR_VT0, WAH_JR_VT1);
    wah_x64_rr_imm(f, WAH_X64_PSHIFT_IMM + 2, 0, 6, WAH_JR_VT0, 32);
    int d = wah_x64_vcopy(f, a);
    wah_x64_rr(f, WAH_X64_PMULUDQ, 0, d, b);
    wah_x64_rr(f, WAH_X64_PADDQ, 0, d, WAH_JR_VT0);
    wah_jit_finish_vec(f, d, a, b, -1);
}

static void wah_x64_vi64_abs(wah_jit_fn_t *f) {
    int a = wah_jit_pop_vec_ro(f);
    wah_x64_rr(f, WAH_X64_PXOR, 0, WAH_JR_VT0, WAH_JR_VT0);
    wah_x64_rr(f, WAH_X64_PCMPGTQ, 0, WAH_JR_VT0, a);
    int d = wah_x64_vcopy(f, a);
    wah_x64_rr(f, WAH_X64_PXOR, 0, d, WAH_JR_VT0);
    wah_x64_rr(f, WAH_X64_LANE(WAH_X64_PSUBB, 3), 0, d, WAH_JR_VT0);
    wah_jit_finish_vec(f, d, a, -1, -1);
}

// Saturating conversions to i32 from f32 (lanes 0-3) or f64 (lanes 0-1, zeroing the rest)
static void wah_x64_vtrunc_sat(wah_jit_fn_t *f, int type, bool is_signed) {
    int a = wah_jit_pop_vec_ro(f), d = wah_x64_vcopy(f, a);
    if (type == 0 && is_signed) {
        wah_jit_vmov(f, WAH_JR_VT0, d);
        wah_x64_rr_imm(f, WAH_X64_CMPPS, 0, WAH_JR_VT0, WAH_JR_VT0, 0); // Not NaN
        wah_x64_rr(f, WAH_X64_ANDPS, 0, d, WAH_JR_VT0);
        wah_x64_rr(f, WAH_X64_PXOR, 0, WAH_JR_VT0, d); // Sign bit set if not negative
        wah_x64_rr(f, WAH_X64_CVTTPS2DQ, 0, d, d); // 0x80000000 if out of range
        wah_x64_rr(f, WAH_X64_PAND, 0, WAH_JR_VT0, d);
        wah_x64_rr_imm(f, WAH_X64_PSHIFT_IMM + 1, 0, 4, WAH_JR_VT0, 31);
        wah_x64_rr(f, WAH_X64_PXOR, 0, d, WAH_JR_VT0);
    } else if (type == 0) { // Values from 2^31 are converted after subtracting it
        wah_x64_rr(f, WAH_X64_PXOR, 0, WAH_JR_VT0, WAH_JR_VT0);
        wah_x64_rr(f, WAH_X64_MAXS, 0, d, WAH_JR_VT0); // NaN and negative values become 0
        wah_x64_vsplat_op(f, WAH_X64_MOVAPS_LOAD, WAH_JR_VT0, 0x4f000000, 2); // 2^31
        wah_jit_vmov(f, WAH_JR_VT1, d);
        wah_x64_rr(f, WAH_X64_SUBS, 0, WAH_JR_VT1, WAH_JR_VT0);
        wah_x64_rr_imm(f, WAH_X64_CMPPS, 0, WAH_JR_VT0, WAH_JR_VT1, 2); // Overflows of the subtracted value
        wah_x64_rr(f, WAH_X64_CVTTPS2DQ, 0, WAH_JR_VT1, WAH_JR_VT1);
        wah_x64_rr(f, WAH_X64_PXOR, 0, WAH_JR_VT1, WAH_JR_VT0);
        wah_x64_rr(f, WAH_X64_PXOR, 0, WAH_JR_VT0, WAH_JR_VT0);
        wah_x64_rr(f, WAH_X64_PMAXSD, 0, WAH_JR_VT1, WAH_JR_VT0);
        wah_x64_rr(f, WAH_X64_CVTTPS2DQ, 0, d, d);
        wah_x64_rr(f, WAH_X64_LANE(WAH_X64_PADDB, 2), 0, d, WAH_JR_VT1);
    } else if (is_signed) { // Clamped to INT32_MAX, as the conversion gives INT32_MIN for overflows
        wah_jit_vmov(f, WAH_JR_VT0, d);
        wah_x64_rr_imm(f, WAH_X64_PFP(1) | WAH_X64_CMPPS, 0, WAH_JR_VT0, WAH_JR_VT0, 0);
        wah_x64_vsplat_op(f, WAH_X64_ANDPS, WAH_JR_VT0, 0x41dfffffffc00000ull, 3); // 2147483647.0
        wah_x64_rr(f, WAH_X64_PFP(1) | WAH_X64_MINS, 0, d, WAH_JR_VT0); // NaN becomes 0
        wah_x64_rr(f, WAH_X64_CVTTPD2DQ, 0, d, d);
    } else { // Clamped and truncated, then placed in the mantissa of 2^52 + d
        wah_x64_rr(f, WAH_X64_PXOR, 0, WAH_JR_VT0, WAH_JR_VT0);
        wah_x64_rr(f, WAH_X64_PFP(1) | WAH_X64_MAXS, 0, d, WAH_JR_VT0);
        wah_x64_vsplat_op(f, WAH_X64_PFP(1) | WAH_X64_MINS, d, 0x41efffffffe00000ull, 3); // 4294967295.0
        wah_x64_rr_imm(f, WAH_X64_ROUNDPS + 1, 0, d, d, 3);
        wah_x64_vsplat_op(f, WAH_X64_PFP(1) | WAH_X64_ADDS, d, 0x4330000000000000ull, 3);
        wah_x64_rr_imm(f, WAH_X64_SHUFPS, 0, d, WAH_JR_VT0, 0x88);
    }
    wah_jit_finish_vec(f, d, a, -1, -1);
}

static void wah_x64_vconvert_u(wah_jit_fn_t *f, bool to_double) {
    int a = wah_jit_pop_vec_ro(f), d = wah_x64_vcopy(f, a);
    if (to_double) { // 2^52 + d as doubles, minus 2^52
        wah_x64_vsplat_op(f, WAH_X64_UNPCKLPS, d, 0x43300000, 2);
        wah_x64_vsplat_op(f, WAH_X64_PFP(1) | WAH_X64_SUBS, d, 0x4330000000000000ull, 3);
    } else { // Lower 16 bits and the rest (halved to be in range) are converted exactly, and added with one rounding
        wah_x64_rr(f, WAH_X64_PXOR, 0, WAH_JR_VT0, WAH_JR_VT0);
        wah_x64_rr_imm(f, WAH_X64_PBLENDW, 0, WAH_JR_VT0, d, 0x55);
        wah_x64_rr(f, WAH_X64_LANE(WAH_X64_PSUBB, 2), 0, d, WAH_JR_VT0);
        wah_x64_rr(f, WAH_X64_CVTDQ2PS, 0, WAH_JR_VT0, WAH_JR_VT0);
        wah_x64_rr_imm(f, WAH_X64_PSHIFT_IMM + 1, 0, 2, d, 1);
        wah_x64_rr(f, WAH_X64_CVTDQ2PS, 0, d, d);
        wah_x64_rr(f, WAH_X64_ADDS, 0, d, d);
        wah_x64_rr(f, WAH_X64_ADDS, 0, d, WAH_JR_VT0);
    }
    wah_jit_finish_vec(f, d, a, -1, -1);
}

// ((a ^ b) & c) ^ b
static void wah_x64_vbitselect(wah_jit_fn_t *f) {
    int c = wah_jit_pop_vec_ro(f), b = wah_jit_pop_vec_ro(f), a = wah_jit_pop_vec_ro(f), d = wah_x64_vcopy(f, a);
    wah_x64_rr(f, WAH_X64_PXOR, 0, d, b);
    wah_x64_rr(f, WAH_X64_PAND, 0, d, c);
    wah_x64_rr(f, WAH_X64_PXOR, 0, d, b);
    wah_jit_finish_vec(f, d, a, b, c);
}

#if ((WAH_COMPILED_FEATURES) & WAH_FEATURE_RELAXED_SIMD)
// a * b + c, or c - a * b if negated, rounded twice
static void wah_x64_vmadd(wah_jit_fn_t *f, int type, bool negated) {
    int c = wah_jit_pop_vec_ro(f), b = wah_jit_pop_vec_ro(f), a = wah_jit_pop_vec_ro(f);
    wah_jit_vmov(f, WAH_JR_VT0, a);
    wah_x64_rr(f, WAH_X64_PFP(type) | WAH_X64_MULS, 0, WAH_JR_VT0, b);
    int d = wah_jit_vdest3(f, c, a, b);
    if (d != c) wah_jit_vmov(f, d, c);
    wah_x64_rr(f, WAH_X64_PFP(type) | (negated ? WAH_X64_SUBS : WAH_X64_ADDS), 0, d, WAH_JR_VT0);
    wah_x64_vcanonicalize(f, type, d);
    wah_jit_finish_vec(f, d, a, b, c);
}

// Sums of products of signed bytes, with adjacent pairs saturated to i16, then summed again with c if given
static void wah_x64_vdot_i8(wah_jit_fn_t *f, bool add) {
    int c = add ? wah_jit_pop_vec_ro(f) : -1, b = wah_jit_pop_vec_ro(f), a = wah_jit_pop_vec_ro(f);
    int t = wah_jit_alloc(f, true);
    wah_x64_rr(f, WAH_X64_PMOVSXBW, 0, WAH_JR_VT0, a);
    wah_x64_rr(f, WAH_X64_PMOVSXBW, 0, WAH_JR_VT1, b);
    wah_x64_rr(f, WAH_X64_PMADDWD, 0, WAH_JR_VT0, WAH_JR_VT1);
    wah_x64_rr_imm(f, WAH_X64_PSHUFD, 0, t, a, 0xEE);
    wah_x64_rr(f, WAH_X64_PMOVSXBW, 0, t, t);
    wah_x64_rr_imm(f, WAH_X64_PSHUFD, 0, WAH_JR_VT1, b, 0xEE);
    wah_x64_rr(f, WAH_X64_PMOVSXBW, 0, WAH_JR_VT1, WAH_JR_VT1);
    wah_x64_rr(f, WAH_X64_PMADDWD, 0, t, WAH_JR_VT1);
    if (add) { // Only 32768 is out of range
        for (int k = 0; k < 2; k++) {
            int r = k ? t : WAH_JR_VT0;
            wah_x64_vsplat_op(f, WAH_X64_MOVAPS_LOAD, WAH_JR_VT1, 32768, 2);
            wah_x64_rr(f, WAH_X64_LANE(WAH_X64_PCMPEQB, 2), 0, WAH_JR_VT1, r);
            wah_x64_rr(f, WAH_X64_LANE(WAH_X64_PADDB, 2), 0, r, WAH_JR_VT1);
        }
        wah_x64_rr(f, WAH_X64_PHADDD, 0, WAH_JR_VT0, t);
        wah_x64_rr(f, WAH_X64_LANE(WAH_X64_PADDB, 2), 0, WAH_JR_VT0, c);
    } else {
        wah_x64_rr(f, WAH_X64_PACKSSDW, 0, WAH_JR_VT0, t);
    }
    wah_jit_free_fpr(f, t);
    int d = add ? wah_jit_vdest3(f, c, a, b) : wah_jit_dest(f, true, a, b);
    wah_jit_vmov(f, d, WAH_JR_VT0);
    wah_jit_finish_vec(f, d, a, b, c);
}
#endif

static void wah_jit_simd(wah_jit_fn_t *f, const wah_decoded_instr_t *in, bool fuse) {
    switch (in->opcode) {
        case WAH_OP_V128_LOAD: wah_x64_vload(f, in, 4, 0, 0); break;
        case WAH_OP_V128_LOAD8X8_S: wah_x64_vload(f, in, 3, 1, WAH_X64_PMOVSXBW); break;
        case WAH_OP_V128_LOAD8X8_U: wah_x64_vload(f, in, 3, 1, WAH_X64_PMOVSXBW + 0x10); break;
        case WAH_OP_V128_LOAD16X4_S: wah_x64_vload(f, in, 3, 1, WAH_X64_PMOVSXBW + 3); break;
        case WAH_OP_V128_LOAD16X4_U: wah_x64_vload(f, in, 3, 1, WAH_X64_PMOVSXBW + 0x13); break;
        case WAH_OP_V128_LOAD32X2_S: wah_x64_vload(f, in, 3, 1, WAH_X64_PMOVSXBW + 5); break;
        case WAH_OP_V128_LOAD32X2_U: wah_x64_vload(f, in, 3, 1, WAH_X64_PMOVSXBW + 0x15); break;
        case WAH_OP_V128_LOAD8_SPLAT: wah_x64_vload(f, in, 0, 3, 0); break;
        case WAH_OP_V128_LOAD16_SPLAT: wah_x64_vload(f, in, 1, 3, 0); break;
        case WAH_OP_V128_LOAD32_SPLAT: wah_x64_vload(f, in, 2, 3, 0); break;
        case WAH_OP_V128_LOAD64_SPLAT: wah_x64_vload(f, in, 3, 3, 0); break;
        case WAH_OP_V128_LOAD32_ZERO: wah_x64_vload(f, in, 2, 4, 0); break;
        case WAH_OP_V128_LOAD64_ZERO: wah_x64_vload(f, in, 3, 4, 0); break;
        case WAH_OP_V128_LOAD8_LANE: wah_x64_vload(f, in, 0, 5, 0); break;
        case WAH_OP_V128_LOAD16_LANE: wah_x64_vload(f, in, 1, 5, 0); break;
        case WAH_OP_V128_LOAD32_LANE: wah_x64_vload(f, in, 2, 5, 0); break;
        case WAH_OP_V128_LOAD64_LANE: wah_x64_vload(f, in, 3, 5, 0); break;
        case WAH_OP_V128_STORE: wah_x64_vstore(f, in, 4, false); break;
        case WAH_OP_V128_STORE8_LANE: wah_x64_vstore(f, in, 0, true); break;
        case WAH_OP_V128_STORE16_LANE: wah_x64_vstore(f, in, 1, true); break;
        case WAH_OP_V128_STORE32_LANE: wah_x64_vstore(f, in, 2, true); break;
        case WAH_OP_V128_STORE64_LANE: wah_x64_vstore(f, in, 3, true); break;

        case WAH_OP_V128_CONST: {
            int d = wah_jit_alloc(f, true);
            wah_jit_load_vconst(f, d, in->imm.v128);
            wah_jit_push_vec(f, d);
            break;
        }
        case WAH_OP_I8X16_SHUFFLE: wah_x64_vshuffle(f, in->imm.v128); break;
        case WAH_OP_I8X16_SWIZZLE: WAH_IF_RELAXED_SIMD(case WAH_OP_I8X16_RELAXED_SWIZZLE:) wah_x64_vswizzle(f); break;
        case WAH_OP_I8X16_EXTRACT_LANE_S: wah_x64_vextract(f, 0, in->imm.u32, true, false); break;
        case WAH_OP_I8X16_EXTRACT_LANE_U: wah_x64_vextract(f, 0, in->imm.u32, false, false); break;
        case WAH_OP_I16X8_EXTRACT_LANE_S: wah_x64_vextract(f, 1, in->imm.u32, true, false); break;
        case WAH_OP_I16X8_EXTRACT_LANE_U: wah_x64_vextract(f, 1, in->imm.u32, false, false); break;
        case WAH_OP_I32X4_EXTRACT_LANE: wah_x64_vextract(f, 2, in->imm.u32, false, false); break;
        case WAH_OP_I64X2_EXTRACT_LANE: wah_x64_vextract(f, 3, in->imm.u32, false, false); break;
        case WAH_OP_F32X4_EXTRACT_LANE: wah_x64_vextract(f, 2, in->imm.u32, false, true); break;
        case WAH_OP_F64X2_EXTRACT_LANE: wah_x64_vextract(f, 3, in->imm.u32, false, true); break;
        case WAH_OP_I8X16_REPLACE_LANE: wah_x64_vreplace(f, 0, in->imm.u32, false); break;
        case WAH_OP_I16X8_REPLACE_LANE: wah_x64_vreplace(f, 1, in->imm.u32, false); break;
        case WAH_OP_I32X4_REPLACE_LANE: wah_x64_vreplace(f, 2, in->imm.u32, false); break;
        case WAH_OP_I64X2_REPLACE_LANE: wah_x64_vreplace(f, 3, in->imm.u32, false); break;
        case WAH_OP_F32X4_REPLACE_LANE: wah_x64_vreplace(f, 2, in->imm.u32, true); break;
        case WAH_OP_F64X2_REPLACE_LANE: wah_x64_vreplace(f, 3, in->imm.u32, true); break;
        case WAH_OP_I8X16_SPLAT: wah_x64_vsplat(f, 0, false); break;
        case WAH_OP_I16X8_SPLAT: wah_x64_vsplat(f, 1, false); break;
        case WAH_OP_I32X4_SPLAT: wah_x64_vsplat(f, 2, false); break;
        case WAH_OP_I64X2_SPLAT: wah_x64_vsplat(f, 3, false); break;
        case WAH_OP_F32X4_SPLAT: wah_x64_vsplat(f, 2, true); break;
        case WAH_OP_F64X2_SPLAT: wah_x64_vsplat(f, 3, true); break;

        case WAH_OP_V128_NOT: {
            int a = wah_jit_pop_vec_ro(f), d = wah_x64_vcopy(f, a);
            wah_x64_vnot(f, d);
            wah_jit_finish_vec(f, d, a, -1, -1);
            break;
        }
        case WAH_OP_V128_AND: WAH_X64_VBIN_C(f, WAH_X64_PAND); break;
        case WAH_OP_V128_ANDNOT: wah_x64_vbin(f, WAH_X64_PANDN, true, false, -1, false, -1); break; // ~b & a
        case WAH_OP_V128_OR: WAH_X64_VBIN_C(f, WAH_X64_POR); break;
        case WAH_OP_V128_XOR: WAH_X64_VBIN_C(f, WAH_X64_PXOR); break;
        case WAH_OP_V128_BITSELECT:
        WAH_IF_RELAXED_SIMD(case WAH_OP_I8X16_RELAXED_LANESELECT: case WAH_OP_I16X8_RELAXED_LANESELECT:
                            case WAH_OP_I32X4_RELAXED_LANESELECT: case WAH_OP_I64X2_RELAXED_LANESELECT:)
            wah_x64_vbitselect(f);
            break;
        case WAH_OP_V128_ANY_TRUE: wah_x64_vtest(f, -1, fuse); break;

#define WAH_X64_VCMP(N, sz, eq, gt) \
        case WAH_OP_##N##_EQ: wah_x64_vbin(f, eq, false, true, -1, false, -1); break; \
        case WAH_OP_##N##_NE: wah_x64_vbin(f, eq, false, true, -1, true, -1); break; \
        case WAH_OP_##N##_LT_S: wah_x64_vbin(f, gt, true, false, -1, false, -1); break; \
        case WAH_OP_##N##_GT_S: wah_x64_vbin(f, gt, false, false, -1, false, -1); break; \
        case WAH_OP_##N##_LE_S: wah_x64_vbin(f, gt, false, false, -1, true, -1); break; \
        case WAH_OP_##N##_GE_S: wah_x64_vbin(f, gt, true, false, -1, true, -1); break;
#define WAH_X64_VCMP_U(N, sz) \
        case WAH_OP_##N##_LT_U: wah_x64_vcmp_u(f, sz, true, true); break; \
        case WAH_OP_##N##_GT_U: wah_x64_vcmp_u(f, sz, false, true); break; \
        case WAH_OP_##N##_LE_U: wah_x64_vcmp_u(f, sz, false, false); break; \
        case WAH_OP_##N##_GE_U: wah_x64_vcmp_u(f, sz, true, false); break;
        WAH_X64_VCMP(I8X16, 0, WAH_X64_PCMPEQB, WAH_X64_PCMPGTB) WAH_X64_VCMP_U(I8X16, 0)
        WAH_X64_VCMP(I16X8, 1, WAH_X64_PCMPEQB + 1, WAH_X64_PCMPGTB + 1) WAH_X64_VCMP_U(I16X8, 1)
        WAH_X64_VCMP(I32X4, 2, WAH_X64_PCMPEQB + 2, WAH_X64_PCMPGTB + 2) WAH_X64_VCMP_U(I32X4, 2)
        WAH_X64_VCMP(I64X2, 3, WAH_X64_PCMPEQQ, WAH_X64_PCMPGTQ)
#undef WAH_X64_VCMP
#undef WAH_X64_VCMP_U
        // Unordered lanes compare unequal and false otherwise; greater is less with swapped operands
#define WAH_X64_VFCMP(N, type) \
        case WAH_OP_##N##_EQ: wah_x64_vbin(f, WAH_X64_PFP(type) | WAH_X64_CMPPS, false, true, 0, false, -1); break; \
        case WAH_OP_##N##_NE: wah_x64_vbin(f, WAH_X64_PFP(type) | WAH_X64_CMPPS, false, true, 4, false, -1); break; \
        case WAH_OP_##N##_LT: wah_x64_vbin(f, WAH_X64_PFP(type) | WAH_X64_CMPPS, false, false, 1, false, -1); break; \
        case WAH_OP_##N##_GT: wah_x64_vbin(f, WAH_X64_PFP(type) | WAH_X64_CMPPS, true, false, 1, false, -1); break; \
        case WAH_OP_##N##_LE: wah_x64_vbin(f, WAH_X64_PFP(type) | WAH_X64_CMPPS, false, false, 2, false, -1); break; \
        case WAH_OP_##N##_GE: wah_x64_vbin(f, WAH_X64_PFP(type) | WAH_X64_CMPPS, true, false, 2, false, -1); break;
        WAH_X64_VFCMP(F32X4, 0)
        WAH_X64_VFCMP(F64X2, 1)
#undef WAH_X64_VFCMP

#define WAH_X64_VINT(N, sz) \
        case WAH_OP_##N##_NEG: wah_x64_vneg(f, sz); break; \
        case WAH_OP_##N##_ALL_TRUE: wah_x64_vtest(f, sz, fuse); break; \
        case WAH_OP_##N##_BITMASK: wah_x64_vbitmask(f, sz); break; \
        case WAH_OP_##N##_SHL: wah_x64_vshift(f, sz, WAH_JS_SHL); break; \
        case WAH_OP_##N##_SHR_S: wah_x64_vshift(f, sz, WAH_JS_SHR_S); break; \
        case WAH_OP_##N##_SHR_U: wah_x64_vshift(f, sz, WAH_JS_SHR_U); break; \
        case WAH_OP_##N##_ADD: WAH_X64_VBIN_C(f, (sz) == 3 ? WAH_X64_PADDQ : WAH_X64_LANE(WAH_X64_PADDB, sz)); break; \
        case WAH_OP_##N##_SUB: WAH_X64_VBIN(f, WAH_X64_LANE(WAH_X64_PSUBB, sz)); break;
        WAH_X64_VINT(I8X16, 0)
        WAH_X64_VINT(I16X8, 1)
        WAH_X64_VINT(I32X4, 2)
        WAH_X64_VINT(I64X2, 3)
#undef WAH_X64_VINT
        case WAH_OP_I8X16_ABS: wah_x64_vun(f, WAH_X64_PABSB, -1, -1); break;
        case WAH_OP_I16X8_ABS: wah_x64_vun(f, WAH_X64_PABSB + 1, -1, -1); break;
        case WAH_OP_I32X4_ABS: wah_x64_vun(f, WAH_X64_PABSB + 2, -1, -1); break;
        case WAH_OP_I64X2_ABS: wah_x64_vi64_abs(f); break;
        case WAH_OP_I8X16_ADD_SAT_S: WAH_X64_VBIN_C(f, WAH_X64_PADDSB); break;
        case WAH_OP_I8X16_ADD_SAT_U: WAH_X64_VBIN_C(f, WAH_X64_PADDUSB); break;
        case WAH_OP_I8X16_SUB_SAT_S: WAH_X64_VBIN(f, WAH_X64_PSUBSB); break;
        case WAH_OP_I8X16_SUB_SAT_U: WAH_X64_VBIN(f, WAH_X64_PSUBUSB); break;
        case WAH_OP_I8X16_AVGR_U: WAH_X64_VBIN_C(f, WAH_X64_PAVGB); break;
        case WAH_OP_I8X16_MIN_S: WAH_X64_VBIN_C(f, WAH_X64_PMINSB); break;
        case WAH_OP_I8X16_MIN_U: WAH_X64_VBIN_C(f, WAH_X64_PMINUB); break;
        case WAH_OP_I8X16_MAX_S: WAH_X64_VBIN_C(f, WAH_X64_PMAXSB); break;
        case WAH_OP_I8X16_MAX_U: WAH_X64_VBIN_C(f, WAH_X64_PMAXUB); break;
        case WAH_OP_I16X8_ADD_SAT_S: WAH_X64_VBIN_C(f, WAH_X64_PADDSW); break;
        case WAH_OP_I16X8_ADD_SAT_U: WAH_X64_VBIN_C(f, WAH_X64_PADDUSW); break;
        case WAH_OP_I16X8_SUB_SAT_S: WAH_X64_VBIN(f, WAH_X64_PSUBSW); break;
        case WAH_OP_I16X8_SUB_SAT_U: WAH_X64_VBIN(f, WAH_X64_PSUBUSW); break;
        case WAH_OP_I16X8_AVGR_U: WAH_X64_VBIN_C(f, WAH_X64_PAVGW); break;
        case WAH_OP_I16X8_MIN_S: WAH_X64_VBIN_C(f, WAH_X64_PMINSW); break;
        case WAH_OP_I16X8_MIN_U: WAH_X64_VBIN_C(f, WAH_X64_PMINUW); break;
        case WAH_OP_I16X8_MAX_S: WAH_X64_VBIN_C(f, WAH_X64_PMAXSW); break;
        case WAH_OP_I16X8_MAX_U: WAH_X64_VBIN_C(f, WAH_X64_PMAXUW); break;
        case WAH_OP_I32X4_MIN_S: WAH_X64_VBIN_C(f, WAH_X64_PMINSD); break;
        case WAH_OP_I32X4_MIN_U: WAH_X64_VBIN_C(f, WAH_X64_PMINUD); break;
        case WAH_OP_I32X4_MAX_S: WAH_X64_VBIN_C(f, WAH_X64_PMAXSD); break;
        case WAH_OP_I32X4_MAX_U: WAH_X64_VBIN_C(f, WAH_X64_PMAXUD); break;
        case WAH_OP_I8X16_POPCNT: wah_x64_vpopcnt(f); break;
        case WAH_OP_I16X8_MUL: WAH_X64_VBIN_C(f, WAH_X64_PMULLW); break;
        case WAH_OP_I32X4_MUL: WAH_X64_VBIN_C(f, WAH_X64_PMULLD); break;
        case WAH_OP_I64X2_MUL: wah_x64_vi64_mul(f); break;
        case WAH_OP_I16X8_Q15MULR_SAT_S: WAH_IF_RELAXED_SIMD(case WAH_OP_I16X8_RELAXED_Q15MULR_S:) wah_x64_vq15mulr(f); break;
        case WAH_OP_I32X4_DOT_I16X8_S: WAH_X64_VBIN_C(f, WAH_X64_PMADDWD); break;
        case WAH_OP_I8X16_NARROW_I16X8_S: WAH_X64_VBIN(f, WAH_X64_PACKSSWB); break;
        case WAH_OP_I8X16_NARROW_I16X8_U: WAH_X64_VBIN(f, WAH_X64_PACKUSWB); break;
        case WAH_OP_I16X8_NARROW_I32X4_S: WAH_X64_VBIN(f, WAH_X64_PACKSSDW); break;
        case WAH_OP_I16X8_NARROW_I32X4_U: WAH_X64_VBIN(f, WAH_X64_PACKUSDW); break;

#define WAH_X64_VEXT(N, FROM, sz, ext) \
        case WAH_OP_##N##_EXTEND_LOW_##FROM##_S: wah_x64_vun(f, ext, -1, -1); break; \
        case WAH_OP_##N##_EXTEND_HIGH_##FROM##_S: wah_x64_vextend_high(f, ext); break; \
        case WAH_OP_##N##_EXTEND_LOW_##FROM##_U: wah_x64_vun(f, (ext) + 0x10, -1, -1); break; \
        case WAH_OP_##N##_EXTEND_HIGH_##FROM##_U: wah_x64_vextend_high(f, (ext) + 0x10); break; \
        case WAH_OP_##N##_EXTMUL_LOW_##FROM##_S: wah_x64_vextmul(f, sz, false, true); break; \
        case WAH_OP_##N##_EXTMUL_HIGH_##FROM##_S: wah_x64_vextmul(f, sz, true, true); break; \
        case WAH_OP_##N##_EXTMUL_LOW_##FROM##_U: wah_x64_vextmul(f, sz, false, false); break; \
        case WAH_OP_##N##_EXTMUL_HIGH_##FROM##_U: wah_x64_vextmul(f, sz, true, false); break;
        WAH_X64_VEXT(I16X8, I8X16, 0, WAH_X64_PMOVSXBW)
        WAH_X64_VEXT(I32X4, I16X8, 1, WAH_X64_PMOVSXBW + 3)
        WAH_X64_VEXT(I64X2, I32X4, 2, WAH_X64_PMOVSXBW + 5)
#undef WAH_X64_VEXT
        case WAH_OP_I16X8_EXTADD_PAIRWISE_I8X16_S: wah_x64_vextadd(f, 0, true); break;
        case WAH_OP_I16X8_EXTADD_PAIRWISE_I8X16_U: wah_x64_vextadd(f, 0, false); break;
        case WAH_OP_I32X4_EXTADD_PAIRWISE_I16X8_S: wah_x64_vextadd(f, 1, true); break;
        case WAH_OP_I32X4_EXTADD_PAIRWISE_I16X8_U: wah_x64_vextadd(f, 1, false); break;

        // MXCSR doesn't make NaN results canonical, so every operation which may return NaN checks it
#define WAH_X64_VFP(N, type) \
        case WAH_OP_##N##_CEIL: wah_x64_vun(f, WAH_X64_ROUNDPS + (type), 8 | 2, type); break; \
        case WAH_OP_##N##_FLOOR: wah_x64_vun(f, WAH_X64_ROUNDPS + (type), 8 | 1, type); break; \
        case WAH_OP_##N##_TRUNC: wah_x64_vun(f, WAH_X64_ROUNDPS + (type), 8 | 3, type); break; \
        case WAH_OP_##N##_NEAREST: wah_x64_vun(f, WAH_X64_ROUNDPS + (type), 8 | 0, type); break; \
        case WAH_OP_##N##_ABS: \
            wah_x64_vconst_bin(f, WAH_X64_ANDPS, (type) ? 0x7fffffffffffffffull : 0x7fffffffu, 2 + (type)); break; \
        case WAH_OP_##N##_NEG: \
            wah_x64_vconst_bin(f, WAH_X64_XORPS, (type) ? 0x8000000000000000ull : 0x80000000u, 2 + (type)); break; \
        case WAH_OP_##N##_SQRT: wah_x64_vun(f, WAH_X64_PFP(type) | WAH_X64_SQRTS, -1, type); break; \
        case WAH_OP_##N##_ADD: wah_x64_vbin(f, WAH_X64_PFP(type) | WAH_X64_ADDS, false, true, -1, false, type); break; \
        case WAH_OP_##N##_SUB: wah_x64_vbin(f, WAH_X64_PFP(type) | WAH_X64_SUBS, false, false, -1, false, type); break; \
        case WAH_OP_##N##_MUL: wah_x64_vbin(f, WAH_X64_PFP(type) | WAH_X64_MULS, false, true, -1, false, type); break; \
        case WAH_OP_##N##_DIV: wah_x64_vbin(f, WAH_X64_PFP(type) | WAH_X64_DIVS, false, false, -1, false, type); break; \
        case WAH_OP_##N##_MIN: WAH_IF_RELAXED_SIMD(case WAH_OP_##N##_RELAXED_MIN:) wah_x64_vminmax(f, type, false); break; \
        case WAH_OP_##N##_MAX: WAH_IF_RELAXED_SIMD(case WAH_OP_##N##_RELAXED_MAX:) wah_x64_vminmax(f, type, true); break; \
        /* MINPS b, a gives b only if b < a, as pmin does */ \
        case WAH_OP_##N##_PMIN: wah_x64_vbin(f, WAH_X64_PFP(type) | WAH_X64_MINS, true, false, -1, false, -1); break; \
        case WAH_OP_##N##_PMAX: wah_x64_vbin(f, WAH_X64_PFP(type) | WAH_X64_MAXS, true, false, -1, false, -1); break; \
        WAH_IF_RELAXED_SIMD(case WAH_OP_##N##_RELAXED_MADD: wah_x64_vmadd(f, type, false); break; \
                            case WAH_OP_##N##_RELAXED_NMADD: wah_x64_vmadd(f, type, true); break;)
        WAH_X64_VFP(F32X4, 0)
        WAH_X64_VFP(F64X2, 1)
#undef WAH_X64_VFP

        case WAH_OP_I32X4_TRUNC_SAT_F32X4_S: WAH_IF_RELAXED_SIMD(case WAH_OP_I32X4_RELAXED_TRUNC_F32X4_S:)
            wah_x64_vtrunc_sat(f, 0, true);
            break;
        case WAH_OP_I32X4_TRUNC_SAT_F32X4_U: WAH_IF_RELAXED_SIMD(case WAH_OP_I32X4_RELAXED_TRUNC_F32X4_U:)
            wah_x64_vtrunc_sat(f, 0, false);
            break;
        case WAH_OP_I32X4_TRUNC_SAT_F64X2_S_ZERO: WAH_IF_RELAXED_SIMD(case WAH_OP_I32X4_RELAXED_TRUNC_F64X2_S:)
            wah_x64_vtrunc_sat(f, 1, true);
            break;
        case WAH_OP_I32X4_TRUNC_SAT_F64X2_U_ZERO: WAH_IF_RELAXED_SIMD(case WAH_OP_I32X4_RELAXED_TRUNC_F64X2_U:)
            wah_x64_vtrunc_sat(f, 1, false);
            break;
        case WAH_OP_F32X4_CONVERT_I32X4_S: wah_x64_vun(f, WAH_X64_CVTDQ2PS, -1, -1); break;
        case WAH_OP_F32X4_CONVERT_I32X4_U: wah_x64_vconvert_u(f, false); break;
        case WAH_OP_F64X2_CONVERT_LOW_I32X4_S: wah_x64_vun(f, WAH_X64_CVTDQ2PD, -1, -1); break;
        case WAH_OP_F64X2_CONVERT_LOW_I32X4_U: wah_x64_vconvert_u(f, true); break;
        case WAH_OP_F32X4_DEMOTE_F64X2_ZERO: wah_x64_vun(f, WAH_X64_PFP(1) | WAH_X64_CVTPS2PD, -1, 0); break;
        case WAH_OP_F64X2_PROMOTE_LOW_F32X4: wah_x64_vun(f, WAH_X64_CVTPS2PD, -1, 1); break;

#if ((WAH_COMPILED_FEATURES) & WAH_FEATURE_RELAXED_SIMD)
        case WAH_OP_I16X8_RELAXED_DOT_I8X16_I7X16_S: wah_x64_vdot_i8(f, false); break;
        case WAH_OP_I32X4_RELAXED_DOT_I8X16_I7X16_ADD_S: wah_x64_vdot_i8(f, true); break;
#endif
        default: WAH_JIT_FAIL(f, "unhandled SIMD opcode"); break;
    }
}

#endif // WAH_FEATURE_SIMD

#elif defined(WAH_JIT_AARCH64)

////////////////////////////////////////////////////////////////////////////////
// Code generation (AArch64) ///////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Generated code uses the following registers:
//
// - x19: exec context, x20: frame base
// - x21/x22: base and size of memory 0, reloaded after anything that may grow memories
// - x28-x23, x15-x6 and v15-v8: pinned locals in this order, otherwise the operand stack cache (if below x16)
// - x0-x5, v0-v7 and v16-v29: operand stack cache, x16/x17 and v30/v31: scratch
//
// sp stays at the trampoline frame, which also keeps the FPCR of the host.
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
    if (host) {
        wah_a64_ldst(f, WAH_A64_LDRX, WAH_JR_T1, WAH_A64_SP, WAH_A64_FRAME_OFF(fpcr_host));
        wah_a64(f, 0xD51B4400u | WAH_JR_T1); // MSR FPCR, x17
    }
    wah_a64_call_abs(f, fn);
    if (host) wah_a64_set_fpcr(f);
}

// Returns a register holding the address of globals.
static int wah_jit_globals(wah_jit_fn_t *f) {
    wah_a64_ldst(f, WAH_A64_LDRX, WAH_JR_T1, WAH_JR_CTX, WAH_JIT_CTX_OFF(globals));
    return WAH_JR_T1;
}

static void wah_jit_memory_size(wah_jit_fn_t *f, uint32_t mem_idx) {
    int d = wah_jit_alloc(f, false);
    int src = WAH_JR_MEMSZ;
    if (mem_idx != 0) {
        wah_a64_ldst(f, WAH_A64_LDRX, d, WAH_JR_CTX, WAH_JIT_CTX_OFF(memories));
        wah_a64_ldst(f, WAH_A64_LDRX, d, d, (uint64_t)mem_idx * sizeof(wah_memory_inst_t) +
                                            offsetof(wah_memory_inst_t, size));
        src = d;
    } else if (f->jm->guarded) {
        wah_a64_ldst(f, WAH_A64_LDRX, d, WAH_JR_CTX, WAH_JIT_CTX_OFF(memory_size));
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
    int b = -1;
    if (wah_a64_top_imm12(f, sf, &imm, &neg)) {
        f->h--;
        a = wah_jit_pop_gpr_ro(f);
        wah_a64_addsub_imm(f, neg ? WAH_A64_ADDSI : WAH_A64_SUBSI, sf, WAH_A64_ZR, a, imm, false);
    } else {
        b = wah_jit_pop_gpr_ro(f);
        a = wah_jit_pop_gpr_ro(f);
        wah_a64_rrr(f, WAH_A64_SUBS, sf, WAH_A64_ZR, a, b);
    }
    if (fuse) {
        wah_jit_free_gpr(f, a);
        if (b >= 0) wah_jit_free_gpr(f, b);
        wah_jit_push(f, (wah_jit_val_t){ .kind = WAH_JV_FLAGS, .reg = (uint8_t)cond });
    } else {
        int d = wah_jit_dest(f, false, a, b);
        wah_a64_cset(f, d, cond);
        wah_jit_finish_gpr(f, d, a, b);
    }
}

static void wah_jit_fcmp(wah_jit_fn_t *f, int type, int kind, bool fuse) {
    static const int conds[] = { WAH_A64_EQ, WAH_A64_NE, WAH_A64_MI, WAH_A64_GT, WAH_A64_LS, WAH_A64_GE };
    int b = wah_jit_pop_fpr_ro(f), a = wah_jit_pop_fpr_ro(f);
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
        int a = wah_jit_pop_gpr_ro(f), d = wah_jit_dest(f, false, a, -1);
        wah_a64_addsub_imm(f, (kind == WAH_JI_SUB) != neg ? WAH_A64_SUBI : WAH_A64_ADDI, sf, d, a, imm, false);
        wah_jit_finish_gpr(f, d, a, -1);
        return;
    }
    int b = wah_jit_pop_gpr_ro(f), a = wah_jit_pop_gpr_ro(f), d = wah_jit_dest(f, false, a, b);
    wah_a64_rrr(f, ops[kind], sf, d, a, b);
    wah_jit_finish_gpr(f, d, a, b);
}

static void wah_jit_iunop(wah_jit_fn_t *f, bool sf, int kind) {
    int a = wah_jit_pop_gpr_ro(f), d = wah_jit_dest(f, false, a, -1);
    switch (kind) {
        case WAH_JI_CTZ:
            wah_a64_rrr(f, WAH_A64_RBIT, sf, d, a, 0);
            wah_a64_rrr(f, WAH_A64_CLZ, sf, d, d, 0);
            break;
        case WAH_JI_CLZ: wah_a64_rrr(f, WAH_A64_CLZ, sf, d, a, 0); break;
        case WAH_JI_POPCNT:
            wah_a64_fmov_from_gp(f, sf, WAH_JR_VT1, a);
            wah_a64(f, 0x0E205800u | (uint32_t)WAH_JR_VT1 << 5 | WAH_JR_VT1); // CNT v31.8b
            wah_a64(f, 0x0E31B800u | (uint32_t)WAH_JR_VT1 << 5 | WAH_JR_VT1); // ADDV b31, v31.8b
            wah_a64_fmov_to_gp(f, false, d, WAH_JR_VT1);
            break;
    }
    wah_jit_finish_gpr(f, d, a, -1);
}

static void wah_jit_shift(wah_jit_fn_t *f, bool sf, int kind) {
    uint32_t bits = sf ? 64 : 32;
    const wah_jit_val_t *top = &f->vals[f->h - 1];
    if (top->kind == WAH_JV_CONST) {
        uint32_t s = (uint32_t)top->imm & (bits - 1);
        f->h--;
        int a = wah_jit_pop_gpr_ro(f), d = wah_jit_dest(f, false, a, -1);
        switch (s == 0 ? -1 : kind) {
            case -1: if (d != a) wah_jit_mov64(f, d, a); break;
            case WAH_JS_SHL: wah_a64_bfm(f, false, sf, d, a, (bits - s) & (bits - 1), bits - 1 - s); break;
            case WAH_JS_SHR_S: wah_a64_bfm(f, true, sf, d, a, s, bits - 1); break;
            case WAH_JS_SHR_U: wah_a64_bfm(f, false, sf, d, a, s, bits - 1); break;
            case WAH_JS_ROTL: wah_a64_extr(f, sf, d, a, a, bits - s); break;
            case WAH_JS_ROTR: wah_a64_extr(f, sf, d, a, a, s); break;
        }
        wah_jit_finish_gpr(f, d, a, -1);
        return;
    }
    int b = wah_jit_pop_gpr_ro(f), a = wah_jit_pop_gpr_ro(f), d = wah_jit_dest(f, false, a, b);
    switch (kind) {
        case WAH_JS_SHL: wah_a64_rrr(f, WAH_A64_LSLV, sf, d, a, b); break;
        case WAH_JS_SHR_S: wah_a64_rrr(f, WAH_A64_ASRV, sf, d, a, b); break;
        case WAH_JS_SHR_U: wah_a64_rrr(f, WAH_A64_LSRV, sf, d, a, b); break;
        case WAH_JS_ROTL:
            wah_a64_rrr(f, WAH_A64_SUB, false, WAH_JR_T0, WAH_A64_ZR, b);
            wah_a64_rrr(f, WAH_A64_RORV, sf, d, a, WAH_JR_T0);
            break;
        case WAH_JS_ROTR: wah_a64_rrr(f, WAH_A64_RORV, sf, d, a, b); break;
    }
    wah_jit_finish_gpr(f, d, a, b);
}

static void wah_jit_divrem(wah_jit_fn_t *f, bool sf, int kind) {
    const wah_jit_val_t *top = &f->vals[f->h - 1];
    bool safe = top->kind == WAH_JV_CONST && (sf ? top->imm : (int32_t)top->imm) != 0 &&
                ((sf ? top->imm : (int32_t)top->imm) != -1 || kind != WAH_JD_DIV_S);
    int b = wah_jit_pop_gpr_ro(f), a = wah_jit_pop_gpr_ro(f);
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
    int d = wah_jit_dest(f, false, a, b);
    if (kind == WAH_JD_DIV_S || kind == WAH_JD_DIV_U) {
        wah_a64_rrr(f, div, sf, d, a, b);
    } else { // INT_MIN % -1 is 0 as sdiv gives INT_MIN
        wah_a64_rrr(f, div, sf, WAH_JR_T0, a, b);
        wah_a64_msub(f, sf, d, WAH_JR_T0, b, a);
    }
    wah_jit_finish_gpr(f, d, a, b);
}

static void wah_jit_fbinop(wah_jit_fn_t *f, int type, int kind) {
    static const uint32_t ops[] = { WAH_A64_FADD, WAH_A64_FSUB, WAH_A64_FMUL, WAH_A64_FDIV, WAH_A64_FMIN, WAH_A64_FMAX };
    int b = wah_jit_pop_fpr_ro(f), a = wah_jit_pop_fpr_ro(f), d = wah_jit_dest(f, true, a, b);
    wah_a64_fp2(f, ops[kind], type, d, a, b);
    wah_jit_finish_fpr(f, d, a, b);
}

static void wah_jit_funop(wah_jit_fn_t *f, int type, int kind) {
    static const uint32_t ops[] = {
        WAH_A64_FABS, WAH_A64_FNEG, WAH_A64_FRINTP, WAH_A64_FRINTM, WAH_A64_FRINTZ, WAH_A64_FRINTN, WAH_A64_FSQRT,
    };
    int a = wah_jit_pop_fpr_ro(f), d = wah_jit_dest(f, true, a, -1);
    wah_a64_fp2(f, ops[kind], type, d, a, 0);
    wah_jit_finish_fpr(f, d, a, -1);
}

static void wah_jit_copysign(wah_jit_fn_t *f, int type) {
    int b = wah_jit_pop_fpr_ro(f), a = wah_jit_pop_fpr(f);
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
    int a = wah_jit_pop_gpr_ro(f), d = wah_jit_dest(f, false, a, -1);
    if (is_signed) wah_a64_bfm(f, true, sf, d, a, 0, (uint32_t)bits - 1);
    else wah_a64_mov(f, false, d, a);
    wah_jit_finish_gpr(f, d, a, -1);
}

static void wah_jit_zext_top(wah_jit_fn_t *f) { (void)f; }

// Trapping float to integer conversion: lo/hi are exclusive or inclusive bounds as described by lo_ge.
static void wah_jit_trunc(wah_jit_fn_t *f, int type, bool sf, bool is_signed, double lo, bool lo_ge, double hi) {
    int a = wah_jit_pop_fpr_ro(f);
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

#if ((WAH_COMPILED_FEATURES) & WAH_FEATURE_NONTRAPPING_F2I)
static void wah_jit_trunc_sat(wah_jit_fn_t *f, int type, bool sf, bool is_signed) {
    int a = wah_jit_pop_fpr_ro(f), d = wah_jit_alloc(f, false);
    wah_a64_fcvt_gp(f, is_signed ? WAH_A64_FCVTZS : WAH_A64_FCVTZU, sf, type, d, a);
    wah_jit_free_fpr(f, a);
    wah_jit_push_gpr(f, d);
}
#endif

static void wah_jit_convert(wah_jit_fn_t *f, int type, bool sf, bool is_signed) {
    int a = wah_jit_pop_gpr_ro(f), d = wah_jit_alloc(f, true);
    wah_a64_fcvt_gp(f, is_signed ? WAH_A64_SCVTF : WAH_A64_UCVTF, sf, type, d, a);
    wah_jit_free_gpr(f, a);
    wah_jit_push_fpr(f, d);
}

// Effective address of an access: base + uxtw(index) if index >= 0, x16 + imm if imm_form, otherwise base + x16.
// base is also the register to free if it is not WAH_JR_MEM.
typedef struct { int base, index; bool imm_form; uint32_t imm; } wah_a64_addr_t;

// Loads or stores rt by op (in its unsigned offset form) at the address.
static void wah_a64_access(wah_jit_fn_t *f, uint32_t op, int rt, const wah_a64_addr_t *ad) {
    if (ad->index >= 0) {
        wah_a64(f, (op & ~0x01000000u) | 0x00204800u | (uint32_t)ad->index << 16 | (uint32_t)ad->base << 5 | (uint32_t)rt);
    } else if (ad->imm_form) {
        wah_a64_ldst(f, op, rt, WAH_JR_T0, ad->imm);
    } else {
        wah_a64_ldst_reg(f, op, rt, ad->base, WAH_JR_T0);
    }
}

// Bounds-checks the address on the stack, which is base + x16 unless fold allows other forms without checks.
// The index register is already freed, so it may be the destination of the access as long as it is read first.
static wah_a64_addr_t wah_jit_address(wah_jit_fn_t *f, const wah_memarg_t *m, uint32_t size, int *size_reg, bool fold) {
    int a = wah_jit_pop_gpr_ro(f);
    int base = WAH_JR_MEM, limit = WAH_JR_MEMSZ;
    bool mem64 = wah_jit_mem64(f, m->memidx);
    uint64_t end = wah_jit_access_end(m, size);
    bool check = mem64 || !f->jm->guarded || end > WAH_JIT_GUARD_SIZE;
    *size_reg = -1;
    if (m->memidx != 0 && (base = wah_jit_cached_base(f, m->memidx)) >= 0) {
        if (check) {
            limit = wah_jit_alloc(f, false);
            *size_reg = limit;
            wah_a64_ldst(f, WAH_A64_LDRX, limit, WAH_JR_CTX, WAH_JIT_CTX_OFF(memories));
            wah_a64_ldst(f, WAH_A64_LDRX, limit, limit, (uint64_t)m->memidx * sizeof(wah_memory_inst_t) +
                                                        offsetof(wah_memory_inst_t, size));
        }
    } else if (m->memidx != 0) {
        base = wah_jit_alloc(f, false);
        uint64_t off = (uint64_t)m->memidx * sizeof(wah_memory_inst_t);
        wah_a64_ldst(f, WAH_A64_LDRX, base, WAH_JR_CTX, WAH_JIT_CTX_OFF(memories));
        if (check) {
            limit = wah_jit_alloc(f, false);
            *size_reg = limit;
            wah_a64_ldst(f, WAH_A64_LDRX, limit, base, off + offsetof(wah_memory_inst_t, size));
        }
        wah_a64_ldst(f, WAH_A64_LDRX, base, base, off + offsetof(wah_memory_inst_t, data));
        if (f->jm->guarded && !mem64) f->gpr_local[base] = (int32_t)(f->nlocals + m->memidx);
    } else if (check && f->jm->guarded) { // The size register may be pinned
        limit = wah_jit_alloc(f, false);
        *size_reg = limit;
        wah_a64_ldst(f, WAH_A64_LDRX, limit, WAH_JR_CTX, WAH_JIT_CTX_OFF(memory_size));
    }
    wah_a64_addr_t ad = { .base = base, .index = -1 };
    if (mem64) { // a + end may wrap around, which sets the carry
        if (end < 4096) {
            wah_a64_addsub_imm(f, WAH_A64_ADDSI, true, WAH_JR_T1, a, (uint32_t)end, false);
        } else {
            wah_a64_mov_imm(f, true, WAH_JR_T1, end);
            wah_a64_rrr(f, WAH_A64_ADDS, true, WAH_JR_T1, WAH_JR_T1, a);
        }
        wah_jit_bcond(f, WAH_A64_HS, f->l_oob);
        wah_a64_rrr(f, WAH_A64_SUBS, true, WAH_A64_ZR, WAH_JR_T1, limit);
        wah_jit_bcond(f, WAH_A64_HI, f->l_oob);
        if (m->offset < 4096) {
            wah_a64_addsub_imm(f, WAH_A64_ADDI, true, WAH_JR_T0, a, (uint32_t)m->offset, false);
        } else {
            wah_a64_mov_imm(f, true, WAH_JR_T0, m->offset);
            wah_a64_rrr(f, WAH_A64_ADD, true, WAH_JR_T0, WAH_JR_T0, a);
        }
        wah_jit_free_gpr(f, a);
        return ad;
    }
    if (fold && !check && m->offset == 0) {
        ad.index = a;
        wah_jit_free_gpr(f, a);
        return ad;
    }
    if (fold && !check && m->offset % size == 0 && m->offset / size < 4096) {
        wah_a64(f, 0x8B204000u | (uint32_t)a << 16 | (uint32_t)base << 5 | WAH_JR_T0); // ADD x16, base, wA, UXTW
        wah_jit_free_gpr(f, a);
        ad.imm_form = true;
        ad.imm = m->offset;
        return ad;
    }
    if (m->offset < ((uint64_t)1 << 24)) {
        wah_a64_mov(f, false, WAH_JR_T0, a); // Zero-extended
        wah_a64_add_imm(f, WAH_JR_T0, WAH_JR_T0, m->offset); // Fits in 33 bits, so no overflow
    } else { // wah_a64_add_imm would use x16 as a scratch
        wah_a64_mov_imm(f, true, WAH_JR_T0, m->offset);
        wah_a64(f, 0x8B204000u | (uint32_t)a << 16 | (uint32_t)WAH_JR_T0 << 5 | WAH_JR_T0); // ADD x16, x16, wA, UXTW
    }
    wah_jit_free_gpr(f, a);
    if (check) {
        wah_a64_addsub_imm(f, WAH_A64_ADDI, true, WAH_JR_T1, WAH_JR_T0, size, false);
        wah_a64_rrr(f, WAH_A64_SUBS, true, WAH_A64_ZR, WAH_JR_T1, limit);
        wah_jit_bcond(f, WAH_A64_HI, f->l_oob);
    }
    return ad;
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
    wah_a64_addr_t ad = wah_jit_address(f, m, 1u << size_log2, &size_reg, true);
    if (size_reg >= 0) wah_jit_free_gpr(f, size_reg);
    if (ad.base != WAH_JR_MEM) wah_jit_free_gpr(f, ad.base);
    int d = !fp && f->hint >= 0 ? f->hint : wah_jit_alloc(f, fp);
    wah_a64_access(f, op, d, &ad);
    if (fp) wah_jit_push_fpr(f, d);
    else wah_jit_push_gpr(f, d);
}

static void wah_jit_store(wah_jit_fn_t *f, const wah_memarg_t *m, uint32_t size_log2, bool fp_ok) {
    static const uint32_t ops[] = { WAH_A64_STRB, WAH_A64_STRH, WAH_A64_STRW, WAH_A64_STRX };
    wah_jit_val_t *v = &f->vals[f->h - 1];
    int r, fp = 0;
    if ((v->kind == WAH_JV_FPR || v->kind == WAH_JV_LFPR) && fp_ok) {
        r = wah_jit_pop_fpr_ro(f);
        fp = 1;
    } else if (v->kind == WAH_JV_CONST && v->imm == 0) {
        f->h--;
        r = WAH_A64_ZR;
    } else {
        r = wah_jit_pop_gpr_ro(f);
    }
    int size_reg;
    wah_a64_addr_t ad = wah_jit_address(f, m, 1u << size_log2, &size_reg, true);
    wah_a64_access(f, fp ? (size_log2 == 3 ? WAH_A64_STRD : WAH_A64_STRS) : ops[size_log2], r, &ad);
    if (size_reg >= 0) wah_jit_free_gpr(f, size_reg);
    if (ad.base != WAH_JR_MEM) wah_jit_free_gpr(f, ad.base);
    if (fp) wah_jit_free_fpr(f, r);
    else if (r != WAH_A64_ZR) wah_jit_free_gpr(f, r);
}

static void wah_jit_select(wah_jit_fn_t *f) {
    wah_jit_cond_t c = wah_jit_pop_cond(f);
    bool fp = wah_jit_is_fpr(f->vals[f->h - 1].kind) || wah_jit_is_fpr(f->vals[f->h - 2].kind);
    int b, a;
    if (fp) { b = wah_jit_pop_fpr_ro(f); a = wah_jit_pop_fpr(f); }
    else { b = wah_jit_pop_gpr_ro(f); a = wah_jit_pop_gpr(f); }
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
    uint32_t resume = s->resume;
    if (s->poll) { // Pinned locals are kept in slots while suspended, and restored when resumed or continued
        uint32_t suspend = wah_jit_new_label(f);
        wah_jit_sync_pins(f, true);
        wah_jit_call_helper(f, (uintptr_t)wah_jit_poll, (const wah_jit_arg_t[]){ { WAH_JA_CTX, 0 } }, 1, false);
        wah_jit_cbz(f, true, false, WAH_JR_RET, suspend);
        wah_jit_sync_pins(f, false);
        wah_jit_b(f, s->cont);
        if (f->npins > 0) {
            resume = wah_jit_new_label(f);
            wah_jit_bind(f, resume);
            wah_jit_sync_pins(f, false);
            wah_jit_b(f, s->resume);
        }
        wah_jit_bind(f, suspend);
    }
    // w0 is a status to suspend with, or an error
    wah_a64_addsub_imm(f, WAH_A64_SUBSI, false, WAH_A64_ZR, WAH_JR_RET, 0, false);
    wah_jit_bcond(f, WAH_A64_LT, f->l_err);
    wah_a64_ldst(f, WAH_A64_LDRX, WAH_JR_T0, WAH_JR_CTX, WAH_JIT_CTX_OFF(jit));
    wah_jit_adr(f, WAH_JR_T1, resume);
    wah_a64_ldst(f, WAH_A64_STRX, WAH_JR_T1, WAH_JR_T0, offsetof(wah_jit_ctx_t, resume_pc));
    wah_a64_ldst(f, WAH_A64_STRX, WAH_JR_FP, WAH_JR_T0, offsetof(wah_jit_ctx_t, resume_fp));
    wah_jit_exit(f);
}

// Checks the stack and saves the return address. Declared locals are zeroed by the caller.
static void wah_jit_prologue(wah_jit_fn_t *f) {
    wah_a64_add_imm(f, WAH_JR_T1, WAH_JR_FP, f->frame_bytes);
    wah_a64_ldst(f, WAH_A64_LDRX, WAH_JR_T0, WAH_JR_CTX, WAH_JIT_CTX_OFF(frame_ptr)); // The limit
    wah_a64_rrr(f, WAH_A64_SUBS, true, WAH_A64_ZR, WAH_JR_T1, WAH_JR_T0);
    wah_jit_bcond(f, WAH_A64_HI, f->l_stack);
    wah_a64_ldst(f, WAH_A64_STRX, 30, WAH_JR_FP, f->link_off);
}

// Zeroes n slots from [fp + off].
static void wah_jit_zero_loop(wah_jit_fn_t *f, uint32_t off, uint32_t n) {
    uint32_t loop = wah_jit_new_label(f);
    wah_a64_add_imm(f, WAH_JR_T0, WAH_JR_FP, off);
    wah_a64_mov_imm(f, false, WAH_JR_T1, n);
    wah_jit_bind(f, loop);
    wah_a64(f, 0xA8800000u | 2u << 15 | WAH_A64_ZR << 10 | (uint32_t)WAH_JR_T0 << 5 | WAH_A64_ZR); // STP xzr, xzr, [x16], #16
    wah_a64_addsub_imm(f, WAH_A64_SUBSI, false, WAH_JR_T1, WAH_JR_T1, 1, false);
    wah_jit_bcond(f, WAH_A64_NE, loop);
}

#if ((WAH_COMPILED_FEATURES) & WAH_FEATURE_SIMD)

// SIMD (AArch64) ///////////////////////////////////////////////////////////////

// Advanced SIMD encodings on 128-bit vectors unless noted, where the element size goes to bits 22-23 (WAH_A64_SZ).
// For FP instructions, bit 22 is 0 for single and 1 for double precision.
#define WAH_A64_SZ(size_log2) ((uint32_t)(size_log2) << 22)
#define WAH_A64_Q 0x40000000u // The upper half for instructions working on 64-bit halves (the "2" variants)
// Three registers of the same type
#define WAH_A64_V_ADD     0x4E208400u
#define WAH_A64_V_SUB     0x6E208400u
#define WAH_A64_V_MUL     0x4E209C00u
#define WAH_A64_V_SQADD   0x4E200C00u
#define WAH_A64_V_UQADD   0x6E200C00u
#define WAH_A64_V_SQSUB   0x4E202C00u
#define WAH_A64_V_UQSUB   0x6E202C00u
#define WAH_A64_V_URHADD  0x6E201400u
#define WAH_A64_V_CMGT    0x4E203400u
#define WAH_A64_V_CMHI    0x6E203400u
#define WAH_A64_V_CMGE    0x4E203C00u
#define WAH_A64_V_CMHS    0x6E203C00u
#define WAH_A64_V_CMEQ    0x6E208C00u
#define WAH_A64_V_SSHL    0x4E204400u
#define WAH_A64_V_USHL    0x6E204400u
#define WAH_A64_V_SMAX    0x4E206400u
#define WAH_A64_V_UMAX    0x6E206400u
#define WAH_A64_V_SMIN    0x4E206C00u
#define WAH_A64_V_UMIN    0x6E206C00u
#define WAH_A64_V_ADDP    0x4E20BC00u
#define WAH_A64_V_SQRDMULH 0x6E20B400u
#define WAH_A64_V_AND     0x4E201C00u
#define WAH_A64_V_BIC     0x4E601C00u
#define WAH_A64_V_ORR     0x4EA01C00u
#define WAH_A64_V_EOR     0x6E201C00u
#define WAH_A64_V_BSL     0x6E601C00u // d = (n & d) | (m & ~d)
#define WAH_A64_V_BIT     0x6EA01C00u // d = (n & m) | (d & ~m)
#define WAH_A64_V_BIF     0x6EE01C00u // d = (d & m) | (n & ~m)
#define WAH_A64_V_FADD    0x4E20D400u
#define WAH_A64_V_FSUB    0x4EA0D400u
#define WAH_A64_V_FMUL    0x6E20DC00u
#define WAH_A64_V_FDIV    0x6E20FC00u
#define WAH_A64_V_FMAX    0x4E20F400u
#define WAH_A64_V_FMIN    0x4EA0F400u
#define WAH_A64_V_FCMEQ   0x4E20E400u
#define WAH_A64_V_FCMGE   0x6E20E400u
#define WAH_A64_V_FCMGT   0x6EA0E400u
// Two registers, where those working on halves are 64-bit and write zeroes to the upper half
#define WAH_A64_V_SADDLP  0x4E202800u
#define WAH_A64_V_UADDLP  0x6E202800u
#define WAH_A64_V_SADALP  0x4E206800u
#define WAH_A64_V_CNT     0x4E205800u
#define WAH_A64_V_NOT     0x6E205800u
#define WAH_A64_V_CMEQ0   0x4E209800u
#define WAH_A64_V_ABS     0x4E20B800u
#define WAH_A64_V_NEG     0x6E20B800u
#define WAH_A64_V_SQXTN   0x0E214800u // The size is of the narrow elements
#define WAH_A64_V_UQXTN   0x2E214800u
#define WAH_A64_V_SQXTUN  0x2E212800u
#define WAH_A64_V_FCVTN   0x0E616800u // From double
#define WAH_A64_V_FCVTL   0x0E617800u // To double
#define WAH_A64_V_FRINTN  0x4E218800u
#define WAH_A64_V_FRINTM  0x4E219800u
#define WAH_A64_V_FRINTP  0x4EA18800u
#define WAH_A64_V_FRINTZ  0x4EA19800u
#define WAH_A64_V_FCVTZS  0x4EA1B800u
#define WAH_A64_V_FCVTZU  0x6EA1B800u
#define WAH_A64_V_SCVTF   0x4E21D800u
#define WAH_A64_V_UCVTF   0x6E21D800u
#define WAH_A64_V_FABS    0x4EA0F800u
#define WAH_A64_V_FNEG    0x6EA0F800u
#define WAH_A64_V_FSQRT   0x6EA1F800u
#define WAH_A64_V_UMAXV   0x6E30A800u
#define WAH_A64_V_UMINV   0x6E31A800u
#define WAH_A64_V_ADDV    0x4E31B800u
// Long results from lower halves (or upper halves with WAH_A64_Q), the size is of the source elements
#define WAH_A64_V_SMULL   0x0E20C000u
#define WAH_A64_V_UMULL   0x2E20C000u
#define WAH_A64_V_SSHLL   0x0F00A400u // Shift amount (0) plus the element size in bits 16-22
#define WAH_A64_V_USHLL   0x2F00A400u
// Permutations
#define WAH_A64_V_UZP1    0x4E001800u
#define WAH_A64_V_UZP2    0x4E005800u
#define WAH_A64_V_ZIP1    0x4E003800u
#define WAH_A64_V_TBL1    0x4E000000u // Table {n}, indices m
#define WAH_A64_V_TBL2    0x4E002000u // Table {n, n + 1}, indices m
#define WAH_A64_V_EXT     0x6E000000u // Byte offset in bits 11-14
// Shifts by an immediate in bits 16-22, which is the element size plus the amount for SHL, or twice the element
// size minus the amount for SSHR/USHR
#define WAH_A64_V_SSHR    0x4F000400u
#define WAH_A64_V_USHR    0x6F000400u
#define WAH_A64_V_SHL     0x4F005400u
// Element moves with imm5 in bits 16-20
#define WAH_A64_V_DUP_ELEM 0x4E000400u
#define WAH_A64_V_DUP_GEN  0x4E000C00u
#define WAH_A64_V_INS_GEN  0x4E001C00u
#define WAH_A64_V_INS_ELEM 0x6E000400u // Source index in bits 11-14
#define WAH_A64_V_SMOV     0x0E002C00u // To w registers
#define WAH_A64_V_UMOV     0x0E003C00u // To w registers, or x registers with WAH_A64_Q
#define WAH_A64_S_DUP_ELEM 0x5E000400u // To a scalar register

static void wah_a64_v3(wah_jit_fn_t *f, uint32_t insn, int d, int n, int m) {
    wah_a64(f, insn | (uint32_t)m << 16 | (uint32_t)n << 5 | (uint32_t)d);
}
static void wah_a64_v2(wah_jit_fn_t *f, uint32_t insn, int d, int n) { wah_a64_v3(f, insn, d, n, 0); }

// imm5 of element moves, for lane i of 1 << size_log2 bytes
static uint32_t wah_a64_imm5(uint32_t size_log2, uint32_t i) { return (i << (size_log2 + 1) | 1u << size_log2) << 16; }

// LD1/ST1 of a single lane, which only takes an address register
static void wah_a64_ldst_lane(wah_jit_fn_t *f, bool load, uint32_t size_log2, uint32_t lane, int v, int rn) {
    uint32_t q, s, size, opcode;
    switch (size_log2) {
        case 0: q = lane >> 3; s = lane >> 2 & 1; size = lane & 3; opcode = 0; break;
        case 1: q = lane >> 2; s = lane >> 1 & 1; size = (lane & 1) << 1; opcode = 2; break;
        case 2: q = lane >> 1; s = lane & 1; size = 0; opcode = 4; break;
        default: q = lane; s = 0; size = 1; opcode = 4; break;
    }
    wah_a64(f, (load ? 0x0D400000u : 0x0D000000u) | q << 30 | opcode << 13 | s << 12 | size << 10 |
               (uint32_t)rn << 5 | (uint32_t)v);
}

static bool wah_jit_simd_supported(const wah_jit_module_t *jm) { (void)jm; return true; }

static void wah_jit_load_vconst(wah_jit_fn_t *f, int d, const uint8_t *bytes) {
    static const uint8_t zeroes[16] = { 0 }, ones[16] = { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 };
    if (memcmp(bytes, zeroes, 16) == 0) { wah_a64(f, 0x6F00E400u | (uint32_t)d); return; } // MOVI v.2d, #0
    if (memcmp(bytes, ones, 16) == 0) { wah_a64(f, 0x6F07E7E0u | (uint32_t)d); return; }   // MOVI v.2d, #-1
    if (f->long_branches) { // Placed inline, as the end of the function may be out of range
        wah_a64(f, 0x9C000000u | 2u << 5 | (uint32_t)d); // LDR q, #8
        wah_a64(f, 0x14000005u); // B #20
        wah_jit_emit(f, bytes, 16);
        return;
    }
    uint32_t label = wah_jit_vconst_label(f, bytes);
    wah_jit_patch_here(f, WAH_JP_A64_B19, label, 0);
    wah_a64(f, 0x9C000000u | (uint32_t)d); // LDR q, label
}

static void wah_jit_vselect(wah_jit_fn_t *f) {
    wah_jit_cond_t c = wah_jit_pop_cond(f);
    int b = wah_jit_pop_vec_ro(f), a = wah_jit_pop_vec(f);
    uint32_t keep = wah_jit_new_label(f);
    wah_jit_branch_if(f, c, true, keep);
    wah_jit_vmov(f, a, b);
    wah_jit_bind(f, keep);
    wah_jit_release_cond(f, c);
    wah_jit_finish_vec(f, a, a, b, -1);
}

// d = a op b (or b op a if swapped), negated if `not`
static void wah_a64_vbin(wah_jit_fn_t *f, uint32_t insn, bool swap, bool not) {
    int b = wah_jit_pop_vec_ro(f), a = wah_jit_pop_vec_ro(f), d = wah_jit_dest(f, true, a, b);
    wah_a64_v3(f, insn, d, swap ? b : a, swap ? a : b);
    if (not) wah_a64_v2(f, WAH_A64_V_NOT, d, d);
    wah_jit_finish_vec(f, d, a, b, -1);
}

static void wah_a64_vun(wah_jit_fn_t *f, uint32_t insn) {
    int a = wah_jit_pop_vec_ro(f), d = wah_jit_dest(f, true, a, -1);
    wah_a64_v2(f, insn, d, a);
    wah_jit_finish_vec(f, d, a, -1, -1);
}

// Two instructions on halves, the second one writing the upper half: d = insn(a) : insn2(b) or insn(a) : insn2(a)
static void wah_a64_vhalves(wah_jit_fn_t *f, uint32_t insn, bool binary) {
    int b = binary ? wah_jit_pop_vec_ro(f) : -1, a = wah_jit_pop_vec_ro(f);
    int d = f->fpr[a] == WAH_JO_TEMP ? a : wah_jit_alloc(f, true); // Not b, which is read after d is written
    wah_a64_v2(f, insn, d, a);
    wah_a64_v2(f, insn | WAH_A64_Q, d, binary ? b : a);
    wah_jit_finish_vec(f, d, a, b, -1);
}

// Long multiplication of lower or upper halves
static void wah_a64_vmull(wah_jit_fn_t *f, uint32_t insn) {
    int b = wah_jit_pop_vec_ro(f), a = wah_jit_pop_vec_ro(f), d = wah_jit_dest(f, true, a, b);
    wah_a64_v3(f, insn, d, a, b);
    wah_jit_finish_vec(f, d, a, b, -1);
}

// Shifts by the i32 on top of the stack, modulo the lane size; kind is WAH_JS_SHL, WAH_JS_SHR_S or WAH_JS_SHR_U.
static void wah_a64_vshift(wah_jit_fn_t *f, uint32_t size_log2, int kind) {
    uint32_t bits = 8u << size_log2;
    const wah_jit_val_t *top = &f->vals[f->h - 1];
    if (top->kind == WAH_JV_CONST) {
        uint32_t s = (uint32_t)top->imm & (bits - 1);
        f->h--;
        int a = wah_jit_pop_vec_ro(f), d = wah_jit_dest(f, true, a, -1);
        if (s == 0) {
            if (d != a) wah_jit_vmov(f, d, a);
        } else if (kind == WAH_JS_SHL) {
            wah_a64_v2(f, WAH_A64_V_SHL | (bits + s) << 16, d, a);
        } else {
            wah_a64_v2(f, (kind == WAH_JS_SHR_S ? WAH_A64_V_SSHR : WAH_A64_V_USHR) | (2 * bits - s) << 16, d, a);
        }
        wah_jit_finish_vec(f, d, a, -1, -1);
        return;
    }
    int n = wah_jit_pop_gpr_ro(f), a = wah_jit_pop_vec_ro(f), d = wah_jit_dest(f, true, a, -1);
    wah_a64(f, 0x12000000u | (size_log2 + 2) << 10 | (uint32_t)n << 5 | WAH_JR_T0); // AND w16, wn, #(bits - 1)
    if (kind != WAH_JS_SHL) wah_a64_rrr(f, WAH_A64_SUB, false, WAH_JR_T0, WAH_A64_ZR, WAH_JR_T0); // Shifted right if negative
    wah_a64_v2(f, WAH_A64_V_DUP_GEN | wah_a64_imm5(size_log2, 0), WAH_JR_VT0, WAH_JR_T0);
    wah_a64_v3(f, (kind == WAH_JS_SHR_S ? WAH_A64_V_SSHL : WAH_A64_V_USHL) | WAH_A64_SZ(size_log2), d, a, WAH_JR_VT0);
    wah_jit_free_gpr(f, n);
    wah_jit_finish_vec(f, d, a, -1, -1);
}

// Pushes the condition of the comparison with zero which has just set flags, as flags or a register.
static void wah_a64_push_cond(wah_jit_fn_t *f, int cond, bool fuse) {
    if (fuse) {
        wah_jit_push(f, (wah_jit_val_t){ .kind = WAH_JV_FLAGS, .reg = (uint8_t)cond });
    } else {
        int d = wah_jit_alloc(f, false);
        wah_a64_cset(f, d, cond);
        wah_jit_push_gpr(f, d);
    }
}

// any_true (size_log2 < 0) and all_true: reduces into VT0 and compares with zero.
static void wah_a64_vtest(wah_jit_fn_t *f, int size_log2, bool fuse) {
    int a = wah_jit_pop_vec_ro(f);
    int cond = WAH_A64_NE;
    if (size_log2 < 0) {
        wah_a64_v2(f, WAH_A64_V_UMAXV | WAH_A64_SZ(2), WAH_JR_VT0, a);
    } else if (size_log2 < 3) {
        wah_a64_v2(f, WAH_A64_V_UMINV | WAH_A64_SZ(size_log2), WAH_JR_VT0, a);
    } else { // All lanes are nonzero if no lane equals zero
        wah_a64_v2(f, WAH_A64_V_CMEQ0 | WAH_A64_SZ(3), WAH_JR_VT0, a);
        wah_a64_v2(f, WAH_A64_V_UMAXV | WAH_A64_SZ(2), WAH_JR_VT0, WAH_JR_VT0);
        cond = WAH_A64_EQ;
    }
    wah_jit_free_fpr(f, a);
    wah_a64_fmov_to_gp(f, false, WAH_JR_T0, WAH_JR_VT0);
    wah_a64_addsub_imm(f, WAH_A64_SUBSI, false, WAH_A64_ZR, WAH_JR_T0, 0, false);
    wah_a64_push_cond(f, cond, fuse);
}

static void wah_a64_vbitmask(wah_jit_fn_t *f, uint32_t size_log2) {
    static const uint8_t bits8[16] = { 1, 2, 4, 8, 16, 32, 64, 128, 1, 2, 4, 8, 16, 32, 64, 128 };
    static const uint16_t bits16[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };
    static const uint32_t bits32[4] = { 1, 2, 4, 8 };
    uint32_t lane_bits = 8u << size_log2;
    int a = wah_jit_pop_vec_ro(f);
    int d = wah_jit_alloc(f, false);
    if (size_log2 == 3) {
        wah_a64_v2(f, WAH_A64_V_USHR | (128 - 63) << 16, WAH_JR_VT0, a);
        wah_a64_fmov_to_gp(f, true, d, WAH_JR_VT0);
        wah_a64_v2(f, WAH_A64_V_UMOV | WAH_A64_Q | wah_a64_imm5(3, 1), WAH_JR_T0, WAH_JR_VT0);
        wah_a64(f, 0xAA000000u | (uint32_t)WAH_JR_T0 << 16 | 1u << 10 | (uint32_t)d << 5 | (uint32_t)d); // ORR d, d, x16, LSL #1
    } else {
        uint8_t mask[16];
        if (size_log2 == 0) memcpy(mask, bits8, 16);
        else if (size_log2 == 1) { memcpy(mask, bits16, 16); }
        else memcpy(mask, bits32, 16);
        wah_a64_v2(f, WAH_A64_V_SSHR | (2 * lane_bits - (lane_bits - 1)) << 16, WAH_JR_VT0, a); // All ones if negative
        wah_jit_load_vconst(f, WAH_JR_VT1, mask);
        wah_a64_v3(f, WAH_A64_V_AND, WAH_JR_VT0, WAH_JR_VT0, WAH_JR_VT1);
        if (size_log2 == 0) { // Bits of upper bytes go to upper halves of 16-bit lanes
            wah_a64_v3(f, WAH_A64_V_EXT | 8u << 11, WAH_JR_VT1, WAH_JR_VT0, WAH_JR_VT0);
            wah_a64_v3(f, WAH_A64_V_ZIP1, WAH_JR_VT0, WAH_JR_VT0, WAH_JR_VT1);
        }
        wah_a64_v2(f, WAH_A64_V_ADDV | WAH_A64_SZ(size_log2 == 2 ? 2 : 1), WAH_JR_VT0, WAH_JR_VT0);
        wah_a64_fmov_to_gp(f, false, d, WAH_JR_VT0);
    }
    wah_jit_free_fpr(f, a);
    wah_jit_push_gpr(f, d);
}

static void wah_a64_vextract(wah_jit_fn_t *f, uint32_t size_log2, uint32_t lane, bool is_signed, bool fp) {
    int a = wah_jit_pop_vec_ro(f);
    int d = wah_jit_alloc(f, fp);
    if (fp) wah_a64_v2(f, WAH_A64_S_DUP_ELEM | wah_a64_imm5(size_log2, lane), d, a);
    else if (is_signed) wah_a64_v2(f, WAH_A64_V_SMOV | wah_a64_imm5(size_log2, lane), d, a);
    else wah_a64_v2(f, WAH_A64_V_UMOV | (size_log2 == 3 ? WAH_A64_Q : 0) | wah_a64_imm5(size_log2, lane), d, a);
    wah_jit_free_fpr(f, a);
    if (fp) wah_jit_push_fpr(f, d);
    else wah_jit_push_gpr(f, d);
}

static void wah_a64_vreplace(wah_jit_fn_t *f, uint32_t size_log2, uint32_t lane, bool fp) {
    const wah_jit_val_t *top = &f->vals[f->h - 1];
    int s;
    if (fp) s = wah_jit_pop_fpr_ro(f);
    else if (top->kind == WAH_JV_CONST && top->imm == 0) { f->h--; s = WAH_A64_ZR; }
    else s = wah_jit_pop_gpr_ro(f);
    int d = wah_jit_pop_vec(f);
    if (fp) wah_a64_v2(f, WAH_A64_V_INS_ELEM | wah_a64_imm5(size_log2, lane), d, s);
    else wah_a64_v2(f, WAH_A64_V_INS_GEN | wah_a64_imm5(size_log2, lane), d, s);
    if (fp) wah_jit_free_fpr(f, s);
    else if (s != WAH_A64_ZR) wah_jit_free_gpr(f, s);
    wah_jit_push_vec(f, d);
}

static void wah_a64_vsplat(wah_jit_fn_t *f, uint32_t size_log2, bool fp) {
    const wah_jit_val_t *top = &f->vals[f->h - 1];
    if (top->kind == WAH_JV_CONST) {
        wah_jit_v16_t c = wah_jit_splat_const((uint64_t)top->imm, size_log2);
        f->h--;
        int d = wah_jit_alloc(f, true);
        wah_jit_load_vconst(f, d, c.b);
        wah_jit_push_vec(f, d);
        return;
    }
    int s = fp ? wah_jit_pop_fpr_ro(f) : wah_jit_pop_gpr_ro(f);
    int d = fp ? wah_jit_dest(f, true, s, -1) : wah_jit_alloc(f, true);
    wah_a64_v2(f, (fp ? WAH_A64_V_DUP_ELEM : WAH_A64_V_DUP_GEN) | wah_a64_imm5(size_log2, 0), d, s);
    if (fp) wah_jit_finish_vec(f, d, s, -1, -1);
    else { wah_jit_free_gpr(f, s); wah_jit_push_vec(f, d); }
}

// Loads with an access of 1 << size_log2 bytes, as a whole (kind 0), extended from 64 bits with lanes of ext_bits
// (1 signed, 2 unsigned), splatted (3), zero-extended (4) or into a lane (5)
static void wah_a64_vload(wah_jit_fn_t *f, const wah_decoded_instr_t *in, uint32_t size_log2, int kind, uint32_t ext_bits) {
    const wah_memarg_t *m = kind == 5 ? &in->imm.memarg_lane.memarg : &in->imm.memarg;
    int d = kind == 5 ? wah_jit_pop_vec(f) : -1, size_reg;
    wah_a64_addr_t ad = wah_jit_address(f, m, 1u << size_log2, &size_reg, kind != 3 && kind != 5);
    int base = ad.base;
    if (size_reg >= 0) wah_jit_free_gpr(f, size_reg);
    if (base != WAH_JR_MEM) wah_jit_free_gpr(f, base);
    if (d < 0) d = wah_jit_alloc(f, true);
    switch (kind) {
        case 0: wah_a64_access(f, WAH_A64_LDRQ, d, &ad); break;
        case 1: case 2:
            wah_a64_access(f, WAH_A64_LDRD, d, &ad);
            wah_a64_v2(f, (kind == 1 ? WAH_A64_V_SSHLL : WAH_A64_V_USHLL) | ext_bits << 16, d, d);
            break;
        case 3: case 5:
            wah_a64_rrr(f, WAH_A64_ADD, true, WAH_JR_T0, base, WAH_JR_T0);
            if (kind == 3) wah_a64(f, 0x4D40C000u | size_log2 << 10 | (uint32_t)WAH_JR_T0 << 5 | (uint32_t)d); // LD1R
            else wah_a64_ldst_lane(f, true, size_log2, in->imm.memarg_lane.lane, d, WAH_JR_T0);
            break;
        default: wah_a64_access(f, size_log2 == 3 ? WAH_A64_LDRD : WAH_A64_LDRS, d, &ad); break;
    }
    wah_jit_push_vec(f, d);
}

static void wah_a64_vstore(wah_jit_fn_t *f, const wah_decoded_instr_t *in, uint32_t size_log2, bool lane) {
    const wah_memarg_t *m = lane ? &in->imm.memarg_lane.memarg : &in->imm.memarg;
    int v = wah_jit_pop_vec_ro(f), size_reg;
    wah_a64_addr_t ad = wah_jit_address(f, m, 1u << size_log2, &size_reg, !lane);
    if (lane) {
        wah_a64_rrr(f, WAH_A64_ADD, true, WAH_JR_T0, ad.base, WAH_JR_T0);
        wah_a64_ldst_lane(f, false, size_log2, in->imm.memarg_lane.lane, v, WAH_JR_T0);
    } else {
        wah_a64_access(f, WAH_A64_STRQ, v, &ad);
    }
    if (size_reg >= 0) wah_jit_free_gpr(f, size_reg);
    if (ad.base != WAH_JR_MEM) wah_jit_free_gpr(f, ad.base);
    wah_jit_free_fpr(f, v);
}

static void wah_a64_vshuffle(wah_jit_fn_t *f, const uint8_t *lanes) {
    bool all_a = true, all_b = true;
    uint8_t mask[16];
    for (int i = 0; i < 16; i++) {
        all_a = all_a && lanes[i] < 16;
        all_b = all_b && lanes[i] >= 16;
    }
    int b = wah_jit_pop_vec_ro(f), a = wah_jit_pop_vec_ro(f), d = wah_jit_dest(f, true, a, b);
    if (all_a || all_b) {
        for (int i = 0; i < 16; i++) mask[i] = lanes[i] & 15;
        wah_jit_load_vconst(f, WAH_JR_VT0, mask);
        wah_a64_v3(f, WAH_A64_V_TBL1, d, all_a ? a : b, WAH_JR_VT0);
    } else { // TBL with two registers takes consecutive ones
        wah_jit_vmov(f, WAH_JR_VT0, a);
        wah_jit_vmov(f, WAH_JR_VT1, b);
        wah_jit_load_vconst(f, d, lanes);
        wah_a64_v3(f, WAH_A64_V_TBL2, d, WAH_JR_VT0, d);
    }
    wah_jit_finish_vec(f, d, a, b, -1);
}

static void wah_a64_vbitselect(wah_jit_fn_t *f) {
    int c = wah_jit_pop_vec_ro(f), b = wah_jit_pop_vec_ro(f), a = wah_jit_pop_vec_ro(f), d;
    if (f->fpr[c] == WAH_JO_TEMP) {
        wah_a64_v3(f, WAH_A64_V_BSL, d = c, a, b);
    } else if (f->fpr[a] == WAH_JO_TEMP) {
        wah_a64_v3(f, WAH_A64_V_BIF, d = a, b, c);
    } else if (f->fpr[b] == WAH_JO_TEMP) {
        wah_a64_v3(f, WAH_A64_V_BIT, d = b, a, c);
    } else {
        wah_jit_vmov(f, d = wah_jit_alloc(f, true), c);
        wah_a64_v3(f, WAH_A64_V_BSL, d, a, b);
    }
    wah_jit_finish_vec(f, d, a, b, c);
}

// pmin and pmax: a lane of b if it is less (pmin) or greater (pmax) than that of a, otherwise of a
static void wah_a64_vpminmax(wah_jit_fn_t *f, uint32_t type, bool max) {
    int b = wah_jit_pop_vec_ro(f), a = wah_jit_pop_vec_ro(f), d = wah_jit_dest(f, true, a, b);
    wah_a64_v3(f, WAH_A64_V_FCMGT | WAH_A64_SZ(type), WAH_JR_VT0, max ? b : a, max ? a : b);
    wah_a64_v3(f, WAH_A64_V_BSL, WAH_JR_VT0, b, a);
    wah_jit_vmov(f, d, WAH_JR_VT0);
    wah_jit_finish_vec(f, d, a, b, -1);
}

#if ((WAH_COMPILED_FEATURES) & WAH_FEATURE_RELAXED_SIMD)
// a * b + c, or c - a * b if negated, rounded twice
static void wah_a64_vmadd(wah_jit_fn_t *f, uint32_t type, bool negated) {
    int c = wah_jit_pop_vec_ro(f), b = wah_jit_pop_vec_ro(f), a = wah_jit_pop_vec_ro(f);
    wah_a64_v3(f, WAH_A64_V_FMUL | WAH_A64_SZ(type), WAH_JR_VT0, a, b);
    int d = wah_jit_vdest3(f, c, a, b);
    if (negated) wah_a64_v3(f, WAH_A64_V_FSUB | WAH_A64_SZ(type), d, c, WAH_JR_VT0);
    else wah_a64_v3(f, WAH_A64_V_FADD | WAH_A64_SZ(type), d, WAH_JR_VT0, c);
    wah_jit_finish_vec(f, d, a, b, c);
}

// Sums of products of signed bytes, with adjacent pairs saturated to i16, then summed again with c if given
static void wah_a64_vdot_i8(wah_jit_fn_t *f, bool add) {
    int c = add ? wah_jit_pop_vec_ro(f) : -1, b = wah_jit_pop_vec_ro(f), a = wah_jit_pop_vec_ro(f);
    wah_a64_v3(f, WAH_A64_V_SMULL, WAH_JR_VT0, a, b);
    wah_a64_v3(f, WAH_A64_V_SMULL | WAH_A64_Q, WAH_JR_VT1, a, b);
    wah_a64_v2(f, WAH_A64_V_SADDLP | WAH_A64_SZ(1), WAH_JR_VT0, WAH_JR_VT0);
    wah_a64_v2(f, WAH_A64_V_SADDLP | WAH_A64_SZ(1), WAH_JR_VT1, WAH_JR_VT1);
    int d = add ? wah_jit_vdest3(f, c, a, b) : wah_jit_dest(f, true, a, b);
    int s = add ? WAH_JR_VT0 : d;
    wah_a64_v2(f, WAH_A64_V_SQXTN | WAH_A64_SZ(1), s, WAH_JR_VT0);
    wah_a64_v2(f, WAH_A64_V_SQXTN | WAH_A64_SZ(1) | WAH_A64_Q, s, WAH_JR_VT1);
    if (add) {
        if (d != c) wah_jit_vmov(f, d, c);
        wah_a64_v2(f, WAH_A64_V_SADALP | WAH_A64_SZ(1), d, WAH_JR_VT0);
    }
    wah_jit_finish_vec(f, d, a, b, c);
}
#endif

static void wah_a64_vi64_mul(wah_jit_fn_t *f) {
    int b = wah_jit_pop_vec_ro(f), a = wah_jit_pop_vec_ro(f), d = wah_jit_dest(f, true, a, b);
    for (uint32_t i = 0; i < 2; i++) {
        wah_a64_v2(f, WAH_A64_V_UMOV | WAH_A64_Q | wah_a64_imm5(3, i), WAH_JR_T0, a);
        wah_a64_v2(f, WAH_A64_V_UMOV | WAH_A64_Q | wah_a64_imm5(3, i), WAH_JR_T1, b);
        wah_a64_rrr(f, WAH_A64_MUL, true, WAH_JR_T0, WAH_JR_T0, WAH_JR_T1);
        wah_a64_v2(f, WAH_A64_V_INS_GEN | wah_a64_imm5(3, i), WAH_JR_VT0, WAH_JR_T0);
    }
    wah_jit_vmov(f, d, WAH_JR_VT0);
    wah_jit_finish_vec(f, d, a, b, -1);
}

static void wah_a64_vdot_i16(wah_jit_fn_t *f) {
    int b = wah_jit_pop_vec_ro(f), a = wah_jit_pop_vec_ro(f), d = wah_jit_dest(f, true, a, b);
    wah_a64_v3(f, WAH_A64_V_SMULL | WAH_A64_SZ(1), WAH_JR_VT0, a, b);
    wah_a64_v3(f, WAH_A64_V_SMULL | WAH_A64_SZ(1) | WAH_A64_Q, WAH_JR_VT1, a, b);
    wah_a64_v3(f, WAH_A64_V_ADDP | WAH_A64_SZ(2), d, WAH_JR_VT0, WAH_JR_VT1);
    wah_jit_finish_vec(f, d, a, b, -1);
}

// Unary instruction into VT0, then insn2 into the result
static void wah_a64_vun2(wah_jit_fn_t *f, uint32_t insn, uint32_t insn2) {
    int a = wah_jit_pop_vec_ro(f);
    wah_a64_v2(f, insn, WAH_JR_VT0, a);
    int d = wah_jit_dest(f, true, a, -1);
    wah_a64_v2(f, insn2, d, WAH_JR_VT0);
    wah_jit_finish_vec(f, d, a, -1, -1);
}

static void wah_jit_simd(wah_jit_fn_t *f, const wah_decoded_instr_t *in, bool fuse) {
    switch (in->opcode) {
        case WAH_OP_V128_LOAD: wah_a64_vload(f, in, 4, 0, 0); break;
        case WAH_OP_V128_LOAD8X8_S: wah_a64_vload(f, in, 3, 1, 8); break;
        case WAH_OP_V128_LOAD8X8_U: wah_a64_vload(f, in, 3, 2, 8); break;
        case WAH_OP_V128_LOAD16X4_S: wah_a64_vload(f, in, 3, 1, 16); break;
        case WAH_OP_V128_LOAD16X4_U: wah_a64_vload(f, in, 3, 2, 16); break;
        case WAH_OP_V128_LOAD32X2_S: wah_a64_vload(f, in, 3, 1, 32); break;
        case WAH_OP_V128_LOAD32X2_U: wah_a64_vload(f, in, 3, 2, 32); break;
        case WAH_OP_V128_LOAD8_SPLAT: wah_a64_vload(f, in, 0, 3, 0); break;
        case WAH_OP_V128_LOAD16_SPLAT: wah_a64_vload(f, in, 1, 3, 0); break;
        case WAH_OP_V128_LOAD32_SPLAT: wah_a64_vload(f, in, 2, 3, 0); break;
        case WAH_OP_V128_LOAD64_SPLAT: wah_a64_vload(f, in, 3, 3, 0); break;
        case WAH_OP_V128_LOAD32_ZERO: wah_a64_vload(f, in, 2, 4, 0); break;
        case WAH_OP_V128_LOAD64_ZERO: wah_a64_vload(f, in, 3, 4, 0); break;
        case WAH_OP_V128_LOAD8_LANE: wah_a64_vload(f, in, 0, 5, 0); break;
        case WAH_OP_V128_LOAD16_LANE: wah_a64_vload(f, in, 1, 5, 0); break;
        case WAH_OP_V128_LOAD32_LANE: wah_a64_vload(f, in, 2, 5, 0); break;
        case WAH_OP_V128_LOAD64_LANE: wah_a64_vload(f, in, 3, 5, 0); break;
        case WAH_OP_V128_STORE: wah_a64_vstore(f, in, 4, false); break;
        case WAH_OP_V128_STORE8_LANE: wah_a64_vstore(f, in, 0, true); break;
        case WAH_OP_V128_STORE16_LANE: wah_a64_vstore(f, in, 1, true); break;
        case WAH_OP_V128_STORE32_LANE: wah_a64_vstore(f, in, 2, true); break;
        case WAH_OP_V128_STORE64_LANE: wah_a64_vstore(f, in, 3, true); break;

        case WAH_OP_V128_CONST: {
            int d = wah_jit_alloc(f, true);
            wah_jit_load_vconst(f, d, in->imm.v128);
            wah_jit_push_vec(f, d);
            break;
        }
        case WAH_OP_I8X16_SHUFFLE: wah_a64_vshuffle(f, in->imm.v128); break;
        case WAH_OP_I8X16_SWIZZLE: WAH_IF_RELAXED_SIMD(case WAH_OP_I8X16_RELAXED_SWIZZLE:) wah_a64_vbin(f, WAH_A64_V_TBL1, false, false); break;
        case WAH_OP_I8X16_EXTRACT_LANE_S: wah_a64_vextract(f, 0, in->imm.u32, true, false); break;
        case WAH_OP_I8X16_EXTRACT_LANE_U: wah_a64_vextract(f, 0, in->imm.u32, false, false); break;
        case WAH_OP_I16X8_EXTRACT_LANE_S: wah_a64_vextract(f, 1, in->imm.u32, true, false); break;
        case WAH_OP_I16X8_EXTRACT_LANE_U: wah_a64_vextract(f, 1, in->imm.u32, false, false); break;
        case WAH_OP_I32X4_EXTRACT_LANE: wah_a64_vextract(f, 2, in->imm.u32, false, false); break;
        case WAH_OP_I64X2_EXTRACT_LANE: wah_a64_vextract(f, 3, in->imm.u32, false, false); break;
        case WAH_OP_F32X4_EXTRACT_LANE: wah_a64_vextract(f, 2, in->imm.u32, false, true); break;
        case WAH_OP_F64X2_EXTRACT_LANE: wah_a64_vextract(f, 3, in->imm.u32, false, true); break;
        case WAH_OP_I8X16_REPLACE_LANE: wah_a64_vreplace(f, 0, in->imm.u32, false); break;
        case WAH_OP_I16X8_REPLACE_LANE: wah_a64_vreplace(f, 1, in->imm.u32, false); break;
        case WAH_OP_I32X4_REPLACE_LANE: wah_a64_vreplace(f, 2, in->imm.u32, false); break;
        case WAH_OP_I64X2_REPLACE_LANE: wah_a64_vreplace(f, 3, in->imm.u32, false); break;
        case WAH_OP_F32X4_REPLACE_LANE: wah_a64_vreplace(f, 2, in->imm.u32, true); break;
        case WAH_OP_F64X2_REPLACE_LANE: wah_a64_vreplace(f, 3, in->imm.u32, true); break;
        case WAH_OP_I8X16_SPLAT: wah_a64_vsplat(f, 0, false); break;
        case WAH_OP_I16X8_SPLAT: wah_a64_vsplat(f, 1, false); break;
        case WAH_OP_I32X4_SPLAT: wah_a64_vsplat(f, 2, false); break;
        case WAH_OP_I64X2_SPLAT: wah_a64_vsplat(f, 3, false); break;
        case WAH_OP_F32X4_SPLAT: wah_a64_vsplat(f, 2, true); break;
        case WAH_OP_F64X2_SPLAT: wah_a64_vsplat(f, 3, true); break;

        case WAH_OP_V128_NOT: wah_a64_vun(f, WAH_A64_V_NOT); break;
        case WAH_OP_V128_AND: wah_a64_vbin(f, WAH_A64_V_AND, false, false); break;
        case WAH_OP_V128_ANDNOT: wah_a64_vbin(f, WAH_A64_V_BIC, false, false); break;
        case WAH_OP_V128_OR: wah_a64_vbin(f, WAH_A64_V_ORR, false, false); break;
        case WAH_OP_V128_XOR: wah_a64_vbin(f, WAH_A64_V_EOR, false, false); break;
        case WAH_OP_V128_BITSELECT:
        WAH_IF_RELAXED_SIMD(case WAH_OP_I8X16_RELAXED_LANESELECT: case WAH_OP_I16X8_RELAXED_LANESELECT:
                            case WAH_OP_I32X4_RELAXED_LANESELECT: case WAH_OP_I64X2_RELAXED_LANESELECT:)
            wah_a64_vbitselect(f);
            break;
        case WAH_OP_V128_ANY_TRUE: wah_a64_vtest(f, -1, fuse); break;

#define WAH_A64_VCMP(N, sz) \
        case WAH_OP_##N##_EQ: wah_a64_vbin(f, WAH_A64_V_CMEQ | WAH_A64_SZ(sz), false, false); break; \
        case WAH_OP_##N##_NE: wah_a64_vbin(f, WAH_A64_V_CMEQ | WAH_A64_SZ(sz), false, true); break; \
        case WAH_OP_##N##_LT_S: wah_a64_vbin(f, WAH_A64_V_CMGT | WAH_A64_SZ(sz), true, false); break; \
        case WAH_OP_##N##_GT_S: wah_a64_vbin(f, WAH_A64_V_CMGT | WAH_A64_SZ(sz), false, false); break; \
        case WAH_OP_##N##_LE_S: wah_a64_vbin(f, WAH_A64_V_CMGE | WAH_A64_SZ(sz), true, false); break; \
        case WAH_OP_##N##_GE_S: wah_a64_vbin(f, WAH_A64_V_CMGE | WAH_A64_SZ(sz), false, false); break;
#define WAH_A64_VCMP_U(N, sz) \
        case WAH_OP_##N##_LT_U: wah_a64_vbin(f, WAH_A64_V_CMHI | WAH_A64_SZ(sz), true, false); break; \
        case WAH_OP_##N##_GT_U: wah_a64_vbin(f, WAH_A64_V_CMHI | WAH_A64_SZ(sz), false, false); break; \
        case WAH_OP_##N##_LE_U: wah_a64_vbin(f, WAH_A64_V_CMHS | WAH_A64_SZ(sz), true, false); break; \
        case WAH_OP_##N##_GE_U: wah_a64_vbin(f, WAH_A64_V_CMHS | WAH_A64_SZ(sz), false, false); break;
        WAH_A64_VCMP(I8X16, 0) WAH_A64_VCMP_U(I8X16, 0)
        WAH_A64_VCMP(I16X8, 1) WAH_A64_VCMP_U(I16X8, 1)
        WAH_A64_VCMP(I32X4, 2) WAH_A64_VCMP_U(I32X4, 2)
        WAH_A64_VCMP(I64X2, 3)
#undef WAH_A64_VCMP
#undef WAH_A64_VCMP_U
#define WAH_A64_VFCMP(N, type) \
        case WAH_OP_##N##_EQ: wah_a64_vbin(f, WAH_A64_V_FCMEQ | WAH_A64_SZ(type), false, false); break; \
        case WAH_OP_##N##_NE: wah_a64_vbin(f, WAH_A64_V_FCMEQ | WAH_A64_SZ(type), false, true); break; \
        case WAH_OP_##N##_LT: wah_a64_vbin(f, WAH_A64_V_FCMGT | WAH_A64_SZ(type), true, false); break; \
        case WAH_OP_##N##_GT: wah_a64_vbin(f, WAH_A64_V_FCMGT | WAH_A64_SZ(type), false, false); break; \
        case WAH_OP_##N##_LE: wah_a64_vbin(f, WAH_A64_V_FCMGE | WAH_A64_SZ(type), true, false); break; \
        case WAH_OP_##N##_GE: wah_a64_vbin(f, WAH_A64_V_FCMGE | WAH_A64_SZ(type), false, false); break;
        WAH_A64_VFCMP(F32X4, 0)
        WAH_A64_VFCMP(F64X2, 1)
#undef WAH_A64_VFCMP

#define WAH_A64_VINT(N, sz) \
        case WAH_OP_##N##_ABS: wah_a64_vun(f, WAH_A64_V_ABS | WAH_A64_SZ(sz)); break; \
        case WAH_OP_##N##_NEG: wah_a64_vun(f, WAH_A64_V_NEG | WAH_A64_SZ(sz)); break; \
        case WAH_OP_##N##_ALL_TRUE: wah_a64_vtest(f, sz, fuse); break; \
        case WAH_OP_##N##_BITMASK: wah_a64_vbitmask(f, sz); break; \
        case WAH_OP_##N##_SHL: wah_a64_vshift(f, sz, WAH_JS_SHL); break; \
        case WAH_OP_##N##_SHR_S: wah_a64_vshift(f, sz, WAH_JS_SHR_S); break; \
        case WAH_OP_##N##_SHR_U: wah_a64_vshift(f, sz, WAH_JS_SHR_U); break; \
        case WAH_OP_##N##_ADD: wah_a64_vbin(f, WAH_A64_V_ADD | WAH_A64_SZ(sz), false, false); break; \
        case WAH_OP_##N##_SUB: wah_a64_vbin(f, WAH_A64_V_SUB | WAH_A64_SZ(sz), false, false); break;
#define WAH_A64_VINT_SMALL(N, sz) \
        case WAH_OP_##N##_ADD_SAT_S: wah_a64_vbin(f, WAH_A64_V_SQADD | WAH_A64_SZ(sz), false, false); break; \
        case WAH_OP_##N##_ADD_SAT_U: wah_a64_vbin(f, WAH_A64_V_UQADD | WAH_A64_SZ(sz), false, false); break; \
        case WAH_OP_##N##_SUB_SAT_S: wah_a64_vbin(f, WAH_A64_V_SQSUB | WAH_A64_SZ(sz), false, false); break; \
        case WAH_OP_##N##_SUB_SAT_U: wah_a64_vbin(f, WAH_A64_V_UQSUB | WAH_A64_SZ(sz), false, false); break; \
        case WAH_OP_##N##_AVGR_U: wah_a64_vbin(f, WAH_A64_V_URHADD | WAH_A64_SZ(sz), false, false); break;
#define WAH_A64_VMINMAX(N, sz) \
        case WAH_OP_##N##_MIN_S: wah_a64_vbin(f, WAH_A64_V_SMIN | WAH_A64_SZ(sz), false, false); break; \
        case WAH_OP_##N##_MIN_U: wah_a64_vbin(f, WAH_A64_V_UMIN | WAH_A64_SZ(sz), false, false); break; \
        case WAH_OP_##N##_MAX_S: wah_a64_vbin(f, WAH_A64_V_SMAX | WAH_A64_SZ(sz), false, false); break; \
        case WAH_OP_##N##_MAX_U: wah_a64_vbin(f, WAH_A64_V_UMAX | WAH_A64_SZ(sz), false, false); break;
        WAH_A64_VINT(I8X16, 0) WAH_A64_VINT_SMALL(I8X16, 0) WAH_A64_VMINMAX(I8X16, 0)
        WAH_A64_VINT(I16X8, 1) WAH_A64_VINT_SMALL(I16X8, 1) WAH_A64_VMINMAX(I16X8, 1)
        WAH_A64_VINT(I32X4, 2) WAH_A64_VMINMAX(I32X4, 2)
        WAH_A64_VINT(I64X2, 3)
#undef WAH_A64_VINT
#undef WAH_A64_VINT_SMALL
#undef WAH_A64_VMINMAX
        case WAH_OP_I8X16_POPCNT: wah_a64_vun(f, WAH_A64_V_CNT); break;
        case WAH_OP_I16X8_MUL: wah_a64_vbin(f, WAH_A64_V_MUL | WAH_A64_SZ(1), false, false); break;
        case WAH_OP_I32X4_MUL: wah_a64_vbin(f, WAH_A64_V_MUL | WAH_A64_SZ(2), false, false); break;
        case WAH_OP_I64X2_MUL: wah_a64_vi64_mul(f); break;
        case WAH_OP_I16X8_Q15MULR_SAT_S: WAH_IF_RELAXED_SIMD(case WAH_OP_I16X8_RELAXED_Q15MULR_S:)
            wah_a64_vbin(f, WAH_A64_V_SQRDMULH | WAH_A64_SZ(1), false, false);
            break;
        case WAH_OP_I32X4_DOT_I16X8_S: wah_a64_vdot_i16(f); break;
        case WAH_OP_I8X16_NARROW_I16X8_S: wah_a64_vhalves(f, WAH_A64_V_SQXTN | WAH_A64_SZ(0), true); break;
        case WAH_OP_I8X16_NARROW_I16X8_U: wah_a64_vhalves(f, WAH_A64_V_SQXTUN | WAH_A64_SZ(0), true); break;
        case WAH_OP_I16X8_NARROW_I32X4_S: wah_a64_vhalves(f, WAH_A64_V_SQXTN | WAH_A64_SZ(1), true); break;
        case WAH_OP_I16X8_NARROW_I32X4_U: wah_a64_vhalves(f, WAH_A64_V_SQXTUN | WAH_A64_SZ(1), true); break;

#define WAH_A64_VEXT(N, FROM, bits) \
        case WAH_OP_##N##_EXTEND_LOW_##FROM##_S: wah_a64_vun(f, WAH_A64_V_SSHLL | (bits) << 16); break; \
        case WAH_OP_##N##_EXTEND_HIGH_##FROM##_S: wah_a64_vun(f, WAH_A64_V_SSHLL | WAH_A64_Q | (bits) << 16); break; \
        case WAH_OP_##N##_EXTEND_LOW_##FROM##_U: wah_a64_vun(f, WAH_A64_V_USHLL | (bits) << 16); break; \
        case WAH_OP_##N##_EXTEND_HIGH_##FROM##_U: wah_a64_vun(f, WAH_A64_V_USHLL | WAH_A64_Q | (bits) << 16); break; \
        case WAH_OP_##N##_EXTMUL_LOW_##FROM##_S: wah_a64_vmull(f, WAH_A64_V_SMULL | WAH_A64_SZ(bits / 16)); break; \
        case WAH_OP_##N##_EXTMUL_HIGH_##FROM##_S: wah_a64_vmull(f, WAH_A64_V_SMULL | WAH_A64_SZ(bits / 16) | WAH_A64_Q); break; \
        case WAH_OP_##N##_EXTMUL_LOW_##FROM##_U: wah_a64_vmull(f, WAH_A64_V_UMULL | WAH_A64_SZ(bits / 16)); break; \
        case WAH_OP_##N##_EXTMUL_HIGH_##FROM##_U: wah_a64_vmull(f, WAH_A64_V_UMULL | WAH_A64_SZ(bits / 16) | WAH_A64_Q); break;
        WAH_A64_VEXT(I16X8, I8X16, 8)
        WAH_A64_VEXT(I32X4, I16X8, 16)
        WAH_A64_VEXT(I64X2, I32X4, 32)
#undef WAH_A64_VEXT
        case WAH_OP_I16X8_EXTADD_PAIRWISE_I8X16_S: wah_a64_vun(f, WAH_A64_V_SADDLP | WAH_A64_SZ(0)); break;
        case WAH_OP_I16X8_EXTADD_PAIRWISE_I8X16_U: wah_a64_vun(f, WAH_A64_V_UADDLP | WAH_A64_SZ(0)); break;
        case WAH_OP_I32X4_EXTADD_PAIRWISE_I16X8_S: wah_a64_vun(f, WAH_A64_V_SADDLP | WAH_A64_SZ(1)); break;
        case WAH_OP_I32X4_EXTADD_PAIRWISE_I16X8_U: wah_a64_vun(f, WAH_A64_V_UADDLP | WAH_A64_SZ(1)); break;

        // FPCR.DN makes NaN results canonical, also for vectors
#define WAH_A64_VFP(N, type) \
        case WAH_OP_##N##_CEIL: wah_a64_vun(f, WAH_A64_V_FRINTP | WAH_A64_SZ(type)); break; \
        case WAH_OP_##N##_FLOOR: wah_a64_vun(f, WAH_A64_V_FRINTM | WAH_A64_SZ(type)); break; \
        case WAH_OP_##N##_TRUNC: wah_a64_vun(f, WAH_A64_V_FRINTZ | WAH_A64_SZ(type)); break; \
        case WAH_OP_##N##_NEAREST: wah_a64_vun(f, WAH_A64_V_FRINTN | WAH_A64_SZ(type)); break; \
        case WAH_OP_##N##_ABS: wah_a64_vun(f, WAH_A64_V_FABS | WAH_A64_SZ(type)); break; \
        case WAH_OP_##N##_NEG: wah_a64_vun(f, WAH_A64_V_FNEG | WAH_A64_SZ(type)); break; \
        case WAH_OP_##N##_SQRT: wah_a64_vun(f, WAH_A64_V_FSQRT | WAH_A64_SZ(type)); break; \
        case WAH_OP_##N##_ADD: wah_a64_vbin(f, WAH_A64_V_FADD | WAH_A64_SZ(type), false, false); break; \
        case WAH_OP_##N##_SUB: wah_a64_vbin(f, WAH_A64_V_FSUB | WAH_A64_SZ(type), false, false); break; \
        case WAH_OP_##N##_MUL: wah_a64_vbin(f, WAH_A64_V_FMUL | WAH_A64_SZ(type), false, false); break; \
        case WAH_OP_##N##_DIV: wah_a64_vbin(f, WAH_A64_V_FDIV | WAH_A64_SZ(type), false, false); break; \
        case WAH_OP_##N##_MIN: WAH_IF_RELAXED_SIMD(case WAH_OP_##N##_RELAXED_MIN:) \
            wah_a64_vbin(f, WAH_A64_V_FMIN | WAH_A64_SZ(type), false, false); break; \
        case WAH_OP_##N##_MAX: WAH_IF_RELAXED_SIMD(case WAH_OP_##N##_RELAXED_MAX:) \
            wah_a64_vbin(f, WAH_A64_V_FMAX | WAH_A64_SZ(type), false, false); break; \
        case WAH_OP_##N##_PMIN: wah_a64_vpminmax(f, type, false); break; \
        case WAH_OP_##N##_PMAX: wah_a64_vpminmax(f, type, true); break; \
        WAH_IF_RELAXED_SIMD(case WAH_OP_##N##_RELAXED_MADD: wah_a64_vmadd(f, type, false); break; \
                            case WAH_OP_##N##_RELAXED_NMADD: wah_a64_vmadd(f, type, true); break;)
        WAH_A64_VFP(F32X4, 0)
        WAH_A64_VFP(F64X2, 1)
#undef WAH_A64_VFP

        case WAH_OP_I32X4_TRUNC_SAT_F32X4_S: WAH_IF_RELAXED_SIMD(case WAH_OP_I32X4_RELAXED_TRUNC_F32X4_S:)
            wah_a64_vun(f, WAH_A64_V_FCVTZS);
            break;
        case WAH_OP_I32X4_TRUNC_SAT_F32X4_U: WAH_IF_RELAXED_SIMD(case WAH_OP_I32X4_RELAXED_TRUNC_F32X4_U:)
            wah_a64_vun(f, WAH_A64_V_FCVTZU);
            break;
        case WAH_OP_I32X4_TRUNC_SAT_F64X2_S_ZERO: WAH_IF_RELAXED_SIMD(case WAH_OP_I32X4_RELAXED_TRUNC_F64X2_S:)
            wah_a64_vun2(f, WAH_A64_V_FCVTZS | WAH_A64_SZ(1), WAH_A64_V_SQXTN | WAH_A64_SZ(2));
            break;
        case WAH_OP_I32X4_TRUNC_SAT_F64X2_U_ZERO: WAH_IF_RELAXED_SIMD(case WAH_OP_I32X4_RELAXED_TRUNC_F64X2_U:)
            wah_a64_vun2(f, WAH_A64_V_FCVTZU | WAH_A64_SZ(1), WAH_A64_V_UQXTN | WAH_A64_SZ(2));
            break;
        case WAH_OP_F32X4_CONVERT_I32X4_S: wah_a64_vun(f, WAH_A64_V_SCVTF); break;
        case WAH_OP_F32X4_CONVERT_I32X4_U: wah_a64_vun(f, WAH_A64_V_UCVTF); break;
        case WAH_OP_F64X2_CONVERT_LOW_I32X4_S: wah_a64_vun2(f, WAH_A64_V_SSHLL | 32u << 16, WAH_A64_V_SCVTF | WAH_A64_SZ(1)); break;
        case WAH_OP_F64X2_CONVERT_LOW_I32X4_U: wah_a64_vun2(f, WAH_A64_V_USHLL | 32u << 16, WAH_A64_V_UCVTF | WAH_A64_SZ(1)); break;
        case WAH_OP_F32X4_DEMOTE_F64X2_ZERO: wah_a64_vun(f, WAH_A64_V_FCVTN); break;
        case WAH_OP_F64X2_PROMOTE_LOW_F32X4: wah_a64_vun(f, WAH_A64_V_FCVTL); break;

#if ((WAH_COMPILED_FEATURES) & WAH_FEATURE_RELAXED_SIMD)
        case WAH_OP_I16X8_RELAXED_DOT_I8X16_I7X16_S: wah_a64_vdot_i8(f, false); break;
        case WAH_OP_I32X4_RELAXED_DOT_I8X16_I7X16_ADD_S: wah_a64_vdot_i8(f, true); break;
#endif
        default: WAH_JIT_FAIL(f, "unhandled SIMD opcode"); break;
    }
}

#endif // WAH_FEATURE_SIMD

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
    if (!f->jm->guarded) wah_jit_ldst(f, false, false, true, WAH_JR_MEMSZ, WAH_JR_CTX, WAH_JIT_CTX_OFF(memory_size));
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

#if ((WAH_COMPILED_FEATURES) & WAH_FEATURE_BULK_MEMORY)
// After a helper call returning a status, which suspends at `resume` if positive.
static void wah_jit_check_status(wah_jit_fn_t *f, uint32_t resume) {
    uint32_t stub = wah_jit_new_label(f);
    wah_jit_cbz(f, true, false, WAH_JR_RET, stub);
    wah_jit_add_stub(f, stub, resume, 0, false);
}
#endif

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
    return t == WAH_TYPE_I32 || t == WAH_TYPE_I64 || t == WAH_TYPE_F32 || t == WAH_TYPE_F64 || t == WAH_TYPE_V128;
}

static bool wah_jit_numeric_func_type(const wah_func_type_t *ft) {
    for (uint32_t i = 0; i < ft->param_count; i++) if (!wah_jit_numeric_type(ft->param_types[i])) return false;
    for (uint32_t i = 0; i < ft->result_count; i++) if (!wah_jit_numeric_type(ft->result_types[i])) return false;
    return true;
}

// Loads a local or global of type t at [base + off] into a new register and pushes it.
static void wah_jit_push_load(wah_jit_fn_t *f, wah_type_t t, int base, uint32_t off) {
    bool fp = t != WAH_TYPE_I32 && t != WAH_TYPE_I64;
    int r = wah_jit_alloc(f, fp);
    if (t == WAH_TYPE_V128) {
        wah_jit_vldst(f, false, r, base, off);
        wah_jit_push_vec(f, r);
    } else {
        wah_jit_ldst(f, false, fp, t == WAH_TYPE_I64 || t == WAH_TYPE_F64, r, base, off);
        if (fp) wah_jit_push_fpr(f, r);
        else wah_jit_push_gpr(f, r);
    }
}

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
        case WAH_OP_MEMORY_SIZE: case WAH_OP_MEMORY_GROW:
        WAH_IF_BULK_MEMORY(case WAH_OP_MEMORY_INIT: case WAH_OP_DATA_DROP: case WAH_OP_MEMORY_COPY: case WAH_OP_MEMORY_FILL:)
        case WAH_OP_I32_CONST: case WAH_OP_I64_CONST: case WAH_OP_F32_CONST: case WAH_OP_F64_CONST:
            return true;
        default:
            if (in->opcode >= WAH_OP_I32_LOAD && in->opcode <= WAH_OP_I64_STORE32) return true;
            if (in->opcode >= WAH_OP_I32_EQZ && in->opcode <= WAH_OP_F64_REINTERPRET_I64) return true;
            WAH_IF_SIGN_EXT(if (in->opcode >= WAH_OP_I32_EXTEND8_S && in->opcode <= WAH_OP_I64_EXTEND32_S) return true;)
            WAH_IF_NONTRAPPING_F2I(if (in->opcode >= WAH_OP_I32_TRUNC_SAT_F32_S && in->opcode <= WAH_OP_I64_TRUNC_SAT_F64_U) return true;)
#if ((WAH_COMPILED_FEATURES) & WAH_FEATURE_SIMD)
            if (in->opcode >= WAH_FD && in->opcode < WAH_FE) return wah_jit_simd_supported(module->jit);
#endif
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

// Whether local idx is read by one of the next few instructions before being set, without control flow in between.
static bool wah_jit_read_soon(const wah_jit_fn_t *f, uint32_t idx) {
    for (uint32_t j = f->instr_idx + 1; j < f->ninstrs && j <= f->instr_idx + 32; j++) {
        const wah_decoded_instr_t *in = &f->instrs[j];
        switch (in->opcode) {
            case WAH_OP_LOCAL_GET: if (in->imm.u32 == idx) return true; break;
            case WAH_OP_LOCAL_SET: case WAH_OP_LOCAL_TEE: if (in->imm.u32 == idx) return false; break;
            case WAH_OP_BLOCK: case WAH_OP_LOOP: case WAH_OP_IF: case WAH_OP_ELSE: case WAH_OP_END:
            case WAH_OP_BR: case WAH_OP_BR_IF: case WAH_OP_BR_TABLE: case WAH_OP_RETURN: case WAH_OP_UNREACHABLE:
            case WAH_OP_CALL: case WAH_OP_CALL_INDIRECT: return false;
            default: break;
        }
    }
    return false;
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

    // A result set to a pinned local right away goes into its register, unless its old value is still needed
    f->hint = -1;
    uint32_t arity = wah_jit_hint_arity(op);
    if (arity > 0 && next && (next->opcode == WAH_OP_LOCAL_SET || next->opcode == WAH_OP_LOCAL_TEE) && f->pin_of &&
        f->pin_of[next->imm.u32] >= 0 && !(f->pin_of[next->imm.u32] & WAH_JIT_PIN_FP) && f->h >= arity) {
        uint32_t idx = next->imm.u32, i = 0;
        while (i < f->h - arity && !(f->vals[i].kind == WAH_JV_LGPR && f->vals[i].imm == (int64_t)idx)) i++;
        if (i == f->h - arity) {
            f->hint = f->pin_of[idx];
            f->hint_idx = idx;
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
            wah_jit_sync_pins(f, true);
            uint32_t args = f->h - ft->param_count;
            if (idx < f->module->import_function_count) {
                const wah_jit_arg_t a[] = { { WAH_JA_CTX, 0 }, { WAH_JA_IMM, idx }, { WAH_JA_SLOT, args } };
                wah_jit_call_helper(f, (uintptr_t)wah_jit_call_host, a, 3, true);
                wah_jit_cbz(f, true, false, WAH_JR_RET, f->l_err);
            } else {
                wah_jit_call_wasm(f, wah_jit_slot_off(f, args), idx - f->module->import_function_count);
            }
            wah_jit_reload_memory(f);
            wah_jit_sync_pins(f, false);
            wah_jit_reset(f, args + ft->result_count);
            break;
        }
        case WAH_OP_CALL_INDIRECT: {
            const wah_func_type_t *ft = &f->module->types[in->imm.u32_u32.a];
            wah_jit_flush(f);
            wah_jit_sync_pins(f, true);
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
            wah_jit_sync_pins(f, false);
            wah_jit_reset(f, args + ft->result_count);
            break;
        }

        case WAH_OP_DROP: wah_jit_drop(f); break;
        case WAH_OP_SELECT: case WAH_OP_SELECT_T:
#if ((WAH_COMPILED_FEATURES) & WAH_FEATURE_SIMD)
            if (in->imm.i32 == WAH_TYPE_V128) { wah_jit_vselect(f); break; }
#endif
            wah_jit_select(f);
            break;

        case WAH_OP_LOCAL_GET: {
            uint32_t idx = in->imm.u32;
            if (f->pin_of && f->pin_of[idx] >= 0) {
                wah_jit_push(f, wah_jit_pinned_val(f, idx));
                break;
            }
            wah_type_t t = wah_jit_local_type(f, body, idx);
            bool fp = t != WAH_TYPE_I32 && t != WAH_TYPE_I64;
            const int32_t *local = fp ? f->fpr_local : f->gpr_local;
            int c = 31;
            while (c >= 0 && local[c] != (int32_t)idx) c--;
            if (c < 0) {
                wah_jit_push_load(f, t, WAH_JR_FP, idx * (uint32_t)sizeof(wah_value_t));
                break;
            }
            int r = wah_jit_alloc(f, fp); // c if it is the only free register, still holding the value
            if (r != c && t == WAH_TYPE_V128) wah_jit_vmov(f, r, c);
            else if (r != c && fp) wah_jit_fmov(f, r, c);
            else if (r != c) wah_jit_mov64(f, r, c);
            if (t == WAH_TYPE_V128) wah_jit_push_vec(f, r);
            else if (fp) wah_jit_push_fpr(f, r);
            else wah_jit_push_gpr(f, r);
            break;
        }
        case WAH_OP_LOCAL_SET: case WAH_OP_LOCAL_TEE: {
            uint32_t idx = in->imm.u32, off = idx * (uint32_t)sizeof(wah_value_t);
            if (f->pin_of && f->pin_of[idx] >= 0) {
                wah_jit_set_pinned(f, idx);
                wah_jit_drop(f);
                if (op == WAH_OP_LOCAL_TEE) wah_jit_push(f, wah_jit_pinned_val(f, idx));
                break;
            }
            wah_type_t t = wah_jit_local_type(f, body, idx);
            bool vec = t == WAH_TYPE_V128, fp = t != WAH_TYPE_I32 && t != WAH_TYPE_I64;
            for (int r = 0; r < 32; r++) { // Registers holding the old value
                if (f->gpr_local[r] == (int32_t)idx) f->gpr_local[r] = -1;
                if (f->fpr_local[r] == (int32_t)idx) f->fpr_local[r] = -1;
            }
            if (op == WAH_OP_LOCAL_TEE && f->vals[f->h - 1].kind == WAH_JV_SLOT) {
                if (vec) wah_jit_to_vec(f, f->h - 1);
                else if (fp) wah_jit_to_fpr(f, f->h - 1);
                else wah_jit_to_gpr(f, f->h - 1);
            }
            wah_jit_copy_val(f, f->h - 1, off);
            const wah_jit_val_t *v = &f->vals[f->h - 1];
            int32_t *local = fp ? f->fpr_local : f->gpr_local;
            if (v->kind != (vec ? WAH_JV_VEC : fp ? WAH_JV_FPR : WAH_JV_GPR)) {
                if (op == WAH_OP_LOCAL_SET) wah_jit_drop(f);
            } else if (op == WAH_OP_LOCAL_SET) { // The freed register keeps the value until reused
                local[v->reg] = (int32_t)idx;
                wah_jit_drop(f);
            } else if (wah_jit_read_soon(f, idx)) { // Copied to a free register, as the value on the stack may change
                int c = wah_jit_free_reg(f, fp);
                if (c >= 0) {
                    if (vec) wah_jit_vmov(f, c, v->reg);
                    else if (fp) wah_jit_fmov(f, c, v->reg);
                    else wah_jit_mov64(f, c, v->reg);
                    local[c] = (int32_t)idx;
                }
            }
            break;
        }
        case WAH_OP_GLOBAL_GET: case WAH_OP_GLOBAL_SET: {
            uint32_t idx = in->imm.u32, off = idx * (uint32_t)sizeof(wah_value_t);
            const wah_module_t *m = f->module;
            bool indirect = idx < m->import_global_count && m->global_imports[idx].is_mutable;
            wah_type_t t = idx < m->import_global_count ? m->global_imports[idx].type
                                                        : m->globals[idx - m->import_global_count].type;
            bool vec = t == WAH_TYPE_V128, fp = t != WAH_TYPE_I32 && t != WAH_TYPE_I64;
            bool get = op == WAH_OP_GLOBAL_GET;
            int r = get ? wah_jit_alloc(f, fp) : vec ? wah_jit_pop_vec_ro(f) : fp ? wah_jit_pop_fpr_ro(f) : wah_jit_pop_gpr_ro(f);
            int base = wah_jit_globals(f);
            if (indirect) {
                wah_jit_ldst(f, false, false, true, WAH_JR_T1, base, off);
                base = WAH_JR_T1;
                off = 0;
            }
            if (vec) wah_jit_vldst(f, !get, r, base, off);
            else wah_jit_ldst(f, !get, fp, t == WAH_TYPE_I64 || t == WAH_TYPE_F64, r, base, off);
            if (get) wah_jit_push(f, (wah_jit_val_t){ .kind = vec ? WAH_JV_VEC : fp ? WAH_JV_FPR : WAH_JV_GPR, .reg = (uint8_t)r });
            else if (fp) wah_jit_free_fpr(f, r);
            else wah_jit_free_gpr(f, r);
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
            wah_jit_sync_pins(f, true);
            const wah_jit_arg_t a[] = { { WAH_JA_CTX, 0 }, { WAH_JA_IMM, in->imm.u32 }, { WAH_JA_SLOT, f->h - 1 } };
            wah_jit_call_helper(f, (uintptr_t)wah_jit_memory_grow, a, 3, false);
            wah_jit_reload_memory(f);
            wah_jit_sync_pins(f, false); // Doesn't change the return register
            wah_jit_reset(f, f->h - 1);
            f->gpr[WAH_JR_RET] = WAH_JO_TEMP;
            wah_jit_push_gpr(f, WAH_JR_RET); // Already zero-extended if i32
            break;
        }
#if ((WAH_COMPILED_FEATURES) & WAH_FEATURE_BULK_MEMORY)
        case WAH_OP_MEMORY_FILL: case WAH_OP_MEMORY_COPY: case WAH_OP_MEMORY_INIT: {
            wah_jit_flush(f);
            wah_jit_sync_pins(f, true);
            uint32_t start = wah_jit_new_label(f);
            wah_jit_bind(f, start); // Resumes here with operands and pinned locals in slots
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
            wah_jit_sync_pins(f, false);
            wah_jit_reset(f, f->h - 3);
            break;
        }
        case WAH_OP_DATA_DROP: {
            wah_jit_flush(f);
            wah_jit_sync_pins(f, true);
            const wah_jit_arg_t a[] = { { WAH_JA_CTX, 0 }, { WAH_JA_IMM, in->imm.u32 } };
            wah_jit_call_helper(f, (uintptr_t)wah_jit_data_drop, a, 2, false);
            wah_jit_sync_pins(f, false);
            wah_jit_reset(f, f->h);
            break;
        }
#endif

        case WAH_OP_I32_CONST: {
            int64_t v = WAH_JIT_ZEXT_I32 ? (int64_t)(uint32_t)in->imm.i32 : in->imm.i32;
            wah_jit_push(f, (wah_jit_val_t){ .kind = WAH_JV_CONST, .imm = v });
            break;
        }
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

        case WAH_OP_I32_WRAP_I64: wah_jit_zext_top(f); break; // i32 operations ignore the upper half otherwise
        case WAH_OP_I64_EXTEND_I32_S: WAH_IF_SIGN_EXT(case WAH_OP_I64_EXTEND32_S:) wah_jit_extend(f, true, 32, true); break;
        case WAH_OP_I64_EXTEND_I32_U: wah_jit_extend(f, true, 32, false); break;
#if ((WAH_COMPILED_FEATURES) & WAH_FEATURE_SIGN_EXT)
        case WAH_OP_I32_EXTEND8_S: wah_jit_extend(f, false, 8, true); break;
        case WAH_OP_I32_EXTEND16_S: wah_jit_extend(f, false, 16, true); break;
        case WAH_OP_I64_EXTEND8_S: wah_jit_extend(f, true, 8, true); break;
        case WAH_OP_I64_EXTEND16_S: wah_jit_extend(f, true, 16, true); break;
#endif

        case WAH_OP_I32_TRUNC_F32_S: wah_jit_trunc(f, 0, false, true, -2147483648.0, true, 2147483648.0); break;
        case WAH_OP_I32_TRUNC_F32_U: wah_jit_trunc(f, 0, false, false, -1.0, false, 4294967296.0); break;
        case WAH_OP_I32_TRUNC_F64_S: wah_jit_trunc(f, 1, false, true, -2147483649.0, false, 2147483648.0); break;
        case WAH_OP_I32_TRUNC_F64_U: wah_jit_trunc(f, 1, false, false, -1.0, false, 4294967296.0); break;
        case WAH_OP_I64_TRUNC_F32_S: wah_jit_trunc(f, 0, true, true, -9223372036854775808.0, true, 9223372036854775808.0); break;
        case WAH_OP_I64_TRUNC_F32_U: wah_jit_trunc(f, 0, true, false, -1.0, false, 18446744073709551616.0); break;
        case WAH_OP_I64_TRUNC_F64_S: wah_jit_trunc(f, 1, true, true, -9223372036854775808.0, true, 9223372036854775808.0); break;
        case WAH_OP_I64_TRUNC_F64_U: wah_jit_trunc(f, 1, true, false, -1.0, false, 18446744073709551616.0); break;
#if ((WAH_COMPILED_FEATURES) & WAH_FEATURE_NONTRAPPING_F2I)
        case WAH_OP_I32_TRUNC_SAT_F32_S: wah_jit_trunc_sat(f, 0, false, true); break;
        case WAH_OP_I32_TRUNC_SAT_F32_U: wah_jit_trunc_sat(f, 0, false, false); break;
        case WAH_OP_I32_TRUNC_SAT_F64_S: wah_jit_trunc_sat(f, 1, false, true); break;
        case WAH_OP_I32_TRUNC_SAT_F64_U: wah_jit_trunc_sat(f, 1, false, false); break;
        case WAH_OP_I64_TRUNC_SAT_F32_S: wah_jit_trunc_sat(f, 0, true, true); break;
        case WAH_OP_I64_TRUNC_SAT_F32_U: wah_jit_trunc_sat(f, 0, true, false); break;
        case WAH_OP_I64_TRUNC_SAT_F64_S: wah_jit_trunc_sat(f, 1, true, true); break;
        case WAH_OP_I64_TRUNC_SAT_F64_U: wah_jit_trunc_sat(f, 1, true, false); break;
#endif
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
            if (wah_jit_is_fpr(f->vals[f->h - 1].kind)) wah_jit_to_gpr(f, f->h - 1);
            if (op == WAH_OP_I32_REINTERPRET_F32) wah_jit_zext_top(f); // f32 values may have anything above
            break;
        case WAH_OP_F32_REINTERPRET_I32: case WAH_OP_F64_REINTERPRET_I64:
            if (f->vals[f->h - 1].kind == WAH_JV_GPR || f->vals[f->h - 1].kind == WAH_JV_LGPR ||
                f->vals[f->h - 1].kind == WAH_JV_FLAGS) wah_jit_to_fpr(f, f->h - 1);
            break;

        default:
#if ((WAH_COMPILED_FEATURES) & WAH_FEATURE_SIMD)
            if (op >= WAH_FD && op < WAH_FE) { wah_jit_simd(f, in, fuse); break; }
#endif
            WAH_JIT_FAIL(f, "unhandled opcode");
            break;
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

    uint32_t start = jm->len, call_start = jm->call_count;
    for (int attempt = 0; attempt < 2; attempt++) {
        *f = (wah_jit_fn_t){ .jm = jm, .alloc = &module->alloc, .module = module, .ft = f->ft, .long_branches = attempt > 0,
                             .hint = -1 };
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
    wah_free(f->alloc, f->pin_of);
    wah_free(f->alloc, f->vals);
    wah_free(f->alloc, f->ctl);
    wah_free(f->alloc, f->labels);
    wah_free(f->alloc, f->patches);
    wah_free(f->alloc, f->stubs);
    wah_free(f->alloc, f->vconsts);
}

// Pins the most used locals to registers, weighting uses by loop depth as each architecture prefers.
static void wah_jit_choose_pins(wah_jit_fn_t *f, const wah_code_body_t *body, const wah_analyzed_code_t *ac) {
    if (WAH_JIT_PIN_GPR_COUNT + WAH_JIT_PIN_FPR_COUNT == 0 || f->nlocals == 0) return;
    void *p = NULL;
    if (wah_malloc(f->alloc, f->nlocals, sizeof(uint64_t), &p) != WAH_OK) return; // Not pinned then
    uint64_t *score = (uint64_t *)p;
    memset(score, 0, sizeof(uint64_t) * f->nlocals);
    uint32_t depth = 0, loops[WAH_MAX_CONTROL_DEPTH + 1] = {0}, nblocks = 0;
    for (uint32_t i = 0; i < ac->instr_count; i++) {
        const wah_decoded_instr_t *in = &ac->instrs[i];
        switch (in->opcode) {
            case WAH_OP_BLOCK: case WAH_OP_IF: case WAH_OP_LOOP:
                if (nblocks <= WAH_MAX_CONTROL_DEPTH) loops[nblocks] = in->opcode == WAH_OP_LOOP;
                depth += in->opcode == WAH_OP_LOOP;
                nblocks++;
                break;
            case WAH_OP_END:
                if (nblocks > 0 && --nblocks <= WAH_MAX_CONTROL_DEPTH) depth -= loops[nblocks];
                break;
            case WAH_OP_LOCAL_GET: case WAH_OP_LOCAL_SET: case WAH_OP_LOCAL_TEE:
                score[in->imm.u32] += WAH_JIT_USE_WEIGHT(depth);
                break;
            default: break;
        }
    }
    for (int fp = 0; fp < 2; fp++) {
        const uint8_t *regs = fp ? wah_jit_pin_fprs : wah_jit_pin_gprs;
        uint32_t n = fp ? WAH_JIT_PIN_FPR_COUNT : WAH_JIT_PIN_GPR_COUNT - !f->jm->guarded;
        for (uint32_t k = 0; k < n; k++) {
            uint32_t best = UINT32_MAX;
            for (uint32_t i = 0; i < f->nlocals; i++) {
                wah_type_t t = wah_jit_local_type(f, body, i);
                if (score[i] > 0 && (t != WAH_TYPE_I32 && t != WAH_TYPE_I64) == fp &&
                    (best == UINT32_MAX || score[i] > score[best])) best = i;
            }
            if (best == UINT32_MAX) break;
            score[best] = 0;
            if (!f->pin_of) {
                if (wah_malloc(f->alloc, f->nlocals, sizeof(int8_t), &p) != WAH_OK) break;
                f->pin_of = (int8_t *)p;
                memset(f->pin_of, -1, f->nlocals);
            }
            bool vec = wah_jit_local_type(f, body, best) == WAH_TYPE_V128;
            f->pin_of[best] = (int8_t)(regs[k] | (fp ? WAH_JIT_PIN_FP : 0) | (vec ? WAH_JIT_PIN_VEC : 0));
            f->pins[f->npins++] = (wah_jit_pin_t){ .idx = best, .reg = regs[k], .fp = fp != 0, .vec = vec };
            if (fp) f->fpr_pins |= 1u << regs[k];
            else f->gpr_pins |= 1u << regs[k];
        }
    }
    wah_free(f->alloc, score);
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
    f->instrs = ac->instrs;
    f->ninstrs = ac->instr_count;
    wah_jit_choose_pins(f, body, ac);
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
    // Pinned declared locals are zeroed in registers only, as their slots are not read until stored
    uint32_t first = f->ft->param_count, nzero = 0;
    for (uint32_t i = first; i < f->nlocals; i++) nzero += !f->pin_of || f->pin_of[i] < 0;
    if (nzero <= 8) {
        for (uint32_t i = first; i < f->nlocals; i++) {
            if (f->pin_of && f->pin_of[i] >= 0) continue;
            wah_jit_store_imm(f, WAH_JR_FP, (uint64_t)i * sizeof(wah_value_t), 0);
            if (wah_jit_local_type(f, body, i) == WAH_TYPE_V128) {
                wah_jit_store_imm(f, WAH_JR_FP, (uint64_t)i * sizeof(wah_value_t) + 8, 0);
            }
        }
    } else {
        wah_jit_zero_loop(f, first * (uint32_t)sizeof(wah_value_t), body->local_count);
    }
    for (uint32_t i = 0; i < f->npins; i++) {
        const wah_jit_pin_t *p = &f->pins[i];
        if (p->idx < first) {
            if (p->vec) wah_jit_vldst(f, false, p->reg, WAH_JR_FP, p->idx * (uint32_t)sizeof(wah_value_t));
            else wah_jit_ldst(f, false, p->fp, true, p->reg, WAH_JR_FP, p->idx * (uint32_t)sizeof(wah_value_t));
        } else if (p->fp) { // Also zeroes upper bits for v128
            wah_jit_mov_imm(f, WAH_JR_T0, 0);
            wah_jit_gpr_to_fpr(f, p->reg, WAH_JR_T0);
        } else {
            wah_jit_mov_imm(f, p->reg, 0);
        }
    }
    wah_jit_poll_check(f);

    for (uint32_t i = 0; i < ac->instr_count && !f->fail; i++) {
        f->instr_idx = i;
        wah_jit_instr(f, body, &ac->instrs[i], i + 1 < ac->instr_count ? &ac->instrs[i + 1] : NULL);
    }
    if (!f->dead || f->nctl != 0) WAH_JIT_FAIL(f, "unbalanced control"); // The function-level END should have been compiled
    wah_jit_emit_stubs(f);
    wah_jit_emit_vconsts(f);
    wah_jit_resolve_patches(f);
}

////////////////////////////////////////////////////////////////////////////////
// Guard pages /////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Guarded modules and memories are only made once this is WAH_HOST_HANDLES_FAULT or WAH_HANDLES_FAULT.
// It is 1 while installing handlers, which only one thread can do.
#ifdef _MSC_VER
static volatile long wah_jit_guard_mode;
static int wah_jit_guard_load(void) { return (int)_InterlockedCompareExchange(&wah_jit_guard_mode, 0, 0); }
static void wah_jit_guard_store(int v) { _InterlockedExchange(&wah_jit_guard_mode, v); }
static bool wah_jit_guard_cas(int old, int v) { return _InterlockedCompareExchange(&wah_jit_guard_mode, v, old) == old; }
#define WAH_JIT_THREAD_LOCAL __declspec(thread)
#else
static int wah_jit_guard_mode;
static int wah_jit_guard_load(void) { return __atomic_load_n(&wah_jit_guard_mode, __ATOMIC_ACQUIRE); }
static void wah_jit_guard_store(int v) { __atomic_store_n(&wah_jit_guard_mode, v, __ATOMIC_RELEASE); }
static bool wah_jit_guard_cas(int old, int v) {
    return __atomic_compare_exchange_n(&wah_jit_guard_mode, &old, v, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
}
#define WAH_JIT_THREAD_LOCAL __thread
#endif

static bool wah_jit_guard_enabled(void) { return wah_jit_guard_load() >= WAH_HOST_HANDLES_FAULT; }

// Innermost native activation of a guarded module in this thread, read by the fault handler
typedef struct wah_jit_activation_s {
    const wah_jit_module_t *jm;
    const wah_exec_context_t *ctx;
    struct wah_jit_activation_s *prev;
} wah_jit_activation_t;
static WAH_JIT_THREAD_LOCAL wah_jit_activation_t *wah_jit_activation;

static uint8_t *wah_jit_reserve_memory(uint64_t size) {
    if (!wah_jit_guard_enabled()) return NULL;
#ifdef _WIN32
    void *p = VirtualAlloc(NULL, WAH_JIT_GUARD_RESERVE, MEM_RESERVE, PAGE_NOACCESS);
    if (!p) return NULL;
    if (size > 0 && !VirtualAlloc(p, (size_t)size, MEM_COMMIT, PAGE_READWRITE)) {
        VirtualFree(p, 0, MEM_RELEASE);
        return NULL;
    }
#else
    int flags = MAP_PRIVATE | MAP_ANONYMOUS;
#ifdef MAP_NORESERVE
    flags |= MAP_NORESERVE;
#endif
    void *p = mmap(NULL, WAH_JIT_GUARD_RESERVE, PROT_NONE, flags, -1, 0);
    if (p == MAP_FAILED) return NULL;
    if (size > 0 && mprotect(p, (size_t)size, PROT_READ | PROT_WRITE) != 0) {
        munmap(p, WAH_JIT_GUARD_RESERVE);
        return NULL;
    }
#endif
    return (uint8_t *)p;
}

static bool wah_jit_commit_memory(uint8_t *data, uint64_t old_size, uint64_t new_size) {
#ifdef _WIN32
    return VirtualAlloc(data + old_size, (size_t)(new_size - old_size), MEM_COMMIT, PAGE_READWRITE) != NULL;
#else
    return mprotect(data + old_size, (size_t)(new_size - old_size), PROT_READ | PROT_WRITE) == 0;
#endif
}

static void wah_jit_release_memory(uint8_t *data) {
#ifdef _WIN32
    VirtualFree(data, 0, MEM_RELEASE);
#else
    munmap(data, WAH_JIT_GUARD_RESERVE);
#endif
}

// Whether a fault at pc accessing addr is an out-of-bounds access of the current activation, and where to resume.
static bool wah_jit_fault(uintptr_t pc, uintptr_t addr, uintptr_t *resume) {
    const wah_jit_activation_t *a = wah_jit_activation;
    if (!a || pc - (uintptr_t)a->jm->code >= a->jm->code_size) return false;
    for (uint32_t i = 0; i < a->ctx->memory_count; i++) {
        // Imported memories share the data and size of their owners, which are all guarded except for memory64
        if (wah_memory_type(a->ctx->module, i)->addr_type == WAH_TYPE_I64) continue;
        const wah_memory_inst_t *m = &a->ctx->memories[i];
        if (addr - (uintptr_t)m->data - m->size < WAH_JIT_GUARD_RESERVE - m->size) {
            *resume = (uintptr_t)a->jm->code + a->jm->oob_pos;
            return true;
        }
    }
    return false;
}

#ifdef _WIN32

bool wah_handle_fault(int signo, void *info, void *uctx) {
    (void)signo; (void)uctx;
    EXCEPTION_POINTERS *ep = (EXCEPTION_POINTERS *)info;
    if (!ep) return false;
    const EXCEPTION_RECORD *er = ep->ExceptionRecord;
    if (er->ExceptionCode != EXCEPTION_ACCESS_VIOLATION || er->NumberParameters < 2) return false;
    uintptr_t resume;
    if (!wah_jit_fault((uintptr_t)ep->ContextRecord->Rip, (uintptr_t)er->ExceptionInformation[1], &resume)) return false;
    ep->ContextRecord->Rip = resume;
    return true;
}

static LONG CALLBACK wah_jit_exception_handler(EXCEPTION_POINTERS *ep) {
    return wah_handle_fault(0, ep, NULL) ? EXCEPTION_CONTINUE_EXECUTION : EXCEPTION_CONTINUE_SEARCH;
}

static wah_error_t wah_jit_install_fault_handler(void) {
    return AddVectoredExceptionHandler(1, wah_jit_exception_handler) ? WAH_OK : WAH_ERROR_UNSUPPORTED;
}

#else

#if defined(__APPLE__) && defined(WAH_JIT_X86_64)
#define WAH_JIT_UCTX_PC(uc) ((ucontext_t *)(uc))->uc_mcontext->__ss.__rip
#elif defined(__APPLE__)
#define WAH_JIT_UCTX_PC(uc) ((ucontext_t *)(uc))->uc_mcontext->__ss.__pc
#elif defined(WAH_JIT_X86_64)
#define WAH_JIT_UCTX_PC(uc) ((ucontext_t *)(uc))->uc_mcontext.gregs[16] // REG_RIP, only named with _GNU_SOURCE
#else
#define WAH_JIT_UCTX_PC(uc) ((ucontext_t *)(uc))->uc_mcontext.pc
#endif

bool wah_handle_fault(int signo, void *info, void *uctx) {
    if ((signo != SIGSEGV && signo != SIGBUS) || !info || !uctx) return false;
    uintptr_t resume;
    if (!wah_jit_fault((uintptr_t)WAH_JIT_UCTX_PC(uctx), (uintptr_t)((siginfo_t *)info)->si_addr, &resume)) return false;
    WAH_JIT_UCTX_PC(uctx) = resume;
    return true;
}

static struct sigaction wah_jit_prev_segv, wah_jit_prev_bus;

static void wah_jit_signal_handler(int signo, siginfo_t *info, void *uctx) {
    if (wah_handle_fault(signo, info, uctx)) return;
    const struct sigaction *prev = signo == SIGBUS ? &wah_jit_prev_bus : &wah_jit_prev_segv;
    if (prev->sa_flags & SA_SIGINFO) {
        prev->sa_sigaction(signo, info, uctx);
    } else if (prev->sa_handler == SIG_DFL || prev->sa_handler == SIG_IGN) {
        sigaction(signo, prev, NULL); // The faulting instruction will be retried and handled by it
    } else {
        prev->sa_handler(signo);
    }
}

static wah_error_t wah_jit_install_fault_handler(void) {
    struct sigaction sa = {0};
    sa.sa_sigaction = wah_jit_signal_handler;
    sa.sa_flags = SA_SIGINFO | SA_ONSTACK | SA_NODEFER;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGSEGV, &sa, &wah_jit_prev_segv) != 0) return WAH_ERROR_UNSUPPORTED;
    if (sigaction(SIGBUS, &sa, &wah_jit_prev_bus) != 0) {
        sigaction(SIGSEGV, &wah_jit_prev_segv, NULL);
        return WAH_ERROR_UNSUPPORTED;
    }
    return WAH_OK;
}

#endif

wah_error_t wah_enable_guard_pages(wah_fault_handling_t mode) {
    WAH_ENSURE(mode == WAH_HOST_HANDLES_FAULT || mode == WAH_HANDLES_FAULT, WAH_ERROR_MISUSE);
    int cur;
    do { // Spins only while another thread is installing handlers
        cur = wah_jit_guard_load();
        if (cur == WAH_HANDLES_FAULT || (cur == WAH_HOST_HANDLES_FAULT && mode == WAH_HOST_HANDLES_FAULT)) return WAH_OK;
    } while (cur == 1 || !wah_jit_guard_cas(cur, mode == WAH_HANDLES_FAULT ? 1 : (int)mode));
    if (mode == WAH_HOST_HANDLES_FAULT) return WAH_OK;
    wah_error_t err = wah_jit_install_fault_handler();
    wah_jit_guard_store(err == WAH_OK ? WAH_HANDLES_FAULT : cur);
    return err;
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
    jm->sse42 = (ecx1 >> 20) & 1;
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
    if (!f->jm->guarded) wah_jit_ldst(f, false, false, true, WAH_JR_MEMSZ, WAH_JR_CTX, WAH_JIT_CTX_OFF(memory_size));
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
    wah_a64(f, 0xA9800000u | (uint32_t)(-(int)sizeof(wah_a64_frame_t) / 8 & 0x7f) << 15 | 30u << 10 |
               WAH_A64_SP << 5 | 29u); // STP x29, x30, [sp, #-size]!
    wah_a64_addsub_imm(f, WAH_A64_ADDI, true, 29, WAH_A64_SP, 0, false);
    for (int i = 0; i < 5; i++) { // STP x19+2i, x20+2i, [sp, #x19+16i]
        wah_a64(f, 0xA9000000u | (WAH_A64_FRAME_OFF(x19) / 8 + 2 * i) << 15 |
                   (uint32_t)(20 + 2 * i) << 10 | WAH_A64_SP << 5 | (uint32_t)(19 + 2 * i));
    }
    for (int i = 0; i < 4; i++) { // STP d8+2i, d9+2i, [sp, #d8+16i]
        wah_a64(f, 0x6D000000u | (WAH_A64_FRAME_OFF(d8) / 8 + 2 * i) << 15 |
                   (uint32_t)(9 + 2 * i) << 10 | WAH_A64_SP << 5 | (uint32_t)(8 + 2 * i));
    }
    wah_a64_mov(f, true, WAH_JR_CTX, 0);
    wah_a64_mov(f, true, WAH_JR_FP, 3);
    wah_a64(f, 0xD53B4400u | WAH_JR_T0); // MRS x16, FPCR
    wah_a64_ldst(f, WAH_A64_STRX, WAH_JR_T0, WAH_A64_SP, WAH_A64_FRAME_OFF(fpcr_host));
    wah_a64_set_fpcr(f);
    wah_a64_ldst(f, WAH_A64_LDRX, WAH_JR_MEM, WAH_JR_CTX, WAH_JIT_CTX_OFF(memory_base));
    if (!f->jm->guarded) wah_a64_ldst(f, WAH_A64_LDRX, WAH_JR_MEMSZ, WAH_JR_CTX, WAH_JIT_CTX_OFF(memory_size));
    wah_a64(f, 0xB5000000u | 4u << 5 | 2u); // CBNZ x2, BR x2 below
    wah_a64(f, 0xD63F0000u | 1u << 5);      // BLR x1, returning here
    wah_a64_movw(f, WAH_A64_MOVZ, false, 0, 0, 0);
    wah_a64(f, 0x14000002u);                // B exit
    wah_a64(f, 0xD61F0000u | 2u << 5);      // BR x2
    f->jm->exit_pos = wah_jit_pos(f);
    wah_a64_ldst(f, WAH_A64_LDRX, WAH_JR_T0, WAH_A64_SP, WAH_A64_FRAME_OFF(fpcr_host));
    wah_a64(f, 0xD51B4400u | WAH_JR_T0); // MSR FPCR, x16
    for (int i = 3; i >= 0; i--) { // LDP
        wah_a64(f, 0x6D400000u | (WAH_A64_FRAME_OFF(d8) / 8 + 2 * i) << 15 |
                   (uint32_t)(9 + 2 * i) << 10 | WAH_A64_SP << 5 | (uint32_t)(8 + 2 * i));
    }
    for (int i = 4; i >= 0; i--) {
        wah_a64(f, 0xA9400000u | (WAH_A64_FRAME_OFF(x19) / 8 + 2 * i) << 15 |
                   (uint32_t)(20 + 2 * i) << 10 | WAH_A64_SP << 5 | (uint32_t)(19 + 2 * i));
    }
    wah_a64(f, 0xA8C00000u | (uint32_t)(sizeof(wah_a64_frame_t) / 8) << 15 | 30u << 10 |
               WAH_A64_SP << 5 | 29u); // LDP x29, x30, [sp], #size
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
    jm->guarded = wah_jit_guard_enabled();
    wah_jit_fn_t f = { .jm = jm, .alloc = &module->alloc };
    wah_jit_trampoline(&f);
    jm->oob_pos = wah_jit_pos(&f);
    wah_jit_exit_with(&f, WAH_ERROR_MEMORY_OUT_OF_BOUNDS);
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
// Code relying on guard pages also needs every memory to be guarded.
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
    for (uint32_t i = 0; m->jit->guarded && i < ctx->memory_count; i++) {
        if (wah_memory_type(m, i)->addr_type == WAH_TYPE_I64) continue; // Always checked
        const wah_memory_inst_t *mem = &ctx->memories[i];
        if (mem->is_imported) mem = mem->import_ctx ? &mem->import_ctx->memories[mem->import_idx] : NULL;
        if (!mem || !mem->guarded) return false;
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

// -fsanitize=function (in -fsanitize=undefined since Clang 17) reads a type signature just before the callee,
// which is unmapped for the trampoline at the start of the code.
#if defined(__clang__) && WAH_HAS_ATTRIBUTE(no_sanitize)
__attribute__((no_sanitize("function")))
#endif
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
        if (WAH_JIT_ZEXT_I32) { // Params may come from the host with anything above i32 values
            const wah_func_type_t *ft = &m->types[m->function_type_indices[frame->func_idx]];
            for (uint32_t i = 0; i < ft->param_count; i++) {
                if (ft->param_types[i] == WAH_TYPE_I32) fp[i].i64 = (uint32_t)fp[i].i32;
            }
        }
    }
    wah_jit_activation_t act = { .jm = jm, .ctx = ctx, .prev = wah_jit_activation };
    if (jm->guarded) wah_jit_activation = &act;
    wah_error_t err = ((wah_jit_entry_t)(uintptr_t)jm->code)(ctx, code, resume, fp);
    wah_jit_activation = act.prev;
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
