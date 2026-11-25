use std::collections::VecDeque;
use std::fmt;
use std::future::Future;
use std::pin::pin;
use std::sync::{Arc, Mutex};
use std::task::{Context, Poll, Waker};

use wah::*;

/// Every test runs interpreted and (where the platform has it) natively compiled.
fn configs() -> [Config; 2] {
    let mut interp = Config::new();
    interp.jit(false);
    [interp, Config::new()]
}

fn module(config: &Config, wat: &str) -> Module {
    Module::new(config, &wat::parse_str(wat).unwrap()).unwrap()
}

#[test]
fn calls() {
    for c in configs() {
        let m = module(
            &c,
            r#"(module
                (func (export "add") (param i32 i32) (result i32) local.get 0 local.get 1 i32.add)
                (func (export "pair") (param i64) (result i64)
                    local.get 0 i64.const 1 i64.add))"#,
        );
        let mut store = Store::new(());
        let i = Linker::new().instantiate(&mut store, &m).unwrap();
        let add = i.get_typed_func::<(i32, i32), i32>(&store, "add").unwrap();
        assert_eq!(add.call(&mut store, (40, 2)).unwrap(), 42);
        assert!(i.get_typed_func::<(i32, i64), i32>(&store, "add").is_err());
        let pair = i.get_func(&store, "pair").unwrap();
        let mut out = [Val::I64(0)];
        pair.call(&mut store, &[Val::I64(7)], &mut out).unwrap();
        assert_eq!(out, [Val::I64(8)]);
        assert!(pair.call(&mut store, &[Val::I32(7)], &mut out).is_err());
    }
}

#[cfg(feature = "multi-value")]
#[test]
fn multi_value() {
    for c in configs() {
        let m = module(
            &c,
            r#"(module (func (export "pair") (param i64) (result i64 f64)
                local.get 0 i64.const 1 i64.add local.get 0 f64.convert_i64_s))"#,
        );
        let mut store = Store::new(());
        let i = Instance::new(&mut store, &m).unwrap();
        let mut out = [Val::I64(0), Val::F64(0)];
        i.get_func(&store, "pair").unwrap().call(&mut store, &[Val::I64(7)], &mut out).unwrap();
        assert_eq!(out, [Val::I64(8), Val::from(7.0f64)]);
        let typed = i.get_typed_func::<i64, (i64, f64)>(&store, "pair").unwrap();
        assert_eq!(typed.call(&mut store, 7).unwrap(), (8, 7.0));
    }
}

