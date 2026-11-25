# wah

Safe Rust bindings to [WAH](https://github.com/lifthrasiir/wah), a WebAssembly interpreter in a single C header
(with an optional baseline JIT for x86-64 and AArch64). WAH implements the WebAssembly 3.0 deterministic profile and
is meant to run untrusted modules with predictable resource bounds.

The API is close to Wasmtime's: a `Module` is parsed and validated once, instantiated in a `Store` that holds the
host's data, with host functions and memories defined by a `Linker`; a `ResourceLimiter` bounds memories and tables,
and host functions may wait (`Linker::func_wrap_poll`, `Func::call_async`) without fibers.

```rust
use wah::*;

let wasm = wat::parse_str(r#"(module
    (import "env" "double" (func $double (param i32) (result i32)))
    (func (export "run") (param i32) (result i32) (call $double (local.get 0))))"#)?;
let module = Module::new(&Config::new(), &wasm)?;
let mut linker = Linker::<()>::new();
linker.func_wrap("env", "double", |_: Caller<'_, ()>, x: i32| x * 2)?;
let mut store = Store::new(());
let instance = linker.instantiate(&mut store, &module)?;
let run = instance.get_typed_func::<i32, i32>(&store, "run")?;
assert_eq!(run.call(&mut store, 21)?, 42);
```

The features `wasm2` and `wasm3` (default) and those of each proposal choose what is compiled in, and `jit` (default)
the native compiler. See [`rust/README.md`](https://github.com/lifthrasiir/wah/blob/main/rust/README.md) for how the
API maps to Wasmtime's and what is not there yet, and [the repository](https://github.com/lifthrasiir/wah) for WAH
itself. The raw bindings are [`wah-sys`](https://crates.io/crates/wah-sys).

Licensed under MIT-0.
