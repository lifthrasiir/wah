# WAH (WebAssembly in a Header) Project Overview

This project implements a WebAssembly (WASM) interpreter entirely within a single C header file (`wah.h`). It provides the necessary data structures, parsing logic, and execution engine to run WebAssembly modules.

## Implementation Strategy

The WAH interpreter employs a multi-phase approach for efficient WebAssembly module execution:

1. **Validation:** After parsing the WASM binary structure, each function's bytecode undergoes validation (`wah_validate_opcode`) to ensure type safety, stack discipline, and instruction validity.

2. **Pre-parsing:** After validation, the bytecode undergoes a two-pass "pre-parsing" phase (`wah_preparse_code`) to optimize interpretation:
   - **Pass 1: Analysis and Layout** - Scans raw WASM bytecode to calculate final size and resolve jump targets. Identifies control flow blocks and resolves jump destinations.
   - **Pass 2: Code Generation** - Generates optimized bytecode with fixed-size native types, pre-calculated jump offsets, and structural opcodes removed.

3. **Custom Call Stack:** WAH implements its own explicit call stack (`wah_call_frame_t` within `wah_exec_context_t`) for portability and fine-grained execution control.

4. **Interpreter Dispatch:** Three dispatch strategies controlled by compiler capabilities:
   - **Switch-based:** Traditional while loop with switch statement
   - **Computed Goto:** Direct threading with jump table (`WAH_USE_COMPUTED_GOTO` for GCC/Clang)
   - **Direct Threaded:** Tail call optimization with `__attribute__((musttail))` (`WAH_USE_MUSTTAIL`)

## Module Linkage and Cross-Module Calls

WAH supports linking multiple WebAssembly modules together, enabling cross-module function calls:

- **Linking API:** Use `wah_link_module()` to link additional modules to the primary execution context before instantiation.
- **Import Resolution:** During `wah_instantiate()`, the interpreter resolves imports by matching import declarations to exports from linked modules.
- **Wasm-to-Wasm Calls:** Fully supported. A wasm function in one module can call wasm functions in linked modules through the import mechanism.
- **Host Functions:** Linking also works with host functions exported via `wah_module_export_func*()`.
- **Globals:** Each module maintains its own global variables. The execution context allocates separate globals space for each linked module, and the call frame tracks the correct globals offset for cross-module calls.

**Usage Pattern:**
```c
wah_module_t primary = {0};
wah_module_t linked = {0};
// ... parse or create modules ...

wah_exec_context_t ctx = {0};
wah_exec_context_create(&ctx, &primary);
wah_link_module(&ctx, "linkedModuleName", &linked);
wah_instantiate(&ctx);  // Resolves imports from linked modules
// ... call functions ...
wah_exec_context_destroy(&ctx);
```

## Error Handling

All public API functions return a `wah_error_t` enum value. `WAH_OK` indicates success. Use `wah_strerror(err)` for human-readable error descriptions.

## Development Notes

- **Self-contained:** Define `WAH_IMPLEMENTATION` in exactly one C/C++ file. Do not define `WAH_IMPLEMENTATION` from the command line.
- **Dependencies:** Standard C libraries only (`stdint.h`, `stddef.h`, `stdbool.h`, `string.h`, `stdlib.h`, `assert.h`, `math.h`, `stdio.h`).
- **Platform-specific:** Uses compiler intrinsics with generic fallbacks.
- **Emoji Prohibition:** No emoji usage in source files. Use text-based markers.
- **WAH_DEBUG:** Define for debugging. `WAH_LOG` provides printf-like logging with line numbers. Use `./test.bat -g` to enable in test runs.

## Testing

- **Windows:** Run `./test.bat` for all tests. Run `./test.bat <prefix>` for specific tests (e.g., `./test.bat linkage`).
    - Successful test binaries get automatically removed. Consider using `gdb` on failed tests.
- **POSIX:** Run `make test` for all tests. Run `make test_<prefix>` for specific tests.
- **Exit Codes:** Tests return 0 only on success, non-zero on any failure.
- **Regression Tests:** Always add a failing regression test before fixing bugs to demonstrate the fix.
- **Test Utilities:** Use `tests/common.c` DSL for creating simple wasm modules in tests without external tools.

## Critical Implementation Details

- **Overflow Checks:** Explicitly check for numeric overflow during arithmetic operations in parsing. Report with `WAH_ERROR_TOO_LARGE`.

- **Function Index Space:** Functions are identified in a global index space: [imports, local functions, host functions]. When calling via `wah_call()`, use the global index. The function_table maps these indices to actual function implementations.
