# WAH (WebAssembly in a Header) Project Overview

This project implements a WebAssembly (WASM) interpreter entirely within a single C header file (`wah.h`). It provides the necessary data structures, parsing logic, and execution engine to run WebAssembly modules.

## File Structure

- `wah.h` (~16,000 lines): The entire interpreter. Lines 1-1326 are the public API (before `#ifdef WAH_IMPLEMENTATION` on line 1326). Everything after is implementation. The public portion is C++11-compatible.
- `tests/common.c` / `tests/common.h`: DSL-based test utility for constructing WASM modules from spec strings (see "Test DSL" below).
- `tests/test_*.c`: Individual test files, each is a standalone program that includes `common.c` (which includes `wah.h`). `test_cpp.cpp` exercises the C++ surface.
- `tests/spectest.c` / `tests/wast.c`: WebAssembly spec test runner infrastructure.
- `tests/spectest/`: Directory containing `.wast` spec test files.
- `fuzz/fuzz_libfuzzer.c`, `fuzz/fuzz_afl.c`: Fuzzing harnesses with corpora under `fuzz/corpus*` and `fuzz/min_corpus`.
- `test.ps1`: Windows test runner with compiler selection (`-clang`, `-gcc`, `-msvc`). `Makefile`: POSIX test runner.

## Public API Surface

The public types `wah_module_t`, `wah_exec_context_t`, `wah_call_context_t`, and `wah_value_t` are stack-allocatable but with internals hidden behind `#ifdef WAH_IMPLEMENTATION`. Their sizes and alignments are pinned by static asserts (`<= 384` bytes, 16-byte aligned) for ABI stability. Inspect modules through descriptor accessors (`wah_module_function`, `wah_module_global`, `wah_module_memory`, `wah_module_table`, `wah_module_tag`, `wah_module_type`, `wah_module_import`, `wah_module_export`, `wah_export_by_name[_len]`) which fill out `wah_func_desc_t` / `wah_global_desc_t` / etc. Counts come from `wah_module_*_count`.

Naming convention is `wah_<verb>_<noun>` where possible. There are no `_ex` constructor variants; options are passed via dedicated structs (`wah_parse_options_t`, `wah_exec_options_t`).

### Custom Allocator

`wah_alloc_t` (with `malloc` / `realloc` / `free` / `userdata`) can be supplied at parse time (`wah_parse_options_t::alloc`), at exec context creation (`wah_exec_options_t::alloc`), and at programmatic module construction (`wah_new_module(mod, alloc)`). Parse and exec allocators are independent and must be set separately. NULL means the standard allocator.

## Implementation Strategy

The WAH interpreter employs a multi-phase approach for efficient WebAssembly module execution:

1. **Parsing:** `wah_parse_module()` decodes the WASM binary into `wah_module_t`, parsing all sections (type, import, function, table, memory, global, export, element, data, code, tag). Pass `wah_parse_options_t` for feature flags, fuel metering opt-in, and a custom allocator; pass `NULL` for defaults. The parser performs validation up front: type vector counts, br_table targets, local counts, element counts, and GC array operands are all checked before any allocation, and const expressions are bounded to avoid stack overflow.

2. **Validation:** Each function's bytecode undergoes validation (`wah_validate_opcode`) to ensure type safety, stack discipline, and instruction validity. Validation simultaneously produces a `wah_analyzed_code_t` IR (decoded instructions with resolved symbols).

3. **Lowering:** `wah_lower_analyzed_code()` converts the analyzed IR into compact preparsed bytecode (`wah_parsed_code_t`). This bytecode uses fixed-size 16-bit opcodes (read via `wah_read_u16_le`), native-endian immediates, and pre-calculated jump offsets. Structural opcodes (block/loop/if/else/end; note the function level END is retained) are removed; only executable opcodes remain.

4. **Instantiation:** `wah_instantiate()` resolves imports from linked modules, initializes memories/tables/globals, evaluates const expressions, runs element/data segment initialization, and executes the start function if present. After instantiation, modules referenced by an exec context are treated as immutable; the exec context never mutates linked modules.

5. **Execution:** `wah_run_interpreter()` executes the preparsed bytecode using one of three dispatch strategies:
   - **Switch-based:** Traditional `while (1) { switch (opcode) { ... } }` (default on MSVC)
   - **Computed Goto:** GCC/Clang `goto *labels[opcode]` (`WAH_USE_COMPUTED_GOTO`)
   - **Musttail:** Tail-call dispatch with `__attribute__((musttail))` (`WAH_USE_MUSTTAIL`, Clang 13+ / GCC 15+)

   All three use the same opcode handler code via macros: `WAH_RUN(opcode)`, `WAH_NEXT()`, `WAH_CLEANUP()`.

