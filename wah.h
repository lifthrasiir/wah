// WebAssembly interpreter in a Header file (WAH)

#ifndef WAH_H
#define WAH_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>

#ifndef WAH_FORCE_PORTABLE
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#define WAH_X86_64
#include <immintrin.h>
#elif defined(__aarch64__) || defined(_M_ARM64)
#define WAH_AARCH64
#include <arm_neon.h>
#endif
#endif

// Assertions are only used for always-true conditions as a correctness check.
// Any possible runtime error should be checked with WAH_ENSURE etc. instead.
// Exception: Public interfaces without wah_error_t return type may use WAH_ASSERT as a last resort.
#ifndef WAH_ASSERT
#ifdef WAH_DEBUG
#define WAH_ASSERT(cond) assert(cond)
#else
#define WAH_ASSERT(cond) ((void)(cond))
#endif
#endif

typedef enum {
    WAH_OK = 0,
    WAH_ERROR_INVALID_MAGIC_NUMBER = -1,
    WAH_ERROR_INVALID_VERSION = -2,
    WAH_ERROR_UNEXPECTED_EOF = -3,
    WAH_ERROR_UNKNOWN_SECTION = -4,
    WAH_ERROR_TOO_LARGE = -5,
    WAH_ERROR_OUT_OF_MEMORY = -6,
    WAH_ERROR_VALIDATION_FAILED = -7,
    WAH_ERROR_TRAP = -8,
    WAH_ERROR_CALL_STACK_OVERFLOW = -9,
    WAH_ERROR_MEMORY_OUT_OF_BOUNDS = -10,
    WAH_ERROR_NOT_FOUND = -11,
    WAH_ERROR_MISUSE = -12,
    WAH_ERROR_BAD_SPEC = -13,
    WAH_ERROR_IMPORT_NOT_FOUND = -14,
    WAH_OK_BUT_MULTI_RETURN = 1,
} wah_error_t;

// 128-bit vector type
typedef union {
    uint8_t u8[16];
    uint16_t u16[8];
    uint32_t u32[4];
    uint64_t u64[2];
    int8_t i8[16];
    int16_t i16[8];
    int32_t i32[4];
    int64_t i64[2];
    float f32[4];
    double f64[2];
} wah_v128_t;

typedef union {
    int32_t i32;
    int64_t i64;
    float f32;
    double f64;
    wah_v128_t v128;
    void* ref;  // Unified reference type (externref or funcref as wah_function_t*)
    struct { void *sentinel; uint32_t func_idx; } prefuncref; // const_expr funcref intermediate

    // Internal fields
#ifdef WAH_X86_64
    __m128i _m128i;    // For integer SIMD operations
    __m128 _m128;      // For float SIMD operations
    __m128d _m128d;    // For double SIMD operations
#elif defined(WAH_AARCH64)
    uint8x16_t _u8x16;       // Universal 128-bit SIMD
#endif
} wah_value_t;

typedef int32_t wah_type_t;

#define WAH_TYPE_I32 -1
#define WAH_TYPE_I64 -2
#define WAH_TYPE_F32 -3
#define WAH_TYPE_F64 -4
#define WAH_TYPE_V128 -5
#define WAH_TYPE_FUNCREF -16
#define WAH_TYPE_EXTERNREF -17

#define WAH_TYPE_IS_FUNCTION(t) ((t) / 100 == -1)
#define WAH_TYPE_IS_MEMORY(t)   ((t) == -200)
#define WAH_TYPE_IS_TABLE(t)    ((t) == -300)
#define WAH_TYPE_IS_GLOBAL(t)   ((t) >= -5)
#define WAH_TYPE_IS_FUNCREF(t)  ((t) == WAH_TYPE_FUNCREF)
#define WAH_TYPE_IS_EXTERNREF(t) ((t) == WAH_TYPE_EXTERNREF)

typedef uint64_t wah_entry_id_t;

// Call context for host functions
typedef struct wah_call_context_s {
    struct wah_exec_context_s *exec;

    size_t nparams, nresults;
    const wah_value_t *params;
    wah_value_t *results;
    const wah_type_t *param_types, *result_types;

    wah_error_t trap_reason;
} wah_call_context_t;

// Host function types
typedef void (*wah_func_t)(wah_call_context_t *ctx, void *userdata);
typedef void (*wah_finalize_t)(void *userdata);

typedef struct {
    wah_entry_id_t id;
    wah_type_t type;
    const char *name;
    size_t name_len;
    bool is_mutable;

    // Semi-private fields:
    union {
        wah_value_t global_val; // For WAH_TYPE_IS_GLOBAL
        struct { // For WAH_TYPE_IS_MEMORY
            uint64_t min_pages, max_pages;
        } memory;
        struct { // For WAH_TYPE_IS_TABLE
            wah_type_t elem_type;
            uint32_t min_elements, max_elements;
        } table;
        struct { // For WAH_TYPE_IS_FUNCTION
            uint32_t param_count, result_count;
            const wah_type_t *param_types, *result_types;
        } func;
    } u;
} wah_entry_t;

typedef struct wah_func_import_s {
    char *module_name;
    size_t module_name_len;
    char *field_name;
    size_t field_name_len;
    uint32_t type_index;
} wah_func_import_t;

typedef struct wah_module_s {
    uint32_t type_count;
    uint32_t function_count;
    uint32_t code_count;
    uint32_t global_count;
    uint32_t memory_count;
    uint32_t table_count;
    uint32_t element_segment_count;
    uint32_t data_segment_count;
    uint32_t export_count;

    uint32_t start_function_idx;
    bool has_start_function;
    bool has_data_count_section; // True if data count section was present
    uint32_t min_data_segment_count_required;

    struct wah_func_type_s *types;
    uint32_t *function_type_indices; // Index into the types array
    struct wah_code_body_s *code_bodies;
    struct wah_global_s *globals;
    struct wah_memory_type_s *memories;
    struct wah_table_type_s *tables;
    struct wah_element_segment_s *element_segments;
    struct wah_data_segment_s *data_segments;
    struct wah_export_s *exports;

    // Unified function table: functions[0..function_count) are WASM functions,
    // functions[function_count..total_function_count) are host functions.
    uint32_t total_function_count; // function_count + number of host functions
    uint32_t function_capacity;    // allocated capacity of functions[]
    struct wah_function_s *functions; // unified function array

    // Import section
    uint32_t import_function_count;
    wah_func_import_t *func_imports; // [0..import_function_count-1]

    // Dynamic export array growth
    uint32_t capacity_exports;  // Capacity for dynamic export array growth
} wah_module_t;

typedef struct wah_exec_context_s {
    wah_value_t *value_stack;       // A single, large stack for operands and locals
    uint32_t sp;                    // Stack pointer for the value_stack (points to next free slot)
    uint32_t value_stack_capacity;

    struct wah_call_frame_s *call_stack; // The call frame stack
    uint32_t call_depth;            // Current call depth (top of the call_stack)
    uint32_t call_stack_capacity;

    uint32_t max_call_depth;        // Configurable max call depth

    wah_value_t *globals;           // Mutable global values
    uint32_t global_count;

    const struct wah_module_s *module;

    // Memory 0 fast path (kept in sync with memories[0]/memory_sizes[0])
    uint8_t *memory_base; // Pointer to memory 0 (for i32_i32_mem0 ops; will be moved to register)
    uint64_t memory_size; // Size of memory 0 in bytes

    // Multiple memories support (memories[0] == memory_base, memory_sizes[0] == memory_size)
    uint8_t **memories;      // Array[memory_count] of all memory base pointers
    uint64_t *memory_sizes;  // Array[memory_count] of all memory sizes in bytes
    uint32_t memory_count;   // Total number of instantiated memories

    wah_value_t **tables;
    uint32_t *table_sizes; // Array[table_count] of current table sizes (mutable, per-instance)
    uint32_t table_count;

    // Linkage support
    struct wah_linked_modules_s { const char *name; const wah_module_t *module; } *linked_modules;
    uint32_t linked_module_count;
    uint32_t linked_module_capacity;
    bool is_instantiated;

    // Runtime dispatch table (global function index space: imports + locals + hosts)
    struct wah_function_s *function_table;
    uint32_t function_table_count;
} wah_exec_context_t;

// Convert error code to human-readable string
const char *wah_strerror(wah_error_t err);

wah_error_t wah_parse_module(const uint8_t *wasm_binary, size_t binary_size, wah_module_t *module);

size_t wah_module_num_exports(const wah_module_t *module);
wah_error_t wah_module_export(const wah_module_t *module, size_t idx, wah_entry_t *out);
wah_error_t wah_module_export_by_name(const wah_module_t *module, const char *name, wah_entry_t *out);

wah_error_t wah_module_entry(const wah_module_t *module, wah_entry_id_t entry_id, wah_entry_t *out);

// Creates and initializes an execution context.
wah_error_t wah_exec_context_create(wah_exec_context_t *exec_ctx, const wah_module_t *module);

// Destroys and frees resources of an execution context.
void wah_exec_context_destroy(wah_exec_context_t *exec_ctx);

// The main entry point to call a WebAssembly function.
// func_idx can be either a function index or a wah_entry_id_t (always fits in uint64_t).
wah_error_t wah_call(wah_exec_context_t *exec_ctx, uint64_t func_idx, const wah_value_t *params, uint32_t param_count, wah_value_t *result);

// Entry point to call a WebAssembly function with multiple return values.
// func_idx can be either a function index or a wah_entry_id_t (always fits in uint64_t).
wah_error_t wah_call_multi(wah_exec_context_t *exec_ctx, uint64_t func_idx, const wah_value_t *params, uint32_t param_count, wah_value_t *results, uint32_t max_results, uint32_t *actual_results);

// --- Module Cleanup ---
void wah_free_module(wah_module_t *module);

// --- Programmatically create modules ---
wah_error_t wah_new_module(wah_module_t *mod);
wah_error_t wah_module_export_funcv(wah_module_t *mod, const char *name, size_t nparams, const wah_type_t *param_types, size_t nresults, const wah_type_t *result_types, wah_func_t func, void *userdata, wah_finalize_t finalize);
wah_error_t wah_module_export_func(wah_module_t *mod, const char *name, const char *types, wah_func_t func, void *userdata, wah_finalize_t finalize);
wah_error_t wah_module_export_memory(wah_module_t *mod, const char *name, uint64_t min_pages, uint64_t max_pages);
wah_error_t wah_module_export_global_i32(wah_module_t *mod, const char *name, bool mutable, int32_t init_value);
wah_error_t wah_module_export_global_i64(wah_module_t *mod, const char *name, bool mutable, int64_t init_value);
wah_error_t wah_module_export_global_f32(wah_module_t *mod, const char *name, bool mutable, float init_value);
wah_error_t wah_module_export_global_f64(wah_module_t *mod, const char *name, bool mutable, double init_value);
wah_error_t wah_module_export_global_v128(wah_module_t *mod, const char *name, bool mutable, const wah_v128_t *init_value);

// --- Call context for host functions ---

#define WAH_PARAM(ty, field) \
    WAH_ASSERT(ctx && "Call context is NULL"); \
    WAH_ASSERT(index < ctx->nparams && "Parameter index out of bounds"); \
    WAH_ASSERT(ctx->param_types[index] == ty && "Parameter type mismatch"); \
    return ctx->params[index].field
#define WAH_RESULT(ty, field) \
    WAH_ASSERT(ctx && "Call context is NULL"); \
    WAH_ASSERT(index < ctx->nresults && "Result index out of bounds"); \
    WAH_ASSERT(ctx->result_types[index] == ty && "Result type mismatch"); \
    ctx->results[index].field = value

static inline int32_t wah_param_i32(const wah_call_context_t *ctx, size_t index) { WAH_PARAM(WAH_TYPE_I32, i32); }
static inline int64_t wah_param_i64(const wah_call_context_t *ctx, size_t index) { WAH_PARAM(WAH_TYPE_I64, i64); }
static inline float wah_param_f32(const wah_call_context_t *ctx, size_t index) { WAH_PARAM(WAH_TYPE_F32, f32); }
static inline double wah_param_f64(const wah_call_context_t *ctx, size_t index) { WAH_PARAM(WAH_TYPE_F64, f64); }

static inline void wah_result_i32(wah_call_context_t *ctx, size_t index, int32_t value) { WAH_RESULT(WAH_TYPE_I32, i32); }
static inline void wah_result_i64(wah_call_context_t *ctx, size_t index, int64_t value) { WAH_RESULT(WAH_TYPE_I64, i64); }
static inline void wah_result_f32(wah_call_context_t *ctx, size_t index, float value) { WAH_RESULT(WAH_TYPE_F32, f32); }
static inline void wah_result_f64(wah_call_context_t *ctx, size_t index, double value) { WAH_RESULT(WAH_TYPE_F64, f64); }

#undef WAH_PARAM
#undef WAH_RESULT

// Convenience macros for single return values
#define wah_return_i32(ctx, value) wah_result_i32(ctx, 0, value)
#define wah_return_i64(ctx, value) wah_result_i64(ctx, 0, value)
#define wah_return_f32(ctx, value) wah_result_f32(ctx, 0, value)
#define wah_return_f64(ctx, value) wah_result_f64(ctx, 0, value)

void wah_trap(wah_call_context_t *ctx, wah_error_t reason);

// --- Linkage ---
// Fulfills the named import in the primary module of the context.
wah_error_t wah_link_module(wah_exec_context_t *ctx, const char *name, const wah_module_t *mod);

// Optional. Ensures that the execution context is fully instantiated.
// Any `wah_link_*` calls are now invalid. Any `wah_call` call will implicitly call this function.
wah_error_t wah_instantiate(wah_exec_context_t *ctx);

wah_error_t wah_module_entry(const wah_module_t *module, wah_entry_id_t entry_id, wah_entry_t *out);

// Accessors for wah_entry_t
static inline int32_t wah_entry_i32(const wah_entry_t *entry) {
    WAH_ASSERT(entry);
    return entry->type == WAH_TYPE_I32 ? entry->u.global_val.i32 : 0;
}

static inline int64_t wah_entry_i64(const wah_entry_t *entry) {
    WAH_ASSERT(entry);
    return entry->type == WAH_TYPE_I64 ? entry->u.global_val.i64 : 0;
}

static inline float wah_entry_f32(const wah_entry_t *entry) {
    WAH_ASSERT(entry);
    return entry->type == WAH_TYPE_F32 ? entry->u.global_val.f32 : NAN;
}

static inline double wah_entry_f64(const wah_entry_t *entry) {
    WAH_ASSERT(entry);
    return entry->type == WAH_TYPE_F64 ? entry->u.global_val.f64 : NAN;
}

static inline wah_error_t wah_entry_memory(const wah_entry_t *entry, uint64_t *min_pages, uint64_t *max_pages) {
    if (!entry) return WAH_ERROR_MISUSE;
    if (!min_pages) return WAH_ERROR_MISUSE;
    if (!max_pages) return WAH_ERROR_MISUSE;
    if (!WAH_TYPE_IS_MEMORY(entry->type)) return WAH_ERROR_MISUSE;
    *min_pages = entry->u.memory.min_pages;
    *max_pages = entry->u.memory.max_pages;
    return WAH_OK;
}

static inline wah_error_t wah_entry_table(const wah_entry_t *entry, wah_type_t *elem_type, uint32_t *min_elements, uint32_t *max_elements) {
    if (!entry) return WAH_ERROR_MISUSE;
    if (!elem_type) return WAH_ERROR_MISUSE;
    if (!min_elements) return WAH_ERROR_MISUSE;
    if (!max_elements) return WAH_ERROR_MISUSE;
    if (!WAH_TYPE_IS_TABLE(entry->type)) return WAH_ERROR_MISUSE;
    *elem_type = entry->u.table.elem_type; // Directly assign wah_type_t
    *min_elements = entry->u.table.min_elements;
    *max_elements = entry->u.table.max_elements;
    return WAH_OK;
}

static inline wah_error_t wah_entry_func(const wah_entry_t *entry,
                                         uint32_t *out_nargs, const wah_type_t **out_args,
                                         uint32_t *out_nrets, const wah_type_t **out_rets) {
    if (!entry) return WAH_ERROR_MISUSE;
    if (!out_nargs) return WAH_ERROR_MISUSE;
    if (!out_args) return WAH_ERROR_MISUSE;
    if (!out_nrets) return WAH_ERROR_MISUSE;
    if (!out_rets) return WAH_ERROR_MISUSE;
    if (!WAH_TYPE_IS_FUNCTION(entry->type)) return WAH_ERROR_MISUSE;
    *out_nargs = entry->u.func.param_count;
    *out_args = entry->u.func.param_types;
    *out_nrets = entry->u.func.result_count;
    *out_rets = entry->u.func.result_types;
    return WAH_OK;
}

////////////////////////////////////////////////////////////////////////////////

#ifdef WAH_IMPLEMENTATION

#include <string.h> // For memcpy, memset
#include <stdlib.h> // For malloc, free
#include <assert.h> // For assert
#include <stdint.h> // For INT32_MIN, INT32_MAX
#include <math.h> // For floating-point functions
#if defined(_MSC_VER)
#include <intrin.h> // For MSVC intrinsics
#endif

// Adopted from https://stackoverflow.com/a/48045656
#if __cplusplus <= 201703 && defined(__GNUC__) && !defined(__clang__) && !defined(__EDG__)
#define WAH_VA_OPT_SUPPORT(_) 0
#else
#define WAH_VA_ARG1(x,y,...) y
#define WAH_VA_EMPTY(...) WAH_VA_ARG1(__VA_OPT__(,)0,1,)
#define WAH_VA_OPT_SUPPORT WAH_VA_EMPTY
#endif

#ifdef WAH_DEBUG
#include <stdio.h>
#if WAH_VA_OPT_SUPPORT(?)
#define WAH_LOG(fmt, ...) printf("(%d) " fmt "\n", __LINE__ __VA_OPT__(,) __VA_ARGS__)
#else
#define WAH_LOG(fmt, ...) printf("(%d) " fmt "\n", __LINE__, ##__VA_ARGS__) // common extension
#endif
#else
#define WAH_LOG(fmt, ...) (void)(0)
#endif

#ifdef __has_builtin
#define WAH_HAS_BUILTIN(x) __has_builtin(x)
#else
#define WAH_HAS_BUILTIN(x) 0
#endif

#ifdef __has_attribute
#define WAH_HAS_ATTRIBUTE(x) __has_attribute(x)
#else
#define WAH_HAS_ATTRIBUTE(x) 0
#endif

#if WAH_HAS_BUILTIN(__builtin_unreachable) || __GNUC__ * 100 + __GNUC_MINOR__ >= 405
#define WAH_UNREACHABLE() __builtin_unreachable()
#else
#define WAH_UNREACHABLE() abort()
#endif

#if WAH_HAS_ATTRIBUTE(always_inline)
#define WAH_ALWAYS_INLINE __attribute__((always_inline)) inline
#else
#define WAH_ALWAYS_INLINE inline
#endif

#ifdef WAH_X86_64
#define WAH_IF_X86_64(then, ...) then

// Feature detection by compiler version
// Note: These indicate whether the compiler has the intrinsic/inline asm capability,
// not whether the target CPU supports it (that's checked at runtime).
#ifdef __GNUC__
#define WAH_GCC_VER (__GNUC__ * 100 + __GNUC_MINOR__)
#else
#define WAH_GCC_VER 0
#endif
#ifdef __clang__
#define WAH_CLANG_VER (__clang_major__ * 100 + __clang_minor__)
#else
#define WAH_CLANG_VER 0
#endif
#ifdef _MSC_VER
#define WAH_MSVC_VER _MSC_VER
#else
#define WAH_MSVC_VER 0
#endif
#define WAH_CHECK_COMPILER(gcc, clang, msvc) (WAH_GCC_VER >= gcc || WAH_CLANG_VER >= clang || WAH_MSVC_VER >= msvc)
#if WAH_CHECK_COMPILER(403, 206, 1500)
#define WAH_IF_SSSE3(...) __VA_ARGS__
#define WAH_IF_SSE41(...) __VA_ARGS__
#define WAH_IF_SSE42(...) __VA_ARGS__
#else
#define WAH_IF_SSSE3(...)
#define WAH_IF_SSE41(...)
#define WAH_IF_SSE42(...)
#endif
#if WAH_CHECK_COMPILER(404, 209, 1600)
#define WAH_IF_AVX(...) __VA_ARGS__
#else
#define WAH_IF_AVX(...)
#endif
#if WAH_CHECK_COMPILER(409, 303, 1800)
#define WAH_IF_AVX2(...) __VA_ARGS__
#else
#define WAH_IF_AVX2(...)
#endif
#if WAH_CHECK_COMPILER(700, 407, 1932)
#define WAH_IF_AVX512F(...) __VA_ARGS__
#define WAH_IF_AVX512VL(...) __VA_ARGS__
#define WAH_IF_AVX512BW(...) __VA_ARGS__
#else
#define WAH_IF_AVX512F(...)
#define WAH_IF_AVX512VL(...)
#define WAH_IF_AVX512BW(...)
#endif
#if WAH_CHECK_COMPILER(700, 600, 1932)
#define WAH_IF_AVX512IFMA(...) __VA_ARGS__
#else
#define WAH_IF_AVX512IFMA(...)
#endif

#else
#define WAH_IF_X86_64(then, ...) __VA_ARGS__
#endif

#ifdef WAH_AARCH64
#define WAH_IF_AARCH64(then, ...) then
#else
#define WAH_IF_AARCH64(then, ...) __VA_ARGS__
#endif

#define WAH_TYPE_FUNCTION -100
#define WAH_TYPE_HOST_FUNCTION -101
#define WAH_TYPE_MEMORY   -200
#define WAH_TYPE_TABLE    -300

#define WAH_TYPE_ANY -99

#define WAH_ENTRY_KIND_FUNCTION 0
#define WAH_ENTRY_KIND_TABLE    1
#define WAH_ENTRY_KIND_MEMORY   2
#define WAH_ENTRY_KIND_GLOBAL   3

#define WAH_MAKE_ENTRY_ID(kind, index) (((wah_entry_id_t)(kind) << 32) | (index))
#define WAH_GET_ENTRY_KIND(id)         ((uint32_t)((id) >> 32))
#define WAH_GET_ENTRY_INDEX(id)        ((uint32_t)((id) & 0xFFFFFFFF))

// Opcode-remapping constants for compact lookup table.
#define WAH_FB 0xd7 // 0xFB 0x00..1E -> 0xD7..F5
#define WAH_FC 0xf6 // 0xFC 0x00..11 -> 0xF6..107
#define WAH_FD 0x108 // 0xFD 0x00..113 -> 0x108..21B
#define WAH_FE 0x21c // (sentinel)

typedef enum {
    WAH_OPCLASS_, // Each opcode is in its own class by default

    // No opcode arguments, validation consists of pops and pushes only
    WAH_OPCLASS__I_I, WAH_OPCLASS__I_L, WAH_OPCLASS__I_F, WAH_OPCLASS__I_D, WAH_OPCLASS__I_V, WAH_OPCLASS__II_I, WAH_OPCLASS__IV_V,
    WAH_OPCLASS__L_I, WAH_OPCLASS__L_L, WAH_OPCLASS__L_F, WAH_OPCLASS__L_D, WAH_OPCLASS__L_V, WAH_OPCLASS__LL_I, WAH_OPCLASS__LL_L, WAH_OPCLASS__LV_V,
    WAH_OPCLASS__F_I, WAH_OPCLASS__F_L, WAH_OPCLASS__F_F, WAH_OPCLASS__F_V, WAH_OPCLASS__FF_I, WAH_OPCLASS__FF_F,
    WAH_OPCLASS__D_I, WAH_OPCLASS__D_L, WAH_OPCLASS__D_D, WAH_OPCLASS__D_V, WAH_OPCLASS__DD_I, WAH_OPCLASS__DD_L, WAH_OPCLASS__DD_D,
    WAH_OPCLASS__V_I, WAH_OPCLASS__V_V, WAH_OPCLASS__VI_V, WAH_OPCLASS__VV_V, WAH_OPCLASS__VVV_V,

    // Opcodes with immediate arguments to be pasted into the preparsed code
    WAH_OPCLASS_B, WAH_OPCLASS_I, WAH_OPCLASS_II, WAH_OPCLASS_M, WAH_OPCLASS_MB,
} wah_opclass_t;

// --- WebAssembly Opcodes (subset) ---
#define WAH_OPCODES(X) \
    /* Control Flow Operators */ \
    X(UNREACHABLE,, 0x00) X(NOP,, 0x01) X(BLOCK,, 0x02) X(LOOP,, 0x03) X(IF,, 0x04) X(ELSE,, 0x05) X(END,, 0x0B) \
    X(BR,, 0x0C) X(BR_IF,, 0x0D) X(BR_TABLE,, 0x0E) X(RETURN,, 0x0F) X(CALL,I, 0x10) X(CALL_INDIRECT,II, 0x11) \
    \
    /* Parametric Operators */ \
    X(DROP,, 0x1A) X(SELECT,, 0x1B) \
    \
    /* Variable Access */ \
    X(LOCAL_GET,I, 0x20) X(LOCAL_SET,I, 0x21) X(LOCAL_TEE,I, 0x22) X(GLOBAL_GET,I, 0x23) X(GLOBAL_SET,I, 0x24) \
    \
    /* Table Operations */ \
    X(TABLE_GET,I, 0x25) X(TABLE_SET,I, 0x26) \
    X(TABLE_INIT,II, WAH_FC+0x0C) X(ELEM_DROP,I, WAH_FC+0x0D) \
    X(TABLE_COPY,II, WAH_FC+0x0E) X(TABLE_GROW,I, WAH_FC+0x0F) \
    X(TABLE_SIZE,I, WAH_FC+0x10) X(TABLE_FILL,I, WAH_FC+0x11) \
    \
    /* Memory Operators */ \
    X(I32_LOAD,M, 0x28) X(I64_LOAD,M, 0x29) X(F32_LOAD,M, 0x2A) X(F64_LOAD,M, 0x2B) \
    X(I32_LOAD8_S,M, 0x2C) X(I32_LOAD8_U,M, 0x2D) X(I32_LOAD16_S,M, 0x2E) X(I32_LOAD16_U,M, 0x2F) \
    X(I64_LOAD8_S,M, 0x30) X(I64_LOAD8_U,M, 0x31) X(I64_LOAD16_S,M, 0x32) X(I64_LOAD16_U,M, 0x33) \
    X(I64_LOAD32_S,M, 0x34) X(I64_LOAD32_U,M, 0x35) \
    X(I32_STORE,M, 0x36) X(I64_STORE,M, 0x37) X(F32_STORE,M, 0x38) X(F64_STORE,M, 0x39) \
    X(I32_STORE8,M, 0x3A) X(I32_STORE16,M, 0x3B) X(I64_STORE8,M, 0x3C) X(I64_STORE16,M, 0x3D) X(I64_STORE32,M, 0x3E) \
    X(MEMORY_SIZE,I, 0x3F) X(MEMORY_GROW,I, 0x40) \
    X(MEMORY_INIT,II, WAH_FC+0x08) X(MEMORY_COPY,II, WAH_FC+0x0A) X(MEMORY_FILL,I, WAH_FC+0x0B) \
    \
    /* Vector Memory Operators */ \
    X(V128_LOAD,M, WAH_FD+0x00) \
    X(V128_LOAD8X8_S,M, WAH_FD+0x01) X(V128_LOAD8X8_U,M, WAH_FD+0x02) \
    X(V128_LOAD16X4_S,M, WAH_FD+0x03) X(V128_LOAD16X4_U,M, WAH_FD+0x04) \
    X(V128_LOAD32X2_S,M, WAH_FD+0x05) X(V128_LOAD32X2_U,M, WAH_FD+0x06) \
    X(V128_LOAD8_SPLAT,M, WAH_FD+0x07) X(V128_LOAD16_SPLAT,M, WAH_FD+0x08) \
    X(V128_LOAD32_SPLAT,M, WAH_FD+0x09) X(V128_LOAD64_SPLAT,M, WAH_FD+0x0A) \
    X(V128_LOAD32_ZERO,M, WAH_FD+0x5C) X(V128_LOAD64_ZERO,M, WAH_FD+0x5D) \
    X(V128_LOAD8_LANE,MB, WAH_FD+0x54) X(V128_LOAD16_LANE,MB, WAH_FD+0x55) \
    X(V128_LOAD32_LANE,MB, WAH_FD+0x56) X(V128_LOAD64_LANE,MB, WAH_FD+0x57) \
    X(V128_STORE,M, WAH_FD+0x0B) \
    \
    /* Vector Lane Operations */ \
    X(I8X16_SHUFFLE,, WAH_FD+0x0D) X(I8X16_SWIZZLE,_VV_V, WAH_FD+0x0E) \
    X(I8X16_EXTRACT_LANE_S,B, WAH_FD+0x15) X(I8X16_EXTRACT_LANE_U,B, WAH_FD+0x16) X(I8X16_REPLACE_LANE,B, WAH_FD+0x17) \
    X(I16X8_EXTRACT_LANE_S,B, WAH_FD+0x18) X(I16X8_EXTRACT_LANE_U,B, WAH_FD+0x19) X(I16X8_REPLACE_LANE,B, WAH_FD+0x1A) \
    X(I32X4_EXTRACT_LANE,B, WAH_FD+0x1B) X(I32X4_REPLACE_LANE,B, WAH_FD+0x1C) \
    X(I64X2_EXTRACT_LANE,B, WAH_FD+0x1D) X(I64X2_REPLACE_LANE,B, WAH_FD+0x1E) \
    X(F32X4_EXTRACT_LANE,B, WAH_FD+0x1F) X(F32X4_REPLACE_LANE,B, WAH_FD+0x20) \
    X(F64X2_EXTRACT_LANE,B, WAH_FD+0x21) X(F64X2_REPLACE_LANE,B, WAH_FD+0x22) \
    X(I8X16_SPLAT,_I_V, WAH_FD+0x0F) X(I16X8_SPLAT,_I_V, WAH_FD+0x10) X(I32X4_SPLAT,_I_V, WAH_FD+0x11) X(I64X2_SPLAT,_L_V, WAH_FD+0x12) \
    X(F32X4_SPLAT,_F_V, WAH_FD+0x13) X(F64X2_SPLAT,_D_V, WAH_FD+0x14) \
    \
    /* Constants */ \
    X(I32_CONST,, 0x41) X(I64_CONST,, 0x42) X(F32_CONST,, 0x43) X(F64_CONST,, 0x44) X(V128_CONST,, WAH_FD+0x0C) \
    \
    /* Comparison Operators */ \
    X(I32_EQZ,_I_I, 0x45) X(I32_EQ,_II_I, 0x46) X(I32_NE,_II_I, 0x47) \
    X(I32_LT_S,_II_I, 0x48) X(I32_LT_U,_II_I, 0x49) X(I32_GT_S,_II_I, 0x4A) X(I32_GT_U,_II_I, 0x4B) \
    X(I32_LE_S,_II_I, 0x4C) X(I32_LE_U,_II_I, 0x4D) X(I32_GE_S,_II_I, 0x4E) X(I32_GE_U,_II_I, 0x4F) \
    X(I64_EQZ,_L_I, 0x50) X(I64_EQ,_LL_I, 0x51) X(I64_NE,_LL_I, 0x52) \
    X(I64_LT_S,_LL_I, 0x53) X(I64_LT_U,_LL_I, 0x54) X(I64_GT_S,_LL_I, 0x55) X(I64_GT_U,_LL_I, 0x56) \
    X(I64_LE_S,_LL_I, 0x57) X(I64_LE_U,_LL_I, 0x58) X(I64_GE_S,_LL_I, 0x59) X(I64_GE_U,_LL_I, 0x5A) \
    X(F32_EQ,_FF_I, 0x5B) X(F32_NE,_FF_I, 0x5C) X(F32_LT,_FF_I, 0x5D) X(F32_GT,_FF_I, 0x5E) X(F32_LE,_FF_I, 0x5F) X(F32_GE,_FF_I, 0x60) \
    X(F64_EQ,_DD_I, 0x61) X(F64_NE,_DD_I, 0x62) X(F64_LT,_DD_I, 0x63) X(F64_GT,_DD_I, 0x64) X(F64_LE,_DD_I, 0x65) X(F64_GE,_DD_I, 0x66) \
    \
    /* Numeric Operators */ \
    X(I32_CLZ,_I_I, 0x67) X(I32_CTZ,_I_I, 0x68) X(I32_POPCNT,_I_I, 0x69) X(I32_ADD,_II_I, 0x6A) X(I32_SUB,_II_I, 0x6B) \
    X(I32_MUL,_II_I, 0x6C) X(I32_DIV_S,_II_I, 0x6D) X(I32_DIV_U,_II_I, 0x6E) X(I32_REM_S,_II_I, 0x6F) X(I32_REM_U,_II_I, 0x70) \
    X(I32_AND,_II_I, 0x71) X(I32_OR,_II_I, 0x72) X(I32_XOR,_II_I, 0x73) \
    X(I32_SHL,_II_I, 0x74) X(I32_SHR_S,_II_I, 0x75) X(I32_SHR_U,_II_I, 0x76) X(I32_ROTL,_II_I, 0x77) X(I32_ROTR,_II_I, 0x78) \
    X(I64_CLZ,_L_L, 0x79) X(I64_CTZ,_L_L, 0x7A) X(I64_POPCNT,_L_L, 0x7B) X(I64_ADD,_LL_L, 0x7C) X(I64_SUB,_LL_L, 0x7D) \
    X(I64_MUL,_LL_L, 0x7E) X(I64_DIV_S,_LL_L, 0x7F) X(I64_DIV_U,_LL_L, 0x80) X(I64_REM_S,_LL_L, 0x81) X(I64_REM_U,_LL_L, 0x82) \
    X(I64_AND,_LL_L, 0x83) X(I64_OR,_LL_L, 0x84) X(I64_XOR,_LL_L, 0x85) \
    X(I64_SHL,_LL_L, 0x86) X(I64_SHR_S,_LL_L, 0x87) X(I64_SHR_U,_LL_L, 0x88) X(I64_ROTL,_LL_L, 0x89) X(I64_ROTR,_LL_L, 0x8A) \
    X(F32_ABS,_F_F, 0x8B) X(F32_NEG,_F_F, 0x8C) X(F32_CEIL,_F_F, 0x8D) X(F32_FLOOR,_F_F, 0x8E) X(F32_TRUNC,_F_F, 0x8F) X(F32_NEAREST,_F_F, 0x90) \
    X(F32_SQRT,_F_F, 0x91) X(F32_ADD,_FF_F, 0x92) X(F32_SUB,_FF_F, 0x93) X(F32_MUL,_FF_F, 0x94) X(F32_DIV,_FF_F, 0x95) \
    X(F32_MIN,_FF_F, 0x96) X(F32_MAX,_FF_F, 0x97) X(F32_COPYSIGN,_FF_F, 0x98) \
    X(F64_ABS,_D_D, 0x99) X(F64_NEG,_D_D, 0x9A) X(F64_CEIL,_D_D, 0x9B) X(F64_FLOOR,_D_D, 0x9C) X(F64_TRUNC,_D_D, 0x9D) X(F64_NEAREST,_D_D, 0x9E) \
    X(F64_SQRT,_D_D, 0x9F) X(F64_ADD,_DD_D, 0xA0) X(F64_SUB,_DD_D, 0xA1) X(F64_MUL,_DD_D, 0xA2) X(F64_DIV,_DD_D, 0xA3) \
    X(F64_MIN,_DD_D, 0xA4) X(F64_MAX,_DD_D, 0xA5) X(F64_COPYSIGN,_DD_D, 0xA6) \
    \
    /* Vector Numeric Operators */ \
    X(V128_NOT,_V_V, WAH_FD+0x4D) X(V128_AND,_VV_V, WAH_FD+0x4E) X(V128_ANDNOT,_VV_V, WAH_FD+0x4F) X(V128_OR,_VV_V, WAH_FD+0x50) X(V128_XOR,_VV_V, WAH_FD+0x51) \
    X(V128_BITSELECT,_VVV_V, WAH_FD+0x52) X(V128_ANY_TRUE,_V_I, WAH_FD+0x53) \
    \
    X(I8X16_ABS,_V_V, WAH_FD+0x60) X(I8X16_NEG,_V_V, WAH_FD+0x61) X(I8X16_POPCNT,_V_V, WAH_FD+0x62) \
    X(I8X16_ALL_TRUE,_V_I, WAH_FD+0x63) X(I8X16_BITMASK,_V_I, WAH_FD+0x64) \
    X(I8X16_NARROW_I16X8_S,_VV_V, WAH_FD+0x65) X(I8X16_NARROW_I16X8_U,_VV_V, WAH_FD+0x66) \
    X(I8X16_SHL,_VI_V, WAH_FD+0x6B) X(I8X16_SHR_S,_VI_V, WAH_FD+0x6C) X(I8X16_SHR_U,_VI_V, WAH_FD+0x6D) \
    X(I8X16_ADD,_VV_V, WAH_FD+0x6E) X(I8X16_ADD_SAT_S,_VV_V, WAH_FD+0x6F) X(I8X16_ADD_SAT_U,_VV_V, WAH_FD+0x70) \
    X(I8X16_SUB,_VV_V, WAH_FD+0x71) X(I8X16_SUB_SAT_S,_VV_V, WAH_FD+0x72) X(I8X16_SUB_SAT_U,_VV_V, WAH_FD+0x73) \
    X(I8X16_MIN_S,_VV_V, WAH_FD+0x76) X(I8X16_MIN_U,_VV_V, WAH_FD+0x77) X(I8X16_MAX_S,_VV_V, WAH_FD+0x78) X(I8X16_MAX_U,_VV_V, WAH_FD+0x79) \
    X(I8X16_AVGR_U,_VV_V, WAH_FD+0x7B) \
    \
    X(I16X8_EXTADD_PAIRWISE_I8X16_S,_VV_V, WAH_FD+0x7C) X(I16X8_EXTADD_PAIRWISE_I8X16_U,_VV_V, WAH_FD+0x7D) \
    X(I16X8_ABS,_V_V, WAH_FD+0x80) X(I16X8_NEG,_V_V, WAH_FD+0x81) X(I16X8_Q15MULR_SAT_S,_VV_V, WAH_FD+0x82) \
    X(I16X8_ALL_TRUE,_V_I, WAH_FD+0x83) X(I16X8_BITMASK,_V_I, WAH_FD+0x84) \
    X(I16X8_NARROW_I32X4_S,_VV_V, WAH_FD+0x85) X(I16X8_NARROW_I32X4_U,_VV_V, WAH_FD+0x86) \
    X(I16X8_EXTEND_LOW_I8X16_S,_V_V, WAH_FD+0x87) X(I16X8_EXTEND_HIGH_I8X16_S,_V_V, WAH_FD+0x88) \
    X(I16X8_EXTEND_LOW_I8X16_U,_V_V, WAH_FD+0x89) X(I16X8_EXTEND_HIGH_I8X16_U,_V_V, WAH_FD+0x8A) \
    X(I16X8_SHL,_VI_V, WAH_FD+0x8B) X(I16X8_SHR_S,_VI_V, WAH_FD+0x8C) X(I16X8_SHR_U,_VI_V, WAH_FD+0x8D) \
    X(I16X8_ADD,_VV_V, WAH_FD+0x8E) X(I16X8_ADD_SAT_S,_VV_V, WAH_FD+0x8F) X(I16X8_ADD_SAT_U,_VV_V, WAH_FD+0x90) \
    X(I16X8_SUB,_VV_V, WAH_FD+0x91) X(I16X8_SUB_SAT_S,_VV_V, WAH_FD+0x92) X(I16X8_SUB_SAT_U,_VV_V, WAH_FD+0x93) X(I16X8_MUL,_VV_V, WAH_FD+0x95) \
    X(I16X8_MIN_S,_VV_V, WAH_FD+0x96) X(I16X8_MIN_U,_VV_V, WAH_FD+0x97) X(I16X8_MAX_S,_VV_V, WAH_FD+0x98) X(I16X8_MAX_U,_VV_V, WAH_FD+0x99) \
    X(I16X8_AVGR_U,_VV_V, WAH_FD+0x9B) \
    X(I16X8_EXTMUL_LOW_I8X16_S,_VV_V, WAH_FD+0x9C) X(I16X8_EXTMUL_HIGH_I8X16_S,_VV_V, WAH_FD+0x9D) \
    X(I16X8_EXTMUL_LOW_I8X16_U,_VV_V, WAH_FD+0x9E) X(I16X8_EXTMUL_HIGH_I8X16_U,_VV_V, WAH_FD+0x9F) \
    \
    X(I32X4_EXTADD_PAIRWISE_I16X8_S,_VV_V, WAH_FD+0x7E) X(I32X4_EXTADD_PAIRWISE_I16X8_U,_VV_V, WAH_FD+0x7F) \
    X(I32X4_ABS,_V_V, WAH_FD+0xA0) X(I32X4_NEG,_V_V, WAH_FD+0xA1) X(I32X4_ALL_TRUE,_V_I, WAH_FD+0xA3) X(I32X4_BITMASK,_V_I, WAH_FD+0xA4) \
    X(I32X4_EXTEND_LOW_I16X8_S,_V_V, WAH_FD+0xA7) X(I32X4_EXTEND_HIGH_I16X8_S,_V_V, WAH_FD+0xA8) \
    X(I32X4_EXTEND_LOW_I16X8_U,_V_V, WAH_FD+0xA9) X(I32X4_EXTEND_HIGH_I16X8_U,_V_V, WAH_FD+0xAA) \
    X(I32X4_SHL,_VI_V, WAH_FD+0xAB) X(I32X4_SHR_S,_VI_V, WAH_FD+0xAC) X(I32X4_SHR_U,_VI_V, WAH_FD+0xAD) \
    X(I32X4_ADD,_VV_V, WAH_FD+0xAE) X(I32X4_SUB,_VV_V, WAH_FD+0xB1) X(I32X4_MUL,_VV_V, WAH_FD+0xB5) \
    X(I32X4_MIN_S,_VV_V, WAH_FD+0xB6) X(I32X4_MIN_U,_VV_V, WAH_FD+0xB7) X(I32X4_MAX_S,_VV_V, WAH_FD+0xB8) X(I32X4_MAX_U,_VV_V, WAH_FD+0xB9) \
    X(I32X4_DOT_I16X8_S,_VV_V, WAH_FD+0xBA) \
    X(I32X4_EXTMUL_LOW_I16X8_S,_VV_V, WAH_FD+0xBC) X(I32X4_EXTMUL_HIGH_I16X8_S,_VV_V, WAH_FD+0xBD) \
    X(I32X4_EXTMUL_LOW_I16X8_U,_VV_V, WAH_FD+0xBE) X(I32X4_EXTMUL_HIGH_I16X8_U,_VV_V, WAH_FD+0xBF) \
    \
    X(I64X2_ABS,_V_V, WAH_FD+0xC0) X(I64X2_NEG,_V_V, WAH_FD+0xC1) X(I64X2_ALL_TRUE,_V_I, WAH_FD+0xC3) X(I64X2_BITMASK,_V_I, WAH_FD+0xC4) \
    X(I64X2_EXTEND_LOW_I32X4_S,_V_V, WAH_FD+0xC7) X(I64X2_EXTEND_HIGH_I32X4_S,_V_V, WAH_FD+0xC8) \
    X(I64X2_EXTEND_LOW_I32X4_U,_V_V, WAH_FD+0xC9) X(I64X2_EXTEND_HIGH_I32X4_U,_V_V, WAH_FD+0xCA) \
    X(I64X2_SHL,_VI_V, WAH_FD+0xCB) X(I64X2_SHR_S,_VI_V, WAH_FD+0xCC) X(I64X2_SHR_U,_VI_V, WAH_FD+0xCD) \
    X(I64X2_ADD,_VV_V, WAH_FD+0xCE) X(I64X2_SUB,_VV_V, WAH_FD+0xD1) X(I64X2_MUL,_VV_V, WAH_FD+0xD5) \
    X(I64X2_EXTMUL_LOW_I32X4_S,_VV_V, WAH_FD+0xDC) X(I64X2_EXTMUL_HIGH_I32X4_S,_VV_V, WAH_FD+0xDD) \
    X(I64X2_EXTMUL_LOW_I32X4_U,_VV_V, WAH_FD+0xDE) X(I64X2_EXTMUL_HIGH_I32X4_U,_VV_V, WAH_FD+0xDF) \
    \
    X(F32X4_CEIL,_V_V, WAH_FD+0x67) X(F32X4_FLOOR,_V_V, WAH_FD+0x68) X(F32X4_TRUNC,_V_V, WAH_FD+0x69) X(F32X4_NEAREST,_V_V, WAH_FD+0x6A) \
    X(F32X4_ABS,_V_V, WAH_FD+0xE0) X(F32X4_NEG,_V_V, WAH_FD+0xE1) X(F32X4_SQRT,_V_V, WAH_FD+0xE3) \
    X(F32X4_ADD,_VV_V, WAH_FD+0xE4) X(F32X4_SUB,_VV_V, WAH_FD+0xE5) X(F32X4_MUL,_VV_V, WAH_FD+0xE6) X(F32X4_DIV,_VV_V, WAH_FD+0xE7) \
    X(F32X4_MIN,_VV_V, WAH_FD+0xE8) X(F32X4_MAX,_VV_V, WAH_FD+0xE9) X(F32X4_PMIN,_VV_V, WAH_FD+0xEA) X(F32X4_PMAX,_VV_V, WAH_FD+0xEB) \
    \
    X(F64X2_CEIL,_V_V, WAH_FD+0x74) X(F64X2_FLOOR,_V_V, WAH_FD+0x75) X(F64X2_TRUNC,_V_V, WAH_FD+0x7A) X(F64X2_NEAREST,_V_V, WAH_FD+0x94) \
    X(F64X2_ABS,_V_V, WAH_FD+0xEC) X(F64X2_NEG,_V_V, WAH_FD+0xED) X(F64X2_SQRT,_V_V, WAH_FD+0xEF) \
    X(F64X2_ADD,_VV_V, WAH_FD+0xF0) X(F64X2_SUB,_VV_V, WAH_FD+0xF1) X(F64X2_MUL,_VV_V, WAH_FD+0xF2) X(F64X2_DIV,_VV_V, WAH_FD+0xF3) \
    X(F64X2_MIN,_VV_V, WAH_FD+0xF4) X(F64X2_MAX,_VV_V, WAH_FD+0xF5) X(F64X2_PMIN,_VV_V, WAH_FD+0xF6) X(F64X2_PMAX,_VV_V, WAH_FD+0xF7) \
    \
    X(I32X4_TRUNC_SAT_F32X4_S,_V_V, WAH_FD+0xF8) X(I32X4_TRUNC_SAT_F32X4_U,_V_V, WAH_FD+0xF9) \
    X(F32X4_CONVERT_I32X4_S,_V_V, WAH_FD+0xFA) X(F32X4_CONVERT_I32X4_U,_V_V, WAH_FD+0xFB) \
    X(I32X4_TRUNC_SAT_F64X2_S_ZERO,_V_V, WAH_FD+0xFC) X(I32X4_TRUNC_SAT_F64X2_U_ZERO,_V_V, WAH_FD+0xFD) \
    X(F64X2_CONVERT_LOW_I32X4_S,_V_V, WAH_FD+0xFE) X(F64X2_CONVERT_LOW_I32X4_U,_V_V, WAH_FD+0xFF) \
    X(F32X4_DEMOTE_F64X2_ZERO,_V_V, WAH_FD+0x5E) X(F64X2_PROMOTE_LOW_F32X4,_V_V, WAH_FD+0x5F) \
    \
    X(I8X16_RELAXED_SWIZZLE,_V_V, WAH_FD+0x100) \
    X(I32X4_RELAXED_TRUNC_F32X4_S,_V_V, WAH_FD+0x101) X(I32X4_RELAXED_TRUNC_F32X4_U,_V_V, WAH_FD+0x102) \
    X(I32X4_RELAXED_TRUNC_F64X2_S,_V_V, WAH_FD+0x103) X(I32X4_RELAXED_TRUNC_F64X2_U,_V_V, WAH_FD+0x104) \
    X(F32X4_RELAXED_MADD,_V_V, WAH_FD+0x105) X(F32X4_RELAXED_NMADD,_V_V, WAH_FD+0x106) \
    X(F64X2_RELAXED_MADD,_V_V, WAH_FD+0x107) X(F64X2_RELAXED_NMADD,_V_V, WAH_FD+0x108) \
    X(I8X16_RELAXED_LANESELECT,_V_V, WAH_FD+0x109) X(I16X8_RELAXED_LANESELECT,_V_V, WAH_FD+0x10A) \
    X(I32X4_RELAXED_LANESELECT,_V_V, WAH_FD+0x10B) X(I64X2_RELAXED_LANESELECT,_V_V, WAH_FD+0x10C) \
    X(F32X4_RELAXED_MIN,_V_V, WAH_FD+0x10D) X(F32X4_RELAXED_MAX,_V_V, WAH_FD+0x10E) \
    X(F64X2_RELAXED_MIN,_V_V, WAH_FD+0x10F) X(F64X2_RELAXED_MAX,_V_V, WAH_FD+0x110) \
    X(I16X8_RELAXED_Q15MULR_S,_V_V, WAH_FD+0x111) \
    X(I16X8_RELAXED_DOT_I8X16_I7X16_S,_V_V, WAH_FD+0x112) \
    X(I32X4_RELAXED_DOT_I8X16_I7X16_ADD_S,_V_V, WAH_FD+0x113) \
    \
    /* Vector Comparison Operators */ \
    X(I8X16_EQ,_V_V, WAH_FD+0x23) X(I8X16_NE,_V_V, WAH_FD+0x24) \
    X(I8X16_LT_S,_V_V, WAH_FD+0x25) X(I8X16_LT_U,_V_V, WAH_FD+0x26) X(I8X16_GT_S,_V_V, WAH_FD+0x27) X(I8X16_GT_U,_V_V, WAH_FD+0x28) \
    X(I8X16_LE_S,_V_V, WAH_FD+0x29) X(I8X16_LE_U,_V_V, WAH_FD+0x2A) X(I8X16_GE_S,_V_V, WAH_FD+0x2B) X(I8X16_GE_U,_V_V, WAH_FD+0x2C) \
    X(I16X8_EQ,_V_V, WAH_FD+0x2D) X(I16X8_NE,_V_V, WAH_FD+0x2E) \
    X(I16X8_LT_S,_V_V, WAH_FD+0x2F) X(I16X8_LT_U,_V_V, WAH_FD+0x30) X(I16X8_GT_S,_V_V, WAH_FD+0x31) X(I16X8_GT_U,_V_V, WAH_FD+0x32) \
    X(I16X8_LE_S,_V_V, WAH_FD+0x33) X(I16X8_LE_U,_V_V, WAH_FD+0x34) X(I16X8_GE_S,_V_V, WAH_FD+0x35) X(I16X8_GE_U,_V_V, WAH_FD+0x36) \
    X(I32X4_EQ,_V_V, WAH_FD+0x37) X(I32X4_NE,_V_V, WAH_FD+0x38) \
    X(I32X4_LT_S,_V_V, WAH_FD+0x39) X(I32X4_LT_U,_V_V, WAH_FD+0x3A) X(I32X4_GT_S,_V_V, WAH_FD+0x3B) X(I32X4_GT_U,_V_V, WAH_FD+0x3C) \
    X(I32X4_LE_S,_V_V, WAH_FD+0x3D) X(I32X4_LE_U,_V_V, WAH_FD+0x3E) X(I32X4_GE_S,_V_V, WAH_FD+0x3F) X(I32X4_GE_U,_V_V, WAH_FD+0x40) \
    X(I64X2_EQ,_V_V, WAH_FD+0xD6) X(I64X2_NE,_V_V, WAH_FD+0xD7) \
    X(I64X2_LT_S,_V_V, WAH_FD+0xD8) X(I64X2_GT_S,_V_V, WAH_FD+0xD9) X(I64X2_LE_S,_V_V, WAH_FD+0xDA) X(I64X2_GE_S,_V_V, WAH_FD+0xDB) \
    X(F32X4_EQ,_V_V, WAH_FD+0x41) X(F32X4_NE,_V_V, WAH_FD+0x42) \
    X(F32X4_LT,_V_V, WAH_FD+0x43) X(F32X4_GT,_V_V, WAH_FD+0x44) X(F32X4_LE,_V_V, WAH_FD+0x45) X(F32X4_GE,_V_V, WAH_FD+0x46) \
    X(F64X2_EQ,_V_V, WAH_FD+0x47) X(F64X2_NE,_V_V, WAH_FD+0x48) \
    X(F64X2_LT,_V_V, WAH_FD+0x49) X(F64X2_GT,_V_V, WAH_FD+0x4A) X(F64X2_LE,_V_V, WAH_FD+0x4B) X(F64X2_GE,_V_V, WAH_FD+0x4C) \
    \
    /* Conversion Operators */ \
    X(I32_WRAP_I64,_L_I, 0xA7) X(I32_TRUNC_F32_S,_F_I, 0xA8) X(I32_TRUNC_F32_U,_F_I, 0xA9) X(I32_TRUNC_F64_S,_D_I, 0xAA) X(I32_TRUNC_F64_U,_D_I, 0xAB) \
    X(I64_EXTEND_I32_S,_I_L, 0xAC) X(I64_EXTEND_I32_U,_I_L, 0xAD) \
    X(I64_TRUNC_F32_S,_F_L, 0xAE) X(I64_TRUNC_F32_U,_F_L, 0xAF) X(I64_TRUNC_F64_S,_D_L, 0xB0) X(I64_TRUNC_F64_U,_D_L, 0xB1) \
    X(F32_CONVERT_I32_S,_I_F, 0xB2) X(F32_CONVERT_I32_U,_I_F, 0xB3) X(F32_CONVERT_I64_S,_L_F, 0xB4) X(F32_CONVERT_I64_U,_L_F, 0xB5) X(F32_DEMOTE_F64,, 0xB6) \
    X(F64_CONVERT_I32_S,_I_D, 0xB7) X(F64_CONVERT_I32_U,_I_D, 0xB8) X(F64_CONVERT_I64_S,_L_D, 0xB9) X(F64_CONVERT_I64_U,_L_D, 0xBA) X(F64_PROMOTE_F32,, 0xBB) \
    X(I32_REINTERPRET_F32,_F_I, 0xBC) X(I64_REINTERPRET_F64,_D_L, 0xBD) X(F32_REINTERPRET_I32,_I_F, 0xBE) X(F64_REINTERPRET_I64,_L_D, 0xBF) \
    X(I32_EXTEND8_S,_I_I, 0xC0) X(I32_EXTEND16_S,_I_I, 0xC1) X(I64_EXTEND8_S,_L_L, 0xC2) X(I64_EXTEND16_S,_L_L, 0xC3) X(I64_EXTEND32_S,_L_L, 0xC4) \
    X(I32_TRUNC_SAT_F32_S,_F_I, WAH_FC+0x00) X(I32_TRUNC_SAT_F32_U,_F_I, WAH_FC+0x01) \
    X(I32_TRUNC_SAT_F64_S,_D_I, WAH_FC+0x02) X(I32_TRUNC_SAT_F64_U,_D_I, WAH_FC+0x03) \
    X(I64_TRUNC_SAT_F32_S,_F_L, WAH_FC+0x04) X(I64_TRUNC_SAT_F32_U,_F_L, WAH_FC+0x05) \
    X(I64_TRUNC_SAT_F64_S,_D_L, WAH_FC+0x06) X(I64_TRUNC_SAT_F64_U,_D_L, WAH_FC+0x07) \
    \
    /* Reference Types */ \
    X(REF_NULL,, 0xD0) X(REF_IS_NULL,, 0xD1) X(REF_FUNC,, 0xD2)

#define WAH_I32_MEM0_OPCODES_M(X) \
    X(I32_LOAD,i32_mem0) X(I64_LOAD,i32_mem0) X(F32_LOAD,i32_mem0) X(F64_LOAD,i32_mem0) \
    X(I32_LOAD8_S,i32_mem0) X(I32_LOAD8_U,i32_mem0) X(I32_LOAD16_S,i32_mem0) X(I32_LOAD16_U,i32_mem0) \
    X(I64_LOAD8_S,i32_mem0) X(I64_LOAD8_U,i32_mem0) X(I64_LOAD16_S,i32_mem0) X(I64_LOAD16_U,i32_mem0) \
    X(I64_LOAD32_S,i32_mem0) X(I64_LOAD32_U,i32_mem0) \
    X(I32_STORE,i32_mem0) X(I64_STORE,i32_mem0) X(F32_STORE,i32_mem0) X(F64_STORE,i32_mem0) \
    X(I32_STORE8,i32_mem0) X(I32_STORE16,i32_mem0) X(I64_STORE8,i32_mem0) X(I64_STORE16,i32_mem0) X(I64_STORE32,i32_mem0) \
    X(V128_LOAD,i32_mem0) \
    X(V128_LOAD8X8_S,i32_mem0) X(V128_LOAD8X8_U,i32_mem0) \
    X(V128_LOAD16X4_S,i32_mem0) X(V128_LOAD16X4_U,i32_mem0) \
    X(V128_LOAD32X2_S,i32_mem0) X(V128_LOAD32X2_U,i32_mem0) \
    X(V128_LOAD8_SPLAT,i32_mem0) X(V128_LOAD16_SPLAT,i32_mem0) X(V128_LOAD32_SPLAT,i32_mem0) X(V128_LOAD64_SPLAT,i32_mem0) \
    X(V128_LOAD32_ZERO,i32_mem0) X(V128_LOAD64_ZERO,i32_mem0) \
    X(V128_STORE,i32_mem0)
#define WAH_I32_MEM0_OPCODES_MB(X) \
    X(V128_LOAD8_LANE,i32_mem0) X(V128_LOAD16_LANE,i32_mem0) X(V128_LOAD32_LANE,i32_mem0) X(V128_LOAD64_LANE,i32_mem0)

// i64-addressed memory opcodes (non-mem0)
#define WAH_I64_MEM_OPCODES_M(X) \
    X(I32_LOAD,i64) X(I64_LOAD,i64) X(F32_LOAD,i64) X(F64_LOAD,i64) \
    X(I32_LOAD8_S,i64) X(I32_LOAD8_U,i64) X(I32_LOAD16_S,i64) X(I32_LOAD16_U,i64) \
    X(I64_LOAD8_S,i64) X(I64_LOAD8_U,i64) X(I64_LOAD16_S,i64) X(I64_LOAD16_U,i64) \
    X(I64_LOAD32_S,i64) X(I64_LOAD32_U,i64) \
    X(I32_STORE,i64) X(I64_STORE,i64) X(F32_STORE,i64) X(F64_STORE,i64) \
    X(I32_STORE8,i64) X(I32_STORE16,i64) X(I64_STORE8,i64) X(I64_STORE16,i64) X(I64_STORE32,i64) \
    X(V128_LOAD,i64) \
    X(V128_LOAD8X8_S,i64) X(V128_LOAD8X8_U,i64) \
    X(V128_LOAD16X4_S,i64) X(V128_LOAD16X4_U,i64) \
    X(V128_LOAD32X2_S,i64) X(V128_LOAD32X2_U,i64) \
    X(V128_LOAD8_SPLAT,i64) X(V128_LOAD16_SPLAT,i64) X(V128_LOAD32_SPLAT,i64) X(V128_LOAD64_SPLAT,i64) \
    X(V128_LOAD32_ZERO,i64) X(V128_LOAD64_ZERO,i64) \
    X(V128_STORE,i64)
#define WAH_I64_MEM_OPCODES_MB(X) \
    X(V128_LOAD8_LANE,i64) X(V128_LOAD16_LANE,i64) X(V128_LOAD32_LANE,i64) X(V128_LOAD64_LANE,i64)

// i64-addressed memory 0 fast-path opcodes
#define WAH_I64_MEM0_OPCODES_M(X) \
    X(I32_LOAD,i64_mem0) X(I64_LOAD,i64_mem0) X(F32_LOAD,i64_mem0) X(F64_LOAD,i64_mem0) \
    X(I32_LOAD8_S,i64_mem0) X(I32_LOAD8_U,i64_mem0) X(I32_LOAD16_S,i64_mem0) X(I32_LOAD16_U,i64_mem0) \
    X(I64_LOAD8_S,i64_mem0) X(I64_LOAD8_U,i64_mem0) X(I64_LOAD16_S,i64_mem0) X(I64_LOAD16_U,i64_mem0) \
    X(I64_LOAD32_S,i64_mem0) X(I64_LOAD32_U,i64_mem0) \
    X(I32_STORE,i64_mem0) X(I64_STORE,i64_mem0) X(F32_STORE,i64_mem0) X(F64_STORE,i64_mem0) \
    X(I32_STORE8,i64_mem0) X(I32_STORE16,i64_mem0) X(I64_STORE8,i64_mem0) X(I64_STORE16,i64_mem0) X(I64_STORE32,i64_mem0) \
    X(V128_LOAD,i64_mem0) \
    X(V128_LOAD8X8_S,i64_mem0) X(V128_LOAD8X8_U,i64_mem0) \
    X(V128_LOAD16X4_S,i64_mem0) X(V128_LOAD16X4_U,i64_mem0) \
    X(V128_LOAD32X2_S,i64_mem0) X(V128_LOAD32X2_U,i64_mem0) \
    X(V128_LOAD8_SPLAT,i64_mem0) X(V128_LOAD16_SPLAT,i64_mem0) X(V128_LOAD32_SPLAT,i64_mem0) X(V128_LOAD64_SPLAT,i64_mem0) \
    X(V128_LOAD32_ZERO,i64_mem0) X(V128_LOAD64_ZERO,i64_mem0) \
    X(V128_STORE,i64_mem0)
#define WAH_I64_MEM0_OPCODES_MB(X) \
    X(V128_LOAD8_LANE,i64_mem0) X(V128_LOAD16_LANE,i64_mem0) X(V128_LOAD32_LANE,i64_mem0) X(V128_LOAD64_LANE,i64_mem0)

// i64-addressed memory.size/grow/fill/init/copy opcodes (non-mem0 and mem0)
#define WAH_I64_MEM_SIZE_OPCODES(X) \
    X(MEMORY_SIZE,i64) X(MEMORY_GROW,i64) \
    X(MEMORY_FILL,i64) X(MEMORY_INIT,i64) X(MEMORY_COPY,i64)

#ifdef WAH_X86_64

// Single-version opcodes (only one optimized implementation)
#define WAH_X86_64_EXTRA_OPCODES_SINGLE(X) \
    X(I8X16_ABS,ssse3) X(I16X8_ABS,ssse3) X(I32X4_ABS,ssse3) \
    X(I16X8_Q15MULR_SAT_S,ssse3) X(I16X8_RELAXED_Q15MULR_S,ssse3) X(I8X16_RELAXED_SWIZZLE,ssse3) \
    X(I8X16_MIN_S,sse41) X(I8X16_MAX_S,sse41) X(I16X8_MIN_U,sse41) X(I16X8_MAX_U,sse41) \
    X(I32X4_MIN_S,sse41) X(I32X4_MIN_U,sse41) X(I32X4_MAX_S,sse41) X(I32X4_MAX_U,sse41) \
    X(I32X4_MUL,sse41) \
    X(I16X8_EXTEND_LOW_I8X16_S,sse41) X(I16X8_EXTEND_HIGH_I8X16_S,sse41) \
    X(I16X8_EXTEND_LOW_I8X16_U,sse41) X(I16X8_EXTEND_HIGH_I8X16_U,sse41) \
    X(I32X4_EXTEND_LOW_I16X8_S,sse41) X(I32X4_EXTEND_HIGH_I16X8_S,sse41) \
    X(I32X4_EXTEND_LOW_I16X8_U,sse41) X(I32X4_EXTEND_HIGH_I16X8_U,sse41) \
    X(I64X2_EXTEND_LOW_I32X4_S,sse41) X(I64X2_EXTEND_HIGH_I32X4_S,sse41) \
    X(I64X2_EXTEND_LOW_I32X4_U,sse41) X(I64X2_EXTEND_HIGH_I32X4_U,sse41) \
    X(I16X8_EXTMUL_LOW_I8X16_S,sse41) X(I16X8_EXTMUL_HIGH_I8X16_S,sse41) \
    X(I16X8_EXTMUL_LOW_I8X16_U,sse41) X(I16X8_EXTMUL_HIGH_I8X16_U,sse41) \
    X(I32X4_EXTMUL_LOW_I16X8_S,sse41) X(I32X4_EXTMUL_HIGH_I16X8_S,sse41) \
    X(I32X4_EXTMUL_LOW_I16X8_U,sse41) X(I32X4_EXTMUL_HIGH_I16X8_U,sse41) \
    X(I64X2_EXTMUL_LOW_I32X4_S,sse41) X(I64X2_EXTMUL_HIGH_I32X4_S,sse41) \
    X(I64X2_EXTMUL_LOW_I32X4_U,sse41) X(I64X2_EXTMUL_HIGH_I32X4_U,sse41) \
    X(I64X2_EQ,sse41) X(I64X2_NE,sse41) \
    X(I32X4_TRUNC_SAT_F32X4_S,sse41) \
    X(F32X4_CEIL,sse41) X(F32X4_FLOOR,sse41) X(F32X4_TRUNC,sse41) X(F32X4_NEAREST,sse41) \
    X(F64X2_CEIL,sse41) X(F64X2_FLOOR,sse41) X(F64X2_TRUNC,sse41) X(F64X2_NEAREST,sse41) \
    X(I64X2_GT_S,sse42) X(I64X2_LT_S,sse42) \
    X(I64X2_LE_S,sse42) X(I64X2_GE_S,sse42) \
    X(I8X16_EQ,avx512f) X(I8X16_GT_S,avx512f) X(I8X16_LT_U,avx512f) \
    X(F32X4_PMIN,avx512f) X(F32X4_PMAX,avx512f) X(F64X2_PMIN,avx512f) X(F64X2_PMAX,avx512f) \
    X(V128_BITSELECT,avx512f) \
    X(F32X4_CONVERT_I32X4_U,avx512vl) X(F64X2_CONVERT_LOW_I32X4_U,avx512vl)

// Multi-version opcodes (multiple implementations with priority ordering)
// These are handled manually in wah_x86_64_opcode function
#define WAH_X86_64_EXTRA_OPCODES_MULTI(X) \
    X(I8X16_POPCNT,avx2) X(I8X16_POPCNT,avx512f) \
    X(I32X4_TRUNC_SAT_F32X4_U,sse41) X(I32X4_TRUNC_SAT_F32X4_U,avx512vl) \
    X(I64X2_MUL,avx512f) X(I64X2_MUL,avx512ifma)

#endif

#define WAH_EXTRA_OPCODES(X) \
    WAH_I32_MEM0_OPCODES_M(X) WAH_I32_MEM0_OPCODES_MB(X) \
    WAH_I64_MEM_OPCODES_M(X) WAH_I64_MEM_OPCODES_MB(X) \
    WAH_I64_MEM0_OPCODES_M(X) WAH_I64_MEM0_OPCODES_MB(X) \
    WAH_I64_MEM_SIZE_OPCODES(X) \
    WAH_IF_X86_64(WAH_X86_64_EXTRA_OPCODES_SINGLE(X) WAH_X86_64_EXTRA_OPCODES_MULTI(X))

typedef enum {
#define WAH_OPCODE_INIT(name, cls, val) WAH_OP_##name = val,
#define WAH_EXTRA_OPCODE_INIT(name, suffix) WAH_OP_##name##_##suffix,
    WAH_OPCODES(WAH_OPCODE_INIT)
    WAH_LAST_OPCODE = WAH_FE - 1, // To make the first extra opcode have value WAH_FE
    WAH_EXTRA_OPCODES(WAH_EXTRA_OPCODE_INIT)
    WAH_NUM_OPCODES,
#undef WAH_OPCODE_INIT
#undef WAH_EXTRA_OPCODE_INIT
} wah_opcode_t;

static const uint8_t wah_opclasses[WAH_FE] = {
#define WAH_OPCLASS_INIT(name, cls, val) [WAH_OP_##name] = WAH_OPCLASS_##cls,
    WAH_OPCODES(WAH_OPCLASS_INIT)
#undef WAH_OPCLASS_INIT
};

#ifdef WAH_X86_64

#ifdef __GNUC__
#include <cpuid.h>
#else
#include <intrin.h>
#endif

// SSE2/SSSE3/SSE4.1 inline assembly wrappers for GCC/Clang (must be declared before use)
#ifdef __GNUC__
#define WAH_ASM_UNARY_M128I(intrin, insn) \
    static WAH_ALWAYS_INLINE __m128i wah##intrin(__m128i a) { __m128i res; __asm__(insn " %1, %0" : "=x"(res) : "x"(a)); return res; }
#define WAH_ASM_UNARY_M128(intrin, insn) \
    static WAH_ALWAYS_INLINE __m128 wah##intrin(__m128 a) { __m128 res; __asm__(insn " %1, %0" : "=x"(res) : "x"(a)); return res; }
#define WAH_ASM_UNARY_M128D(intrin, insn) \
    static WAH_ALWAYS_INLINE __m128d wah##intrin(__m128d a) { __m128d res; __asm__(insn " %1, %0" : "=x"(res) : "x"(a)); return res; }
#define WAH_ASM_BINARY_M128I(intrin, insn) \
    static WAH_ALWAYS_INLINE __m128i wah##intrin(__m128i a, __m128i b) \
        { __m128i res; __asm__(insn " %2, %0" : "=x"(res) : "0"(a), "x"(b)); return res; }
#define WAH_ASM_BINARY_M128I_3OP(intrin, insn) \
    static WAH_ALWAYS_INLINE __m128i wah##intrin(__m128i a, __m128i b) \
        { __m128i res; __asm__(insn " %2, %1, %0" : "=x"(res) : "x"(a), "x"(b)); return res; }
#define WAH_ASM_BINARY_M128(intrin, insn) \
    static WAH_ALWAYS_INLINE __m128 wah##intrin(__m128 a, __m128 b) \
        { __m128 res; __asm__(insn " %2, %0" : "=x"(res) : "0"(a), "x"(b)); return res; }
#define WAH_ASM_BINARY_M128_IMM(intrin, insn, imm) \
    static WAH_ALWAYS_INLINE __m128 wah##intrin(__m128 a, __m128 b) \
        { __m128 res; __asm__(insn " $"#imm ", %2, %1, %0" : "=x"(res) : "x"(a), "x"(b)); return res; }
#define WAH_ASM_BINARY_M128D(intrin, insn) \
    static WAH_ALWAYS_INLINE __m128d wah##intrin(__m128d a, __m128d b) \
        { __m128d res; __asm__(insn " %2, %0" : "=x"(res) : "0"(a), "x"(b)); return res; }
#define WAH_ASM_BINARY_M128D_IMM(intrin, insn, imm) \
    static WAH_ALWAYS_INLINE __m128d wah##intrin(__m128d a, __m128d b) \
        { __m128d res; __asm__(insn " $"#imm ", %2, %1, %0" : "=x"(res) : "x"(a), "x"(b)); return res; }
#define WAH_ASM_TERNARY_M128I(intrin, insn) \
    static WAH_ALWAYS_INLINE __m128i wah##intrin(__m128i a, __m128i b, __m128i c, int imm) \
        { __m128i res; __asm__(insn " %4, %3, %2, %0" : "=x"(res) : "0"(a), "x"(b), "x"(c), "i"(imm)); return res; }
#define WAH_ASM_CONV_M128I_TO_M128(intrin, insn) \
    static WAH_ALWAYS_INLINE __m128 wah##intrin(__m128i a) { __m128 res; __asm__(insn " %1, %0" : "=x"(res) : "x"(a)); return res; }
#define WAH_ASM_CONV_M128I_TO_M128D(intrin, insn) \
    static WAH_ALWAYS_INLINE __m128d wah##intrin(__m128i a) { __m128d res; __asm__(insn " %1, %0" : "=x"(res) : "x"(a)); return res; }
#define WAH_ASM_CONV_M128_TO_M128I(intrin, insn) \
    static WAH_ALWAYS_INLINE __m128i wah##intrin(__m128 a) { __m128i res; __asm__(insn " %1, %0" : "=x"(res) : "x"(a)); return res; }
#else
#define WAH_ASM_UNARY_M128I(intrin, insn) static WAH_ALWAYS_INLINE __m128i wah##intrin(__m128i a) { return intrin(a); }
#define WAH_ASM_UNARY_M128(intrin, insn) static WAH_ALWAYS_INLINE __m128 wah##intrin(__m128 a) { return intrin(a); }
#define WAH_ASM_UNARY_M128D(intrin, insn) static WAH_ALWAYS_INLINE __m128d wah##intrin(__m128d a) { return intrin(a); }
#define WAH_ASM_BINARY_M128I(intrin, insn) static WAH_ALWAYS_INLINE __m128i wah##intrin(__m128i a, __m128i b) { return intrin(a, b); }
#define WAH_ASM_BINARY_M128I_3OP(intrin, insn) static WAH_ALWAYS_INLINE __m128i wah##intrin(__m128i a, __m128i b) { return intrin(a, b); }
#define WAH_ASM_BINARY_M128(intrin, insn) static WAH_ALWAYS_INLINE __m128 wah##intrin(__m128 a, __m128 b) { return intrin(a, b); }
#define WAH_ASM_BINARY_M128_IMM(intrin, insn, imm) static WAH_ALWAYS_INLINE __m128 wah##intrin(__m128 a, __m128 b) { return intrin(a, b, imm); }
#define WAH_ASM_BINARY_M128D(intrin, insn) static WAH_ALWAYS_INLINE __m128d wah##intrin(__m128d a, __m128d b) { return intrin(a, b); }
#define WAH_ASM_BINARY_M128D_IMM(intrin, insn, imm) static WAH_ALWAYS_INLINE __m128d wah##intrin(__m128d a, __m128d b) { return intrin(a, b, imm); }
#define WAH_ASM_TERNARY_M128I(intrin, insn) static WAH_ALWAYS_INLINE __m128i wah##intrin(__m128i a, __m128i b, __m128i c, int imm) { return intrin(a, b, c, imm); }
#define WAH_ASM_CONV_M128I_TO_M128(intrin, insn) static WAH_ALWAYS_INLINE __m128 wah##intrin(__m128i a) { return intrin(a); }
#define WAH_ASM_CONV_M128I_TO_M128D(intrin, insn) static WAH_ALWAYS_INLINE __m128d wah##intrin(__m128i a) { return intrin(a); }
#define WAH_ASM_CONV_M128_TO_M128I(intrin, insn) static WAH_ALWAYS_INLINE __m128i wah##intrin(__m128 a) { return intrin(a); }
#endif

// SSSE3 wrappers
WAH_IF_SSSE3(
    WAH_ASM_UNARY_M128I(_mm_abs_epi8, "pabsb")
    WAH_ASM_UNARY_M128I(_mm_abs_epi16, "pabsw")
    WAH_ASM_UNARY_M128I(_mm_abs_epi32, "pabsd")
    WAH_ASM_BINARY_M128I(_mm_mulhrs_epi16, "pmulhrsw")
    WAH_ASM_BINARY_M128I(_mm_shuffle_epi8, "pshufb")
    WAH_ASM_BINARY_M128I(_mm_maddubs_epi16, "pmaddubsw")
)

#define _mm_round_ps_trunc(a) _mm_round_ps((a), _MM_FROUND_TO_ZERO)
#define _mm_round_ps_nearest(a) _mm_round_ps((a), _MM_FROUND_TO_NEAREST_INT)
#define _mm_round_pd_trunc(a) _mm_round_pd((a), _MM_FROUND_TO_ZERO)
#define _mm_round_pd_nearest(a) _mm_round_pd((a), _MM_FROUND_TO_NEAREST_INT)

WAH_IF_SSE41(
    // SSE4.1 wrappers - min/max
    WAH_ASM_BINARY_M128I(_mm_min_epi8, "pminsb")
    WAH_ASM_BINARY_M128I(_mm_max_epi8, "pmaxsb")
    WAH_ASM_BINARY_M128I(_mm_min_epu16, "pminuw")
    WAH_ASM_BINARY_M128I(_mm_max_epu16, "pmaxuw")
    WAH_ASM_BINARY_M128I(_mm_min_epi32, "pminsd")
    WAH_ASM_BINARY_M128I(_mm_max_epi32, "pmaxsd")
    WAH_ASM_BINARY_M128I(_mm_min_epu32, "pminud")
    WAH_ASM_BINARY_M128I(_mm_max_epu32, "pmaxud")

    // SSE4.1 wrappers - extend
    WAH_ASM_UNARY_M128I(_mm_cvtepi8_epi16, "pmovsxbw")
    WAH_ASM_UNARY_M128I(_mm_cvtepu8_epi16, "pmovzxbw")
    WAH_ASM_UNARY_M128I(_mm_cvtepi16_epi32, "pmovsxwd")
    WAH_ASM_UNARY_M128I(_mm_cvtepu16_epi32, "pmovzxwd")
    WAH_ASM_UNARY_M128I(_mm_cvtepi32_epi64, "pmovsxdq")
    WAH_ASM_UNARY_M128I(_mm_cvtepu32_epi64, "pmovzxdq")

    // SSE4.1 wrappers - compare/pack/mul
    WAH_ASM_BINARY_M128I(_mm_cmpeq_epi64, "pcmpeqq")
    WAH_ASM_BINARY_M128I(_mm_cmpgt_epi64, "pcmpgtq")
    WAH_ASM_BINARY_M128I(_mm_packus_epi32, "packusdw")
    WAH_ASM_BINARY_M128I(_mm_mullo_epi32, "pmulld")
    WAH_ASM_BINARY_M128I(_mm_mul_epi32, "pmuldq")

    // SSE4.1 wrappers - float round with immediate operand
    WAH_ASM_UNARY_M128(_mm_ceil_ps, "roundps $0x2,")
    WAH_ASM_UNARY_M128(_mm_floor_ps, "roundps $0x1,")
    WAH_ASM_UNARY_M128(_mm_round_ps_trunc, "roundps $0x3,")
    WAH_ASM_UNARY_M128(_mm_round_ps_nearest, "roundps $0x0,")

    WAH_ASM_UNARY_M128D(_mm_ceil_pd, "roundpd $0x2,")
    WAH_ASM_UNARY_M128D(_mm_floor_pd, "roundpd $0x1,")
    WAH_ASM_UNARY_M128D(_mm_round_pd_trunc, "roundpd $0x3,")
    WAH_ASM_UNARY_M128D(_mm_round_pd_nearest, "roundpd $0x0,")
)

WAH_IF_AVX512F(
    // AVX-512 popcount wrappers
    WAH_ASM_UNARY_M128I(_mm_popcnt_epi8, "vpopcntb")
    WAH_ASM_UNARY_M128I(_mm_popcnt_epi32, "vpopcntd")
)

// AVX-512 ternary logic wrapper (vpternlogd)
// Specialized for V128_BITSELECT which uses imm8 = 0xD8
#ifdef __GNUC__
WAH_IF_AVX512F(
    static WAH_ALWAYS_INLINE __m128i wah_mm_ternarylogic_epi32_bitselect(__m128i a, __m128i b, __m128i c) {
        __m128i res = a;
        __asm__("vpternlogd $0xD8, %2, %1, %0" : "+x"(res) : "x"(b), "x"(c));
        return res;
    }
)
#else
WAH_IF_AVX512F(
    static WAH_ALWAYS_INLINE __m128i wah_mm_ternarylogic_epi32_bitselect(__m128i a, __m128i b, __m128i c) {
        return _mm_ternarylogic_epi32(a, b, c, 0xD8);
    }
)
#endif

// AVX-512 FP min/max wrappers (vrange)
// Using SAE=1 with mode 3 (<= min) and mode 7 (>= max)
WAH_IF_AVX512F(
    WAH_ASM_BINARY_M128_IMM(_mm_range_ps_min, "vrangeps", 0x83)
    WAH_ASM_BINARY_M128_IMM(_mm_range_ps_max, "vrangeps", 0x87)
    WAH_ASM_BINARY_M128D_IMM(_mm_range_pd_min, "vrangepd", 0x83)
    WAH_ASM_BINARY_M128D_IMM(_mm_range_pd_max, "vrangepd", 0x87)
)

WAH_IF_AVX512VL(
    // AVX-512 unsigned conversion wrappers (i32x4 -> f32x4/f64x2)
    WAH_ASM_CONV_M128I_TO_M128(_mm_cvtepu32_ps, "vcvtudq2ps")
    WAH_ASM_CONV_M128I_TO_M128D(_mm_cvtepu32_pd, "vcvtudq2pd")

    // AVX-512VL truncate-to-unsigned wrappers (f32x4 -> i32x4, with saturation)
    WAH_ASM_CONV_M128_TO_M128I(_mm_cvttps_epu32, "vcvttps2udq")
)

// AVX-512IFMA 64x64->64 multiply
WAH_IF_AVX512IFMA(
    WAH_ASM_BINARY_M128I_3OP(_mm_mullox_epi64, "vpmullq")
)

// SSE4.1 wrapper with implicit xmm0 operand (pblendvb)
#ifdef __GNUC__
WAH_IF_SSE41(
    static WAH_ALWAYS_INLINE __m128i wah_mm_blendv_epi8(__m128i a, __m128i b, __m128i mask) {
        __m128i res = a;
        __asm__ __volatile__("movdqa %2, %%xmm0; pblendvb %1, %0"
            : "+x"(res)
            : "x"(b), "x"(mask)
            : "xmm0");
        return res;
    }
)
#else
WAH_IF_SSE41(
    static WAH_ALWAYS_INLINE __m128i wah_mm_blendv_epi8(__m128i a, __m128i b, __m128i mask) {
        return _mm_blendv_epi8(a, b, mask);
    }
)
#endif

#undef WAH_ASM_UNARY_M128I
#undef WAH_ASM_BINARY_M128I
#undef WAH_ASM_BINARY_M128I_3OP
#undef WAH_ASM_UNARY_M128
#undef WAH_ASM_BINARY_M128
#undef WAH_ASM_BINARY_M128_IMM
#undef WAH_ASM_UNARY_M128D
#undef WAH_ASM_BINARY_M128D
#undef WAH_ASM_BINARY_M128D_IMM
#undef WAH_ASM_TERNARY_M128I
#undef WAH_ASM_CONV_M128I_TO_M128
#undef WAH_ASM_CONV_M128I_TO_M128D
#undef WAH_ASM_CONV_M128_TO_M128I

typedef struct {
    uint32_t ssse3 : 1, sse41 : 1, sse42 : 1, avx2 : 1;
    uint32_t avx512f : 1, avx512vl : 1, avx512bw : 1, avx512ifma : 1;
} wah_x86_64_features_t;

static wah_x86_64_features_t wah_x86_64_features(void) {
    uint32_t eax, ebx, ecx, edx;
#if defined(_MSC_VER)
    int cpu_info[4];
    __cpuid(cpu_info, 1);
    eax = (uint32_t)cpu_info[0];
    ebx = (uint32_t)cpu_info[1];
    ecx = (uint32_t)cpu_info[2];
    edx = (uint32_t)cpu_info[3];
#else
    __cpuid(1, eax, ebx, ecx, edx);
#endif
    uint32_t avx512f = (ebx >> 16) & 1;
    return (wah_x86_64_features_t){
        .ssse3 = (ecx >> 9) & 1,
        .sse41 = (ecx >> 19) & 1,
        .sse42 = (ecx >> 20) & 1,
        .avx2 = (ebx >> 5) & 1,
        .avx512f = avx512f,
        .avx512vl = avx512f & (ebx >> 17),
        .avx512bw = avx512f & (ebx >> 30),
        .avx512ifma = avx512f & (ebx >> 21),
    };
}

static wah_opcode_t wah_x86_64_opcode(wah_opcode_t opcode, wah_x86_64_features_t features) {
    switch (opcode) {
        // Single-version opcodes
        #define WAH_X86_64_OPCODE_CASE(name, suffix) case WAH_OP_##name: if (features.suffix) return WAH_OP_##name##_##suffix;
        WAH_X86_64_EXTRA_OPCODES_SINGLE(WAH_X86_64_OPCODE_CASE)
        #undef WAH_X86_64_OPCODE_CASE

        // Multi-version opcodes with priority ordering
        case WAH_OP_I8X16_POPCNT:
            if (features.avx512bw) return WAH_OP_I8X16_POPCNT_avx512f;
            if (features.avx2) return WAH_OP_I8X16_POPCNT_avx2;
            break;
        case WAH_OP_I32X4_TRUNC_SAT_F32X4_U:
            if (features.avx512vl) return WAH_OP_I32X4_TRUNC_SAT_F32X4_U_avx512vl;
            if (features.sse41) return WAH_OP_I32X4_TRUNC_SAT_F32X4_U_sse41;
            break;
        case WAH_OP_I64X2_MUL:
            if (features.avx512ifma) return WAH_OP_I64X2_MUL_avx512ifma;
            if (features.avx512f) return WAH_OP_I64X2_MUL_avx512f;
            break;
        default: ; // pass through
    }
    return opcode;
}

#endif

// --- Memory Structure ---
#define WAH_WASM_PAGE_SIZE 65536 // 64 KB

typedef struct wah_memory_type_s {
    wah_type_t addr_type;     // WAH_TYPE_I32 or WAH_TYPE_I64
    uint64_t min_pages, max_pages; // max_pages: UINT64_MAX if no maximum
} wah_memory_type_t;

// --- WebAssembly Table Structures ---
typedef struct wah_table_type_s {
    wah_type_t elem_type;
    uint32_t min_elements;
    uint32_t max_elements; // UINT32_MAX if no maximum
} wah_table_type_t;

typedef struct wah_data_segment_s {
    uint32_t flags;
    uint32_t memory_idx; // Only for active segments (flags & 0x02)
    uint64_t offset;     // Result of the offset_expr (i32.const X end or i64.const X end)
    uint32_t data_len;
    const uint8_t *data; // Pointer to the raw data bytes within the WASM binary
} wah_data_segment_t;

// Unified function entry used in wah_module_t::functions[].
// For WASM functions only kind is meaningful; the rest is zero.
// For host functions all fields are valid.
typedef struct wah_function_s {
    bool is_host;

    // Global function index (for reference types)
    uint32_t global_idx;  // Index in the global function index space [imports, local, host]

    // Host function fields (only valid when is_host == true)
    char *name;           // owned
    wah_func_t func;
    void *userdata;
    wah_finalize_t finalize;
    size_t nparams, nresults;
    wah_type_t *param_types, *result_types; // owned

    // WASM function fields (only valid when is_host == false)
    uint32_t local_idx;                    // local index within fn_module->code_bodies[]
    const struct wah_module_s *fn_module;  // owning module (NULL means ctx->module)
} wah_function_t;

typedef struct wah_export_s {
    const char *name;
    size_t name_len;
    uint8_t kind; // WASM export kind (0=func, 1=table, 2=mem, 3=global)
    uint32_t index; // Index into the respective module array (functions, tables, etc.)
} wah_export_t;

// --- Operand Stack ---
typedef struct {
    wah_value_t *data; // Dynamically allocated based on function requirements
    uint32_t sp; // Stack pointer
    uint32_t capacity; // Allocated capacity
} wah_stack_t;

// --- Type Stack for Validation ---
#define WAH_MAX_TYPE_STACK_SIZE 1024 // Maximum size of the type stack for validation
typedef struct {
    wah_type_t data[WAH_MAX_TYPE_STACK_SIZE];
    uint32_t sp; // Stack pointer
} wah_type_stack_t;

// --- Execution Context ---

// Represents a single function call's state on the call stack.
typedef struct wah_call_frame_s {
    const uint8_t *bytecode_ip;  // Instruction pointer into the parsed bytecode
    const struct wah_code_body_s *code; // The function body being executed
    uint32_t locals_offset;      // Offset into the shared value_stack for this frame's locals
    uint32_t func_idx;           // Local index of the function being executed (debug)
    uint32_t result_count;       // Number of return values (used by RETURN/END)
    const struct wah_module_s *module; // The module this function belongs to (for cross-module calls)
    uint32_t globals_offset;     // Offset into ctx->globals for this module's globals (for cross-module calls)
} wah_call_frame_t;

// The main context for the entire WebAssembly interpretation.
#define WAH_DEFAULT_MAX_CALL_DEPTH 1024
#define WAH_DEFAULT_VALUE_STACK_SIZE (64 * 1024)

// --- Function Type ---
typedef struct wah_func_type_s {
    uint32_t param_count;
    uint32_t result_count;
    wah_type_t *param_types;
    wah_type_t *result_types;
} wah_func_type_t;

// --- Pre-parsed Opcode Structure for Optimized Execution ---
typedef struct {
    uint8_t *bytecode;           // Combined array of opcodes and arguments
    uint32_t bytecode_size;      // Total size of the bytecode array
} wah_parsed_code_t;

// --- Code Body Structure ---
typedef struct wah_code_body_s {
    uint32_t local_count;
    wah_type_t *local_types; // Array of types for local variables
    uint32_t code_size;
    const uint8_t *code; // Pointer to the raw instruction bytes within the WASM binary
    uint32_t max_stack_depth; // Maximum operand stack depth required
    wah_parsed_code_t parsed_code; // Pre-parsed opcodes and arguments for optimized execution
} wah_code_body_t;

// --- WebAssembly Element Segment Structure ---
typedef struct wah_element_segment_s {
    bool is_active;           // true for active (flags&3 == 0 or 2), false for passive
    bool is_dropped;          // true if declarative (dropped after validation) or dropped via elem.drop
    uint32_t table_idx;       // For active modes
    uint32_t num_elems;
    bool is_expr_elem;        // true if elem* are expressions, false if func indices

    // offset expression (for active modes) - preparsed for evaluation
    wah_parsed_code_t offset_expr;

    // elem* data
    union {
        uint32_t *func_indices;              // For is_expr_elem == false
        struct {
            uint8_t **bytecodes;                // Array of pointers to each expression
            uint32_t *bytecode_sizes;           // Array of sizes for each expression
        } expr;
    } u;
} wah_element_segment_t;

// --- Global Variable Structure ---
typedef struct wah_global_s {
    wah_type_t type;
    bool is_mutable;
    wah_parsed_code_t init_expr; // Preparsed const expression (evaluated at instantiation)
} wah_global_t;

// --- Validation Context ---
#define WAH_MAX_CONTROL_DEPTH 256
typedef struct {
    wah_opcode_t opcode;
    uint32_t type_stack_sp; // Type stack pointer at the start of the block
    wah_func_type_t block_type; // For if/block/loop
    bool else_found; // For if blocks
    bool is_unreachable; // True if this control frame is currently unreachable
    uint32_t stack_height; // Stack height at the beginning of the block
} wah_validation_control_frame_t;

typedef struct {
    wah_type_stack_t type_stack;
    const wah_func_type_t *func_type; // Type of the function being validated
    wah_module_t *module; // Reference to the module for global/function lookups
    uint32_t total_locals; // Total number of locals (params + declared locals)
    uint32_t current_stack_depth; // Current stack depth during validation
    uint32_t max_stack_depth; // Maximum stack depth seen during validation
    bool is_unreachable; // True if the current code path is unreachable

    // Control flow validation stack
    wah_validation_control_frame_t control_stack[WAH_MAX_CONTROL_DEPTH];
    uint32_t control_sp;
} wah_validation_context_t;

// --- Forward Declarations ---

static wah_error_t wah_call_module(wah_exec_context_t *exec_ctx, uint32_t func_idx, const wah_value_t *params, uint32_t param_count, wah_value_t *result);

// Sentinel used by wah_value_t.prefuncref to distinguish ref.func 0 from ref.null.
// A prefuncref with .sentinel == wah_funcref_sentinel is a valid function reference;
// .ref == NULL means null. This sentinel is never executed.
static wah_function_t wah_funcref_sentinel[1];

// --- Helper Macros ---
#define WAH_CHECK(expr) do { \
    wah_error_t _err = (expr); \
    if (_err != WAH_OK) { WAH_LOG("WAH_CHECK(%s) failed due to: %s", #expr, wah_strerror(_err)); return _err; } \
} while(0)

#define WAH_CHECK_GOTO(expr, label) do { \
    err = (expr); \
    if (err != WAH_OK) { WAH_LOG("WAH_CHECK_GOTO(%s, %s) failed due to: %s", #expr, #label, wah_strerror(err)); goto label; } \
} while(0)

#define WAH_ENSURE(cond, error) do { \
    if (!(cond)) { WAH_LOG("WAH_ENSURE(%s, %s) failed", #cond, #error); return (error); } \
} while(0)

#define WAH_ENSURE_GOTO(cond, error, label) do { \
    if (!(cond)) { err = (error); WAH_LOG("WAH_ENSURE_GOTO(%s, %s, %s) failed", #cond, #error, #label); goto label; } \
} while(0)

// Helper macro to check for __builtin_xxx functions with __has_builtin
#if defined(__has_builtin)
#define WAH_HAS_BUILTIN(x) __has_builtin(x)
#else
#define WAH_HAS_BUILTIN(x) 0
#endif

// --- Safe Memory Allocation ---
static inline wah_error_t wah_malloc(size_t count, size_t elemsize, void** out_ptr) {
    *out_ptr = NULL;
    if (count == 0) {
        return WAH_OK;
    }
    WAH_ENSURE(elemsize == 0 || count <= SIZE_MAX / elemsize, WAH_ERROR_OUT_OF_MEMORY);
    size_t total_size = count * elemsize;
    *out_ptr = malloc(total_size);
    WAH_ENSURE(*out_ptr, WAH_ERROR_OUT_OF_MEMORY);
    return WAH_OK;
}

static inline wah_error_t wah_realloc(size_t count, size_t elemsize, void** p_ptr) {
    if (count == 0) {
        free(*p_ptr);
        *p_ptr = NULL;
        return WAH_OK;
    }
    WAH_ENSURE(elemsize == 0 || count <= SIZE_MAX / elemsize, WAH_ERROR_OUT_OF_MEMORY);
    size_t total_size = count * elemsize;
    void* new_ptr = realloc(*p_ptr, total_size);
    WAH_ENSURE(new_ptr, WAH_ERROR_OUT_OF_MEMORY);
    *p_ptr = new_ptr;
    return WAH_OK;
}

#define WAH_MALLOC_ARRAY(ptr, count) \
    do { \
        void *_alloc_ptr; \
        wah_error_t _alloc_err = wah_malloc((count), sizeof(*(ptr)), &_alloc_ptr); \
        if (_alloc_err != WAH_OK) { \
            WAH_LOG("WAH_MALLOC_ARRAY(%s, %s) failed due to OOM", #ptr, #count); \
            return _alloc_err; \
        } \
        (ptr) = _alloc_ptr; \
    } while (0)

#define WAH_REALLOC_ARRAY(ptr, count) \
    do { \
        void *_alloc_ptr = (ptr); \
        wah_error_t _alloc_err = wah_realloc((count), sizeof(*(ptr)), &_alloc_ptr); \
        if (_alloc_err != WAH_OK) { \
            WAH_LOG("WAH_REALLOC_ARRAY(%s, %s) failed due to OOM", #ptr, #count); \
            return _alloc_err; \
        } \
        (ptr) = _alloc_ptr; \
    } while (0)

#define WAH_MALLOC_ARRAY_GOTO(ptr, count, label) \
    do { \
        void* _alloc_ptr; \
        err = wah_malloc((count), sizeof(*(ptr)), &_alloc_ptr); \
        if (err != WAH_OK) { \
            WAH_LOG("WAH_MALLOC_ARRAY_GOTO(%s, %s, %s) failed due to OOM", #ptr, #count, #label); \
            goto label; \
        } \
        (ptr) = _alloc_ptr; \
    } while (0)

#define WAH_REALLOC_ARRAY_GOTO(ptr, count, label) \
    do { \
        void* _alloc_ptr = ptr; \
        err = wah_realloc((count), sizeof(*(ptr)), &_alloc_ptr); \
        if (err != WAH_OK) { \
            WAH_LOG("WAH_REALLOC_ARRAY_GOTO(%s, %s, %s) failed due to OOM", #ptr, #count, #label); \
            goto label; \
        } \
        (ptr) = _alloc_ptr; \
    } while (0)

const char *wah_strerror(wah_error_t err) {
    switch (err) {
        case WAH_OK: return "Success";
        case WAH_ERROR_INVALID_MAGIC_NUMBER: return "Invalid WASM magic number";
        case WAH_ERROR_INVALID_VERSION: return "Invalid WASM version";
        case WAH_ERROR_UNEXPECTED_EOF: return "Unexpected end of file";
        case WAH_ERROR_UNKNOWN_SECTION: return "Unknown section or opcode";
        case WAH_ERROR_TOO_LARGE: return "Exceeding implementation limits (or value too large)";
        case WAH_ERROR_OUT_OF_MEMORY: return "Out of memory";
        case WAH_ERROR_VALIDATION_FAILED: return "Validation failed";
        case WAH_ERROR_TRAP: return "Runtime trap";
        case WAH_ERROR_CALL_STACK_OVERFLOW: return "Call stack overflow";
        case WAH_ERROR_MEMORY_OUT_OF_BOUNDS: return "Memory access out of bounds";
        case WAH_ERROR_NOT_FOUND: return "Item not found";
        case WAH_ERROR_MISUSE: return "API misused: invalid arguments";
        case WAH_ERROR_BAD_SPEC: return "Invalid DSL spec";
        case WAH_OK_BUT_MULTI_RETURN: return "Function succeeded but returned multiple values (only first value available)";
        default: return "Unknown error";
    }
}

// --- LEB128 Decoding ---
// Helper function to decode an unsigned LEB128 integer
static inline wah_error_t wah_decode_uleb128(const uint8_t **ptr, const uint8_t *end, uint32_t *result) {
    uint64_t val = 0;
    uint32_t shift = 0;
    uint8_t byte;

    for (int i = 0; i < 5; ++i) { // Max 5 bytes for a 32-bit value
        WAH_ENSURE(*ptr < end, WAH_ERROR_UNEXPECTED_EOF);
        byte = *(*ptr)++;
        val |= (uint64_t)(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) {
            WAH_ENSURE(val <= UINT32_MAX, WAH_ERROR_TOO_LARGE);
            *result = (uint32_t)val;
            return WAH_OK;
        }
        shift += 7;
    }
    // If we get here, it means the 5th byte had the continuation bit set.
    return WAH_ERROR_TOO_LARGE;
}

// Helper function to decode an unsigned LEB128 integer (64-bit)
static inline wah_error_t wah_decode_uleb128_u64(const uint8_t **ptr, const uint8_t *end, uint64_t *result) {
    uint64_t val = 0;
    uint32_t shift = 0;
    uint8_t byte;
    for (int i = 0; i < 10; ++i) { // Max 10 bytes for a 64-bit value
        WAH_ENSURE(*ptr < end, WAH_ERROR_UNEXPECTED_EOF);
        byte = *(*ptr)++;
        val |= (uint64_t)(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) {
            *result = val;
            return WAH_OK;
        }
        shift += 7;
    }
    return WAH_ERROR_TOO_LARGE;
}

// Helper function to decode a signed LEB128 integer (32-bit)
static inline wah_error_t wah_decode_sleb128_32(const uint8_t **ptr, const uint8_t *end, int32_t *result) {
    uint64_t val = 0;
    uint32_t shift = 0;
    uint8_t byte;

    for (int i = 0; i < 5; ++i) { // Max 5 bytes for a 32-bit value
        WAH_ENSURE(*ptr < end, WAH_ERROR_UNEXPECTED_EOF);
        byte = *(*ptr)++;
        val |= (uint64_t)(byte & 0x7F) << shift;
        shift += 7;
        if ((byte & 0x80) == 0) {
            // Sign extend
            if (shift < 64) {
                uint64_t sign_bit = 1ULL << (shift - 1);
                if ((val & sign_bit) != 0) {
                    val |= ~0ULL << shift;
                }
            }
            WAH_ENSURE((int64_t)val >= INT32_MIN && (int64_t)val <= INT32_MAX, WAH_ERROR_TOO_LARGE);
            *result = (int32_t)val;
            return WAH_OK;
        }
    }
    // If we get here, it means the 5th byte had the continuation bit set.
    return WAH_ERROR_TOO_LARGE;
}

// Helper function to decode an unsigned LEB128 integer (64-bit)
static inline wah_error_t wah_decode_uleb128_64(const uint8_t **ptr, const uint8_t *end, uint64_t *result) {
    *result = 0;
    uint64_t shift = 0;
    uint8_t byte;

    do {
        WAH_ENSURE(*ptr < end, WAH_ERROR_UNEXPECTED_EOF);
        WAH_ENSURE(shift < 64, WAH_ERROR_TOO_LARGE);
        byte = *(*ptr)++;
        *result |= (uint64_t)(byte & 0x7F) << shift;
        shift += 7;
    } while (byte & 0x80);

    return WAH_OK;
}

// Helper function to decode WebAssembly 3.0 memarg
// WebAssembly 3.0 memarg format: align:u32 [memidx:u32] offset:u64
// If align is 0..63, memidx is assumed to be 0
// If align is 64..127, memidx is present and align is adjusted by subtracting 64
static inline wah_error_t wah_decode_memarg(const uint8_t **ptr, const uint8_t *end, uint32_t *align, uint32_t *memidx, uint64_t *offset) {
    WAH_CHECK(wah_decode_uleb128(ptr, end, align));
    *memidx = 0;
    // WebAssembly 3.0: if align >= 64, memidx is present
    if (*align >= 64) {
        WAH_CHECK(wah_decode_uleb128(ptr, end, memidx));
        *align -= 64; // adjust align to actual value
    }
    WAH_CHECK(wah_decode_uleb128_64(ptr, end, offset));
    return WAH_OK;
}

// Helper function to decode a signed LEB128 integer (64-bit)
static inline wah_error_t wah_decode_sleb128_64(const uint8_t **ptr, const uint8_t *end, int64_t *result) {
    uint64_t val = 0;
    uint32_t shift = 0;
    uint8_t byte;

    do {
        WAH_ENSURE(*ptr < end, WAH_ERROR_UNEXPECTED_EOF);
        WAH_ENSURE(shift < 64, WAH_ERROR_TOO_LARGE);
        byte = *(*ptr)++;
        val |= (uint64_t)(byte & 0x7F) << shift;
        shift += 7;
    } while (byte & 0x80);

    if (shift < 64) {
        uint64_t sign_bit = 1ULL << (shift - 1);
        if ((val & sign_bit) != 0) {
            val |= ~0ULL << shift;
        }
    }
    *result = (int64_t)val;
    return WAH_OK;
}

// Helper function to decode a ULEB128 count and validate it against remaining section size
static inline wah_error_t wah_decode_and_validate_count(const uint8_t **ptr, const uint8_t *section_end, uint32_t *count, uint32_t min_bytes_per_item) {
    WAH_CHECK(wah_decode_uleb128(ptr, section_end, count));

    // Check for potential integer overflow before multiplication
    if (min_bytes_per_item > 0 && *count > (UINT32_MAX / min_bytes_per_item)) {
        return WAH_ERROR_TOO_LARGE; // Count * min_bytes_per_item would overflow
    }

    // Ensure that the declared count of items does not exceed the remaining section size.
    // This prevents excessively large allocations and potential hangs/OOM.
    // We use min_bytes_per_item to make a more accurate check.
    if ((uint64_t)*count * min_bytes_per_item > (uint64_t)(section_end - *ptr)) {
        return WAH_ERROR_VALIDATION_FAILED;
    }
    return WAH_OK;
}

// Helper function to validate if a byte sequence is valid UTF-8
static inline bool wah_is_valid_utf8(const char *s, size_t len) {
    const unsigned char *bytes = (const unsigned char *)s;
    size_t i = 0;
    while (i < len) {
        unsigned char byte = bytes[i];
        if (byte < 0x80) { // 1-byte sequence (0xxxxxxx)
            i++;
            continue;
        }
        if ((byte & 0xE0) == 0xC0) { // 2-byte sequence (110xxxxx 10xxxxxx)
            if (i + 1 >= len || (bytes[i+1] & 0xC0) != 0x80 || (byte & 0xFE) == 0xC0) return false; // Overlong encoding
            i += 2;
            continue;
        }
        if ((byte & 0xF0) == 0xE0) { // 3-byte sequence (1110xxxx 10xxxxxx 10xxxxxx)
            if (i + 2 >= len || (bytes[i+1] & 0xC0) != 0x80 || (bytes[i+2] & 0xC0) != 0x80 ||
                (byte == 0xE0 && bytes[i+1] < 0xA0) || // Overlong encoding
                (byte == 0xED && bytes[i+1] >= 0xA0)) return false; // Surrogate pair
            i += 3;
            continue;
        }
        if ((byte & 0xF8) == 0xF0) { // 4-byte sequence (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)
            if (i + 3 >= len || (bytes[i+1] & 0xC0) != 0x80 || (bytes[i+2] & 0xC0) != 0x80 || (bytes[i+3] & 0xC0) != 0x80 ||
                (byte == 0xF0 && bytes[i+1] < 0x90) || // Overlong encoding
                (byte == 0xF4 && bytes[i+1] >= 0x90)) return false; // Codepoint > 0x10FFFF
            i += 4;
            continue;
        }
        return false; // Invalid start byte
    }
    return true;
}

// Helper function to duplicate a string
static inline char* wah_strdup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* copy = (char*)malloc(len + 1);
    if (copy) {
        memcpy(copy, s, len + 1);
    }
    return copy;
}

// Helper to calculate effective address with overflow and bounds checking
static inline wah_error_t wah_check_effective_addr(uint64_t addr, uint64_t offset, uint64_t size, uint64_t memory_size, uint64_t *effective_addr) {
#if WAH_HAS_BUILTIN(__builtin_add_overflow) || __GNUC__ >= 5
    uint64_t end;
    if (__builtin_add_overflow(addr, offset, effective_addr) ||
        __builtin_add_overflow(*effective_addr, size, &end) || end > memory_size) {
        return WAH_ERROR_MEMORY_OUT_OF_BOUNDS;
    }
#else
    // Check for overflow in addr + offset
    if (offset > UINT64_MAX - addr) return WAH_ERROR_MEMORY_OUT_OF_BOUNDS;
    *effective_addr = addr + offset;

    // Check for overflow in effective_addr + size
    if (size > UINT64_MAX - *effective_addr) return WAH_ERROR_MEMORY_OUT_OF_BOUNDS;

    // Check that effective_addr + size <= memory_size
    if (*effective_addr + size > memory_size) return WAH_ERROR_MEMORY_OUT_OF_BOUNDS;
#endif

    return WAH_OK;
}

// Helper to read a uint8_t from a byte array in little-endian format
static inline uint8_t wah_read_u8_le(const uint8_t *ptr) {
    return ptr[0];
}

// Helper to write a uint8_t to a byte array in little-endian format
static inline void wah_write_u8_le(uint8_t *ptr, uint8_t val) {
    ptr[0] = val;
}

// Helper to read a uint16_t from a byte array in little-endian format
static inline uint16_t wah_read_u16_le(const uint8_t *ptr) {
    return ((uint16_t)ptr[0] << 0) |
           ((uint16_t)ptr[1] << 8);
}

// Helper to write a uint16_t to a byte array in little-endian format
static inline void wah_write_u16_le(uint8_t *ptr, uint16_t val) {
    ptr[0] = (uint8_t)(val >> 0);
    ptr[1] = (uint8_t)(val >> 8);
}

// Helper to read a uint32_t from a byte array in little-endian format
static inline uint32_t wah_read_u32_le(const uint8_t *ptr) {
    return ((uint32_t)ptr[0] << 0) |
           ((uint32_t)ptr[1] << 8) |
           ((uint32_t)ptr[2] << 16) |
           ((uint32_t)ptr[3] << 24);
}

// Helper to read a uint64_t from a byte array in little-endian format
static inline uint64_t wah_read_u64_le(const uint8_t *ptr) {
    return ((uint64_t)ptr[0] << 0) |
           ((uint64_t)ptr[1] << 8) |
           ((uint64_t)ptr[2] << 16) |
           ((uint64_t)ptr[3] << 24) |
           ((uint64_t)ptr[4] << 32) |
           ((uint64_t)ptr[5] << 40) |
           ((uint64_t)ptr[6] << 48) |
           ((uint64_t)ptr[7] << 56);
}

// Helper to write a uint32_t to a byte array in little-endian format
static inline void wah_write_u32_le(uint8_t *ptr, uint32_t val) {
    ptr[0] = (uint8_t)(val >> 0);
    ptr[1] = (uint8_t)(val >> 8);
    ptr[2] = (uint8_t)(val >> 16);
    ptr[3] = (uint8_t)(val >> 24);
}

// Helper to write a uint64_t to a byte array in little-endian format
static inline void wah_write_u64_le(uint8_t *ptr, uint64_t val) {
    ptr[0] = (uint8_t)(val >> 0);
    ptr[1] = (uint8_t)(val >> 8);
    ptr[2] = (uint8_t)(val >> 16);
    ptr[3] = (uint8_t)(val >> 24);
    ptr[4] = (uint8_t)(val >> 32);
    ptr[5] = (uint8_t)(val >> 40);
    ptr[6] = (uint8_t)(val >> 48);
    ptr[7] = (uint8_t)(val >> 56);
}

// Helper to read a float from a byte array in little-endian format
static inline float wah_read_f32_le(const uint8_t *ptr) {
    union { uint32_t i; float f; } u = { .i = wah_read_u32_le(ptr) };
    return u.f;
}

// Helper to read a double from a byte array in little-endian format
static inline double wah_read_f64_le(const uint8_t *ptr) {
    union { uint64_t i; double d; } u = { .i = wah_read_u64_le(ptr) };
    return u.d;
}

// Helper to write a float to a byte array in little-endian format
static inline void wah_write_f32_le(uint8_t *ptr, float val) {
    union { uint32_t i; float f; } u = { .f = val };
    wah_write_u32_le(ptr, u.i);
}

// Helper to write a double to a byte array in little-endian format
static inline void wah_write_f64_le(uint8_t *ptr, double val) {
    union { uint64_t i; double f; } u = { .f = val };
    wah_write_u64_le(ptr, u.i);
}

// --- Parser Functions ---
static wah_error_t wah_decode_memarg(const uint8_t **ptr, const uint8_t *end, uint32_t *align, uint32_t *memidx, uint64_t *offset);
static wah_error_t wah_decode_opcode(const uint8_t **ptr, const uint8_t *end, uint16_t *opcode_val);
static wah_error_t wah_decode_val_type(const uint8_t **ptr, const uint8_t *end, wah_type_t *out_type);
static wah_error_t wah_decode_ref_type(const uint8_t **ptr, const uint8_t *end, wah_type_t *out_type);

// The core interpreter loop (internal).
static wah_error_t wah_run_interpreter(wah_exec_context_t *exec_ctx);

// Internal helper: Call a host function with given context
static wah_error_t wah_call_host_function_internal(
    wah_exec_context_t *exec_ctx,
    const wah_function_t *fn,
    const wah_value_t *params,
    uint32_t param_count,
    wah_value_t *results
);

// Validation helper functions
static wah_error_t wah_validate_opcode(uint16_t opcode_val, const uint8_t **code_ptr, const uint8_t *code_end, wah_validation_context_t *vctx, const wah_code_body_t* code_body);

// Pre-parsing functions
static wah_error_t wah_preparse_code(const wah_module_t* module, uint32_t func_idx, const uint8_t *code, uint32_t code_size, wah_parsed_code_t *parsed_code);
static void wah_free_parsed_code(wah_parsed_code_t *parsed_code);

// Const expression functions
static wah_error_t wah_eval_const_expr(wah_exec_context_t *ctx, const uint8_t *bytecode, uint32_t bytecode_size, wah_value_t *result);
static void wah_free_element_segment_data(wah_element_segment_t *segment);

// WebAssembly canonical NaN bit patterns
//
// Note that WebAssembly prior to 3.0 had *two* canonical NaNs varying only by their signs.
// These are canonical in terms of the WebAssembly 3.0 deterministic profile, which needs a positive sign.
static const union { uint32_t i; float f; } WAH_CANONICAL_NAN32 = { .i = 0x7fc00000U };
static const union { uint64_t i; double f; } WAH_CANONICAL_NAN64 = { .i = 0x7ff8000000000000ULL };

static inline float wah_canonicalize_f32(float val) { return val == val ? val : WAH_CANONICAL_NAN32.f; }
static inline double wah_canonicalize_f64(double val) { return val == val ? val : WAH_CANONICAL_NAN64.f; }

#ifdef WAH_X86_64 // SSE2

static inline __m128 wah_canonicalize_f32x4_sse2(__m128 v) {
    __m128 nan_mask = _mm_cmpeq_ps(v, v);
    return _mm_or_ps(_mm_and_ps(nan_mask, v), _mm_andnot_ps(nan_mask, _mm_set1_ps(WAH_CANONICAL_NAN32.f)));
}

static inline __m128d wah_canonicalize_f64x2_sse2(__m128d v) {
    __m128d nan_mask = _mm_cmpeq_pd(v, v);
    return _mm_or_pd(_mm_and_pd(nan_mask, v), _mm_andnot_pd(nan_mask, _mm_set1_pd(WAH_CANONICAL_NAN64.f)));
}

#endif

#ifdef WAH_AARCH64 // NEON

static WAH_ALWAYS_INLINE uint8x16_t wah_canonicalize_f32x4_neon(uint8x16_t v) {
    float32x4_t fv = vreinterpretq_f32_u8(v);
    uint32x4_t not_nan = vceqq_f32(fv, fv);
    return vreinterpretq_u8_f32(vbslq_f32(not_nan, fv, vdupq_n_f32(WAH_CANONICAL_NAN32.f)));
}

static WAH_ALWAYS_INLINE uint8x16_t wah_canonicalize_f64x2_neon(uint8x16_t v) {
    float64x2_t fv = vreinterpretq_f64_u8(v);
    uint64x2_t not_nan = vceqq_f64(fv, fv);
    return vreinterpretq_u8_f64(vbslq_f64(not_nan, fv, vdupq_n_f64(WAH_CANONICAL_NAN64.f)));
}

#endif

// --- Integer Utility Functions ---

// popcnt
static WAH_ALWAYS_INLINE uint32_t wah_popcount_u32(uint32_t n) {
#if WAH_HAS_BUILTIN(__builtin_popcount) || defined(__GNUC__)
    return __builtin_popcount(n);
#elif defined(_MSC_VER)
    return __popcnt(n);
#else
    // Generic software implementation
    uint32_t count = 0;
    while (n > 0) {
        n &= (n - 1);
        count++;
    }
    return count;
#endif
}

static WAH_ALWAYS_INLINE uint64_t wah_popcount_u64(uint64_t n) {
#if WAH_HAS_BUILTIN(__builtin_popcountll) || defined(__GNUC__)
    return __builtin_popcountll(n);
#elif defined(_MSC_VER)
    return __popcnt64(n);
#else
    // Generic software implementation
    uint64_t count = 0;
    while (n > 0) {
        n &= (n - 1);
        count++;
    }
    return count;
#endif
}

static WAH_ALWAYS_INLINE uint8_t wah_popcount_u8(uint8_t n) {
    uint8_t count = 0;
    while (n > 0) {
        n &= (n - 1);
        count++;
    }
    return count;
}

// clz (count leading zeros)
static WAH_ALWAYS_INLINE uint32_t wah_clz_u32(uint32_t n) {
#if WAH_HAS_BUILTIN(__builtin_clz) || defined(__GNUC__)
    return n == 0 ? 32 : __builtin_clz(n);
#elif defined(_MSC_VER)
    unsigned long index;
    if (_BitScanReverse(&index, n)) {
        return 31 - index;
    } else {
        return 32; // All bits are zero
    }
#else
    // Generic software implementation
    if (n == 0) return 32;
    uint32_t count = 0;
    if (n <= 0x0000FFFF) { count += 16; n <<= 16; }
    if (n <= 0x00FFFFFF) { count += 8; n <<= 8; }
    if (n <= 0x0FFFFFFF) { count += 4; n <<= 4; }
    if (n <= 0x3FFFFFFF) { count += 2; n <<= 2; }
    if (n <= 0x7FFFFFFF) { count += 1; }
    return count;
#endif
}

static WAH_ALWAYS_INLINE uint64_t wah_clz_u64(uint64_t n) {
#if WAH_HAS_BUILTIN(__builtin_clzll) || defined(__GNUC__)
    return n == 0 ? 64 : __builtin_clzll(n);
#elif defined(_MSC_VER)
    unsigned long index;
    if (_BitScanReverse64(&index, n)) {
        return 63 - index;
    } else {
        return 64; // All bits are zero
    }
#else
    // Generic software implementation
    if (n == 0) return 64;
    uint64_t count = 0;
    if (n <= 0x00000000FFFFFFFFULL) { count += 32; n <<= 32; }
    if (n <= 0x0000FFFFFFFFFFFFULL) { count += 16; n <<= 16; }
    if (n <= 0x00FFFFFFFFFFFFFFULL) { count += 8; n <<= 8; }
    if (n <= 0x0FFFFFFFFFFFFFFFULL) { count += 4; n <<= 4; }
    if (n <= 0x3FFFFFFFFFFFFFFFULL) { count += 2; n <<= 2; }
    if (n <= 0x7FFFFFFFFFFFFFFFULL) { count += 1; }
    return count;
#endif
}

// ctz (count trailing zeros)
static WAH_ALWAYS_INLINE uint32_t wah_ctz_u32(uint32_t n) {
#if WAH_HAS_BUILTIN(__builtin_ctz) || defined(__GNUC__)
    return n == 0 ? 32 : __builtin_ctz(n);
#elif defined(_MSC_VER)
    unsigned long index;
    if (_BitScanForward(&index, n)) {
        return index;
    } else {
        return 32; // All bits are zero
    }
#else
    // Generic software implementation
    if (n == 0) return 32;
    uint32_t count = 0;
    while ((n & 1) == 0) {
        n >>= 1;
        count++;
    }
    return count;
#endif
}

static WAH_ALWAYS_INLINE uint64_t wah_ctz_u64(uint64_t n) {
#if WAH_HAS_BUILTIN(__builtin_ctzll) || defined(__GNUC__)
    return n == 0 ? 64 : __builtin_ctzll(n);
#elif defined(_MSC_VER)
    unsigned long index;
    if (_BitScanForward64(&index, n)) {
        return index;
    } else {
        return 64; // All bits are zero
    }
#else
    // Generic software implementation
    if (n == 0) return 64;
    uint64_t count = 0;
    while ((n & 1) == 0) {
        n >>= 1;
        count++;
    }
    return count;
#endif
}

// rotl (rotate left)
static WAH_ALWAYS_INLINE uint32_t wah_rotl_u32(uint32_t n, uint32_t shift) {
#if WAH_HAS_BUILTIN(__builtin_rotateleft32)
    return __builtin_rotateleft32(n, shift);
#elif defined(_MSC_VER)
    return _rotl(n, shift);
#else
    shift &= 31; // Ensure shift is within 0-31
    return (n << shift) | (n >> (32 - shift));
#endif
}

static WAH_ALWAYS_INLINE uint64_t wah_rotl_u64(uint64_t n, uint64_t shift) {
#if WAH_HAS_BUILTIN(__builtin_rotateleft64)
    return __builtin_rotateleft64(n, shift);
#elif defined(_MSC_VER)
    return _rotl64(n, (int)shift);
#else
    shift &= 63; // Ensure shift is within 0-63
    return (n << shift) | (n >> (64 - shift));
#endif
}

// rotr (rotate right)
static WAH_ALWAYS_INLINE uint32_t wah_rotr_u32(uint32_t n, uint32_t shift) {
#if WAH_HAS_BUILTIN(__builtin_rotateright32)
    return __builtin_rotateright32(n, shift);
#elif defined(_MSC_VER)
    return _rotr(n, shift);
#else
    shift &= 31; // Ensure shift is within 0-31
    return (n >> shift) | (n << (32 - shift));
#endif
}

static WAH_ALWAYS_INLINE uint64_t wah_rotr_u64(uint64_t n, uint64_t shift) {
#if WAH_HAS_BUILTIN(__builtin_rotateright64)
    return __builtin_rotateright64(n, shift);
#elif defined(_MSC_VER)
    return _rotr64(n, (int)shift);
#else
    shift &= 63; // Ensure shift is within 0-63
    return (n >> shift) | (n << (64 - shift));
#endif
}

// Helper function for Q15 multiplication with rounding and saturation
static WAH_ALWAYS_INLINE wah_v128_t wah_q15mulr_sat_s(wah_v128_t a, wah_v128_t b) {
    wah_v128_t result;
    for (int i = 0; i < 8; ++i) {
        int32_t prod = (int32_t)a.i16[i] * b.i16[i];
        int32_t res = (prod + 16384) >> 15; // Rounding and shift
        if (res > 32767) res = 32767;
        if (res < -32768) res = -32768;
        result.i16[i] = (int16_t)res;
    }
    return result;
}

#ifdef WAH_X86_64

static WAH_ALWAYS_INLINE int32_t wah_i16x8_bitmask_sse2(__m128i v) {
    int32_t mask = _mm_movemask_epi8(_mm_cmpeq_epi16(v, _mm_setzero_si128())) & 0x5555; // 0a0b 0c0d 0e0f 0g0h
    mask = (mask | (mask >> 1)) & 0x3333; // 00ab 00cd 00ef 00gh
    mask = (mask | (mask >> 2)) & 0x0f0f; // 0000 abcd 0000 efgh
    return (mask | (mask >> 4)) & 0x00ff; // 0000 0000 abcd efgh
}

static WAH_ALWAYS_INLINE int32_t wah_i32x4_bitmask_sse2(__m128i v) {
    int32_t mask = _mm_movemask_epi8(_mm_cmpeq_epi32(v, _mm_setzero_si128())) & 0x1111; // 000a 000b 000c 000d
    mask = (mask | (mask >> 3)) & 0x0202; // 0000 00ab 0000 00cd
    return (mask | (mask >> 6)) & 0x000f; // 0000 0000 0000 abcd
}

static WAH_ALWAYS_INLINE int32_t wah_i64x2_bitmask_sse2(__m128i v) {
    // Compare each 64-bit element with zero
    // Since we're comparing with zero, we can check if both 32-bit halves are zero
    __m128i v_hi = _mm_shuffle_epi32(v, _MM_SHUFFLE(3, 3, 1, 1));  // [v1_hi, v1_hi, v0_hi, v0_hi]
    __m128i v_lo = v;  // [v1_lo, v1_lo, v0_lo, v0_lo]
    __m128i eq_lo = _mm_cmpeq_epi32(v_lo, _mm_setzero_si128());  // v_lo == 0
    __m128i eq_hi = _mm_cmpeq_epi32(v_hi, _mm_setzero_si128());  // v_hi == 0
    __m128i eq64 = _mm_and_si128(eq_lo, eq_hi);  // Both halves equal (entire 64-bit is zero)
    int32_t mask = _mm_movemask_epi8(eq64) & 0x0101;  // 0000 000a 0000 000b
    return (mask | (mask >> 7)) & 0x0003;  // 0000 0000 0000 00ab
}

#endif

// nearest (round to nearest, ties to even)
// XXX: Clang doesn't support __builtin_roundeven(f) without recent enough -march, so we opt in for known archs
static WAH_ALWAYS_INLINE float wah_nearest_f32(float f) {
#if WAH_HAS_BUILTIN(__builtin_roundevenf) && defined(__clang__) && defined(__SSE4_1__)
    return __builtin_roundevenf(f);
#else
    if (isnan(f) || isinf(f) || f == 0.0f) return f;
    float rounded = roundf(f);
    if (fabsf(f - rounded) == 0.5f && ((long long)rounded % 2) != 0) return rounded - copysignf(1.0f, f);
    return rounded;
#endif
}

static WAH_ALWAYS_INLINE double wah_nearest_f64(double d) {
#if WAH_HAS_BUILTIN(__builtin_roundeven) && defined(__clang__) && defined(__SSE4_1__)
    return __builtin_roundeven(d);
#else
    if (isnan(d) || isinf(d) || d == 0.0) return d;
    double rounded = round(d);
    if (fabs(d - rounded) == 0.5 && ((long long)rounded % 2) != 0) return rounded - copysign(1.0, d);
    return rounded;
#endif
}

// Helper functions for floating-point to integer truncations with trap handling
#define DEFINE_TRUNC_F2I(N, fty, T, ity, lo, hi, call) \
static WAH_ALWAYS_INLINE wah_error_t wah_trunc_f##N##_to_##T(fty val, ity *result) { \
    if (isnan(val) || isinf(val)) return WAH_ERROR_TRAP; \
    if (val < (lo) || val >= (hi)) return WAH_ERROR_TRAP; \
    *result = (ity)call(val); \
    return WAH_OK; \
}

DEFINE_TRUNC_F2I(32, float,  i32,  int32_t,  (float)INT32_MIN,  (float) INT32_MAX + 1.0f, truncf)
DEFINE_TRUNC_F2I(32, float,  u32, uint32_t,                 0,  (float)UINT32_MAX + 1.0f, truncf)
DEFINE_TRUNC_F2I(64, double, i32,  int32_t, (double)INT32_MIN, (double) INT32_MAX + 1.0,  trunc)
DEFINE_TRUNC_F2I(64, double, u32, uint32_t,                 0, (double)UINT32_MAX + 1.0,  trunc)
DEFINE_TRUNC_F2I(32, float,  i64,  int64_t,  (float)INT64_MIN,  (float) INT64_MAX + 1.0f, truncf)
DEFINE_TRUNC_F2I(32, float,  u64, uint64_t,                 0,  (float)UINT64_MAX + 1.0f, truncf)
DEFINE_TRUNC_F2I(64, double, i64,  int64_t, (double)INT64_MIN, (double) INT64_MAX + 1.0,  trunc)
DEFINE_TRUNC_F2I(64, double, u64, uint64_t,                 0, (double)UINT64_MAX + 1.0,  trunc)

// Helper functions for floating-point to integer truncations with saturation
#define DEFINE_TRUNC_SAT_F2I(N, fty, T, ity, min_val, max_val, call) \
static WAH_ALWAYS_INLINE ity wah_trunc_sat_f##N##_to_##T(fty val) { \
    if (isnan(val)) return 0; \
    if (val <= (fty)min_val) return min_val; \
    if (val >= (fty)max_val) return max_val; \
    return (ity)call(val); \
}

DEFINE_TRUNC_SAT_F2I(32, float,  i32,  int32_t,  INT32_MIN,  INT32_MAX, truncf)
DEFINE_TRUNC_SAT_F2I(32, float,  u32, uint32_t,          0, UINT32_MAX, truncf)
DEFINE_TRUNC_SAT_F2I(64, double, i32,  int32_t,  INT32_MIN,  INT32_MAX, trunc)
DEFINE_TRUNC_SAT_F2I(64, double, u32, uint32_t,          0, UINT32_MAX, trunc)
DEFINE_TRUNC_SAT_F2I(32, float,  i64,  int64_t,  INT64_MIN,  INT64_MAX, truncf)
DEFINE_TRUNC_SAT_F2I(32, float,  u64, uint64_t,          0, UINT64_MAX, truncf)
DEFINE_TRUNC_SAT_F2I(64, double, i64,  int64_t,  INT64_MIN,  INT64_MAX, trunc)
DEFINE_TRUNC_SAT_F2I(64, double, u64, uint64_t,          0, UINT64_MAX, trunc)

static WAH_ALWAYS_INLINE int8_t wah_trunc_sat_i16_to_i8(int16_t val) {
    if (val < -128) return -128;
    if (val > 127) return 127;
    return (int8_t)val;
}

static WAH_ALWAYS_INLINE uint8_t wah_trunc_sat_i16_to_u8(int16_t val) {
    if (val < 0) return 0;
    if (val > 255) return 255;
    return (uint8_t)val;
}

static WAH_ALWAYS_INLINE int16_t wah_trunc_sat_i32_to_i16(int32_t val) {
    if (val < -32768) return -32768;
    if (val > 32767) return 32767;
    return (int16_t)val;
}

static WAH_ALWAYS_INLINE uint16_t wah_trunc_sat_i32_to_u16(int32_t val) {
    if (val < 0) return 0;
    if (val > 65535) return 65535;
    return (uint16_t)val;
}

static WAH_ALWAYS_INLINE int16_t WAH_MIN_S_16(int16_t a, int16_t b) { return a < b ? a : b; }
static WAH_ALWAYS_INLINE uint16_t WAH_MIN_U_16(uint16_t a, uint16_t b) { return a < b ? a : b; }
static WAH_ALWAYS_INLINE int16_t WAH_MAX_S_16(int16_t a, int16_t b) { return a > b ? a : b; }
static WAH_ALWAYS_INLINE uint16_t WAH_MAX_U_16(uint16_t a, uint16_t b) { return a > b ? a : b; }

static WAH_ALWAYS_INLINE int32_t WAH_MIN_S_32(int32_t a, int32_t b) { return a < b ? a : b; }
static WAH_ALWAYS_INLINE uint32_t WAH_MIN_U_32(uint32_t a, uint32_t b) { return a < b ? a : b; }
static WAH_ALWAYS_INLINE int32_t WAH_MAX_S_32(int32_t a, int32_t b) { return a > b ? a : b; }
static WAH_ALWAYS_INLINE uint32_t WAH_MAX_U_32(uint32_t a, uint32_t b) { return a > b ? a : b; }

static WAH_ALWAYS_INLINE int8_t WAH_MIN_S_8(int8_t a, int8_t b) { return a < b ? a : b; }
static WAH_ALWAYS_INLINE uint8_t WAH_MIN_U_8(uint8_t a, uint8_t b) { return a < b ? a : b; }
static WAH_ALWAYS_INLINE int8_t WAH_MAX_S_8(int8_t a, int8_t b) { return a > b ? a : b; }
static WAH_ALWAYS_INLINE uint8_t WAH_MAX_U_8(uint8_t a, uint8_t b) { return a > b ? a : b; }

static WAH_ALWAYS_INLINE float wah_pminf(float a, float b) {
    if (isnan(a) || isnan(b)) return WAH_CANONICAL_NAN32.f;
    return fminf(a, b);
}

static WAH_ALWAYS_INLINE float wah_pmaxf(float a, float b) {
    if (isnan(a) || isnan(b)) return WAH_CANONICAL_NAN32.f;
    return fmaxf(a, b);
}

static WAH_ALWAYS_INLINE double wah_pmin(double a, double b) {
    if (isnan(a) || isnan(b)) return WAH_CANONICAL_NAN64.f;
    return fmin(a, b);
}

static WAH_ALWAYS_INLINE double wah_pmax(double a, double b) {
    if (isnan(a) || isnan(b)) return WAH_CANONICAL_NAN64.f;
    return fmax(a, b);
}

#ifdef WAH_X86_64

#define DEFINE_PMINMAX(wasm_type, minmax, __m128x, suffix, canon_nan_s) \
static WAH_ALWAYS_INLINE __m128x wah_##wasm_type##_p##minmax##_sse2(__m128x a, __m128x b) { \
    /* Check for NaN in either operand */ \
    __m128x a_is_nan = _mm_cmpunord_##suffix(a, a); \
    __m128x b_is_nan = _mm_cmpunord_##suffix(b, b); \
    __m128x any_nan = _mm_or_##suffix(a_is_nan, b_is_nan); \
    /* Regular min/max for non-NaN case */ \
    __m128x result = _mm_##minmax##_##suffix(a, b); \
    /* If either operand is NaN, replace result with canonical NaN (WASM semantics) */ \
    __m128x canon_nan = _mm_set1_##suffix(canon_nan_s); \
    return _mm_or_##suffix(_mm_andnot_##suffix(any_nan, result), _mm_and_##suffix(any_nan, canon_nan)); \
}

DEFINE_PMINMAX(f32x4, min, __m128,  ps, WAH_CANONICAL_NAN32.f)
DEFINE_PMINMAX(f32x4, max, __m128,  ps, WAH_CANONICAL_NAN32.f)
DEFINE_PMINMAX(f64x2, min, __m128d, pd, WAH_CANONICAL_NAN64.f)
DEFINE_PMINMAX(f64x2, max, __m128d, pd, WAH_CANONICAL_NAN64.f)

#define DEFINE_CMP_U(wasm_type, cmp, const, N) \
static WAH_ALWAYS_INLINE __m128i wah_##wasm_type##_##cmp##_u_sse2(__m128i a, __m128i b) { \
    __m128i sign_bit = _mm_set1_epi##N(const); \
    __m128i a_flipped = _mm_xor_si128(a, sign_bit); \
    __m128i b_flipped = _mm_xor_si128(b, sign_bit); \
    return _mm_cmp##cmp##_epi##N(a_flipped, b_flipped); \
}

DEFINE_CMP_U(i8x16, lt, 0x80, 8)
DEFINE_CMP_U(i8x16, gt, 0x80, 8)
DEFINE_CMP_U(i16x8, lt, 0x8000, 16)
DEFINE_CMP_U(i16x8, gt, 0x8000, 16)
DEFINE_CMP_U(i32x4, lt, 0x80000000, 32)
DEFINE_CMP_U(i32x4, gt, 0x80000000, 32)

static WAH_ALWAYS_INLINE __m128i wah_v128_not_sse2(__m128i x) { return _mm_xor_si128(x, _mm_set1_epi8(0xff)); }
static WAH_ALWAYS_INLINE __m128i wah_v128_andnot_rev_sse2(__m128i a, __m128i b) { return _mm_andnot_si128(b, a); }

// I64X2 multiply using SSE2
// Computes low 64 bits of 64x64 multiplication (what WebAssembly requires)
static WAH_ALWAYS_INLINE __m128i wah_i64x2_mul_sse2(__m128i a, __m128i b) {
    // For each 64-bit element: result = (a * b) & 0xffffffffffffffff
    // Split each 64-bit value into high and low 32-bit parts
    // a = a_hi * 2^32 + a_lo, b = b_hi * 2^32 + b_lo
    // low_64(a * b) = ((a_hi * b_lo + a_lo * b_hi) << 32) + low_32(a_lo * b_lo)

    __m128i a_lo = _mm_shuffle_epi32(a, _MM_SHUFFLE(2, 0, 2, 0));  // [a0_lo, a0_lo, a1_lo, a1_lo]
    __m128i a_hi = _mm_shuffle_epi32(a, _MM_SHUFFLE(3, 1, 3, 1));  // [a0_hi, a0_hi, a1_hi, a1_hi]
    __m128i b_lo = _mm_shuffle_epi32(b, _MM_SHUFFLE(2, 0, 2, 0));  // [b0_lo, b0_lo, b1_lo, b1_lo]
    __m128i b_hi = _mm_shuffle_epi32(b, _MM_SHUFFLE(3, 1, 3, 1));  // [b0_hi, b0_hi, b1_hi, b1_hi]

    // 32x32 -> 64 multiplications
    __m128i prod_ll = _mm_mul_epu32(a_lo, b_lo);  // [a0_lo*b0_lo, a1_lo*b1_lo]
    __m128i prod_hl = _mm_mul_epu32(a_hi, b_lo);  // [a0_hi*b0_lo, a1_hi*b1_lo]
    __m128i prod_lh = _mm_mul_epu32(a_lo, b_hi);  // [a0_lo*b0_hi, a1_lo*b1_hi]

    // Sum cross terms
    __m128i prod_cross = _mm_add_epi64(prod_hl, prod_lh);

    // Shift cross terms left by 32 bits (each 64-bit element)
    __m128i prod_cross_shuffled = _mm_shuffle_epi32(prod_cross, _MM_SHUFFLE(3, 1, 2, 0));
    __m128i prod_cross_shifted = _mm_slli_si128(prod_cross_shuffled, 4);

    // Add low parts
    __m128i result = _mm_add_epi64(prod_ll, prod_cross_shifted);

    return result;
}

// I8X16 popcount using SSE2
// Counts set bits in each of 16 bytes
static WAH_ALWAYS_INLINE __m128i wah_i8x16_popcnt_sse2(__m128i v) {
    // Parallel popcount: count bits in pairs, then nibbles, then bytes

    // Step 1: Count bits in pairs (2 bits)
    // For each 2-bit pair, add the bits together
    __m128i m1 = _mm_set1_epi8(0x55);  // 01010101
    __m128i v1 = _mm_and_si128(v, m1);                     // Even-position bits
    __m128i v2 = _mm_and_si128(_mm_srli_epi64(v, 1), m1);   // Odd-position bits
    __m128i pair_count = _mm_add_epi8(v1, v2);              // Count in each pair (0-2)

    // Step 2: Count bits in nibbles (4 bits)
    // Add adjacent 2-bit counts
    __m128i m2 = _mm_set1_epi8(0x33);  // 00110011
    __m128i pc_low = _mm_and_si128(pair_count, m2);
    __m128i pc_high = _mm_and_si128(_mm_srli_epi64(pair_count, 2), m2);
    __m128i nibble_count = _mm_add_epi8(pc_low, pc_high);  // Count in each nibble (0-4)

    // Step 3: Count bits in bytes (8 bits)
    // Add adjacent nibble counts
    __m128i nc_low = _mm_and_si128(nibble_count, _mm_set1_epi8(0x0f));
    __m128i nc_high = _mm_and_si128(_mm_srli_epi64(nibble_count, 4), _mm_set1_epi8(0x0f));
    __m128i byte_count = _mm_add_epi8(nc_low, nc_high);  // Count in each byte (0-8)

    return byte_count;
}

// abs(x) = (x ^ mask) - mask where mask = (x < 0) ? 0xFF : 0x00
static WAH_ALWAYS_INLINE __m128i wah_i8x16_abs_sse2(__m128i x) {
    __m128i mask = _mm_cmplt_epi8(x, _mm_setzero_si128());
    return _mm_sub_epi8(_mm_xor_si128(x, mask), mask);
}
static WAH_ALWAYS_INLINE __m128i wah_i16x8_abs_sse2(__m128i x) {
    __m128i mask = _mm_cmplt_epi16(x, _mm_setzero_si128());
    return _mm_sub_epi16(_mm_xor_si128(x, mask), mask);
}
static WAH_ALWAYS_INLINE __m128i wah_i32x4_abs_sse2(__m128i x) {
    __m128i mask = _mm_cmplt_epi32(x, _mm_setzero_si128());
    return _mm_sub_epi32(_mm_xor_si128(x, mask), mask);
}
static WAH_ALWAYS_INLINE __m128i wah_i64x2_abs_sse2(__m128i x) { // For 64-bit: if MSB is set, negate: (~x + 1)
    __m128i mask = _mm_srai_epi32(x, 31);  // Get sign bit of each 64-bit element (as 32-bit pairs)
    __m128i mask64 = _mm_shuffle_epi32(mask, _MM_SHUFFLE(3, 3, 1, 1));  // Spread sign to both 32-bit parts
    __m128i neg = _mm_sub_epi64(_mm_xor_si128(x, mask64), mask64);
    __m128i is_neg = _mm_cmpeq_epi32(mask, _mm_set1_epi32(0xFFFFFFFF));
    __m128i is_neg64 = _mm_shuffle_epi32(is_neg, _MM_SHUFFLE(3, 3, 1, 1));
    return _mm_or_si128(_mm_and_si128(neg, is_neg64), _mm_andnot_si128(is_neg64, x));
}

// SSE2 implementations for SSE4.1 SIMD instructions

static WAH_ALWAYS_INLINE __m128i wah_i8x16_min_s_sse2(__m128i a, __m128i b) {
    __m128i mask = _mm_cmplt_epi8(a, b);
    return _mm_or_si128(_mm_and_si128(mask, a), _mm_andnot_si128(mask, b));
}
static WAH_ALWAYS_INLINE __m128i wah_i8x16_max_s_sse2(__m128i a, __m128i b) {
    __m128i mask = _mm_cmpgt_epi8(a, b);
    return _mm_or_si128(_mm_and_si128(mask, a), _mm_andnot_si128(mask, b));
}

static WAH_ALWAYS_INLINE __m128i wah_i16x8_min_u_sse2(__m128i a, __m128i b) {
    __m128i a_xor = _mm_xor_si128(a, _mm_set1_epi16(0x8000));
    __m128i b_xor = _mm_xor_si128(b, _mm_set1_epi16(0x8000));
    __m128i mask = _mm_cmplt_epi16(a_xor, b_xor);
    return _mm_or_si128(_mm_and_si128(mask, a), _mm_andnot_si128(mask, b));
}
static WAH_ALWAYS_INLINE __m128i wah_i16x8_max_u_sse2(__m128i a, __m128i b) {
    __m128i a_xor = _mm_xor_si128(a, _mm_set1_epi16(0x8000));
    __m128i b_xor = _mm_xor_si128(b, _mm_set1_epi16(0x8000));
    __m128i mask = _mm_cmpgt_epi16(a_xor, b_xor);
    return _mm_or_si128(_mm_and_si128(mask, a), _mm_andnot_si128(mask, b));
}

static WAH_ALWAYS_INLINE __m128i wah_i32x4_mul_sse2(__m128i a, __m128i b) {
    // i32x4 mul: multiply each 32-bit lane, keep low 32 bits of result
    // SSE2's _mm_mul_epi32 only works on 2 elements at a time, so we do it in pairs

    // Multiply a[0,2] * b[0,2]
    __m128i prod02 = _mm_mul_epu32(a, b);
    // prod02 = [a0*b0 (64-bit), a2*b2 (64-bit)]

    // Multiply a[1,3] * b[1,3] - need to shuffle first
    __m128i a_13 = _mm_shuffle_epi32(a, _MM_SHUFFLE(3, 3, 1, 1));
    __m128i b_13 = _mm_shuffle_epi32(b, _MM_SHUFFLE(3, 3, 1, 1));
    __m128i prod13 = _mm_mul_epu32(a_13, b_13);
    // prod13 = [a1*b1 (64-bit), a3*b3 (64-bit)]

    // Interleave low 32 bits to get final order: [a0*b0, a1*b1, a2*b2, a3*b3]
    return _mm_unpacklo_epi32(prod02, prod13);
}

static WAH_ALWAYS_INLINE __m128i wah_i32x4_min_s_sse2(__m128i a, __m128i b) {
    __m128i mask = _mm_cmplt_epi32(a, b);
    return _mm_or_si128(_mm_and_si128(mask, a), _mm_andnot_si128(mask, b));
}
static WAH_ALWAYS_INLINE __m128i wah_i32x4_max_s_sse2(__m128i a, __m128i b) {
    __m128i mask = _mm_cmpgt_epi32(a, b);
    return _mm_or_si128(_mm_and_si128(mask, a), _mm_andnot_si128(mask, b));
}

static WAH_ALWAYS_INLINE __m128i wah_i32x4_min_u_sse2(__m128i a, __m128i b) {
    __m128i a_xor = _mm_xor_si128(a, _mm_set1_epi32(0x80000000));
    __m128i b_xor = _mm_xor_si128(b, _mm_set1_epi32(0x80000000));
    __m128i mask = _mm_cmplt_epi32(a_xor, b_xor);
    return _mm_or_si128(_mm_and_si128(mask, a), _mm_andnot_si128(mask, b));
}
static WAH_ALWAYS_INLINE __m128i wah_i32x4_max_u_sse2(__m128i a, __m128i b) {
    __m128i a_xor = _mm_xor_si128(a, _mm_set1_epi32(0x80000000));
    __m128i b_xor = _mm_xor_si128(b, _mm_set1_epi32(0x80000000));
    __m128i mask = _mm_cmpgt_epi32(a_xor, b_xor);
    return _mm_or_si128(_mm_and_si128(mask, a), _mm_andnot_si128(mask, b));
}

static WAH_ALWAYS_INLINE __m128i wah_i16x8_narrow_i32x4_u_sse2(__m128i a, __m128i b) {
    __m128i max_val = _mm_set1_epi32(65535);
    __m128i a_gt_max = _mm_cmpgt_epi32(a, max_val);
    __m128i b_gt_max = _mm_cmpgt_epi32(b, max_val);
    __m128i a_lt_zero = _mm_cmplt_epi32(a, _mm_setzero_si128());
    __m128i b_lt_zero = _mm_cmplt_epi32(b, _mm_setzero_si128());

    __m128i a_clamped = _mm_or_si128(_mm_and_si128(a_gt_max, max_val), _mm_andnot_si128(a_gt_max, a));
    __m128i b_clamped = _mm_or_si128(_mm_and_si128(b_gt_max, max_val), _mm_andnot_si128(b_gt_max, b));
    a_clamped = _mm_andnot_si128(a_lt_zero, a_clamped);
    b_clamped = _mm_andnot_si128(b_lt_zero, b_clamped);

    __m128i ab_low = _mm_unpacklo_epi32(a_clamped, b_clamped);
    __m128i ab_high = _mm_unpackhi_epi32(a_clamped, b_clamped);

    __m128i low = _mm_shuffle_epi32(ab_low, _MM_SHUFFLE(3, 1, 2, 0));
    __m128i high = _mm_shuffle_epi32(ab_high, _MM_SHUFFLE(3, 1, 2, 0));

    return _mm_unpacklo_epi64(low, high);
}

static WAH_ALWAYS_INLINE int32_t wah_i64x2_all_true_sse2(__m128i v) {
    __m128i v_hi = _mm_shuffle_epi32(v, _MM_SHUFFLE(3, 3, 1, 1));
    __m128i eq_lo = _mm_cmpeq_epi32(v, _mm_setzero_si128());
    __m128i eq_hi = _mm_cmpeq_epi32(v_hi, _mm_setzero_si128());
    __m128i eq64 = _mm_and_si128(eq_lo, eq_hi);
    return _mm_movemask_epi8(eq64) == 0xffff;
}

// Since SSE2 has no byte shift, we unpack to 16-bit, shift, and repack
static WAH_ALWAYS_INLINE __m128i wah_i8x16_shl_sse2(__m128i a, int32_t count) {
    count &= 7;  // Mask to 0-7
    if (count == 0) return a;
    __m128i zero = _mm_setzero_si128();
    __m128i a_lo = _mm_unpacklo_epi8(a, zero);   // [a0, a1, ..., a7] as 16-bit
    __m128i a_hi = _mm_unpackhi_epi8(a, zero);   // [a8, a9, ..., a15] as 16-bit
    __m128i lo = _mm_slli_epi16(a_lo, count);
    __m128i hi = _mm_slli_epi16(a_hi, count);
    // Mask to get only low 8 bits of each 16-bit element
    __m128i mask = _mm_set1_epi16(0xFF);
    lo = _mm_and_si128(lo, mask);
    hi = _mm_and_si128(hi, mask);
    return _mm_packus_epi16(lo, hi);
}
static WAH_ALWAYS_INLINE __m128i wah_i8x16_shr_s_sse2(__m128i a, int32_t count) {
    count &= 7;  // Mask to 0-7
    if (count == 0) return a;
    __m128i zero = _mm_setzero_si128();
    __m128i a_lo = _mm_unpacklo_epi8(a, zero);
    __m128i a_hi = _mm_unpackhi_epi8(a, zero);
    __m128i lo = _mm_srai_epi16(a_lo, count);
    __m128i hi = _mm_srai_epi16(a_hi, count);
    return _mm_packs_epi16(lo, hi);
}
static WAH_ALWAYS_INLINE __m128i wah_i8x16_shr_u_sse2(__m128i a, int32_t count) {
    count &= 7;  // Mask to 0-7
    if (count == 0) return a;
    __m128i zero = _mm_setzero_si128();
    __m128i a_lo = _mm_unpacklo_epi8(a, zero);
    __m128i a_hi = _mm_unpackhi_epi8(a, zero);
    __m128i lo = _mm_srli_epi16(a_lo, count);
    __m128i hi = _mm_srli_epi16(a_hi, count);
    return _mm_packus_epi16(lo, hi);
}

// I7X16 sign-extension: treat lower 7 bits as signed (bit6=sign, bit7 ignored)
// (b & 0x7F) | ((b << 1) & 0x80) -- shifts bit6 into bit7 position for sign
static WAH_ALWAYS_INLINE __m128i wah_i7x16_to_i8_sse2(__m128i b) {
    return _mm_or_si128(
        _mm_and_si128(b, _mm_set1_epi8(0x7F)),
        _mm_and_si128(_mm_add_epi8(b, b), _mm_set1_epi8(0x80))
    );
}

// I16X8 relaxed dot: i8x16 (signed) * i7x16 (7-bit signed), sum adjacent pairs -> i16x8
// Correctly sign-extends i7 operand, unlike pmaddubsw which treats both as i8.
static WAH_ALWAYS_INLINE __m128i wah_i16x8_relaxed_dot_i8x16_i7x16_s_sse2(__m128i a, __m128i b) {
    __m128i b_i8 = wah_i7x16_to_i8_sse2(b);
    __m128i zero = _mm_setzero_si128();
    // Sign-extend i8 bytes to i16 for both operands
    __m128i a_sign = _mm_cmplt_epi8(a, zero);      // 0xFF if negative, 0x00 otherwise
    __m128i b_sign = _mm_cmplt_epi8(b_i8, zero);
    __m128i a_lo = _mm_unpacklo_epi8(a, a_sign);   // bytes 0-7 as i16x8
    __m128i a_hi = _mm_unpackhi_epi8(a, a_sign);   // bytes 8-15 as i16x8
    __m128i b_lo = _mm_unpacklo_epi8(b_i8, b_sign);
    __m128i b_hi = _mm_unpackhi_epi8(b_i8, b_sign);
    // pmaddwd: result[i] = a[2i]*b[2i] + a[2i+1]*b[2i+1] as i32
    __m128i dot_lo = _mm_madd_epi16(a_lo, b_lo);   // 4 i32 values from bytes 0-7
    __m128i dot_hi = _mm_madd_epi16(a_hi, b_hi);   // 4 i32 values from bytes 8-15
    // Pack i32 -> i16; max |result| = 127*63*2 = 16002 < 32767, no saturation
    return _mm_packs_epi32(dot_lo, dot_hi);
}

// I32X4 relaxed dot-add: sum 4 consecutive (i8*i7) products per lane, add accumulator
// Uses shuffle trick for horizontal add (avoids SSSE3 phaddd).
static WAH_ALWAYS_INLINE __m128i wah_i32x4_relaxed_dot_i8x16_i7x16_add_s_sse2(__m128i a, __m128i b, __m128i c) {
    __m128i b_i8 = wah_i7x16_to_i8_sse2(b);
    __m128i zero = _mm_setzero_si128();
    __m128i a_sign = _mm_cmplt_epi8(a, zero);
    __m128i b_sign = _mm_cmplt_epi8(b_i8, zero);
    __m128i a_lo = _mm_unpacklo_epi8(a, a_sign);
    __m128i a_hi = _mm_unpackhi_epi8(a, a_sign);
    __m128i b_lo = _mm_unpacklo_epi8(b_i8, b_sign);
    __m128i b_hi = _mm_unpackhi_epi8(b_i8, b_sign);
    // dots_lo[i] = a[2i]*b[2i] + a[2i+1]*b[2i+1] as i32 (4 results from bytes 0-7)
    // dots_hi[i] = a[8+2i]*b[8+2i] + a[9+2i]*b[9+2i] as i32 (4 results from bytes 8-15)
    __m128i dots_lo = _mm_madd_epi16(a_lo, b_lo);
    __m128i dots_hi = _mm_madd_epi16(a_hi, b_hi);
    // Horizontal sum of adjacent pairs using float shuffle (SSE1, no SSSE3 required):
    // shuffle_ps(a, b, {0,2,0,2}) = [a[0],a[2],b[0],b[2]] (even lanes)
    // shuffle_ps(a, b, {1,3,1,3}) = [a[1],a[3],b[1],b[3]] (odd lanes)
    // sum = [a[0]+a[1], a[2]+a[3], b[0]+b[1], b[2]+b[3]]
    __m128i evens = _mm_castps_si128(_mm_shuffle_ps(
        _mm_castsi128_ps(dots_lo), _mm_castsi128_ps(dots_hi), _MM_SHUFFLE(2,0,2,0)));
    __m128i odds  = _mm_castps_si128(_mm_shuffle_ps(
        _mm_castsi128_ps(dots_lo), _mm_castsi128_ps(dots_hi), _MM_SHUFFLE(3,1,3,1)));
    return _mm_add_epi32(_mm_add_epi32(evens, odds), c);
}

WAH_IF_SSE41(
    static WAH_ALWAYS_INLINE __m128i wah_i16x8_extend_high_i8x16_s_sse41(__m128i a) {
        // Shuffle bytes 8-15 to low positions: [8,9,10,11,12,13,14,15, 0x80,...]
        return wah_mm_cvtepi8_epi16(wah_mm_shuffle_epi8(a, _mm_set_epi8(0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80, 15,14,13,12,11,10,9,8)));
    }
    static WAH_ALWAYS_INLINE __m128i wah_i16x8_extend_high_i8x16_u_sse41(__m128i a) {
        return wah_mm_cvtepu8_epi16(wah_mm_shuffle_epi8(a, _mm_set_epi8(0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80, 15,14,13,12,11,10,9,8)));
    }
    static WAH_ALWAYS_INLINE __m128i wah_i32x4_extend_high_i16x8_s_sse41(__m128i a) {
        // Shuffle words 4-7 (bytes 8-15) to low positions
        return wah_mm_cvtepi16_epi32(wah_mm_shuffle_epi8(a, _mm_set_epi8(0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80, 15,14,13,12,11,10,9,8)));
    }
    static WAH_ALWAYS_INLINE __m128i wah_i32x4_extend_high_i16x8_u_sse41(__m128i a) {
        return wah_mm_cvtepu16_epi32(wah_mm_shuffle_epi8(a, _mm_set_epi8(0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80, 15,14,13,12,11,10,9,8)));
    }
    static WAH_ALWAYS_INLINE __m128i wah_i64x2_extend_high_i32x4_s_sse41(__m128i a) {
        // Shuffle dwords 2-3 (bytes 8-15) to low positions
        return wah_mm_cvtepi32_epi64(wah_mm_shuffle_epi8(a, _mm_set_epi8(0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80, 15,14,13,12,11,10,9,8)));
    }
    static WAH_ALWAYS_INLINE __m128i wah_i64x2_extend_high_i32x4_u_sse41(__m128i a) {
        return wah_mm_cvtepu32_epi64(wah_mm_shuffle_epi8(a, _mm_set_epi8(0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80, 15,14,13,12,11,10,9,8)));
    }

    static WAH_ALWAYS_INLINE __m128i wah_i16x8_extmul_low_i8x16_s_sse41(__m128i a, __m128i b) {
        // Extract and sign-extend low 8 bytes of each input
        __m128i zero = _mm_setzero_si128();
        __m128i a_low = _mm_unpacklo_epi8(a, zero);
        __m128i b_low = _mm_unpacklo_epi8(b, zero);

        // Sign-extend from 8-bit to 16-bit
        __m128i a_sign_ext = wah_mm_cvtepi8_epi16(wah_mm_shuffle_epi8(a_low,
            _mm_set_epi8(7,6,5,4,3,2,1,0, 0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80)));
        __m128i b_sign_ext = wah_mm_cvtepi8_epi16(wah_mm_shuffle_epi8(b_low,
            _mm_set_epi8(7,6,5,4,3,2,1,0, 0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80)));

        // Multiply sign-extended values
        return _mm_mullo_epi16(a_sign_ext, b_sign_ext);
    }
    static WAH_ALWAYS_INLINE __m128i wah_i16x8_extmul_low_i8x16_u_sse41(__m128i a, __m128i b) {
        // Extract and zero-extend low 8 bytes of each input
        __m128i a_low = wah_mm_cvtepu8_epi16(wah_mm_shuffle_epi8(a,
            _mm_set_epi8(7,6,5,4,3,2,1,0, 0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80)));
        __m128i b_low = wah_mm_cvtepu8_epi16(wah_mm_shuffle_epi8(b,
            _mm_set_epi8(7,6,5,4,3,2,1,0, 0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80)));
        return _mm_mullo_epi16(a_low, b_low);
    }
    static WAH_ALWAYS_INLINE __m128i wah_i16x8_extmul_high_i8x16_s_sse41(__m128i a, __m128i b) {
        __m128i a_high = wah_mm_cvtepi8_epi16(wah_mm_shuffle_epi8(a,
            _mm_set_epi8(15,14,13,12,11,10,9,8, 0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80)));
        __m128i b_high = wah_mm_cvtepi8_epi16(wah_mm_shuffle_epi8(b,
            _mm_set_epi8(15,14,13,12,11,10,9,8, 0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80)));
        return _mm_mullo_epi16(a_high, b_high);
    }
    static WAH_ALWAYS_INLINE __m128i wah_i16x8_extmul_high_i8x16_u_sse41(__m128i a, __m128i b) {
        __m128i a_high = wah_mm_cvtepu8_epi16(wah_mm_shuffle_epi8(a,
            _mm_set_epi8(15,14,13,12,11,10,9,8, 0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80)));
        __m128i b_high = wah_mm_cvtepu8_epi16(wah_mm_shuffle_epi8(b,
            _mm_set_epi8(15,14,13,12,11,10,9,8, 0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80)));
        return _mm_mullo_epi16(a_high, b_high);
    }

    static WAH_ALWAYS_INLINE __m128i wah_i32x4_extmul_low_i16x8_s_sse41(__m128i a, __m128i b) {
        // Use SSE2 mullo_epi16 for each pair
        __m128i prod = _mm_mullo_epi16(a, b);
        // Sign extend to 32-bit
        __m128i prod_low = wah_mm_cvtepi16_epi32(prod);
        __m128i prod_high = wah_mm_cvtepi16_epi32(_mm_srli_si128(prod, 8));
        // Interleave results using 64-bit unpack to maintain correct order
        return _mm_unpacklo_epi64(prod_low, prod_high);
    }
    static WAH_ALWAYS_INLINE __m128i wah_i32x4_extmul_low_i16x8_u_sse41(__m128i a, __m128i b) {
        __m128i prod = _mm_mullo_epi16(a, b);
        __m128i prod_low = wah_mm_cvtepu16_epi32(prod);
        __m128i prod_high = wah_mm_cvtepu16_epi32(_mm_srli_si128(prod, 8));
        return _mm_unpacklo_epi64(prod_low, prod_high);
    }
    static WAH_ALWAYS_INLINE __m128i wah_i32x4_extmul_high_i16x8_s_sse41(__m128i a, __m128i b) {
        __m128i a_high = _mm_srli_si128(a, 8);
        __m128i b_high = _mm_srli_si128(b, 8);
        __m128i prod = _mm_mullo_epi16(a_high, b_high);
        __m128i prod_low = wah_mm_cvtepi16_epi32(prod);
        __m128i prod_high = wah_mm_cvtepi16_epi32(_mm_srli_si128(prod, 8));
        return _mm_unpacklo_epi64(prod_low, prod_high);
    }
    static WAH_ALWAYS_INLINE __m128i wah_i32x4_extmul_high_i16x8_u_sse41(__m128i a, __m128i b) {
        __m128i a_high = _mm_srli_si128(a, 8);
        __m128i b_high = _mm_srli_si128(b, 8);
        __m128i prod = _mm_mullo_epi16(a_high, b_high);
        __m128i prod_low = wah_mm_cvtepu16_epi32(prod);
        __m128i prod_high = wah_mm_cvtepu16_epi32(_mm_srli_si128(prod, 8));
        return _mm_unpacklo_epi64(prod_low, prod_high);
    }

    static WAH_ALWAYS_INLINE __m128i wah_i64x2_extmul_low_i32x4_s_sse41(__m128i a, __m128i b) {
        // Use SSE4.1's mul_epi32 (32x32->64) with sign extension
        __m128i prod0 = wah_mm_mul_epi32(a, b);
        __m128i prod1 = wah_mm_mul_epi32(_mm_srli_si128(a, 8), _mm_srli_si128(b, 8));
        return _mm_unpacklo_epi64(prod0, prod1);
    }
    static WAH_ALWAYS_INLINE __m128i wah_i64x2_extmul_low_i32x4_u_sse41(__m128i a, __m128i b) {
        // _mm_mul_epu32 only uses the low 32 bits of each lane, no need to zero-extend
        __m128i prod0 = _mm_mul_epu32(a, b);
        __m128i prod1 = _mm_mul_epu32(_mm_srli_si128(a, 8), _mm_srli_si128(b, 8));
        return _mm_unpacklo_epi64(prod0, prod1);
    }
    static WAH_ALWAYS_INLINE __m128i wah_i64x2_extmul_high_i32x4_s_sse41(__m128i a, __m128i b) {
        __m128i a_high = _mm_srli_si128(a, 8);
        __m128i b_high = _mm_srli_si128(b, 8);
        __m128i prod0 = wah_mm_mul_epi32(a_high, b_high);
        __m128i prod1 = wah_mm_mul_epi32(_mm_srli_si128(a_high, 8), _mm_srli_si128(b_high, 8));
        return _mm_unpacklo_epi64(prod0, prod1);
    }
    static WAH_ALWAYS_INLINE __m128i wah_i64x2_extmul_high_i32x4_u_sse41(__m128i a, __m128i b) {
        __m128i a_high = _mm_srli_si128(a, 8);
        __m128i b_high = _mm_srli_si128(b, 8);
        __m128i prod0 = _mm_mul_epu32(a_high, b_high);
        __m128i prod1 = _mm_mul_epu32(_mm_srli_si128(a_high, 8), _mm_srli_si128(b_high, 8));
        return _mm_unpacklo_epi64(prod0, prod1);
    }

    static WAH_ALWAYS_INLINE __m128i wah_i32x4_trunc_sat_f32x4_s_sse41(__m128 a) {
        // First truncate (no saturation)
        __m128i truncated = _mm_cvttps_epi32(a);

        // Check for NaN and create a mask
        __m128 cmp_nan = _mm_cmpunord_ps(a, a);

        // Check for overflow - use >= and <= to handle exact boundary values
        __m128 cmp_gt = _mm_cmpge_ps(a, _mm_set1_ps((float)INT32_MAX));
        __m128 cmp_lt = _mm_cmple_ps(a, _mm_set1_ps((float)INT32_MIN));

        // Select: if NaN -> 0, if >= max -> INT32_MAX, if <= min -> INT32_MIN, else truncated
        __m128i result = truncated;
        result = wah_mm_blendv_epi8(result, _mm_setzero_si128(), _mm_castps_si128(cmp_nan));
        result = wah_mm_blendv_epi8(result, _mm_set1_epi32(INT32_MAX), _mm_castps_si128(cmp_gt));
        result = wah_mm_blendv_epi8(result, _mm_set1_epi32(INT32_MIN), _mm_castps_si128(cmp_lt));
        return result;
    }

    static WAH_ALWAYS_INLINE __m128i wah_i32x4_trunc_sat_f32x4_u_sse41(__m128 a) {
        // Check for NaN
        __m128 cmp_nan = _mm_cmpunord_ps(a, a);

        // Check for negative
        __m128 cmp_lt = _mm_cmplt_ps(a, _mm_setzero_ps());

        // Check for overflow (>= UINT32_MAX)
        __m128 cmp_gt = _mm_cmpge_ps(a, _mm_set1_ps((float)UINT32_MAX));

        // Truncate
        __m128i truncated = _mm_cvttps_epi32(a);

        // Select: if NaN or < 0 -> 0, if > max -> UINT32_MAX, else truncated
        __m128i result = truncated;
        __m128i invalid = _mm_or_si128(_mm_castps_si128(cmp_nan), _mm_castps_si128(cmp_lt));
        result = wah_mm_blendv_epi8(result, _mm_setzero_si128(), invalid);
        result = wah_mm_blendv_epi8(result, _mm_set1_epi32(UINT32_MAX), _mm_castps_si128(cmp_gt));
        return result;
    }
)

WAH_IF_SSE42(
    // SSE4.2 64-bit comparison wrappers (pcmpgtq)
    static WAH_ALWAYS_INLINE __m128i wah_i64x2_lt_s_sse42(__m128i a, __m128i b) {
        return wah_mm_cmpgt_epi64(b, a);
    }
    static WAH_ALWAYS_INLINE __m128i wah_i64x2_gt_s_sse42(__m128i a, __m128i b) {
        return wah_mm_cmpgt_epi64(a, b);
    }
    static WAH_ALWAYS_INLINE __m128i wah_i64x2_le_s_sse42(__m128i a, __m128i b) {
        return wah_v128_not_sse2(wah_mm_cmpgt_epi64(a, b)); // a <= b  is  NOT(a > b)
    }
    static WAH_ALWAYS_INLINE __m128i wah_i64x2_ge_s_sse42(__m128i a, __m128i b) {
        return wah_v128_not_sse2(wah_mm_cmpgt_epi64(b, a)); // a >= b  is  NOT(b > a)
    }
)

WAH_IF_AVX2(
    // AVX2 optimized popcount using SSSE3 pshufb lookup table
    static WAH_ALWAYS_INLINE __m128i wah_i8x16_popcnt_avx2(__m128i v) {
        // Split bytes into nibbles, look up popcount for each
        __m128i low_nibbles = _mm_and_si128(v, _mm_set1_epi8(0x0F));
        __m128i high_nibbles = _mm_and_si128(_mm_srli_epi16(v, 4), _mm_set1_epi8(0x0F));

        // Popcount lookup table (for nibbles 0-15)
        __m128i popcnt_table = _mm_set_epi8(4,3,3,2,3,2,2,1,3,2,2,1,2,1,1,0);

        // Look up popcount for each nibble
        __m128i low_popcnt = wah_mm_shuffle_epi8(popcnt_table, low_nibbles);
        __m128i high_popcnt = wah_mm_shuffle_epi8(popcnt_table, high_nibbles);

        // Add the two popcounts together
        return _mm_add_epi8(low_popcnt, high_popcnt);
    }
)

WAH_IF_AVX512F(
    // AVX-512F 64x64->64 multiply (partial products method, more efficient than SSE2)
    static WAH_ALWAYS_INLINE __m128i wah_i64x2_mul_avx512f(__m128i a, __m128i b) {
        __m128i a_lo = _mm_and_si128(a, _mm_set1_epi32(0xFFFFFFFF));
        __m128i b_lo = _mm_and_si128(b, _mm_set1_epi32(0xFFFFFFFF));

        __m128i a_hi = _mm_srli_epi64(a, 32);
        __m128i b_hi = _mm_srli_epi64(b, 32);

        __m128i p0 = _mm_mul_epu32(a_lo, b_lo);
        __m128i p1 = _mm_mul_epu32(a_lo, b_hi);
        __m128i p2 = _mm_mul_epu32(a_hi, b_lo);

        __m128i p1_p2 = _mm_add_epi64(p1, p2);
        p1_p2 = _mm_slli_epi64(p1_p2, 32);

        __m128i result = _mm_add_epi64(p0, p1_p2);
        return result;
    }
)

WAH_IF_AVX512VL(
    // AVX-512VL HIGH conversion helpers (shuffle high 64 bits then convert)
    static WAH_ALWAYS_INLINE __m128 wah_f32x4_convert_i32x4_u_high_avx512vl(__m128i a) {
        __m128i a_high = _mm_srli_si128(a, 8);
        return wah_mm_cvtepu32_ps(a_high);
    }

    static WAH_ALWAYS_INLINE __m128d wah_f64x2_convert_high_i32x4_u_avx512vl(__m128i a) {
        __m128i a_high = _mm_srli_si128(a, 8);
        return wah_mm_cvtepu32_pd(a_high);
    }
)

// AVX-512BW comparison helpers (vpcmpeqb/vpcmpb -> k -> vpmovm2b, no __mmask types needed)
#if defined(__GNUC__)
WAH_IF_AVX512BW(
    static WAH_ALWAYS_INLINE __m128i wah_mm_cmpeq_epi8_avx512(__m128i a, __m128i b) {
        __m128i result;
        __asm__("vpcmpeqb %[b], %[a], %%k1\n\t vpmovm2b %%k1, %[result]"
            : [result] "=v" (result) : [a] "v" (a), [b] "v" (b) : "k1");
        return result;
    }
    static WAH_ALWAYS_INLINE __m128i wah_mm_cmpgt_epi8_avx512(__m128i a, __m128i b) {
        __m128i result;
        __asm__("vpcmpb $6, %[b], %[a], %%k1\n\t vpmovm2b %%k1, %[result]"
            : [result] "=v" (result) : [a] "v" (a), [b] "v" (b) : "k1");
        return result;
    }
    static WAH_ALWAYS_INLINE __m128i wah_mm_cmplt_epu8_avx512(__m128i a, __m128i b) {
        __m128i result;
        __asm__("vpcmpub $1, %[b], %[a], %%k1\n\t vpmovm2b %%k1, %[result]"
            : [result] "=v" (result) : [a] "v" (a), [b] "v" (b) : "k1");
        return result;
    }
)
#else
WAH_IF_AVX512BW(
    static WAH_ALWAYS_INLINE __m128i wah_mm_cmpeq_epi8_avx512(__m128i a, __m128i b) {
        return _mm_mask_blend_epi8(_mm_cmpeq_epi8_mask(a, b), _mm_setzero_si128(), _mm_set1_epi8(-1));
    }
    static WAH_ALWAYS_INLINE __m128i wah_mm_cmpgt_epi8_avx512(__m128i a, __m128i b) {
        return _mm_mask_blend_epi8(_mm_cmpgt_epi8_mask(a, b), _mm_setzero_si128(), _mm_set1_epi8(-1));
    }
    static WAH_ALWAYS_INLINE __m128i wah_mm_cmplt_epu8_avx512(__m128i a, __m128i b) {
        return _mm_mask_blend_epi8(_mm_cmplt_epu8_mask(a, b), _mm_setzero_si128(), _mm_set1_epi8(-1));
    }
)
#endif

#endif // defined WAH_X86_64

#ifdef WAH_AARCH64 // NEON helper functions

// NEON lacks same-type reinterpret; provide identity macro for u8.
#define vreinterpretq_u8_u8(x) (x)

// Macro-generated typed wrappers for integer SIMD operations.
// NEON uses typed vectors, but _u8x16 stores everything as uint8x16_t.
// These wrappers reinterpret to the correct type, call the NEON intrinsic, and reinterpret back.

#define WAH_NEON_BIN(neon_fn, lane_t) \
    static WAH_ALWAYS_INLINE uint8x16_t wah_##neon_fn##_##lane_t(uint8x16_t a, uint8x16_t b) { \
        return vreinterpretq_u8_##lane_t( \
            neon_fn##_##lane_t(vreinterpretq_##lane_t##_u8(a), vreinterpretq_##lane_t##_u8(b))); \
    }
#define WAH_NEON_UNA(neon_fn, lane_t) \
    static WAH_ALWAYS_INLINE uint8x16_t wah_##neon_fn##_##lane_t(uint8x16_t a) { \
        return vreinterpretq_u8_##lane_t(neon_fn##_##lane_t(vreinterpretq_##lane_t##_u8(a))); \
    }

// --- Arithmetic ---
WAH_NEON_BIN(vaddq,  s8) WAH_NEON_BIN(vsubq,  s8) WAH_NEON_BIN(vqaddq,  s8) WAH_NEON_BIN(vqsubq,  s8)
WAH_NEON_BIN(vaddq,  u8) WAH_NEON_BIN(vsubq,  u8) WAH_NEON_BIN(vqaddq,  u8) WAH_NEON_BIN(vqsubq,  u8)
WAH_NEON_BIN(vaddq, s16) WAH_NEON_BIN(vsubq, s16) WAH_NEON_BIN(vqaddq, s16) WAH_NEON_BIN(vqsubq, s16) WAH_NEON_BIN(vmulq, s16)
WAH_NEON_BIN(vaddq, u16) WAH_NEON_BIN(vsubq, u16) WAH_NEON_BIN(vqaddq, u16) WAH_NEON_BIN(vqsubq, u16)
WAH_NEON_BIN(vaddq, s32) WAH_NEON_BIN(vsubq, s32)                                                     WAH_NEON_BIN(vmulq, s32)
WAH_NEON_BIN(vaddq, u32) WAH_NEON_BIN(vsubq, u32)
WAH_NEON_BIN(vaddq, s64) WAH_NEON_BIN(vsubq, s64)
WAH_NEON_BIN(vaddq, u64) WAH_NEON_BIN(vsubq, u64)

// --- Compare (returns all-ones/0 per lane) ---
WAH_NEON_BIN(vceqq,  s8) WAH_NEON_BIN(vcgtq,  s8) WAH_NEON_BIN(vcltq,  s8)
WAH_NEON_BIN(vceqq,  u8) WAH_NEON_BIN(vcgtq,  u8) WAH_NEON_BIN(vcltq,  u8)
WAH_NEON_BIN(vceqq, s16) WAH_NEON_BIN(vcgtq, s16) WAH_NEON_BIN(vcltq, s16)
WAH_NEON_BIN(vceqq, u16) WAH_NEON_BIN(vcgtq, u16) WAH_NEON_BIN(vcltq, u16)
WAH_NEON_BIN(vceqq, s32) WAH_NEON_BIN(vcgtq, s32) WAH_NEON_BIN(vcltq, s32)
WAH_NEON_BIN(vceqq, u32) WAH_NEON_BIN(vcgtq, u32) WAH_NEON_BIN(vcltq, u32)
WAH_NEON_BIN(vceqq, s64) WAH_NEON_BIN(vcgtq, s64) WAH_NEON_BIN(vcltq, s64)

// --- Min/max ---
WAH_NEON_BIN(vminq,  s8) WAH_NEON_BIN(vmaxq,  s8)
WAH_NEON_BIN(vminq,  u8) WAH_NEON_BIN(vmaxq,  u8)
WAH_NEON_BIN(vminq, s16) WAH_NEON_BIN(vmaxq, s16)
WAH_NEON_BIN(vminq, u16) WAH_NEON_BIN(vmaxq, u16)
WAH_NEON_BIN(vminq, s32) WAH_NEON_BIN(vmaxq, s32)
WAH_NEON_BIN(vminq, u32) WAH_NEON_BIN(vmaxq, u32)

// --- Abs/neg ---
WAH_NEON_UNA(vabsq,  s8) WAH_NEON_UNA(vnegq,  s8)
WAH_NEON_UNA(vabsq, s16) WAH_NEON_UNA(vnegq, s16)
WAH_NEON_UNA(vabsq, s32) WAH_NEON_UNA(vnegq, s32)
WAH_NEON_UNA(vabsq, s64) WAH_NEON_UNA(vnegq, s64)

// --- Float/double arithmetic ---
WAH_NEON_BIN(vaddq,  f32) WAH_NEON_BIN(vaddq,  f64)
WAH_NEON_BIN(vsubq,  f32) WAH_NEON_BIN(vsubq,  f64)
WAH_NEON_BIN(vmulq,  f32) WAH_NEON_BIN(vmulq,  f64)
WAH_NEON_BIN(vdivq,  f32) WAH_NEON_BIN(vdivq,  f64)
WAH_NEON_BIN(vceqq,  f32) WAH_NEON_BIN(vceqq,  f64)
WAH_NEON_BIN(vcltq,  f32) WAH_NEON_BIN(vcltq,  f64)
WAH_NEON_BIN(vcgtq,  f32) WAH_NEON_BIN(vcgtq,  f64)
WAH_NEON_BIN(vminq,  f32) WAH_NEON_BIN(vminq,  f64)
WAH_NEON_BIN(vmaxq,  f32) WAH_NEON_BIN(vmaxq,  f64)
WAH_NEON_UNA(vabsq,  f32) WAH_NEON_UNA(vabsq,  f64)
WAH_NEON_UNA(vnegq,  f32) WAH_NEON_UNA(vnegq,  f64)
WAH_NEON_UNA(vsqrtq, f32) WAH_NEON_UNA(vsqrtq, f64)
WAH_NEON_UNA(vrndpq, f32) WAH_NEON_UNA(vrndpq, f64)
WAH_NEON_UNA(vrndmq, f32) WAH_NEON_UNA(vrndmq, f64)
WAH_NEON_UNA(vrndq,  f32) WAH_NEON_UNA(vrndq,  f64)
WAH_NEON_UNA(vrndnq, f32) WAH_NEON_UNA(vrndnq, f64)

#undef WAH_NEON_BIN
#undef WAH_NEON_UNA

// --- Special-case helpers ---

// Bitmask: extract sign bits into packed integer
static WAH_ALWAYS_INLINE int32_t wah_bitmask_i8x16_neon(uint8x16_t a) {
    static const uint8_t lut[16] = {1,2,4,8,16,32,64,128, 1,2,4,8,16,32,64,128};
    uint8x16_t bits = vshrq_n_u8(a, 7);
    uint8x16_t weights = vld1q_u8(lut);
    uint8x16_t weighted = vandq_u8(bits, weights);
    uint8x16_t lo = vpaddlq_u16(vpaddlq_u8(weighted));
    return (int32_t)vgetq_lane_u32(lo, 0) | ((int32_t)vgetq_lane_u32(lo, 1) << 8);
}
static WAH_ALWAYS_INLINE int32_t wah_bitmask_i16x8_neon(uint8x16_t a) {
    int16x8_t v = vreinterpretq_s16_u8(a);
    uint16x8_t bits = vcltzq_s16(v);
    static const uint16_t lut[8] = {1,2,4,8,16,32,64,128};
    uint16x8_t weights = vld1q_u16(lut);
    uint16x8_t weighted = vandq_u16(bits, weights);
    uint32x4_t pairs = vpaddlq_u16(weighted);
    uint64x2_t quads = vpaddlq_u32(pairs);
    return (int32_t)vgetq_lane_u64(quads, 0);
}
static WAH_ALWAYS_INLINE int32_t wah_bitmask_i32x4_neon(uint8x16_t a) {
    int32x4_t v = vreinterpretq_s32_u8(a);
    uint32x4_t bits = vcltzq_s32(v);
    static const uint32_t lut[4] = {1,2,4,8};
    uint32x4_t weights = vld1q_u32(lut);
    uint32x4_t weighted = vandq_u32(bits, weights);
    uint64x2_t pairs = vpaddlq_u32(weighted);
    return (int32_t)vgetq_lane_u64(pairs, 0);
}
static WAH_ALWAYS_INLINE int32_t wah_bitmask_i64x2_neon(uint8x16_t a) {
    int64x2_t v = vreinterpretq_s64_u8(a);
    uint64x2_t bits = vcltzq_s64(v);
    uint64x2_t weighted = vandq_u64(bits, vreinterpretq_u64_u32(vdupq_n_u32(2)));
    uint64x2_t lo = vextq_u64(weighted, vdupq_n_u64(0), 1);
    return (int32_t)(vgetq_lane_u64(weighted, 0) | (vgetq_lane_u64(lo, 0) << 1));
}

// All-true: check all lanes nonzero
static WAH_ALWAYS_INLINE int32_t wah_all_true_i8x16_neon(uint8x16_t a) {
    return (int32_t)(vminvq_u8(a) != 0);
}
static WAH_ALWAYS_INLINE int32_t wah_all_true_i16x8_neon(uint8x16_t a) {
    return (int32_t)(vminvq_u16(vreinterpretq_u16_u8(a)) != 0);
}
static WAH_ALWAYS_INLINE int32_t wah_all_true_i32x4_neon(uint8x16_t a) {
    return (int32_t)(vminvq_u32(vreinterpretq_u32_u8(a)) != 0);
}
static WAH_ALWAYS_INLINE int32_t wah_all_true_i64x2_neon(uint8x16_t a) {
    uint64x2_t v = vreinterpretq_u64_u8(a);
    return (int32_t)(vgetq_lane_u64(v, 0) != 0 && vgetq_lane_u64(v, 1) != 0);
}

// Any-true
static WAH_ALWAYS_INLINE int32_t wah_any_true_neon(uint8x16_t a) {
    return (int32_t)(vmaxvq_u8(a) != 0);
}

// ANDNOT (a & ~b)
static WAH_ALWAYS_INLINE uint8x16_t wah_v128_andnot_neon(uint8x16_t a, uint8x16_t b) {
    return vandq_u8(a, vmvnq_u8(b));
}

// i8x16 shifts (NEON handles byte shifts natively)
static WAH_ALWAYS_INLINE uint8x16_t wah_i8x16_shl_neon(uint8x16_t a, int32_t count) {
    return vreinterpretq_u8_s8(vshlq_s8(vreinterpretq_s8_u8(a), vdupq_n_s8((int8_t)(count & 7))));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_i8x16_shr_s_neon(uint8x16_t a, int32_t count) {
    return vreinterpretq_u8_s8(vshlq_s8(vreinterpretq_s8_u8(a), vdupq_n_s8((int8_t)(-(count & 7)))));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_i8x16_shr_u_neon(uint8x16_t a, int32_t count) {
    return vshlq_u8(a, vdupq_n_s8((int8_t)(-(count & 7))));
}

// i16x8 shifts
static WAH_ALWAYS_INLINE uint8x16_t wah_i16x8_shl_neon(uint8x16_t a, int32_t count) {
    return vreinterpretq_u8_s16(vshlq_s16(vreinterpretq_s16_u8(a), vdupq_n_s16((int16_t)(count & 15))));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_i16x8_shr_s_neon(uint8x16_t a, int32_t count) {
    return vreinterpretq_u8_s16(vshlq_s16(vreinterpretq_s16_u8(a), vdupq_n_s16((int16_t)(-(count & 15)))));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_i16x8_shr_u_neon(uint8x16_t a, int32_t count) {
    return vreinterpretq_u8_u16(vshlq_u16(vreinterpretq_u16_u8(a), vdupq_n_s16((int16_t)(-(count & 15)))));
}

// i32x4 shifts
static WAH_ALWAYS_INLINE uint8x16_t wah_i32x4_shl_neon(uint8x16_t a, int32_t count) {
    return vreinterpretq_u8_s32(vshlq_s32(vreinterpretq_s32_u8(a), vdupq_n_s32(count & 31)));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_i32x4_shr_s_neon(uint8x16_t a, int32_t count) {
    return vreinterpretq_u8_s32(vshlq_s32(vreinterpretq_s32_u8(a), vdupq_n_s32(-(count & 31))));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_i32x4_shr_u_neon(uint8x16_t a, int32_t count) {
    return vreinterpretq_u8_u32(vshlq_u32(vreinterpretq_u32_u8(a), vdupq_n_s32(-(count & 31))));
}

// i64x2 shifts
static WAH_ALWAYS_INLINE uint8x16_t wah_i64x2_shl_neon(uint8x16_t a, int32_t count) {
    return vreinterpretq_u8_s64(vshlq_s64(vreinterpretq_s64_u8(a), vdupq_n_s64(count & 63)));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_i64x2_shr_u_neon(uint8x16_t a, int32_t count) {
    return vreinterpretq_u8_u64(vshlq_u64(vreinterpretq_u64_u8(a), vdupq_n_s64(-(count & 63))));
}

// Narrow operations
static WAH_ALWAYS_INLINE uint8x16_t wah_i8x16_narrow_i16x8_s_neon(uint8x16_t a, uint8x16_t b) {
    int16x8_t va = vreinterpretq_s16_u8(a), vb = vreinterpretq_s16_u8(b);
    return vreinterpretq_u8_s8(vcombine_s8(vmovn_s16(va), vmovn_s16(vb)));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_i8x16_narrow_i16x8_u_neon(uint8x16_t a, uint8x16_t b) {
    uint16x8_t va = vreinterpretq_u16_u8(a), vb = vreinterpretq_u16_u8(b);
    return vcombine_u8(vqmovn_u16(va), vqmovn_u16(vb));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_i16x8_narrow_i32x4_s_neon(uint8x16_t a, uint8x16_t b) {
    int32x4_t va = vreinterpretq_s32_u8(a), vb = vreinterpretq_s32_u8(b);
    return vreinterpretq_u8_s16(vcombine_s16(vmovn_s32(va), vmovn_s32(vb)));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_i16x8_narrow_i32x4_u_neon(uint8x16_t a, uint8x16_t b) {
    int32x4_t va = vreinterpretq_s32_u8(a), vb = vreinterpretq_s32_u8(b);
    // Clamp signed i32 to [0, UINT16_MAX]
    va = vmaxq_s32(va, vdupq_n_s32(0));
    va = vminq_s32(va, vdupq_n_s32(0xFFFF));
    vb = vmaxq_s32(vb, vdupq_n_s32(0));
    vb = vminq_s32(vb, vdupq_n_s32(0xFFFF));
    return vreinterpretq_u8_u16(vcombine_u16(vreinterpret_u16_s32(vmovn_s32(va)), vreinterpret_u16_s32(vmovn_s32(vb))));
}

// Extend operations
static WAH_ALWAYS_INLINE uint8x16_t wah_i16x8_extend_low_i8x16_s_neon(uint8x16_t a) {
    return vreinterpretq_u8_s16(vmovl_s8(vget_low_s8(vreinterpretq_s8_u8(a))));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_i16x8_extend_high_i8x16_s_neon(uint8x16_t a) {
    return vreinterpretq_u8_s16(vmovl_s8(vget_high_s8(vreinterpretq_s8_u8(a))));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_i16x8_extend_low_i8x16_u_neon(uint8x16_t a) {
    return vreinterpretq_u8_u16(vmovl_u8(vget_low_u8(a)));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_i16x8_extend_high_i8x16_u_neon(uint8x16_t a) {
    return vreinterpretq_u8_u16(vmovl_u8(vget_high_u8(a)));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_i32x4_extend_low_i16x8_s_neon(uint8x16_t a) {
    return vreinterpretq_u8_s32(vmovl_s16(vget_low_s16(vreinterpretq_s16_u8(a))));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_i32x4_extend_high_i16x8_s_neon(uint8x16_t a) {
    return vreinterpretq_u8_s32(vmovl_s16(vget_high_s16(vreinterpretq_s16_u8(a))));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_i32x4_extend_low_i16x8_u_neon(uint8x16_t a) {
    return vreinterpretq_u8_u32(vmovl_u16(vget_low_u16(vreinterpretq_u16_u8(a))));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_i32x4_extend_high_i16x8_u_neon(uint8x16_t a) {
    return vreinterpretq_u8_u32(vmovl_u16(vget_high_u16(vreinterpretq_u16_u8(a))));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_i64x2_extend_low_i32x4_s_neon(uint8x16_t a) {
    return vreinterpretq_u8_s64(vmovl_s32(vget_low_s32(vreinterpretq_s32_u8(a))));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_i64x2_extend_high_i32x4_s_neon(uint8x16_t a) {
    return vreinterpretq_u8_s64(vmovl_s32(vget_high_s32(vreinterpretq_s32_u8(a))));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_i64x2_extend_low_i32x4_u_neon(uint8x16_t a) {
    return vreinterpretq_u8_u64(vmovl_u32(vget_low_u32(vreinterpretq_u32_u8(a))));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_i64x2_extend_high_i32x4_u_neon(uint8x16_t a) {
    return vreinterpretq_u8_u64(vmovl_u32(vget_high_u32(vreinterpretq_u32_u8(a))));
}

// Extmul operations
static WAH_ALWAYS_INLINE uint8x16_t wah_extmul_low_s8_neon(uint8x16_t a, uint8x16_t b) {
    int16x8_t ea = vmovl_s8(vget_low_s8(vreinterpretq_s8_u8(a)));
    int16x8_t eb = vmovl_s8(vget_low_s8(vreinterpretq_s8_u8(b)));
    return vreinterpretq_u8_s16(vmulq_s16(ea, eb));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_extmul_high_s8_neon(uint8x16_t a, uint8x16_t b) {
    int16x8_t ea = vmovl_s8(vget_high_s8(vreinterpretq_s8_u8(a)));
    int16x8_t eb = vmovl_s8(vget_high_s8(vreinterpretq_s8_u8(b)));
    return vreinterpretq_u8_s16(vmulq_s16(ea, eb));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_extmul_low_u8_neon(uint8x16_t a, uint8x16_t b) {
    uint16x8_t ea = vmovl_u8(vget_low_u8(a));
    uint16x8_t eb = vmovl_u8(vget_low_u8(b));
    return vreinterpretq_u8_u16(vmulq_u16(ea, eb));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_extmul_high_u8_neon(uint8x16_t a, uint8x16_t b) {
    uint16x8_t ea = vmovl_u8(vget_high_u8(a));
    uint16x8_t eb = vmovl_u8(vget_high_u8(b));
    return vreinterpretq_u8_u16(vmulq_u16(ea, eb));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_extmul_low_s16_neon(uint8x16_t a, uint8x16_t b) {
    int32x4_t ea = vmovl_s16(vget_low_s16(vreinterpretq_s16_u8(a)));
    int32x4_t eb = vmovl_s16(vget_low_s16(vreinterpretq_s16_u8(b)));
    return vreinterpretq_u8_s32(vmulq_s32(ea, eb));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_extmul_high_s16_neon(uint8x16_t a, uint8x16_t b) {
    int32x4_t ea = vmovl_s16(vget_high_s16(vreinterpretq_s16_u8(a)));
    int32x4_t eb = vmovl_s16(vget_high_s16(vreinterpretq_s16_u8(b)));
    return vreinterpretq_u8_s32(vmulq_s32(ea, eb));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_extmul_low_u16_neon(uint8x16_t a, uint8x16_t b) {
    uint32x4_t ea = vmovl_u16(vget_low_u16(vreinterpretq_u16_u8(a)));
    uint32x4_t eb = vmovl_u16(vget_low_u16(vreinterpretq_u16_u8(b)));
    return vreinterpretq_u8_u32(vmulq_u32(ea, eb));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_extmul_high_u16_neon(uint8x16_t a, uint8x16_t b) {
    uint32x4_t ea = vmovl_u16(vget_high_u16(vreinterpretq_u16_u8(a)));
    uint32x4_t eb = vmovl_u16(vget_high_u16(vreinterpretq_u16_u8(b)));
    return vreinterpretq_u8_u32(vmulq_u32(ea, eb));
}
// Extadd pairwise
static WAH_ALWAYS_INLINE uint8x16_t wah_extadd_pairwise_s8_neon(uint8x16_t a) {
    return vreinterpretq_u8_s16(vpaddlq_s8(vreinterpretq_s8_u8(a)));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_extadd_pairwise_u8_neon(uint8x16_t a) {
    return vreinterpretq_u8_u16(vpaddlq_u8(a));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_extadd_pairwise_s16_neon(uint8x16_t a) {
    return vreinterpretq_u8_s32(vpaddlq_s16(vreinterpretq_s16_u8(a)));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_extadd_pairwise_u16_neon(uint8x16_t a) {
    return vreinterpretq_u8_u32(vpaddlq_u16(vreinterpretq_u16_u8(a)));
}

// Average unsigned
static WAH_ALWAYS_INLINE uint8x16_t wah_avgr_u16_neon(uint8x16_t a, uint8x16_t b) {
    return vreinterpretq_u8_u16(vrhaddq_u16(vreinterpretq_u16_u8(a), vreinterpretq_u16_u8(b)));
}

// i32x4_trunc_sat_f32x4_u (no direct NEON intrinsic)
static WAH_ALWAYS_INLINE uint8x16_t wah_i32x4_trunc_sat_f32x4_u_neon(uint8x16_t v) {
    float32x4_t fv = vreinterpretq_f32_u8(v);
    float32x4_t clamped = vbslq_f32(vcgeq_f32(fv, vdupq_n_f32(0)), fv, vdupq_n_f32(0));
    uint32x4_t overflow = vcgtq_f32(clamped, vdupq_n_f32(4294967296.0f));
    int32x4_t converted = vcvtq_s32_f32(clamped);
    uint32x4_t result = vreinterpretq_u32_s32(converted);
    result = vbslq_u32(overflow, vdupq_n_u32(0xFFFFFFFF), result);
    return vreinterpretq_u8_u32(result);
}

// pmin/pmax for float (NaN canonicalization)
static WAH_ALWAYS_INLINE uint8x16_t wah_f32x4_pmin_neon(uint8x16_t a, uint8x16_t b) {
    float32x4_t fa = vreinterpretq_f32_u8(a), fb = vreinterpretq_f32_u8(b);
    float32x4_t ca = vreinterpretq_f32_u8(wah_canonicalize_f32x4_neon(a));
    float32x4_t cb = vreinterpretq_f32_u8(wah_canonicalize_f32x4_neon(b));
    return vreinterpretq_u8_f32(vbslq_f32(vcltq_f32(cb, ca), fb, fa));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_f32x4_pmax_neon(uint8x16_t a, uint8x16_t b) {
    float32x4_t fa = vreinterpretq_f32_u8(a), fb = vreinterpretq_f32_u8(b);
    float32x4_t ca = vreinterpretq_f32_u8(wah_canonicalize_f32x4_neon(a));
    float32x4_t cb = vreinterpretq_f32_u8(wah_canonicalize_f32x4_neon(b));
    return vreinterpretq_u8_f32(vbslq_f32(vcltq_f32(ca, cb), fb, fa));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_f64x2_pmin_neon(uint8x16_t a, uint8x16_t b) {
    float64x2_t fa = vreinterpretq_f64_u8(a), fb = vreinterpretq_f64_u8(b);
    float64x2_t ca = vreinterpretq_f64_u8(wah_canonicalize_f64x2_neon(a));
    float64x2_t cb = vreinterpretq_f64_u8(wah_canonicalize_f64x2_neon(b));
    return vreinterpretq_u8_f64(vbslq_f64(vcltq_f64(cb, ca), fb, fa));
}
static WAH_ALWAYS_INLINE uint8x16_t wah_f64x2_pmax_neon(uint8x16_t a, uint8x16_t b) {
    float64x2_t fa = vreinterpretq_f64_u8(a), fb = vreinterpretq_f64_u8(b);
    float64x2_t ca = vreinterpretq_f64_u8(wah_canonicalize_f64x2_neon(a));
    float64x2_t cb = vreinterpretq_f64_u8(wah_canonicalize_f64x2_neon(b));
    return vreinterpretq_u8_f64(vbslq_f64(vcltq_f64(ca, cb), fb, fa));
}

// Float convert i32x4 signed
static WAH_ALWAYS_INLINE uint8x16_t wah_f32x4_convert_i32x4_s_neon(uint8x16_t a) {
    return vreinterpretq_u8_f32(vcvtq_f32_s32(vreinterpretq_s32_u8(a)));
}

// Float convert i32x4 unsigned
static WAH_ALWAYS_INLINE uint8x16_t wah_f32x4_convert_i32x4_u_neon(uint8x16_t a) {
    return vreinterpretq_u8_f32(vcvtq_f32_u32(vreinterpretq_u32_u8(a)));
}

// I32X4_TRUNC_SAT_F32X4_S
static WAH_ALWAYS_INLINE uint8x16_t wah_i32x4_trunc_sat_f32x4_s_neon(uint8x16_t v) {
    return vreinterpretq_u8_s32(vcvtq_s32_f32(vreinterpretq_f32_u8(v)));
}

#endif // defined WAH_AARCH64

static inline wah_error_t wah_type_stack_push(wah_type_stack_t *stack, wah_type_t type) {
    WAH_ENSURE(stack->sp < WAH_MAX_TYPE_STACK_SIZE, WAH_ERROR_TOO_LARGE);
    stack->data[stack->sp++] = type;
    return WAH_OK;
}

static inline wah_error_t wah_type_stack_pop(wah_type_stack_t *stack, wah_type_t *out_type) {
    WAH_ENSURE(stack->sp > 0, WAH_ERROR_VALIDATION_FAILED);
    *out_type = stack->data[--stack->sp];
    return WAH_OK;
}

// Stack depth tracking helpers
static inline wah_error_t wah_validation_push_type(wah_validation_context_t *vctx, wah_type_t type) {
    WAH_CHECK(wah_type_stack_push(&vctx->type_stack, vctx->is_unreachable ? WAH_TYPE_ANY : type));
    vctx->current_stack_depth++;
    if (vctx->current_stack_depth > vctx->max_stack_depth) {
        vctx->max_stack_depth = vctx->current_stack_depth;
    }
    return WAH_OK;
}

static inline wah_error_t wah_validation_pop_type(wah_validation_context_t *vctx, wah_type_t *out_type) {
    if (vctx->is_unreachable) {
        *out_type = WAH_TYPE_ANY;
        // In an unreachable state, pop always succeeds conceptually, and stack height still changes.
        // We still decrement current_stack_depth to track the conceptual stack height.
        // We don't need to pop from type_stack.data as it's already filled with WAH_TYPE_ANY or ignored.
        if (vctx->current_stack_depth > 0) {
            vctx->current_stack_depth--;
        }
        return WAH_OK;
    }

    // If reachable, check for stack underflow
    WAH_ENSURE(vctx->current_stack_depth > 0, WAH_ERROR_VALIDATION_FAILED);
    WAH_CHECK(wah_type_stack_pop(&vctx->type_stack, out_type));
    vctx->current_stack_depth--;
    return WAH_OK;
}

// Helper function to validate if an actual type matches an expected type, considering WAH_TYPE_ANY
static inline wah_error_t wah_validate_type_match(wah_type_t actual, wah_type_t expected) {
    WAH_ENSURE(actual == expected || actual == WAH_TYPE_ANY, WAH_ERROR_VALIDATION_FAILED);
    return WAH_OK;
}

// Helper function to pop a type from the stack and validate it against an expected type
static inline wah_error_t wah_validation_pop_and_match_type(wah_validation_context_t *vctx, wah_type_t expected_type) {
    wah_type_t actual_type;
    WAH_CHECK(wah_validation_pop_type(vctx, &actual_type));
    return wah_validate_type_match(actual_type, expected_type);
}

// Helper to read a section header
static wah_error_t wah_read_section_header(const uint8_t **ptr, const uint8_t *end, uint8_t *id, uint32_t *size) {
    WAH_ENSURE(*ptr < end, WAH_ERROR_UNEXPECTED_EOF);
    *id = *(*ptr)++;
    return wah_decode_uleb128(ptr, end, size);
}

// --- Internal Section Parsing Functions ---
static wah_error_t wah_parse_type_section(const uint8_t **ptr, const uint8_t *section_end, wah_module_t *module) {
    uint32_t count;
    // A function type requires at least 3 bytes (form, param_count_uleb128, result_count_uleb128).
    WAH_CHECK(wah_decode_and_validate_count(ptr, section_end, &count, 3));

    module->type_count = 0; // Doubles as how many entries have been initialized (for cleanup)
    WAH_MALLOC_ARRAY(module->types, count);

    for (uint32_t i = 0; i < count; ++i) {
        // Initialize pointer fields for safe cleanup on parsing failure
        module->types[i].param_types = NULL;
        module->types[i].result_types = NULL;
        ++module->type_count;

        WAH_ENSURE(**ptr == 0x60, WAH_ERROR_VALIDATION_FAILED);
        (*ptr)++;

        // Parse parameter types
        uint32_t param_count_type;
        WAH_CHECK(wah_decode_uleb128(ptr, section_end, &param_count_type));

        module->types[i].param_count = param_count_type;
        WAH_MALLOC_ARRAY(module->types[i].param_types, param_count_type);
        for (uint32_t j = 0; j < param_count_type; ++j) {
            wah_type_t type;
            WAH_CHECK(wah_decode_val_type(ptr, section_end, &type));
            module->types[i].param_types[j] = type;
        }

        // Parse result types
        uint32_t result_count_type;
        WAH_CHECK(wah_decode_uleb128(ptr, section_end, &result_count_type));

        module->types[i].result_count = result_count_type;
        WAH_MALLOC_ARRAY(module->types[i].result_types, result_count_type);

        for (uint32_t j = 0; j < result_count_type; ++j) {
            wah_type_t type;
            WAH_CHECK(wah_decode_val_type(ptr, section_end, &type));
            module->types[i].result_types[j] = type;
        }
    }

    return WAH_OK;
}

static wah_error_t wah_parse_function_section(const uint8_t **ptr, const uint8_t *section_end, wah_module_t *module) {
    uint32_t count;
    WAH_CHECK(wah_decode_and_validate_count(ptr, section_end, &count, 1));

    module->function_count = count;
    WAH_MALLOC_ARRAY(module->function_type_indices, count);

    for (uint32_t i = 0; i < count; ++i) {
        WAH_CHECK(wah_decode_uleb128(ptr, section_end, &module->function_type_indices[i]));
        WAH_ENSURE(module->function_type_indices[i] < module->type_count, WAH_ERROR_VALIDATION_FAILED);
    }
    return WAH_OK;
}

// Validation helper function that handles a single opcode
static wah_error_t wah_validate_opcode(uint16_t opcode_val, const uint8_t **code_ptr, const uint8_t *code_end, wah_validation_context_t *vctx, const wah_code_body_t* code_body) {
#define POP(T) WAH_CHECK(wah_validation_pop_and_match_type(vctx, WAH_TYPE_##T))
#define PUSH(T) WAH_CHECK(wah_validation_push_type(vctx, WAH_TYPE_##T))

    // Note: WAH_OPCLASS__AB_C should POP(B) first, then POP(A) and PUSH(C)!
    switch (wah_opclasses[opcode_val]) {
        case WAH_OPCLASS__I_I:   POP(I32); PUSH(I32); return WAH_OK;
        case WAH_OPCLASS__I_L:   POP(I32); PUSH(I64); return WAH_OK;
        case WAH_OPCLASS__I_F:   POP(I32); PUSH(F32); return WAH_OK;
        case WAH_OPCLASS__I_D:   POP(I32); PUSH(F64); return WAH_OK;
        case WAH_OPCLASS__I_V:   POP(I32); PUSH(V128); return WAH_OK;
        case WAH_OPCLASS__II_I:  POP(I32); POP(I32); PUSH(I32); return WAH_OK;
        case WAH_OPCLASS__IV_V:  POP(V128); POP(I32); PUSH(V128); return WAH_OK;
        case WAH_OPCLASS__L_I:   POP(I64); PUSH(I32); return WAH_OK;
        case WAH_OPCLASS__L_L:   POP(I64); PUSH(I64); return WAH_OK;
        case WAH_OPCLASS__L_F:   POP(I64); PUSH(F32); return WAH_OK;
        case WAH_OPCLASS__L_D:   POP(I64); PUSH(F64); return WAH_OK;
        case WAH_OPCLASS__L_V:   POP(I64); PUSH(V128); return WAH_OK;
        case WAH_OPCLASS__LL_I:  POP(I64); POP(I64); PUSH(I32); return WAH_OK;
        case WAH_OPCLASS__LL_L:  POP(I64); POP(I64); PUSH(I64); return WAH_OK;
        case WAH_OPCLASS__LV_V:  POP(V128); POP(I64); PUSH(V128); return WAH_OK;
        case WAH_OPCLASS__F_I:   POP(F32); PUSH(I32); return WAH_OK;
        case WAH_OPCLASS__F_L:   POP(F32); PUSH(I64); return WAH_OK;
        case WAH_OPCLASS__F_F:   POP(F32); PUSH(F32); return WAH_OK;
        case WAH_OPCLASS__F_V:   POP(F32); PUSH(V128); return WAH_OK;
        case WAH_OPCLASS__FF_I:  POP(F32); POP(F32); PUSH(I32); return WAH_OK;
        case WAH_OPCLASS__FF_F:  POP(F32); POP(F32); PUSH(F32); return WAH_OK;
        case WAH_OPCLASS__D_I:   POP(F64); PUSH(I32); return WAH_OK;
        case WAH_OPCLASS__D_L:   POP(F64); PUSH(I64); return WAH_OK;
        case WAH_OPCLASS__D_D:   POP(F64); PUSH(F64); return WAH_OK;
        case WAH_OPCLASS__D_V:   POP(F64); PUSH(V128); return WAH_OK;
        case WAH_OPCLASS__DD_I:  POP(F64); POP(F64); PUSH(I32); return WAH_OK;
        case WAH_OPCLASS__DD_L:  POP(F64); POP(F64); PUSH(I64); return WAH_OK;
        case WAH_OPCLASS__DD_D:  POP(F64); POP(F64); PUSH(F64); return WAH_OK;
        case WAH_OPCLASS__V_I:   POP(V128); PUSH(I32); return WAH_OK;
        case WAH_OPCLASS__V_V:   POP(V128); PUSH(V128); return WAH_OK;
        case WAH_OPCLASS__VI_V:  POP(I32); POP(V128); PUSH(V128); return WAH_OK;
        case WAH_OPCLASS__VV_V:  POP(V128); POP(V128); PUSH(V128); return WAH_OK;
        case WAH_OPCLASS__VVV_V: POP(V128); POP(V128); POP(V128); PUSH(V128); return WAH_OK;
    }

    switch (opcode_val) {
#define LOAD_OP(T, max_lg_align) { \
            uint32_t align, memidx; \
            uint64_t offset; \
            WAH_CHECK(wah_decode_memarg(code_ptr, code_end, &align, &memidx, &offset)); \
            WAH_ENSURE(align <= max_lg_align, WAH_ERROR_VALIDATION_FAILED); \
            WAH_ENSURE(memidx < vctx->module->memory_count, WAH_ERROR_VALIDATION_FAILED); \
            wah_type_t addr_type = vctx->module->memories[memidx].addr_type; \
            if (addr_type == WAH_TYPE_I32) WAH_ENSURE(offset <= 0xFFFFFFFFU, WAH_ERROR_VALIDATION_FAILED); \
            WAH_CHECK(wah_validation_pop_and_match_type(vctx, addr_type)); PUSH(T); break; \
        }

#define STORE_OP(T, max_lg_align) { \
            uint32_t align, memidx; \
            uint64_t offset; \
            WAH_CHECK(wah_decode_memarg(code_ptr, code_end, &align, &memidx, &offset)); \
            WAH_ENSURE(align <= max_lg_align, WAH_ERROR_VALIDATION_FAILED); \
            WAH_ENSURE(memidx < vctx->module->memory_count, WAH_ERROR_VALIDATION_FAILED); \
            wah_type_t addr_type = vctx->module->memories[memidx].addr_type; \
            if (addr_type == WAH_TYPE_I32) WAH_ENSURE(offset <= 0xFFFFFFFFU, WAH_ERROR_VALIDATION_FAILED); \
            POP(T); WAH_CHECK(wah_validation_pop_and_match_type(vctx, addr_type)); break; \
        }

#define LOAD_V128_LANE_OP(max_lg_align) { \
            uint32_t align, memidx; \
            uint64_t offset; \
            WAH_CHECK(wah_decode_memarg(code_ptr, code_end, &align, &memidx, &offset)); \
            WAH_ENSURE(*code_ptr < code_end, WAH_ERROR_UNEXPECTED_EOF); \
            uint8_t lane_idx = *(*code_ptr)++; \
            WAH_ENSURE(align <= max_lg_align, WAH_ERROR_VALIDATION_FAILED); \
            WAH_ENSURE(lane_idx < (16 >> max_lg_align), WAH_ERROR_VALIDATION_FAILED); \
            WAH_ENSURE(memidx < vctx->module->memory_count, WAH_ERROR_VALIDATION_FAILED); \
            wah_type_t addr_type = vctx->module->memories[memidx].addr_type; \
            if (addr_type == WAH_TYPE_I32) WAH_ENSURE(offset <= 0xFFFFFFFFU, WAH_ERROR_VALIDATION_FAILED); \
            WAH_CHECK(wah_validation_pop_and_match_type(vctx, addr_type)); POP(V128); PUSH(V128); break; \
        }

#define EXTRACT_LANE_OP(SCALAR_TYPE, LANE_COUNT) { \
            POP(V128); PUSH(SCALAR_TYPE); \
            WAH_ENSURE(*code_ptr < code_end, WAH_ERROR_UNEXPECTED_EOF); \
            uint8_t lane_idx = *(*code_ptr)++; \
            WAH_ENSURE(lane_idx < LANE_COUNT, WAH_ERROR_VALIDATION_FAILED); \
            break; \
        }

#define REPLACE_LANE_OP(SCALAR_TYPE, LANE_COUNT) { \
            POP(SCALAR_TYPE); POP(V128); PUSH(V128); \
            WAH_ENSURE(*code_ptr < code_end, WAH_ERROR_UNEXPECTED_EOF); \
            uint8_t lane_idx = *(*code_ptr)++; \
            WAH_ENSURE(lane_idx < LANE_COUNT, WAH_ERROR_VALIDATION_FAILED); \
            break; \
        }

        case WAH_OP_I32_LOAD: LOAD_OP(I32, 2)
        case WAH_OP_I64_LOAD: LOAD_OP(I64, 3)
        case WAH_OP_F32_LOAD: LOAD_OP(F32, 2)
        case WAH_OP_F64_LOAD: LOAD_OP(F64, 3)
        case WAH_OP_I32_LOAD8_S: case WAH_OP_I32_LOAD8_U: LOAD_OP(I32, 0)
        case WAH_OP_I32_LOAD16_S: case WAH_OP_I32_LOAD16_U: LOAD_OP(I32, 1)
        case WAH_OP_I64_LOAD8_S: case WAH_OP_I64_LOAD8_U: LOAD_OP(I64, 0)
        case WAH_OP_I64_LOAD16_S: case WAH_OP_I64_LOAD16_U: LOAD_OP(I64, 1)
        case WAH_OP_I64_LOAD32_S: case WAH_OP_I64_LOAD32_U: LOAD_OP(I64, 2)
        case WAH_OP_I32_STORE: STORE_OP(I32, 2)
        case WAH_OP_I64_STORE: STORE_OP(I64, 3)
        case WAH_OP_F32_STORE: STORE_OP(F32, 2)
        case WAH_OP_F64_STORE: STORE_OP(F64, 3)
        case WAH_OP_I32_STORE8: STORE_OP(I32, 0)
        case WAH_OP_I32_STORE16: STORE_OP(I32, 1)
        case WAH_OP_I64_STORE8: STORE_OP(I64, 0)
        case WAH_OP_I64_STORE16: STORE_OP(I64, 1)
        case WAH_OP_I64_STORE32: STORE_OP(I64, 2)
        case WAH_OP_V128_LOAD: LOAD_OP(V128, 4)
        case WAH_OP_V128_LOAD8X8_S: case WAH_OP_V128_LOAD8X8_U: LOAD_OP(V128, 3)
        case WAH_OP_V128_LOAD16X4_S: case WAH_OP_V128_LOAD16X4_U: LOAD_OP(V128, 3)
        case WAH_OP_V128_LOAD32X2_S: case WAH_OP_V128_LOAD32X2_U: LOAD_OP(V128, 3)
        case WAH_OP_V128_LOAD8_SPLAT: LOAD_OP(V128, 0)
        case WAH_OP_V128_LOAD16_SPLAT: LOAD_OP(V128, 1)
        case WAH_OP_V128_LOAD32_SPLAT: LOAD_OP(V128, 2)
        case WAH_OP_V128_LOAD64_SPLAT: LOAD_OP(V128, 3)
        case WAH_OP_V128_LOAD32_ZERO: LOAD_OP(V128, 2)
        case WAH_OP_V128_LOAD64_ZERO: LOAD_OP(V128, 3)
        case WAH_OP_V128_STORE: STORE_OP(V128, 4)

        case WAH_OP_V128_LOAD8_LANE: LOAD_V128_LANE_OP(0)
        case WAH_OP_V128_LOAD16_LANE: LOAD_V128_LANE_OP(1)
        case WAH_OP_V128_LOAD32_LANE: LOAD_V128_LANE_OP(2)
        case WAH_OP_V128_LOAD64_LANE: LOAD_V128_LANE_OP(3)

        /* Vector Lane Operations */
        case WAH_OP_I8X16_SHUFFLE: {
            POP(V128); POP(V128); PUSH(V128);
            for (int i = 0; i < 16; i++) WAH_ENSURE((*code_ptr)[i] < 32, WAH_ERROR_VALIDATION_FAILED);
            *code_ptr += 16; // 16 immediate bytes for the shuffle mask
            break;
        }

        case WAH_OP_I8X16_EXTRACT_LANE_S: EXTRACT_LANE_OP(I32, 16)
        case WAH_OP_I8X16_EXTRACT_LANE_U: EXTRACT_LANE_OP(I32, 16)
        case WAH_OP_I8X16_REPLACE_LANE: REPLACE_LANE_OP(I32, 16)
        case WAH_OP_I16X8_EXTRACT_LANE_S: EXTRACT_LANE_OP(I32, 8)
        case WAH_OP_I16X8_EXTRACT_LANE_U: EXTRACT_LANE_OP(I32, 8)
        case WAH_OP_I16X8_REPLACE_LANE: REPLACE_LANE_OP(I32, 8)
        case WAH_OP_I32X4_EXTRACT_LANE: EXTRACT_LANE_OP(I32, 4)
        case WAH_OP_I32X4_REPLACE_LANE: REPLACE_LANE_OP(I32, 4)
        case WAH_OP_I64X2_EXTRACT_LANE: EXTRACT_LANE_OP(I64, 2)
        case WAH_OP_I64X2_REPLACE_LANE: REPLACE_LANE_OP(I64, 2)
        case WAH_OP_F32X4_EXTRACT_LANE: EXTRACT_LANE_OP(F32, 4)
        case WAH_OP_F32X4_REPLACE_LANE: REPLACE_LANE_OP(F32, 4)
        case WAH_OP_F64X2_EXTRACT_LANE: EXTRACT_LANE_OP(F64, 2)
        case WAH_OP_F64X2_REPLACE_LANE: REPLACE_LANE_OP(F64, 2)

#undef LOAD_OP
#undef STORE_OP
#undef LOAD_V128_LANE_OP
#undef EXTRACT_LANE_OP
#undef REPLACE_LANE_OP

        case WAH_OP_MEMORY_SIZE: {
            uint32_t mem_idx;
            WAH_CHECK(wah_decode_uleb128(code_ptr, code_end, &mem_idx));
            WAH_ENSURE(mem_idx < vctx->module->memory_count, WAH_ERROR_VALIDATION_FAILED);
            // memory.size: [] -> [t]  (no pop, push addr_type)
            WAH_CHECK(wah_validation_push_type(vctx, vctx->module->memories[mem_idx].addr_type));
            break;
        }
        case WAH_OP_MEMORY_GROW: {
            uint32_t mem_idx;
            WAH_CHECK(wah_decode_uleb128(code_ptr, code_end, &mem_idx));
            WAH_ENSURE(mem_idx < vctx->module->memory_count, WAH_ERROR_VALIDATION_FAILED);
            wah_type_t addr_type = vctx->module->memories[mem_idx].addr_type;
            // memory.grow: [t] -> [t]  (pop delta, push previous size)
            WAH_CHECK(wah_validation_pop_and_match_type(vctx, addr_type));
            WAH_CHECK(wah_validation_push_type(vctx, addr_type));
            break;
        }
        case WAH_OP_MEMORY_FILL: {
            uint32_t mem_idx;
            WAH_CHECK(wah_decode_uleb128(code_ptr, code_end, &mem_idx));
            WAH_ENSURE(mem_idx < vctx->module->memory_count, WAH_ERROR_VALIDATION_FAILED);
            // Stack: [i32_size, i32_val, addr] (val is always i32)
            wah_type_t addr_type = vctx->module->memories[mem_idx].addr_type;
            POP(I32); POP(I32); WAH_CHECK(wah_validation_pop_and_match_type(vctx, addr_type));
            break;
        }
        case WAH_OP_MEMORY_INIT: {
            uint32_t data_idx, mem_idx;
            WAH_CHECK(wah_decode_uleb128(code_ptr, code_end, &data_idx));
            WAH_CHECK(wah_decode_uleb128(code_ptr, code_end, &mem_idx));
            WAH_ENSURE(mem_idx < vctx->module->memory_count, WAH_ERROR_VALIDATION_FAILED);
            // Stack: [i32_size, i32_src_offset, addr] (src_offset is always i32)
            wah_type_t addr_type = vctx->module->memories[mem_idx].addr_type;
            POP(I32); POP(I32); WAH_CHECK(wah_validation_pop_and_match_type(vctx, addr_type));
            if (data_idx + 1 > vctx->module->min_data_segment_count_required) {
                vctx->module->min_data_segment_count_required = data_idx + 1;
            }
            break;
        }
        case WAH_OP_MEMORY_COPY: {
            uint32_t dest_mem_idx, src_mem_idx;
            WAH_CHECK(wah_decode_uleb128(code_ptr, code_end, &dest_mem_idx));
            WAH_CHECK(wah_decode_uleb128(code_ptr, code_end, &src_mem_idx));
            WAH_ENSURE(dest_mem_idx < vctx->module->memory_count, WAH_ERROR_VALIDATION_FAILED);
            WAH_ENSURE(src_mem_idx < vctx->module->memory_count, WAH_ERROR_VALIDATION_FAILED);
            // Stack: [i32_size, src_addr, dest_addr]
            WAH_CHECK(wah_validation_pop_and_match_type(vctx, vctx->module->memories[src_mem_idx].addr_type));
            WAH_CHECK(wah_validation_pop_and_match_type(vctx, vctx->module->memories[dest_mem_idx].addr_type));
            POP(I32); // size is always i32
            break;
        }
        case WAH_OP_CALL: {
            uint32_t func_idx;
            WAH_CHECK(wah_decode_uleb128(code_ptr, code_end, &func_idx));
            uint32_t total_func_idx_count = vctx->module->function_count + vctx->module->import_function_count;
            WAH_ENSURE(func_idx < total_func_idx_count, WAH_ERROR_VALIDATION_FAILED);
            // Type checking for call (note that we don't yet have function_table!)
            const wah_func_type_t *called_func_type;
            if (func_idx < vctx->module->import_function_count) {
                called_func_type = &vctx->module->types[vctx->module->func_imports[func_idx].type_index];
            } else {
                uint32_t local_idx = func_idx - vctx->module->import_function_count;
                called_func_type = &vctx->module->types[vctx->module->function_type_indices[local_idx]];
            }
            // Pop parameters from type stack
            for (int32_t j = called_func_type->param_count - 1; j >= 0; --j) {
                WAH_CHECK(wah_validation_pop_and_match_type(vctx, called_func_type->param_types[j]));
            }
            // Push results onto type stack
            for (uint32_t j = 0; j < called_func_type->result_count; ++j) {
                WAH_CHECK(wah_validation_push_type(vctx, called_func_type->result_types[j]));
            }
            break;
        }
        case WAH_OP_CALL_INDIRECT: {
            uint32_t type_idx, table_idx;
            WAH_CHECK(wah_decode_uleb128(code_ptr, code_end, &type_idx));
            WAH_CHECK(wah_decode_uleb128(code_ptr, code_end, &table_idx));

            // Validate table index
            WAH_ENSURE(table_idx < vctx->module->table_count, WAH_ERROR_VALIDATION_FAILED);
            // Only funcref tables are supported for now
            WAH_ENSURE(vctx->module->tables[table_idx].elem_type == WAH_TYPE_FUNCREF, WAH_ERROR_VALIDATION_FAILED);

            // Validate type index
            WAH_ENSURE(type_idx < vctx->module->type_count, WAH_ERROR_VALIDATION_FAILED);

            // Pop function index (i32) from stack
            WAH_CHECK(wah_validation_pop_and_match_type(vctx, WAH_TYPE_I32));

            // Get the expected function type
            const wah_func_type_t *expected_func_type = &vctx->module->types[type_idx];

            // Pop parameters from type stack
            for (int32_t j = expected_func_type->param_count - 1; j >= 0; --j) {
                WAH_CHECK(wah_validation_pop_and_match_type(vctx, expected_func_type->param_types[j]));
            }
            // Push results onto type stack
            for (uint32_t j = 0; j < expected_func_type->result_count; ++j) {
                WAH_CHECK(wah_validation_push_type(vctx, expected_func_type->result_types[j]));
            }
            break;
        }
        case WAH_OP_LOCAL_GET:
        case WAH_OP_LOCAL_SET:
        case WAH_OP_LOCAL_TEE: {
            uint32_t local_idx;
            WAH_CHECK(wah_decode_uleb128(code_ptr, code_end, &local_idx));

            WAH_ENSURE(local_idx < vctx->total_locals, WAH_ERROR_VALIDATION_FAILED);

            wah_type_t expected_type;
            if (local_idx < vctx->func_type->param_count) {
                expected_type = vctx->func_type->param_types[local_idx];
            } else {
                expected_type = code_body->local_types[local_idx - vctx->func_type->param_count];
            }

            if (opcode_val == WAH_OP_LOCAL_GET) {
                WAH_CHECK(wah_validation_push_type(vctx, expected_type));
            } else if (opcode_val == WAH_OP_LOCAL_SET) {
                WAH_CHECK(wah_validation_pop_and_match_type(vctx, expected_type));
            } else { // WAH_OP_LOCAL_TEE
                WAH_CHECK(wah_validation_pop_and_match_type(vctx, expected_type));
                WAH_CHECK(wah_validation_push_type(vctx, expected_type));
            }
            break;
        }

        case WAH_OP_GLOBAL_GET: {
            uint32_t global_idx;
            WAH_CHECK(wah_decode_uleb128(code_ptr, code_end, &global_idx));
            WAH_ENSURE(global_idx < vctx->module->global_count, WAH_ERROR_VALIDATION_FAILED);
            wah_type_t global_type = vctx->module->globals[global_idx].type;
            WAH_CHECK(wah_validation_push_type(vctx, global_type));
            break;
        }
        case WAH_OP_GLOBAL_SET: {
            uint32_t global_idx;
            WAH_CHECK(wah_decode_uleb128(code_ptr, code_end, &global_idx));
            WAH_ENSURE(global_idx < vctx->module->global_count, WAH_ERROR_VALIDATION_FAILED);
            WAH_ENSURE(vctx->module->globals[global_idx].is_mutable, WAH_ERROR_VALIDATION_FAILED); // Cannot set immutable global
            WAH_CHECK(wah_validation_pop_and_match_type(vctx, vctx->module->globals[global_idx].type));
            break;
        }
        case WAH_OP_TABLE_GET: {
            uint32_t table_idx;
            WAH_CHECK(wah_decode_uleb128(code_ptr, code_end, &table_idx));
            WAH_ENSURE(table_idx < vctx->module->table_count, WAH_ERROR_VALIDATION_FAILED);
            POP(I32); // index
            WAH_CHECK(wah_validation_push_type(vctx, vctx->module->tables[table_idx].elem_type)); // push element type
            break;
        }
        case WAH_OP_TABLE_SET: {
            uint32_t table_idx;
            WAH_CHECK(wah_decode_uleb128(code_ptr, code_end, &table_idx));
            WAH_ENSURE(table_idx < vctx->module->table_count, WAH_ERROR_VALIDATION_FAILED);
            wah_type_t elem_type = vctx->module->tables[table_idx].elem_type;
            WAH_CHECK(wah_validation_pop_and_match_type(vctx, elem_type)); // value (top)
            POP(I32); // index (below)
            break;
        }
        case WAH_OP_TABLE_SIZE: {
            uint32_t table_idx;
            WAH_CHECK(wah_decode_uleb128(code_ptr, code_end, &table_idx));
            WAH_ENSURE(table_idx < vctx->module->table_count, WAH_ERROR_VALIDATION_FAILED);
            PUSH(I32); break;
        }
        case WAH_OP_TABLE_GROW: {
            uint32_t table_idx;
            WAH_CHECK(wah_decode_uleb128(code_ptr, code_end, &table_idx));
            WAH_ENSURE(table_idx < vctx->module->table_count, WAH_ERROR_VALIDATION_FAILED);
            wah_type_t elem_type = vctx->module->tables[table_idx].elem_type;
            POP(I32); // delta (on top: [ref t, i32] spec order)
            WAH_CHECK(wah_validation_pop_and_match_type(vctx, elem_type)); // init value
            PUSH(I32); break; // old size or -1
        }
        case WAH_OP_TABLE_FILL: {
            uint32_t table_idx;
            WAH_CHECK(wah_decode_uleb128(code_ptr, code_end, &table_idx));
            WAH_ENSURE(table_idx < vctx->module->table_count, WAH_ERROR_VALIDATION_FAILED);
            wah_type_t elem_type = vctx->module->tables[table_idx].elem_type;
            POP(I32); // size
            WAH_CHECK(wah_validation_pop_and_match_type(vctx, elem_type)); // value
            POP(I32); break; // offset
        }
        case WAH_OP_TABLE_COPY: {
            uint32_t dst_table_idx, src_table_idx;
            WAH_CHECK(wah_decode_uleb128(code_ptr, code_end, &dst_table_idx));
            WAH_CHECK(wah_decode_uleb128(code_ptr, code_end, &src_table_idx));
            WAH_ENSURE(dst_table_idx < vctx->module->table_count, WAH_ERROR_VALIDATION_FAILED);
            WAH_ENSURE(src_table_idx < vctx->module->table_count, WAH_ERROR_VALIDATION_FAILED);
            wah_type_t dst_type = vctx->module->tables[dst_table_idx].elem_type;
            wah_type_t src_type = vctx->module->tables[src_table_idx].elem_type;
            WAH_ENSURE(dst_type == src_type, WAH_ERROR_VALIDATION_FAILED);
            POP(I32); POP(I32); POP(I32); break; // size, src offset, dst offset
        }
        case WAH_OP_TABLE_INIT: {
            uint32_t elem_idx, table_idx;
            WAH_CHECK(wah_decode_uleb128(code_ptr, code_end, &elem_idx));
            WAH_CHECK(wah_decode_uleb128(code_ptr, code_end, &table_idx));
            WAH_ENSURE(elem_idx < vctx->module->element_segment_count, WAH_ERROR_VALIDATION_FAILED);
            WAH_ENSURE(table_idx < vctx->module->table_count, WAH_ERROR_VALIDATION_FAILED);
            POP(I32); POP(I32); POP(I32); break; // size, src offset, dst offset
        }
        case WAH_OP_ELEM_DROP: {
            uint32_t elem_idx;
            WAH_CHECK(wah_decode_uleb128(code_ptr, code_end, &elem_idx));
            WAH_ENSURE(elem_idx < vctx->module->element_segment_count, WAH_ERROR_VALIDATION_FAILED);
            break;
        }

        case WAH_OP_I32_CONST: {
            int32_t val;
            WAH_CHECK(wah_decode_sleb128_32(code_ptr, code_end, &val));
            PUSH(I32); break;
        }
        case WAH_OP_I64_CONST: {
            int64_t val;
            WAH_CHECK(wah_decode_sleb128_64(code_ptr, code_end, &val));
            PUSH(I64); break;
        }
        case WAH_OP_F32_CONST: {
            WAH_ENSURE(code_end - *code_ptr >= 4, WAH_ERROR_UNEXPECTED_EOF);
            *code_ptr += 4;
            PUSH(F32); break;
        }
        case WAH_OP_F64_CONST: {
            WAH_ENSURE(code_end - *code_ptr >= 8, WAH_ERROR_UNEXPECTED_EOF);
            *code_ptr += 8;
            PUSH(F64); break;
        }
        case WAH_OP_V128_CONST: {
            WAH_ENSURE(code_end - *code_ptr >= 16, WAH_ERROR_UNEXPECTED_EOF);
            *code_ptr += 16;
            PUSH(V128); break;
        }

        case WAH_OP_F32_DEMOTE_F64:  POP(F64); PUSH(F32); break;
        case WAH_OP_F64_PROMOTE_F32: POP(F32); PUSH(F64); break;

        // Parametric operations
        case WAH_OP_DROP: {
            wah_type_t type;
            return wah_validation_pop_type(vctx, &type);
        }

        case WAH_OP_SELECT: {
            wah_type_t b_type, a_type;
            WAH_CHECK(wah_validation_pop_and_match_type(vctx, WAH_TYPE_I32));
            WAH_CHECK(wah_validation_pop_type(vctx, &b_type));
            WAH_CHECK(wah_validation_pop_type(vctx, &a_type));
            // If a_type and b_type are different, and neither is ANY, then it's an error.
            WAH_ENSURE(a_type == b_type || a_type == WAH_TYPE_ANY || b_type == WAH_TYPE_ANY, WAH_ERROR_VALIDATION_FAILED);
            // If either is ANY, the result is ANY. Otherwise, it's a_type (which equals b_type).
            if (a_type == WAH_TYPE_ANY || b_type == WAH_TYPE_ANY) {
                return wah_validation_push_type(vctx, WAH_TYPE_ANY);
            } else {
                return wah_validation_push_type(vctx, a_type);
            }
        }

        // Control flow operations
        case WAH_OP_NOP: break;
        case WAH_OP_UNREACHABLE: vctx->is_unreachable = true; break;

        case WAH_OP_BLOCK:
        case WAH_OP_LOOP:
        case WAH_OP_IF: {
            wah_type_t cond_type;
            if (opcode_val == WAH_OP_IF) {
                WAH_CHECK(wah_validation_pop_type(vctx, &cond_type));
                WAH_CHECK(wah_validate_type_match(cond_type, WAH_TYPE_I32));
            }

            int32_t block_type_val;
            WAH_CHECK(wah_decode_sleb128_32(code_ptr, code_end, &block_type_val));

            WAH_ENSURE(vctx->control_sp < WAH_MAX_CONTROL_DEPTH, WAH_ERROR_TOO_LARGE);
            wah_validation_control_frame_t* frame = &vctx->control_stack[vctx->control_sp++];
            frame->opcode = (wah_opcode_t)opcode_val;
            frame->else_found = false;
            frame->is_unreachable = vctx->is_unreachable; // Initialize with current reachability
            frame->stack_height = vctx->current_stack_depth; // Store current stack height

            wah_func_type_t* bt = &frame->block_type;
            *bt = (wah_func_type_t){0};

            if (block_type_val < 0) { // Value type
                wah_type_t result_type = 0; // Initialize to a default invalid value
                switch(block_type_val) {
                    case -1: result_type = WAH_TYPE_I32; break;
                    case -2: result_type = WAH_TYPE_I64; break;
                    case -3: result_type = WAH_TYPE_F32; break;
                    case -4: result_type = WAH_TYPE_F64; break;
                    case -0x40: break; // empty
                    default: return WAH_ERROR_VALIDATION_FAILED;
                }

                if (result_type != 0) { // If not empty
                    bt->result_count = 1;
                    WAH_MALLOC_ARRAY(bt->result_types, 1);
                    bt->result_types[0] = result_type;
                }
            } else { // Function type index
                uint32_t type_idx = (uint32_t)block_type_val;
                WAH_ENSURE(type_idx < vctx->module->type_count, WAH_ERROR_VALIDATION_FAILED);
                const wah_func_type_t* referenced_type = &vctx->module->types[type_idx];

                bt->param_count = referenced_type->param_count;
                if (bt->param_count > 0) {
                    WAH_MALLOC_ARRAY(bt->param_types, bt->param_count);
                    memcpy(bt->param_types, referenced_type->param_types, sizeof(wah_type_t) * bt->param_count);
                }

                bt->result_count = referenced_type->result_count;
                if (bt->result_count > 0) {
                    WAH_MALLOC_ARRAY(bt->result_types, bt->result_count);
                    memcpy(bt->result_types, referenced_type->result_types, sizeof(wah_type_t) * bt->result_count);
                }
            }

            // Check params are available on stack (but don't consume them for block type)
            // Block type parameters are inputs to the block, not consumed by block declaration
            WAH_ENSURE(vctx->current_stack_depth >= bt->param_count, WAH_ERROR_VALIDATION_FAILED);
            for (uint32_t i = 0; i < bt->param_count; ++i) {
                // Peek at the type without popping: stack[sp - param_count + i]
                wah_type_t actual_type = vctx->type_stack.data[vctx->type_stack.sp - bt->param_count + i];
                WAH_CHECK(wah_validate_type_match(actual_type, bt->param_types[i]));
            }

            frame->type_stack_sp = vctx->type_stack.sp;
            return WAH_OK;
        }
        case WAH_OP_ELSE: {
            WAH_ENSURE(vctx->control_sp > 0, WAH_ERROR_VALIDATION_FAILED);
            wah_validation_control_frame_t* frame = &vctx->control_stack[vctx->control_sp - 1];
            WAH_ENSURE(frame->opcode == WAH_OP_IF && !frame->else_found, WAH_ERROR_VALIDATION_FAILED);
            frame->else_found = true;

            // Pop results of 'if' branch and verify
            for (int32_t i = frame->block_type.result_count - 1; i >= 0; --i) {
                // If the stack was unreachable, any type is fine. Otherwise, it must match.
                WAH_CHECK(wah_validation_pop_and_match_type(vctx, frame->block_type.result_types[i]));
            }

            // Reset stack to the state before the 'if' block
            vctx->type_stack.sp = frame->type_stack_sp;
            vctx->current_stack_depth = frame->type_stack_sp;

            // The 'else' branch is now reachable
            vctx->is_unreachable = false;
            return WAH_OK;
        }
        case WAH_OP_END: {
            if (vctx->control_sp == 0) {
                // This is the final 'end' of the function body.
                // Final validation for the function's result types.
                // The stack should contain exactly the function's result types.
                if (vctx->func_type->result_count == 0) {
                    WAH_ENSURE(vctx->type_stack.sp == 0, WAH_ERROR_VALIDATION_FAILED);
                } else { // handle multi-results
                    WAH_ENSURE(vctx->type_stack.sp == vctx->func_type->result_count, WAH_ERROR_VALIDATION_FAILED);
                    // Pop and match each result type in reverse order
                    for (int32_t j = vctx->func_type->result_count - 1; j >= 0; --j) {
                        WAH_CHECK(wah_validation_pop_and_match_type(vctx, vctx->func_type->result_types[j]));
                    }
                }
                // Reset unreachable state for the function's end
                vctx->is_unreachable = false;
                return WAH_OK;
            }

            wah_validation_control_frame_t* frame = &vctx->control_stack[vctx->control_sp - 1];

            // if without else is only valid when the block type has no results
            if (frame->opcode == WAH_OP_IF && !frame->else_found) {
                WAH_ENSURE(frame->block_type.result_count == 0, WAH_ERROR_VALIDATION_FAILED);
            }

            // Pop results from the executed branch and verify
            for (int32_t i = frame->block_type.result_count - 1; i >= 0; --i) {
                WAH_CHECK(wah_validation_pop_and_match_type(vctx, frame->block_type.result_types[i]));
            }

            // Reset stack to the state before the block
            vctx->type_stack.sp = frame->type_stack_sp;
            vctx->current_stack_depth = frame->type_stack_sp;

            // Push final results of the block
            for (uint32_t i = 0; i < frame->block_type.result_count; ++i) {
                WAH_CHECK(wah_validation_push_type(vctx, frame->block_type.result_types[i]));
            }

            // Free memory allocated for the block type in the control frame
            free(frame->block_type.param_types);
            free(frame->block_type.result_types);

            vctx->control_sp--;
            // Restore the unreachable state from the parent control frame
            if (vctx->control_sp > 0) {
                vctx->is_unreachable = vctx->control_stack[vctx->control_sp - 1].is_unreachable;
            } else {
                vctx->is_unreachable = false; // Function level is reachable by default
            }
            return WAH_OK;
        }

        case WAH_OP_BR: {
            uint32_t label_idx;
            WAH_CHECK(wah_decode_uleb128(code_ptr, code_end, &label_idx));
            WAH_ENSURE(label_idx < vctx->control_sp, WAH_ERROR_VALIDATION_FAILED);

            wah_validation_control_frame_t *target_frame = &vctx->control_stack[vctx->control_sp - 1 - label_idx];

            // Pop the expected result types of the target block from the current stack
            // The stack must contain these values for the branch to be valid.
            for (int32_t i = target_frame->block_type.result_count - 1; i >= 0; --i) {
                WAH_CHECK(wah_validation_pop_and_match_type(vctx, target_frame->block_type.result_types[i]));
            }

            // Discard any remaining values on the stack above the target frame's stack height
            while (vctx->current_stack_depth > target_frame->stack_height) {
                wah_type_t temp_type;
                WAH_CHECK(wah_validation_pop_type(vctx, &temp_type));
            }

            vctx->is_unreachable = true; // br makes the current path unreachable
            return WAH_OK;
        }

        case WAH_OP_BR_IF: {
            uint32_t label_idx;
            WAH_CHECK(wah_decode_uleb128(code_ptr, code_end, &label_idx));
            WAH_ENSURE(label_idx < vctx->control_sp, WAH_ERROR_VALIDATION_FAILED);

            // Pop condition (i32) from stack for br_if
            WAH_CHECK(wah_validation_pop_and_match_type(vctx, WAH_TYPE_I32));

            // If the current path is unreachable, the stack is polymorphic, so type checks are trivial.
            // We only need to ensure the conceptual stack height is maintained.
            if (vctx->is_unreachable) {
                return WAH_OK;
            }

            wah_validation_control_frame_t *target_frame = &vctx->control_stack[vctx->control_sp - 1 - label_idx];

            // Check if there are enough values on the stack for the target block's results
            // (which become parameters to the target block if branched to).
            WAH_ENSURE(vctx->current_stack_depth >= target_frame->block_type.result_count, WAH_ERROR_VALIDATION_FAILED);

            // Check the types of these values without popping them
            for (uint32_t i = 0; i < target_frame->block_type.result_count; ++i) {
                // Access the type stack directly to peek at values
                wah_type_t actual_type = vctx->type_stack.data[vctx->type_stack.sp - target_frame->block_type.result_count + i];
                WAH_CHECK(wah_validate_type_match(actual_type, target_frame->block_type.result_types[i]));
            }

            // The stack state for the fall-through path is now correct (condition popped).
            // The current path remains reachable.
            return WAH_OK;
        }

        case WAH_OP_BR_TABLE: {
            wah_error_t err = WAH_OK; // Declare err here for goto cleanup
            uint32_t num_targets;
            WAH_CHECK(wah_decode_uleb128(code_ptr, code_end, &num_targets));

            // Store all label_idx values to process them after decoding all
            uint32_t* label_indices = NULL;
            WAH_MALLOC_ARRAY_GOTO(label_indices, num_targets + 1, cleanup_br_table); // +1 for default target

            // Decode target table (vector of label_idx)
            for (uint32_t i = 0; i < num_targets; ++i) {
                WAH_CHECK_GOTO(wah_decode_uleb128(code_ptr, code_end, &label_indices[i]), cleanup_br_table);
                WAH_ENSURE_GOTO(label_indices[i] < vctx->control_sp, WAH_ERROR_VALIDATION_FAILED, cleanup_br_table);
            }

            // Decode default target (label_idx)
            WAH_CHECK_GOTO(wah_decode_uleb128(code_ptr, code_end, &label_indices[num_targets]), cleanup_br_table); // Last element is default
            WAH_ENSURE_GOTO(label_indices[num_targets] < vctx->control_sp, WAH_ERROR_VALIDATION_FAILED, cleanup_br_table);

            // Pop index (i32) from stack
            WAH_CHECK_GOTO(wah_validation_pop_and_match_type(vctx, WAH_TYPE_I32), cleanup_br_table);

            // Get the block type of the default target as the reference
            wah_validation_control_frame_t *default_target_frame = &vctx->control_stack[vctx->control_sp - 1 - label_indices[num_targets]];
            const wah_func_type_t *expected_block_type = &default_target_frame->block_type;

            // Check type consistency for all targets
            for (uint32_t i = 0; i < num_targets + 1; ++i) {
                wah_validation_control_frame_t *current_target_frame = &vctx->control_stack[vctx->control_sp - 1 - label_indices[i]];
                const wah_func_type_t *current_block_type = &current_target_frame->block_type;

                // All target blocks must have the same result count and types
                WAH_ENSURE_GOTO(current_block_type->result_count == expected_block_type->result_count, WAH_ERROR_VALIDATION_FAILED, cleanup_br_table);
                for (uint32_t j = 0; j < expected_block_type->result_count; ++j) {
                    WAH_ENSURE_GOTO(current_block_type->result_types[j] == expected_block_type->result_types[j], WAH_ERROR_VALIDATION_FAILED, cleanup_br_table);
                }
            }

            // Pop the expected result types of the target block from the current stack
            // The stack must contain these values for the branch to be valid.
            for (int32_t i = expected_block_type->result_count - 1; i >= 0; --i) {
                WAH_CHECK_GOTO(wah_validation_pop_and_match_type(vctx, expected_block_type->result_types[i]), cleanup_br_table);
            }

            // Discard any remaining values on the stack above the target frame's stack height
            while (vctx->current_stack_depth > default_target_frame->stack_height) {
                wah_type_t temp_type;
                WAH_CHECK_GOTO(wah_validation_pop_type(vctx, &temp_type), cleanup_br_table);
            }

            vctx->is_unreachable = true; // br_table makes the current path unreachable
            err = WAH_OK; // Set err to WAH_OK before cleanup
        cleanup_br_table:
            free(label_indices);
            return err;
        }

        case WAH_OP_RETURN:
            // Pop the function's result types from the stack
            for (int32_t j = vctx->func_type->result_count - 1; j >= 0; --j) {
                WAH_CHECK(wah_validation_pop_and_match_type(vctx, vctx->func_type->result_types[j]));
            }
            vctx->is_unreachable = true; // After return, the current path becomes unreachable
            return WAH_OK;

        case WAH_OP_REF_NULL: {
            // Read ref type from original bytecode (validation happens before preparsing)
            wah_type_t ref_type;
            WAH_CHECK(wah_decode_ref_type(code_ptr, code_end, &ref_type));

            // Push null reference
            WAH_CHECK(wah_validation_push_type(vctx, ref_type));
            break;
        }

        case WAH_OP_REF_IS_NULL: {
            // Pop reference, push i32 result
            wah_type_t ref_type;
            WAH_CHECK(wah_validation_pop_type(vctx, &ref_type));
            WAH_ENSURE(ref_type == WAH_TYPE_FUNCREF || ref_type == WAH_TYPE_EXTERNREF, WAH_ERROR_VALIDATION_FAILED);
            WAH_CHECK(wah_validation_push_type(vctx, WAH_TYPE_I32));
            break;
        }

        case WAH_OP_REF_FUNC: {
            // Read function index from bytecode
            uint32_t func_idx;
            WAH_CHECK(wah_decode_uleb128(code_ptr, code_end, &func_idx));
            WAH_ENSURE(func_idx < vctx->module->function_count + vctx->module->import_function_count, WAH_ERROR_VALIDATION_FAILED);
            WAH_CHECK(wah_validation_push_type(vctx, WAH_TYPE_FUNCREF));
            break;
        }

        default:
            return WAH_ERROR_VALIDATION_FAILED;
    }

#undef PUSH
#undef POP

    return WAH_OK;
}

static wah_error_t wah_parse_code_section(const uint8_t **ptr, const uint8_t *section_end, wah_module_t *module) {
    wah_error_t err = WAH_OK;
    wah_validation_context_t vctx = {0};

    uint32_t count;
    // A code body entry requires at least 3 bytes (body_size, num_locals, END opcode).
    WAH_CHECK_GOTO(wah_decode_and_validate_count(ptr, section_end, &count, 3), cleanup);
    WAH_ENSURE_GOTO(count == module->function_count, WAH_ERROR_VALIDATION_FAILED, cleanup);
    WAH_MALLOC_ARRAY_GOTO(module->code_bodies, count, cleanup);
    module->code_count = 0; // For a while, this doubles as how many entries have been initialized (for cleanup).

    for (uint32_t i = 0; i < count; ++i) {
        // Initialize pointer fields for safe cleanup on parsing failure
        module->code_bodies[i].local_types = NULL;
        module->code_bodies[i].parsed_code = (wah_parsed_code_t){0};
        ++module->code_count;

        uint32_t body_size;
        WAH_CHECK_GOTO(wah_decode_uleb128(ptr, section_end, &body_size), cleanup);

        WAH_ENSURE_GOTO(body_size <= (size_t)(section_end - *ptr), WAH_ERROR_VALIDATION_FAILED, cleanup);
        const uint8_t *code_body_end = *ptr + body_size;

        // Parse locals
        uint32_t num_local_entries;
        // Each local entry requires at least 2 bytes (local_type_count, local_type).
        WAH_CHECK_GOTO(wah_decode_and_validate_count(ptr, code_body_end, &num_local_entries, 2), cleanup);

        uint32_t current_local_count = 0;
        const uint8_t* ptr_count = *ptr;
        for (uint32_t j = 0; j < num_local_entries; ++j) {
            uint32_t local_type_count;
            WAH_CHECK_GOTO(wah_decode_uleb128(&ptr_count, code_body_end, &local_type_count), cleanup);
            ptr_count++; // Skip the actual type byte
            WAH_ENSURE_GOTO(UINT32_MAX - current_local_count >= local_type_count, WAH_ERROR_TOO_LARGE, cleanup);
            current_local_count += local_type_count;
        }
        module->code_bodies[i].local_count = current_local_count;
        WAH_MALLOC_ARRAY_GOTO(module->code_bodies[i].local_types, current_local_count, cleanup);

        uint32_t local_idx = 0;
        for (uint32_t j = 0; j < num_local_entries; ++j) {
            uint32_t local_type_count;
            WAH_CHECK_GOTO(wah_decode_uleb128(ptr, code_body_end, &local_type_count), cleanup);
            wah_type_t type;
            WAH_CHECK_GOTO(wah_decode_val_type(ptr, code_body_end, &type), cleanup);
            for (uint32_t k = 0; k < local_type_count; ++k) {
                module->code_bodies[i].local_types[local_idx++] = type;
            }
        }

        module->code_bodies[i].code_size = (uint32_t)(code_body_end - *ptr);
        module->code_bodies[i].code = *ptr;

        // --- Validation Pass for Code Body ---
        const wah_func_type_t *func_type = &module->types[module->function_type_indices[i]];

        vctx = (wah_validation_context_t){
            .is_unreachable = false, // Functions start in a reachable state
            .module = module,
            .func_type = func_type,
            .total_locals = func_type->param_count + module->code_bodies[i].local_count,
        };

        const uint8_t *code_ptr_validation = module->code_bodies[i].code;
        const uint8_t *validation_end = code_ptr_validation + module->code_bodies[i].code_size;

        while (code_ptr_validation < validation_end) {
            uint16_t current_opcode_val;
            WAH_CHECK_GOTO(wah_decode_opcode(&code_ptr_validation, validation_end, &current_opcode_val), cleanup);

            if (current_opcode_val == WAH_OP_END) {
                 if (vctx.control_sp == 0) { // End of function
                    if (vctx.func_type->result_count == 0) {
                        WAH_ENSURE_GOTO(vctx.type_stack.sp == 0, WAH_ERROR_VALIDATION_FAILED, cleanup); // Unmatched control frames
                    } else { // handle multi-results
                        // If unreachable, the stack is polymorphic, so we don't strictly check sp.
                        // We still pop to ensure the conceptual stack height is correct.
                        for (int32_t j = vctx.func_type->result_count - 1; j >= 0; --j) {
                            WAH_CHECK_GOTO(wah_validation_pop_and_match_type(&vctx, vctx.func_type->result_types[j]), cleanup);
                        }
                    }
                    // END opcode must be the very last byte in function body
                    WAH_ENSURE_GOTO(code_ptr_validation == validation_end, WAH_ERROR_VALIDATION_FAILED, cleanup);
                    break; // End of validation loop
                }
            }
            WAH_CHECK_GOTO(wah_validate_opcode(current_opcode_val, &code_ptr_validation, validation_end, &vctx, &module->code_bodies[i]), cleanup);
        }
        WAH_ENSURE_GOTO(vctx.control_sp == 0, WAH_ERROR_VALIDATION_FAILED, cleanup);
        // --- End Validation Pass ---

        module->code_bodies[i].max_stack_depth = vctx.max_stack_depth;

        // Pre-parse the code for optimized execution
        WAH_CHECK_GOTO(wah_preparse_code(module, i, module->code_bodies[i].code, module->code_bodies[i].code_size, &module->code_bodies[i].parsed_code), cleanup);

        *ptr = code_body_end;
    }
    err = WAH_OK; // Ensure err is WAH_OK if everything succeeded

cleanup:
    if (err != WAH_OK) {
        // Free memory allocated for control frames during validation
        for (int32_t j = vctx.control_sp - 1; j >= 0; --j) {
            wah_validation_control_frame_t* frame = &vctx.control_stack[j];
            free(frame->block_type.param_types);
            free(frame->block_type.result_types);
        }

        if (module->code_bodies) {
            for (uint32_t i = 0; i < module->code_count; ++i) {
                free(module->code_bodies[i].local_types);
                wah_free_parsed_code(&module->code_bodies[i].parsed_code);
            }
            free(module->code_bodies);
            module->code_bodies = NULL;
        }
    }
    return err;
}

// --- Const Expression Validation ---
// Validates a const expression for WebAssembly 3.0
// max_global_idx: maximum global index that can be referenced (globals defined earlier only)
static wah_error_t wah_validate_const_expr(
    const uint8_t **ptr,
    const uint8_t *section_end,
    wah_type_t expected_type,
    wah_module_t *module,
    uint32_t max_global_idx
) {
    // Simple type stack for const expression validation
    wah_type_t type_stack[16];
    uint32_t stack_depth = 0;

    // Push to type stack
    #define CONST_PUSH(t) do { \
        WAH_ENSURE(stack_depth < 16, WAH_ERROR_TOO_LARGE); \
        type_stack[stack_depth++] = (t); \
    } while(0)

    // Pop from type stack
    #define CONST_POP(t) do { \
        WAH_ENSURE(stack_depth > 0, WAH_ERROR_VALIDATION_FAILED); \
        wah_type_t _actual = type_stack[--stack_depth]; \
        WAH_ENSURE(_actual == (t) || _actual == WAH_TYPE_ANY, WAH_ERROR_VALIDATION_FAILED); \
    } while(0)

    while (1) {
        WAH_ENSURE(*ptr < section_end, WAH_ERROR_UNEXPECTED_EOF);
        uint8_t raw_opcode = *(*ptr)++;

        // Check for extended opcodes
        uint16_t opcode_val;
        if (raw_opcode == 0xFB || raw_opcode == 0xFC || raw_opcode == 0xFD) {
            WAH_ENSURE(*ptr < section_end, WAH_ERROR_UNEXPECTED_EOF);
            uint8_t ext = *(*ptr)++;
            opcode_val = (raw_opcode == 0xFD ? 0x108 : (raw_opcode == 0xFC ? 0xf6 : 0xd7)) + ext;
        } else {
            opcode_val = raw_opcode;
        }

        if (opcode_val == WAH_OP_END) {
            // End of const expression - check type matches expected
            WAH_ENSURE(stack_depth == 1, WAH_ERROR_VALIDATION_FAILED);
            WAH_ENSURE(type_stack[0] == expected_type || type_stack[0] == WAH_TYPE_ANY, WAH_ERROR_VALIDATION_FAILED);
            return WAH_OK;
        }

        // Validate based on opcode
        switch (opcode_val) {
            // Constants
            case WAH_OP_I32_CONST: {
                int32_t val;
                WAH_CHECK(wah_decode_sleb128_32(ptr, section_end, &val));
                CONST_PUSH(WAH_TYPE_I32);
                break;
            }
            case WAH_OP_I64_CONST: {
                int64_t val;
                WAH_CHECK(wah_decode_sleb128_64(ptr, section_end, &val));
                CONST_PUSH(WAH_TYPE_I64);
                break;
            }
            case WAH_OP_F32_CONST: {
                WAH_ENSURE(*ptr + 4 <= section_end, WAH_ERROR_UNEXPECTED_EOF);
                *ptr += 4;
                CONST_PUSH(WAH_TYPE_F32);
                break;
            }
            case WAH_OP_F64_CONST: {
                WAH_ENSURE(*ptr + 8 <= section_end, WAH_ERROR_UNEXPECTED_EOF);
                *ptr += 8;
                CONST_PUSH(WAH_TYPE_F64);
                break;
            }

            // Integer binary operations
            case WAH_OP_I32_ADD:
            case WAH_OP_I32_SUB:
            case WAH_OP_I32_MUL:
                CONST_POP(WAH_TYPE_I32);
                CONST_POP(WAH_TYPE_I32);
                CONST_PUSH(WAH_TYPE_I32);
                break;

            case WAH_OP_I64_ADD:
            case WAH_OP_I64_SUB:
            case WAH_OP_I64_MUL:
                CONST_POP(WAH_TYPE_I64);
                CONST_POP(WAH_TYPE_I64);
                CONST_PUSH(WAH_TYPE_I64);
                break;

            // Reference types
            case WAH_OP_REF_NULL: {
                wah_type_t ref_type;
                WAH_CHECK(wah_decode_ref_type(ptr, section_end, &ref_type));
                CONST_PUSH(ref_type);
                break;
            }
            case WAH_OP_REF_FUNC: {
                uint32_t func_idx;
                WAH_CHECK(wah_decode_uleb128(ptr, section_end, &func_idx));
                // Validate function index exists
                // During const expr validation (element section parsing), host functions
                // haven't been added yet, so only validate against local + imported functions
                WAH_ENSURE(func_idx < module->import_function_count + module->function_count,
                          WAH_ERROR_VALIDATION_FAILED);
                CONST_PUSH(WAH_TYPE_FUNCREF);
                break;
            }

            // Global get (only for globals defined earlier)
            case WAH_OP_GLOBAL_GET: {
                uint32_t global_idx;
                WAH_CHECK(wah_decode_uleb128(ptr, section_end, &global_idx));
                // Can only reference globals defined before the current one
                WAH_ENSURE(global_idx < max_global_idx, WAH_ERROR_VALIDATION_FAILED);
                WAH_ENSURE(global_idx < module->global_count, WAH_ERROR_VALIDATION_FAILED);
                CONST_PUSH(module->globals[global_idx].type);
                break;
            }

            // Future extensions: REF_I31, STRUCT_NEW, ARRAY_NEW, etc.
            // For now, reject all other opcodes
            default:
                return WAH_ERROR_VALIDATION_FAILED;
        }
    }

#undef CONST_PUSH
#undef CONST_POP
}

static wah_error_t wah_parse_global_section(const uint8_t **ptr, const uint8_t *section_end, wah_module_t *module) {
    uint32_t count;
    // A global entry requires at least 5 bytes (type, is_mutable, init_expr (min 3 bytes)).
    WAH_CHECK(wah_decode_and_validate_count(ptr, section_end, &count, 5));

    module->global_count = 0;
    WAH_MALLOC_ARRAY(module->globals, count);

    for (uint32_t i = 0; i < count; ++i) {
        module->globals[i] = (wah_global_t){0};
        ++module->global_count;

        wah_type_t global_declared_type;
        WAH_CHECK(wah_decode_val_type(ptr, section_end, &global_declared_type));
        module->globals[i].type = global_declared_type;

        // Mutability
        WAH_ENSURE(*ptr < section_end, WAH_ERROR_UNEXPECTED_EOF);
        module->globals[i].is_mutable = (*(*ptr)++ == 1);

        // Init Expr - store start position for validation and preparsing
        const uint8_t *expr_start = *ptr;

        // Step 1: Validate const expression
        WAH_CHECK(wah_validate_const_expr(ptr, section_end, global_declared_type, module, i));

        // Step 2: Preparse const expression using existing preparsing infrastructure
        // Note: ptr was moved by validation, so we need to calculate the size
        uint32_t expr_size = (uint32_t)(*ptr - expr_start);
        WAH_CHECK(wah_preparse_code(module, 0, expr_start, expr_size, &module->globals[i].init_expr));
    }
    return WAH_OK;
}

static wah_error_t wah_parse_memory_section(const uint8_t **ptr, const uint8_t *section_end, wah_module_t *module) {
    uint32_t count;
    // A memory entry requires at least 2 bytes (flags, min_pages_uleb128).
    WAH_CHECK(wah_decode_and_validate_count(ptr, section_end, &count, 2));

    module->memory_count = count;
    if (count > 0) {
        WAH_MALLOC_ARRAY(module->memories, count);

        for (uint32_t i = 0; i < count; ++i) {
            WAH_ENSURE(*ptr < section_end, WAH_ERROR_UNEXPECTED_EOF);
            uint8_t flags = *(*ptr)++; // Flags for memory type

            // Determine address type from flags
            if (flags == 0x00 || flags == 0x01) {
                module->memories[i].addr_type = WAH_TYPE_I32;
            } else if (flags == 0x04 || flags == 0x05) {
                module->memories[i].addr_type = WAH_TYPE_I64;
            } else {
                return WAH_ERROR_VALIDATION_FAILED; // Unknown memory type flags
            }

            if (flags == 0x04 || flags == 0x05) {
                WAH_CHECK(wah_decode_uleb128_u64(ptr, section_end, &module->memories[i].min_pages));
                if (flags & 0x01) {
                    WAH_CHECK(wah_decode_uleb128_u64(ptr, section_end, &module->memories[i].max_pages));
                } else {
                    module->memories[i].max_pages = UINT64_MAX;
                }
            } else {
                uint32_t min32, max32;
                WAH_CHECK(wah_decode_uleb128(ptr, section_end, &min32));
                module->memories[i].min_pages = min32;
                if (flags & 0x01) {
                    WAH_CHECK(wah_decode_uleb128(ptr, section_end, &max32));
                    module->memories[i].max_pages = max32;
                } else {
                    module->memories[i].max_pages = UINT64_MAX;
                }
            }
        }
    }
    return WAH_OK;
}

static wah_error_t wah_parse_table_section(const uint8_t **ptr, const uint8_t *section_end, wah_module_t *module) {
    uint32_t count;
    // A table entry requires at least 3 bytes (elem_type, flags, min_elements_uleb128).
    WAH_CHECK(wah_decode_and_validate_count(ptr, section_end, &count, 3));

    module->table_count = count;
    if (count > 0) {
        WAH_MALLOC_ARRAY(module->tables, count);

        for (uint32_t i = 0; i < count; ++i) {
            wah_type_t elem_type;
            WAH_CHECK(wah_decode_ref_type(ptr, section_end, &elem_type));
            module->tables[i].elem_type = elem_type;
            WAH_ENSURE(elem_type == WAH_TYPE_FUNCREF, WAH_ERROR_VALIDATION_FAILED); // Only funcref is supported for now

            WAH_ENSURE(*ptr < section_end, WAH_ERROR_UNEXPECTED_EOF);
            uint8_t flags = *(*ptr)++; // Flags for table type (0x00 for fixed, 0x01 for resizable)

            WAH_CHECK(wah_decode_uleb128(ptr, section_end, &module->tables[i].min_elements));

            if (flags & 0x01) { // has explicit max, read it
                WAH_CHECK(wah_decode_uleb128(ptr, section_end, &module->tables[i].max_elements));
            } else {
                module->tables[i].max_elements = UINT32_MAX; // no maximum
            }
        }
    }
    return WAH_OK;
}

// Placeholder for unimplemented sections
static wah_error_t wah_parse_unimplemented_section(const uint8_t **ptr, const uint8_t *section_end, wah_module_t *module) {
    (void)module; // Suppress unused parameter warning
    *ptr = section_end; // Skip the section
    return WAH_OK;
}

static wah_error_t wah_parse_custom_section(const uint8_t **ptr, const uint8_t *section_end, wah_module_t *module) {
    return wah_parse_unimplemented_section(ptr, section_end, module);
}

static wah_error_t wah_parse_import_section(const uint8_t **ptr, const uint8_t *section_end, wah_module_t *module) {
    wah_error_t err = WAH_OK;
    uint32_t count = 0;
    uint32_t import_func_count = 0;

    WAH_CHECK(wah_decode_and_validate_count(ptr, section_end, &count, 4));

    if (count == 0) return WAH_OK;

    WAH_MALLOC_ARRAY_GOTO(module->func_imports, count, cleanup);

    for (uint32_t i = 0; i < count; ++i) {
        // module_name
        uint32_t module_name_len;
        WAH_CHECK_GOTO(wah_decode_uleb128(ptr, section_end, &module_name_len), cleanup);
        WAH_ENSURE_GOTO(*ptr + module_name_len <= section_end, WAH_ERROR_UNEXPECTED_EOF, cleanup);
        char *module_name;
        WAH_MALLOC_ARRAY_GOTO(module_name, module_name_len + 1, cleanup);
        memcpy(module_name, *ptr, module_name_len);
        module_name[module_name_len] = '\0';
        *ptr += module_name_len;

        // field_name
        uint32_t field_name_len;
        WAH_CHECK_GOTO(wah_decode_uleb128(ptr, section_end, &field_name_len), cleanup);
        WAH_ENSURE_GOTO(*ptr + field_name_len <= section_end, WAH_ERROR_UNEXPECTED_EOF, cleanup);
        char *field_name;
        WAH_MALLOC_ARRAY_GOTO(field_name, field_name_len + 1, cleanup);
        memcpy(field_name, *ptr, field_name_len);
        field_name[field_name_len] = '\0';
        *ptr += field_name_len;

        // kind
        WAH_ENSURE_GOTO(*ptr < section_end, WAH_ERROR_UNEXPECTED_EOF, cleanup);
        uint8_t kind = *(*ptr)++;

        if (kind == 0) {
            // Function import
            uint32_t type_index;
            WAH_CHECK_GOTO(wah_decode_uleb128(ptr, section_end, &type_index), cleanup);
            WAH_ENSURE_GOTO(type_index < module->type_count, WAH_ERROR_VALIDATION_FAILED, cleanup);

            wah_func_import_t *fi = &module->func_imports[import_func_count];
            fi->module_name = module_name;
            fi->module_name_len = module_name_len;
            fi->field_name = field_name;
            fi->field_name_len = field_name_len;
            fi->type_index = type_index;
            import_func_count++;
        } else {
            // table/memory/global imports: skip for now
            free(module_name);
            free(field_name);
            err = WAH_ERROR_UNKNOWN_SECTION;
            WAH_LOG("Non-function import kind=%u not supported", kind);
            goto cleanup;
        }
    }

    module->import_function_count = import_func_count;
    return WAH_OK;

cleanup:
    if (module->func_imports) {
        for (uint32_t i = 0; i < import_func_count; i++) {
            free(module->func_imports[i].module_name);
            free(module->func_imports[i].field_name);
        }
        free(module->func_imports);
        module->func_imports = NULL;
    }
    module->import_function_count = 0;
    return err;
}

static wah_error_t wah_parse_export_section(const uint8_t **ptr, const uint8_t *section_end, wah_module_t *module) {
    wah_error_t err = WAH_OK;
    uint32_t count = 0;
    // An export entry requires at least 3 bytes (name_len, kind, index).
    WAH_CHECK_GOTO(wah_decode_and_validate_count(ptr, section_end, &count, 3), cleanup);

    module->export_count = 0; // Doubles as how many entries have been initialized (for cleanup)
    module->capacity_exports = count;  // For parsed modules, capacity equals count
    WAH_MALLOC_ARRAY_GOTO(module->exports, count, cleanup);

    for (uint32_t i = 0; i < count; ++i) {
        wah_export_t *export_entry = &module->exports[i];
        *export_entry = (wah_export_t){0};
        ++module->export_count;

        // Name length
        uint32_t name_len;
        WAH_CHECK_GOTO(wah_decode_uleb128(ptr, section_end, &name_len), cleanup);
        export_entry->name_len = name_len;

        // Name string
        WAH_ENSURE_GOTO(*ptr + name_len <= section_end, WAH_ERROR_UNEXPECTED_EOF, cleanup);

        // Allocate memory for the name and copy it, ensuring null-termination
        WAH_MALLOC_ARRAY_GOTO(export_entry->name, name_len + 1, cleanup);
        memcpy((void*)export_entry->name, *ptr, name_len);
        ((char*)export_entry->name)[name_len] = '\0';

        WAH_ENSURE_GOTO(wah_is_valid_utf8(export_entry->name, export_entry->name_len), WAH_ERROR_VALIDATION_FAILED, cleanup);

        // Check for duplicate export names
        for (uint32_t j = 0; j < i; ++j) {
            if (module->exports[j].name_len == export_entry->name_len &&
                strncmp(module->exports[j].name, export_entry->name, export_entry->name_len) == 0) {
                err = WAH_ERROR_VALIDATION_FAILED; // Duplicate export name
                goto cleanup;
            }
        }

        *ptr += name_len;

        // Export kind
        WAH_ENSURE_GOTO(*ptr < section_end, WAH_ERROR_UNEXPECTED_EOF, cleanup);
        export_entry->kind = *(*ptr)++;

        // Export index
        WAH_CHECK_GOTO(wah_decode_uleb128(ptr, section_end, &export_entry->index), cleanup);

        // Basic validation of index based on kind
        switch (export_entry->kind) {
            case 0: // Function
                WAH_ENSURE_GOTO(export_entry->index < module->function_count + module->import_function_count, WAH_ERROR_VALIDATION_FAILED, cleanup);
                break;
            case 1: // Table
                WAH_ENSURE_GOTO(export_entry->index < module->table_count, WAH_ERROR_VALIDATION_FAILED, cleanup);
                break;
            case 2: // Memory
                WAH_ENSURE_GOTO(export_entry->index < module->memory_count, WAH_ERROR_VALIDATION_FAILED, cleanup);
                break;
            case 3: // Global
                WAH_ENSURE_GOTO(export_entry->index < module->global_count, WAH_ERROR_VALIDATION_FAILED, cleanup);
                break;
            default:
                err = WAH_ERROR_VALIDATION_FAILED; // Unknown export kind
                goto cleanup;
        }
    }

cleanup:
    if (err != WAH_OK) {
        if (module->exports) {
            // Free names that were already allocated
            for (uint32_t k = 0; k < module->export_count; ++k) {
                if (module->exports[k].name) {
                    free((void*)module->exports[k].name);
                }
            }
            free(module->exports);
            module->exports = NULL;
            module->export_count = 0;
        }
    }
    return err;
}

static wah_error_t wah_parse_start_section(const uint8_t **ptr, const uint8_t *section_end, wah_module_t *module) {
    WAH_CHECK(wah_decode_uleb128(ptr, section_end, &module->start_function_idx));
    WAH_ENSURE(module->start_function_idx < module->function_count + module->import_function_count, WAH_ERROR_VALIDATION_FAILED);
    module->has_start_function = true;
    return WAH_OK;
}

static wah_error_t wah_parse_element_section(const uint8_t **ptr, const uint8_t *section_end, wah_module_t *module) {
    uint32_t count;
    // An element segment requires at least 1 byte (flags)
    WAH_CHECK(wah_decode_and_validate_count(ptr, section_end, &count, 1));

    module->element_segment_count = 0;
    if (count > 0) {
        WAH_MALLOC_ARRAY(module->element_segments, count);

        for (uint32_t i = 0; i < count; ++i) {
            wah_element_segment_t *segment = &module->element_segments[i];
            memset(segment, 0, sizeof(wah_element_segment_t));
            ++module->element_segment_count;

            uint32_t flags;
            WAH_CHECK(wah_decode_uleb128(ptr, section_end, &flags));

            uint32_t mode = flags & 3;
            bool is_expr_elem = (flags & 4) != 0;

            segment->is_active = (mode == 0 || mode == 2);
            segment->is_expr_elem = is_expr_elem;

            // Parse based on mode
            if (mode == 0) {
                // Active: table 0, offset_expr, elem* (funcref type implied)
                segment->table_idx = 0;
            } else if (mode == 1) {
                // Passive: elemkind/reftype, elem*
                segment->is_active = false;
            } else if (mode == 2) {
                // Active: tableidx, offset_expr, elemkind/reftype, elem*
                WAH_CHECK(wah_decode_uleb128(ptr, section_end, &segment->table_idx));
            } else { // mode == 3
                // Declarative: elemkind/reftype, elem* (dropped after validation)
                segment->is_dropped = true;
            }

            // For active modes, parse offset_expr
            if (segment->is_active) {
                const uint8_t *offset_start = *ptr;
                // Validate offset expression (must return i32)
                // TODO: When imported globals are supported, use module->global_count + module->import_global_count
                WAH_CHECK(wah_validate_const_expr(ptr, section_end, WAH_TYPE_I32, module, module->global_count));

                // Preparse offset expression for later evaluation
                uint32_t offset_expr_size = (uint32_t)(*ptr - offset_start);
                WAH_CHECK(wah_preparse_code(module, 0, offset_start, offset_expr_size, &segment->offset_expr));
            }

            // Parse elemkind/reftype
            // In mode 0, elemkind/reftype is implicitly funcref (not present in binary)
            // In modes 1,2,3, we need to read the elemkind/reftype byte
            if (mode > 0) {
                if (is_expr_elem) {
                    wah_type_t ref_type;
                    WAH_CHECK(wah_decode_ref_type(ptr, section_end, &ref_type));
                    WAH_ENSURE(ref_type == WAH_TYPE_FUNCREF, WAH_ERROR_VALIDATION_FAILED); // TODO: externref should be possible
                } else {
                    WAH_ENSURE(*ptr < section_end, WAH_ERROR_UNEXPECTED_EOF);
                    uint8_t elemkind = *(*ptr)++;
                    WAH_ENSURE(elemkind == 0x00, WAH_ERROR_VALIDATION_FAILED); // Only funcref supported
                }
            }

            // Parse num_elems
            uint32_t num_elems;
            WAH_CHECK(wah_decode_uleb128(ptr, section_end, &num_elems));

            // If declarative, validate and skip elem* and mark as dropped
            if (segment->is_dropped) {
                for (uint32_t j = 0; j < num_elems; ++j) {
                    if (is_expr_elem) {
                        // Validate expression (must return funcref)
                        // TODO: When imported globals are supported, use module->global_count + module->import_global_count
                        WAH_CHECK(wah_validate_const_expr(ptr, section_end, WAH_TYPE_FUNCREF, module, module->global_count));
                    } else {
                        // Skip funcidx
                        uint32_t funcidx;
                        WAH_CHECK(wah_decode_uleb128(ptr, section_end, &funcidx));
                        WAH_ENSURE(funcidx < module->function_count + module->import_function_count, WAH_ERROR_VALIDATION_FAILED);
                    }
                }
                continue; // Don't store elem data for declarative segments
            }

            // Parse elem* based on is_expr_elem
            segment->num_elems = 0;
            if (num_elems > 0) {
                if (!is_expr_elem) {
                    // elem* are function indices
                    WAH_MALLOC_ARRAY(segment->u.func_indices, num_elems);
                    for (uint32_t j = 0; j < num_elems; ++j) {
                        ++segment->num_elems;
                        WAH_CHECK(wah_decode_uleb128(ptr, section_end, &segment->u.func_indices[j]));
                        WAH_ENSURE(segment->u.func_indices[j] < module->function_count + module->import_function_count, WAH_ERROR_VALIDATION_FAILED);
                    }
                } else {
                    // elem* are constant expressions (must return funcref)
                    // Allocate arrays for each expression
                    WAH_MALLOC_ARRAY(segment->u.expr.bytecodes, num_elems);
                    WAH_MALLOC_ARRAY(segment->u.expr.bytecode_sizes, num_elems);

                    for (uint32_t j = 0; j < num_elems; ++j) {
                        segment->u.expr.bytecodes[j] = NULL; // Initialize to NULL for safe cleanup
                        ++segment->num_elems;

                        const uint8_t *expr_start = *ptr;
                        // Validate expression (must return funcref)
                        // TODO: When imported globals are supported, use module->global_count + module->import_global_count
                        WAH_CHECK(wah_validate_const_expr(ptr, section_end, WAH_TYPE_FUNCREF, module, module->global_count));

                        uint32_t raw_expr_size = (uint32_t)(*ptr - expr_start);

                        // Preparse the expression for efficient evaluation
                        wah_parsed_code_t parsed_expr;
                        WAH_CHECK(wah_preparse_code(module, 0, expr_start, raw_expr_size, &parsed_expr));

                        segment->u.expr.bytecode_sizes[j] = parsed_expr.bytecode_size;
                        segment->u.expr.bytecodes[j] = parsed_expr.bytecode; // Transfer ownership
                    }
                }
            }
        }
    }
    return WAH_OK;
}

static wah_error_t wah_parse_data_section(const uint8_t **ptr, const uint8_t *section_end, wah_module_t *module) {
    uint32_t count;
    // A data segment requires at least 2 bytes (flags, data_len_uleb128).
    WAH_CHECK(wah_decode_and_validate_count(ptr, section_end, &count, 2));

    // If a datacount section was present, validate that the data section count matches.
    if (module->has_data_count_section) {
        WAH_ENSURE(count == module->data_segment_count, WAH_ERROR_VALIDATION_FAILED);
    }

    if (count > 0) {
        WAH_MALLOC_ARRAY(module->data_segments, count);

        module->data_segment_count = 0; // Doubles as how many entries have been initialized (for cleanup)
        for (uint32_t i = 0; i < count; ++i) {
            wah_data_segment_t *segment = &module->data_segments[i];
            segment->data = NULL; // Initialize to NULL for safe cleanup
            ++module->data_segment_count;

            WAH_CHECK(wah_decode_uleb128(ptr, section_end, &segment->flags));

            if (segment->flags == 0x00) { // Active segment, memory index 0
                segment->memory_idx = 0;
            } else if (segment->flags == 0x01) { // Passive segment
                // No memory index or offset expression for passive segments
            } else if (segment->flags == 0x02) { // Active segment, with memory index
                WAH_CHECK(wah_decode_uleb128(ptr, section_end, &segment->memory_idx));
                WAH_ENSURE(segment->memory_idx < module->memory_count, WAH_ERROR_VALIDATION_FAILED);
            } else {
                return WAH_ERROR_VALIDATION_FAILED; // Unknown data segment flags
            }

            if (segment->flags == 0x00 || segment->flags == 0x02) { // Active segments have offset expression
                // Parse offset_expr (must match the memory's address type)
                WAH_ENSURE(segment->memory_idx < module->memory_count, WAH_ERROR_VALIDATION_FAILED);
                wah_type_t mem_addr_type = module->memories[segment->memory_idx].addr_type;

                WAH_ENSURE(*ptr < section_end, WAH_ERROR_UNEXPECTED_EOF);
                wah_opcode_t opcode = (wah_opcode_t)*(*ptr)++;

                if (opcode == WAH_OP_I32_CONST && mem_addr_type == WAH_TYPE_I32) {
                    int32_t offset_val;
                    WAH_CHECK(wah_decode_sleb128_32(ptr, section_end, &offset_val));
                    segment->offset = (uint32_t)offset_val;
                } else if (opcode == WAH_OP_I64_CONST && mem_addr_type == WAH_TYPE_I64) {
                    int64_t offset_val;
                    WAH_CHECK(wah_decode_sleb128_64(ptr, section_end, &offset_val));
                    segment->offset = (uint64_t)offset_val;
                } else {
                    return WAH_ERROR_VALIDATION_FAILED;
                }

                WAH_ENSURE(*ptr < section_end, WAH_ERROR_UNEXPECTED_EOF);
                WAH_ENSURE(*(*ptr)++ == WAH_OP_END, WAH_ERROR_VALIDATION_FAILED);
            }

            WAH_CHECK(wah_decode_uleb128(ptr, section_end, &segment->data_len));

            WAH_ENSURE(*ptr + segment->data_len <= section_end, WAH_ERROR_UNEXPECTED_EOF);
            if (segment->data_len > 0) {
                uint8_t *data_copy;
                WAH_MALLOC_ARRAY(data_copy, segment->data_len);
                memcpy(data_copy, *ptr, segment->data_len);
                segment->data = data_copy;
            }
            *ptr += segment->data_len;
        }
    }
    return WAH_OK;
}

static wah_error_t wah_parse_datacount_section(const uint8_t **ptr, const uint8_t *section_end, wah_module_t *module) {
    WAH_CHECK(wah_decode_uleb128(ptr, section_end, &module->data_segment_count));
    module->has_data_count_section = true;
    return WAH_OK;
}

// Pre-parsing function to convert bytecode to optimized structure
static wah_error_t wah_preparse_code(const wah_module_t* module, uint32_t func_idx, const uint8_t *code, uint32_t code_size, wah_parsed_code_t *parsed_code) {
    (void)module; (void)func_idx; // Suppress unused parameter warnings

    wah_error_t err = WAH_OK;
    *parsed_code = (wah_parsed_code_t){0};

    typedef struct { wah_opcode_t opcode; uint32_t target_idx; } wah_control_frame_t;

    uint32_t* block_targets = NULL;
    uint32_t block_target_count = 0, block_target_capacity = 0;
    wah_control_frame_t control_stack[WAH_MAX_CONTROL_DEPTH];
    uint32_t control_sp = 0;
    uint32_t preparsed_size = 0;

    const uint8_t *ptr = code, *end = code + code_size;

    // --- Pass 1: Find block boundaries and calculate preparsed size ---
    while (ptr < end) {
        const uint8_t* instr_ptr = ptr;
        uint16_t opcode;
        WAH_CHECK_GOTO(wah_decode_opcode(&ptr, end, &opcode), cleanup);

        uint32_t preparsed_instr_size = sizeof(uint16_t);

        #define GROW_BLOCK_TARGETS() do { \
                if (block_target_count >= block_target_capacity) { \
                    block_target_capacity = block_target_capacity == 0 ? 16 : block_target_capacity * 2; \
                    WAH_REALLOC_ARRAY_GOTO(block_targets, block_target_capacity, cleanup); \
                } \
            } while (0)

        switch (wah_opclasses[opcode]) {
            case WAH_OPCLASS_B: {
                ptr += 1;
                preparsed_instr_size += 1;
                break;
            }
            case WAH_OPCLASS_I: {
                uint32_t a;
                WAH_CHECK_GOTO(wah_decode_uleb128(&ptr, end, &a), cleanup);
                preparsed_instr_size += sizeof(uint32_t);
                break;
            }
            case WAH_OPCLASS_II: {
                uint32_t a, b;
                WAH_CHECK_GOTO(wah_decode_uleb128(&ptr, end, &a), cleanup);
                WAH_CHECK_GOTO(wah_decode_uleb128(&ptr, end, &b), cleanup);
                preparsed_instr_size += sizeof(uint32_t) * 2;
                break;
            }
            case WAH_OPCLASS_M: {
                uint32_t align, memidx;
                uint64_t offset;
                WAH_CHECK_GOTO(wah_decode_memarg(&ptr, end, &align, &memidx, &offset), cleanup);
                if (memidx != 0) preparsed_instr_size += sizeof(uint32_t); // memidx > 0
                preparsed_instr_size += sizeof(uint64_t); // offset
                break;
            }
            case WAH_OPCLASS_MB: {
                uint32_t align, memidx;
                uint64_t offset;
                WAH_CHECK_GOTO(wah_decode_memarg(&ptr, end, &align, &memidx, &offset), cleanup);
                ptr += 1; // lane index
                if (memidx != 0) preparsed_instr_size += sizeof(uint32_t); // memidx > 0
                preparsed_instr_size += sizeof(uint64_t) + 1; // offset + lane
                break;
            }
            default: switch (opcode) {
                case WAH_OP_BLOCK: case WAH_OP_LOOP: case WAH_OP_IF: {
                    int32_t block_type;
                    WAH_CHECK_GOTO(wah_decode_sleb128_32(&ptr, end, &block_type), cleanup);
                    GROW_BLOCK_TARGETS();
                    WAH_ASSERT(control_sp < WAH_MAX_CONTROL_DEPTH && "validation should have verified control stack size");
                    uint32_t target_idx = block_target_count++;
                    block_targets[target_idx] = preparsed_size; // To be overwritten for WAH_OP_IF
                    control_stack[control_sp++] = (wah_control_frame_t){.opcode=(wah_opcode_t)opcode, .target_idx=target_idx};
                    preparsed_instr_size = (opcode == WAH_OP_IF) ? sizeof(uint16_t) + sizeof(uint32_t) : 0;
                    break;
                }
                case WAH_OP_ELSE: {
                    WAH_ASSERT(control_sp > 0 && control_stack[control_sp - 1].opcode == WAH_OP_IF && "validation should have verified ELSE is inside IF");
                    preparsed_instr_size = sizeof(uint16_t) + sizeof(uint32_t);
                    block_targets[control_stack[control_sp - 1].target_idx] = preparsed_size + preparsed_instr_size;
                    GROW_BLOCK_TARGETS();
                    uint32_t target_idx = block_target_count++;
                    control_stack[control_sp - 1] = (wah_control_frame_t){.opcode=(wah_opcode_t)opcode, .target_idx=target_idx};
                    break;
                }
                case WAH_OP_END: {
                    if (control_sp > 0) {
                        wah_control_frame_t frame = control_stack[--control_sp];
                        if (frame.opcode != WAH_OP_LOOP) { // BLOCK, IF, ELSE
                            block_targets[frame.target_idx] = preparsed_size;
                        }
                        preparsed_instr_size = 0;
                    } else { // Final END
                        preparsed_instr_size = sizeof(uint16_t);
                    }
                    break;
                }
                case WAH_OP_BR: case WAH_OP_BR_IF: {
                    uint32_t d;
                    WAH_CHECK_GOTO(wah_decode_uleb128(&ptr, end, &d), cleanup);
                    preparsed_instr_size += sizeof(uint32_t);
                    break;
                }
                case WAH_OP_BR_TABLE: {
                    uint32_t num_targets;
                    WAH_CHECK_GOTO(wah_decode_uleb128(&ptr, end, &num_targets), cleanup);
                    preparsed_instr_size += sizeof(uint32_t) * (num_targets + 2);
                    for (uint32_t i = 0; i < num_targets + 1; ++i) {
                        uint32_t d;
                        WAH_CHECK_GOTO(wah_decode_uleb128(&ptr, end, &d), cleanup);
                    }
                    break;
                }
                case WAH_OP_I32_CONST: {
                    int32_t v;
                    WAH_CHECK_GOTO(wah_decode_sleb128_32(&ptr, end, &v), cleanup);
                    preparsed_instr_size += sizeof(int32_t);
                    break;
                }
                case WAH_OP_I64_CONST: {
                    int64_t v;
                    WAH_CHECK_GOTO(wah_decode_sleb128_64(&ptr, end, &v), cleanup);
                    preparsed_instr_size += sizeof(int64_t);
                    break;
                }
                case WAH_OP_F32_CONST: ptr += 4; preparsed_instr_size += 4; break;
                case WAH_OP_F64_CONST: ptr += 8; preparsed_instr_size += 8; break;
                case WAH_OP_V128_CONST: case WAH_OP_I8X16_SHUFFLE: ptr += 16; preparsed_instr_size += 16; break;
                case WAH_OP_REF_NULL: {
                    wah_type_t ref_type;
                    WAH_CHECK_GOTO(wah_decode_ref_type(&ptr, end, &ref_type), cleanup);
                    preparsed_instr_size += sizeof(uint32_t); // For type (converted to uint32_t)
                    break;
                }
                case WAH_OP_REF_FUNC: {
                    uint32_t ref_func_idx;
                    WAH_CHECK_GOTO(wah_decode_uleb128(&ptr, end, &ref_func_idx), cleanup);
                    preparsed_instr_size += sizeof(uint32_t); // For function index
                    break;
                }
            }
        }
        preparsed_size += preparsed_instr_size;
        ptr = instr_ptr + (ptr - instr_ptr); // This is not a bug; it's to make it clear that ptr is advanced inside the switch
    }
    WAH_ENSURE_GOTO(control_sp == 0, WAH_ERROR_VALIDATION_FAILED, cleanup);

    // --- Allocate and perform Pass 2: Generate preparsed bytecode ---
    WAH_MALLOC_ARRAY_GOTO(parsed_code->bytecode, preparsed_size, cleanup);
    parsed_code->bytecode_size = preparsed_size;

    ptr = code; control_sp = 0;
    uint8_t* write_ptr = parsed_code->bytecode;
    uint32_t current_block_idx = 0;

    #ifdef WAH_X86_64
    wah_x86_64_features_t features = wah_x86_64_features(); // TODO: cache across multiple calls
    #endif

    while (ptr < end) {
        uint16_t opcode;
        const uint8_t* saved_ptr = ptr;
        WAH_CHECK_GOTO(wah_decode_opcode(&ptr, end, &opcode), cleanup);

        if (opcode == WAH_OP_BLOCK || opcode == WAH_OP_LOOP) {
            int32_t block_type; WAH_CHECK_GOTO(wah_decode_sleb128_32(&ptr, end, &block_type), cleanup);
            WAH_ASSERT(control_sp < WAH_MAX_CONTROL_DEPTH && "validation should have verified control stack size");
            control_stack[control_sp++] = (wah_control_frame_t){.opcode=(wah_opcode_t)opcode, .target_idx=current_block_idx++};
            continue;
        }
        if (opcode == WAH_OP_END) {
            if (control_sp > 0) { control_sp--; continue; }
        }

        #ifdef WAH_X86_64
        uint16_t native_opcode = wah_x86_64_opcode(opcode, features);
        #else
        uint16_t native_opcode = opcode;
        #endif

        wah_write_u16_le(write_ptr, native_opcode);
        write_ptr += sizeof(uint16_t);

        switch (wah_opclasses[opcode]) {
            case WAH_OPCLASS_B: {
                *write_ptr++ = *ptr++;
                break;
            }
            case WAH_OPCLASS_I: {
                uint32_t a;
                WAH_CHECK_GOTO(wah_decode_uleb128(&ptr, end, &a), cleanup);
                // Redirect memory.size/memory.grow/memory.fill to i64 variants when needed
                if ((opcode == WAH_OP_MEMORY_SIZE || opcode == WAH_OP_MEMORY_GROW || opcode == WAH_OP_MEMORY_FILL)
                    && a < module->memory_count && module->memories[a].addr_type == WAH_TYPE_I64) {
                    uint16_t new_opcode;
                    if (opcode == WAH_OP_MEMORY_SIZE) new_opcode = WAH_OP_MEMORY_SIZE_i64;
                    else if (opcode == WAH_OP_MEMORY_GROW) new_opcode = WAH_OP_MEMORY_GROW_i64;
                    else new_opcode = WAH_OP_MEMORY_FILL_i64;
                    wah_write_u16_le(write_ptr - sizeof(uint16_t), new_opcode);
                }
                wah_write_u32_le(write_ptr, a);
                write_ptr += sizeof(uint32_t);
                break;
            }
            case WAH_OPCLASS_II: {
                uint32_t a, b;
                WAH_CHECK_GOTO(wah_decode_uleb128(&ptr, end, &a), cleanup);
                WAH_CHECK_GOTO(wah_decode_uleb128(&ptr, end, &b), cleanup);
                // Redirect memory.init/memory.copy to i64 variants when needed
                if (opcode == WAH_OP_MEMORY_INIT && b < module->memory_count && module->memories[b].addr_type == WAH_TYPE_I64) {
                    wah_write_u16_le(write_ptr - sizeof(uint16_t), WAH_OP_MEMORY_INIT_i64);
                }
                if (opcode == WAH_OP_MEMORY_COPY) {
                    if ((a < module->memory_count && module->memories[a].addr_type == WAH_TYPE_I64)
                        || (b < module->memory_count && module->memories[b].addr_type == WAH_TYPE_I64)) {
                        wah_write_u16_le(write_ptr - sizeof(uint16_t), WAH_OP_MEMORY_COPY_i64);
                    }
                }
                wah_write_u32_le(write_ptr, a);
                write_ptr += sizeof(uint32_t);
                wah_write_u32_le(write_ptr, b);
                write_ptr += sizeof(uint32_t);
                break;
            }
            case WAH_OPCLASS_M: {
                uint32_t align, memidx;
                uint64_t offset;
                WAH_CHECK_GOTO(wah_decode_memarg(&ptr, end, &align, &memidx, &offset), cleanup);
                bool is_i64_mem = module->memories[memidx].addr_type == WAH_TYPE_I64;
                if (memidx == 0 && !is_i64_mem) {
                    // Redirect to i32+mem0 fast-path opcode
                    switch (opcode) {
                        #define WAH_I32_MEM0_OPCODE_CASE(name, suffix) case WAH_OP_##name: opcode = WAH_OP_##name##_##suffix; break;
                        WAH_I32_MEM0_OPCODES_M(WAH_I32_MEM0_OPCODE_CASE)
                        #undef WAH_I32_MEM0_OPCODE_CASE
                    }
                    wah_write_u16_le(write_ptr - sizeof(uint16_t), opcode);
                } else if (memidx == 0 && is_i64_mem) {
                    // Redirect to i64+mem0 fast-path opcode
                    switch (opcode) {
                        #define WAH_I64_MEM0_OPCODE_CASE(name, suffix) case WAH_OP_##name: opcode = WAH_OP_##name##_##suffix; break;
                        WAH_I64_MEM0_OPCODES_M(WAH_I64_MEM0_OPCODE_CASE)
                        #undef WAH_I64_MEM0_OPCODE_CASE
                    }
                    wah_write_u16_le(write_ptr - sizeof(uint16_t), opcode);
                } else if (is_i64_mem) {
                    // Non-mem0 i64 memory: redirect to _i64 opcode
                    switch (opcode) {
                        #define WAH_I64_MEM_OPCODE_CASE(name, suffix) case WAH_OP_##name: opcode = WAH_OP_##name##_##suffix; break;
                        WAH_I64_MEM_OPCODES_M(WAH_I64_MEM_OPCODE_CASE)
                        #undef WAH_I64_MEM_OPCODE_CASE
                    }
                    wah_write_u16_le(write_ptr - sizeof(uint16_t), opcode);
                    wah_write_u32_le(write_ptr, memidx);
                    write_ptr += sizeof(uint32_t);
                } else {
                    // Non-mem0 i32 memory: base opcode is already correct
                    wah_write_u32_le(write_ptr, memidx);
                    write_ptr += sizeof(uint32_t);
                }
                wah_write_u64_le(write_ptr, offset);
                write_ptr += sizeof(uint64_t);
                break;
            }
            case WAH_OPCLASS_MB: {
                uint32_t align, memidx;
                uint64_t offset;
                WAH_CHECK_GOTO(wah_decode_memarg(&ptr, end, &align, &memidx, &offset), cleanup);
                uint8_t lane_idx = *ptr++;
                bool is_i64_mem = module->memories[memidx].addr_type == WAH_TYPE_I64;
                if (memidx == 0 && !is_i64_mem) {
                    switch (opcode) {
                        #define WAH_I32_MEM0_OPCODE_CASE(name, suffix) case WAH_OP_##name: opcode = WAH_OP_##name##_##suffix; break;
                        WAH_I32_MEM0_OPCODES_MB(WAH_I32_MEM0_OPCODE_CASE)
                        #undef WAH_I32_MEM0_OPCODE_CASE
                    }
                    wah_write_u16_le(write_ptr - sizeof(uint16_t), opcode);
                } else if (memidx == 0 && is_i64_mem) {
                    switch (opcode) {
                        #define WAH_I64_MEM0_OPCODE_CASE(name, suffix) case WAH_OP_##name: opcode = WAH_OP_##name##_##suffix; break;
                        WAH_I64_MEM0_OPCODES_MB(WAH_I64_MEM0_OPCODE_CASE)
                        #undef WAH_I64_MEM0_OPCODE_CASE
                    }
                    wah_write_u16_le(write_ptr - sizeof(uint16_t), opcode);
                } else if (is_i64_mem) {
                    switch (opcode) {
                        #define WAH_I64_MEM_OPCODE_CASE(name, suffix) case WAH_OP_##name: opcode = WAH_OP_##name##_##suffix; break;
                        WAH_I64_MEM_OPCODES_MB(WAH_I64_MEM_OPCODE_CASE)
                        #undef WAH_I64_MEM_OPCODE_CASE
                    }
                    wah_write_u16_le(write_ptr - sizeof(uint16_t), opcode);
                    wah_write_u32_le(write_ptr, memidx);
                    write_ptr += sizeof(uint32_t);
                } else {
                    wah_write_u32_le(write_ptr, memidx);
                    write_ptr += sizeof(uint32_t);
                }
                wah_write_u64_le(write_ptr, offset);
                write_ptr += sizeof(uint64_t);
                *write_ptr++ = lane_idx;
                break;
            }
            default: switch (opcode) {
                case WAH_OP_IF: {
                    int32_t block_type;
                    WAH_CHECK_GOTO(wah_decode_sleb128_32(&ptr, end, &block_type), cleanup);
                    WAH_ASSERT(control_sp < WAH_MAX_CONTROL_DEPTH && "validation should have verified control stack size");
                    control_stack[control_sp++] = (wah_control_frame_t){.opcode=WAH_OP_IF, .target_idx=current_block_idx};
                    wah_write_u32_le(write_ptr, block_targets[current_block_idx++]);
                    write_ptr += sizeof(uint32_t);
                    break;
                }
                case WAH_OP_ELSE: {
                    WAH_ASSERT(control_sp > 0 && control_stack[control_sp - 1].opcode == WAH_OP_IF && "validation should have verified ELSE is inside IF");
                    control_stack[control_sp - 1] = (wah_control_frame_t){.opcode=WAH_OP_ELSE, .target_idx=current_block_idx};
                    wah_write_u32_le(write_ptr, block_targets[current_block_idx++]);
                    write_ptr += sizeof(uint32_t);
                    break;
                }
                case WAH_OP_BR: case WAH_OP_BR_IF: {
                    uint32_t relative_depth;
                    WAH_CHECK_GOTO(wah_decode_uleb128(&ptr, end, &relative_depth), cleanup);
                    WAH_ASSERT(relative_depth < control_sp && "validation should have verified relative depth");
                    wah_control_frame_t* frame = &control_stack[control_sp - 1 - relative_depth];
                    wah_write_u32_le(write_ptr, block_targets[frame->target_idx]);
                    write_ptr += sizeof(uint32_t);
                    break;
                }
                case WAH_OP_BR_TABLE: {
                    uint32_t num_targets;
                    WAH_CHECK_GOTO(wah_decode_uleb128(&ptr, end, &num_targets), cleanup);
                    wah_write_u32_le(write_ptr, num_targets);
                    write_ptr += sizeof(uint32_t);
                    for (uint32_t i = 0; i < num_targets + 1; ++i) {
                        uint32_t relative_depth;
                        WAH_CHECK_GOTO(wah_decode_uleb128(&ptr, end, &relative_depth), cleanup);
                        WAH_ASSERT(relative_depth < control_sp && "validation should have verified relative depth");
                        wah_control_frame_t* frame = &control_stack[control_sp - 1 - relative_depth];
                        wah_write_u32_le(write_ptr, block_targets[frame->target_idx]);
                        write_ptr += sizeof(uint32_t);
                    }
                    break;
                }
                case WAH_OP_I32_CONST: {
                    int32_t v;
                    WAH_CHECK_GOTO(wah_decode_sleb128_32(&ptr, end, &v), cleanup);
                    wah_write_u32_le(write_ptr, (uint32_t)v);
                    write_ptr += sizeof(uint32_t);
                    break;
                }
                case WAH_OP_I64_CONST: {
                    int64_t v;
                    WAH_CHECK_GOTO(wah_decode_sleb128_64(&ptr, end, &v), cleanup);
                    wah_write_u64_le(write_ptr, (uint64_t)v);
                    write_ptr += sizeof(uint64_t);
                    break;
                }
                case WAH_OP_F32_CONST: memcpy(write_ptr, ptr, 4); ptr += 4; write_ptr += 4; break;
                case WAH_OP_F64_CONST: memcpy(write_ptr, ptr, 8); ptr += 8; write_ptr += 8; break;
                case WAH_OP_V128_CONST: case WAH_OP_I8X16_SHUFFLE: memcpy(write_ptr, ptr, 16); ptr += 16; write_ptr += 16; break;
                case WAH_OP_REF_NULL: {
                    wah_type_t ref_type;
                    WAH_CHECK_GOTO(wah_decode_ref_type(&ptr, end, &ref_type), cleanup);
                    // ref_type is already converted to WAH_TYPE_FUNCREF (-16) or WAH_TYPE_EXTERNREF (-17)
                    wah_write_u32_le(write_ptr, (uint32_t)ref_type);
                    write_ptr += sizeof(uint32_t);
                    break;
                }
                case WAH_OP_REF_FUNC: {
                    uint32_t ref_func_idx;
                    WAH_CHECK_GOTO(wah_decode_uleb128(&ptr, end, &ref_func_idx), cleanup);
                    wah_write_u32_le(write_ptr, ref_func_idx);
                    write_ptr += sizeof(uint32_t);
                    break;
                }
            }
        }
        ptr = saved_ptr + (ptr - saved_ptr);
    }

cleanup:
    free(block_targets);
    if (err != WAH_OK && parsed_code->bytecode) {
        free(parsed_code->bytecode);
        parsed_code->bytecode = NULL;
        parsed_code->bytecode_size = 0;
    }
    return err;
}

static void wah_free_parsed_code(wah_parsed_code_t *parsed_code) {
    if (!parsed_code) return;
    free(parsed_code->bytecode);
    parsed_code->bytecode = NULL;
    parsed_code->bytecode_size = 0;
}

static wah_error_t wah_decode_opcode(const uint8_t **ptr, const uint8_t *end, uint16_t *opcode_val) {
    WAH_ENSURE(*ptr < end, WAH_ERROR_UNEXPECTED_EOF);
    uint8_t first_byte = *(*ptr)++;
    uint32_t sub_opcode_val;

    switch (first_byte) {
        case 0xFB:
            WAH_CHECK(wah_decode_uleb128(ptr, end, &sub_opcode_val));
            WAH_ENSURE(sub_opcode_val < WAH_FC - WAH_FB, WAH_ERROR_VALIDATION_FAILED);
            *opcode_val = WAH_FB + (uint16_t)sub_opcode_val;
            break;
        case 0xFC:
            WAH_CHECK(wah_decode_uleb128(ptr, end, &sub_opcode_val));
            WAH_ENSURE(sub_opcode_val < WAH_FD - WAH_FC, WAH_ERROR_VALIDATION_FAILED);
            *opcode_val = WAH_FC + (uint16_t)sub_opcode_val;
            break;
        case 0xFD:
            WAH_CHECK(wah_decode_uleb128(ptr, end, &sub_opcode_val));
            WAH_ENSURE(sub_opcode_val < WAH_FE - WAH_FD, WAH_ERROR_VALIDATION_FAILED);
            *opcode_val = WAH_FD + (uint16_t)sub_opcode_val;
            break;
        default:
            WAH_ENSURE(first_byte < WAH_FB, WAH_ERROR_VALIDATION_FAILED);
            *opcode_val = (uint16_t)first_byte;
            break;
    }
    return WAH_OK;
}

// Helper function to decode a raw byte representing a value type into a wah_type_t
static wah_error_t wah_decode_val_type(const uint8_t **ptr, const uint8_t *end, wah_type_t *out_type) {
    WAH_ENSURE(*ptr < end, WAH_ERROR_UNEXPECTED_EOF);
    uint8_t byte = *(*ptr)++;
    switch (byte) {
        case 0x7F: *out_type = WAH_TYPE_I32; return WAH_OK;
        case 0x7E: *out_type = WAH_TYPE_I64; return WAH_OK;
        case 0x7D: *out_type = WAH_TYPE_F32; return WAH_OK;
        case 0x7C: *out_type = WAH_TYPE_F64; return WAH_OK;
        case 0x7B: *out_type = WAH_TYPE_V128; return WAH_OK;
        case 0x70: *out_type = WAH_TYPE_FUNCREF; return WAH_OK;
        case 0x6F: *out_type = WAH_TYPE_EXTERNREF; return WAH_OK;
        default: return WAH_ERROR_VALIDATION_FAILED; // Unknown value type
    }
}

// Helper function to decode a raw byte representing a reference type into a wah_type_t
static wah_error_t wah_decode_ref_type(const uint8_t **ptr, const uint8_t *end, wah_type_t *out_type) {
    WAH_ENSURE(*ptr < end, WAH_ERROR_UNEXPECTED_EOF);
    uint8_t byte = *(*ptr)++;
    switch (byte) {
        case 0x70: *out_type = WAH_TYPE_FUNCREF; return WAH_OK;
        case 0x6F: *out_type = WAH_TYPE_EXTERNREF; return WAH_OK;
        default: return WAH_ERROR_VALIDATION_FAILED; // Unknown value type
    }
}

// Global array of section handlers, indexed by the section ID
static const struct wah_section_handler_s {
    int8_t order; // Expected order of the section (0 for custom, 1 for Type, etc.)
    wah_error_t (*parser_func)(const uint8_t **ptr, const uint8_t *section_end, wah_module_t *module);
} wah_section_handlers[] = {
    [0]  = { .order = 0,  .parser_func = wah_parse_custom_section },
    [1]  = { .order = 1,  .parser_func = wah_parse_type_section },
    [2]  = { .order = 2,  .parser_func = wah_parse_import_section },
    [3]  = { .order = 3,  .parser_func = wah_parse_function_section },
    [4]  = { .order = 4,  .parser_func = wah_parse_table_section },
    [5]  = { .order = 5,  .parser_func = wah_parse_memory_section },
    [6]  = { .order = 6,  .parser_func = wah_parse_global_section },
    [7]  = { .order = 7,  .parser_func = wah_parse_export_section },
    [8]  = { .order = 8,  .parser_func = wah_parse_start_section },
    [9]  = { .order = 9,  .parser_func = wah_parse_element_section },
    [12] = { .order = 10, .parser_func = wah_parse_datacount_section },
    [10] = { .order = 11, .parser_func = wah_parse_code_section },
    [11] = { .order = 12, .parser_func = wah_parse_data_section },
};

wah_error_t wah_parse_module(const uint8_t *wasm_binary, size_t binary_size, wah_module_t *module) {
    wah_error_t err = WAH_OK;
    WAH_ENSURE(wasm_binary && module && binary_size >= 8, WAH_ERROR_UNEXPECTED_EOF);

    *module = (wah_module_t){0}; // Initialize module struct

    const uint8_t *ptr = wasm_binary;
    const uint8_t *end = wasm_binary + binary_size;

    // For section order validation
    int8_t last_parsed_order = 0; // Start with 0, as Type section is 1. Custom sections are 0 in map.

    // 1. Check Magic Number
    uint32_t magic = wah_read_u32_le(ptr);
    ptr += 4;
    WAH_ENSURE(magic == 0x6D736100, WAH_ERROR_INVALID_MAGIC_NUMBER);

    // 2. Check Version
    WAH_ENSURE(ptr + 4 <= end, WAH_ERROR_UNEXPECTED_EOF);
    uint32_t version = wah_read_u32_le(ptr);
    ptr += 4;
    WAH_ENSURE(version == 0x01, WAH_ERROR_INVALID_VERSION);

    // 3. Parse Sections
    while (ptr < end) {
        uint8_t section_id;
        uint32_t section_size;
        WAH_CHECK_GOTO(wah_read_section_header(&ptr, end, &section_id, &section_size), cleanup_parse);

        // Get section handler from lookup table
        WAH_ENSURE_GOTO(section_id < sizeof(wah_section_handlers) / sizeof(*wah_section_handlers), WAH_ERROR_UNKNOWN_SECTION, cleanup_parse);
        const struct wah_section_handler_s *handler = &wah_section_handlers[section_id];

        // Section order validation
        if (section_id != 0) { // Custom sections do not affect the order
            WAH_ENSURE_GOTO(handler->order > last_parsed_order, WAH_ERROR_VALIDATION_FAILED, cleanup_parse);
            last_parsed_order = handler->order;
        }

        const uint8_t *section_payload_end = ptr + section_size;
        WAH_ENSURE_GOTO(section_payload_end <= end, WAH_ERROR_UNEXPECTED_EOF, cleanup_parse);

        WAH_LOG("Parsing section ID: %d, size: %u", section_id, section_size);
        WAH_CHECK_GOTO(handler->parser_func(&ptr, section_payload_end, module), cleanup_parse);

        // Ensure we consumed exactly the section_size bytes
        WAH_ENSURE_GOTO(ptr == section_payload_end, WAH_ERROR_VALIDATION_FAILED, cleanup_parse); // Indicate a parsing error within the section
    }

    // After all sections are parsed, validate that function_count matches code_count
    WAH_ENSURE_GOTO(module->function_count == module->code_count, WAH_ERROR_VALIDATION_FAILED, cleanup_parse);

    // Validate data segment references
    WAH_ENSURE_GOTO(module->data_segment_count >= module->min_data_segment_count_required, WAH_ERROR_VALIDATION_FAILED, cleanup_parse);

    // If a data count section was present, ensure data segments were actually allocated.
    if (module->has_data_count_section && module->data_segment_count > 0) {
        WAH_ENSURE_GOTO(module->data_segments != NULL, WAH_ERROR_VALIDATION_FAILED, cleanup_parse);
    }

    // Build the unified functions[] array for the WASM functions.
    // Host functions may be appended later via wah_module_export_funcv.
    if (module->function_count > 0) {
        WAH_MALLOC_ARRAY_GOTO(module->functions, module->function_count, cleanup_parse);
        memset(module->functions, 0, module->function_count * sizeof(wah_function_t));
        // All entries have is_host == false (already set by memset).
        module->function_capacity = module->function_count;
    }
    module->total_function_count = module->function_count;

    return WAH_OK;

cleanup_parse:
    wah_free_module(module);
    return err;
}

// --- Interpreter Implementation ---

wah_error_t wah_exec_context_create(wah_exec_context_t *exec_ctx, const wah_module_t *module) {
    *exec_ctx = (wah_exec_context_t){0};
    wah_error_t err = WAH_OK;
    exec_ctx->is_instantiated = false;

    exec_ctx->value_stack_capacity = WAH_DEFAULT_VALUE_STACK_SIZE;
    WAH_MALLOC_ARRAY_GOTO(exec_ctx->value_stack, exec_ctx->value_stack_capacity, cleanup);

    exec_ctx->call_stack_capacity = WAH_DEFAULT_MAX_CALL_DEPTH;
    WAH_MALLOC_ARRAY_GOTO(exec_ctx->call_stack, exec_ctx->call_stack_capacity, cleanup);

    if (module->global_count > 0) {
        WAH_MALLOC_ARRAY_GOTO(exec_ctx->globals, module->global_count, cleanup);
        // Note: Globals are initialized later in wah_instantiate() by evaluating const expressions
        // Zero-initialize for now
        memset(exec_ctx->globals, 0, sizeof(wah_value_t) * module->global_count);
    }

    exec_ctx->module = module;
    exec_ctx->global_count = module->global_count;
    exec_ctx->max_call_depth = WAH_DEFAULT_MAX_CALL_DEPTH;
    exec_ctx->sp = 0;
    exec_ctx->call_depth = 0;

    if (module->memory_count > 0) {
        WAH_MALLOC_ARRAY_GOTO(exec_ctx->memories, module->memory_count, cleanup);
        WAH_MALLOC_ARRAY_GOTO(exec_ctx->memory_sizes, module->memory_count, cleanup);
        memset(exec_ctx->memories, 0, sizeof(uint8_t*) * module->memory_count);
        memset(exec_ctx->memory_sizes, 0, sizeof(uint64_t) * module->memory_count);
        exec_ctx->memory_count = module->memory_count;

        for (uint32_t i = 0; i < module->memory_count; ++i) {
            WAH_ENSURE_GOTO(module->memories[i].min_pages <= SIZE_MAX / WAH_WASM_PAGE_SIZE, WAH_ERROR_TOO_LARGE, cleanup);
            exec_ctx->memory_sizes[i] = module->memories[i].min_pages * (uint64_t)WAH_WASM_PAGE_SIZE;
            if (exec_ctx->memory_sizes[i] > 0) {
                WAH_MALLOC_ARRAY_GOTO(exec_ctx->memories[i], exec_ctx->memory_sizes[i], cleanup);
                memset(exec_ctx->memories[i], 0, exec_ctx->memory_sizes[i]);
            }
        }
        // Set memory_base/memory_size as fast-path aliases to memory 0
        exec_ctx->memory_base = exec_ctx->memories[0];
        exec_ctx->memory_size = exec_ctx->memory_sizes[0];
    }

    if (module->table_count > 0) {
        WAH_MALLOC_ARRAY_GOTO(exec_ctx->tables, module->table_count, cleanup);
        memset(exec_ctx->tables, 0, sizeof(wah_value_t*) * module->table_count);
        WAH_MALLOC_ARRAY_GOTO(exec_ctx->table_sizes, module->table_count, cleanup);
        memset(exec_ctx->table_sizes, 0, sizeof(uint32_t) * module->table_count);

        exec_ctx->table_count = module->table_count;
        for (uint32_t i = 0; i < exec_ctx->table_count; ++i) {
            uint32_t min_elements = module->tables[i].min_elements;
            exec_ctx->table_sizes[i] = min_elements;
            WAH_MALLOC_ARRAY_GOTO(exec_ctx->tables[i], min_elements, cleanup);
            memset(exec_ctx->tables[i], 0, sizeof(wah_value_t) * min_elements); // Initialize to null (0)
        }

    }

    // Build the runtime function_table (global index space: imports + locals + hosts).
    // Import slots are zero-initialized here; wah_instantiate() fills them in.
    {
        uint32_t import_count = module->import_function_count;
        uint32_t table_size = import_count + module->total_function_count;
        exec_ctx->function_table_count = table_size;
        if (table_size > 0) {
            WAH_MALLOC_ARRAY_GOTO(exec_ctx->function_table, table_size, cleanup);
            // Initialize import slots to NULL (host functions will be filled in later)
            if (import_count > 0) {
                memset(exec_ctx->function_table, 0, import_count * sizeof(wah_function_t));
            }
            // Copy local/host functions at offset import_count
            for (uint32_t i = 0; i < module->total_function_count; i++) {
                exec_ctx->function_table[import_count + i] = module->functions[i];
                if (!module->functions[i].is_host) {
                    exec_ctx->function_table[import_count + i].local_idx = i;
                    exec_ctx->function_table[import_count + i].fn_module = NULL; // = ctx->module
                }
            }
        }
    }

    return WAH_OK;

cleanup:
    if (err != WAH_OK) wah_exec_context_destroy(exec_ctx);
    return err;
}

void wah_exec_context_destroy(wah_exec_context_t *exec_ctx) {
    if (!exec_ctx) return;
    free(exec_ctx->value_stack);
    free(exec_ctx->call_stack);
    free(exec_ctx->globals);
    if (exec_ctx->memories) {
        for (uint32_t i = 0; i < exec_ctx->memory_count; ++i) {
            free(exec_ctx->memories[i]);
        }
        free(exec_ctx->memories);
    }
    free(exec_ctx->memory_sizes);
    free(exec_ctx->function_table);
    free(exec_ctx->table_sizes);
    if (exec_ctx->tables) {
        for (uint32_t i = 0; i < exec_ctx->table_count; ++i) {
            free(exec_ctx->tables[i]);
        }
        free(exec_ctx->tables);
    }

    // Free linked modules
    if (exec_ctx->linked_modules) {
        for (uint32_t i = 0; i < exec_ctx->linked_module_count; ++i) {
            free((void*)exec_ctx->linked_modules[i].name);
        }
        free(exec_ctx->linked_modules);
    }

    memset(exec_ctx, 0, sizeof(wah_exec_context_t));
}

// Pushes a new call frame. This is an internal helper.
// local_idx: index into fn_module->code_bodies[] for the function body.
// result_count: number of return values (stored in frame for RETURN/END).
static wah_error_t wah_push_frame(wah_exec_context_t *ctx, const wah_module_t *fn_module, uint32_t local_idx, uint32_t locals_offset, uint32_t result_count) {
    WAH_ENSURE(ctx->call_depth < ctx->max_call_depth, WAH_ERROR_CALL_STACK_OVERFLOW);

    const wah_code_body_t *code_body = &fn_module->code_bodies[local_idx];
    wah_call_frame_t *frame = &ctx->call_stack[ctx->call_depth++];

    frame->code = code_body;
    frame->bytecode_ip = code_body->parsed_code.bytecode;
    frame->locals_offset = locals_offset;
    frame->func_idx = local_idx;
    frame->result_count = result_count;
    frame->module = fn_module;

    // Calculate globals_offset for cross-module calls
    if (fn_module == ctx->module) {
        frame->globals_offset = 0;  // Primary module
    } else {
        // Find the module in linked_modules and calculate offset
        uint32_t offset = ctx->module->global_count;
        bool found = false;
        for (uint32_t i = 0; i < ctx->linked_module_count; i++) {
            if (ctx->linked_modules[i].module == fn_module) {
                frame->globals_offset = offset;
                found = true;
                break;
            }
            offset += ctx->linked_modules[i].module->global_count;
        }
        if (!found) {
            frame->globals_offset = 0;  // Fallback (shouldn't happen)
        }
    }

    return WAH_OK;
}

#define RELOAD_FRAME() \
    do { \
        if (ctx->call_depth == 0) goto cleanup; \
        frame = &ctx->call_stack[ctx->call_depth - 1]; \
        bytecode_ip = frame->bytecode_ip; \
        bytecode_base = frame->code->parsed_code.bytecode; \
    } while (0)

#ifdef WAH_FORCE_MUSTTAIL
    #define WAH_USE_MUSTTAIL
#elif WAH_HAS_ATTRIBUTE(musttail)
    #define WAH_USE_MUSTTAIL // clang 13+, GCC 15+
#endif

#ifdef WAH_FORCE_COMPUTED_GOTO
    #define WAH_USE_COMPUTED_GOTO
#elif defined(__GNUC__) || defined(__clang__)
    #define WAH_USE_COMPUTED_GOTO
#endif

// Enforce deterministic fp operations.
#if defined(__GNUC__) && !defined(__clang__)
    #pragma GCC push_options
    #pragma GCC optimize("-ffp-contract=off")
    #define WAH_PRAGMA_FP_CONTRACT_OFF()
#elif defined(_MSC_VER)
    #pragma float_control(push)
    #pragma fp_contract(off)
    #define WAH_PRAGMA_FP_CONTRACT_OFF()
#else // Clang etc.
    #define WAH_PRAGMA_FP_CONTRACT_OFF() _Pragma("STDC FP_CONTRACT OFF")
#endif

#if defined(WAH_USE_MUSTTAIL) // --- Tail recursion dispatch ---

#define WAH_RUN(opcode) \
    static wah_error_t wah_run_##opcode(wah_exec_context_t *ctx, wah_call_frame_t *frame, \
                                        const uint8_t *bytecode_ip, const uint8_t *bytecode_base, \
                                        wah_value_t *sp, wah_error_t err)

#define WAH_NEXT() do { \
        (void)bytecode_base; (void)err; \
        __attribute__((musttail)) return wah_run_single(ctx, frame, bytecode_ip, bytecode_base, sp, err); \
    } while (0)

#define WAH_CLEANUP() goto cleanup; cleanup: do { \
        /* Before returning, store the final IP and SP back into the (potentially last) frame */ \
        if (ctx->call_depth > 0) { \
            frame->bytecode_ip = bytecode_ip; \
        } \
        ctx->sp = (uint32_t)(sp - ctx->value_stack); \
        return (err); \
    } while (0)

static wah_error_t wah_run_single(wah_exec_context_t *ctx, wah_call_frame_t *frame,
                                  const uint8_t *bytecode_ip, const uint8_t *bytecode_base,
                                  wah_value_t *sp, wah_error_t err);

#elif defined(WAH_USE_COMPUTED_GOTO) // --- Computed GOTO dispatch ---

static wah_error_t wah_run_interpreter(wah_exec_context_t *ctx) {
    wah_error_t err = WAH_OK;

    // These are pointers to the current frame's state for faster access.
    wah_call_frame_t *frame = &ctx->call_stack[ctx->call_depth - 1];
    const uint8_t *bytecode_ip = frame->bytecode_ip;
    const uint8_t *bytecode_base = frame->code->parsed_code.bytecode;
    wah_value_t *sp = ctx->value_stack + ctx->sp;  // Stack pointer for faster access

    // Computed goto jump table
    static const void* wah_opcode_labels[] = {
#define WAH_OPCODE_LABEL(name, cls, val) [WAH_OP_##name] = &&wah_op_##name,
#define WAH_EXTRA_OPCODE_LABEL(name) [WAH_OP_##name] = &&wah_op_##name,
        WAH_OPCODES(WAH_OPCODE_LABEL)
        WAH_EXTRA_OPCODES(WAH_EXTRA_OPCODE_LABEL)
#undef WAH_OPCODE_LABEL
#undef WAH_EXTRA_OPCODE_LABEL
    };

    goto *wah_opcode_labels[wah_read_u16_le(bytecode_ip)];

    #define WAH_RUN(opcode) wah_op_##opcode: bytecode_ip += sizeof(uint16_t);
    #define WAH_NEXT() goto *wah_opcode_labels[wah_read_u16_le(bytecode_ip)]
    #define WAH_CLEANUP() goto cleanup

#else // --- Default switch-based interpreter ---

static wah_error_t wah_run_interpreter(wah_exec_context_t *ctx) {
    wah_error_t err = WAH_OK;

    // These are pointers to the current frame's state for faster access.
    wah_call_frame_t *frame = &ctx->call_stack[ctx->call_depth - 1];
    const uint8_t *bytecode_ip = frame->bytecode_ip;
    const uint8_t *bytecode_base = frame->code->parsed_code.bytecode;
    wah_value_t *sp = ctx->value_stack + ctx->sp;  // Stack pointer for faster access

    while (1) {
        uint16_t opcode = wah_read_u16_le(bytecode_ip);
        bytecode_ip += sizeof(uint16_t);

        switch (opcode) {

        #define WAH_RUN(opcode) break; case WAH_OP_##opcode:
        #define WAH_NEXT() break
        #define WAH_CLEANUP() goto cleanup

#endif

//------------------------------------------------------------------------------
WAH_RUN(BLOCK) { // Should not appear in preparsed code
    (void)bytecode_base;
    WAH_UNREACHABLE();
    WAH_NEXT();
}

WAH_RUN(LOOP) { // Should not appear in preparsed code
    (void)bytecode_base;
    WAH_UNREACHABLE();
    WAH_NEXT();
}

WAH_RUN(IF) {
    uint32_t offset = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);
    if ((*--sp).i32 == 0) {
        bytecode_ip = bytecode_base + offset;
    }
    WAH_NEXT();
}

WAH_RUN(ELSE) { // This is an unconditional jump
    uint32_t offset = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);
    bytecode_ip = bytecode_base + offset;
    WAH_NEXT();
}

WAH_RUN(BR) {
    uint32_t offset = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);
    bytecode_ip = bytecode_base + offset;
    WAH_NEXT();
}

WAH_RUN(BR_IF) {
    uint32_t offset = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);
    if ((*--sp).i32 != 0) {
        bytecode_ip = bytecode_base + offset;
    }
    WAH_NEXT();
}

WAH_RUN(BR_TABLE) {
    uint32_t index = (uint32_t)(*--sp).i32;
    uint32_t num_targets = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);

    uint32_t target_offset;
    if (index < num_targets) {
        // Jump to the specified target
        target_offset = wah_read_u32_le(bytecode_ip + index * sizeof(uint32_t));
    } else {
        // Jump to the default target (the last one in the list)
        target_offset = wah_read_u32_le(bytecode_ip + num_targets * sizeof(uint32_t));
    }
    bytecode_ip = bytecode_base + target_offset;
    WAH_NEXT();
}

WAH_RUN(I32_CONST) { (*sp++).i32 = (int32_t)wah_read_u32_le(bytecode_ip); bytecode_ip += sizeof(uint32_t); WAH_NEXT(); }
WAH_RUN(I64_CONST) { (*sp++).i64 = (int64_t)wah_read_u64_le(bytecode_ip); bytecode_ip += sizeof(uint64_t); WAH_NEXT(); }
WAH_RUN(F32_CONST) { (*sp++).f32 = wah_read_f32_le(bytecode_ip); bytecode_ip += sizeof(float); WAH_NEXT(); }
WAH_RUN(F64_CONST) { (*sp++).f64 = wah_read_f64_le(bytecode_ip); bytecode_ip += sizeof(double); WAH_NEXT(); }
WAH_RUN(V128_CONST) {
    memcpy(&(*sp++).v128, bytecode_ip, sizeof(wah_v128_t));
    bytecode_ip += sizeof(wah_v128_t);
    WAH_NEXT();
}

WAH_RUN(REF_NULL) {
    // Read type from bytecode (already parsed as uint32_t)
    uint32_t type = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);

    // All references are unified as void*, so null is always NULL
    (void)type; // Type is validated during parsing, but we don't need it here
    (*sp++).ref = NULL;
    WAH_NEXT();
}

WAH_RUN(REF_IS_NULL) {
    wah_value_t ref_val = *--sp;

    // Determine if reference is null based on type
    // For now, we'll check both funcref and externref
    // In a complete implementation, we'd need to track the actual type of each stack value
    int32_t is_null = (ref_val.ref == NULL);
    (*sp++).i32 = is_null;
    WAH_NEXT();
}

WAH_RUN(REF_FUNC) {
    // Read function index from bytecode (already parsed as uint32_t)
    uint32_t func_idx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);

    // Push function reference as pointer to wah_function_t
    // func_idx is the module-local function index (not including imports)
    WAH_ASSERT(func_idx < ctx->module->function_count && "validation should have verified function index");
    (*sp++).ref = &ctx->module->functions[func_idx];
    WAH_NEXT();
}

WAH_RUN(LOCAL_GET) {
    uint32_t local_idx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);
    *sp++ = ctx->value_stack[frame->locals_offset + local_idx];
    WAH_NEXT();
}

WAH_RUN(LOCAL_SET) {
    uint32_t local_idx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);
    ctx->value_stack[frame->locals_offset + local_idx] = *--sp;
    WAH_NEXT();
}

WAH_RUN(LOCAL_TEE) {
    uint32_t local_idx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);
    wah_value_t val = sp[-1];
    ctx->value_stack[frame->locals_offset + local_idx] = val;
    WAH_NEXT();
}

WAH_RUN(GLOBAL_GET) {
    uint32_t global_idx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);
    uint32_t effective_global_idx = frame->globals_offset + global_idx;
    *sp++ = ctx->globals[effective_global_idx];
    WAH_NEXT();
}

WAH_RUN(GLOBAL_SET) {
    uint32_t global_idx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);
    uint32_t effective_global_idx = frame->globals_offset + global_idx;
    ctx->globals[effective_global_idx] = *--sp;
    WAH_NEXT();
}

WAH_RUN(TABLE_GET) {
    uint32_t table_idx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);
    uint32_t elem_idx = (uint32_t)(*--sp).i32;
    WAH_ASSERT(table_idx < ctx->table_count && "validation didn't catch out-of-bound table index"); \
    WAH_ENSURE_GOTO(elem_idx < ctx->table_sizes[table_idx], WAH_ERROR_TRAP, cleanup);
    *sp++ = ctx->tables[table_idx][elem_idx];
    WAH_NEXT();
    WAH_CLEANUP();
}

WAH_RUN(TABLE_SET) {
    uint32_t table_idx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);
    wah_value_t val = *--sp;
    uint32_t elem_idx = (uint32_t)(*--sp).i32;
    WAH_ASSERT(table_idx < ctx->table_count && "validation didn't catch out-of-bound table index"); \
    WAH_ENSURE_GOTO(elem_idx < ctx->table_sizes[table_idx], WAH_ERROR_TRAP, cleanup);
    ctx->tables[table_idx][elem_idx] = val;
    WAH_NEXT();
    WAH_CLEANUP();
}

WAH_RUN(TABLE_SIZE) {
    uint32_t table_idx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);
    WAH_ASSERT(table_idx < ctx->table_count && "validation didn't catch out-of-bound table index"); \
    (*sp++).i32 = (int32_t)ctx->table_sizes[table_idx];
    WAH_NEXT();
}

WAH_RUN(TABLE_GROW) {
    uint32_t table_idx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);
    int32_t delta = (*--sp).i32;
    wah_value_t init_val = *--sp;
    WAH_ASSERT(table_idx < ctx->table_count && "validation didn't catch out-of-bound table index");

    if (delta < 0) {
        (*sp++).i32 = -1;
        WAH_NEXT();
    }

    uint32_t old_size = ctx->table_sizes[table_idx];
    uint64_t new_size = (uint64_t)old_size + delta;

    if (new_size > ctx->module->tables[table_idx].max_elements) {
        (*sp++).i32 = -1;
        WAH_NEXT();
    }

    // Reallocate table
    wah_value_t *new_table = NULL;
    WAH_MALLOC_ARRAY_GOTO(new_table, new_size, cleanup);
    memcpy(new_table, ctx->tables[table_idx], sizeof(wah_value_t) * old_size);
    for (uint64_t i = old_size; i < new_size; ++i) {
        new_table[i] = init_val;
    }
    free(ctx->tables[table_idx]);
    ctx->tables[table_idx] = new_table;
    ctx->table_sizes[table_idx] = (uint32_t)new_size;

    (*sp++).i32 = (int32_t)old_size;
    WAH_NEXT();
    WAH_CLEANUP();
}

WAH_RUN(TABLE_FILL) {
    uint32_t table_idx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);
    // Stack: [offset, value, size] -> [] (per formal spec: i32.const i val i32.const n table.fill x)
    uint32_t size = (uint32_t)(*--sp).i32;
    wah_value_t val = *--sp;
    uint32_t offset = (uint32_t)(*--sp).i32;
    WAH_ASSERT(table_idx < ctx->table_count && "validation didn't catch out-of-bound table index"); \
    WAH_ENSURE_GOTO((uint64_t)offset + size <= ctx->table_sizes[table_idx], WAH_ERROR_TRAP, cleanup);

    for (uint32_t i = 0; i < size; ++i) {
        ctx->tables[table_idx][offset + i] = val;
    }
    WAH_NEXT();
    WAH_CLEANUP();
}

WAH_RUN(TABLE_COPY) {
    uint32_t dst_table_idx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);
    uint32_t src_table_idx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);
    // Stack: [dst offset, src offset, size] -> [] (per formal spec: i32.const d i32.const s i32.const n table.copy x y)
    uint32_t size = (uint32_t)(*--sp).i32;
    uint32_t src_offset = (uint32_t)(*--sp).i32;
    uint32_t dst_offset = (uint32_t)(*--sp).i32;
    WAH_ASSERT(src_table_idx < ctx->table_count && "validation didn't catch out-of-bound table index"); \
    WAH_ASSERT(dst_table_idx < ctx->table_count && "validation didn't catch out-of-bound table index"); \
    WAH_ENSURE_GOTO((uint64_t)src_offset + size <= ctx->table_sizes[src_table_idx], WAH_ERROR_TRAP, cleanup);
    WAH_ENSURE_GOTO((uint64_t)dst_offset + size <= ctx->table_sizes[dst_table_idx], WAH_ERROR_TRAP, cleanup);

    if (src_table_idx == dst_table_idx && src_offset == dst_offset) {
        // Same table, same region - nothing to do
        WAH_NEXT();
    } else if (src_table_idx == dst_table_idx) {
        // Same table, potentially overlapping - use directional copy per spec
        if (dst_offset <= src_offset) {
            // Copy forward to avoid overwriting source (d <= s)
            for (uint32_t i = 0; i < size; ++i) {
                ctx->tables[dst_table_idx][dst_offset + i] = ctx->tables[src_table_idx][src_offset + i];
            }
        } else {
            // Copy backward to avoid overwriting source (d > s)
            for (uint32_t i = size; i > 0; --i) {
                ctx->tables[dst_table_idx][dst_offset + i - 1] = ctx->tables[src_table_idx][src_offset + i - 1];
            }
        }
    } else {
        // Different tables - simple memcpy
        memcpy(&ctx->tables[dst_table_idx][dst_offset], &ctx->tables[src_table_idx][src_offset], sizeof(wah_value_t) * size);
    }
    WAH_NEXT();
    WAH_CLEANUP();
}

WAH_RUN(TABLE_INIT) {
    uint32_t elem_idx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);
    uint32_t table_idx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);
    // Stack: [dst offset, src offset, size] -> []
    uint32_t size = (uint32_t)(*--sp).i32;
    uint32_t src_offset = (uint32_t)(*--sp).i32;
    uint32_t dst_offset = (uint32_t)(*--sp).i32;
    WAH_ASSERT(elem_idx < ctx->module->element_segment_count && "validation didn't catch out-of-bound element segment index");
    WAH_ASSERT(table_idx < ctx->table_count && "validation didn't catch out-of-bound table index");

    const wah_element_segment_t *segment = &ctx->module->element_segments[elem_idx];

    // Check if element segment has been dropped
    WAH_ENSURE_GOTO(!segment->is_dropped, WAH_ERROR_TRAP, cleanup);

    WAH_ENSURE_GOTO((uint64_t)src_offset + size <= segment->num_elems, WAH_ERROR_TRAP, cleanup);
    WAH_ENSURE_GOTO((uint64_t)dst_offset + size <= ctx->table_sizes[table_idx], WAH_ERROR_TRAP, cleanup);

    for (uint32_t i = 0; i < size; ++i) {
        if (!segment->is_expr_elem) {
            uint32_t global_func_idx = segment->u.func_indices[src_offset + i];
            WAH_ASSERT(global_func_idx < ctx->function_table_count && "validation didn't catch out-of-bound function index");
            ctx->tables[table_idx][dst_offset + i].ref = &ctx->function_table[global_func_idx];
        } else {
            wah_value_t elem_val;
            WAH_ENSURE_GOTO(src_offset + i < segment->num_elems, WAH_ERROR_TRAP, cleanup);
            WAH_CHECK_GOTO(wah_eval_const_expr(ctx,
                                               segment->u.expr.bytecodes[src_offset + i],
                                               segment->u.expr.bytecode_sizes[src_offset + i],
                                               &elem_val), cleanup);
            if (elem_val.ref == NULL) {
                ctx->tables[table_idx][dst_offset + i].ref = NULL;
            } else {
                uint32_t global_func_idx = elem_val.prefuncref.func_idx;
                WAH_ASSERT(global_func_idx < ctx->function_table_count && "validation didn't catch out-of-bound function index");
                ctx->tables[table_idx][dst_offset + i].ref = &ctx->function_table[global_func_idx];
            }
        }
    }
    WAH_NEXT();
    WAH_CLEANUP();
}

WAH_RUN(ELEM_DROP) {
    uint32_t elem_idx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);
    WAH_ASSERT(elem_idx < ctx->module->element_segment_count && "validation didn't catch out-of-bound element segment index");

    wah_element_segment_t *segment = &ctx->module->element_segments[elem_idx];

    // Free element data
    wah_free_element_segment_data(segment);

    // Mark as dropped
    segment->is_dropped = true;
    segment->num_elems = 0;
    WAH_NEXT();
}

WAH_RUN(CALL) {
    uint32_t called_func_idx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);

    // Dispatch via the runtime function_table (global index space)
    WAH_ASSERT(called_func_idx < ctx->function_table_count && "validation didn't catch out-of-bound function index");
    const wah_function_t *called_fn = &ctx->function_table[called_func_idx];

    if (called_fn->is_host) {
        // Host function path
        size_t nparams = called_fn->nparams;
        size_t nresults = called_fn->nresults;

        // Validate parameter count
        WAH_ASSERT((size_t)(sp - ctx->value_stack) >= nparams && "validation bug; is this even possible?");

        // Params are at [sp - nparams, sp). Place results ABOVE params so they don't overlap.
        wah_value_t *param_vals = sp - nparams;
        wah_value_t *result_vals = sp;

        // Reserve space for results above params
        WAH_ENSURE_GOTO((size_t)(result_vals + nresults - ctx->value_stack) <= ctx->value_stack_capacity, WAH_ERROR_CALL_STACK_OVERFLOW, cleanup);
        memset(result_vals, 0, sizeof(wah_value_t) * nresults);

        // Call host function: params and results are in distinct, non-overlapping regions
        WAH_CHECK_GOTO(wah_call_host_function_internal(ctx, called_fn, param_vals, (uint32_t)nparams, result_vals), cleanup);

        // Move results down to where params were (standard wasm calling convention)
        if (nresults > 0) {
            memmove(param_vals, result_vals, sizeof(wah_value_t) * nresults);
        }
        sp = param_vals + nresults;

        frame->bytecode_ip = bytecode_ip;
    } else {
        // WASM function call
        const wah_module_t *fn_module = called_fn->fn_module ? called_fn->fn_module : ctx->module;
        uint32_t local_idx = called_fn->local_idx;
        const wah_func_type_t *called_func_type = &fn_module->types[fn_module->function_type_indices[local_idx]];
        const wah_code_body_t *called_code = &fn_module->code_bodies[local_idx];

        uint32_t new_locals_offset = (uint32_t)(sp - ctx->value_stack) - called_func_type->param_count;

        frame->bytecode_ip = bytecode_ip;

        WAH_CHECK_GOTO(wah_push_frame(ctx, fn_module, local_idx, new_locals_offset, called_func_type->result_count), cleanup);

        uint32_t num_locals = called_code->local_count;
        if (num_locals > 0) {
            WAH_ENSURE_GOTO(sp + num_locals - ctx->value_stack <= ctx->value_stack_capacity, WAH_ERROR_CALL_STACK_OVERFLOW, cleanup);
            memset(sp, 0, sizeof(wah_value_t) * num_locals);
            sp += num_locals;
        }

        RELOAD_FRAME();
    }

    WAH_NEXT();
    WAH_CLEANUP();
}

WAH_RUN(CALL_INDIRECT) {
    uint32_t type_idx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);
    uint32_t table_idx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);

    // Pop function index from stack
    uint32_t func_table_idx = (uint32_t)(*--sp).i32;

    // Validate type_idx and table_idx
    WAH_ASSERT(type_idx < ctx->module->type_count && "validation didn't catch out-of-bound type index");
    WAH_ASSERT(table_idx < ctx->table_count && "validation didn't catch out-of-bound table index");

    WAH_ENSURE_GOTO(func_table_idx < ctx->table_sizes[table_idx], WAH_ERROR_TRAP, cleanup); // Function index out of table bounds

    // Get wah_function_t* from table and use it directly.
    // For functions initialized from element segments, table_fn == &ctx->function_table[global_idx],
    // so using table_fn directly is equivalent to the old global_idx indirection.
    // Using table_fn directly also supports linked-module functions not in ctx->function_table.
    const wah_function_t *actual_fn = (const wah_function_t *)ctx->tables[table_idx][func_table_idx].ref;
    WAH_ENSURE_GOTO(actual_fn != NULL, WAH_ERROR_TRAP, cleanup); // Null function reference
    WAH_ASSERT(actual_fn != wah_funcref_sentinel && "prefuncref stored in table without conversion to funcref");

    if (actual_fn->is_host) {
        // Host function path
        size_t nparams = actual_fn->nparams;
        size_t nresults = actual_fn->nresults;

        // Get expected function type (from instruction)
        const wah_func_type_t *expected_func_type = &ctx->module->types[type_idx];

        // Type check: compare expected and host function types
        WAH_ASSERT(expected_func_type->param_count == nparams && expected_func_type->result_count == nresults && "type mismatch (param/result count)");
        for (uint32_t i = 0; i < expected_func_type->param_count; ++i) {
            // Type mismatch (param type)
            WAH_ASSERT(expected_func_type->param_types[i] == actual_fn->param_types[i] && "type mismatch (param type)");
        }
        for (uint32_t i = 0; i < expected_func_type->result_count; ++i) {
            // Type mismatch (result type)
            WAH_ASSERT(expected_func_type->result_types[i] == actual_fn->result_types[i] && "type mismatch (result type)");
        }

        // Validate parameter count
        WAH_ASSERT((size_t)(sp - ctx->value_stack) >= nparams && "validation bug; is this even possible?");

        // Params are at [sp - nparams, sp). Place results ABOVE params so they don't overlap.
        wah_value_t *param_vals = sp - nparams;
        wah_value_t *result_vals = sp;

        // Reserve space for results above params
        WAH_ENSURE_GOTO((size_t)(result_vals + nresults - ctx->value_stack) <= ctx->value_stack_capacity, WAH_ERROR_CALL_STACK_OVERFLOW, cleanup);
        memset(result_vals, 0, sizeof(wah_value_t) * nresults);

        // Call host function: params and results are in distinct, non-overlapping regions
        WAH_CHECK_GOTO(wah_call_host_function_internal(ctx, actual_fn, param_vals, (uint32_t)nparams, result_vals), cleanup);

        // Move results down to where params were
        if (nresults > 0) {
            memmove(param_vals, result_vals, sizeof(wah_value_t) * nresults);
        }
        sp = param_vals + nresults;

        frame->bytecode_ip = bytecode_ip;
    } else {
        // WASM function call
        const wah_module_t *fn_module = actual_fn->fn_module ? actual_fn->fn_module : ctx->module;
        uint32_t local_idx = actual_fn->local_idx;

        // Get expected function type (from instruction)
        const wah_func_type_t *expected_func_type = &ctx->module->types[type_idx];
        // Get actual function type (from local_idx)
        const wah_func_type_t *actual_func_type = &fn_module->types[fn_module->function_type_indices[local_idx]];

        // Type check: compare expected and actual function types
        WAH_ENSURE_GOTO(expected_func_type->param_count == actual_func_type->param_count &&
                        expected_func_type->result_count == actual_func_type->result_count,
                        WAH_ERROR_TRAP, cleanup); // Type mismatch (param/result count)
        for (uint32_t i = 0; i < expected_func_type->param_count; ++i) {
            // Type mismatch (param type)
            WAH_ENSURE_GOTO(expected_func_type->param_types[i] == actual_func_type->param_types[i], WAH_ERROR_TRAP, cleanup);
        }
        for (uint32_t i = 0; i < expected_func_type->result_count; ++i) {
            // Type mismatch (result type)
            WAH_ENSURE_GOTO(expected_func_type->result_types[i] == actual_func_type->result_types[i], WAH_ERROR_TRAP, cleanup);
        }
        // Perform the call using local_idx
        const wah_code_body_t *called_code = &fn_module->code_bodies[local_idx];
        uint32_t new_locals_offset = (uint32_t)(sp - ctx->value_stack) - expected_func_type->param_count;

        frame->bytecode_ip = bytecode_ip;

        WAH_CHECK_GOTO(wah_push_frame(ctx, fn_module, local_idx, new_locals_offset, actual_func_type->result_count), cleanup);

        uint32_t num_locals = called_code->local_count;
        if (num_locals > 0) {
            WAH_ENSURE_GOTO(sp + num_locals - ctx->value_stack <= ctx->value_stack_capacity, WAH_ERROR_CALL_STACK_OVERFLOW, cleanup);
            memset(sp, 0, sizeof(wah_value_t) * num_locals);
            sp += num_locals;
        }

        RELOAD_FRAME();
    }

    WAH_NEXT();
    WAH_CLEANUP();
}

WAH_RUN(RETURN) {
    uint32_t results_to_keep = frame->result_count;
    wah_value_t *results_src = sp - results_to_keep;

    sp = ctx->value_stack + frame->locals_offset;
    ctx->call_depth--;

    if (results_to_keep > 0) {
        memmove(sp, results_src, sizeof(wah_value_t) * results_to_keep);
        sp += results_to_keep;
    }

    RELOAD_FRAME();
    WAH_NEXT();
    WAH_CLEANUP();
}

WAH_RUN(END) { // End of function
    uint32_t results_to_keep = frame->result_count;
    wah_value_t *results_src = sp - results_to_keep;

    sp = ctx->value_stack + frame->locals_offset;
    ctx->call_depth--;

    if (results_to_keep > 0) {
        memmove(sp, results_src, sizeof(wah_value_t) * results_to_keep);
        sp += results_to_keep;
    }

    RELOAD_FRAME();
    WAH_NEXT();
    WAH_CLEANUP();
}

#define BINOP_I(N,op) { sp[-2].i##N = (int##N##_t)((uint##N##_t)sp[-2].i##N op (uint##N##_t)sp[-1].i##N); sp--; WAH_NEXT(); }
#define CMP_I_S(N,op) { sp[-2].i32 = sp[-2].i##N op sp[-1].i##N ? 1 : 0; sp--; WAH_NEXT(); }
#define CMP_I_U(N,op) { sp[-2].i32 = (uint##N##_t)sp[-2].i##N op (uint##N##_t)sp[-1].i##N ? 1 : 0; sp--; WAH_NEXT(); }
#define BINOP_F(N,op) { sp[-2].f##N = wah_canonicalize_f##N(sp[-2].f##N op sp[-1].f##N); sp--; WAH_NEXT(); }
#define CMP_F(N,op)   { sp[-2].i32 = sp[-2].f##N op sp[-1].f##N ? 1 : 0; sp--; WAH_NEXT(); }
#define UNOP_I_FN(N,fn)  { sp[-1].i##N = (int##N##_t)fn((uint##N##_t)sp[-1].i##N); WAH_NEXT(); }
#define BINOP_I_FN(N,fn) { sp[-2].i##N = (int##N##_t)fn((uint##N##_t)sp[-2].i##N, (uint##N##_t)sp[-1].i##N); sp--; WAH_NEXT(); }
#define UNOP_F_FN(N,fn)  { sp[-1].f##N = wah_canonicalize_f##N(fn(sp[-1].f##N)); WAH_NEXT(); }
#define BINOP_F_FN(N,fn) { sp[-2].f##N = wah_canonicalize_f##N(fn(sp[-2].f##N, sp[-1].f##N)); sp--; WAH_NEXT(); }

#define NUM_OPS(N,_F) \
    WAH_RUN(I##N##_CLZ) UNOP_I_FN(N, wah_clz_u##N) \
    WAH_RUN(I##N##_CTZ) UNOP_I_FN(N, wah_ctz_u##N) \
    WAH_RUN(I##N##_POPCNT) UNOP_I_FN(N, wah_popcount_u##N) \
    WAH_RUN(I##N##_ADD) BINOP_I(N,+) \
    WAH_RUN(I##N##_SUB) BINOP_I(N,-) \
    WAH_RUN(I##N##_MUL) BINOP_I(N,*) \
    WAH_RUN(I##N##_DIV_S) {  \
        WAH_ENSURE_GOTO(sp[-1].i##N != 0, WAH_ERROR_TRAP, cleanup); \
        WAH_ENSURE_GOTO(sp[-2].i##N != INT##N##_MIN || sp[-1].i##N != -1, WAH_ERROR_TRAP, cleanup); \
        sp[-2].i##N /= sp[-1].i##N; sp--; WAH_NEXT(); \
        WAH_CLEANUP(); \
    } \
    WAH_RUN(I##N##_DIV_U) {  \
        WAH_ENSURE_GOTO(sp[-1].i##N != 0, WAH_ERROR_TRAP, cleanup); \
        sp[-2].i##N = (int##N##_t)((uint##N##_t)sp[-2].i##N / (uint##N##_t)sp[-1].i##N); \
        sp--; \
        WAH_NEXT(); \
        WAH_CLEANUP(); \
    } \
    WAH_RUN(I##N##_REM_S) {  \
        WAH_ENSURE_GOTO(sp[-1].i##N != 0, WAH_ERROR_TRAP, cleanup); \
        if (sp[-2].i##N == INT##N##_MIN && sp[-1].i##N == -1) sp[-2].i##N = 0; else sp[-2].i##N %= sp[-1].i##N; \
        sp--; \
        WAH_NEXT(); \
        WAH_CLEANUP(); \
    } \
    WAH_RUN(I##N##_REM_U) {  \
        WAH_ENSURE_GOTO(sp[-1].i##N != 0, WAH_ERROR_TRAP, cleanup); \
        sp[-2].i##N = (int##N##_t)((uint##N##_t)sp[-2].i##N % (uint##N##_t)sp[-1].i##N); \
        sp--; \
        WAH_NEXT(); \
        WAH_CLEANUP(); \
    } \
    WAH_RUN(I##N##_AND) BINOP_I(N,&) \
    WAH_RUN(I##N##_OR)  BINOP_I(N,|) \
    WAH_RUN(I##N##_XOR) BINOP_I(N,^) \
    WAH_RUN(I##N##_SHL) { sp[-2].i##N = (int##N##_t)((uint##N##_t)sp[-2].i##N << (sp[-1].i##N & (N-1))); sp--; WAH_NEXT(); } \
    WAH_RUN(I##N##_SHR_S) { sp[-2].i##N >>= (sp[-1].i##N & (N-1)); sp--; WAH_NEXT(); } \
    WAH_RUN(I##N##_SHR_U) { sp[-2].i##N = (int##N##_t)((uint##N##_t)sp[-2].i##N >> (sp[-1].i##N & (N-1))); sp--; WAH_NEXT(); } \
    WAH_RUN(I##N##_ROTL) BINOP_I_FN(N, wah_rotl_u##N) \
    WAH_RUN(I##N##_ROTR) BINOP_I_FN(N, wah_rotr_u##N) \
    \
    WAH_RUN(I##N##_EQ)   CMP_I_S(N,==) \
    WAH_RUN(I##N##_NE)   CMP_I_S(N,!=) \
    WAH_RUN(I##N##_LT_S) CMP_I_S(N,<) \
    WAH_RUN(I##N##_LT_U) CMP_I_U(N,<) \
    WAH_RUN(I##N##_GT_S) CMP_I_S(N,>) \
    WAH_RUN(I##N##_GT_U) CMP_I_U(N,>) \
    WAH_RUN(I##N##_LE_S) CMP_I_S(N,<=) \
    WAH_RUN(I##N##_LE_U) CMP_I_U(N,<=) \
    WAH_RUN(I##N##_GE_S) CMP_I_S(N,>=) \
    WAH_RUN(I##N##_GE_U) CMP_I_U(N,>=) \
    WAH_RUN(I##N##_EQZ) { sp[-1].i32 = (sp[-1].i##N == 0) ? 1 : 0; WAH_NEXT(); } \
    \
    WAH_RUN(F##N##_ABS) UNOP_F_FN(N, fabs##_F) \
    WAH_RUN(F##N##_NEG) UNOP_F_FN(N, -) \
    WAH_RUN(F##N##_CEIL) UNOP_F_FN(N, ceil##_F) \
    WAH_RUN(F##N##_FLOOR) UNOP_F_FN(N, floor##_F) \
    WAH_RUN(F##N##_TRUNC) UNOP_F_FN(N, trunc##_F) \
    WAH_RUN(F##N##_NEAREST) UNOP_F_FN(N, wah_nearest_f##N) \
    WAH_RUN(F##N##_SQRT) UNOP_F_FN(N, sqrt##_F) \
    WAH_RUN(F##N##_ADD) BINOP_F(N,+) \
    WAH_RUN(F##N##_SUB) BINOP_F(N,-) \
    WAH_RUN(F##N##_MUL) BINOP_F(N,*) \
    WAH_RUN(F##N##_DIV) BINOP_F(N,/) /* Let hardware handle division by zero (NaN/inf) */ \
    WAH_RUN(F##N##_EQ) CMP_F(N,==) \
    WAH_RUN(F##N##_NE) CMP_F(N,!=) \
    WAH_RUN(F##N##_LT) CMP_F(N,<) \
    WAH_RUN(F##N##_GT) CMP_F(N,>) \
    WAH_RUN(F##N##_LE) CMP_F(N,<=) \
    WAH_RUN(F##N##_GE) CMP_F(N,>=) \
    WAH_RUN(F##N##_MIN) BINOP_F_FN(N, fmin##_F) \
    WAH_RUN(F##N##_MAX) BINOP_F_FN(N, fmax##_F) \
    WAH_RUN(F##N##_COPYSIGN) BINOP_F_FN(N, copysign##_F)

#define LOAD_OP(N, T, value_field, cast) { \
    uint32_t memidx = wah_read_u32_le(bytecode_ip); \
    bytecode_ip += sizeof(uint32_t); \
    uint64_t offset = wah_read_u64_le(bytecode_ip); \
    bytecode_ip += sizeof(uint64_t); \
    uint32_t addr = (uint32_t)(*--sp).i32; \
    uint64_t effective_addr; \
    \
    WAH_ASSERT(memidx < ctx->memory_count && "validation didn't catch out-of-bound memory index"); \
    WAH_CHECK_GOTO(wah_check_effective_addr(addr, offset, N/8, ctx->memory_sizes[memidx], &effective_addr), cleanup); \
    (*sp++).value_field = cast wah_read_##T##_le(ctx->memories[memidx] + effective_addr); \
    WAH_NEXT(); \
    WAH_CLEANUP(); \
}

#define LOAD_OP_I32_MEM0(N, T, value_field, cast) { \
    uint64_t offset = wah_read_u64_le(bytecode_ip); \
    bytecode_ip += sizeof(uint64_t); \
    uint32_t addr = (uint32_t)(*--sp).i32; \
    uint64_t effective_addr; \
    \
    WAH_CHECK_GOTO(wah_check_effective_addr(addr, offset, N/8, ctx->memory_size, &effective_addr), cleanup); \
    (*sp++).value_field = cast wah_read_##T##_le(ctx->memory_base + effective_addr); \
    WAH_NEXT(); \
    WAH_CLEANUP(); \
}

#define STORE_OP(N, T, value_field, value_type, cast) { \
    uint32_t memidx = wah_read_u32_le(bytecode_ip); \
    bytecode_ip += sizeof(uint32_t); \
    uint64_t offset = wah_read_u64_le(bytecode_ip); \
    bytecode_ip += sizeof(uint64_t); \
    value_type val = (*--sp).value_field; \
    uint32_t addr = (uint32_t)(*--sp).i32; \
    uint64_t effective_addr; \
    \
    WAH_ASSERT(memidx < ctx->memory_count && "validation didn't catch out-of-bound memory index"); \
    WAH_CHECK_GOTO(wah_check_effective_addr(addr, offset, N/8, ctx->memory_sizes[memidx], &effective_addr), cleanup); \
    wah_write_##T##_le(ctx->memories[memidx] + effective_addr, cast (val)); \
    WAH_NEXT(); \
    WAH_CLEANUP(); \
}

#define STORE_OP_I32_MEM0(N, T, value_field, value_type, cast) { \
    uint64_t offset = wah_read_u64_le(bytecode_ip); \
    bytecode_ip += sizeof(uint64_t); \
    value_type val = (*--sp).value_field; \
    uint32_t addr = (uint32_t)(*--sp).i32; \
    uint64_t effective_addr; \
    \
    WAH_CHECK_GOTO(wah_check_effective_addr(addr, offset, N/8, ctx->memory_size, &effective_addr), cleanup); \
    wah_write_##T##_le(ctx->memory_base + effective_addr, cast (val)); \
    WAH_NEXT(); \
    WAH_CLEANUP(); \
}

// i64-addressed memory: non-mem0 (reads i64 address from stack)
#define LOAD_OP_I64(N, T, value_field, cast) { \
    uint32_t memidx = wah_read_u32_le(bytecode_ip); \
    bytecode_ip += sizeof(uint32_t); \
    uint64_t offset = wah_read_u64_le(bytecode_ip); \
    bytecode_ip += sizeof(uint64_t); \
    uint64_t addr = (uint64_t)(*--sp).i64; \
    uint64_t effective_addr; \
    \
    WAH_ASSERT(memidx < ctx->memory_count && "validation didn't catch out-of-bound memory index"); \
    WAH_CHECK_GOTO(wah_check_effective_addr(addr, offset, N/8, ctx->memory_sizes[memidx], &effective_addr), cleanup); \
    (*sp++).value_field = cast wah_read_##T##_le(ctx->memories[memidx] + effective_addr); \
    WAH_NEXT(); \
    WAH_CLEANUP(); \
}

#define STORE_OP_I64(N, T, value_field, value_type, cast) { \
    uint32_t memidx = wah_read_u32_le(bytecode_ip); \
    bytecode_ip += sizeof(uint32_t); \
    uint64_t offset = wah_read_u64_le(bytecode_ip); \
    bytecode_ip += sizeof(uint64_t); \
    value_type val = (*--sp).value_field; \
    uint64_t addr = (uint64_t)(*--sp).i64; \
    uint64_t effective_addr; \
    \
    WAH_ASSERT(memidx < ctx->memory_count && "validation didn't catch out-of-bound memory index"); \
    WAH_CHECK_GOTO(wah_check_effective_addr(addr, offset, N/8, ctx->memory_sizes[memidx], &effective_addr), cleanup); \
    wah_write_##T##_le(ctx->memories[memidx] + effective_addr, cast (val)); \
    WAH_NEXT(); \
    WAH_CLEANUP(); \
}

// i64-addressed memory: mem0 fast-path (reads i64 address from stack)
#define LOAD_OP_I64_MEM0(N, T, value_field, cast) { \
    uint64_t offset = wah_read_u64_le(bytecode_ip); \
    bytecode_ip += sizeof(uint64_t); \
    uint64_t addr = (uint64_t)(*--sp).i64; \
    uint64_t effective_addr; \
    \
    WAH_CHECK_GOTO(wah_check_effective_addr(addr, offset, N/8, ctx->memory_size, &effective_addr), cleanup); \
    (*sp++).value_field = cast wah_read_##T##_le(ctx->memory_base + effective_addr); \
    WAH_NEXT(); \
    WAH_CLEANUP(); \
}

#define STORE_OP_I64_MEM0(N, T, value_field, value_type, cast) { \
    uint64_t offset = wah_read_u64_le(bytecode_ip); \
    bytecode_ip += sizeof(uint64_t); \
    value_type val = (*--sp).value_field; \
    uint64_t addr = (uint64_t)(*--sp).i64; \
    uint64_t effective_addr; \
    \
    WAH_CHECK_GOTO(wah_check_effective_addr(addr, offset, N/8, ctx->memory_size, &effective_addr), cleanup); \
    wah_write_##T##_le(ctx->memory_base + effective_addr, cast (val)); \
    WAH_NEXT(); \
    WAH_CLEANUP(); \
}

#define CONVERT(from_field, cast, to_field) { \
    sp[-1].to_field = cast (sp[-1].from_field); \
    WAH_NEXT(); \
}
#define CONVERT_CHECK(from_field, call, ty, cast, to_field) { \
    ty res; \
    WAH_CHECK(call(sp[-1].from_field, &res)); \
    sp[-1].to_field = cast (res); \
    WAH_NEXT(); \
}
#define REINTERPRET(from_field, from_ty, to_field, to_ty) { \
    union { from_ty from; to_ty to; } u = { .from = sp[-1].from_field }; \
    sp[-1].to_field = u.to; \
    WAH_NEXT(); \
}

NUM_OPS(32,f)
NUM_OPS(64,)

WAH_RUN(I32_LOAD) LOAD_OP(32, u32, i32, (int32_t))
WAH_RUN(I64_LOAD) LOAD_OP(64, u64, i64, (int64_t))
WAH_RUN(F32_LOAD) LOAD_OP(32, f32, f32, )
WAH_RUN(F64_LOAD) LOAD_OP(64, f64, f64, )
WAH_RUN(I32_LOAD8_S) LOAD_OP(8, u8, i32, (int32_t)(int8_t))
WAH_RUN(I32_LOAD8_U) LOAD_OP(8, u8, i32, (int32_t))
WAH_RUN(I32_LOAD16_S) LOAD_OP(16, u16, i32, (int32_t)(int16_t))
WAH_RUN(I32_LOAD16_U) LOAD_OP(16, u16, i32, (int32_t))
WAH_RUN(I64_LOAD8_S) LOAD_OP(8, u8, i64, (int64_t)(int8_t))
WAH_RUN(I64_LOAD8_U) LOAD_OP(8, u8, i64, (int64_t))
WAH_RUN(I64_LOAD16_S) LOAD_OP(16, u16, i64, (int64_t)(int16_t))
WAH_RUN(I64_LOAD16_U) LOAD_OP(16, u16, i64, (int64_t))
WAH_RUN(I64_LOAD32_S) LOAD_OP(32, u32, i64, (int64_t)(int32_t))
WAH_RUN(I64_LOAD32_U) LOAD_OP(32, u32, i64, (int64_t))

WAH_RUN(I32_LOAD_i32_mem0) LOAD_OP_I32_MEM0(32, u32, i32, (int32_t))
WAH_RUN(I64_LOAD_i32_mem0) LOAD_OP_I32_MEM0(64, u64, i64, (int64_t))
WAH_RUN(F32_LOAD_i32_mem0) LOAD_OP_I32_MEM0(32, f32, f32, )
WAH_RUN(F64_LOAD_i32_mem0) LOAD_OP_I32_MEM0(64, f64, f64, )
WAH_RUN(I32_LOAD8_S_i32_mem0) LOAD_OP_I32_MEM0(8, u8, i32, (int32_t)(int8_t))
WAH_RUN(I32_LOAD8_U_i32_mem0) LOAD_OP_I32_MEM0(8, u8, i32, (int32_t))
WAH_RUN(I32_LOAD16_S_i32_mem0) LOAD_OP_I32_MEM0(16, u16, i32, (int32_t)(int16_t))
WAH_RUN(I32_LOAD16_U_i32_mem0) LOAD_OP_I32_MEM0(16, u16, i32, (int32_t))
WAH_RUN(I64_LOAD8_S_i32_mem0) LOAD_OP_I32_MEM0(8, u8, i64, (int64_t)(int8_t))
WAH_RUN(I64_LOAD8_U_i32_mem0) LOAD_OP_I32_MEM0(8, u8, i64, (int64_t))
WAH_RUN(I64_LOAD16_S_i32_mem0) LOAD_OP_I32_MEM0(16, u16, i64, (int64_t)(int16_t))
WAH_RUN(I64_LOAD16_U_i32_mem0) LOAD_OP_I32_MEM0(16, u16, i64, (int64_t))
WAH_RUN(I64_LOAD32_S_i32_mem0) LOAD_OP_I32_MEM0(32, u32, i64, (int64_t)(int32_t))
WAH_RUN(I64_LOAD32_U_i32_mem0) LOAD_OP_I32_MEM0(32, u32, i64, (int64_t))

WAH_RUN(I32_STORE) STORE_OP(32, u32, i32, int32_t, (uint32_t))
WAH_RUN(I64_STORE) STORE_OP(64, u64, i64, int64_t, (uint64_t))
WAH_RUN(F32_STORE) STORE_OP(32, f32, f32, float, )
WAH_RUN(F64_STORE) STORE_OP(64, f64, f64, double, )
WAH_RUN(I32_STORE8) STORE_OP(8, u8, i32, int32_t, (uint8_t))
WAH_RUN(I32_STORE16) STORE_OP(16, u16, i32, int32_t, (uint16_t))
WAH_RUN(I64_STORE8) STORE_OP(8, u8, i64, int64_t, (uint8_t))
WAH_RUN(I64_STORE16) STORE_OP(16, u16, i64, int64_t, (uint16_t))
WAH_RUN(I64_STORE32) STORE_OP(32, u32, i64, int64_t, (uint32_t))

WAH_RUN(I32_STORE_i32_mem0) STORE_OP_I32_MEM0(32, u32, i32, int32_t, (uint32_t))
WAH_RUN(I64_STORE_i32_mem0) STORE_OP_I32_MEM0(64, u64, i64, int64_t, (uint64_t))
WAH_RUN(F32_STORE_i32_mem0) STORE_OP_I32_MEM0(32, f32, f32, float, )
WAH_RUN(F64_STORE_i32_mem0) STORE_OP_I32_MEM0(64, f64, f64, double, )
WAH_RUN(I32_STORE8_i32_mem0) STORE_OP_I32_MEM0(8, u8, i32, int32_t, (uint8_t))
WAH_RUN(I32_STORE16_i32_mem0) STORE_OP_I32_MEM0(16, u16, i32, int32_t, (uint16_t))
WAH_RUN(I64_STORE8_i32_mem0) STORE_OP_I32_MEM0(8, u8, i64, int64_t, (uint8_t))
WAH_RUN(I64_STORE16_i32_mem0) STORE_OP_I32_MEM0(16, u16, i64, int64_t, (uint16_t))
WAH_RUN(I64_STORE32_i32_mem0) STORE_OP_I32_MEM0(32, u32, i64, int64_t, (uint32_t))

// --- i64-addressed memory loads/stores (non-mem0) ---
WAH_RUN(I32_LOAD_i64) LOAD_OP_I64(32, u32, i32, (int32_t))
WAH_RUN(I64_LOAD_i64) LOAD_OP_I64(64, u64, i64, (int64_t))
WAH_RUN(F32_LOAD_i64) LOAD_OP_I64(32, f32, f32, )
WAH_RUN(F64_LOAD_i64) LOAD_OP_I64(64, f64, f64, )
WAH_RUN(I32_LOAD8_S_i64) LOAD_OP_I64(8, u8, i32, (int32_t)(int8_t))
WAH_RUN(I32_LOAD8_U_i64) LOAD_OP_I64(8, u8, i32, (int32_t))
WAH_RUN(I32_LOAD16_S_i64) LOAD_OP_I64(16, u16, i32, (int32_t)(int16_t))
WAH_RUN(I32_LOAD16_U_i64) LOAD_OP_I64(16, u16, i32, (int32_t))
WAH_RUN(I64_LOAD8_S_i64) LOAD_OP_I64(8, u8, i64, (int64_t)(int8_t))
WAH_RUN(I64_LOAD8_U_i64) LOAD_OP_I64(8, u8, i64, (int64_t))
WAH_RUN(I64_LOAD16_S_i64) LOAD_OP_I64(16, u16, i64, (int64_t)(int16_t))
WAH_RUN(I64_LOAD16_U_i64) LOAD_OP_I64(16, u16, i64, (int64_t))
WAH_RUN(I64_LOAD32_S_i64) LOAD_OP_I64(32, u32, i64, (int64_t)(int32_t))
WAH_RUN(I64_LOAD32_U_i64) LOAD_OP_I64(32, u32, i64, (int64_t))

WAH_RUN(I32_STORE_i64) STORE_OP_I64(32, u32, i32, int32_t, (uint32_t))
WAH_RUN(I64_STORE_i64) STORE_OP_I64(64, u64, i64, int64_t, (uint64_t))
WAH_RUN(F32_STORE_i64) STORE_OP_I64(32, f32, f32, float, )
WAH_RUN(F64_STORE_i64) STORE_OP_I64(64, f64, f64, double, )
WAH_RUN(I32_STORE8_i64) STORE_OP_I64(8, u8, i32, int32_t, (uint8_t))
WAH_RUN(I32_STORE16_i64) STORE_OP_I64(16, u16, i32, int32_t, (uint16_t))
WAH_RUN(I64_STORE8_i64) STORE_OP_I64(8, u8, i64, int64_t, (uint8_t))
WAH_RUN(I64_STORE16_i64) STORE_OP_I64(16, u16, i64, int64_t, (uint16_t))
WAH_RUN(I64_STORE32_i64) STORE_OP_I64(32, u32, i64, int64_t, (uint32_t))

// --- i64-addressed memory loads/stores (mem0 fast-path) ---
WAH_RUN(I32_LOAD_i64_mem0) LOAD_OP_I64_MEM0(32, u32, i32, (int32_t))
WAH_RUN(I64_LOAD_i64_mem0) LOAD_OP_I64_MEM0(64, u64, i64, (int64_t))
WAH_RUN(F32_LOAD_i64_mem0) LOAD_OP_I64_MEM0(32, f32, f32, )
WAH_RUN(F64_LOAD_i64_mem0) LOAD_OP_I64_MEM0(64, f64, f64, )
WAH_RUN(I32_LOAD8_S_i64_mem0) LOAD_OP_I64_MEM0(8, u8, i32, (int32_t)(int8_t))
WAH_RUN(I32_LOAD8_U_i64_mem0) LOAD_OP_I64_MEM0(8, u8, i32, (int32_t))
WAH_RUN(I32_LOAD16_S_i64_mem0) LOAD_OP_I64_MEM0(16, u16, i32, (int32_t)(int16_t))
WAH_RUN(I32_LOAD16_U_i64_mem0) LOAD_OP_I64_MEM0(16, u16, i32, (int32_t))
WAH_RUN(I64_LOAD8_S_i64_mem0) LOAD_OP_I64_MEM0(8, u8, i64, (int64_t)(int8_t))
WAH_RUN(I64_LOAD8_U_i64_mem0) LOAD_OP_I64_MEM0(8, u8, i64, (int64_t))
WAH_RUN(I64_LOAD16_S_i64_mem0) LOAD_OP_I64_MEM0(16, u16, i64, (int64_t)(int16_t))
WAH_RUN(I64_LOAD16_U_i64_mem0) LOAD_OP_I64_MEM0(16, u16, i64, (int64_t))
WAH_RUN(I64_LOAD32_S_i64_mem0) LOAD_OP_I64_MEM0(32, u32, i64, (int64_t)(int32_t))
WAH_RUN(I64_LOAD32_U_i64_mem0) LOAD_OP_I64_MEM0(32, u32, i64, (int64_t))

WAH_RUN(I32_STORE_i64_mem0) STORE_OP_I64_MEM0(32, u32, i32, int32_t, (uint32_t))
WAH_RUN(I64_STORE_i64_mem0) STORE_OP_I64_MEM0(64, u64, i64, int64_t, (uint64_t))
WAH_RUN(F32_STORE_i64_mem0) STORE_OP_I64_MEM0(32, f32, f32, float, )
WAH_RUN(F64_STORE_i64_mem0) STORE_OP_I64_MEM0(64, f64, f64, double, )
WAH_RUN(I32_STORE8_i64_mem0) STORE_OP_I64_MEM0(8, u8, i32, int32_t, (uint8_t))
WAH_RUN(I32_STORE16_i64_mem0) STORE_OP_I64_MEM0(16, u16, i32, int32_t, (uint16_t))
WAH_RUN(I64_STORE8_i64_mem0) STORE_OP_I64_MEM0(8, u8, i64, int64_t, (uint8_t))
WAH_RUN(I64_STORE16_i64_mem0) STORE_OP_I64_MEM0(16, u16, i64, int64_t, (uint16_t))
WAH_RUN(I64_STORE32_i64_mem0) STORE_OP_I64_MEM0(32, u32, i64, int64_t, (uint32_t))

WAH_RUN(I32_WRAP_I64) CONVERT(i64, (int32_t), i32)
WAH_RUN(I32_TRUNC_F32_S) CONVERT_CHECK(f32, wah_trunc_f32_to_i32, int32_t, , i32)
WAH_RUN(I32_TRUNC_F32_U) CONVERT_CHECK(f32, wah_trunc_f32_to_u32, uint32_t, (int32_t), i32)
WAH_RUN(I32_TRUNC_F64_S) CONVERT_CHECK(f64, wah_trunc_f64_to_i32, int32_t, , i32)
WAH_RUN(I32_TRUNC_F64_U) CONVERT_CHECK(f64, wah_trunc_f64_to_u32, uint32_t, (int32_t), i32)

WAH_RUN(I64_EXTEND_I32_S) CONVERT(i32, (int64_t), i64)
WAH_RUN(I64_EXTEND_I32_U) CONVERT(i32, (int64_t)(uint32_t), i64)
WAH_RUN(I64_TRUNC_F32_S) CONVERT_CHECK(f32, wah_trunc_f32_to_i64, int64_t, , i64)
WAH_RUN(I64_TRUNC_F32_U) CONVERT_CHECK(f32, wah_trunc_f32_to_u64, uint64_t, (int64_t), i64)
WAH_RUN(I64_TRUNC_F64_S) CONVERT_CHECK(f64, wah_trunc_f64_to_i64, int64_t, , i64)
WAH_RUN(I64_TRUNC_F64_U) CONVERT_CHECK(f64, wah_trunc_f64_to_u64, uint64_t, (int64_t), i64)

WAH_RUN(F32_CONVERT_I32_S) CONVERT(i32, (float), f32)
WAH_RUN(F32_CONVERT_I32_U) CONVERT(i32, (float)(uint32_t), f32)
WAH_RUN(F32_CONVERT_I64_S) CONVERT(i64, (float), f32)
WAH_RUN(F32_CONVERT_I64_U) CONVERT(i64, (float)(uint64_t), f32)
WAH_RUN(F32_DEMOTE_F64) { sp[-1].f32 = wah_canonicalize_f32((float)sp[-1].f64); WAH_NEXT(); }

WAH_RUN(F64_CONVERT_I32_S) CONVERT(i32, (double), f64)
WAH_RUN(F64_CONVERT_I32_U) CONVERT(i32, (double)(uint32_t), f64)
WAH_RUN(F64_CONVERT_I64_S) CONVERT(i64, (double), f64)
WAH_RUN(F64_CONVERT_I64_U) CONVERT(i64, (double)(uint64_t), f64)
WAH_RUN(F64_PROMOTE_F32) { sp[-1].f64 = wah_canonicalize_f64((double)sp[-1].f32); WAH_NEXT(); }

WAH_RUN(I32_REINTERPRET_F32) REINTERPRET(f32, float, i32, int32_t)
WAH_RUN(I64_REINTERPRET_F64) REINTERPRET(f64, double, i64, int64_t)
WAH_RUN(F32_REINTERPRET_I32) REINTERPRET(i32, int32_t, f32, float)
WAH_RUN(F64_REINTERPRET_I64) REINTERPRET(i64, int64_t, f64, double)

WAH_RUN(I32_EXTEND8_S)  { sp[-1].i32 = (int32_t) (int8_t)sp[-1].i32; WAH_NEXT(); }
WAH_RUN(I32_EXTEND16_S) { sp[-1].i32 = (int32_t)(int16_t)sp[-1].i32; WAH_NEXT(); }
WAH_RUN(I64_EXTEND8_S)  { sp[-1].i64 = (int64_t) (int8_t)sp[-1].i64; WAH_NEXT(); }
WAH_RUN(I64_EXTEND16_S) { sp[-1].i64 = (int64_t)(int16_t)sp[-1].i64; WAH_NEXT(); }
WAH_RUN(I64_EXTEND32_S) { sp[-1].i64 = (int64_t)(int32_t)sp[-1].i64; WAH_NEXT(); }

WAH_RUN(I32_TRUNC_SAT_F32_S) { sp[-1].i32 =          wah_trunc_sat_f32_to_i32(sp[-1].f32); WAH_NEXT(); }
WAH_RUN(I32_TRUNC_SAT_F32_U) { sp[-1].i32 = (int32_t)wah_trunc_sat_f32_to_u32(sp[-1].f32); WAH_NEXT(); }
WAH_RUN(I32_TRUNC_SAT_F64_S) { sp[-1].i32 =          wah_trunc_sat_f64_to_i32(sp[-1].f64); WAH_NEXT(); }
WAH_RUN(I32_TRUNC_SAT_F64_U) { sp[-1].i32 = (int32_t)wah_trunc_sat_f64_to_u32(sp[-1].f64); WAH_NEXT(); }
WAH_RUN(I64_TRUNC_SAT_F32_S) { sp[-1].i64 =          wah_trunc_sat_f32_to_i64(sp[-1].f32); WAH_NEXT(); }
WAH_RUN(I64_TRUNC_SAT_F32_U) { sp[-1].i64 = (int64_t)wah_trunc_sat_f32_to_u64(sp[-1].f32); WAH_NEXT(); }
WAH_RUN(I64_TRUNC_SAT_F64_S) { sp[-1].i64 =          wah_trunc_sat_f64_to_i64(sp[-1].f64); WAH_NEXT(); }
WAH_RUN(I64_TRUNC_SAT_F64_U) { sp[-1].i64 = (int64_t)wah_trunc_sat_f64_to_u64(sp[-1].f64); WAH_NEXT(); }

#undef BINOP_I
#undef CMP_I_S
#undef CMP_I_U
#undef BINOP_F
#undef CMP_F
#undef UNOP_I_FN
#undef BINOP_I_FN
#undef UNOP_F_FN
#undef BINOP_F_FN
#undef NUM_OPS
#undef LOAD_OP
#undef LOAD_OP_I32_MEM0
#undef STORE_OP
#undef STORE_OP_I32_MEM0
#undef LOAD_OP_I64
#undef STORE_OP_I64
#undef LOAD_OP_I64_MEM0
#undef STORE_OP_I64_MEM0
#undef CONVERT
#undef CONVERT_CHECK
#undef REINTERPRET

WAH_RUN(MEMORY_SIZE) {
    uint32_t mem_idx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);
    WAH_ASSERT(mem_idx < ctx->memory_count && "validation didn't catch out-of-bound memory index");
    (*sp++).i32 = (int32_t)(ctx->memory_sizes[mem_idx] / WAH_WASM_PAGE_SIZE);
    WAH_NEXT();
}

WAH_RUN(MEMORY_GROW) {
    uint32_t mem_idx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);
    WAH_ASSERT(mem_idx < ctx->memory_count && "validation didn't catch out-of-bound memory index");

    int32_t pages_to_grow = (*--sp).i32;
    if (pages_to_grow < 0) {
        (*sp++).i32 = -1; // Cannot grow by negative pages
        WAH_NEXT();
    }

    uint32_t old_pages = ctx->memory_sizes[mem_idx] / WAH_WASM_PAGE_SIZE;
    uint64_t new_pages = (uint64_t)old_pages + pages_to_grow;

    if (new_pages > ctx->module->memories[mem_idx].max_pages) {
        (*sp++).i32 = -1; // Exceeds max memory
        WAH_NEXT();
    }

    size_t new_memory_size = (size_t)new_pages * WAH_WASM_PAGE_SIZE;
    WAH_REALLOC_ARRAY_GOTO(ctx->memories[mem_idx], new_memory_size, cleanup);

    // Initialize newly allocated memory to zero
    if (new_memory_size > ctx->memory_sizes[mem_idx]) {
        memset(ctx->memories[mem_idx] + ctx->memory_sizes[mem_idx], 0, new_memory_size - ctx->memory_sizes[mem_idx]);
    }

    WAH_ENSURE_GOTO(new_memory_size <= UINT64_MAX, WAH_ERROR_TOO_LARGE, cleanup);
    ctx->memory_sizes[mem_idx] = (uint64_t)new_memory_size;
    // Keep memory_base/memory_size (fast-path aliases) in sync for memory 0
    if (mem_idx == 0) {
        ctx->memory_base = ctx->memories[0];
        ctx->memory_size = ctx->memory_sizes[0];
    }
    (*sp++).i32 = (int32_t)old_pages;
    WAH_NEXT();
    WAH_CLEANUP();
}

WAH_RUN(MEMORY_FILL) {
    uint32_t mem_idx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);
    WAH_ASSERT(mem_idx < ctx->memory_count && "validation didn't catch out-of-bound memory index");

    uint32_t size = (uint32_t)(*--sp).i32;
    uint8_t val = (uint8_t)(*--sp).i32;
    uint32_t dst = (uint32_t)(*--sp).i32;

    WAH_ENSURE_GOTO((uint64_t)dst + size <= ctx->memory_sizes[mem_idx], WAH_ERROR_MEMORY_OUT_OF_BOUNDS, cleanup);
    memset(ctx->memories[mem_idx] + dst, val, size);
    WAH_NEXT();
    WAH_CLEANUP();
}

WAH_RUN(MEMORY_INIT) {
    uint32_t data_idx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);
    uint32_t mem_idx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);

    WAH_ASSERT(mem_idx < ctx->memory_count && "validation didn't catch out-of-bound memory index");
    WAH_ASSERT(data_idx < ctx->module->data_segment_count && "validation didn't catch out-of-bound data segment index");

    uint32_t size = (uint32_t)(*--sp).i32;
    uint32_t src_offset = (uint32_t)(*--sp).i32;
    uint32_t dest_offset = (uint32_t)(*--sp).i32;

    const wah_data_segment_t *segment = &ctx->module->data_segments[data_idx];

    WAH_ENSURE_GOTO((uint64_t)dest_offset + size <= ctx->memory_sizes[mem_idx], WAH_ERROR_MEMORY_OUT_OF_BOUNDS, cleanup);
    WAH_ENSURE_GOTO((uint64_t)src_offset + size <= segment->data_len, WAH_ERROR_TRAP, cleanup); // Ensure source data is within segment bounds
    WAH_ENSURE_GOTO(size <= segment->data_len, WAH_ERROR_TRAP, cleanup); // Cannot initialize more than available data

    memcpy(ctx->memories[mem_idx] + dest_offset, segment->data + src_offset, size);
    WAH_NEXT();
    WAH_CLEANUP();
}

WAH_RUN(MEMORY_COPY) {
    uint32_t dest_mem_idx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);
    uint32_t src_mem_idx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);

    WAH_ASSERT(dest_mem_idx < ctx->memory_count && "validation didn't catch out-of-bound destination memory index");
    WAH_ASSERT(src_mem_idx < ctx->memory_count && "validation didn't catch out-of-bound source memory index");

    uint32_t size = (uint32_t)(*--sp).i32;
    uint32_t src = (uint32_t)(*--sp).i32;
    uint32_t dest = (uint32_t)(*--sp).i32;

    WAH_ENSURE_GOTO((uint64_t)dest + size <= ctx->memory_sizes[dest_mem_idx], WAH_ERROR_MEMORY_OUT_OF_BOUNDS, cleanup);
    WAH_ENSURE_GOTO((uint64_t)src + size <= ctx->memory_sizes[src_mem_idx], WAH_ERROR_MEMORY_OUT_OF_BOUNDS, cleanup);

    memmove(ctx->memories[dest_mem_idx] + dest, ctx->memories[src_mem_idx] + src, size);
    WAH_NEXT();
    WAH_CLEANUP();
}

// --- i64-addressed memory.size/grow ---
WAH_RUN(MEMORY_SIZE_i64) {
    uint32_t mem_idx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);
    WAH_ASSERT(mem_idx < ctx->memory_count && "validation didn't catch out-of-bound memory index");
    (*sp++).i64 = (int64_t)(ctx->memory_sizes[mem_idx] / WAH_WASM_PAGE_SIZE);
    WAH_NEXT();
}

WAH_RUN(MEMORY_GROW_i64) {
    uint32_t mem_idx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);
    WAH_ASSERT(mem_idx < ctx->memory_count && "validation didn't catch out-of-bound memory index");

    int64_t pages_to_grow = (*--sp).i64;
    if (pages_to_grow < 0) {
        (*sp++).i64 = -1;
        WAH_NEXT();
    }

    uint64_t old_pages = ctx->memory_sizes[mem_idx] / WAH_WASM_PAGE_SIZE;
    uint64_t new_pages = old_pages + (uint64_t)pages_to_grow;

    if (new_pages > ctx->module->memories[mem_idx].max_pages) {
        (*sp++).i64 = -1;
        WAH_NEXT();
    }

    size_t new_memory_size = (size_t)new_pages * WAH_WASM_PAGE_SIZE;
    WAH_REALLOC_ARRAY_GOTO(ctx->memories[mem_idx], new_memory_size, cleanup);

    if (new_memory_size > ctx->memory_sizes[mem_idx]) {
        memset(ctx->memories[mem_idx] + ctx->memory_sizes[mem_idx], 0, new_memory_size - ctx->memory_sizes[mem_idx]);
    }

    ctx->memory_sizes[mem_idx] = (uint64_t)new_memory_size;
    if (mem_idx == 0) {
        ctx->memory_base = ctx->memories[0];
        ctx->memory_size = ctx->memory_sizes[0];
    }
    (*sp++).i64 = (int64_t)old_pages;
    WAH_NEXT();
    WAH_CLEANUP();
}

// --- i64-addressed memory.fill/init/copy ---
WAH_RUN(MEMORY_FILL_i64) {
    uint32_t mem_idx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);
    WAH_ASSERT(mem_idx < ctx->memory_count && "validation didn't catch out-of-bound memory index");

    uint32_t size = (uint32_t)(*--sp).i32;
    uint8_t val = (uint8_t)(*--sp).i32;
    uint64_t dst = (uint64_t)(*--sp).i64;

    WAH_ENSURE_GOTO(dst + size <= ctx->memory_sizes[mem_idx], WAH_ERROR_MEMORY_OUT_OF_BOUNDS, cleanup);
    memset(ctx->memories[mem_idx] + dst, val, size);
    WAH_NEXT();
    WAH_CLEANUP();
}

WAH_RUN(MEMORY_INIT_i64) {
    uint32_t data_idx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);
    uint32_t mem_idx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);

    WAH_ASSERT(mem_idx < ctx->memory_count && "validation didn't catch out-of-bound memory index");
    WAH_ASSERT(data_idx < ctx->module->data_segment_count && "validation didn't catch out-of-bound data segment index");

    uint32_t size = (uint32_t)(*--sp).i32;
    uint32_t src_offset = (uint32_t)(*--sp).i32;
    uint64_t dest_offset = (uint64_t)(*--sp).i64;

    const wah_data_segment_t *segment = &ctx->module->data_segments[data_idx];

    WAH_ENSURE_GOTO(dest_offset + size <= ctx->memory_sizes[mem_idx], WAH_ERROR_MEMORY_OUT_OF_BOUNDS, cleanup);
    WAH_ENSURE_GOTO((uint64_t)src_offset + size <= segment->data_len, WAH_ERROR_TRAP, cleanup);
    WAH_ENSURE_GOTO(size <= segment->data_len, WAH_ERROR_TRAP, cleanup);

    memcpy(ctx->memories[mem_idx] + dest_offset, segment->data + src_offset, size);
    WAH_NEXT();
    WAH_CLEANUP();
}

WAH_RUN(MEMORY_COPY_i64) {
    uint32_t dest_mem_idx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);
    uint32_t src_mem_idx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);

    WAH_ASSERT(dest_mem_idx < ctx->memory_count && "validation didn't catch out-of-bound destination memory index");
    WAH_ASSERT(src_mem_idx < ctx->memory_count && "validation didn't catch out-of-bound source memory index");

    uint32_t size = (uint32_t)(*--sp).i32;
    bool src_i64 = ctx->module->memories[src_mem_idx].addr_type == WAH_TYPE_I64;
    bool dest_i64 = ctx->module->memories[dest_mem_idx].addr_type == WAH_TYPE_I64;
    uint64_t src = src_i64 ? (uint64_t)(*--sp).i64 : (uint32_t)(*--sp).i32;
    uint64_t dest = dest_i64 ? (uint64_t)(*--sp).i64 : (uint32_t)(*--sp).i32;

    WAH_ENSURE_GOTO(dest + size <= ctx->memory_sizes[dest_mem_idx], WAH_ERROR_MEMORY_OUT_OF_BOUNDS, cleanup);
    WAH_ENSURE_GOTO(src + size <= ctx->memory_sizes[src_mem_idx], WAH_ERROR_MEMORY_OUT_OF_BOUNDS, cleanup);

    memmove(ctx->memories[dest_mem_idx] + dest, ctx->memories[src_mem_idx] + src, size);
    WAH_NEXT();
    WAH_CLEANUP();
}

WAH_RUN(DROP) { sp--; WAH_NEXT(); }

WAH_RUN(SELECT) {
    wah_value_t c = *--sp;
    wah_value_t b = *--sp;
    wah_value_t a = *--sp;
    *sp++ = c.i32 ? a : b;
    WAH_NEXT();
}

WAH_RUN(NOP) { WAH_NEXT(); }

WAH_RUN(UNREACHABLE) {
    (void)bytecode_base;
    err = WAH_ERROR_TRAP;
    WAH_CLEANUP();
}

// --- Vector instructions ---

#define M128I_UNARY_OP(fn, ...) WAH_IF_X86_64({ sp[-1]._m128i = fn(sp[-1]._m128i); WAH_NEXT(); }, __VA_ARGS__)
#define M128I_UNARY_I32_OP(fn, ...) WAH_IF_X86_64({ sp[-1].i32 = fn(sp[-1]._m128i); WAH_NEXT(); }, __VA_ARGS__)
#define M128I_BINARY_OP(fn, ...) WAH_IF_X86_64({ sp[-2]._m128i = fn(sp[-2]._m128i, sp[-1]._m128i); sp--; WAH_NEXT(); }, __VA_ARGS__)
#define M128I_NOT_BINARY_OP(fn, ...) \
    WAH_IF_X86_64({ sp[-2]._m128i = _mm_xor_si128(fn(sp[-2]._m128i, sp[-1]._m128i), _mm_set1_epi8(-1)); sp--; WAH_NEXT(); }, __VA_ARGS__)
#define M128I_SHIFT_OP(fn, mask, ...) \
    WAH_IF_X86_64({ int32_t count = sp[-1].i32; sp[-2]._m128i = fn(sp[-2]._m128i, count & mask); sp--; WAH_NEXT(); }, __VA_ARGS__)
#define M128I_TERNARY_OP(expr, ...) \
    WAH_IF_X86_64({ __m128i c = sp[-1]._m128i; __m128i b = sp[-2]._m128i; __m128i a = sp[-3]._m128i; \
                    sp[-3]._m128i = (expr); sp -= 2; WAH_NEXT(); }, __VA_ARGS__)

#define M128_UNARY_OP(fn, ...) WAH_IF_X86_64({ sp[-1]._m128 = fn(sp[-1]._m128); WAH_NEXT(); }, __VA_ARGS__)
#define M128_BINARY_OP(fn, ...) WAH_IF_X86_64({ sp[-2]._m128 = fn(sp[-2]._m128, sp[-1]._m128); sp--; WAH_NEXT(); }, __VA_ARGS__)
#define M128_NOT_BINARY_OP(fn, ...) \
    WAH_IF_X86_64({ sp[-2]._m128 = _mm_xor_ps(fn(sp[-2]._m128, sp[-1]._m128), _mm_castsi128_ps(_mm_set1_epi8(-1))); \
                    sp--; WAH_NEXT(); }, __VA_ARGS__)
#define M128_TERNARY_OP(expr, ...) \
    WAH_IF_X86_64({ __m128 c = sp[-1]._m128; __m128 b = sp[-2]._m128; __m128 a = sp[-3]._m128; \
                    sp[-3]._m128 = (expr); sp -= 2; WAH_NEXT(); }, __VA_ARGS__)

#define M128D_UNARY_OP(fn, ...) WAH_IF_X86_64({ sp[-1]._m128d = fn(sp[-1]._m128d); WAH_NEXT(); }, __VA_ARGS__)
#define M128D_BINARY_OP(fn, ...) WAH_IF_X86_64({ sp[-2]._m128d = fn(sp[-2]._m128d, sp[-1]._m128d); sp--; WAH_NEXT(); }, __VA_ARGS__)
#define M128D_NOT_BINARY_OP(fn, ...) \
    WAH_IF_X86_64({ sp[-2]._m128d = _mm_xor_pd(fn(sp[-2]._m128d, sp[-1]._m128d), _mm_castsi128_pd(_mm_set1_epi8(-1))); \
                    sp--; WAH_NEXT(); }, __VA_ARGS__)
#define M128D_TERNARY_OP(expr, ...) \
    WAH_IF_X86_64({ __m128d c = sp[-1]._m128d; __m128d b = sp[-2]._m128d; __m128d a = sp[-3]._m128d; \
                    sp[-3]._m128d = (expr); sp -= 2; WAH_NEXT(); }, __VA_ARGS__)

#define N128_UNARY_OP(fn, ...) WAH_IF_AARCH64({ sp[-1]._u8x16 = fn(sp[-1]._u8x16); WAH_NEXT(); }, __VA_ARGS__)
#define N128_UNARY_I32_OP(fn, ...) WAH_IF_AARCH64({ sp[-1].i32 = fn(sp[-1]._u8x16); WAH_NEXT(); }, __VA_ARGS__)
#define N128_BINARY_OP(fn, ...) WAH_IF_AARCH64({ sp[-2]._u8x16 = fn(sp[-2]._u8x16, sp[-1]._u8x16); sp--; WAH_NEXT(); }, __VA_ARGS__)
#define N128_NOT_BINARY_OP(fn, ...) \
    WAH_IF_AARCH64({ sp[-2]._u8x16 = veorq_u8(fn(sp[-2]._u8x16, sp[-1]._u8x16), vdupq_n_u8(0xFF)); sp--; WAH_NEXT(); }, __VA_ARGS__)
#define N128_SHIFT_OP(fn, ...) \
    WAH_IF_AARCH64({ int32_t count = sp[-1].i32; sp[-2]._u8x16 = fn(sp[-2]._u8x16, count); sp--; WAH_NEXT(); }, __VA_ARGS__)
#define N128_TERNARY_OP(expr, ...) \
    WAH_IF_AARCH64({ uint8x16_t c = sp[-1]._u8x16; uint8x16_t b = sp[-2]._u8x16; uint8x16_t a = sp[-3]._u8x16; \
                     sp[-3]._u8x16 = (expr); sp -= 2; WAH_NEXT(); }, __VA_ARGS__)

// addr_expr pops the address from the stack with appropriate type:
//   i32 addressing: WAH_SP_ADDR_I32  (zero-extends via uint32_t)
//   i64 addressing: WAH_SP_ADDR_I64
#define WAH_SP_ADDR_I32 ((uint64_t)(uint32_t)(*--sp).i32)
#define WAH_SP_ADDR_I64 ((uint64_t)(*--sp).i64)

#define V128_LOAD_COMMON(mem0, read_size, addr_expr) \
    uint32_t memidx = 0; \
    if (!mem0) { \
        memidx = wah_read_u32_le(bytecode_ip); \
        bytecode_ip += sizeof(uint32_t); \
    } \
    uint64_t offset = wah_read_u64_le(bytecode_ip); \
    bytecode_ip += sizeof(uint64_t); \
    uint64_t addr = (addr_expr); \
    uint64_t effective_addr; \
    WAH_ASSERT(memidx < ctx->memory_count && "validation didn't catch out-of-bound memory index"); \
    WAH_CHECK(wah_check_effective_addr(addr, offset, (read_size), ctx->memory_sizes[memidx], &effective_addr))

#define V128_LOAD_HALF_OP(addr_expr, N, elem_ty, cast) { \
    V128_LOAD_COMMON(0, 8, addr_expr); \
    wah_v128_t *v = &(*sp++).v128; \
    for (int i = 0; i < 64/N; ++i) { \
        v->elem_ty[i] = cast(wah_read_u##N##_le(ctx->memories[memidx] + effective_addr + i * (N/8))); \
    } \
    WAH_NEXT(); \
}

#define V128_LOAD_HALF_OP_MEM0(addr_expr, N, elem_ty, cast) { \
    V128_LOAD_COMMON(1, 8, addr_expr); \
    wah_v128_t *v = &(*sp++).v128; \
    for (int i = 0; i < 64/N; ++i) { \
        v->elem_ty[i] = cast(wah_read_u##N##_le(ctx->memory_base + effective_addr + i * (N/8))); \
    } \
    WAH_NEXT(); \
}

#define V128_LOAD_SPLAT_OP(addr_expr, N) { \
    V128_LOAD_COMMON(0, N/8, addr_expr); \
    wah_v128_t *v = &(*sp++).v128; \
    uint##N##_t val = wah_read_u##N##_le(ctx->memories[memidx] + effective_addr); \
    for (int i = 0; i < 128/N; ++i) v->u##N[i] = val; \
    WAH_NEXT(); \
}

#define V128_LOAD_SPLAT_OP_MEM0(addr_expr, N) { \
    V128_LOAD_COMMON(1, N/8, addr_expr); \
    wah_v128_t *v = &(*sp++).v128; \
    uint##N##_t val = wah_read_u##N##_le(ctx->memory_base + effective_addr); \
    for (int i = 0; i < 128/N; ++i) v->u##N[i] = val; \
    WAH_NEXT(); \
}

#define V128_LOAD_LANE_OP(addr_expr, N) { \
    uint32_t memidx = wah_read_u32_le(bytecode_ip); \
    bytecode_ip += sizeof(uint32_t); \
    uint64_t offset = wah_read_u64_le(bytecode_ip); \
    bytecode_ip += sizeof(uint64_t); \
    uint32_t lane_idx = *bytecode_ip++; \
    wah_v128_t val = (*--sp).v128; /* Existing vector */ \
    uint64_t addr = (addr_expr); \
    uint64_t effective_addr; \
    \
    WAH_ASSERT(memidx < ctx->memory_count && "validation didn't catch out-of-bound memory index"); \
    WAH_CHECK_GOTO(wah_check_effective_addr(addr, offset, N/8, ctx->memory_sizes[memidx], &effective_addr), cleanup); \
    WAH_ASSERT(lane_idx < 128/N && "validation didn't catch out-of-bound lane index"); \
    val.u##N[lane_idx] = wah_read_u##N##_le(ctx->memories[memidx] + effective_addr); \
    (*sp++).v128 = val; \
    WAH_NEXT(); \
    WAH_CLEANUP(); \
}

#define V128_LOAD_LANE_OP_MEM0(addr_expr, N) { \
    uint64_t offset = wah_read_u64_le(bytecode_ip); \
    bytecode_ip += sizeof(uint64_t); \
    uint32_t lane_idx = *bytecode_ip++; \
    wah_v128_t val = (*--sp).v128; /* Existing vector */ \
    uint64_t addr = (addr_expr); \
    uint64_t effective_addr; \
    \
    WAH_CHECK_GOTO(wah_check_effective_addr(addr, offset, N/8, ctx->memory_size, &effective_addr), cleanup); \
    WAH_ASSERT(lane_idx < 128/N && "validation didn't catch out-of-bound lane index"); \
    val.u##N[lane_idx] = wah_read_u##N##_le(ctx->memory_base + effective_addr); \
    (*sp++).v128 = val; \
    WAH_NEXT(); \
    WAH_CLEANUP(); \
}

WAH_RUN(V128_LOAD) {
    V128_LOAD_COMMON(0, sizeof(wah_v128_t), WAH_SP_ADDR_I32);
    memcpy(&(*sp++).v128, ctx->memories[memidx] + effective_addr, sizeof(wah_v128_t));
    WAH_NEXT();
}

WAH_RUN(V128_LOAD_i32_mem0) {
    V128_LOAD_COMMON(1, sizeof(wah_v128_t), WAH_SP_ADDR_I32);
    memcpy(&(*sp++).v128, ctx->memory_base + effective_addr, sizeof(wah_v128_t));
    WAH_NEXT();
}

WAH_RUN(V128_LOAD8X8_S) V128_LOAD_HALF_OP(WAH_SP_ADDR_I32, 8, i16, (int16_t)(int8_t))
WAH_RUN(V128_LOAD8X8_U) V128_LOAD_HALF_OP(WAH_SP_ADDR_I32, 8, u16, (uint16_t))
WAH_RUN(V128_LOAD16X4_S) V128_LOAD_HALF_OP(WAH_SP_ADDR_I32, 16, i32, (int32_t)(int16_t))
WAH_RUN(V128_LOAD16X4_U) V128_LOAD_HALF_OP(WAH_SP_ADDR_I32, 16, u32, (uint32_t))
WAH_RUN(V128_LOAD32X2_S) V128_LOAD_HALF_OP(WAH_SP_ADDR_I32, 32, i64, (int64_t)(int32_t))
WAH_RUN(V128_LOAD32X2_U) V128_LOAD_HALF_OP(WAH_SP_ADDR_I32, 32, u64, (uint64_t))

WAH_RUN(V128_LOAD8X8_S_i32_mem0) V128_LOAD_HALF_OP_MEM0(WAH_SP_ADDR_I32, 8, i16, (int16_t)(int8_t))
WAH_RUN(V128_LOAD8X8_U_i32_mem0) V128_LOAD_HALF_OP_MEM0(WAH_SP_ADDR_I32, 8, u16, (uint16_t))
WAH_RUN(V128_LOAD16X4_S_i32_mem0) V128_LOAD_HALF_OP_MEM0(WAH_SP_ADDR_I32, 16, i32, (int32_t)(int16_t))
WAH_RUN(V128_LOAD16X4_U_i32_mem0) V128_LOAD_HALF_OP_MEM0(WAH_SP_ADDR_I32, 16, u32, (uint32_t))
WAH_RUN(V128_LOAD32X2_S_i32_mem0) V128_LOAD_HALF_OP_MEM0(WAH_SP_ADDR_I32, 32, i64, (int64_t)(int32_t))
WAH_RUN(V128_LOAD32X2_U_i32_mem0) V128_LOAD_HALF_OP_MEM0(WAH_SP_ADDR_I32, 32, u64, (uint64_t))

WAH_RUN(V128_LOAD8_SPLAT) V128_LOAD_SPLAT_OP(WAH_SP_ADDR_I32, 8)
WAH_RUN(V128_LOAD16_SPLAT) V128_LOAD_SPLAT_OP(WAH_SP_ADDR_I32, 16)
WAH_RUN(V128_LOAD32_SPLAT) V128_LOAD_SPLAT_OP(WAH_SP_ADDR_I32, 32)
WAH_RUN(V128_LOAD64_SPLAT) V128_LOAD_SPLAT_OP(WAH_SP_ADDR_I32, 64)

WAH_RUN(V128_LOAD8_SPLAT_i32_mem0) V128_LOAD_SPLAT_OP_MEM0(WAH_SP_ADDR_I32, 8)
WAH_RUN(V128_LOAD16_SPLAT_i32_mem0) V128_LOAD_SPLAT_OP_MEM0(WAH_SP_ADDR_I32, 16)
WAH_RUN(V128_LOAD32_SPLAT_i32_mem0) V128_LOAD_SPLAT_OP_MEM0(WAH_SP_ADDR_I32, 32)
WAH_RUN(V128_LOAD64_SPLAT_i32_mem0) V128_LOAD_SPLAT_OP_MEM0(WAH_SP_ADDR_I32, 64)

WAH_RUN(V128_LOAD32_ZERO) {
    V128_LOAD_COMMON(0, 4, WAH_SP_ADDR_I32);
    wah_v128_t *v = &(*sp++).v128;
    *v = (wah_v128_t){.u64 = {wah_read_u32_le(ctx->memories[memidx] + effective_addr), 0}};
    WAH_NEXT();
}
WAH_RUN(V128_LOAD64_ZERO) {
    V128_LOAD_COMMON(0, 8, WAH_SP_ADDR_I32);
    wah_v128_t *v = &(*sp++).v128;
    *v = (wah_v128_t){.u64 = {wah_read_u64_le(ctx->memories[memidx] + effective_addr), 0}};
    WAH_NEXT();
}

WAH_RUN(V128_LOAD32_ZERO_i32_mem0) {
    V128_LOAD_COMMON(1, 4, WAH_SP_ADDR_I32);
    wah_v128_t *v = &(*sp++).v128;
    *v = (wah_v128_t){.u64 = {wah_read_u32_le(ctx->memory_base + effective_addr), 0}};
    WAH_NEXT();
}
WAH_RUN(V128_LOAD64_ZERO_i32_mem0) {
    V128_LOAD_COMMON(1, 8, WAH_SP_ADDR_I32);
    wah_v128_t *v = &(*sp++).v128;
    *v = (wah_v128_t){.u64 = {wah_read_u64_le(ctx->memory_base + effective_addr), 0}};
    WAH_NEXT();
}

WAH_RUN(V128_LOAD8_LANE) V128_LOAD_LANE_OP(WAH_SP_ADDR_I32, 8)
WAH_RUN(V128_LOAD16_LANE) V128_LOAD_LANE_OP(WAH_SP_ADDR_I32, 16)
WAH_RUN(V128_LOAD32_LANE) V128_LOAD_LANE_OP(WAH_SP_ADDR_I32, 32)
WAH_RUN(V128_LOAD64_LANE) V128_LOAD_LANE_OP(WAH_SP_ADDR_I32, 64)

WAH_RUN(V128_LOAD8_LANE_i32_mem0) V128_LOAD_LANE_OP_MEM0(WAH_SP_ADDR_I32, 8)
WAH_RUN(V128_LOAD16_LANE_i32_mem0) V128_LOAD_LANE_OP_MEM0(WAH_SP_ADDR_I32, 16)
WAH_RUN(V128_LOAD32_LANE_i32_mem0) V128_LOAD_LANE_OP_MEM0(WAH_SP_ADDR_I32, 32)
WAH_RUN(V128_LOAD64_LANE_i32_mem0) V128_LOAD_LANE_OP_MEM0(WAH_SP_ADDR_I32, 64)

WAH_RUN(V128_STORE) {
    uint32_t memidx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);
    uint64_t offset = wah_read_u64_le(bytecode_ip);
    bytecode_ip += sizeof(uint64_t);
    wah_v128_t val = (*--sp).v128;
    uint32_t addr = (uint32_t)(*--sp).i32;
    uint64_t effective_addr;
    WAH_ASSERT(memidx < ctx->memory_count && "validation didn't catch out-of-bound memory index");
    WAH_CHECK_GOTO(wah_check_effective_addr(addr, offset, sizeof(wah_v128_t), ctx->memory_sizes[memidx], &effective_addr), cleanup);
    memcpy(ctx->memories[memidx] + effective_addr, &val, sizeof(wah_v128_t));
    WAH_NEXT();
    WAH_CLEANUP();
}

WAH_RUN(V128_STORE_i32_mem0) {
    uint64_t offset = wah_read_u64_le(bytecode_ip);
    bytecode_ip += sizeof(uint64_t);
    wah_v128_t val = (*--sp).v128;
    uint32_t addr = (uint32_t)(*--sp).i32;
    uint64_t effective_addr;
    WAH_CHECK_GOTO(wah_check_effective_addr(addr, offset, sizeof(wah_v128_t), ctx->memory_size, &effective_addr), cleanup);
    memcpy(ctx->memory_base + effective_addr, &val, sizeof(wah_v128_t));
    WAH_NEXT();
    WAH_CLEANUP();
}

// --- i64-addressed V128 memory operations ---

WAH_RUN(V128_LOAD_i64) {
    V128_LOAD_COMMON(0, sizeof(wah_v128_t), WAH_SP_ADDR_I64);
    memcpy(&(*sp++).v128, ctx->memories[memidx] + effective_addr, sizeof(wah_v128_t));
    WAH_NEXT();
}

WAH_RUN(V128_LOAD_i64_mem0) {
    V128_LOAD_COMMON(1, sizeof(wah_v128_t), WAH_SP_ADDR_I64);
    memcpy(&(*sp++).v128, ctx->memory_base + effective_addr, sizeof(wah_v128_t));
    WAH_NEXT();
}

WAH_RUN(V128_LOAD8X8_S_i64) V128_LOAD_HALF_OP(WAH_SP_ADDR_I64, 8, i16, (int16_t)(int8_t))
WAH_RUN(V128_LOAD8X8_U_i64) V128_LOAD_HALF_OP(WAH_SP_ADDR_I64, 8, u16, (uint16_t))
WAH_RUN(V128_LOAD16X4_S_i64) V128_LOAD_HALF_OP(WAH_SP_ADDR_I64, 16, i32, (int32_t)(int16_t))
WAH_RUN(V128_LOAD16X4_U_i64) V128_LOAD_HALF_OP(WAH_SP_ADDR_I64, 16, u32, (uint32_t))
WAH_RUN(V128_LOAD32X2_S_i64) V128_LOAD_HALF_OP(WAH_SP_ADDR_I64, 32, i64, (int64_t)(int32_t))
WAH_RUN(V128_LOAD32X2_U_i64) V128_LOAD_HALF_OP(WAH_SP_ADDR_I64, 32, u64, (uint64_t))

WAH_RUN(V128_LOAD8X8_S_i64_mem0) V128_LOAD_HALF_OP_MEM0(WAH_SP_ADDR_I64, 8, i16, (int16_t)(int8_t))
WAH_RUN(V128_LOAD8X8_U_i64_mem0) V128_LOAD_HALF_OP_MEM0(WAH_SP_ADDR_I64, 8, u16, (uint16_t))
WAH_RUN(V128_LOAD16X4_S_i64_mem0) V128_LOAD_HALF_OP_MEM0(WAH_SP_ADDR_I64, 16, i32, (int32_t)(int16_t))
WAH_RUN(V128_LOAD16X4_U_i64_mem0) V128_LOAD_HALF_OP_MEM0(WAH_SP_ADDR_I64, 16, u32, (uint32_t))
WAH_RUN(V128_LOAD32X2_S_i64_mem0) V128_LOAD_HALF_OP_MEM0(WAH_SP_ADDR_I64, 32, i64, (int64_t)(int32_t))
WAH_RUN(V128_LOAD32X2_U_i64_mem0) V128_LOAD_HALF_OP_MEM0(WAH_SP_ADDR_I64, 32, u64, (uint64_t))

WAH_RUN(V128_LOAD8_SPLAT_i64) V128_LOAD_SPLAT_OP(WAH_SP_ADDR_I64, 8)
WAH_RUN(V128_LOAD16_SPLAT_i64) V128_LOAD_SPLAT_OP(WAH_SP_ADDR_I64, 16)
WAH_RUN(V128_LOAD32_SPLAT_i64) V128_LOAD_SPLAT_OP(WAH_SP_ADDR_I64, 32)
WAH_RUN(V128_LOAD64_SPLAT_i64) V128_LOAD_SPLAT_OP(WAH_SP_ADDR_I64, 64)

WAH_RUN(V128_LOAD8_SPLAT_i64_mem0) V128_LOAD_SPLAT_OP_MEM0(WAH_SP_ADDR_I64, 8)
WAH_RUN(V128_LOAD16_SPLAT_i64_mem0) V128_LOAD_SPLAT_OP_MEM0(WAH_SP_ADDR_I64, 16)
WAH_RUN(V128_LOAD32_SPLAT_i64_mem0) V128_LOAD_SPLAT_OP_MEM0(WAH_SP_ADDR_I64, 32)
WAH_RUN(V128_LOAD64_SPLAT_i64_mem0) V128_LOAD_SPLAT_OP_MEM0(WAH_SP_ADDR_I64, 64)

WAH_RUN(V128_LOAD32_ZERO_i64) {
    V128_LOAD_COMMON(0, 4, WAH_SP_ADDR_I64);
    wah_v128_t *v = &(*sp++).v128;
    *v = (wah_v128_t){.u64 = {wah_read_u32_le(ctx->memories[memidx] + effective_addr), 0}};
    WAH_NEXT();
}
WAH_RUN(V128_LOAD64_ZERO_i64) {
    V128_LOAD_COMMON(0, 8, WAH_SP_ADDR_I64);
    wah_v128_t *v = &(*sp++).v128;
    *v = (wah_v128_t){.u64 = {wah_read_u64_le(ctx->memories[memidx] + effective_addr), 0}};
    WAH_NEXT();
}

WAH_RUN(V128_LOAD32_ZERO_i64_mem0) {
    V128_LOAD_COMMON(1, 4, WAH_SP_ADDR_I64);
    wah_v128_t *v = &(*sp++).v128;
    *v = (wah_v128_t){.u64 = {wah_read_u32_le(ctx->memory_base + effective_addr), 0}};
    WAH_NEXT();
}
WAH_RUN(V128_LOAD64_ZERO_i64_mem0) {
    V128_LOAD_COMMON(1, 8, WAH_SP_ADDR_I64);
    wah_v128_t *v = &(*sp++).v128;
    *v = (wah_v128_t){.u64 = {wah_read_u64_le(ctx->memory_base + effective_addr), 0}};
    WAH_NEXT();
}

WAH_RUN(V128_LOAD8_LANE_i64) V128_LOAD_LANE_OP(WAH_SP_ADDR_I64, 8)
WAH_RUN(V128_LOAD16_LANE_i64) V128_LOAD_LANE_OP(WAH_SP_ADDR_I64, 16)
WAH_RUN(V128_LOAD32_LANE_i64) V128_LOAD_LANE_OP(WAH_SP_ADDR_I64, 32)
WAH_RUN(V128_LOAD64_LANE_i64) V128_LOAD_LANE_OP(WAH_SP_ADDR_I64, 64)

WAH_RUN(V128_LOAD8_LANE_i64_mem0) V128_LOAD_LANE_OP_MEM0(WAH_SP_ADDR_I64, 8)
WAH_RUN(V128_LOAD16_LANE_i64_mem0) V128_LOAD_LANE_OP_MEM0(WAH_SP_ADDR_I64, 16)
WAH_RUN(V128_LOAD32_LANE_i64_mem0) V128_LOAD_LANE_OP_MEM0(WAH_SP_ADDR_I64, 32)
WAH_RUN(V128_LOAD64_LANE_i64_mem0) V128_LOAD_LANE_OP_MEM0(WAH_SP_ADDR_I64, 64)

WAH_RUN(V128_STORE_i64) {
    uint32_t memidx = wah_read_u32_le(bytecode_ip);
    bytecode_ip += sizeof(uint32_t);
    uint64_t offset = wah_read_u64_le(bytecode_ip);
    bytecode_ip += sizeof(uint64_t);
    wah_v128_t val = (*--sp).v128;
    uint64_t addr = (uint64_t)(*--sp).i64;
    uint64_t effective_addr;
    WAH_ASSERT(memidx < ctx->memory_count && "validation didn't catch out-of-bound memory index");
    WAH_CHECK_GOTO(wah_check_effective_addr(addr, offset, sizeof(wah_v128_t), ctx->memory_sizes[memidx], &effective_addr), cleanup);
    memcpy(ctx->memories[memidx] + effective_addr, &val, sizeof(wah_v128_t));
    WAH_NEXT();
    WAH_CLEANUP();
}

WAH_RUN(V128_STORE_i64_mem0) {
    uint64_t offset = wah_read_u64_le(bytecode_ip);
    bytecode_ip += sizeof(uint64_t);
    wah_v128_t val = (*--sp).v128;
    uint64_t addr = (uint64_t)(*--sp).i64;
    uint64_t effective_addr;
    WAH_CHECK_GOTO(wah_check_effective_addr(addr, offset, sizeof(wah_v128_t), ctx->memory_size, &effective_addr), cleanup);
    memcpy(ctx->memory_base + effective_addr, &val, sizeof(wah_v128_t));
    WAH_NEXT();
    WAH_CLEANUP();
}

#undef WAH_SP_ADDR_I32
#undef WAH_SP_ADDR_I64
#undef V128_LOAD_COMMON
#undef V128_LOAD_HALF_OP
#undef V128_LOAD_HALF_OP_MEM0
#undef V128_LOAD_LANE_OP
#undef V128_LOAD_LANE_OP_MEM0
#undef V128_LOAD_SPLAT_OP
#undef V128_LOAD_SPLAT_OP_MEM0

#define EXTRACT_LANE_OP(VEC_TYPE, SCALAR_TYPE, LANE_COUNT) { \
    wah_v128_t vec = (*--sp).v128; \
    uint8_t laneidx = *bytecode_ip++; \
    WAH_ASSERT(laneidx < LANE_COUNT && "validation didn't catch out-of-bound lane index"); \
    wah_value_t result; \
    result.SCALAR_TYPE = vec.VEC_TYPE[laneidx]; \
    *sp++ = result; \
    WAH_NEXT(); \
}

#define REPLACE_LANE_OP(VEC_TYPE, C_VEC_TYPE, SCALAR_TYPE, LANE_COUNT) { \
    wah_value_t scalar_val = *--sp; \
    wah_v128_t vec = (*--sp).v128; \
    uint8_t laneidx = *bytecode_ip++; \
    WAH_ASSERT(laneidx < LANE_COUNT && "validation didn't catch out-of-bound lane index"); \
    vec.VEC_TYPE[laneidx] = (C_VEC_TYPE)scalar_val.SCALAR_TYPE; \
    (*sp++).v128 = vec; \
    WAH_NEXT(); \
}

#define SPLAT_OP(VEC_TYPE, C_VEC_TYPE, SCALAR_TYPE) { \
    wah_value_t scalar_val = *--sp; \
    wah_v128_t result = {0}; \
    for (uint32_t i = 0; i < sizeof(wah_v128_t) / sizeof(result.VEC_TYPE[0]); ++i) { \
        result.VEC_TYPE[i] = (C_VEC_TYPE)scalar_val.SCALAR_TYPE; \
    } \
    (*sp++).v128 = result; \
    WAH_NEXT(); \
}

WAH_RUN(I8X16_SHUFFLE) {
    wah_v128_t vec2 = (*--sp).v128;
    wah_v128_t vec1 = (*--sp).v128;
    wah_v128_t result = {0};
    for (uint32_t i = 0; i < 16; ++i) {
        uint8_t lane_idx = bytecode_ip[i];
        result.u8[i] = lane_idx < 16 ? vec1.u8[lane_idx] : vec2.u8[lane_idx - 16];
    }
    bytecode_ip += 16; // Advance past the shuffle mask
    (*sp++).v128 = result;
    WAH_NEXT();
}

WAH_RUN(I8X16_SWIZZLE) {
    wah_v128_t mask = (*--sp).v128;
    wah_v128_t data = (*--sp).v128;
    wah_v128_t result;
    for (uint32_t i = 0; i < 16; ++i) {
        uint8_t lane_idx = mask.u8[i];
        result.u8[i] = lane_idx < 16 ? data.u8[lane_idx] : 0;
    }
    (*sp++).v128 = result;
    WAH_NEXT();
}

WAH_RUN(I8X16_EXTRACT_LANE_S) EXTRACT_LANE_OP(i8, i32, 16)
WAH_RUN(I8X16_EXTRACT_LANE_U) EXTRACT_LANE_OP(u8, i32, 16)
WAH_RUN(I8X16_REPLACE_LANE) REPLACE_LANE_OP(i8, int8_t, i32, 16)
WAH_RUN(I16X8_EXTRACT_LANE_S) EXTRACT_LANE_OP(i16, i32, 8)
WAH_RUN(I16X8_EXTRACT_LANE_U) EXTRACT_LANE_OP(u16, i32, 8)
WAH_RUN(I16X8_REPLACE_LANE) REPLACE_LANE_OP(i16, int16_t, i32, 8)
WAH_RUN(I32X4_EXTRACT_LANE) EXTRACT_LANE_OP(i32, i32, 4)
WAH_RUN(I32X4_REPLACE_LANE) REPLACE_LANE_OP(i32, int32_t, i32, 4)
WAH_RUN(I64X2_EXTRACT_LANE) EXTRACT_LANE_OP(i64, i64, 2)
WAH_RUN(I64X2_REPLACE_LANE) REPLACE_LANE_OP(i64, int64_t, i64, 2)
WAH_RUN(F32X4_EXTRACT_LANE) EXTRACT_LANE_OP(f32, f32, 4)
WAH_RUN(F32X4_REPLACE_LANE) REPLACE_LANE_OP(f32, float, f32, 4)
WAH_RUN(F64X2_EXTRACT_LANE) EXTRACT_LANE_OP(f64, f64, 2)
WAH_RUN(F64X2_REPLACE_LANE) REPLACE_LANE_OP(f64, double, f64, 2)

WAH_RUN(I8X16_SPLAT) SPLAT_OP(i8, int8_t, i32)
WAH_RUN(I16X8_SPLAT) SPLAT_OP(i16, int16_t, i32)
WAH_RUN(I32X4_SPLAT) SPLAT_OP(i32, int32_t, i32)
WAH_RUN(I64X2_SPLAT) SPLAT_OP(i64, int64_t, i64)
WAH_RUN(F32X4_SPLAT) SPLAT_OP(f32, float, f32)
WAH_RUN(F64X2_SPLAT) SPLAT_OP(f64, double, f64)

#undef EXTRACT_LANE_OP
#undef REPLACE_LANE_OP
#undef SPLAT_OP

#define V128_UNARY_OP(op) { sp[-1].v128 = op(sp[-1].v128); WAH_NEXT(); }
#define V128_BINARY_OP(op) { sp[-2].v128 = op(sp[-2].v128, sp[-1].v128); sp--; WAH_NEXT(); }
#define V128_BINARY_OP_LANE(N, op, field) { \
    for (int i = 0; i < 128/N; ++i) { \
        sp[-2].v128.field[i] = sp[-2].v128.field[i] op sp[-1].v128.field[i]; \
    } \
    sp--; \
    WAH_NEXT(); \
}
#define V128_BINARY_OP_LANE_SAT_S(N, op, field, min_val, max_val) { \
    for (int i = 0; i < 128/N; ++i) { \
        int64_t res = (int64_t)sp[-2].v128.field[i] op (int64_t)sp[-1].v128.field[i]; \
        if (res < min_val) res = min_val; \
        if (res > max_val) res = max_val; \
        sp[-2].v128.field[i] = (int##N##_t)res; \
    } \
    sp--; \
    WAH_NEXT(); \
}
#define V128_BINARY_OP_LANE_SAT_U(N, op, field, max_val) { \
    for (int i = 0; i < 128/N; ++i) { \
        int64_t res = (uint64_t)sp[-2].v128.field[i] op (uint64_t)sp[-1].v128.field[i]; \
        if (res < 0) res = 0; \
        if (res > max_val) res = max_val; \
        sp[-2].v128.field[i] = (uint##N##_t)res; \
    } \
    sp--; \
    WAH_NEXT(); \
}
#define V128_BINARY_OP_LANE_F(N, op, field) { \
    for (int i = 0; i < 128/N; ++i) { \
        sp[-2].v128.field[i] = wah_canonicalize_##field(sp[-2].v128.field[i] op sp[-1].v128.field[i]); \
    } \
    sp--; \
    WAH_NEXT(); \
}

#define V128_CMP_I_LANE(N, op, field) { \
    wah_v128_t b = sp[-1].v128, a = sp[-2].v128; \
    for (int i = 0; i < 128/N; ++i) { \
        a.field[i] = (a.field[i] op b.field[i]) ? (~0ULL >> (64 - N)) : 0; \
    } \
    sp[-2].v128 = a; sp--; WAH_NEXT(); \
}
#define V128_CMP_I_LANE_S(N, op, field) { \
    wah_v128_t b = sp[-1].v128, a = sp[-2].v128; \
    for (int i = 0; i < 128/N; ++i) { \
        a.field[i] = (((int##N##_t)a.field[i]) op ((int##N##_t)b.field[i])) ? (~0ULL >> (64 - N)) : 0; \
    } \
    sp[-2].v128 = a; sp--; WAH_NEXT(); \
}
#define V128_CMP_I_LANE_U(N, op, field) { \
    wah_v128_t b = sp[-1].v128, a = sp[-2].v128; \
    for (int i = 0; i < 128/N; ++i) { \
        a.field[i] = (((uint##N##_t)a.field[i]) op ((uint##N##_t)b.field[i])) ? (~0ULL >> (64 - N)) : 0; \
    } \
    sp[-2].v128 = a; sp--; WAH_NEXT(); \
}
#define V128_CMP_F_LANE(N, op, field) { \
    wah_v128_t *a_ptr = &sp[-2].v128; \
    wah_v128_t b = sp[-1].v128; \
    for (int i = 0; i < 128/N; ++i) a_ptr->i##N[i] = (a_ptr->field[i] op b.field[i]) ? -1 : 0; \
    sp--; WAH_NEXT(); \
}

#define V128_TRUNC_SAT_PAIR_OP(fn, dest_field, src_field, K) { \
    wah_v128_t b = sp[-1].v128, a = sp[-2].v128, result; \
    for (int i = 0; i < K; ++i) { \
        result.dest_field[i] = fn(a.src_field[i]); \
        result.dest_field[i+K] = fn(b.src_field[i]); \
    } \
    sp[-2].v128 = result; sp--; WAH_NEXT(); \
}

#define V128_TRUNC_SAT_F2I_OP(N, F_FIELD, I_FIELD, TRUNC_FUNC) { \
    wah_v128_t val = sp[-1].v128; \
    for (int i = 0; i < 128/N; ++i) val.I_FIELD[i] = TRUNC_FUNC(val.F_FIELD[i]); \
    sp[-1].v128 = val; \
    WAH_NEXT(); \
}

#define V128_TRUNC_SAT_F2I_ZERO_OP(N, F_FIELD, I_FIELD, TRUNC_FUNC) { \
    wah_v128_t val = sp[-1].v128; \
    wah_v128_t result = { .I_FIELD = {TRUNC_FUNC(val.F_FIELD[0]), TRUNC_FUNC(val.F_FIELD[1])} }; \
    sp[-1].v128 = result; \
    WAH_NEXT(); \
}

#define V128_TERNARY_OP_F(N, op1, op2, field) { \
    WAH_PRAGMA_FP_CONTRACT_OFF() /* No FMA allowed here */ \
    wah_v128_t c = (*--sp).v128; \
    wah_v128_t b = (*--sp).v128; \
    wah_v128_t a = (*--sp).v128; \
    wah_v128_t result; \
    for (int i = 0; i < 128/N; ++i) { \
        result.field[i] = wah_canonicalize_##field(a.field[i] op1 b.field[i] op2 c.field[i]); \
    } \
    (*sp++).v128 = result; \
    WAH_NEXT(); \
}

#define V128_LANESELECT_OP(N, U_FIELD) { \
    wah_v128_t v3 = (*--sp).v128; \
    wah_v128_t v2 = (*--sp).v128; \
    wah_v128_t v1 = (*--sp).v128; \
    wah_v128_t result; \
    for (int i = 0; i < 128/N; ++i) { \
        result.U_FIELD[i] = (v1.U_FIELD[i] & v2.U_FIELD[i]) | (~v1.U_FIELD[i] & v3.U_FIELD[i]); \
    } \
    (*sp++).v128 = result; \
    WAH_NEXT(); \
}

WAH_RUN(V128_NOT) M128I_UNARY_OP(wah_v128_not_sse2, N128_UNARY_OP(vmvnq_u8, {
    wah_v128_t val = sp[-1].v128;
    for (int i = 0; i < 16; ++i) val.u8[i] = ~val.u8[i];
    sp[-1].v128 = val;
    WAH_NEXT();
}))
WAH_RUN(V128_AND) M128I_BINARY_OP(_mm_and_si128, N128_BINARY_OP(vandq_u8, {
    wah_v128_t b = sp[-1].v128;
    wah_v128_t a = sp[-2].v128;
    for (int i = 0; i < 16; ++i) a.u8[i] &= b.u8[i];
    sp[-2].v128 = a;
    sp--;
    WAH_NEXT();
}))
WAH_RUN(V128_ANDNOT) M128I_BINARY_OP(wah_v128_andnot_rev_sse2, N128_BINARY_OP(wah_v128_andnot_neon, {
    wah_v128_t b = sp[-1].v128;
    wah_v128_t a = sp[-2].v128;
    for (int i = 0; i < 16; ++i) a.u8[i] &= ~b.u8[i];
    sp[-2].v128 = a;
    sp--;
    WAH_NEXT();
}))
WAH_RUN(V128_OR) M128I_BINARY_OP(_mm_or_si128, N128_BINARY_OP(vorrq_u8, {
    wah_v128_t b = sp[-1].v128;
    wah_v128_t a = sp[-2].v128;
    for (int i = 0; i < 16; ++i) a.u8[i] |= b.u8[i];
    sp[-2].v128 = a;
    sp--;
    WAH_NEXT();
}))
WAH_RUN(V128_XOR) M128I_BINARY_OP(_mm_xor_si128, N128_BINARY_OP(veorq_u8, {
    wah_v128_t b = sp[-1].v128;
    wah_v128_t a = sp[-2].v128;
    for (int i = 0; i < 16; ++i) a.u8[i] ^= b.u8[i];
    sp[-2].v128 = a;
    sp--;
    WAH_NEXT();
}))

WAH_RUN(I8X16_ADD) M128I_BINARY_OP(_mm_add_epi8, N128_BINARY_OP(wah_vaddq_s8, V128_BINARY_OP_LANE(8, +, i8)))
WAH_RUN(I8X16_ADD_SAT_S) M128I_BINARY_OP(_mm_adds_epi8, N128_BINARY_OP(wah_vqaddq_s8, V128_BINARY_OP_LANE_SAT_S(8, +, i8, -128, 127)))
WAH_RUN(I8X16_ADD_SAT_U) M128I_BINARY_OP(_mm_adds_epu8, N128_BINARY_OP(wah_vqaddq_u8, V128_BINARY_OP_LANE_SAT_U(8, +, u8, 255)))
WAH_RUN(I8X16_SUB) M128I_BINARY_OP(_mm_sub_epi8, N128_BINARY_OP(wah_vsubq_s8, V128_BINARY_OP_LANE(8, -, i8)))
WAH_RUN(I8X16_SUB_SAT_S) M128I_BINARY_OP(_mm_subs_epi8, N128_BINARY_OP(wah_vqsubq_s8, V128_BINARY_OP_LANE_SAT_S(8, -, i8, -128, 127)))
WAH_RUN(I8X16_SUB_SAT_U) M128I_BINARY_OP(_mm_subs_epu8, N128_BINARY_OP(wah_vqsubq_u8, V128_BINARY_OP_LANE_SAT_U(8, -, u8, 255)))

WAH_RUN(I16X8_ADD) M128I_BINARY_OP(_mm_add_epi16, N128_BINARY_OP(wah_vaddq_s16, V128_BINARY_OP_LANE(16, +, i16)))
WAH_RUN(I16X8_ADD_SAT_S) M128I_BINARY_OP(_mm_adds_epi16, N128_BINARY_OP(wah_vqaddq_s16, V128_BINARY_OP_LANE_SAT_S(16, +, i16, -32768, 32767)))
WAH_RUN(I16X8_ADD_SAT_U) M128I_BINARY_OP(_mm_adds_epu16, N128_BINARY_OP(wah_vqaddq_u16, V128_BINARY_OP_LANE_SAT_U(16, +, u16, 65535)))
WAH_RUN(I16X8_SUB) M128I_BINARY_OP(_mm_sub_epi16, N128_BINARY_OP(wah_vsubq_s16, V128_BINARY_OP_LANE(16, -, i16)))
WAH_RUN(I16X8_SUB_SAT_S) M128I_BINARY_OP(_mm_subs_epi16, N128_BINARY_OP(wah_vqsubq_s16, V128_BINARY_OP_LANE_SAT_S(16, -, i16, -32768, 32767)))
WAH_RUN(I16X8_SUB_SAT_U) M128I_BINARY_OP(_mm_subs_epu16, N128_BINARY_OP(wah_vqsubq_u16, V128_BINARY_OP_LANE_SAT_U(16, -, u16, 65535)))
WAH_RUN(I16X8_MUL) M128I_BINARY_OP(_mm_mullo_epi16, N128_BINARY_OP(wah_vmulq_s16, V128_BINARY_OP_LANE(16, *, i16)))

WAH_RUN(I32X4_ADD) M128I_BINARY_OP(_mm_add_epi32, N128_BINARY_OP(wah_vaddq_s32, V128_BINARY_OP_LANE(32, +, i32)))
WAH_RUN(I32X4_SUB) M128I_BINARY_OP(_mm_sub_epi32, N128_BINARY_OP(wah_vsubq_s32, V128_BINARY_OP_LANE(32, -, i32)))
WAH_RUN(I32X4_MUL) M128I_BINARY_OP(wah_i32x4_mul_sse2, N128_BINARY_OP(wah_vmulq_s32, V128_BINARY_OP_LANE(32, *, i32)))

WAH_RUN(I64X2_ADD) M128I_BINARY_OP(_mm_add_epi64, N128_BINARY_OP(wah_vaddq_s64, V128_BINARY_OP_LANE(64, +, i64)))
WAH_RUN(I64X2_SUB) M128I_BINARY_OP(_mm_sub_epi64, N128_BINARY_OP(wah_vsubq_s64, V128_BINARY_OP_LANE(64, -, i64)))
WAH_RUN(I64X2_MUL) M128I_BINARY_OP(wah_i64x2_mul_sse2, V128_BINARY_OP_LANE(64, *, i64))

WAH_RUN(F32X4_ADD) M128_BINARY_OP(_mm_add_ps, N128_BINARY_OP(wah_vaddq_f32, V128_BINARY_OP_LANE_F(32, +, f32)))
WAH_RUN(F32X4_SUB) M128_BINARY_OP(_mm_sub_ps, N128_BINARY_OP(wah_vsubq_f32, V128_BINARY_OP_LANE_F(32, -, f32)))
WAH_RUN(F32X4_MUL) M128_BINARY_OP(_mm_mul_ps, N128_BINARY_OP(wah_vmulq_f32, V128_BINARY_OP_LANE_F(32, *, f32)))
WAH_RUN(F32X4_DIV) M128_BINARY_OP(_mm_div_ps, N128_BINARY_OP(wah_vdivq_f32, V128_BINARY_OP_LANE_F(32, /, f32)))

WAH_RUN(F64X2_ADD) M128D_BINARY_OP(_mm_add_pd, N128_BINARY_OP(wah_vaddq_f64, V128_BINARY_OP_LANE_F(64, +, f64)))
WAH_RUN(F64X2_SUB) M128D_BINARY_OP(_mm_sub_pd, N128_BINARY_OP(wah_vsubq_f64, V128_BINARY_OP_LANE_F(64, -, f64)))
WAH_RUN(F64X2_MUL) M128D_BINARY_OP(_mm_mul_pd, N128_BINARY_OP(wah_vmulq_f64, V128_BINARY_OP_LANE_F(64, *, f64)))
WAH_RUN(F64X2_DIV) M128D_BINARY_OP(_mm_div_pd, N128_BINARY_OP(wah_vdivq_f64, V128_BINARY_OP_LANE_F(64, /, f64)))

#define V128_UNARY_OP_LANE(N, op, field) { \
    wah_v128_t val = sp[-1].v128; \
    for (int i = 0; i < 128/N; ++i) val.field[i] = op(val.field[i]); \
    sp[-1].v128 = val; \
    WAH_NEXT(); \
}

#define V128_UNARY_OP_LANE_FN(N, fn, field) { \
    wah_v128_t val = sp[-1].v128; \
    for (int i = 0; i < 128/N; ++i) val.field[i] = fn(val.field[i]); \
    sp[-1].v128 = val; \
    WAH_NEXT(); \
}

#define V128_UNARY_OP_LANE_CAST_FN(N, fn, field, cast_type) { \
    wah_v128_t val = sp[-1].v128; \
    for (int i = 0; i < 128/N; ++i) val.field[i] = (field)fn((cast_type)val.field[i]); \
    sp[-1].v128 = val; \
    WAH_NEXT(); \
}

#define V128_BINARY_OP_LANE_FN(N, fn, field) { \
    wah_v128_t b = sp[-1].v128, a = sp[-2].v128; \
    for (int i = 0; i < 128/N; ++i) a.field[i] = fn(a.field[i], b.field[i]); \
    sp[-2].v128 = a; \
    sp--; \
    WAH_NEXT(); \
}

#define V128_SHIFT_OP_LANE(N, op, field) { \
    int32_t b = sp[-1].i32; \
    wah_v128_t a = sp[-2].v128; \
    for (int i = 0; i < 128/N; ++i) a.field[i] = a.field[i] op (b & (N - 1)); \
    sp[-2].v128 = a; \
    sp--; \
    WAH_NEXT(); \
}

#define V128_SHIFT_OP_LANE_U(N, op, field) { \
    int32_t b = sp[-1].i32; \
    wah_v128_t a = sp[-2].v128; \
    for (int i = 0; i < 128/N; ++i) { \
        a.field[i] = (uint##N##_t)((uint##N##_t)a.field[i] op (b & (N - 1))); \
    } \
    sp[-2].v128 = a; \
    sp--; \
    WAH_NEXT(); \
}

#define V128_ABS_OP(N, field, abs_func) { \
    wah_v128_t val = sp[-1].v128; \
    for (int i = 0; i < 128/N; ++i) val.field[i] = (int##N##_t)abs_func(val.field[i]); \
    sp[-1].v128 = val; \
    WAH_NEXT(); \
}

#define V128_ALL_TRUE_OP(N, field) { \
    wah_v128_t val = sp[-1].v128; \
    int32_t result = 1; \
    for (int i = 0; i < 128/N; ++i) { \
        if (val.field[i] == 0) { \
            result = 0; \
            break; \
        } \
    } \
    (*sp++).i32 = result; \
    WAH_NEXT(); \
}

#define V128_BITMASK_OP(N, field) { \
    wah_v128_t val = sp[-1].v128; \
    int32_t result = 0; \
    for (int i = 0; i < 128/N; ++i) { \
        if (val.field[i] < 0) { \
            result |= (1 << i); \
        } \
    } \
    (*sp++).i32 = result; \
    WAH_NEXT(); \
}

#define V128_EXTEND_LOW_OP(DST_N, DST_FIELD, SRC_N, SRC_FIELD, SIGN_TYPE) { \
    wah_v128_t val = sp[-1].v128; \
    wah_v128_t result; \
    for (int i = 0; i < 128/DST_N; ++i) result.DST_FIELD[i] = (SIGN_TYPE##DST_N##_t)val.SRC_FIELD[i]; \
    sp[-1].v128 = result; \
    WAH_NEXT(); \
}

#define V128_EXTEND_HIGH_OP(DST_N, DST_FIELD, SRC_N, SRC_FIELD, SIGN_TYPE) { \
    wah_v128_t val = sp[-1].v128; \
    wah_v128_t result; \
    for (int i = 0; i < 128/DST_N; ++i) result.DST_FIELD[i] = (SIGN_TYPE##DST_N##_t)val.SRC_FIELD[i + (128/SRC_N)/2]; \
    sp[-1].v128 = result; \
    WAH_NEXT(); \
}

#define V128_EXTMUL_LOW_OP(DST_N, DST_FIELD, INTERM_T, SRC_N, SRC_FIELD, SIGN_TYPE) { \
    wah_v128_t b = sp[-1].v128, a = sp[-2].v128; \
    wah_v128_t result; \
    for (int i = 0; i < 128/DST_N; ++i) { \
        result.DST_FIELD[i] = (SIGN_TYPE##DST_N##_t)((INTERM_T)a.SRC_FIELD[i] * (INTERM_T)b.SRC_FIELD[i]); \
    } \
    sp[-2].v128 = result; \
    sp--; \
    WAH_NEXT(); \
}

#define V128_EXTMUL_HIGH_OP(DST_N, DST_FIELD, INTERM_T, SRC_N, SRC_FIELD, SIGN_TYPE) { \
    wah_v128_t b = sp[-1].v128, a = sp[-2].v128; \
    wah_v128_t result; \
    int offset = (128/SRC_N)/2; \
    for (int i = 0; i < 128/DST_N; ++i) { \
        result.DST_FIELD[i] = (SIGN_TYPE##DST_N##_t)((INTERM_T)a.SRC_FIELD[i + offset] * (INTERM_T)b.SRC_FIELD[i + offset]); \
    } \
    sp[-2].v128 = result; \
    sp--; \
    WAH_NEXT(); \
}

WAH_RUN(I8X16_ABS) M128I_UNARY_OP(wah_i8x16_abs_sse2, N128_UNARY_OP(wah_vabsq_s8, V128_ABS_OP(8, i8, abs)))
WAH_RUN(I8X16_NEG)
    WAH_IF_X86_64({ sp[-1]._m128i = _mm_sub_epi8(_mm_setzero_si128(), sp[-1]._m128i); WAH_NEXT(); },
        N128_UNARY_OP(wah_vnegq_s8, V128_UNARY_OP_LANE(8, -, i8)))
WAH_RUN(I8X16_POPCNT) M128I_UNARY_OP(wah_i8x16_popcnt_sse2, N128_UNARY_OP(vcntq_u8, V128_UNARY_OP_LANE_FN(8, wah_popcount_u8, u8)))
WAH_RUN(I8X16_ALL_TRUE)
    WAH_IF_X86_64({ sp[-1].i32 = (_mm_movemask_epi8(_mm_cmpeq_epi8(sp[-1]._m128i, _mm_setzero_si128())) == 0xffff); WAH_NEXT(); },
        N128_UNARY_I32_OP(wah_all_true_i8x16_neon, V128_ALL_TRUE_OP(8, u8)))
WAH_RUN(I8X16_BITMASK) M128I_UNARY_I32_OP(_mm_movemask_epi8, N128_UNARY_I32_OP(wah_bitmask_i8x16_neon, V128_BITMASK_OP(8, i8)))
WAH_RUN(I8X16_NARROW_I16X8_S)
    M128I_BINARY_OP(_mm_packs_epi16, N128_BINARY_OP(wah_i8x16_narrow_i16x8_s_neon,
        V128_TRUNC_SAT_PAIR_OP(wah_trunc_sat_i16_to_i8, i8, i16, 8)))
WAH_RUN(I8X16_NARROW_I16X8_U)
    M128I_BINARY_OP(_mm_packus_epi16, N128_BINARY_OP(wah_i8x16_narrow_i16x8_u_neon,
        V128_TRUNC_SAT_PAIR_OP(wah_trunc_sat_i16_to_u8, u8, u16, 8)))
WAH_RUN(I8X16_SHL) M128I_SHIFT_OP(wah_i8x16_shl_sse2, -1, N128_SHIFT_OP(wah_i8x16_shl_neon, V128_SHIFT_OP_LANE(8, <<, i8)))
WAH_RUN(I8X16_SHR_S) M128I_SHIFT_OP(wah_i8x16_shr_s_sse2, -1, N128_SHIFT_OP(wah_i8x16_shr_s_neon, V128_SHIFT_OP_LANE(8, >>, i8)))
WAH_RUN(I8X16_SHR_U) M128I_SHIFT_OP(wah_i8x16_shr_u_sse2, -1, N128_SHIFT_OP(wah_i8x16_shr_u_neon, V128_SHIFT_OP_LANE_U(8, >>, u8)))
WAH_RUN(I8X16_MIN_S) M128I_BINARY_OP(wah_i8x16_min_s_sse2, N128_BINARY_OP(wah_vminq_s8, V128_BINARY_OP_LANE_FN(8, WAH_MIN_S_8, i8)))
WAH_RUN(I8X16_MIN_U) M128I_BINARY_OP(_mm_min_epu8, N128_BINARY_OP(wah_vminq_u8, V128_BINARY_OP_LANE_FN(8, WAH_MIN_U_8, u8)))
WAH_RUN(I8X16_MAX_S) M128I_BINARY_OP(wah_i8x16_max_s_sse2, N128_BINARY_OP(wah_vmaxq_s8, V128_BINARY_OP_LANE_FN(8, WAH_MAX_S_8, i8)))
WAH_RUN(I8X16_MAX_U) M128I_BINARY_OP(_mm_max_epu8, N128_BINARY_OP(wah_vmaxq_u8, V128_BINARY_OP_LANE_FN(8, WAH_MAX_U_8, u8)))
WAH_RUN(I8X16_AVGR_U) M128I_BINARY_OP(_mm_avg_epu8, N128_BINARY_OP(vrhaddq_u8, {
    wah_v128_t b = sp[-1].v128;
    wah_v128_t a = sp[-2].v128;
    for (int i = 0; i < 16; ++i) {
        a.u8[i] = (uint8_t)(((uint16_t)a.u8[i] + (uint16_t)b.u8[i] + 1) >> 1);
    }
    sp[-2].v128 = a;
    sp--;
    WAH_NEXT();
}))

WAH_RUN(I16X8_EXTADD_PAIRWISE_I8X16_S) N128_UNARY_OP(wah_extadd_pairwise_s8_neon, {
    wah_v128_t val = sp[-1].v128;
    wah_v128_t result;
    for (int i = 0; i < 8; ++i) {
        result.i16[i] = (int16_t)val.i8[i*2] + (int16_t)val.i8[i*2+1];
    }
    sp[-1].v128 = result;
    WAH_NEXT();
})
WAH_RUN(I16X8_EXTADD_PAIRWISE_I8X16_U) N128_UNARY_OP(wah_extadd_pairwise_u8_neon, {
    wah_v128_t val = sp[-1].v128;
    wah_v128_t result;
    for (int i = 0; i < 8; ++i) {
        result.u16[i] = (uint16_t)val.u8[i*2] + (uint16_t)val.u8[i*2+1];
    }
    sp[-1].v128 = result;
    WAH_NEXT();
})
WAH_RUN(I16X8_ABS) M128I_UNARY_OP(wah_i16x8_abs_sse2, N128_UNARY_OP(wah_vabsq_s16, V128_ABS_OP(16, i16, abs)))
WAH_RUN(I16X8_NEG)
    WAH_IF_X86_64({ sp[-1]._m128i = _mm_sub_epi16(_mm_setzero_si128(), sp[-1]._m128i); WAH_NEXT(); },
        N128_UNARY_OP(wah_vnegq_s16, V128_UNARY_OP_LANE(16, -, i16)))

WAH_RUN(I16X8_Q15MULR_SAT_S) {
    wah_v128_t b = sp[-1].v128, a = sp[-2].v128;
    sp[-2].v128 = wah_q15mulr_sat_s(a, b);
    sp--;
    WAH_NEXT();
}
WAH_RUN(I16X8_ALL_TRUE)
    WAH_IF_X86_64({ sp[-1].i32 = (_mm_movemask_epi8(_mm_cmpeq_epi16(sp[-1]._m128i, _mm_setzero_si128())) == 0xffff); WAH_NEXT(); },
        V128_ALL_TRUE_OP(16, u16))
WAH_RUN(I16X8_BITMASK) M128I_UNARY_I32_OP(wah_i16x8_bitmask_sse2, N128_UNARY_I32_OP(wah_bitmask_i16x8_neon, V128_BITMASK_OP(16, i16)))
WAH_RUN(I16X8_NARROW_I32X4_S)
    M128I_BINARY_OP(_mm_packs_epi32, N128_BINARY_OP(wah_i16x8_narrow_i32x4_s_neon,
        V128_TRUNC_SAT_PAIR_OP(wah_trunc_sat_i32_to_i16, i16, i32, 4)))
WAH_RUN(I16X8_NARROW_I32X4_U)
    M128I_BINARY_OP(wah_i16x8_narrow_i32x4_u_sse2, N128_BINARY_OP(wah_i16x8_narrow_i32x4_u_neon,
        V128_TRUNC_SAT_PAIR_OP(wah_trunc_sat_i32_to_u16, u16, i32, 4)))
WAH_RUN(I16X8_EXTEND_LOW_I8X16_S) N128_UNARY_OP(wah_i16x8_extend_low_i8x16_s_neon, V128_EXTEND_LOW_OP(16, i16, 8, i8, int))
WAH_RUN(I16X8_EXTEND_HIGH_I8X16_S) N128_UNARY_OP(wah_i16x8_extend_high_i8x16_s_neon, V128_EXTEND_HIGH_OP(16, i16, 8, i8, int))
WAH_RUN(I16X8_EXTEND_LOW_I8X16_U) N128_UNARY_OP(wah_i16x8_extend_low_i8x16_u_neon, V128_EXTEND_LOW_OP(16, u16, 8, u8, uint))
WAH_RUN(I16X8_EXTEND_HIGH_I8X16_U) N128_UNARY_OP(wah_i16x8_extend_high_i8x16_u_neon, V128_EXTEND_HIGH_OP(16, u16, 8, u8, uint))
WAH_RUN(I16X8_SHL) M128I_SHIFT_OP(_mm_slli_epi16, 15, N128_SHIFT_OP(wah_i16x8_shl_neon, V128_SHIFT_OP_LANE(16, <<, i16)))
WAH_RUN(I16X8_SHR_S) M128I_SHIFT_OP(_mm_srai_epi16, 15, N128_SHIFT_OP(wah_i16x8_shr_s_neon, V128_SHIFT_OP_LANE(16, >>, i16)))
WAH_RUN(I16X8_SHR_U) M128I_SHIFT_OP(_mm_srli_epi16, 15, N128_SHIFT_OP(wah_i16x8_shr_u_neon, V128_SHIFT_OP_LANE_U(16, >>, u16)))
WAH_RUN(I16X8_MIN_S) M128I_BINARY_OP(_mm_min_epi16, N128_BINARY_OP(wah_vminq_s16, V128_BINARY_OP_LANE_FN(16, WAH_MIN_S_16, i16)))
WAH_RUN(I16X8_MIN_U) M128I_BINARY_OP(wah_i16x8_min_u_sse2, N128_BINARY_OP(wah_vminq_u16, V128_BINARY_OP_LANE_FN(16, WAH_MIN_U_16, u16)))
WAH_RUN(I16X8_MAX_S) M128I_BINARY_OP(_mm_max_epi16, N128_BINARY_OP(wah_vmaxq_s16, V128_BINARY_OP_LANE_FN(16, WAH_MAX_S_16, i16)))
WAH_RUN(I16X8_MAX_U) M128I_BINARY_OP(wah_i16x8_max_u_sse2, N128_BINARY_OP(wah_vmaxq_u16, V128_BINARY_OP_LANE_FN(16, WAH_MAX_U_16, u16)))
WAH_RUN(I16X8_AVGR_U) M128I_BINARY_OP(_mm_avg_epu16, N128_BINARY_OP(wah_avgr_u16_neon, {
    wah_v128_t b = sp[-1].v128;
    wah_v128_t a = sp[-2].v128;
    for (int i = 0; i < 8; ++i) {
        a.u16[i] = (uint16_t)(((uint32_t)a.u16[i] + (uint32_t)b.u16[i] + 1) >> 1);
    }
    sp[-2].v128 = a;
    sp--;
    WAH_NEXT();
}))

WAH_RUN(I16X8_EXTMUL_LOW_I8X16_S) N128_BINARY_OP(wah_extmul_low_s8_neon, V128_EXTMUL_LOW_OP(16, i16, int32_t, 8, i8, int))
WAH_RUN(I16X8_EXTMUL_HIGH_I8X16_S) N128_BINARY_OP(wah_extmul_high_s8_neon, V128_EXTMUL_HIGH_OP(16, i16, int32_t, 8, i8, int))
WAH_RUN(I16X8_EXTMUL_LOW_I8X16_U) N128_BINARY_OP(wah_extmul_low_u8_neon, V128_EXTMUL_LOW_OP(16, u16, uint32_t, 8, u8, uint))
WAH_RUN(I16X8_EXTMUL_HIGH_I8X16_U) N128_BINARY_OP(wah_extmul_high_u8_neon, V128_EXTMUL_HIGH_OP(16, u16, uint32_t, 8, u8, uint))

WAH_RUN(I32X4_EXTADD_PAIRWISE_I16X8_S) N128_UNARY_OP(wah_extadd_pairwise_s16_neon, {
    wah_v128_t val = sp[-1].v128;
    wah_v128_t result;
    for (int i = 0; i < 4; ++i) {
        result.i32[i] = (int32_t)val.i16[i*2] + (int32_t)val.i16[i*2+1];
    }
    sp[-1].v128 = result;
    WAH_NEXT();
})
WAH_RUN(I32X4_EXTADD_PAIRWISE_I16X8_U) N128_UNARY_OP(wah_extadd_pairwise_u16_neon, {
    wah_v128_t val = sp[-1].v128;
    wah_v128_t result;
    for (int i = 0; i < 4; ++i) {
        result.u32[i] = (uint32_t)val.u16[i*2] + (uint32_t)val.u16[i*2+1];
    }
    sp[-1].v128 = result;
    WAH_NEXT();
})

WAH_RUN(I32X4_ABS) M128I_UNARY_OP(wah_i32x4_abs_sse2, N128_UNARY_OP(wah_vabsq_s32, V128_ABS_OP(32, i32, abs)))
WAH_RUN(I32X4_NEG)
    WAH_IF_X86_64({ sp[-1]._m128i = _mm_sub_epi32(_mm_setzero_si128(), sp[-1]._m128i); WAH_NEXT(); },
        N128_UNARY_OP(wah_vnegq_s32, V128_UNARY_OP_LANE(32, -, i32)))
WAH_RUN(I32X4_ALL_TRUE)
    WAH_IF_X86_64({ sp[-1].i32 = (_mm_movemask_epi8(_mm_cmpeq_epi32(sp[-1]._m128i, _mm_setzero_si128())) == 0xffff); WAH_NEXT(); },
        N128_UNARY_I32_OP(wah_all_true_i32x4_neon, V128_ALL_TRUE_OP(32, u32)))
WAH_RUN(I32X4_BITMASK) M128I_UNARY_I32_OP(wah_i32x4_bitmask_sse2, N128_UNARY_I32_OP(wah_bitmask_i32x4_neon, V128_BITMASK_OP(32, i32)))
WAH_RUN(I32X4_EXTEND_LOW_I16X8_S) N128_UNARY_OP(wah_i32x4_extend_low_i16x8_s_neon, V128_EXTEND_LOW_OP(32, i32, 16, i16, int))
WAH_RUN(I32X4_EXTEND_HIGH_I16X8_S) N128_UNARY_OP(wah_i32x4_extend_high_i16x8_s_neon, V128_EXTEND_HIGH_OP(32, i32, 16, i16, int))
WAH_RUN(I32X4_EXTEND_LOW_I16X8_U) N128_UNARY_OP(wah_i32x4_extend_low_i16x8_u_neon, V128_EXTEND_LOW_OP(32, u32, 16, u16, uint))
WAH_RUN(I32X4_EXTEND_HIGH_I16X8_U) N128_UNARY_OP(wah_i32x4_extend_high_i16x8_u_neon, V128_EXTEND_HIGH_OP(32, u32, 16, u16, uint))
WAH_RUN(I32X4_SHL) M128I_SHIFT_OP(_mm_slli_epi32, 31, N128_SHIFT_OP(wah_i32x4_shl_neon, V128_SHIFT_OP_LANE(32, <<, i32)))
WAH_RUN(I32X4_SHR_S) M128I_SHIFT_OP(_mm_srai_epi32, 31, N128_SHIFT_OP(wah_i32x4_shr_s_neon, V128_SHIFT_OP_LANE(32, >>, i32)))
WAH_RUN(I32X4_SHR_U) M128I_SHIFT_OP(_mm_srli_epi32, 31, N128_SHIFT_OP(wah_i32x4_shr_u_neon, V128_SHIFT_OP_LANE_U(32, >>, u32)))
WAH_RUN(I32X4_MIN_S) M128I_BINARY_OP(wah_i32x4_min_s_sse2, N128_BINARY_OP(wah_vminq_s32, V128_BINARY_OP_LANE_FN(32, WAH_MIN_S_32, i32)))
WAH_RUN(I32X4_MIN_U) M128I_BINARY_OP(wah_i32x4_min_u_sse2, N128_BINARY_OP(wah_vminq_u32, V128_BINARY_OP_LANE_FN(32, WAH_MIN_U_32, u32)))
WAH_RUN(I32X4_MAX_S) M128I_BINARY_OP(wah_i32x4_max_s_sse2, N128_BINARY_OP(wah_vmaxq_s32, V128_BINARY_OP_LANE_FN(32, WAH_MAX_S_32, i32)))
WAH_RUN(I32X4_MAX_U) M128I_BINARY_OP(wah_i32x4_max_u_sse2, N128_BINARY_OP(wah_vmaxq_u32, V128_BINARY_OP_LANE_FN(32, WAH_MAX_U_32, u32)))

WAH_RUN(I32X4_DOT_I16X8_S) M128I_BINARY_OP(_mm_madd_epi16, {
    wah_v128_t b = sp[-1].v128;
    wah_v128_t a = sp[-2].v128;
    wah_v128_t result;
    for (int i = 0; i < 4; ++i) {
        result.i32[i] = (int32_t)a.i16[i*2] * b.i16[i*2] + (int32_t)a.i16[i*2+1] * b.i16[i*2+1];
    }
    sp[-2].v128 = result;
    sp--;
    WAH_NEXT();
})
WAH_RUN(I32X4_EXTMUL_LOW_I16X8_S) N128_BINARY_OP(wah_extmul_low_s16_neon, V128_EXTMUL_LOW_OP(32, i32, int64_t, 16, i16, int))
WAH_RUN(I32X4_EXTMUL_HIGH_I16X8_S) N128_BINARY_OP(wah_extmul_high_s16_neon, V128_EXTMUL_HIGH_OP(32, i32, int64_t, 16, i16, int))
WAH_RUN(I32X4_EXTMUL_LOW_I16X8_U) N128_BINARY_OP(wah_extmul_low_u16_neon, V128_EXTMUL_LOW_OP(32, u32, uint64_t, 16, u16, uint))
WAH_RUN(I32X4_EXTMUL_HIGH_I16X8_U) N128_BINARY_OP(wah_extmul_high_u16_neon, V128_EXTMUL_HIGH_OP(32, u32, uint64_t, 16, u16, uint))

WAH_RUN(I64X2_ABS) M128I_UNARY_OP(wah_i64x2_abs_sse2, N128_UNARY_OP(wah_vabsq_s64, V128_ABS_OP(64, i64, llabs)))
WAH_RUN(I64X2_NEG)
    WAH_IF_X86_64({ sp[-1]._m128i = _mm_sub_epi64(_mm_setzero_si128(), sp[-1]._m128i); WAH_NEXT(); },
        N128_UNARY_OP(wah_vnegq_s64, V128_UNARY_OP_LANE(64, -, i64)))
WAH_RUN(I64X2_ALL_TRUE) M128I_UNARY_I32_OP(wah_i64x2_all_true_sse2, N128_UNARY_I32_OP(wah_all_true_i64x2_neon, V128_ALL_TRUE_OP(64, u64)))
WAH_RUN(I64X2_BITMASK) M128I_UNARY_I32_OP(wah_i64x2_bitmask_sse2, N128_UNARY_I32_OP(wah_bitmask_i64x2_neon, V128_BITMASK_OP(64, i64)))
WAH_RUN(I64X2_EXTEND_LOW_I32X4_S) N128_UNARY_OP(wah_i64x2_extend_low_i32x4_s_neon, V128_EXTEND_LOW_OP(64, i64, 32, i32, int))
WAH_RUN(I64X2_EXTEND_HIGH_I32X4_S) N128_UNARY_OP(wah_i64x2_extend_high_i32x4_s_neon, V128_EXTEND_HIGH_OP(64, i64, 32, i32, int))
WAH_RUN(I64X2_EXTEND_LOW_I32X4_U) N128_UNARY_OP(wah_i64x2_extend_low_i32x4_u_neon, V128_EXTEND_LOW_OP(64, u64, 32, u32, uint))
WAH_RUN(I64X2_EXTEND_HIGH_I32X4_U) N128_UNARY_OP(wah_i64x2_extend_high_i32x4_u_neon, V128_EXTEND_HIGH_OP(64, u64, 32, u32, uint))
WAH_RUN(I64X2_SHL) M128I_SHIFT_OP(_mm_slli_epi64, 63, N128_SHIFT_OP(wah_i64x2_shl_neon, V128_SHIFT_OP_LANE(64, <<, i64)))
WAH_RUN(I64X2_SHR_S) V128_SHIFT_OP_LANE(64, >>, i64)
WAH_RUN(I64X2_SHR_U) M128I_SHIFT_OP(_mm_srli_epi64, 63, N128_SHIFT_OP(wah_i64x2_shr_u_neon, V128_SHIFT_OP_LANE_U(64, >>, u64)))
WAH_RUN(I64X2_EXTMUL_LOW_I32X4_S) V128_EXTMUL_LOW_OP(64, i64, int64_t, 32, i32, int)
WAH_RUN(I64X2_EXTMUL_HIGH_I32X4_S) V128_EXTMUL_HIGH_OP(64, i64, int64_t, 32, i32, int)
WAH_RUN(I64X2_EXTMUL_LOW_I32X4_U) V128_EXTMUL_LOW_OP(64, u64, uint64_t, 32, u32, uint)
WAH_RUN(I64X2_EXTMUL_HIGH_I32X4_U) V128_EXTMUL_HIGH_OP(64, u64, uint64_t, 32, u32, uint)

WAH_RUN(F32X4_CEIL) N128_UNARY_OP(wah_vrndpq_f32, V128_UNARY_OP_LANE_FN(32, ceilf, f32))
WAH_RUN(F32X4_FLOOR) N128_UNARY_OP(wah_vrndmq_f32, V128_UNARY_OP_LANE_FN(32, floorf, f32))
WAH_RUN(F32X4_TRUNC) N128_UNARY_OP(wah_vrndq_f32, V128_UNARY_OP_LANE_FN(32, truncf, f32))
WAH_RUN(F32X4_NEAREST) N128_UNARY_OP(wah_vrndnq_f32, V128_UNARY_OP_LANE_FN(32, wah_nearest_f32, f32))
WAH_RUN(F32X4_ABS)
    WAH_IF_X86_64({ sp[-1]._m128 = _mm_andnot_ps(_mm_set1_ps(-0.0f), sp[-1]._m128); WAH_NEXT(); },
        N128_UNARY_OP(wah_vabsq_f32, V128_UNARY_OP_LANE_FN(32, fabsf, f32)))
WAH_RUN(F32X4_NEG)
    WAH_IF_X86_64({ sp[-1]._m128 = _mm_xor_ps(_mm_set1_ps(-0.0f), sp[-1]._m128); WAH_NEXT(); },
        N128_UNARY_OP(wah_vnegq_f32, V128_UNARY_OP_LANE(32, -, f32)))
WAH_RUN(F32X4_SQRT) M128_UNARY_OP(_mm_sqrt_ps, N128_UNARY_OP(wah_vsqrtq_f32, V128_UNARY_OP_LANE_FN(32, sqrtf, f32)))
WAH_RUN(F32X4_MIN) M128_BINARY_OP(_mm_min_ps, N128_BINARY_OP(wah_vminq_f32, V128_BINARY_OP_LANE_FN(32, fminf, f32)))
WAH_RUN(F32X4_MAX) M128_BINARY_OP(_mm_max_ps, N128_BINARY_OP(wah_vmaxq_f32, V128_BINARY_OP_LANE_FN(32, fmaxf, f32)))
WAH_RUN(F32X4_PMIN) M128_BINARY_OP(wah_f32x4_pmin_sse2, N128_BINARY_OP(wah_f32x4_pmin_neon, V128_BINARY_OP_LANE_FN(32, wah_pminf, f32)))
WAH_RUN(F32X4_PMAX) M128_BINARY_OP(wah_f32x4_pmax_sse2, N128_BINARY_OP(wah_f32x4_pmax_neon, V128_BINARY_OP_LANE_FN(32, wah_pmaxf, f32)))

WAH_RUN(F64X2_CEIL) N128_UNARY_OP(wah_vrndpq_f64, V128_UNARY_OP_LANE_FN(64, ceil, f64))
WAH_RUN(F64X2_FLOOR) N128_UNARY_OP(wah_vrndmq_f64, V128_UNARY_OP_LANE_FN(64, floor, f64))
WAH_RUN(F64X2_TRUNC) N128_UNARY_OP(wah_vrndq_f64, V128_UNARY_OP_LANE_FN(64, trunc, f64))
WAH_RUN(F64X2_NEAREST) N128_UNARY_OP(wah_vrndnq_f64, V128_UNARY_OP_LANE_FN(64, wah_nearest_f64, f64))
WAH_RUN(F64X2_ABS)
    WAH_IF_X86_64({ sp[-1]._m128d = _mm_andnot_pd(_mm_set1_pd(-0.0), sp[-1]._m128d); WAH_NEXT(); },
        N128_UNARY_OP(wah_vabsq_f64, V128_UNARY_OP_LANE_FN(64, fabs, f64)))
WAH_RUN(F64X2_NEG)
    WAH_IF_X86_64({ sp[-1]._m128d = _mm_xor_pd(_mm_set1_pd(-0.0), sp[-1]._m128d); WAH_NEXT(); },
        N128_UNARY_OP(wah_vnegq_f64, V128_UNARY_OP_LANE(64, -, f64)))
WAH_RUN(F64X2_SQRT) M128D_UNARY_OP(_mm_sqrt_pd, N128_UNARY_OP(wah_vsqrtq_f64, V128_UNARY_OP_LANE_FN(64, sqrt, f64)))
WAH_RUN(F64X2_MIN) M128D_BINARY_OP(_mm_min_pd, N128_BINARY_OP(wah_vminq_f64, V128_BINARY_OP_LANE_FN(64, fmin, f64)))
WAH_RUN(F64X2_MAX) M128D_BINARY_OP(_mm_max_pd, N128_BINARY_OP(wah_vmaxq_f64, V128_BINARY_OP_LANE_FN(64, fmax, f64)))
WAH_RUN(F64X2_PMIN) M128D_BINARY_OP(wah_f64x2_pmin_sse2, N128_BINARY_OP(wah_f64x2_pmin_neon, V128_BINARY_OP_LANE_FN(64, wah_pmin, f64)))
WAH_RUN(F64X2_PMAX) M128D_BINARY_OP(wah_f64x2_pmax_sse2, N128_BINARY_OP(wah_f64x2_pmax_neon, V128_BINARY_OP_LANE_FN(64, wah_pmax, f64)))

WAH_RUN(V128_BITSELECT) M128I_TERNARY_OP(_mm_or_si128(_mm_and_si128(a, b), _mm_andnot_si128(a, c)), N128_TERNARY_OP(vbslq_u8(a, b, c), {
    wah_v128_t v3 = sp[-1].v128;
    wah_v128_t v2 = sp[-2].v128;
    wah_v128_t v1 = sp[-3].v128;
    wah_v128_t result;
    for (int i = 0; i < 16; ++i) {
        result.u8[i] = (v1.u8[i] & v2.u8[i]) | (~v1.u8[i] & v3.u8[i]);
    }
    sp[-3].v128 = result;
    sp -= 2;
    WAH_NEXT();
}))

WAH_RUN(V128_ANY_TRUE) WAH_IF_X86_64({
    sp[-1].i32 = (_mm_movemask_epi8(_mm_cmpeq_epi8(sp[-1]._m128i, _mm_setzero_si128())) != 0xffff);
    WAH_NEXT();
}, N128_UNARY_I32_OP(wah_any_true_neon, {
    wah_v128_t val = sp[-1].v128;
    int32_t result = 0;
    for (int i = 0; i < 16; ++i) {
        if (val.u8[i] != 0) {
            result = 1;
            break;
        }
    }
    sp[-1].i32 = result;
    WAH_NEXT();
}))

WAH_RUN(I8X16_EQ) M128I_BINARY_OP(_mm_cmpeq_epi8, N128_BINARY_OP(wah_vceqq_s8, V128_CMP_I_LANE(8, ==, u8)))
WAH_RUN(I8X16_NE) M128I_NOT_BINARY_OP(_mm_cmpeq_epi8, N128_NOT_BINARY_OP(wah_vceqq_s8, V128_CMP_I_LANE(8, !=, u8)))
WAH_RUN(I8X16_LT_S) M128I_BINARY_OP(_mm_cmplt_epi8, N128_BINARY_OP(wah_vcltq_s8, V128_CMP_I_LANE_S(8, <, i8)))
WAH_RUN(I8X16_LT_U) M128I_BINARY_OP(wah_i8x16_lt_u_sse2, N128_BINARY_OP(wah_vcltq_u8, V128_CMP_I_LANE_U(8, <, u8)))
WAH_RUN(I8X16_GT_S) M128I_BINARY_OP(_mm_cmpgt_epi8, N128_BINARY_OP(wah_vcgtq_s8, V128_CMP_I_LANE_S(8, >, i8)))
WAH_RUN(I8X16_GT_U) M128I_BINARY_OP(wah_i8x16_gt_u_sse2, N128_BINARY_OP(wah_vcgtq_u8, V128_CMP_I_LANE_U(8, >, u8)))
WAH_RUN(I8X16_LE_S) M128I_NOT_BINARY_OP(_mm_cmpgt_epi8, N128_NOT_BINARY_OP(wah_vcgtq_s8, V128_CMP_I_LANE_S(8, <=, i8)))
WAH_RUN(I8X16_LE_U) M128I_NOT_BINARY_OP(wah_i8x16_gt_u_sse2, N128_NOT_BINARY_OP(wah_vcgtq_u8, V128_CMP_I_LANE_U(8, <=, u8)))
WAH_RUN(I8X16_GE_S) M128I_NOT_BINARY_OP(_mm_cmplt_epi8, N128_NOT_BINARY_OP(wah_vcltq_s8, V128_CMP_I_LANE_S(8, >=, i8)))
WAH_RUN(I8X16_GE_U) M128I_NOT_BINARY_OP(wah_i8x16_lt_u_sse2, N128_NOT_BINARY_OP(wah_vcltq_u8, V128_CMP_I_LANE_U(8, >=, u8)))

WAH_RUN(I16X8_EQ) M128I_BINARY_OP(_mm_cmpeq_epi16, N128_BINARY_OP(wah_vceqq_s16, V128_CMP_I_LANE(16, ==, u16)))
WAH_RUN(I16X8_NE) M128I_NOT_BINARY_OP(_mm_cmpeq_epi16, N128_NOT_BINARY_OP(wah_vceqq_s16, V128_CMP_I_LANE(16, !=, u16)))
WAH_RUN(I16X8_LT_S) M128I_BINARY_OP(_mm_cmplt_epi16, N128_BINARY_OP(wah_vcltq_s16, V128_CMP_I_LANE_S(16, <, i16)))
WAH_RUN(I16X8_LT_U) M128I_BINARY_OP(wah_i16x8_lt_u_sse2, N128_BINARY_OP(wah_vcltq_u16, V128_CMP_I_LANE_U(16, <, u16)))
WAH_RUN(I16X8_GT_S) M128I_BINARY_OP(_mm_cmpgt_epi16, N128_BINARY_OP(wah_vcgtq_s16, V128_CMP_I_LANE_S(16, >, i16)))
WAH_RUN(I16X8_GT_U) M128I_BINARY_OP(wah_i16x8_gt_u_sse2, N128_BINARY_OP(wah_vcgtq_u16, V128_CMP_I_LANE_U(16, >, u16)))
WAH_RUN(I16X8_LE_S) M128I_NOT_BINARY_OP(_mm_cmpgt_epi16, N128_NOT_BINARY_OP(wah_vcgtq_s16, V128_CMP_I_LANE_S(16, <=, i16)))
WAH_RUN(I16X8_LE_U) M128I_NOT_BINARY_OP(wah_i16x8_gt_u_sse2, N128_NOT_BINARY_OP(wah_vcgtq_u16, V128_CMP_I_LANE_U(16, <=, u16)))
WAH_RUN(I16X8_GE_S) M128I_NOT_BINARY_OP(_mm_cmplt_epi16, N128_NOT_BINARY_OP(wah_vcltq_s16, V128_CMP_I_LANE_S(16, >=, i16)))
WAH_RUN(I16X8_GE_U) M128I_NOT_BINARY_OP(wah_i16x8_lt_u_sse2, N128_NOT_BINARY_OP(wah_vcltq_u16, V128_CMP_I_LANE_U(16, >=, u16)))

WAH_RUN(I32X4_EQ) M128I_BINARY_OP(_mm_cmpeq_epi32, N128_BINARY_OP(wah_vceqq_s32, V128_CMP_I_LANE(32, ==, u32)))
WAH_RUN(I32X4_NE) M128I_NOT_BINARY_OP(_mm_cmpeq_epi32, N128_NOT_BINARY_OP(wah_vceqq_s32, V128_CMP_I_LANE(32, !=, u32)))
WAH_RUN(I32X4_LT_S) M128I_BINARY_OP(_mm_cmplt_epi32, N128_BINARY_OP(wah_vcltq_s32, V128_CMP_I_LANE_S(32, <, i32)))
WAH_RUN(I32X4_LT_U) M128I_BINARY_OP(wah_i32x4_lt_u_sse2, N128_BINARY_OP(wah_vcltq_u32, V128_CMP_I_LANE_U(32, <, u32)))
WAH_RUN(I32X4_GT_S) M128I_BINARY_OP(_mm_cmpgt_epi32, N128_BINARY_OP(wah_vcgtq_s32, V128_CMP_I_LANE_S(32, >, i32)))
WAH_RUN(I32X4_GT_U) M128I_BINARY_OP(wah_i32x4_gt_u_sse2, N128_BINARY_OP(wah_vcgtq_u32, V128_CMP_I_LANE_U(32, >, u32)))
WAH_RUN(I32X4_LE_S) M128I_NOT_BINARY_OP(_mm_cmpgt_epi32, N128_NOT_BINARY_OP(wah_vcgtq_s32, V128_CMP_I_LANE_S(32, <=, i32)))
WAH_RUN(I32X4_LE_U) M128I_NOT_BINARY_OP(wah_i32x4_gt_u_sse2, N128_NOT_BINARY_OP(wah_vcgtq_u32, V128_CMP_I_LANE_U(32, <=, u32)))
WAH_RUN(I32X4_GE_S) M128I_NOT_BINARY_OP(_mm_cmplt_epi32, N128_NOT_BINARY_OP(wah_vcltq_s32, V128_CMP_I_LANE_S(32, >=, i32)))
WAH_RUN(I32X4_GE_U) M128I_NOT_BINARY_OP(wah_i32x4_lt_u_sse2, N128_NOT_BINARY_OP(wah_vcltq_u32, V128_CMP_I_LANE_U(32, >=, u32)))

WAH_RUN(I64X2_EQ) N128_BINARY_OP(wah_vceqq_s64, V128_CMP_I_LANE(64, ==, u64))
WAH_RUN(I64X2_NE) N128_NOT_BINARY_OP(wah_vceqq_s64, V128_CMP_I_LANE(64, !=, u64))
WAH_RUN(I64X2_LT_S) N128_BINARY_OP(wah_vcltq_s64, V128_CMP_I_LANE_S(64, <, i64))
WAH_RUN(I64X2_GT_S) N128_BINARY_OP(wah_vcgtq_s64, V128_CMP_I_LANE_S(64, >, i64))
WAH_RUN(I64X2_LE_S) N128_NOT_BINARY_OP(wah_vcgtq_s64, V128_CMP_I_LANE_S(64, <=, i64))
WAH_RUN(I64X2_GE_S) N128_NOT_BINARY_OP(wah_vcltq_s64, V128_CMP_I_LANE_S(64, >=, i64))

WAH_RUN(F32X4_EQ) M128_BINARY_OP(_mm_cmpeq_ps, N128_BINARY_OP(wah_vceqq_f32, V128_CMP_F_LANE(32, ==, u32)))
WAH_RUN(F32X4_NE) M128_NOT_BINARY_OP(_mm_cmpeq_ps, N128_NOT_BINARY_OP(wah_vceqq_f32, V128_CMP_F_LANE(32, !=, u32)))
WAH_RUN(F32X4_LT) M128_BINARY_OP(_mm_cmplt_ps, N128_BINARY_OP(wah_vcltq_f32, V128_CMP_F_LANE(32, <, f32)))
WAH_RUN(F32X4_GT) M128_BINARY_OP(_mm_cmpgt_ps, N128_BINARY_OP(wah_vcgtq_f32, V128_CMP_F_LANE(32, >, f32)))
WAH_RUN(F32X4_LE) M128_NOT_BINARY_OP(_mm_cmpgt_ps, N128_NOT_BINARY_OP(wah_vcgtq_f32, V128_CMP_F_LANE(32, <=, f32)))
WAH_RUN(F32X4_GE) M128_NOT_BINARY_OP(_mm_cmplt_ps, N128_NOT_BINARY_OP(wah_vcltq_f32, V128_CMP_F_LANE(32, >=, f32)))

WAH_RUN(F64X2_EQ) M128D_BINARY_OP(_mm_cmpeq_pd, N128_BINARY_OP(wah_vceqq_f64, V128_CMP_F_LANE(64, ==, u64)))
WAH_RUN(F64X2_NE) M128D_NOT_BINARY_OP(_mm_cmpeq_pd, N128_NOT_BINARY_OP(wah_vceqq_f64, V128_CMP_F_LANE(64, !=, u64)))
WAH_RUN(F64X2_LT) M128D_BINARY_OP(_mm_cmplt_pd, N128_BINARY_OP(wah_vcltq_f64, V128_CMP_F_LANE(64, <, f64)))
WAH_RUN(F64X2_GT) M128D_BINARY_OP(_mm_cmpgt_pd, N128_BINARY_OP(wah_vcgtq_f64, V128_CMP_F_LANE(64, >, f64)))
WAH_RUN(F64X2_LE) M128D_NOT_BINARY_OP(_mm_cmpgt_pd, N128_NOT_BINARY_OP(wah_vcgtq_f64, V128_CMP_F_LANE(64, <=, f64)))
WAH_RUN(F64X2_GE) M128D_NOT_BINARY_OP(_mm_cmplt_pd, N128_NOT_BINARY_OP(wah_vcltq_f64, V128_CMP_F_LANE(64, >=, f64)))

WAH_RUN(I32X4_TRUNC_SAT_F32X4_S) N128_UNARY_OP(wah_i32x4_trunc_sat_f32x4_s_neon, {
    wah_v128_t val = sp[-1].v128;
    for (int i = 0; i < 4; ++i) val.i32[i] = wah_trunc_sat_f32_to_i32(val.f32[i]);
    sp[-1].v128 = val;
    WAH_NEXT();
})
WAH_RUN(I32X4_TRUNC_SAT_F32X4_U) N128_UNARY_OP(wah_i32x4_trunc_sat_f32x4_u_neon, {
    wah_v128_t val = sp[-1].v128;
    for (int i = 0; i < 4; ++i) val.u32[i] = wah_trunc_sat_f32_to_u32(val.f32[i]);
    sp[-1].v128 = val;
    WAH_NEXT();
})
WAH_RUN(F32X4_CONVERT_I32X4_S) WAH_IF_X86_64({
    sp[-1]._m128 = _mm_cvtepi32_ps(sp[-1]._m128i);
    WAH_NEXT();
}, N128_UNARY_OP(wah_f32x4_convert_i32x4_s_neon, {
    wah_v128_t val = sp[-1].v128;
    for (int i = 0; i < 4; ++i) val.f32[i] = (float)val.i32[i];
    sp[-1].v128 = val;
    WAH_NEXT();
}))
WAH_RUN(F32X4_CONVERT_I32X4_U) N128_UNARY_OP(wah_f32x4_convert_i32x4_u_neon, {
    wah_v128_t val = sp[-1].v128;
    for (int i = 0; i < 4; ++i) val.f32[i] = (float)val.u32[i];
    sp[-1].v128 = val;
    WAH_NEXT();
})
WAH_RUN(I32X4_TRUNC_SAT_F64X2_S_ZERO) {
    wah_v128_t val = sp[-1].v128;
    wah_v128_t result = { .i32 = {wah_trunc_sat_f64_to_i32(val.f64[0]), wah_trunc_sat_f64_to_i32(val.f64[1])} };
    sp[-1].v128 = result;
    WAH_NEXT();
}
WAH_RUN(I32X4_TRUNC_SAT_F64X2_U_ZERO) {
    wah_v128_t val = sp[-1].v128;
    wah_v128_t result = { .u32 = {wah_trunc_sat_f64_to_u32(val.f64[0]), wah_trunc_sat_f64_to_u32(val.f64[1])} };
    sp[-1].v128 = result;
    WAH_NEXT();
}
WAH_RUN(F64X2_CONVERT_LOW_I32X4_S) WAH_IF_X86_64({
    sp[-1]._m128d = _mm_cvtepi32_pd(sp[-1]._m128i);
    WAH_NEXT();
}, {
    wah_v128_t val = sp[-1].v128;
    wah_v128_t result = ((wah_v128_t) { .f64 = {(double)val.i32[0], (double)val.i32[1]} });
    sp[-1].v128 = result;
    WAH_NEXT();
})
WAH_RUN(F64X2_CONVERT_LOW_I32X4_U) {
    wah_v128_t val = sp[-1].v128;
    wah_v128_t result = ((wah_v128_t) { .f64 = {(double)val.u32[0], (double)val.u32[1]} });
    sp[-1].v128 = result;
    WAH_NEXT();
}
WAH_RUN(F32X4_DEMOTE_F64X2_ZERO) WAH_IF_X86_64({
    sp[-1]._m128 = wah_canonicalize_f32x4_sse2(_mm_cvtpd_ps(sp[-1]._m128d));
    WAH_NEXT();
}, {
    wah_v128_t val = sp[-1].v128;
    wah_v128_t result = ((wah_v128_t) { .f32 = {wah_canonicalize_f32((float)val.f64[0]), wah_canonicalize_f32((float)val.f64[1])} });
    sp[-1].v128 = result;
    WAH_NEXT();
})
WAH_RUN(F64X2_PROMOTE_LOW_F32X4) WAH_IF_X86_64({
    sp[-1]._m128d = wah_canonicalize_f64x2_sse2(_mm_cvtps_pd(sp[-1]._m128));
    WAH_NEXT();
}, {
    wah_v128_t val = sp[-1].v128;
    wah_v128_t result = ((wah_v128_t) { .f64 = {wah_canonicalize_f64((double)val.f32[0]), wah_canonicalize_f64((double)val.f32[1])} });
    sp[-1].v128 = result;
    WAH_NEXT();
})

WAH_RUN(I8X16_RELAXED_SWIZZLE) N128_BINARY_OP(vqtbl1q_u8, {
    wah_v128_t mask = sp[-2].v128;
    wah_v128_t data = sp[-1].v128;
    wah_v128_t result;
    for (uint32_t i = 0; i < 16; ++i) {
        uint8_t lane_idx = mask.u8[i];
        result.u8[i] = lane_idx < 16 ? data.u8[lane_idx] : 0;
    }
    sp[-2].v128 = result;
    sp--;
    WAH_NEXT();
})

WAH_RUN(I32X4_RELAXED_TRUNC_F32X4_S) V128_TRUNC_SAT_F2I_OP(32, f32, i32, wah_trunc_sat_f32_to_i32)
WAH_RUN(I32X4_RELAXED_TRUNC_F32X4_U) V128_TRUNC_SAT_F2I_OP(32, f32, u32, wah_trunc_sat_f32_to_u32)
WAH_RUN(I32X4_RELAXED_TRUNC_F64X2_S) V128_TRUNC_SAT_F2I_ZERO_OP(64, f64, i32, wah_trunc_sat_f64_to_i32)
WAH_RUN(I32X4_RELAXED_TRUNC_F64X2_U) V128_TRUNC_SAT_F2I_ZERO_OP(64, f64, u32, wah_trunc_sat_f64_to_u32)

WAH_RUN(F32X4_RELAXED_MADD)
    M128_TERNARY_OP(_mm_add_ps(_mm_mul_ps(a, b), c), N128_TERNARY_OP(wah_vaddq_f32(wah_vmulq_f32(a, b), c), V128_TERNARY_OP_F(32, *, +, f32)))
WAH_RUN(F32X4_RELAXED_NMADD)
    WAH_IF_X86_64({ __m128 c = sp[-1]._m128; __m128 b = sp[-2]._m128; __m128 a = sp[-3]._m128;
                    sp[-3]._m128 = wah_canonicalize_f32x4_sse2(_mm_sub_ps(c, _mm_mul_ps(a, b))); sp -= 2; WAH_NEXT(); },
        N128_TERNARY_OP(wah_canonicalize_f32x4_neon(wah_vsubq_f32(c, wah_vmulq_f32(a, b))), V128_TERNARY_OP_F(32, *-, +, f32)))
WAH_RUN(F64X2_RELAXED_MADD)
    M128D_TERNARY_OP(_mm_add_pd(_mm_mul_pd(a, b), c), N128_TERNARY_OP(wah_vaddq_f64(wah_vmulq_f64(a, b), c), V128_TERNARY_OP_F(64, *, +, f64)))
WAH_RUN(F64X2_RELAXED_NMADD)
    WAH_IF_X86_64({ __m128d c = sp[-1]._m128d; __m128d b = sp[-2]._m128d; __m128d a = sp[-3]._m128d;
                    sp[-3]._m128d = wah_canonicalize_f64x2_sse2(_mm_sub_pd(c, _mm_mul_pd(a, b))); sp -= 2; WAH_NEXT(); },
        N128_TERNARY_OP(wah_canonicalize_f64x2_neon(wah_vsubq_f64(c, wah_vmulq_f64(a, b))), V128_TERNARY_OP_F(64, *-, +, f64)))

WAH_RUN(I8X16_RELAXED_LANESELECT)
    M128I_TERNARY_OP(_mm_or_si128(_mm_and_si128(a, b), _mm_andnot_si128(a, c)),
        N128_TERNARY_OP(vbslq_u8(a, b, c), V128_LANESELECT_OP(8, u8)))
WAH_RUN(I16X8_RELAXED_LANESELECT)
    M128I_TERNARY_OP(_mm_or_si128(_mm_and_si128(a, b), _mm_andnot_si128(a, c)),
        N128_TERNARY_OP(vbslq_u8(a, b, c), V128_LANESELECT_OP(16, u16)))
WAH_RUN(I32X4_RELAXED_LANESELECT)
    M128I_TERNARY_OP(_mm_or_si128(_mm_and_si128(a, b), _mm_andnot_si128(a, c)),
        N128_TERNARY_OP(vbslq_u8(a, b, c), V128_LANESELECT_OP(32, u32)))
WAH_RUN(I64X2_RELAXED_LANESELECT)
    M128I_TERNARY_OP(_mm_or_si128(_mm_and_si128(a, b), _mm_andnot_si128(a, c)),
        N128_TERNARY_OP(vbslq_u8(a, b, c), V128_LANESELECT_OP(64, u64)))

WAH_RUN(F32X4_RELAXED_MIN) M128_BINARY_OP(_mm_min_ps, N128_BINARY_OP(wah_vminq_f32, V128_BINARY_OP_LANE_FN(32, fminf, f32)))
WAH_RUN(F32X4_RELAXED_MAX) M128_BINARY_OP(_mm_max_ps, N128_BINARY_OP(wah_vmaxq_f32, V128_BINARY_OP_LANE_FN(32, fmaxf, f32)))
WAH_RUN(F64X2_RELAXED_MIN) M128D_BINARY_OP(_mm_min_pd, N128_BINARY_OP(wah_vminq_f64, V128_BINARY_OP_LANE_FN(64, fmin, f64)))
WAH_RUN(F64X2_RELAXED_MAX) M128D_BINARY_OP(_mm_max_pd, N128_BINARY_OP(wah_vmaxq_f64, V128_BINARY_OP_LANE_FN(64, fmax, f64)))

WAH_RUN(I16X8_RELAXED_Q15MULR_S) {
    wah_v128_t b = sp[-1].v128, a = sp[-2].v128;
    sp[-2].v128 = wah_q15mulr_sat_s(a, b);
    sp--;
    WAH_NEXT();
}

WAH_RUN(I16X8_RELAXED_DOT_I8X16_I7X16_S) M128I_BINARY_OP(wah_i16x8_relaxed_dot_i8x16_i7x16_s_sse2, {
    wah_v128_t b = sp[-1].v128, a = sp[-2].v128;
    wah_v128_t result;
    for (int i = 0; i < 8; ++i) {
        int8_t a0 = a.i8[i * 2], a1 = a.i8[i * 2 + 1];
        int8_t b0 = (int8_t)((b.u8[i * 2]     & 0x7F) | ((b.u8[i * 2]     & 0x40) ? 0x80 : 0x00));
        int8_t b1 = (int8_t)((b.u8[i * 2 + 1] & 0x7F) | ((b.u8[i * 2 + 1] & 0x40) ? 0x80 : 0x00));
        result.i16[i] = (int16_t)a0 * b0 + (int16_t)a1 * b1;
    }
    sp[-2].v128 = result;
    sp--;
    WAH_NEXT();
})
WAH_RUN(I32X4_RELAXED_DOT_I8X16_I7X16_ADD_S) M128I_TERNARY_OP(wah_i32x4_relaxed_dot_i8x16_i7x16_add_s_sse2(a, b, c), {
    wah_v128_t c = sp[-3].v128;
    wah_v128_t b = sp[-2].v128;
    wah_v128_t a = sp[-1].v128;
    wah_v128_t result;
    for (int i = 0; i < 4; ++i) {
        int8_t a0 = a.i8[i*4], a1 = a.i8[i*4+1], a2 = a.i8[i*4+2], a3 = a.i8[i*4+3];
        int8_t b0 = (int8_t)((b.u8[i*4]   & 0x7F) | ((b.u8[i*4]   & 0x40) ? 0x80 : 0x00));
        int8_t b1 = (int8_t)((b.u8[i*4+1] & 0x7F) | ((b.u8[i*4+1] & 0x40) ? 0x80 : 0x00));
        int8_t b2 = (int8_t)((b.u8[i*4+2] & 0x7F) | ((b.u8[i*4+2] & 0x40) ? 0x80 : 0x00));
        int8_t b3 = (int8_t)((b.u8[i*4+3] & 0x7F) | ((b.u8[i*4+3] & 0x40) ? 0x80 : 0x00));
        result.i32[i] = c.i32[i] + (int32_t)a0*b0 + (int32_t)a1*b1 + (int32_t)a2*b2 + (int32_t)a3*b3;
    }
    sp[-3].v128 = result;
    sp -= 2;
    WAH_NEXT();
})

// x86-64 specific optimized opcodes
WAH_IF_X86_64(
    WAH_IF_SSSE3(
        WAH_RUN(I8X16_ABS_ssse3) M128I_UNARY_OP(wah_mm_abs_epi8)
        WAH_RUN(I16X8_ABS_ssse3) M128I_UNARY_OP(wah_mm_abs_epi16)
        WAH_RUN(I32X4_ABS_ssse3) M128I_UNARY_OP(wah_mm_abs_epi32)
        WAH_RUN(I8X16_RELAXED_SWIZZLE_ssse3) {
            // pshufb only zeros lanes when mask MSB is set. For WASM semantics,
            // we need to zero lanes where mask >= 16. Set MSB for those lanes.
            __m128i mask = sp[-1]._m128i;
            __m128i oob_mask = _mm_cmpgt_epi8(mask, _mm_set1_epi8(15));
            mask = _mm_or_si128(mask, oob_mask);
            sp[-2]._m128i = wah_mm_shuffle_epi8(sp[-2]._m128i, mask);
            sp--;
            WAH_NEXT();
        }
        WAH_RUN(I16X8_Q15MULR_SAT_S_ssse3) M128I_BINARY_OP(wah_mm_mulhrs_epi16)
        WAH_RUN(I16X8_RELAXED_Q15MULR_S_ssse3) M128I_BINARY_OP(wah_mm_mulhrs_epi16)
    )

    WAH_IF_SSE41(
        WAH_RUN(I32X4_MUL_sse41) M128I_BINARY_OP(wah_mm_mullo_epi32)
        WAH_RUN(I8X16_MIN_S_sse41) M128I_BINARY_OP(wah_mm_min_epi8)
        WAH_RUN(I8X16_MAX_S_sse41) M128I_BINARY_OP(wah_mm_max_epi8)
        WAH_RUN(I16X8_EXTEND_LOW_I8X16_S_sse41) M128I_UNARY_OP(wah_mm_cvtepi8_epi16)
        WAH_RUN(I16X8_EXTEND_HIGH_I8X16_S_sse41) M128I_UNARY_OP(wah_i16x8_extend_high_i8x16_s_sse41)
        WAH_RUN(I16X8_EXTEND_LOW_I8X16_U_sse41) M128I_UNARY_OP(wah_mm_cvtepu8_epi16)
        WAH_RUN(I16X8_EXTEND_HIGH_I8X16_U_sse41) M128I_UNARY_OP(wah_i16x8_extend_high_i8x16_u_sse41)
        WAH_RUN(I16X8_MIN_U_sse41) M128I_BINARY_OP(wah_mm_min_epu16)
        WAH_RUN(I16X8_MAX_U_sse41) M128I_BINARY_OP(wah_mm_max_epu16)
        WAH_RUN(I16X8_EXTMUL_LOW_I8X16_S_sse41) M128I_BINARY_OP(wah_i16x8_extmul_low_i8x16_s_sse41)
        WAH_RUN(I16X8_EXTMUL_HIGH_I8X16_S_sse41) M128I_BINARY_OP(wah_i16x8_extmul_high_i8x16_s_sse41)
        WAH_RUN(I16X8_EXTMUL_LOW_I8X16_U_sse41) M128I_BINARY_OP(wah_i16x8_extmul_low_i8x16_u_sse41)
        WAH_RUN(I16X8_EXTMUL_HIGH_I8X16_U_sse41) M128I_BINARY_OP(wah_i16x8_extmul_high_i8x16_u_sse41)
        WAH_RUN(I32X4_EXTEND_LOW_I16X8_S_sse41) M128I_UNARY_OP(wah_mm_cvtepi16_epi32)
        WAH_RUN(I32X4_EXTEND_HIGH_I16X8_S_sse41) M128I_UNARY_OP(wah_i32x4_extend_high_i16x8_s_sse41)
        WAH_RUN(I32X4_EXTEND_LOW_I16X8_U_sse41) M128I_UNARY_OP(wah_mm_cvtepu16_epi32)
        WAH_RUN(I32X4_EXTEND_HIGH_I16X8_U_sse41) M128I_UNARY_OP(wah_i32x4_extend_high_i16x8_u_sse41)
        WAH_RUN(I32X4_MIN_S_sse41) M128I_BINARY_OP(wah_mm_min_epi32)
        WAH_RUN(I32X4_MIN_U_sse41) M128I_BINARY_OP(wah_mm_min_epu32)
        WAH_RUN(I32X4_MAX_S_sse41) M128I_BINARY_OP(wah_mm_max_epi32)
        WAH_RUN(I32X4_MAX_U_sse41) M128I_BINARY_OP(wah_mm_max_epu32)
        WAH_RUN(I32X4_EXTMUL_LOW_I16X8_S_sse41) M128I_BINARY_OP(wah_i32x4_extmul_low_i16x8_s_sse41)
        WAH_RUN(I32X4_EXTMUL_HIGH_I16X8_S_sse41) M128I_BINARY_OP(wah_i32x4_extmul_high_i16x8_s_sse41)
        WAH_RUN(I32X4_EXTMUL_LOW_I16X8_U_sse41) M128I_BINARY_OP(wah_i32x4_extmul_low_i16x8_u_sse41)
        WAH_RUN(I32X4_EXTMUL_HIGH_I16X8_U_sse41) M128I_BINARY_OP(wah_i32x4_extmul_high_i16x8_u_sse41)
        WAH_RUN(I64X2_EXTEND_LOW_I32X4_S_sse41) M128I_UNARY_OP(wah_mm_cvtepi32_epi64)
        WAH_RUN(I64X2_EXTEND_HIGH_I32X4_S_sse41) M128I_UNARY_OP(wah_i64x2_extend_high_i32x4_s_sse41)
        WAH_RUN(I64X2_EXTEND_LOW_I32X4_U_sse41) M128I_UNARY_OP(wah_mm_cvtepu32_epi64)
        WAH_RUN(I64X2_EXTEND_HIGH_I32X4_U_sse41) M128I_UNARY_OP(wah_i64x2_extend_high_i32x4_u_sse41)
        WAH_RUN(I64X2_EXTMUL_LOW_I32X4_S_sse41) M128I_BINARY_OP(wah_i64x2_extmul_low_i32x4_s_sse41)
        WAH_RUN(I64X2_EXTMUL_HIGH_I32X4_S_sse41) M128I_BINARY_OP(wah_i64x2_extmul_high_i32x4_s_sse41)
        WAH_RUN(I64X2_EXTMUL_LOW_I32X4_U_sse41) M128I_BINARY_OP(wah_i64x2_extmul_low_i32x4_u_sse41)
        WAH_RUN(I64X2_EXTMUL_HIGH_I32X4_U_sse41) M128I_BINARY_OP(wah_i64x2_extmul_high_i32x4_u_sse41)
        WAH_RUN(F32X4_CEIL_sse41) M128_UNARY_OP(wah_mm_ceil_ps)
        WAH_RUN(F32X4_FLOOR_sse41) M128_UNARY_OP(wah_mm_floor_ps)
        WAH_RUN(F32X4_TRUNC_sse41) { sp[-1]._m128 = wah_mm_round_ps_trunc(sp[-1]._m128); WAH_NEXT(); }
        WAH_RUN(F32X4_NEAREST_sse41) { sp[-1]._m128 = wah_mm_round_ps_nearest(sp[-1]._m128); WAH_NEXT(); }
        WAH_RUN(F64X2_CEIL_sse41) M128D_UNARY_OP(wah_mm_ceil_pd)
        WAH_RUN(F64X2_FLOOR_sse41) M128D_UNARY_OP(wah_mm_floor_pd)
        WAH_RUN(F64X2_TRUNC_sse41) { sp[-1]._m128d = wah_mm_round_pd_trunc(sp[-1]._m128d); WAH_NEXT(); }
        WAH_RUN(F64X2_NEAREST_sse41) { sp[-1]._m128d = wah_mm_round_pd_nearest(sp[-1]._m128d); WAH_NEXT(); }
        WAH_RUN(I64X2_EQ_sse41) M128I_BINARY_OP(wah_mm_cmpeq_epi64)
        WAH_RUN(I64X2_NE_sse41) M128I_NOT_BINARY_OP(wah_mm_cmpeq_epi64)
        WAH_RUN(I32X4_TRUNC_SAT_F32X4_S_sse41) { sp[-1]._m128i = wah_i32x4_trunc_sat_f32x4_s_sse41(sp[-1]._m128); WAH_NEXT(); }
        WAH_RUN(I32X4_TRUNC_SAT_F32X4_U_sse41) { sp[-1]._m128i = wah_i32x4_trunc_sat_f32x4_u_sse41(sp[-1]._m128); WAH_NEXT(); }
    )

    WAH_IF_SSE42(
        WAH_RUN(I64X2_LT_S_sse42) M128I_BINARY_OP(wah_i64x2_lt_s_sse42)
        WAH_RUN(I64X2_GT_S_sse42) M128I_BINARY_OP(wah_i64x2_gt_s_sse42)
        WAH_RUN(I64X2_LE_S_sse42) M128I_BINARY_OP(wah_i64x2_le_s_sse42)
        WAH_RUN(I64X2_GE_S_sse42) M128I_BINARY_OP(wah_i64x2_ge_s_sse42)
    )

    WAH_IF_AVX2(
        WAH_RUN(I8X16_POPCNT_avx2) M128I_UNARY_OP(wah_i8x16_popcnt_avx2)
    )

    WAH_IF_AVX512F(
        WAH_RUN(I64X2_MUL_avx512f) M128I_BINARY_OP(wah_i64x2_mul_avx512f)
        WAH_RUN(I8X16_POPCNT_avx512f) M128I_UNARY_OP(wah_mm_popcnt_epi8)
        WAH_RUN(F32X4_PMIN_avx512f) M128_BINARY_OP(wah_mm_range_ps_min)
        WAH_RUN(F32X4_PMAX_avx512f) M128_BINARY_OP(wah_mm_range_ps_max)
        WAH_RUN(F64X2_PMIN_avx512f) M128D_BINARY_OP(wah_mm_range_pd_min)
        WAH_RUN(F64X2_PMAX_avx512f) M128D_BINARY_OP(wah_mm_range_pd_max)
    )

    WAH_IF_AVX512VL(
        WAH_RUN(I32X4_TRUNC_SAT_F32X4_U_avx512vl) { sp[-1]._m128i = wah_mm_cvttps_epu32(sp[-1]._m128); WAH_NEXT(); }
        WAH_RUN(F32X4_CONVERT_I32X4_U_avx512vl) { sp[-1]._m128 = wah_mm_cvtepu32_ps(sp[-1]._m128i); WAH_NEXT(); }
        WAH_RUN(F64X2_CONVERT_LOW_I32X4_U_avx512vl) { sp[-1]._m128d = wah_mm_cvtepu32_pd(sp[-1]._m128i); WAH_NEXT(); }
    )

    WAH_IF_AVX512BW(
        WAH_RUN(V128_BITSELECT_avx512f) M128I_TERNARY_OP(wah_mm_ternarylogic_epi32_bitselect(a, b, c))
        WAH_RUN(I8X16_EQ_avx512f) M128I_BINARY_OP(wah_mm_cmpeq_epi8_avx512)
        WAH_RUN(I8X16_LT_U_avx512f) M128I_BINARY_OP(wah_mm_cmplt_epu8_avx512)
        WAH_RUN(I8X16_GT_S_avx512f) M128I_BINARY_OP(wah_mm_cmpgt_epi8_avx512)
    )

    WAH_IF_AVX512IFMA(
        WAH_RUN(I64X2_MUL_avx512ifma) M128I_BINARY_OP(wah_mm_mullox_epi64)
    )
)

#undef V128_UNARY_OP
#undef V128_BINARY_OP
#undef V128_BINARY_OP_LANE
#undef V128_BINARY_OP_LANE_SAT_S
#undef V128_BINARY_OP_LANE_SAT_U
#undef V128_BINARY_OP_LANE_F
#undef V128_CMP_I_LANE
#undef V128_CMP_I_LANE_S
#undef V128_CMP_I_LANE_U
#undef V128_CMP_F_LANE

#undef M128I_UNARY_OP
#undef M128I_UNARY_I32_OP
#undef M128I_BINARY_OP
#undef M128I_NOT_BINARY_OP
#undef M128I_TERNARY_OP
#undef M128_UNARY_OP
#undef M128_BINARY_OP
#undef M128_NOT_BINARY_OP
#undef M128_TERNARY_OP
#undef M128D_UNARY_OP
#undef M128D_BINARY_OP
#undef M128D_NOT_BINARY_OP
#undef M128D_TERNARY_OP

#undef N128_UNARY_OP
#undef N128_UNARY_I32_OP
#undef N128_BINARY_OP
#undef N128_NOT_BINARY_OP
#undef N128_SHIFT_OP
#undef N128_TERNARY_OP

//------------------------------------------------------------------------------

#ifdef WAH_USE_MUSTTAIL

static wah_error_t wah_run_single(wah_exec_context_t *ctx, wah_call_frame_t *frame,
                                  const uint8_t *bytecode_ip, const uint8_t *bytecode_base,
                                  wah_value_t *sp, wah_error_t err) {
    uint16_t opcode = wah_read_u16_le(bytecode_ip);
    bytecode_ip += sizeof(uint16_t);

    switch (opcode) {
        #define WAH_OPCODE_CASES(opcode, cls, val) \
            case WAH_OP_##opcode: __attribute__((musttail)) return wah_run_##opcode(ctx, frame, bytecode_ip, bytecode_base, sp, err);
        #define WAH_EXTRA_OPCODE_CASES(opcode, suffix) WAH_OPCODE_CASES(opcode##_##suffix,,)
        WAH_OPCODES(WAH_OPCODE_CASES)
        WAH_EXTRA_OPCODES(WAH_EXTRA_OPCODE_CASES)
        #undef WAH_OPCODE_CASES
        #undef WAH_EXTRA_OPCODE_CASES
    default:
        WAH_UNREACHABLE();
    }
}

static wah_error_t wah_run_interpreter(wah_exec_context_t *ctx) {
    // These are pointers to the current frame's state for faster access.
    wah_call_frame_t *frame = &ctx->call_stack[ctx->call_depth - 1];
    const uint8_t *bytecode_ip = frame->bytecode_ip;
    const uint8_t *bytecode_base = frame->code->parsed_code.bytecode;
    wah_value_t *sp = ctx->value_stack + ctx->sp;  // Stack pointer for faster access

    return wah_run_single(ctx, frame, bytecode_ip, bytecode_base, sp, WAH_OK);
}

#else

#ifndef WAH_USE_COMPUTED_GOTO
        default:
            WAH_UNREACHABLE();
        }
    }
#endif

cleanup:
    ctx->sp = (uint32_t)(sp - ctx->value_stack);
    if (ctx->call_depth > 0) {
        frame->bytecode_ip = bytecode_ip;
    }
    return err;
} // End of wah_run_interpreter

#endif

#if defined(__GNUC__) && !defined(__clang__)
    #pragma GCC pop_options
#elif defined(_MSC_VER)
    #pragma float_control(pop)
#endif

static wah_error_t wah_call_module_multi(wah_exec_context_t *exec_ctx, uint32_t func_idx, const wah_value_t *params, uint32_t param_count, wah_value_t *results, uint32_t max_results, uint32_t *actual_results) {
    // Dispatch via the runtime function_table (global index space)
    WAH_ENSURE(func_idx < exec_ctx->function_table_count, WAH_ERROR_NOT_FOUND);
    const wah_function_t *fn = &exec_ctx->function_table[func_idx];

    if (fn->is_host) {
        // Host function path
        size_t nresults = fn->nresults;

        // Check result count
        WAH_ENSURE(max_results >= nresults, WAH_ERROR_VALIDATION_FAILED);

        // Call host function using helper
        WAH_CHECK(wah_call_host_function_internal(exec_ctx, fn, params, param_count, results));

        *actual_results = (uint32_t)nresults;
        return WAH_OK;
    }

    // WASM function call
    uint32_t local_idx = (uint32_t)fn->local_idx;
    const wah_module_t *fn_module = fn->fn_module ? fn->fn_module : exec_ctx->module;
    const wah_func_type_t *func_type = &fn_module->types[fn_module->function_type_indices[local_idx]];
    WAH_ENSURE(param_count == func_type->param_count, WAH_ERROR_VALIDATION_FAILED);

    // Push initial params onto the value stack
    for (uint32_t i = 0; i < param_count; ++i) {
        WAH_ENSURE(exec_ctx->sp < exec_ctx->value_stack_capacity, WAH_ERROR_CALL_STACK_OVERFLOW); // Value stack overflow
        exec_ctx->value_stack[exec_ctx->sp++] = params[i];
    }

    // Push the first frame. Locals offset is the current stack pointer before parameters.
    WAH_CHECK(wah_push_frame(exec_ctx, fn_module, local_idx, exec_ctx->sp - func_type->param_count, func_type->result_count));

    // Reserve space for the function's own locals and initialize them to zero
    uint32_t num_locals = exec_ctx->call_stack[0].code->local_count;
    if (num_locals > 0) {
        WAH_ENSURE(exec_ctx->sp + num_locals <= exec_ctx->value_stack_capacity, WAH_ERROR_OUT_OF_MEMORY);
        memset(&exec_ctx->value_stack[exec_ctx->sp], 0, sizeof(wah_value_t) * num_locals);
        exec_ctx->sp += num_locals;
    }

    // Run the main interpreter loop
    WAH_CHECK(wah_run_interpreter(exec_ctx));

    // After execution, copy multiple results from the stack
    uint32_t copy_count = func_type->result_count;
    if (copy_count > max_results) copy_count = max_results;

    if (results) {
        if (func_type->result_count == 0) {
            // Zero return function - zeroize results for safety
            memset(results, 0, sizeof(wah_value_t) * max_results);
            *actual_results = 0;
        } else if (exec_ctx->sp >= func_type->result_count) {
            // Copy results (as many as fit in the provided buffer)
            for (uint32_t i = 0; i < copy_count; ++i) {
                results[i] = exec_ctx->value_stack[exec_ctx->sp - func_type->result_count + i];
            }
            *actual_results = func_type->result_count;
        } else {
            *actual_results = 0;
        }
    } else {
        *actual_results = 0;
    }

    return WAH_OK;
}

static wah_error_t wah_call_module(wah_exec_context_t *exec_ctx, uint32_t func_idx, const wah_value_t *params, uint32_t param_count, wah_value_t *result) {
    if (!result) {
        // Case where even a single return is not needed (void function)
        uint32_t dummy;
        return wah_call_module_multi(exec_ctx, func_idx, params, param_count, NULL, 0, &dummy);
    }

    uint32_t actual_results;
    wah_error_t err = wah_call_module_multi(exec_ctx, func_idx, params, param_count, result, 1, &actual_results);

    // Return WAH_OK_BUT_MULTI_RETURN for result_count > 1 (compatibility)
    if (err == WAH_OK && actual_results > 1) {
        err = WAH_OK_BUT_MULTI_RETURN;
    }

    return err;
}

wah_error_t wah_call(wah_exec_context_t *exec_ctx, uint64_t func_idx, const wah_value_t *params, uint32_t param_count, wah_value_t *result) {
    WAH_ENSURE(exec_ctx, WAH_ERROR_MISUSE);
    WAH_ENSURE(exec_ctx->module, WAH_ERROR_MISUSE);

    if (!exec_ctx->is_instantiated) {
        WAH_CHECK(wah_instantiate(exec_ctx));
    }

    // func_idx is always uint32_t for functions (wah_entry_id_t values for functions always fit in uint32_t)
    return wah_call_module(exec_ctx, (uint32_t)func_idx, params, param_count, result);
}

wah_error_t wah_call_multi(wah_exec_context_t *exec_ctx, uint64_t func_idx, const wah_value_t *params, uint32_t param_count, wah_value_t *results, uint32_t max_results, uint32_t *actual_results) {
    WAH_ENSURE(exec_ctx, WAH_ERROR_MISUSE);
    WAH_ENSURE(exec_ctx->module, WAH_ERROR_MISUSE);
    WAH_ENSURE(actual_results, WAH_ERROR_MISUSE);

    if (!exec_ctx->is_instantiated) {
        WAH_CHECK(wah_instantiate(exec_ctx));
    }

    // func_idx is always uint32_t for functions (wah_entry_id_t values for functions always fit in uint32_t)
    return wah_call_module_multi(exec_ctx, (uint32_t)func_idx, params, param_count, results, max_results, actual_results);
}

// --- Module Cleanup Implementation ---
void wah_free_module(wah_module_t *module) {
    if (!module) {
        return;
    }

    if (module->types) {
        for (uint32_t i = 0; i < module->type_count; ++i) {
            free(module->types[i].param_types);
            free(module->types[i].result_types);
        }
        free(module->types);
    }
    free(module->function_type_indices);

    if (module->func_imports) {
        for (uint32_t i = 0; i < module->import_function_count; i++) {
            free(module->func_imports[i].module_name);
            free(module->func_imports[i].field_name);
        }
        free(module->func_imports);
    }

    if (module->code_bodies) {
        for (uint32_t i = 0; i < module->code_count; ++i) {
            free(module->code_bodies[i].local_types);
            wah_free_parsed_code(&module->code_bodies[i].parsed_code);
        }
        free(module->code_bodies);
    }

    if (module->globals) {
        for (uint32_t i = 0; i < module->global_count; ++i) {
            wah_free_parsed_code(&module->globals[i].init_expr);
        }
        free(module->globals);
    }
    free(module->memories);
    free(module->tables);

    if (module->element_segments) {
        for (uint32_t i = 0; i < module->element_segment_count; ++i) {
            wah_free_element_segment_data(&module->element_segments[i]);
        }
        free(module->element_segments);
    }

    if (module->data_segments) {
        for (uint32_t i = 0; i < module->data_segment_count; ++i) {
            free((void *)module->data_segments[i].data);
        }
        free(module->data_segments);
    }

    // Free exports. For host function exports the name is owned by functions[], freed below.
    if (module->exports) {
        for (uint32_t i = 0; i < module->export_count; ++i) {
            uint32_t idx = module->exports[i].index;
            bool is_host_export = (module->exports[i].kind == 0 &&
                                   module->functions &&
                                   idx < module->total_function_count &&
                                   module->functions[idx].is_host);
            // For memory (kind 2) and global (kind 3) exports from programmatically created modules,
            // the name is owned by the export entry itself, not by another structure.
            // For WASM function exports (kind 0, not host), the name is also owned by the export entry.
            if (!is_host_export) {
                free((void*)module->exports[i].name);
            }
        }
        free(module->exports);
    }

    // Free host function resources stored in the unified functions[] array.
    if (module->functions) {
        for (uint32_t i = module->function_count; i < module->total_function_count; ++i) {
            wah_function_t *fn = &module->functions[i];
            if (fn->is_host) {
                if (fn->finalize && fn->userdata) {
                    fn->finalize(fn->userdata);
                }
                free(fn->name);
                free(fn->param_types);
                free(fn->result_types);
            }
        }
        free(module->functions);
    }

    // Reset all fields to 0/NULL
    memset(module, 0, sizeof(wah_module_t));
}

// --- Programmatically created module API ---

wah_error_t wah_new_module(wah_module_t *mod) {
    WAH_ENSURE(mod, WAH_ERROR_MISUSE);

    *mod = (wah_module_t){
        .function_capacity = 16,
        .total_function_count = 0,
        .capacity_exports = 16,
    };

    // Allocate initial unified functions[] array (all host functions for a new module)
    WAH_MALLOC_ARRAY(mod->functions, mod->function_capacity);

    // Allocate initial export array
    WAH_MALLOC_ARRAY(mod->exports, mod->capacity_exports);

    return WAH_OK;
}

wah_error_t wah_module_export_funcv(wah_module_t *mod, const char *name, size_t nparams, const wah_type_t *param_types, size_t nresults, const wah_type_t *result_types, wah_func_t func, void *userdata, wah_finalize_t finalize) {
    wah_error_t err;
    char *name_copy = NULL;
    wah_type_t *param_types_copy = NULL;
    wah_type_t *result_types_copy = NULL;

    WAH_ENSURE(mod, WAH_ERROR_MISUSE);
    WAH_ENSURE(name, WAH_ERROR_MISUSE);
    WAH_ENSURE(func, WAH_ERROR_MISUSE);
    WAH_ENSURE(nparams == 0 || param_types, WAH_ERROR_MISUSE);
    WAH_ENSURE(nresults == 0 || result_types, WAH_ERROR_MISUSE);

    // Check for duplicate export name
    for (uint32_t i = 0; i < mod->export_count; ++i) {
        if (mod->exports[i].name && strcmp(mod->exports[i].name, name) == 0) {
            return WAH_ERROR_VALIDATION_FAILED;  // Duplicate export name
        }
    }

    // Grow export array if needed
    if (mod->export_count >= mod->capacity_exports) {
        uint32_t new_capacity = mod->capacity_exports * 2;
        WAH_REALLOC_ARRAY_GOTO(mod->exports, new_capacity, cleanup);
        mod->capacity_exports = new_capacity;
    }

    // Grow functions[] array if needed
    if (mod->total_function_count >= mod->function_capacity) {
        uint32_t new_capacity = mod->function_capacity * 2;
        WAH_REALLOC_ARRAY_GOTO(mod->functions, new_capacity, cleanup);
        mod->function_capacity = new_capacity;
    }

    // Duplicate name and type arrays
    name_copy = wah_strdup(name);
    WAH_ENSURE_GOTO(name_copy, WAH_ERROR_OUT_OF_MEMORY, cleanup);

    if (nparams > 0) {
        WAH_MALLOC_ARRAY_GOTO(param_types_copy, nparams, cleanup);
        memcpy(param_types_copy, param_types, nparams * sizeof(wah_type_t));
    }

    if (nresults > 0) {
        WAH_MALLOC_ARRAY_GOTO(result_types_copy, nresults, cleanup);
        memcpy(result_types_copy, result_types, nresults * sizeof(wah_type_t));
    }

    // Fill in the unified function entry
    uint32_t new_func_idx = mod->total_function_count;
    wah_function_t *fn = &mod->functions[new_func_idx];
    fn->is_host = true;
    fn->name = name_copy;
    fn->func = func;
    fn->userdata = userdata;
    fn->finalize = finalize;
    fn->nparams = nparams;
    fn->param_types = param_types_copy;
    fn->nresults = nresults;
    fn->result_types = result_types_copy;
    mod->total_function_count++;

    // Fill in export entry
    struct wah_export_s *export_entry = &mod->exports[mod->export_count];
    export_entry->name = name_copy;
    export_entry->name_len = strlen(name_copy);
    export_entry->kind = 0;  // FUNCTION
    export_entry->index = new_func_idx;

    mod->export_count++;
    return WAH_OK;

cleanup:
    free(param_types_copy);
    free(result_types_copy);
    free(name_copy);
    return err;
}

static int wah_is_ignorable_in_types(const char c) {
    return c == ' ' || c == '\t' || c == ',' || c == '\n' || c == '\r';
}

// Parse a sequence of types separated by whitespace and commas
// types_end points to the end of the sequence (exclusive)
static wah_error_t wah_parse_type_seq(const char *types, const char *types_end, size_t *out_ntypes, wah_type_t **out_types) {
    *out_ntypes = 0;
    *out_types = NULL;

    // Count types
    size_t ntypes = 0;
    const char *p = types;
    while (p < types_end) {
        // Skip whitespace and commas
        while (p < types_end && wah_is_ignorable_in_types(*p)) ++p;
        if (p < types_end) {
            ntypes++;
            // Validate and skip type identifier (must be exactly i32, i64, f32, f64, or v128)
            if (p + 2 < types_end && (*p == 'i' || *p == 'f')) {
                // Check for i32/f32 (3 characters: letter + '3' + '2')
                if (p[1] == '3' && p[2] == '2') {
                    // Make sure the next character (if any) is a separator
                    if (p + 3 >= types_end || wah_is_ignorable_in_types(p[3])) {
                        p += 3;
                        continue;
                    }
                }
                // Check for i64/f64 (3 characters: letter + '6' + '4')
                if (p[1] == '6' && p[2] == '4') {
                    // Make sure the next character (if any) is a separator
                    if (p + 3 >= types_end || wah_is_ignorable_in_types(p[3])) {
                        p += 3;
                        continue;
                    }
                }
            } else if (p + 3 < types_end && *p == 'v') {
                // Check for v128 (4 characters: 'v' + '1' + '2' + '8')
                if (p[1] == '1' && p[2] == '2' && p[3] == '8') {
                    // Make sure the next character (if any) is a separator
                    if (p + 4 >= types_end || wah_is_ignorable_in_types(p[4])) {
                        p += 4;
                        continue;
                    }
                }
            }
            return WAH_ERROR_BAD_SPEC;  // Invalid type
        }
    }

    // Allocate types array
    wah_type_t *type_array = NULL;
    if (ntypes > 0) {
        wah_error_t err;
        WAH_MALLOC_ARRAY_GOTO(type_array, ntypes, cleanup);

        // Parse types
        p = types;
        size_t idx = 0;
        while (p < types_end) {
            // Skip whitespace and commas
            while (p < types_end && wah_is_ignorable_in_types(*p)) ++p;
            if (p < types_end) {
                if (p + 2 < types_end && (*p == 'i' || *p == 'f')) {
                    char type_char = *p;
                    // Check for i32/f32
                    if (p[1] == '3' && p[2] == '2') {
                        p += 3;
                        type_array[idx++] = (type_char == 'i') ? WAH_TYPE_I32 : WAH_TYPE_F32;
                        continue;
                    }
                    // Check for i64/f64
                    if (p[1] == '6' && p[2] == '4') {
                        p += 3;
                        type_array[idx++] = (type_char == 'i') ? WAH_TYPE_I64 : WAH_TYPE_F64;
                        continue;
                    }
                } else if (p + 3 < types_end && *p == 'v') {
                    // Check for v128
                    if (p[1] == '1' && p[2] == '2' && p[3] == '8') {
                        p += 4;
                        type_array[idx++] = WAH_TYPE_V128;
                        continue;
                    }
                }
                err = WAH_ERROR_BAD_SPEC;  // Invalid type
                goto cleanup;
            }
        }
    }

    *out_ntypes = ntypes;
    *out_types = type_array;
    return WAH_OK;

cleanup:
    free(type_array);
    return WAH_ERROR_BAD_SPEC;
}

wah_error_t wah_parse_func_type(const char *types, size_t *out_nparams, wah_type_t **out_param_types, size_t *out_nresults, wah_type_t **out_result_types) {
    // Initialize all outputs first to ensure they're set even on early return
    *out_nparams = 0;
    *out_nresults = 0;
    *out_param_types = NULL;
    *out_result_types = NULL;

    // Validate input pointers
    WAH_ENSURE(types, WAH_ERROR_MISUSE);
    WAH_ENSURE(out_nparams, WAH_ERROR_MISUSE);
    WAH_ENSURE(out_param_types, WAH_ERROR_MISUSE);
    WAH_ENSURE(out_nresults, WAH_ERROR_MISUSE);
    WAH_ENSURE(out_result_types, WAH_ERROR_MISUSE);

    // Find the separator '->'
    const char *arrow = strstr(types, "->");
    WAH_ENSURE(arrow, WAH_ERROR_BAD_SPEC);

    // Parse parameter types (before '->')
    WAH_CHECK(wah_parse_type_seq(types, arrow, out_nparams, out_param_types));

    // Parse result types (after '->')
    wah_error_t err;
    WAH_CHECK_GOTO(wah_parse_type_seq(arrow + 2, types + strlen(types), out_nresults, out_result_types), cleanup);

    return WAH_OK;

cleanup:
    free(*out_param_types);
    *out_param_types = NULL;  // Prevent double-free
    return err;
}

wah_error_t wah_module_export_func(wah_module_t *mod, const char *name, const char *types, wah_func_t func, void *userdata, wah_finalize_t finalize) {
    WAH_ENSURE(mod, WAH_ERROR_MISUSE);
    WAH_ENSURE(name, WAH_ERROR_MISUSE);
    WAH_ENSURE(types, WAH_ERROR_MISUSE);
    WAH_ENSURE(func, WAH_ERROR_MISUSE);

    // Parse type string
    size_t nparams, nresults;
    wah_type_t *param_types = NULL, *result_types = NULL;
    wah_error_t err;

    WAH_CHECK_GOTO(wah_parse_func_type(types, &nparams, &param_types, &nresults, &result_types), cleanup);

    // Delegate to wah_module_export_funcv
    err = wah_module_export_funcv(mod, name, nparams, param_types, nresults, result_types, func, userdata, finalize);

cleanup:
    // Free the parsed type arrays (wah_module_export_funcv makes copies)
    free(param_types);
    free(result_types);

    return err;
}

// Helper to free element segment data
static void wah_free_element_segment_data(wah_element_segment_t *segment) {
    if (!segment->is_expr_elem) {
        free(segment->u.func_indices);
        segment->u.func_indices = NULL;
    } else {
        if (segment->u.expr.bytecodes) {
            for (uint32_t i = 0; i < segment->num_elems; ++i) {
                free((void*)segment->u.expr.bytecodes[i]);
            }
            free(segment->u.expr.bytecodes);
            segment->u.expr.bytecodes = NULL;
        }
        if (segment->u.expr.bytecode_sizes) {
            free(segment->u.expr.bytecode_sizes);
            segment->u.expr.bytecode_sizes = NULL;
        }
    }
    wah_free_parsed_code(&segment->offset_expr);
}

// Helper to create a preparsed const expression from a simple constant value
static wah_error_t wah_create_const_expr(wah_type_t type, const wah_value_t *value, wah_parsed_code_t *parsed_code) {
    uint32_t size = sizeof(uint16_t);  // opcode
    uint16_t opcode;

    switch (type) {
        case WAH_TYPE_I32:
            opcode = WAH_OP_I32_CONST;
            size += sizeof(int32_t);
            break;
        case WAH_TYPE_I64:
            opcode = WAH_OP_I64_CONST;
            size += sizeof(int64_t);
            break;
        case WAH_TYPE_F32:
            opcode = WAH_OP_F32_CONST;
            size += sizeof(float);
            break;
        case WAH_TYPE_F64:
            opcode = WAH_OP_F64_CONST;
            size += sizeof(double);
            break;
        case WAH_TYPE_V128:
            opcode = WAH_OP_V128_CONST;
            size += sizeof(wah_v128_t);
            break;
        default:
            return WAH_ERROR_VALIDATION_FAILED;
    }

    WAH_MALLOC_ARRAY(parsed_code->bytecode, size);
    parsed_code->bytecode_size = size;

    uint8_t *out = parsed_code->bytecode;
    wah_write_u16_le(out, opcode);
    out += sizeof(uint16_t);

    switch (type) {
        case WAH_TYPE_I32:
            memcpy(out, &value->i32, sizeof(int32_t));
            break;
        case WAH_TYPE_I64:
            memcpy(out, &value->i64, sizeof(int64_t));
            break;
        case WAH_TYPE_F32:
            memcpy(out, &value->f32, sizeof(float));
            break;
        case WAH_TYPE_F64:
            memcpy(out, &value->f64, sizeof(double));
            break;
        case WAH_TYPE_V128:
            memcpy(out, &value->v128, sizeof(wah_v128_t));
            break;
    }

    return WAH_OK;
}

// Internal helper for exporting globals
static wah_error_t wah_module_export_global_internal(wah_module_t *mod, const char *name, wah_type_t type, bool is_mutable, const wah_value_t *init_value) {
    wah_error_t err;
    char *name_copy = NULL;
    wah_global_t *new_globals = NULL;

    WAH_ENSURE(mod, WAH_ERROR_MISUSE);
    WAH_ENSURE(name, WAH_ERROR_MISUSE);

    // Check for duplicate export name
    for (uint32_t i = 0; i < mod->export_count; ++i) {
        if (mod->exports[i].name && strcmp(mod->exports[i].name, name) == 0) {
            return WAH_ERROR_VALIDATION_FAILED;  // Duplicate export name
        }
    }

    // Grow export array if needed
    if (mod->export_count >= mod->capacity_exports) {
        uint32_t new_capacity = mod->capacity_exports * 2;
        WAH_REALLOC_ARRAY_GOTO(mod->exports, new_capacity, cleanup);
        mod->capacity_exports = new_capacity;
    }

    // Reallocate globals array
    uint32_t new_global_count = mod->global_count + 1;
    WAH_REALLOC_ARRAY_GOTO(mod->globals, new_global_count, cleanup);

    // Duplicate name
    name_copy = wah_strdup(name);
    WAH_ENSURE_GOTO(name_copy, WAH_ERROR_OUT_OF_MEMORY, cleanup);

    // Fill in the global entry
    wah_global_t *global = &mod->globals[mod->global_count];
    global->type = type;
    global->is_mutable = is_mutable;
    global->init_expr = (wah_parsed_code_t){0};  // Initialize to zero
    // Create a const expression for the initial value
    WAH_CHECK_GOTO(wah_create_const_expr(type, init_value, &global->init_expr), cleanup);

    // Fill in export entry
    wah_export_t *export_entry = &mod->exports[mod->export_count];
    export_entry->name = name_copy;
    export_entry->name_len = strlen(name_copy);
    export_entry->kind = 3;  // GLOBAL
    export_entry->index = mod->global_count;

    mod->global_count++;
    mod->export_count++;
    return WAH_OK;

cleanup:
    free(name_copy);
    if (new_globals) free(new_globals);
    return err;
}

wah_error_t wah_module_export_memory(wah_module_t *mod, const char *name, uint64_t min_pages, uint64_t max_pages) {
    wah_error_t err;
    char *name_copy = NULL;
    wah_memory_type_t *new_memories = NULL;

    WAH_ENSURE(mod, WAH_ERROR_MISUSE);
    WAH_ENSURE(name, WAH_ERROR_MISUSE);
    WAH_ENSURE(min_pages > 0, WAH_ERROR_MISUSE);
    WAH_ENSURE(max_pages >= min_pages, WAH_ERROR_MISUSE);

    // Check for duplicate export name
    for (uint32_t i = 0; i < mod->export_count; ++i) {
        if (mod->exports[i].name && strcmp(mod->exports[i].name, name) == 0) {
            return WAH_ERROR_VALIDATION_FAILED;  // Duplicate export name
        }
    }

    // Grow export array if needed
    if (mod->export_count >= mod->capacity_exports) {
        uint32_t new_capacity = mod->capacity_exports * 2;
        WAH_REALLOC_ARRAY_GOTO(mod->exports, new_capacity, cleanup);
        mod->capacity_exports = new_capacity;
    }

    // Reallocate memories array
    uint32_t new_memory_count = mod->memory_count + 1;
    WAH_REALLOC_ARRAY_GOTO(mod->memories, new_memory_count, cleanup);

    // Duplicate name
    name_copy = wah_strdup(name);
    WAH_ENSURE_GOTO(name_copy, WAH_ERROR_OUT_OF_MEMORY, cleanup);

    // Fill in the memory entry
    wah_memory_type_t *mem = &mod->memories[mod->memory_count];
    mem->min_pages = min_pages;
    mem->max_pages = max_pages;

    // Fill in export entry
    wah_export_t *export_entry = &mod->exports[mod->export_count];
    export_entry->name = name_copy;
    export_entry->name_len = strlen(name_copy);
    export_entry->kind = 2;  // MEMORY
    export_entry->index = mod->memory_count;

    mod->memory_count++;
    mod->export_count++;
    return WAH_OK;

cleanup:
    free(name_copy);
    if (new_memories) free(new_memories);
    return err;
}

wah_error_t wah_module_export_global_i32(wah_module_t *mod, const char *name, bool is_mutable, int32_t init_value) {
    wah_value_t val = {.i32 = init_value};
    return wah_module_export_global_internal(mod, name, WAH_TYPE_I32, is_mutable, &val);
}

wah_error_t wah_module_export_global_i64(wah_module_t *mod, const char *name, bool is_mutable, int64_t init_value) {
    wah_value_t val = {.i64 = init_value};
    return wah_module_export_global_internal(mod, name, WAH_TYPE_I64, is_mutable, &val);
}

wah_error_t wah_module_export_global_f32(wah_module_t *mod, const char *name, bool is_mutable, float init_value) {
    wah_value_t val = {.f32 = init_value};
    return wah_module_export_global_internal(mod, name, WAH_TYPE_F32, is_mutable, &val);
}

wah_error_t wah_module_export_global_f64(wah_module_t *mod, const char *name, bool is_mutable, double init_value) {
    wah_value_t val = {.f64 = init_value};
    return wah_module_export_global_internal(mod, name, WAH_TYPE_F64, is_mutable, &val);
}

wah_error_t wah_module_export_global_v128(wah_module_t *mod, const char *name, bool is_mutable, const wah_v128_t *init_value) {
    wah_value_t val = {.v128 = *init_value};
    return wah_module_export_global_internal(mod, name, WAH_TYPE_V128, is_mutable, &val);
}

// --- Call Context Implementation ---

void wah_trap(wah_call_context_t *ctx, wah_error_t reason) {
    WAH_ASSERT(ctx && "Call context is NULL");
    WAH_ASSERT(reason != WAH_OK && "Cannot trap with WAH_OK");
    WAH_ASSERT(ctx->trap_reason == WAH_OK && "Call context already has a trap reason set");
    ctx->trap_reason = reason;
}

// Internal helper: Call a host function with given context
static wah_error_t wah_call_host_function_internal(
    wah_exec_context_t *exec_ctx,
    const wah_function_t *fn,
    const wah_value_t *params,
    uint32_t param_count,
    wah_value_t *results
) {
    wah_call_context_t call_ctx = {0};
    call_ctx.exec = exec_ctx;
    call_ctx.nparams = param_count;
    call_ctx.params = params;
    call_ctx.nresults = fn->nresults;
    call_ctx.results = results;
    call_ctx.param_types = fn->param_types;
    call_ctx.result_types = fn->result_types;
    call_ctx.trap_reason = WAH_OK;

    fn->func(&call_ctx, fn->userdata);

    WAH_ENSURE(call_ctx.trap_reason == WAH_OK, call_ctx.trap_reason);

    return WAH_OK;
}

// --- Linkage Implementation ---

wah_error_t wah_link_module(wah_exec_context_t *ctx, const char *name, const wah_module_t *mod) {
    WAH_ENSURE(ctx, WAH_ERROR_MISUSE);
    WAH_ENSURE(name, WAH_ERROR_MISUSE);
    WAH_ENSURE(mod, WAH_ERROR_MISUSE);
    WAH_ENSURE(!ctx->is_instantiated, WAH_ERROR_MISUSE);  // Cannot link after instantiation

    // Check for duplicate module name
    for (uint32_t i = 0; i < ctx->linked_module_count; ++i) {
        if (strcmp(ctx->linked_modules[i].name, name) == 0) {
            return WAH_ERROR_VALIDATION_FAILED;  // Duplicate module name
        }
    }

    // Grow linked modules array if needed
    if (ctx->linked_module_count >= ctx->linked_module_capacity) {
        uint32_t new_capacity = ctx->linked_module_capacity == 0 ? 4 : ctx->linked_module_capacity * 2;
        WAH_REALLOC_ARRAY(ctx->linked_modules, new_capacity);
        ctx->linked_module_capacity = new_capacity;
    }

    // Duplicate module name
    char *name_copy = wah_strdup(name);
    WAH_ENSURE(name_copy, WAH_ERROR_OUT_OF_MEMORY);

    // Add to linked modules
    ctx->linked_modules[ctx->linked_module_count].name = name_copy;
    ctx->linked_modules[ctx->linked_module_count].module = mod;
    ctx->linked_module_count++;

    return WAH_OK;
}

// --- Const Expression Evaluator ---
// Evaluates a preparsed const expression and returns the result
// Used during instantiation to initialize globals
static wah_error_t wah_eval_const_expr(
    wah_exec_context_t *ctx,
    const uint8_t *bytecode,
    uint32_t bytecode_size,
    wah_value_t *result
) {
    // Simple stack-based evaluator for const expressions
    // Uses ctx->value_stack temporarily (saves/restores sp)
    uint32_t saved_sp = ctx->sp;
    uint32_t stack_start = ctx->sp;

    const uint8_t *ip = bytecode;
    const uint8_t *end = bytecode + bytecode_size;

    while (ip < end) {
        uint16_t opcode_val = wah_read_u16_le(ip);
        ip += sizeof(uint16_t);

        switch (opcode_val) {
            case WAH_OP_END:
                // End of const expression
                goto eval_done;

            // Constants
            case WAH_OP_I32_CONST:
                ctx->value_stack[ctx->sp++].i32 = wah_read_u32_le(ip);
                ip += sizeof(int32_t);
                break;
            case WAH_OP_I64_CONST:
                ctx->value_stack[ctx->sp++].i64 = wah_read_u64_le(ip);
                ip += sizeof(int64_t);
                break;
            case WAH_OP_F32_CONST:
                ctx->value_stack[ctx->sp++].f32 = wah_read_f32_le(ip);
                ip += sizeof(float);
                break;
            case WAH_OP_F64_CONST:
                ctx->value_stack[ctx->sp++].f64 = wah_read_f64_le(ip);
                ip += sizeof(double);
                break;

            // Integer binary operations
            case WAH_OP_I32_ADD: {
                int32_t right = ctx->value_stack[--ctx->sp].i32;
                int32_t left = ctx->value_stack[--ctx->sp].i32;
                ctx->value_stack[ctx->sp++].i32 = left + right;
                break;
            }
            case WAH_OP_I32_SUB: {
                int32_t right = ctx->value_stack[--ctx->sp].i32;
                int32_t left = ctx->value_stack[--ctx->sp].i32;
                ctx->value_stack[ctx->sp++].i32 = left - right;
                break;
            }
            case WAH_OP_I32_MUL: {
                int32_t right = ctx->value_stack[--ctx->sp].i32;
                int32_t left = ctx->value_stack[--ctx->sp].i32;
                ctx->value_stack[ctx->sp++].i32 = left * right;
                break;
            }
            case WAH_OP_I64_ADD: {
                int64_t right = ctx->value_stack[--ctx->sp].i64;
                int64_t left = ctx->value_stack[--ctx->sp].i64;
                ctx->value_stack[ctx->sp++].i64 = left + right;
                break;
            }
            case WAH_OP_I64_SUB: {
                int64_t right = ctx->value_stack[--ctx->sp].i64;
                int64_t left = ctx->value_stack[--ctx->sp].i64;
                ctx->value_stack[ctx->sp++].i64 = left - right;
                break;
            }
            case WAH_OP_I64_MUL: {
                int64_t right = ctx->value_stack[--ctx->sp].i64;
                int64_t left = ctx->value_stack[--ctx->sp].i64;
                ctx->value_stack[ctx->sp++].i64 = left * right;
                break;
            }

            // Reference types
            case WAH_OP_REF_NULL:
                ctx->value_stack[ctx->sp++].ref = NULL;
                break;
            case WAH_OP_REF_FUNC: {
                uint32_t func_idx = wah_read_u32_le(ip);
                ip += sizeof(uint32_t);
                // Use prefuncref so func_idx==0 is never confused with ref.null (NULL pointer).
                wah_value_t val;
                val.prefuncref.sentinel = wah_funcref_sentinel;
                val.prefuncref.func_idx = func_idx;
                ctx->value_stack[ctx->sp++] = val;
                break;
            }

            // Global get
            case WAH_OP_GLOBAL_GET: {
                uint32_t global_idx = wah_read_u32_le(ip);
                ip += sizeof(uint32_t);
                WAH_ENSURE(global_idx < ctx->global_count, WAH_ERROR_VALIDATION_FAILED);
                ctx->value_stack[ctx->sp++] = ctx->globals[global_idx];
                break;
            }

            default:
                return WAH_ERROR_VALIDATION_FAILED;
        }
    }

eval_done:
    // Result should be the only thing on the stack
    WAH_ENSURE(ctx->sp == stack_start + 1, WAH_ERROR_VALIDATION_FAILED);
    *result = ctx->value_stack[stack_start];

    // Restore stack pointer
    ctx->sp = saved_sp;
    return WAH_OK;
}

wah_error_t wah_instantiate(wah_exec_context_t *ctx) {
    wah_error_t err = WAH_OK;
    WAH_ENSURE(ctx, WAH_ERROR_MISUSE);
    WAH_ENSURE(!ctx->is_instantiated, WAH_ERROR_MISUSE);  // Already instantiated

    const wah_module_t *module = ctx->module;
    uint32_t import_count = module->import_function_count;

    // Initialize globals by evaluating const expressions
    // Must be done in order, as later globals can reference earlier ones
    for (uint32_t i = 0; i < module->global_count; ++i) {
        WAH_CHECK_GOTO(wah_eval_const_expr(ctx,
                                           module->globals[i].init_expr.bytecode,
                                           module->globals[i].init_expr.bytecode_size,
                                           &ctx->globals[i]), cleanup);
        // Convert funcref prefuncref format (from const_expr) to runtime pointer format
        if (module->globals[i].type == WAH_TYPE_FUNCREF && ctx->globals[i].ref != NULL) {
            uint32_t fidx = ctx->globals[i].prefuncref.func_idx;
            WAH_ENSURE_GOTO(fidx < ctx->function_table_count, WAH_ERROR_VALIDATION_FAILED, cleanup);
            ctx->globals[i].ref = &ctx->function_table[fidx];
        }
    }

    // Allocate and initialize globals for linked modules
    // For simplicity, we allocate globals after the primary module's globals
    uint32_t total_globals = module->global_count;
    for (uint32_t j = 0; j < ctx->linked_module_count; j++) {
        total_globals += ctx->linked_modules[j].module->global_count;
    }

    if (total_globals > module->global_count) {
        // Reallocate globals array to accommodate linked modules
        wah_value_t *new_globals = NULL;
        WAH_MALLOC_ARRAY_GOTO(new_globals, total_globals, cleanup);

        // Copy primary module's globals
        memcpy(new_globals, ctx->globals, module->global_count * sizeof(wah_value_t));

        // Initialize globals for each linked module by evaluating const expressions.
        // Temporarily redirect ctx->globals to each linked module's own slot in new_globals
        // so that global.get inside an initializer resolves against the linked module's
        // own globals rather than the primary module's.
        wah_value_t *saved_globals = ctx->globals;
        uint32_t saved_global_count = ctx->global_count;
        uint32_t offset = module->global_count;
        for (uint32_t j = 0; j < ctx->linked_module_count; j++) {
            const wah_module_t *linked = ctx->linked_modules[j].module;
            ctx->globals = new_globals + offset;
            ctx->global_count = linked->global_count;
            for (uint32_t k = 0; k < linked->global_count; k++) {
                err = wah_eval_const_expr(ctx,
                                         linked->globals[k].init_expr.bytecode,
                                         linked->globals[k].init_expr.bytecode_size,
                                         &new_globals[offset + k]);
                if (err != WAH_OK) {
                    ctx->globals = saved_globals;
                    ctx->global_count = saved_global_count;
                    goto cleanup;
                }
            }
            offset += linked->global_count;
        }
        ctx->globals = saved_globals;
        ctx->global_count = saved_global_count;

        free(ctx->globals);
        ctx->globals = new_globals;
    }

    // Resolve each function import from linked modules
    for (uint32_t i = 0; i < import_count; i++) {
        wah_func_import_t *fi = &module->func_imports[i];

        // Find linked module by module_name
        const wah_module_t *linked = NULL;
        for (uint32_t j = 0; j < ctx->linked_module_count; j++) {
            const char *linked_name = ctx->linked_modules[j].name;
            if (strncmp(linked_name, fi->module_name, fi->module_name_len) == 0 &&
                linked_name[fi->module_name_len] == '\0') {
                linked = ctx->linked_modules[j].module;
                break;
            }
        }
        WAH_ENSURE_GOTO(linked != NULL, WAH_ERROR_IMPORT_NOT_FOUND, cleanup);

        // Find export by field_name (function kind)
        const wah_export_t *exp = NULL;
        for (uint32_t k = 0; k < linked->export_count; k++) {
            if (linked->exports[k].kind == 0 &&
                linked->exports[k].name_len == fi->field_name_len &&
                memcmp(linked->exports[k].name, fi->field_name, fi->field_name_len) == 0) {
                exp = &linked->exports[k];
                break;
            }
        }
        WAH_ENSURE_GOTO(exp != NULL, WAH_ERROR_IMPORT_NOT_FOUND, cleanup);

        // exp->index is the global function index in the linked module.
        // For host-only modules (import_function_count=0), global == local (functions[] index).
        uint32_t linked_import_count = linked->import_function_count;
        WAH_ENSURE_GOTO(exp->index >= linked_import_count, WAH_ERROR_IMPORT_NOT_FOUND, cleanup);
        uint32_t linked_local_idx = exp->index - linked_import_count;
        WAH_ENSURE_GOTO(linked_local_idx < linked->total_function_count, WAH_ERROR_IMPORT_NOT_FOUND, cleanup);

        const wah_function_t *src = &linked->functions[linked_local_idx];
        ctx->function_table[i] = *src;  // shallow copy (pointers shared with linked module)
        ctx->function_table[i].global_idx = exp->index;  // Set global index from export
        if (!src->is_host) {
            ctx->function_table[i].fn_module = linked;
            ctx->function_table[i].local_idx = linked_local_idx;
        }
    }

    // Set global_idx for all functions in module->functions array
    // Local functions have global indices starting from import_count
    // Host functions come after local functions
    for (uint32_t i = 0; i < module->function_count; i++) {
        module->functions[i].global_idx = import_count + i;
        // Also update function_table since it's a shallow copy
        ctx->function_table[import_count + i].global_idx = import_count + i;
    }

    // Set local_idx and fn_module on linked module local functions so that funcref
    // globals resolved to them can be dispatched correctly via call_indirect.
    for (uint32_t j = 0; j < ctx->linked_module_count; j++) {
        const wah_module_t *linked = ctx->linked_modules[j].module;
        for (uint32_t k = 0; k < linked->function_count; k++) {
            if (!linked->functions[k].is_host) {
                linked->functions[k].local_idx = k;
                linked->functions[k].fn_module = linked;
            }
        }
    }

    // Convert funcref globals for linked modules from prefuncref to real pointers.
    // Primary module globals were converted above (after their own eval loop);
    // linked module globals are handled here, after local_idx/fn_module are set.
    {
        uint32_t lg_offset = module->global_count;
        for (uint32_t j = 0; j < ctx->linked_module_count; j++) {
            const wah_module_t *linked = ctx->linked_modules[j].module;
            for (uint32_t k = 0; k < linked->global_count; k++) {
                if (linked->globals[k].type == WAH_TYPE_FUNCREF &&
                    ctx->globals[lg_offset + k].ref != NULL) {
                    uint32_t fidx = ctx->globals[lg_offset + k].prefuncref.func_idx;
                    uint32_t linked_import_count = linked->import_function_count;
                    if (fidx >= linked_import_count) {
                        uint32_t local_k = fidx - linked_import_count;
                        WAH_ENSURE_GOTO(local_k < linked->function_count,
                                        WAH_ERROR_VALIDATION_FAILED, cleanup);
                        ctx->globals[lg_offset + k].ref = &linked->functions[local_k];
                    } else {
                        // ref.func to linked module's own import: resolve from another linked module
                        wah_func_import_t *fi = &linked->func_imports[fidx];
                        const wah_module_t *provider = NULL;
                        for (uint32_t m = 0; m < ctx->linked_module_count; m++) {
                            const char *lname = ctx->linked_modules[m].name;
                            if (strncmp(lname, fi->module_name, fi->module_name_len) == 0 &&
                                lname[fi->module_name_len] == '\0') {
                                provider = ctx->linked_modules[m].module;
                                break;
                            }
                        }
                        WAH_ENSURE_GOTO(provider != NULL, WAH_ERROR_IMPORT_NOT_FOUND, cleanup);
                        const wah_export_t *exp = NULL;
                        for (uint32_t m = 0; m < provider->export_count; m++) {
                            if (provider->exports[m].kind == 0 &&
                                provider->exports[m].name_len == fi->field_name_len &&
                                memcmp(provider->exports[m].name, fi->field_name, fi->field_name_len) == 0) {
                                exp = &provider->exports[m];
                                break;
                            }
                        }
                        WAH_ENSURE_GOTO(exp != NULL, WAH_ERROR_IMPORT_NOT_FOUND, cleanup);
                        uint32_t provider_import_count = provider->import_function_count;
                        WAH_ENSURE_GOTO(exp->index >= provider_import_count, WAH_ERROR_IMPORT_NOT_FOUND, cleanup);
                        uint32_t provider_local_idx = exp->index - provider_import_count;
                        WAH_ENSURE_GOTO(provider_local_idx < provider->total_function_count, WAH_ERROR_IMPORT_NOT_FOUND, cleanup);
                        ctx->globals[lg_offset + k].ref = &provider->functions[provider_local_idx];
                    }
                }
            }
            lg_offset += linked->global_count;
        }
    }

    // Initialize active data segments
    for (uint32_t i = 0; i < module->data_segment_count; ++i) {
        const wah_data_segment_t *segment = &module->data_segments[i];
        if (segment->flags == 0x00 || segment->flags == 0x02) { // Active segments
            WAH_ENSURE_GOTO(segment->memory_idx < ctx->memory_count, WAH_ERROR_VALIDATION_FAILED, cleanup);
            WAH_ENSURE_GOTO((uint64_t)segment->offset + segment->data_len <= ctx->memory_sizes[segment->memory_idx], WAH_ERROR_MEMORY_OUT_OF_BOUNDS, cleanup);
            memcpy(ctx->memories[segment->memory_idx] + segment->offset, segment->data, segment->data_len);
        }
    }

    // Initialize active element segments
    for (uint32_t i = 0; i < module->element_segment_count; ++i) {
        const wah_element_segment_t *segment = &module->element_segments[i];

        // Skip passive and dropped segments
        if (!segment->is_active || segment->is_dropped) {
            continue;
        }

        WAH_ASSERT(segment->table_idx < ctx->table_count);

        // Evaluate offset expression to get the table offset
        wah_value_t offset_val;
        WAH_CHECK_GOTO(wah_eval_const_expr(ctx, segment->offset_expr.bytecode, segment->offset_expr.bytecode_size, &offset_val), cleanup);
        uint32_t offset = (uint32_t)offset_val.i32;

        WAH_ASSERT((uint64_t)offset + segment->num_elems <= module->tables[segment->table_idx].min_elements);

        for (uint32_t j = 0; j < segment->num_elems; ++j) {
            if (!segment->is_expr_elem) {
                uint32_t global_func_idx = segment->u.func_indices[j];
                WAH_ENSURE_GOTO(global_func_idx < ctx->function_table_count, WAH_ERROR_VALIDATION_FAILED, cleanup);
                ctx->tables[segment->table_idx][offset + j].ref = &ctx->function_table[global_func_idx];
            } else {
                wah_value_t elem_val;
                WAH_CHECK_GOTO(wah_eval_const_expr(ctx,
                                                   segment->u.expr.bytecodes[j],
                                                   segment->u.expr.bytecode_sizes[j],
                                                   &elem_val), cleanup);
                if (elem_val.ref == NULL) {
                    ctx->tables[segment->table_idx][offset + j].ref = NULL;
                } else {
                    uint32_t global_func_idx = elem_val.prefuncref.func_idx;
                    WAH_ENSURE_GOTO(global_func_idx < ctx->function_table_count, WAH_ERROR_VALIDATION_FAILED, cleanup);
                    ctx->tables[segment->table_idx][offset + j].ref = &ctx->function_table[global_func_idx];
                }
            }
        }
    }

    // If a start function is defined, call it after all imports/globals/elements are ready.
    if (module->has_start_function) {
        WAH_CHECK_GOTO(wah_call_module(ctx, module->start_function_idx, NULL, 0, NULL), cleanup);
    }

    ctx->is_instantiated = true;
    return WAH_OK;

cleanup:
    return err;
}

// --- Export API Implementation ---
size_t wah_module_num_exports(const wah_module_t *module) {
    if (!module) return 0;
    return module->export_count;
}

wah_error_t wah_module_export(const wah_module_t *module, size_t idx, wah_entry_t *out) {
    WAH_ENSURE(module, WAH_ERROR_MISUSE);
    WAH_ENSURE(out, WAH_ERROR_MISUSE);
    WAH_ENSURE(idx < module->export_count, WAH_ERROR_NOT_FOUND);

    const wah_export_t *export_entry = &module->exports[idx];

    // For host function exports, handle directly via the unified functions[] table.
    // export_entry->index is a global function index; convert to functions[] local index.
    uint32_t exp_local_idx = export_entry->index - module->import_function_count;
    if (export_entry->kind == 0 &&
        module->functions &&
        export_entry->index >= module->import_function_count &&
        exp_local_idx < module->total_function_count &&
        module->functions[exp_local_idx].is_host) {
        const wah_function_t *fn = &module->functions[exp_local_idx];

        out->id = WAH_MAKE_ENTRY_ID(WAH_ENTRY_KIND_FUNCTION, export_entry->index);
        out->type = WAH_TYPE_HOST_FUNCTION;
        out->name = export_entry->name;
        out->name_len = export_entry->name_len;
        out->is_mutable = false;

        out->u.func.param_count = (uint32_t)fn->nparams;
        out->u.func.param_types = fn->param_types;
        out->u.func.result_count = (uint32_t)fn->nresults;
        out->u.func.result_types = fn->result_types;

        return WAH_OK;
    }

    wah_entry_id_t entry_id;
    switch (export_entry->kind) {
        case 0: // Function
            entry_id = WAH_MAKE_ENTRY_ID(WAH_ENTRY_KIND_FUNCTION, export_entry->index);
            break;
        case 1: // Table
            entry_id = WAH_MAKE_ENTRY_ID(WAH_ENTRY_KIND_TABLE, export_entry->index);
            break;
        case 2: // Memory
            entry_id = WAH_MAKE_ENTRY_ID(WAH_ENTRY_KIND_MEMORY, export_entry->index);
            break;
        case 3: // Global
            entry_id = WAH_MAKE_ENTRY_ID(WAH_ENTRY_KIND_GLOBAL, export_entry->index);
            break;
        default:
            WAH_ASSERT(false && "verification should have prevented invalid export kinds");
            return WAH_ERROR_VALIDATION_FAILED;
    }

    WAH_CHECK(wah_module_entry(module, entry_id, out));

    out->name = export_entry->name;
    out->name_len = export_entry->name_len;

    return WAH_OK;
}

wah_error_t wah_module_export_by_name(const wah_module_t *module, const char *name, wah_entry_t *out) {
    WAH_ENSURE(module, WAH_ERROR_MISUSE);
    WAH_ENSURE(name, WAH_ERROR_MISUSE);
    WAH_ENSURE(out, WAH_ERROR_MISUSE);

    size_t lookup_name_len = strlen(name);

    for (uint32_t i = 0; i < module->export_count; ++i) {
        const wah_export_t *export_entry = &module->exports[i];
        if (export_entry->name_len == lookup_name_len && strncmp(export_entry->name, name, lookup_name_len) == 0) {
            return wah_module_export(module, i, out);
        }
    }
    return WAH_ERROR_NOT_FOUND;
}

wah_error_t wah_module_entry(const wah_module_t *module, wah_entry_id_t entry_id, wah_entry_t *out) {
    WAH_ENSURE(module, WAH_ERROR_MISUSE);
    WAH_ENSURE(out, WAH_ERROR_MISUSE);

    uint32_t kind = WAH_GET_ENTRY_KIND(entry_id);
    uint32_t index = WAH_GET_ENTRY_INDEX(entry_id);

    out->id = entry_id;
    out->name = NULL; // No name for non-exported entries
    out->name_len = 0;
    out->is_mutable = false; // Default to false

    switch (kind) {
        case WAH_ENTRY_KIND_FUNCTION: {
            uint32_t import_count = module->import_function_count;
            if (index < import_count) {
                // Import range: return type from func_imports
                WAH_ENSURE(module->func_imports && index < import_count, WAH_ERROR_NOT_FOUND);
                wah_func_import_t *fi = &module->func_imports[index];
                WAH_ENSURE(fi->type_index < module->type_count, WAH_ERROR_NOT_FOUND);
                const wah_func_type_t *ft = &module->types[fi->type_index];
                out->type = WAH_TYPE_FUNCTION;
                out->u.func.param_count = ft->param_count;
                out->u.func.param_types = ft->param_types;
                out->u.func.result_count = ft->result_count;
                out->u.func.result_types = ft->result_types;
            } else {
                // Local/host range: index into functions[] is (index - import_count)
                uint32_t local_fn_idx = index - import_count;
                WAH_ENSURE(module->functions && local_fn_idx < module->total_function_count, WAH_ERROR_NOT_FOUND);
                if (module->functions[local_fn_idx].is_host) {
                    const wah_function_t *fn = &module->functions[local_fn_idx];
                    out->type = WAH_TYPE_HOST_FUNCTION;
                    out->u.func.param_count = (uint32_t)fn->nparams;
                    out->u.func.param_types = fn->param_types;
                    out->u.func.result_count = (uint32_t)fn->nresults;
                    out->u.func.result_types = fn->result_types;
                } else {
                    WAH_ENSURE(local_fn_idx < module->function_count, WAH_ERROR_NOT_FOUND);
                    out->type = WAH_TYPE_FUNCTION;
                    const wah_func_type_t *func_type = &module->types[module->function_type_indices[local_fn_idx]];
                    out->u.func.param_count = func_type->param_count;
                    out->u.func.param_types = func_type->param_types;
                    out->u.func.result_count = func_type->result_count;
                    out->u.func.result_types = func_type->result_types;
                }
            }
            break;
        }
        case WAH_ENTRY_KIND_TABLE:
            WAH_ENSURE(index < module->table_count, WAH_ERROR_NOT_FOUND);
            out->type = WAH_TYPE_TABLE;
            out->u.table.elem_type = module->tables[index].elem_type;
            out->u.table.min_elements = module->tables[index].min_elements;
            out->u.table.max_elements = module->tables[index].max_elements;
            break;
        case WAH_ENTRY_KIND_MEMORY:
            WAH_ENSURE(index < module->memory_count, WAH_ERROR_NOT_FOUND);
            out->type = WAH_TYPE_MEMORY;
            out->u.memory.min_pages = module->memories[index].min_pages;
            out->u.memory.max_pages = module->memories[index].max_pages;
            break;
        case WAH_ENTRY_KIND_GLOBAL:
            WAH_ENSURE(index < module->global_count, WAH_ERROR_NOT_FOUND);
            out->type = module->globals[index].type;
            out->is_mutable = module->globals[index].is_mutable;
            // Note: Global initial values are now evaluated at instantiation time
            // and stored in the execution context, not in the module.
            // This API only provides type information, not current values.
            memset(&out->u.global_val, 0, sizeof(wah_value_t));
            break;
        default:
            return WAH_ERROR_NOT_FOUND; // Unknown entry kind
    }
    return WAH_OK;
}

#endif // WAH_IMPLEMENTATION
#endif // WAH_H
