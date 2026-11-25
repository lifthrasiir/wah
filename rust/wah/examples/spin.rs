//! Times a loop calling a host function, interpreted and natively compiled.
use std::time::Instant;
use wah::*;

fn main() {
    let wasm = wat::parse_str(
        r#"(module
        (import "env" "tick" (func $tick (param i32)))
        (func (export "run") (param i32) (result i32) (local i32)
            (loop $l
                (local.set 1 (i32.add (local.get 1) (i32.mul (local.get 0) (i32.const 3))))
                (if (i32.eqz (i32.and (local.get 0) (i32.const 0xffff))) (then (call $tick (local.get 0))))
                (br_if $l (local.tee 0 (i32.sub (local.get 0) (i32.const 1)))))
            (local.get 1)))"#,
    )
    .unwrap();
    let mut linker = Linker::<u32>::new();
    linker.func_wrap("env", "tick", |mut c: Caller<'_, u32>, _: i32| *c.data_mut() += 1).unwrap();
    for jit in [false, true] {
        let m = Module::new(Config::new().jit(jit), &wasm).unwrap();
        let mut store = Store::new(0);
        let i = linker.instantiate(&mut store, &m).unwrap();
        let run = i.get_typed_func::<i32, i32>(&store, "run").unwrap();
        let t = Instant::now();
        let r = run.call(&mut store, 200_000_000).unwrap();
        println!("jit {jit}: {r} {} ticks in {:?}", store.data(), t.elapsed());
    }
}
