# Rust bindings of WAH

- `wah-sys`: raw FFI of `wah.h`, building the implementation from `../wah.h` (or `wahjit.h`, feature `jit`, default)
  with the `cc` crate; `WAH_SRC_DIR` points elsewhere. The build fails where `WAH_VERSION` of the header differs from
  that of the bindings (`src/lib.rs`), which then need updating for the API changed. The C is always built with at least `-O2`. On MSVC targets
  `cl.exe` gives the switch dispatch of the interpreter; `CC=clang-cl` interprets about 20% faster (the JIT is the
  same either way).
  `cargo test -p wah-sys` checks the Rust layouts against the C ones.
- `wah`: a safe API close to Wasmtime's, for Aqueous (`crates/aqr-wasmrt`) to run on.

Both crates select the proposals compiled in (`WAH_COMPILED_FEATURES`) by features of their names: `multi-value`,
`mutable-globals`, `sign-ext`, `nontrapping-f2i`, `bulk-memory`, `ref-types`, `simd`, `tail-call`, `exception`,
`gc`, `typed-funcref`, `memory64`, `relaxed-simd`, `extended-const`, `multi-memory`, and the groups `wasm2` and
`wasm3` (default). Without any, only WebAssembly 1.0 is; dependencies are implied (`exception` needs `gc`, which needs
`ref-types`; `relaxed-simd` needs `simd`). Modules using a proposal left out fail with `Code::DisabledFeature`.
`Features::COMPILED` tells what is compiled in, which `Config::new()` enables. For example Aqueous's profile is
`default-features = false, features = ["jit", "wasm2", "memory64", "multi-memory"]`.

| Wasmtime | wah |
| --- | --- |
| `Engine` + `Config` | `Config` alone, given to `Module::new` |
| `Module::new(&engine, wasm)` | `Module::new(&config, wasm)`: `Send + Sync`, cheap to clone |
| `Store::new(&engine, data)` | `Store::new(data)` |
| `store.limiter(\|s\| s)`, `ResourceLimiter::memory_growing`, `table_growing` | the same (also asked as memories and tables are made, `current` = 0, before anything is allocated); `memory_grow_failed` and `table_grow_failed` are told the sizes and return nothing |
| `Memory::new(&mut store, ty)` + `Instance::new(&mut store, &module, &externs)` | `Linker::memory` / `func_wrap` by import names, `linker.instantiate_hosts(&mut store)`, `store.get_memory(module, name)` to write it, then `Instance::new(&mut store, &module)` (or `linker.instantiate` at once) |
| `Func::wrap(&mut store, \|c: Caller<'_, T>, ...\| ...)` | `linker.func_wrap(module, name, \|c: Caller<'_, T>, ...\| ...)` |
| `InstancePre` (host functions resolved once) | host modules are built once for their items and shared by every store (and clone of the linker); a store only makes their memories and globals |
| `Config::with_host_memory(Arc<dyn MemoryCreator>)` | `store.set_memory_creator(Arc<dyn MemoryCreator>)`, for memories made from then on; `LinearMemory` never moves (`reserved` given only with guard pages) |
| `Func::wrap_async` (fibers) | `linker.func_wrap_poll(module, name, \|c, cx: &mut Context<'_>, ...\| -> Poll<_>)`: `Pending` suspends the activation in WAH, and the function is called again with the same parameters when polled again (no fiber, no state kept across the wait) |
| `func.call_async(&mut store, ...)` | the same; `call` waits in its thread |
| `memory.data_and_store_mut(&mut caller)` | the same |
| `wasmtime::Error::new(e)`, `downcast_ref` | `wah::Error::new(e)`, `downcast_ref`; `wah_code()` gives traps (`Code::StackOverflow` etc.) |

Not yet: reference values across the boundary, globals' values from the host, growing memories from the host.

## Publishing

Cargo packages only the files under each crate, so `vendor.py` copies `wah.h`, `wahjit.h` and `LICENSE.txt` into
them (`wah-sys/vendor/`, ignored by git). Packaged builds use these copies; builds within the repository use the
originals, warning where the copies are stale.

```sh
python vendor.py
cargo publish -p wah-sys
cargo publish -p wah
```

`cargo package` verifies `wah` against the `wah-sys` it just packaged, but Cargo keeps the sources it extracted of a
version: where `wah` fails against an older `wah-sys` of the same version, remove
`~/.cargo/registry/src/-<hash>/wah-sys-<version>` (of the temporary registry, not of crates.io) and try again.