## Execution Model

WAH supports two calling patterns:

1. **Synchronous:** `wah_call()` / `wah_call_multi()` / `wah_call_by_name()` run to completion in a single call. `wah_call()` returns `WAH_ERROR_MULTI_RETURN` without executing if the target function has multiple return values; use `wah_call_multi` in that case. `wah_call_by_name` looks up an exported function by name.

2. **Resumable:** Three-phase model for cooperative multitasking:
   - `wah_start()` sets up a call frame without executing.
   - `wah_resume()` drives execution until completion, fuel exhaustion (`WAH_STATUS_FUEL_EXHAUSTED`), or interruption (`WAH_STATUS_YIELDED`).
   - `wah_finish()` extracts results after `WAH_OK`.
   - `wah_cancel()` discards a suspended activation.
   - `wah_is_suspended()` / `wah_exec_state()` query the activation state.
   - You must call `wah_finish()` after `WAH_OK` to get results; calling `wah_call()` on an already-running context is misuse.

### Resource Limits

Three independent limits can be active simultaneously and compose cleanly. They are configured through `wah_rlimits_t` (initial values via `wah_exec_options_t::limits`; updated later via `wah_set_limits` / `wah_get_limits`):

- **Fuel metering:** Opt-in at parse time (`enable_fuel_metering` in `wah_parse_options_t`). Each basic block gets a METER opcode that decrements `ctx->fuel`. Bulk operations (memory.fill/copy/init, table.copy/init, etc.) charge fuel at a rate of one unit per `WAH_BULK_ITEMS_PER_FUEL` items (4096 by default, tunable at compile time). Set runtime fuel via `wah_set_fuel()` / `wah_get_fuel()` or via `wah_rlimits_t.fuel`. Fuzzing also consumes fuel during instantiation.

- **Deadline timer:** `wah_rlimits_t.deadline_us` (microseconds; `UINT64_MAX` = no deadline). A background thread monitors wall-clock time and sets `interrupt_flag` when elapsed. TICK opcodes at loop back-edges check the flag cooperatively.

- **Memory budget:** `wah_rlimits_t.max_memory_bytes` caps the sum of linear memory, tables, and GC heap (`UINT64_MAX` = unlimited; `0` is treated as the default). `wah_rlimits_t.no_memory_bytes` enforces a hard 0-byte limit (mutually exclusive with a non-zero `max_memory_bytes`). `wah_rlimits_t.max_stack_bytes` separately bounds the unified value-call stack.

## Key Architecture Details

### Unified Value-Call Stack

The interpreter uses a single flat allocation: values (`wah_value_t`) grow upward, call frames (`wah_call_frame_t`) grow downward from `stack_end`. Overflow is detected when the two regions meet (single comparison). Locals and operand stack share the value array; each call frame's `locals_offset` marks where its locals begin, and `sp` tracks the top.

### Opcode System

Opcodes are defined via X-macros in `WAH_OPCODES(X)`. Multi-byte WASM opcodes (0xFB, 0xFC, 0xFD prefixes) are remapped to a contiguous enum space via `WAH_FB`, `WAH_FC`, `WAH_FD` offset constants. Additional "extra" opcodes exist for specialized variants:

- **Memory fast paths:** `WAH_I32_MEM0_OPCODES_M` generates `_i32_mem0` suffixed opcodes for memory-0 operations (avoiding indirection through `memories[]`).
- **64-bit memory/table:** `WAH_I64_MEM_OPCODES_M`, `WAH_I64_TABLE_OPCODES` for memory64/table64 support. Mixed i32/i64 `memory.copy` and `table.copy` use a reworked dispatch.
- **x86-64 SIMD specializations:** `WAH_X86_64_EXTRA_OPCODES_SINGLE/MULTI` generate architecture-specific opcodes (e.g., `_ssse3`, `_sse41`, `_avx512` suffixes) selected at runtime via CPUID in `wah_x86_64_opcode()`.
- **Internal opcodes:** `WAH_INTERNAL_OPCODES` defines `POLL`, `TICK`, `METER`, `END_TRY_TABLE`, `REF_FUNC_CONST`, `GLOBAL_GET_INDIRECT`, `GLOBAL_SET_INDIRECT` (generated by lowering, not present in WASM bytecode). TICK handles interrupt/deadline checks; METER handles fuel decrements.

Opclasses (`wah_opclass_t`) classify opcodes by their stack signature (e.g., `_II_I` means pop i32, pop i32, push i32) for bulk validation.

### Feature Flags

