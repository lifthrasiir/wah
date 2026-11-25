# <sub><sub><img src="./wah.png" width="64" height="64" valign="middle" alt=""></sub></sub> WebAssembly interpreter in a Header (WAH)

This is **WAH**, a [WebAssembly](https://webassembly.org/) interpreter implemented in a single C header file. It implements the WebAssembly 3.0 [deterministic profile](https://webassembly.github.io/spec/core/appendix/profiles.html#deterministic-profile-small-mathrm-det) -- GC, exception handling, relaxed SIMD, multi-memory, memory64/table64, tail calls, extended const, and everything else in the WASM 2.0/3.0 lineup -- and is designed to run untrusted modules in production with predictable resource bounds.

WAH is a single C99 header (~20k lines). The public API is also C++11-compatible. There are no external dependencies beyond the standard library. An optional second header, `wahjit.h`, adds an experimental baseline JIT compiler for x86-64 and AArch64 without changing the API, and `rust/` provides Rust bindings.

Much of WAH is formally checked as well: CBMC proofs cover the decoders, allocation wrappers, resource accounting and every interpreter handler, and Lean 4 proofs show the type system and the checker of the internal bytecode sound against the spec (see [Verification](#verification)).

## Quick Start

Define `WAH_IMPLEMENTATION` in exactly one translation unit. Everywhere else, just `#include "wah.h"`.

```c
#define WAH_IMPLEMENTATION
#include "wah.h"
#include <stdio.h>
#include <stdlib.h>

static uint8_t *slurp(const char *path, size_t *out_size) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *buf = (uint8_t *)malloc(size);
    if (buf && fread(buf, 1, size, f) != (size_t)size) { free(buf); buf = NULL; }
    fclose(f);
    *out_size = size;
    return buf;
}

int main(int argc, char *argv[]) {
    size_t size;
    uint8_t *buf = slurp(argv[1], &size);

    wah_module_t mod = {0};
    wah_exec_context_t ctx = {0};
    wah_error_t err;

    if ((err = wah_parse_module(&mod, buf, size, NULL))) goto fail;
    free(buf);
    if ((err = wah_new_exec_context(&ctx, &mod, NULL))) goto fail;

    wah_value_t params[2] = {{.i32 = 3}, {.i32 = 4}};
    wah_value_t result;
    if ((err = wah_call_by_name(&ctx, "add", params, 2, &result))) goto fail;
    printf("add(3, 4) = %d\n", result.i32); // 7

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
    return 0;

fail:
    fprintf(stderr, "error: %s\n", wah_strerror(err));
    return 1;
}
```

### Calling host functions from WebAssembly

A module can be built programmatically and exposed to wasm callers without ever touching a `.wasm` file:

```c
static void host_print(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    printf("wasm called: %d\n", wah_param_i32(ctx, 0));
}

wah_module_t host = {0};
wah_new_module(&host, NULL);
wah_export_func(&host, "print", "(i32) -> ()", host_print, NULL, NULL);

wah_exec_context_t ctx = {0};
wah_new_exec_context(&ctx, &user_module, NULL);
wah_link_module(&ctx, "env", &host);  // satisfies (import "env" "print" ...)
wah_instantiate(&ctx);
```

Host functions can also suspend their activation with `wah_suspend` (the call is retried with the same parameters on `wah_resume`), reach per-context state with `wah_exec_userdata`, and read or write the memories of the instantiated context with `wah_memory_data`. Memories may be supplied by the host through `wah_exec_options_t::memory_creator`, and memories and tables accounted as they are made, grow and are freed by `wah_exec_options_t::memory_limiter` and `table_limiter`. A host module can be shared by many exec contexts.

See `tests/test_export_api.c`, `tests/test_linkage.c` and `tests/test_embedding.c` for fuller examples.

### Sandboxing untrusted modules

Three independent limits compose on the same execution context:

```c
wah_parse_options_t popts = { .enable_fuel_metering = true };
wah_parse_module(&mod, buf, size, &popts);

wah_limits_t lim = {
    .max_stack_bytes  = 1 << 20,          // 1 MiB unified value/call stack
    .max_memory_bytes = 64 * 1024 * 1024, // linear memory + tables + GC heap
    .fuel             = 100000000,        // basic-block + bulk-op metering
    .deadline_us      = 5 * 1000 * 1000,  // 5 seconds wall-clock
};
wah_set_limits(&ctx, &lim);
```

Fuel exhaustion and deadline expiration surface as `WAH_STATUS_FUEL_EXHAUSTED` / `WAH_STATUS_YIELDED` from the resumable API (`wah_start` / `wah_resume` / `wah_finish`), so cooperative multitasking and watchdog-style cancellation both fall out naturally.

## Platform Support

| Platform | Compilers | SIMD |
|---|---|---|
| Linux x86-64 | gcc, clang | SSE2 baseline; SSSE3 / SSE4.1 / SSE4.2 / AVX2 / AVX-512 dispatched at runtime via CPUID |
| Linux aarch64 | gcc, clang | NEON |
| macOS arm64 | clang | NEON |
| Windows x86-64 | MSVC, gcc, clang (MSYS2) | same as Linux x86-64 |

CI exercises all eight configurations on every push. Define `WAH_FORCE_PORTABLE` for a scalar-only build that runs anywhere with a C99 compiler.

The interpreter automatically picks the fastest available dispatch strategy: musttail tail calls with the `preserve_none` calling convention (Clang 19+ and GCC 16+ on x86-64 and AArch64, except GCC on Windows), then computed goto (GCC/Clang), then a portable `switch` (default on MSVC). Override with `WAH_FORCE_MUSTTAIL` / `WAH_FORCE_COMPUTED_GOTO` / `WAH_FORCE_SWITCH`.

## Optional JIT

Include `wahjit.h` instead of `wah.h` in the file defining `WAH_IMPLEMENTATION` to enable an experimental baseline JIT compiler; other files keep including `wah.h`. It supports x86-64 on Windows, macOS and Linux (SSE4.1 and POPCNT required, SSE4.2 for SIMD) and AArch64 on macOS and Linux. Elsewhere, or with `WAH_NO_JIT`, it is the same as `wah.h`.

The JIT compiles modules without fuel metering that use WASM 2.0 instructions plus relaxed SIMD, multi-memory and memory64, except reference types and table instructions other than `call_indirect`; other modules, or activations involving linked wasm modules, tables or reference globals, are transparently interpreted. Native execution differs from the interpreter only in resource usage and polling behavior, so results, traps and the resumable API stay identical. Set `wah_parse_options_t::disable_jit` to opt out per module.

`wah_enable_guard_pages` lets native code skip bounds checks of 32-bit memories by reserving 6 GiB of address space per memory, either with signal / vectored exception handlers that WAH installs (`WAH_HANDLES_FAULT`) or with the host forwarding faults to `wah_handle_fault` (`WAH_HOST_HANDLES_FAULT`).

## Rust Bindings

`rust/` is a Cargo workspace of `wah-sys` (raw FFI, building `wah.h` or `wahjit.h` with the `cc` crate) and `wah` (a safe API close to Wasmtime's, including async host functions built on `wah_suspend`). Cargo features select the proposals compiled in. See [`rust/README.md`](./rust/README.md) for how the APIs map to Wasmtime's.

## Build Footprint

WAH is intended to be vendored: copy `wah.h` into your tree and you are done. There is no build system to integrate, no library to link.

WebAssembly proposals can be turned off at compile time via `WAH_COMPILED_FEATURES` to drop their code paths entirely (e.g. omit GC, SIMD, or exceptions in size-constrained builds), and individually toggled at parse time via `wah_parse_options_t::features` for finer-grained per-module gating. The runtime default is `WAH_DEFAULT_FEATURES` (= `WAH_COMPILED_FEATURES` = `WAH_FEATURE_ALL` unless overridden).

A custom `wah_alloc_t` can be plugged in for parse-time, exec-time, and module-construction allocations independently, so WAH can run with a host-supplied arena, pool, or tracking allocator.

## Coverage and Robustness

WAH passes all currently included files in the official WebAssembly spectest corpus with zero failures (based on spec commit [`c840c58`](https://github.com/WebAssembly/spec/commit/c840c58c21304946f009808b9e5e686b1734d8f0)). One local test-data fix is applied: `const.bin.wast` corrected per [WebAssembly/spec#2150](https://github.com/WebAssembly/spec/issues/2150), using regenerated Linux-produced binaries to avoid the upstream generator bug.

Under the deterministic profile, all `[!DET]`-excluded nondeterminism is eliminated: generated NaN values are canonical and positive, and relaxed SIMD instructions have fixed deterministic behavior. Spec-allowed nondeterminism (`memory.grow`/`table.grow` failure due to resource exhaustion) remains.

Beyond the spec corpus, WAH carries an extensive battery of unit tests covering every major feature, the resumable / fuel / deadline / memory-limit surfaces, OOM injection, GC root marking, cross-module linkage, and host-function lifecycle. The codebase is regularly run under ASAN, UBSAN, and LSAN, and is fuzzed with both libFuzzer and AFL harnesses (`fuzz/`).

## Verification

WAH validates modules and then lowers them into an internal bytecode that the interpreter trusts. Beyond testing, the following are checked (CI runs all but `make cbmc-step`, `make lean-spectec` and `make spec-diff`, which need minutes or external tools):

- **Lowered code checker:** with `WAH_CHECK_LOWERED` (always on in tests and fuzzing), every lowered function is re-checked independently of the validator: instruction boundaries, branch targets, operand types, GC reference maps at suspension points, object layouts and exception handlers.
- **CBMC proofs** (`make cbmc`, `proofs/*.c`) check C functions for all inputs within loop bounds: LEB128, UTF-8, opcode and type decoders, range checks, float min/max, rounding and truncations, allocation wrappers, fuel and memory budget accounting, the pin table, i31 references and struct layouts. `make cbmc-step` checks that every interpreter handler moves the value stack, frames and instruction pointer as the abstract machine of the Lean model does.
- **Lean 4 proofs** (`make lean`, `proofs/lean`, no Mathlib) model parts of `wah.h` and prove their properties: struct layouts, type canonicalization, same- and cross-module subtyping and casts following the subtyping of the spec, and the soundness of the lowered code checker (accepted code never goes wrong in the abstract machine). Each model has a bridge to the C code, either a CBMC harness or a differential comparison (`make lean-diff`) on the spec tests and the fuzz corpus. `make lean-spectec` further proves the subtyping of the model sound and complete for the Lean model that [SpecTec](https://github.com/WebAssembly/spec/tree/main/spectec) generates from the spec itself.
- **Spec oracle:** `make spec-diff` compares WAH with the reference interpreter of the spec on the fuzz corpus and on modules generated by `wasm-tools smith`, calling exports with random arguments, and optionally checks validation and instantiation against the SpecTec interpreter.

## Versioning

The `WAH_VERSION` macro identifies the API revision, incremented on any change to the public surface including bug fixes. It is based on the fractional year since 2000 times 100, negated for development versions after the release of the same number (e.g. `-2675` is the development after `2675`). Public type sizes and alignments are pinned by static asserts so that a bumped `WAH_VERSION` never silently breaks ABI on consumers that link against a precompiled `WAH_IMPLEMENTATION` translation unit.

## License

WAH is a Public Domain ([equivalent](./LICENSE.txt)) software. The vast majority of its code has been generated by [Gemini CLI](https://github.com/google-gemini/gemini-cli/) and [Claude Code](https://claude.ai/code), albeit with substantial human guidance and fixes, making it a copyrightable work.
