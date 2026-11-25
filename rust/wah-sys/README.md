# wah-sys

Raw FFI bindings to [WAH](https://github.com/lifthrasiir/wah), a WebAssembly interpreter in a single C header. The
crate builds the implementation from the bundled `wah.h` (or `wahjit.h`, with its baseline JIT for x86-64 and AArch64:
the feature `jit`, default) with the `cc` crate, and declares the C API as it is (`wah_parse_module`,
`wah_new_exec_context`, `wah_call`, ...). `WAH_SRC_DIR` builds other copies of the headers instead.

Most users want [`wah`](https://crates.io/crates/wah), the safe API close to Wasmtime's. The features `wasm2` and
`wasm3` (default) and those of each proposal set `WAH_COMPILED_FEATURES`; `debug` defines `WAH_DEBUG`. The headers'
comments document the API; see [the repository](https://github.com/lifthrasiir/wah) for WAH itself and
[`rust/README.md`](https://github.com/lifthrasiir/wah/blob/main/rust/README.md) for the bindings.

Licensed under MIT-0.