`wah_features_t` is a 64-bit bitmask controlling which proposals are enabled. `WAH_FEATURE_MVP` is a non-zero sentinel always implied; passing `0` for features means "use the default", not "disable everything". Features are opt-in via `wah_parse_module()` with a `wah_parse_options_t`. Instructions requiring disabled features return `WAH_ERROR_DISABLED_FEATURE`. Presets: `WAH_FEATURE_WASM_V2`, `WAH_FEATURE_WASM_V3`, `WAH_FEATURE_ALL`. Features can also be selectively disabled at compile time via `WAH_COMPILED_FEATURES` (default = `WAH_FEATURE_ALL`) and the runtime default tuned via `WAH_DEFAULT_FEATURES`.

### GC Implementation

The GC uses a mark-sweep collector (`wah_gc_step`). Objects are allocated with `wah_gc_alloc()` which prepends a `wah_gc_object_t` header containing a tagged next pointer (low 2 bits encode mark/aux flags); allocation sizes are checked for header overflow. `wah_repr_info_t` describes struct/array layouts with field offsets and a reference map for tracing; struct layout is computed by a padding-minimizing builder. i31ref values use pointer tagging (low bit = 1). `wah_gc_alloc_host` allocates externrefs/anyrefs from the host side; `wah_gc_heap_stats[_from_host]` and `wah_gc_verify_heap` expose introspection.

### Type System

`wah_type_t` is `int32_t`: negative values are built-in types (`WAH_TYPE_I32 = -2`, ..., `WAH_TYPE_NOEXN = -54`), non-negative values are type indices into `module->types[]` / `module->type_defs[]`. `wah_type_def_t` describes composite types (func/struct/array) with subtyping, rec-groups, and finality. In both cases odd numbers denote a nullable variant of even numbers (`WAH_TYPE_FUNC = -32, WAH_TYPE_FUNCREF = -31`). A `canonical_map[]` maps each type index to its canonical representative for structural equality. `type_cast_sets[]` precompute which repr IDs are accepted by each type as sparse bitsets for O(1) `ref.test`/`ref.cast`. Cross-module subtype checks are cached in a 64-entry direct-mapped `type_check_cache[]` (collisions overwrite).

### Deterministic Profile

WAH targets the WebAssembly 3.0 deterministic profile. NaN canonicalization produces positive canonical NaNs (`0x7fc00000` for f32, `0x7ff8000000000000` for f64), including NEON SIMD lanes which are canonicalized by bit pattern. FP contraction is disabled (`#pragma fp_contract(off)` / `STDC FP_CONTRACT OFF`). On x86-64, `sqrtf`/`sqrt` use SSE intrinsics instead of libm to ensure correct rounding. On MSVC, `fabs`/`fabsf` need special handling to preserve NaN payloads.

### Cross-Module Calls

When a WASM function calls into a linked module, the call frame stores `frame->module`, `frame->frame_globals`, `frame->frame_function_table`, and `frame->frame_ctx` to track the callee's module context. The `fctx` variable in the interpreter is the "active" exec context for memory/table access, swapped on cross-module transitions.

## Module Construction and Linkage

WAH supports both binary-parsed modules and programmatically constructed modules, and links multiple of them together:

- **Programmatic build:** `wah_new_module(&mod, alloc)` creates an empty module. Use `wah_define_type` / `wah_define_typev` to declare types (with the `fresh` keyword to bypass structural deduplication). Use `wah_export_func` (type-string DSL) or `wah_export_typed_func` (precomputed `wah_type_t`) for host functions; `wah_export_memory` and `wah_export_global_{i32,i64,f32,f64,v128}` for other exports.
- **Linking API:** Use `wah_link_module()` to link a parsed/built module by import name, or `wah_link_context()` to link an already-instantiated exec context.
- **Import Resolution:** During `wah_instantiate()`, the interpreter resolves imports by matching import declarations to exports from linked modules. Failures return `WAH_ERROR_LINK_FAILED` (formerly `WAH_ERROR_IMPORT_NOT_FOUND`).

**Usage Pattern:**
```c
wah_module_t primary = {0};
wah_module_t linked = {0};
// ... parse or build modules ...

wah_exec_context_t ctx = {0};
wah_new_exec_context(&ctx, &primary, NULL);
wah_link_module(&ctx, "linkedModuleName", &linked);
wah_instantiate(&ctx);  // Resolves imports from linked modules
// ... call functions ...
wah_free_exec_context(&ctx);
wah_free_module(&primary);
wah_free_module(&linked);
```

## Error Handling