/// Modules of each proposal, accepted where it is compiled in and rejected otherwise.
#[test]
fn compiled_features() {
    let cases = [
        (Features::MULTI_VALUE, r#"(module (func (result i32 i32) i32.const 1 i32.const 2))"#),
        (Features::SIGN_EXT, r#"(module (func (param i32) (result i32) local.get 0 i32.extend8_s))"#),
        (Features::NONTRAPPING_F2I, r#"(module (func (param f32) (result i32) local.get 0 i32.trunc_sat_f32_s))"#),
        (
            Features::BULK_MEMORY,
            r#"(module (memory 1) (func (memory.fill (i32.const 0) (i32.const 0) (i32.const 0))))"#,
        ),
        (Features::SIMD, r#"(module (func (result v128) v128.const i64x2 0 0))"#),
        (Features::TAIL_CALL, r#"(module (func $f (return_call $f)))"#),
        (Features::MEMORY64, r#"(module (memory i64 1))"#),
        (Features::MULTI_MEMORY, r#"(module (memory 1) (memory 1))"#),
        (Features::EXTENDED_CONST, r#"(module (global i32 (i32.add (i32.const 1) (i32.const 2))))"#),
        (Features::EXCEPTION, r#"(module (tag $t) (func (throw $t)))"#),
    ];
    for (f, wat) in cases {
        let r = Module::new(&Config::new(), &wat::parse_str(wat).unwrap());
        if Features::COMPILED.contains(f) {
            r.unwrap();
        } else {
            assert_eq!(r.unwrap_err().wah_code(), Some(Code::DisabledFeature), "{f:?} compiled out");
        }
    }
    assert_eq!(Config::new().get_features(), Features::COMPILED);
    assert_eq!(Config::new().features(Features::ALL).get_features(), Features::COMPILED);
}

#[cfg(all(feature = "memory64", feature = "multi-memory"))]
/// Memories made by the host (as Aqueous's `aqr` imports), written before instantiation (and so under data
/// segments), and host functions working on them.
#[test]
fn host_memories() {
    for c in configs() {
        let m = module(
            &c,
            r#"(module
                (import "aqr" "aqr_memory" (memory $w 1))
                (import "aqr" "aqr_params" (memory $p i64 1 1))
                (import "aqr" "aqr_sum" (func $sum (param i32 i32) (result i64)))
                (data (memory $w) (i32.const 0) "\05")
                (func (export "run") (param i64) (result i64)
                    (i64.store8 $w (i32.const 1) (i64.load8_u $p (i64.const 3)))
                    (i64.add (call $sum (i32.const 0) (i32.const 4)) (local.get 0))))"#,
        );
        let mut linker = Linker::<u64>::new();
        linker.memory("aqr", "aqr_memory", MemoryType::new(1, None)).unwrap();
        linker.memory("aqr", "aqr_params", MemoryType::new64(1, Some(1))).unwrap();
        linker
            .func_wrap("aqr", "aqr_sum", |mut c: Caller<'_, u64>, at: i32, len: i32| -> Result<i64> {
                let mem = c.get_memory("aqr", "aqr_memory").unwrap();
                let (d, calls) = mem.data_and_store_mut(&mut c);
                *calls += 1;
                let Some(r) = d.get(at as usize..(at + len) as usize) else { return Err(Error::msg("out of bounds")) };
                Ok(r.iter().map(|&b| b as i64).sum())
            })
            .unwrap();
        let mut store = Store::new(0u64);
        linker.instantiate_hosts(&mut store).unwrap();
        let w = store.get_memory("aqr", "aqr_memory").unwrap();
        let p = store.get_memory("aqr", "aqr_params").unwrap();
        assert!(p.ty(&store).is_64() && p.size(&store) == 1);
        w.data_mut(&mut store)[..4].copy_from_slice(&[100, 100, 1, 2]);
        p.data_mut(&mut store)[3] = 30;
        let i = Instance::new(&mut store, &m).unwrap();
        let run = i.get_typed_func::<i64, i64>(&store, "run").unwrap();
        // 5 (the data segment) + 30 (the params) + 1 + 2, + 1000
        assert_eq!(run.call(&mut store, 1000).unwrap(), 1038);
        assert_eq!(*store.data(), 1);
        assert_eq!(&w.data(&store)[..4], &[5, 30, 1, 2]);
    }
}

#[derive(Debug)]
struct Violation(String);

impl fmt::Display for Violation {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str(&self.0)
    }
}

impl std::error::Error for Violation {}

#[test]
fn errors_and_traps() {
    for c in configs() {
        let m = module(
            &c,
            r#"(module
                (import "env" "fail" (func $fail (param i32)))
                (func (export "fail") (param i32) (call $fail (local.get 0)))
                (func (export "unreachable") unreachable)
                (func $rec (export "rec") (call $rec))
                (func (export "div") (param i32) (result i32) (i32.div_u (i32.const 1) (local.get 0))))"#,
        );
        let mut linker = Linker::<()>::new();
        linker
            .func_wrap("env", "fail", |_: Caller<'_, ()>, how: i32| -> Result<()> {
                match how {
                    0 => Ok(()),
                    1 => Err(Error::new(Violation("bad".into()))),
                    _ => panic!("host panic"),
                }
            })
            .unwrap();
        let mut store = Store::new(());
        let i = linker.instantiate(&mut store, &m).unwrap();
        let fail = i.get_typed_func::<i32, ()>(&store, "fail").unwrap();
        fail.call(&mut store, 0).unwrap();
        let e = fail.call(&mut store, 1).unwrap_err();
        assert_eq!(e.downcast_ref::<Violation>().unwrap().0, "bad");
        let p = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| fail.call(&mut store, 2)));
        assert_eq!(*p.unwrap_err().downcast::<&str>().unwrap(), "host panic");
        let e = i.get_typed_func::<(), ()>(&store, "unreachable").unwrap().call(&mut store, ()).unwrap_err();
        assert_eq!(e.wah_code(), Some(Code::Trap));
        store.set_max_stack_bytes(1 << 16);
        let e = i.get_typed_func::<(), ()>(&store, "rec").unwrap().call(&mut store, ()).unwrap_err();
        assert_eq!(e.wah_code(), Some(Code::StackOverflow));
        let div = i.get_typed_func::<i32, i32>(&store, "div").unwrap();
        assert_eq!(div.call(&mut store, 0).unwrap_err().wah_code(), Some(Code::Trap));
        // (the store still works after all of them)
        assert_eq!(div.call(&mut store, 1).unwrap(), 1);
        fail.call(&mut store, 0).unwrap();
    }
}

#[test]
fn link_failures() {
    for c in configs() {
        let m = module(&c, r#"(module (import "env" "f" (func (param i32))))"#);
        let mut store = Store::new(());
        let e = Instance::new(&mut store, &m).unwrap_err();
        assert_eq!(e.wah_code(), Some(Code::LinkFailed));
        let mut linker = Linker::<()>::new();
        linker.func_wrap("env", "f", |_: Caller<'_, ()>, _: i64| {}).unwrap();
        let e = linker.instantiate(&mut Store::new(()), &m).unwrap_err();
        assert_eq!(e.wah_code(), Some(Code::LinkFailed));
    }
}

#[cfg(feature = "multi-memory")]
/// Counts the bytes of memories (wasmtime's `ResourceLimiter`, as Aqueous's `State` is one).
struct Budget {
    total: u64,
    limit: u64,
    deny: bool,
}

#[cfg(feature = "multi-memory")]
impl ResourceLimiter for Budget {
    fn memory_growing(&mut self, current: u64, desired: u64, _: Option<u64>) -> Result<bool> {
        let total = self.total - current + desired;
        if total > self.limit {
            return if self.deny { Ok(false) } else { Err(Error::new(Violation("over the limit".into()))) };
        }
        self.total = total;
        Ok(true)
    }
}

#[cfg(feature = "multi-memory")]
#[test]
fn limiter() {
    for c in configs() {
        let m = module(
            &c,
            r#"(module
                (import "aqr" "aqr_memory" (memory 1))
                (memory $own 2)
                (func (export "grow") (param i32) (result i32) (memory.grow (local.get 0))))"#,
        );
        let mut linker = Linker::<Budget>::new();
        linker.memory("aqr", "aqr_memory", MemoryType::new(1, None)).unwrap();
        let mut store = Store::new(Budget { total: 0, limit: 5 * PAGE_SIZE, deny: true });
        store.limiter(|b| b);
        let i = linker.instantiate(&mut store, &m).unwrap();
        // (both memories created: the host's and the module's own)
        assert_eq!(store.data().total, 3 * PAGE_SIZE);
        let grow = i.get_typed_func::<i32, i32>(&store, "grow").unwrap();
        assert_eq!(grow.call(&mut store, 2).unwrap(), 1);
        assert_eq!(store.data().total, 5 * PAGE_SIZE);
        assert_eq!(grow.call(&mut store, 1).unwrap(), -1);
        store.data_mut().deny = false;
        let e = grow.call(&mut store, 1).unwrap_err();
        assert_eq!(e.downcast_ref::<Violation>().unwrap().0, "over the limit");

        // a creation over the limit
        let mut store = Store::new(Budget { total: 0, limit: 2 * PAGE_SIZE, deny: false });
        store.limiter(|b| b);
        let e = linker.instantiate(&mut store, &m).unwrap_err();
        assert!(e.is::<Violation>());
    }
}

/// A channel as Aqueous's `aqr_send`/`aqr_recv` are: waiting host functions, polled by its executor.
#[derive(Default)]
struct Chan {
    msgs: VecDeque<i32>,
    receiving: Vec<Waker>,
    sending: Vec<Waker>,
    cap: usize,
}

type Shared = Arc<Mutex<Chan>>;

fn chan_linker() -> Linker<Shared> {
    let mut linker = Linker::<Shared>::new();
    linker
        .func_wrap_poll("env", "send", |c: Caller<'_, Shared>, cx: &mut Context<'_>, v: i32| -> Poll<()> {
            let mut ch = c.data().lock().unwrap();
            if ch.msgs.len() >= ch.cap {
                ch.sending.push(cx.waker().clone());
                return Poll::Pending;
            }
            ch.msgs.push_back(v);
            ch.receiving.drain(..).for_each(Waker::wake);
            Poll::Ready(())
        })
        .unwrap();
    linker
        .func_wrap_poll("env", "recv", |c: Caller<'_, Shared>, cx: &mut Context<'_>| -> Poll<Result<i32>> {
            let mut ch = c.data().lock().unwrap();
            let Some(v) = ch.msgs.pop_front() else {
                ch.receiving.push(cx.waker().clone());
                return Poll::Pending;
            };
            ch.sending.drain(..).for_each(Waker::wake);
            Poll::Ready(if v < 0 { Err(Error::msg("negative")) } else { Ok(v) })
        })
        .unwrap();
    linker
}

const PRODUCER: &str = r#"(module
    (import "env" "send" (func $send (param i32)))
    (func (export "run") (param i32) (local i32)
        (loop $l
            (call $send (local.get 1))
            (local.set 1 (i32.add (local.get 1) (i32.const 1)))
            (br_if $l (i32.lt_u (local.get 1) (local.get 0))))))"#;

const CONSUMER: &str = r#"(module
    (import "env" "recv" (func $recv (result i32)))
    (func (export "run") (param i32) (result i32) (local i32)
        (loop $l
            (local.set 1 (i32.add (local.get 1) (call $recv)))
            (br_if $l (local.tee 0 (i32.sub (local.get 0) (i32.const 1)))))
        (local.get 1)))"#;

/// Polls the futures in turn until all are done, as a single-threaded executor would (wakers ignored).
fn run_all<F: Future + Unpin>(futs: &mut [Option<F>]) -> Vec<F::Output> {
    let mut out: Vec<Option<F::Output>> = futs.iter().map(|_| None).collect();
    let mut cx = Context::from_waker(Waker::noop());
    for _ in 0..10_000 {
        if out.iter().all(Option::is_some) {
            return out.into_iter().map(Option::unwrap).collect();
        }
        for (f, o) in futs.iter_mut().zip(&mut out) {
            if let Some(fut) = f {
                if let Poll::Ready(r) = std::pin::Pin::new(fut).poll(&mut cx) {
                    *o = Some(r);
                    *f = None;
                }
            }
        }
    }
    panic!("a deadlock")
}

#[test]
fn waiting_host_functions() {
    for c in configs() {
        let (p, q) = (module(&c, PRODUCER), module(&c, CONSUMER));
        let shared = Shared::new(Mutex::new(Chan { cap: 2, ..Chan::default() }));
        let linker = chan_linker();
        let mut sp = Store::new(shared.clone());
        let mut sq = Store::new(shared.clone());
        let ip = linker.instantiate(&mut sp, &p).unwrap();
        let iq = linker.instantiate(&mut sq, &q).unwrap();
        let fp = ip.get_func(&sp, "run").unwrap();
        let fq = iq.get_func(&sq, "run").unwrap();
        let (mut rp, mut rq) = ([], [Val::I32(0)]);
        {
            let a = Box::pin(fp.call_async(&mut sp, &[Val::I32(100)], &mut rp));
            let b = Box::pin(fq.call_async(&mut sq, &[Val::I32(100)], &mut rq));
            type Boxed<'a> = std::pin::Pin<Box<dyn Future<Output = Result<()>> + 'a>>;
            let mut futs: Vec<Option<Boxed<'_>>> = vec![Some(a), Some(b)];
            for r in run_all(&mut futs) {
                r.unwrap();
            }
        }
        assert_eq!(rq[0], Val::I32((0..100).sum()));

        // an error of a host function that waited, and a call dropped while suspended
        shared.lock().unwrap().msgs.push_back(-1);
        let e = fq.call(&mut sq, &[Val::I32(1)], &mut rq).unwrap_err();
        assert_eq!(e.to_string(), "negative");
        {
            let mut f = pin!(fq.call_async(&mut sq, &[Val::I32(1)], &mut rq));
            assert!(f.as_mut().poll(&mut Context::from_waker(Waker::noop())).is_pending());
        }
        shared.lock().unwrap().msgs.push_back(5);
        fq.call(&mut sq, &[Val::I32(1)], &mut rq).unwrap();
        assert_eq!(rq[0], Val::I32(5));
    }
}

/// `Func::call` waits in its thread for a waker of another thread.
#[test]
fn blocking_call() {
    for c in configs() {
        let q = module(&c, CONSUMER);
        let shared = Shared::new(Mutex::new(Chan { cap: 1, ..Chan::default() }));
        let mut sq = Store::new(shared.clone());
        let iq = chan_linker().instantiate(&mut sq, &q).unwrap();
        let run = iq.get_typed_func::<i32, i32>(&sq, "run").unwrap();
        let producer = std::thread::spawn(move || {
            for v in 1..=10 {
                loop {
                    let mut ch = shared.lock().unwrap();
                    if ch.msgs.len() < ch.cap {
                        ch.msgs.push_back(v);
                        ch.receiving.drain(..).for_each(Waker::wake);
                        break;
                    }
                    drop(ch);
                    std::thread::yield_now();
                }
            }
        });
        assert_eq!(run.call(&mut sq, 10).unwrap(), 55);
        producer.join().unwrap();
    }
}

#[cfg(feature = "bulk-memory")]
/// A module shared by threads instantiating it at once, and stores moved between threads (suspended in between).
#[test]
fn threads() {
    for c in configs() {
        let m = module(
            &c,
            r#"(module
                (import "aqr" "aqr_memory" (memory 1))
                (func (export "fill") (param i32)
                    (memory.fill (i32.const 0) (local.get 0) (i32.const 65536))))"#,
        );
        let mut linker = Linker::<()>::new();
        linker.memory("aqr", "aqr_memory", MemoryType::new(1, None)).unwrap();
        let handles: Vec<_> = (0..8)
            .map(|k| {
                let (m, linker) = (m.clone(), linker.clone());
                std::thread::spawn(move || {
                    let mut store = Store::new(());
                    let i = linker.instantiate(&mut store, &m).unwrap();
                    i.get_typed_func::<i32, ()>(&store, "fill").unwrap().call(&mut store, k).unwrap();
                    let w = i.get_memory(&store, "aqr_memory");
                    assert!(w.is_none());
                    let w = store.get_memory("aqr", "aqr_memory").unwrap();
                    assert!(w.data(&store).iter().all(|&b| b == k as u8));
                    store
                })
            })
            .collect();
        let stores: Vec<Store<()>> = handles.into_iter().map(|h| h.join().unwrap()).collect();
        assert_eq!(stores.len(), 8);

        let q = module(&c, CONSUMER);
        let shared = Shared::new(Mutex::new(Chan { cap: 1, ..Chan::default() }));
        let mut sq = Store::new(shared.clone());
        let iq = chan_linker().instantiate(&mut sq, &q).unwrap();
        let run = iq.get_func(&sq, "run").unwrap();
        let t = std::thread::spawn(move || {
            let mut rq = [Val::I32(0)];
            {
                let mut f = Box::pin(run.call_async(&mut sq, &[Val::I32(2)], &mut rq));
                let mut cx = Context::from_waker(Waker::noop());
                assert!(f.as_mut().poll(&mut cx).is_pending());
                // (the suspended call moved to another thread)
                std::thread::scope(|s| {
                    s.spawn(|| {
                        shared.lock().unwrap().msgs.extend([3, 4]);
                        let mut cx = Context::from_waker(Waker::noop());
                        assert!(f.as_mut().poll(&mut cx).is_ready());
                    });
                });
            }
            rq[0]
        });
        assert_eq!(t.join().unwrap(), Val::I32(7));
    }
}