All public API functions return a `wah_error_t` enum value. `WAH_OK` indicates success. Use `wah_strerror(err)` for human-readable error descriptions. Resumable status codes (`WAH_STATUS_FUEL_EXHAUSTED`, `WAH_STATUS_YIELDED`) are positive values distinct from error codes. `WAH_ERROR_MULTI_RETURN` (no longer the success-flavored `WAH_OK_BUT_MULTI_RETURN`) signals that a single-return entry point was used on a multi-return function.

Internal error propagation uses helper macros:
- `WAH_CHECK(expr)`: Returns early if `expr` fails.
- `WAH_CHECK_GOTO(expr, label)`: Jumps to `label` on failure (stores error in `err`).
- `WAH_ENSURE(cond, error)`: Returns `error` if `cond` is false.
- `WAH_MALLOC_ARRAY(ptr, count)` / `WAH_REALLOC_ARRAY(ptr, count)`: Safe allocation with overflow checking and OOM handling.
- `WAH_ENSURE_CAP(arr, count, cap)` / `WAH_ENSURE_CAP_GOTO(...)`: Growable array capacity management.

OOM and leak hygiene are tested explicitly (`tests/test_oom.c` injects allocation failures, and the codebase is run under ASAN/UBSAN/LSAN — see Makefile flags).

## Development Notes

- **Self-contained:** Define `WAH_IMPLEMENTATION` in exactly one C/C++ file. Do not define `WAH_IMPLEMENTATION` from the command line.
- **Dependencies:** Standard C libraries only (`stdint.h`, `stddef.h`, `stdbool.h`, `string.h`, `stdlib.h`, `assert.h`, `math.h`, `stdio.h`, `stdarg.h`).
- **Platform-specific SIMD:** x86-64 uses SSE2 baseline with runtime detection for SSSE3/SSE4.1/SSE4.2/AVX2/AVX-512 via CPUID. AArch64 uses NEON. Inline asm wrappers (`WAH_ASM_*` macros) allow using higher ISA instructions without `-march` flags. Generic fallbacks exist for all SIMD ops.
- **Emoji and non-ASCII Prohibition:** Any Unicode character beyond ASCII range is prohibited in source files. Use text-based markers.
- **WAH_DEBUG:** Define for debugging. `WAH_LOG` provides printf-like logging with line numbers. Use `./test.ps1 -g` on Windows or `make DEBUG=1 test` on POSIX to enable in test runs. For DSL errors (`WAH_ERROR_BAD_SPEC`) `WAH_LOG` also prints a detailed error message and position.
- **Coding conventions:** Assume C99 for the implementation; the public header is C++11-compatible. Strive to be compact as long as readability is maintained. Avoid non-portable features, including using `memset` to initialize struct with pointers (use a compound literal instead).
- **WAH_FORCE_* macros:** `WAH_FORCE_PORTABLE` (scalar-only SIMD fallbacks), `WAH_FORCE_SWITCH` / `WAH_FORCE_COMPUTED_GOTO` / `WAH_FORCE_MUSTTAIL` (dispatch strategy overrides). Default priority: musttail > computed goto > switch.

## Testing

- **Windows:** Run `./test.ps1` for all tests. Run `./test.ps1 <prefix>` for specific tests (e.g., `./test.ps1 linkage`).
    - Uses `clang` by default. Pass exactly one of `-clang`, `-gcc`, or `-msvc` to select a compiler (e.g., `./test.ps1 -msvc`, `./test.ps1 -gcc linkage`).
    - Successful test binaries (and `.pdb` files) get automatically removed. Precompiled helper objects (`wah_impl.o`, `common.o`) are reused across test runs; delete them manually when you need a fully fresh build.
- **POSIX:** Run `make test` for all tests. Run `make test_<prefix>` for specific tests.
    - Uses `gcc` compiler. Set `DEBUG=1` for debug builds. Sanitizer flags (ASAN/UBSAN) are wired in via the Makefile.
- **Exit Codes:** Tests return 0 only on success, non-zero on any failure.
- **Regression Tests:** Always add a failing regression test before fixing bugs to demonstrate the fix.
- **Test split:** Tests that `#define WAH_IMPLEMENTATION` compile standalone (link with `common.o` only). Tests without it are API-only and link with the precompiled `wah_impl.o + common.o`. This split is the key build optimization.
- **Fuzzing:** `fuzz/fuzz_libfuzzer.c` (libFuzzer) and `fuzz/fuzz_afl.c` (AFL) drive `wah_parse_module` + instantiation + execution under fuel and memory caps; corpora live under `fuzz/corpus*`.
- **CI:** 8 configurations: Linux (gcc/clang on x86-64 + aarch64), macOS (clang), Windows (msvc/gcc/clang). Windows GCC/Clang use MSYS2.