#[test]
fn fuel_and_deadline() {
    let mut c = Config::new();
    c.consume_fuel(true);
    let m = module(&c, r#"(module (func (export "spin") (loop $l (br $l))))"#);
    let mut store = Store::new(());
    let i = Instance::new(&mut store, &m).unwrap();
    let spin = i.get_typed_func::<(), ()>(&store, "spin").unwrap();
    store.set_fuel(10_000);
    assert_eq!(spin.call(&mut store, ()).unwrap_err().wah_code(), Some(Code::OutOfFuel));
    assert_eq!(store.get_fuel(), Some(0));

    for c in configs() {
        let m = module(&c, r#"(module (func (export "spin") (loop $l (br $l))))"#);
        let mut store = Store::new(());
        let i = Instance::new(&mut store, &m).unwrap();
        store.set_deadline(Some(std::time::Duration::from_millis(20))).unwrap();
        let spin = i.get_typed_func::<(), ()>(&store, "spin").unwrap();
        assert_eq!(spin.call(&mut store, ()).unwrap_err().wah_code(), Some(Code::Interrupted));
    }
}

/// Memories of a fixed capacity, made by the global allocator, logging what is done with them.
struct Buffers {
    log: Arc<Mutex<Vec<String>>>,
    capacity: usize,
    fail: bool,
}

struct Buffer {
    ptr: *mut u8,
    capacity: usize,
    size: usize,
    log: Arc<Mutex<Vec<String>>>,
}

// SAFETY: the buffer is only reached through the memory
unsafe impl Send for Buffer {}
unsafe impl Sync for Buffer {}

impl Buffer {
    fn layout(capacity: usize) -> std::alloc::Layout {
        std::alloc::Layout::from_size_align(capacity, 1 << 16).unwrap()
    }
}

impl Drop for Buffer {
    fn drop(&mut self) {
        self.log.lock().unwrap().push(format!("free {}", self.size));
        // SAFETY: of `new_memory`
        unsafe { std::alloc::dealloc(self.ptr, Buffer::layout(self.capacity)) };
    }
}

// SAFETY: zeroed buffers that never move
unsafe impl LinearMemory for Buffer {
    fn byte_size(&self) -> usize {
        self.size
    }

    fn byte_capacity(&self) -> usize {
        self.capacity
    }

    fn grow_to(&mut self, new_size: usize) -> Result<()> {
        if new_size == 3 << 16 {
            panic!("grown to 3 pages");
        }
        self.log.lock().unwrap().push(format!("grow {} {new_size}", self.size));
        self.size = new_size;
        Ok(())
    }

    fn as_ptr(&self) -> *mut u8 {
        self.ptr
    }
}

// SAFETY: as `Buffer`
unsafe impl MemoryCreator for Buffers {
    fn new_memory(
        &self,
        ty: MemoryType,
        minimum: usize,
        maximum: Option<usize>,
        reserved: Option<usize>,
        guard: usize,
    ) -> std::result::Result<Box<dyn LinearMemory>, String> {
        assert_eq!((reserved, guard), (None, 0)); // (no guard pages)
        self.log.lock().unwrap().push(format!("new {} {minimum} {maximum:?}", ty.minimum()));
        if self.fail {
            return Err("no memory for you".into());
        }
        // SAFETY: of a nonzero size
        let ptr = unsafe { std::alloc::alloc_zeroed(Buffer::layout(self.capacity)) };
        Ok(Box::new(Buffer { ptr, capacity: self.capacity, size: minimum, log: self.log.clone() }))
    }
}

#[test]
fn memory_creator() {
    for c in configs() {
        let imported = module(
            &c,
            r#"(module
                (import "aqr" "mem" (memory 1))
                (func (export "grow") (param i32) (result i32) (memory.grow (local.get 0)))
                (func (export "peek") (param i32) (result i32) (i32.load8_u (local.get 0))))"#,
        );
        let defined = module(&c, r#"(module (memory (export "own") 2 8) (data (i32.const 5) "\07"))"#);
        let log = Arc::new(Mutex::new(Vec::new()));
        let mut linker = Linker::<()>::new();
        linker.memory("aqr", "mem", MemoryType::new(1, Some(5))).unwrap();
        let mut store = Store::new(());
        store.set_memory_creator(Arc::new(Buffers { log: log.clone(), capacity: 4 << 16, fail: false }));
        linker.instantiate_hosts(&mut store).unwrap();
        let mem = store.get_memory("aqr", "mem").unwrap();
        let base = mem.data(&store).as_ptr();
        mem.data_mut(&mut store)[100] = 42;
        let i = Instance::new(&mut store, &imported).unwrap();
        let grow = i.get_typed_func::<i32, i32>(&store, "grow").unwrap();
        let peek = i.get_typed_func::<i32, i32>(&store, "peek").unwrap();
        assert_eq!(peek.call(&mut store, 100).unwrap(), 42);
        assert_eq!(grow.call(&mut store, 1).unwrap(), 1);
        assert_eq!(peek.call(&mut store, (2 << 16) - 1).unwrap(), 0);
        assert_eq!((mem.data(&store).as_ptr(), mem.data_size(&store)), (base, 2 << 16));
        assert_eq!(grow.call(&mut store, 3).unwrap(), -1); // beyond the capacity
        let d = Instance::new(&mut store, &defined).unwrap();
        assert_eq!(d.get_memory(&store, "own").unwrap().data(&store)[5], 7);
        // panics in the memory pass through
        let p = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| grow.call(&mut store, 1)));
        assert_eq!(p.unwrap_err().downcast_ref::<&str>(), Some(&"grown to 3 pages"));
        drop(store);
        let want = [
            "new 1 65536 Some(327680)",
            "grow 65536 131072",
            "new 2 131072 Some(524288)",
            "free 131072",
            "free 131072",
        ];
        assert_eq!(*log.lock().unwrap(), want);
        log.lock().unwrap().clear();

        // errors of the creator fail what makes the memory
        let mut store = Store::new(());
        store.set_memory_creator(Arc::new(Buffers { log: log.clone(), capacity: 4 << 16, fail: true }));
        let e = linker.instantiate_hosts(&mut store).unwrap_err();
        assert!(e.to_string().contains("no memory for you"), "{e}");
        assert_eq!(e.wah_code(), Some(Code::OutOfMemory));
        assert!(Instance::new(&mut store, &defined).is_err());
    }
}

/// A memory over the limit is refused before anything makes it (the creator, or WAH: no allocation of its size).
#[cfg(feature = "multi-memory")]
#[test]
fn limiter_before_creation() {
    for c in configs() {
        let log = Arc::new(Mutex::new(Vec::new()));
        let mut linker = Linker::<Budget>::new();
        linker.memory("aqr", "big", MemoryType::new(2, None)).unwrap();
        let mut store = Store::new(Budget { total: 0, limit: PAGE_SIZE, deny: true });
        store.limiter(|b| b);
        store.set_memory_creator(Arc::new(Buffers { log: log.clone(), capacity: 4 << 16, fail: false }));
        let e = linker.instantiate_hosts(&mut store).unwrap_err();
        assert_eq!(e.wah_code(), Some(Code::OutOfMemory));
        let defined = module(&c, r#"(module (memory 2))"#);
        assert_eq!(Instance::new(&mut store, &defined).unwrap_err().wah_code(), Some(Code::OutOfMemory));
        assert_eq!(store.data().total, 0);
        drop(store);
        assert_eq!(*log.lock().unwrap(), Vec::<String>::new());
    }
}

#[test]
fn shared_host_modules() {
    for c in configs() {
        let m = module(
            &c,
            r#"(module
                (import "env" "get" (func $get (result i32)))
                (import "env" "mem" (memory 1))
                (func (export "run") (result i32) (i32.add (call $get) (i32.load8_u (i32.const 0)))))"#,
        );
        let mut linker = Linker::<i32>::new();
        linker.func_wrap("env", "get", |c: Caller<'_, i32>| *c.data()).unwrap();
        let run = |linker: &Linker<i32>, data: i32, byte: u8| {
            let mut store = Store::new(data);
            linker.instantiate_hosts(&mut store).unwrap();
            store.get_memory("env", "mem").unwrap().data_mut(&mut store)[0] = byte;
            let i = Instance::new(&mut store, &m).unwrap();
            i.get_typed_func::<(), i32>(&store, "run").unwrap().call(&mut store, ()).unwrap()
        };
        // each store sees its own data through the same host module, also from other threads
        let mut l1 = linker.clone();
        l1.memory("env", "mem", MemoryType::new(1, None)).unwrap();
        assert_eq!(run(&l1, 10, 1), 11);
        assert_eq!(run(&l1, 20, 2), 22);
        std::thread::scope(|s| {
            let l1 = &l1;
            let t: Vec<_> = (0..4).map(|k| s.spawn(move || run(l1, 100 * k, 3))).collect();
            let r: Vec<i32> = t.into_iter().map(|t| t.join().unwrap()).collect();
            assert_eq!(r, [3, 103, 203, 303]);
        });
        let mut l2 = linker.clone();
        l2.memory("env", "mem", MemoryType::new(2, None)).unwrap();
        assert_eq!(run(&l2, 30, 4), 34);
    }
}