### Build Gotcha

`wah_impl.o` is compiled with `-O1` even in debug builds because unoptimized musttail dispatch is ~15x slower on Windows GCC/Clang. This means `-O0` debugging may require `WAH_FORCE_SWITCH` to get reasonable performance.

### Performance Tips

Prefer `-msvc` on Windows for initial runs, as it is much faster than GCC and Clang. Also consider using `-j#` for parallel building on POSIX, but do not try to use all available cores at once (that hurts interactivity).

### Test DSL (`tests/common.c`)

Tests construct WASM modules using `wah_parse_module_from_spec()` with a string DSL instead of hand-crafting binary data. Key syntax:

- **Bare words** map to WASM bytes: `wasm` (magic+version), `types`/`funcs`/`code`/etc. (section IDs), `i32`/`i64`/`f32`/`f64` (type bytes), `fn` (0x60), opcodes like `local.get`/`i32.add`/`end`.
- **Numbers** encode as LEB128: `42`, `-1`. Float literals use suffixes: `3.14f32`, `2.71828f64`.
- **`[ ... ]`** auto-counts comma-separated items and prepends the count as a varint.
- **`{ ... }`** auto-measures byte length and prepends it as a varint.
- **Verbatims:** `'name'` for raw strings. `%'414243'` for hex bytes.
- **Placeholders:** `%d32`/`%d64` for signed LEB128, `%i32`/`%i64` for fixed-width LE integers, `%f32`/`%f64` for floats, `%v128` for 16 bytes, `%t` for recursive DSL string.

Example: A module exporting an `add` function:
```c
wah_parse_module_from_spec(&mod, "wasm \
    types {[fn [i32,i32] [i32]]} funcs {[0]} \
    exports {[ {'add'} fn# 0 ]} \
    code {[{[] local.get 0 local.get 1 i32.add end}]}");
```

Keep in mind that this DSL is very basic and you are required to produce a correctly ordered and well-formed spec string. Look for other test cases as examples. Use `WAH_DEBUG` to get detailed error messages and positions as a last resort.

### Type Spec DSL

`wah_define_type()` and `wah_export_func()` accept human-readable type strings:

- Function types: `"fn (i32, i64) -> (f64)"`, `"fn () -> ()"`. The `fn` prefix may be omitted in `wah_export_func` (function context only).
- Composite types: `"struct { i32, mut funcref }"`, `"array i32"`, `"array mut anyref"`.
- Reference types: `"ref (struct %T)"`, `"ref null func"`.
- `%T` placeholders bind to module type indices at runtime.
- Prefix `fresh` on a definition to force a distinct type instead of structural deduplication (the default merges identical types).
- Packed types (i8, i16) are only valid in struct field / array element contexts, not as bare function parameter types.

## Critical Implementation Details

- **Overflow Checks:** Explicitly check for numeric overflow during arithmetic operations in parsing. Report with `WAH_ERROR_TOO_LARGE`. `table.init` ranges and other bulk ops verify ranges without overflowing.

- **Preparsed Bytecode Format:** Each opcode is a 16-bit value (little-endian). Immediates follow in native sizes: `uint32_t` for indices, `uint64_t` for offsets, etc. Branch targets are absolute byte offsets into the preparsed bytecode. The `operand_ref_map` bitmap tracks which value stack slots hold GC references at each POLL point.

- **POLL/TICK/METER Opcodes:** POLL is inserted at loop back-edges and function entry; it drives stop-the-world GC and checks `poll_flag`. TICK checks `interrupt_flag` (set by deadline timer or external threads). METER decrements fuel per basic block; bulk ops charge fuel proportional to the work performed and reset their accounting at chunk boundaries. The GC cannot run between POLL points, so reference tracking only needs to be precise at these points.

- **Exception Handling:** Uses `try_table` with a fixed-depth exception handler stack (`WAH_MAX_EXCEPTION_HANDLER_DEPTH = 64`). `wah_exception_t` carries tag ID, values, and types. `throw` sets `pending_exception` and unwinds the call stack looking for matching handlers.

- **Host Function Integration:** `wah_export_func()` accepts a function type string DSL (e.g., `"(i32, i32) -> i32"`). Host functions receive a `wah_call_context_t` with typed param/result accessors (`wah_param_*`, `wah_result_*`, `wah_param_type`, `wah_result_type`, `wah_param_count`, `wah_result_count`). Return values are truncated to declared types just like wasm-side returns. Use `wah_trap` to abort with a specific error. GC references in params are valid only for the call duration; host must not retain pointers.
