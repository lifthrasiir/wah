use std::future::Future;
use std::marker::PhantomData;
use std::pin::Pin;
use std::sync::Arc;
use std::task::{Context, Poll, Wake};

use wah_sys as sys;

use crate::error::{Code, Error, Result};
use crate::func::{WasmParams, WasmResults};
use crate::module::Module;
use crate::store::{self, AsContext, AsContextMut, Store, StoreCtx};
use crate::types::{ExternType, FuncType, MemoryType, Val};

/// An instance of a module in a store.
#[derive(Debug, Clone)]
pub struct Instance {
    pub(crate) store: u64,
    pub(crate) ctx: u32,
    module: Module,
}

impl Instance {
    /// Instantiates `module` in `store`, its imports resolved by their module names to the host modules
    /// (`Linker::instantiate_hosts`) and instances (`Store::register`) of the store; runs its start function.
    ///
    /// A failed instantiation can leave the other contexts of the store unusable, as WAH can't undo it.
    pub fn new<T: 'static>(store: &mut Store<T>, module: &Module) -> Result<Instance> {
        let s = store.raw();
        // SAFETY: `s` is the store's, which nothing borrows
        let ctx = unsafe { store::new_instance(s, module)? };
        Ok(Instance { store: unsafe { (*s).id() }, ctx, module: module.clone() })
    }

    pub fn module(&self) -> &Module {
        &self.module
    }

    pub fn get_func(&self, store: impl AsContext, name: &str) -> Option<Func> {
        store.as_context().check(self.store);
        let e = self.module.get_export(name)?;
        let ExternType::Func(ty) = &e.ty else { return None };
        Some(Func { store: self.store, ctx: self.ctx, index: e.index, ty: ty.clone() })
    }

    /// The function export `name`, of the parameters `P` and results `R`.
    pub fn get_typed_func<P: WasmParams, R: WasmResults>(
        &self,
        store: impl AsContext,
        name: &str,
    ) -> Result<TypedFunc<P, R>> {
        let f = self
            .get_func(store, name)
            .ok_or_else(|| Error::with_context(Code::NotFound, format!("function export {name}")))?;
        f.typed()
    }

    pub fn get_memory(&self, store: impl AsContext, name: &str) -> Option<Memory> {
        store.as_context().check(self.store);
        let e = self.module.get_export(name)?;
        matches!(e.ty, ExternType::Memory(_)).then_some(Memory { store: self.store, ctx: self.ctx, index: e.index })
    }
}

/// A linear memory of a store.
///
/// Its data stays where it is but for `memory.grow`, which needs the store borrowed mutably: so the slices given
/// are valid for as long as they borrow the store.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Memory {
    pub(crate) store: u64,
    /// The context and the index of the memory in its module.
    pub(crate) ctx: u32,
    pub(crate) index: u32,
}

impl Memory {
    pub fn data<'a, S: AsContext + 'a>(&self, store: &'a S) -> &'a [u8] {
        let (p, n) = store.as_context().memory(self);
        // SAFETY: WAH's memory, which nothing writes while the store is borrowed
        unsafe { std::slice::from_raw_parts(p, n) }
    }

    pub fn data_mut<'a, S: AsContextMut + 'a>(&self, store: &'a mut S) -> &'a mut [u8] {
        let (p, n) = store.as_context().memory(self);
        // SAFETY: as `data`, borrowed mutably
        unsafe { std::slice::from_raw_parts_mut(p, n) }
    }

    /// The data of the memory and of the store, at once.
    pub fn data_and_store_mut<'a, S: AsContextMut + 'a>(&self, store: &'a mut S) -> (&'a mut [u8], &'a mut S::Data) {
        let s = store.as_context_mut();
        let (p, n) = s.memory(self);
        // SAFETY: as `data_mut`: the memory is WAH's allocation, apart from the store's data
        (unsafe { std::slice::from_raw_parts_mut(p, n) }, s.data_mut())
    }

    /// The size in bytes.
    pub fn data_size(&self, store: impl AsContext) -> usize {
        store.as_context().memory(self).1
    }

    /// The size in pages.
    pub fn size(&self, store: impl AsContext) -> u64 {
        self.data_size(store) as u64 / crate::PAGE_SIZE
    }

    pub fn ty(&self, store: impl AsContext) -> MemoryType {
        let s = store.as_context();
        s.check(self.store);
        // SAFETY: the module is the store's
        unsafe {
            let mut d = std::mem::zeroed::<sys::wah_memory_desc_t>();
            let r = sys::wah_module_memory(s.module_ptr(self.ctx), self.index, &mut d);
            assert_eq!(r, sys::WAH_OK, "a memory of the module");
            MemoryType::from_raw(&d)
        }
    }
}

/// A function of an instance.
#[derive(Debug, Clone)]
pub struct Func {
    store: u64,
    ctx: u32,
    index: u32,
    ty: FuncType,
}

impl Func {
    pub fn ty(&self) -> &FuncType {
        &self.ty
    }

    /// The function as of the parameters `P` and results `R`, if it is of them.
    pub fn typed<P: WasmParams, R: WasmResults>(&self) -> Result<TypedFunc<P, R>> {
        if !P::types().eq(self.ty.params().iter().copied()) || !R::types().eq(self.ty.results().iter().copied()) {
            return Err(Error::misuse(format!("a function of {:?} called as of other types", self.ty)));
        }
        Ok(TypedFunc { func: self.clone(), _marker: PhantomData })
    }

    /// Calls the function, waiting in this thread for host functions that wait.
    pub fn call<T: 'static>(&self, store: &mut Store<T>, params: &[Val], results: &mut [Val]) -> Result<()> {
        block_on(self.call_async(store, params, results))
    }

    /// Calls the function: host functions that wait (`Poll::Pending`) suspend it, and they are called again as the
    /// future is polled again. Dropping the future cancels the call.
    pub fn call_async<'a, T: 'static>(
        &'a self,
        store: &'a mut Store<T>,
        params: &[Val],
        results: &'a mut [Val],
    ) -> impl Future<Output = Result<()>> + 'a {
        let call = self.check(store, params, results.len()).map(|()| Call::new(store, self, params));
        async move {
            let r = call?.await?;
            for ((v, &ty), slot) in r.iter().zip(self.ty.results()).zip(results.iter_mut()) {
                // SAFETY: WAH's results, of the function's types
                *slot = unsafe { Val::from_raw(&v.0, ty) };
            }
            Ok(())
        }
    }

    fn check<T: 'static>(&self, store: &Store<T>, params: &[Val], results: usize) -> Result<()> {
        store.as_context().check(self.store);
        let ty = &self.ty;
        if params.len() != ty.params().len() || params.iter().zip(ty.params()).any(|(v, &t)| v.ty() != t) {
            return Err(Error::misuse(format!("parameters {params:?} for a function of {ty:?}")));
        }
        if results != ty.results().len() {
            return Err(Error::misuse(format!("{results} results for a function of {ty:?}")));
        }
        if ty.results().iter().any(|t| !t.is_num()) {
            return Err(Error::msg("results of reference types are not supported"));
        }
        Ok(())
    }
}

/// A function of the parameters `P` and results `R`.
#[derive(Debug)]
pub struct TypedFunc<P, R> {
    func: Func,
    _marker: PhantomData<fn(P) -> R>,
}

impl<P, R> Clone for TypedFunc<P, R> {
    fn clone(&self) -> Self {
        TypedFunc { func: self.func.clone(), _marker: PhantomData }
    }
}

impl<P: WasmParams, R: WasmResults> TypedFunc<P, R> {
    pub fn func(&self) -> &Func {
        &self.func
    }

    /// As [`Func::call`].
    pub fn call<T: 'static>(&self, store: &mut Store<T>, params: P) -> Result<R> {
        block_on(self.call_async(store, params))
    }

    /// As [`Func::call_async`].
    pub fn call_async<'a, T: 'static>(
        &'a self,
        store: &'a mut Store<T>,
        params: P,
    ) -> impl Future<Output = Result<R>> + 'a {
        store.as_context().check(self.func.store);
        let mut raw = Vec::with_capacity(self.func.ty.params().len());
        params.into_raw(&mut raw);
        let call = Call::raw(store, &self.func, raw);
        async move {
            let r = call.await?;
            // SAFETY: WAH's results, of the function's types (as checked by `Func::typed`)
            Ok(unsafe { R::from_raw(&mut r.iter().map(|v| &v.0)) })
        }
    }
}

/// A value of WAH, moved along with the call.
#[repr(transparent)]
#[doc(hidden)]
pub struct Raw(pub(crate) sys::wah_value_t);

// SAFETY: values of numeric and vector types alone
unsafe impl Send for Raw {}

/// A call in progress.
struct Call<'a, T: 'static> {
    store: &'a mut Store<T>,
    ctx: u32,
    index: u32,
    results: usize,
    params: Vec<Raw>,
    state: State,
}

#[derive(PartialEq, Eq)]
enum State {
    New,
    Started,
    Done,
}

impl<'a, T: 'static> Call<'a, T> {
    fn new(store: &'a mut Store<T>, f: &Func, params: &[Val]) -> Call<'a, T> {
        Call::raw(store, f, params.iter().map(|v| Raw(v.to_raw())).collect())
    }

    fn raw(store: &'a mut Store<T>, f: &Func, params: Vec<Raw>) -> Call<'a, T> {
        Call { store, ctx: f.ctx, index: f.index, results: f.ty.results().len(), params, state: State::New }
    }
}

impl<T: 'static> Future for Call<'_, T> {
    type Output = Result<Vec<Raw>>;

    fn poll(self: Pin<&mut Self>, cx: &mut Context<'_>) -> Poll<Result<Vec<Raw>>> {
        let this = self.get_mut();
        assert!(this.state != State::Done, "a call polled after it completed");
        let s: *mut StoreCtx<T> = this.store.raw();
        // SAFETY: the store is borrowed by the call, and nothing of it otherwise while WAH runs
        unsafe {
            let ctx = (*s).ctx_ptr(this.ctx);
            let metered = (*s).wasm_module(this.ctx).is_some_and(Module::consumes_fuel);
            let fuel = (*s).fuel().filter(|_| metered);
            if this.state == State::New {
                if let Some(f) = fuel {
                    crate::error::check(sys::wah_set_fuel(ctx, f.min(i64::MAX as u64) as i64))?;
                }
                let p = &this.params;
                let r = sys::wah_start(ctx, this.index.into(), p.as_ptr().cast(), p.len() as u32);
                if r != sys::WAH_OK {
                    this.state = State::Done;
                    return Poll::Ready(Err(Error::with_context(Code::from_raw(r), "call not started")));
                }
                this.state = State::Started;
            }
            let (r, pending) = store::run(s, cx, || sys::wah_resume(ctx));
            if fuel.is_some() {
                (*s).set_fuel(sys::wah_get_fuel(ctx).max(0) as u64);
            }
            if r == sys::WAH_STATUS_YIELDED && pending {
                return Poll::Pending;
            }
            this.state = State::Done;
            if r != sys::WAH_OK {
                sys::wah_cancel(ctx);
                return Poll::Ready(Err(store::failure(s, r)));
            }
            let mut out: Vec<Raw> = (0..this.results).map(|_| Raw(sys::wah_value_t::ZERO)).collect();
            let mut n = 0;
            let r = sys::wah_finish(ctx, out.as_mut_ptr().cast(), out.len() as u32, &mut n);
            Poll::Ready(crate::error::check(r).map(|()| out))
        }
    }
}

impl<T: 'static> Drop for Call<'_, T> {
    fn drop(&mut self) {
        if self.state == State::Started {
            let s = self.store.raw();
            // SAFETY: the activation is suspended, and the store borrowed by the call
            unsafe { sys::wah_cancel((*s).ctx_ptr(self.ctx)) };
        }
    }
}

struct ThreadWaker(std::thread::Thread);

impl Wake for ThreadWaker {
    fn wake(self: Arc<Self>) {
        self.0.unpark();
    }

    fn wake_by_ref(self: &Arc<Self>) {
        self.0.unpark();
    }
}

/// Runs `f` to completion in this thread.
fn block_on<F: Future>(f: F) -> F::Output {
    let mut f = std::pin::pin!(f);
    let waker = Arc::new(ThreadWaker(std::thread::current())).into();
    let mut cx = Context::from_waker(&waker);
    loop {
        if let Poll::Ready(r) = f.as_mut().poll(&mut cx) {
            return r;
        }
        std::thread::park();
    }
}

/// Calls are futures that can move between threads, as their stores can.
#[allow(dead_code)]
fn assert_send<T: Send + 'static>(f: &Func, t: &TypedFunc<i32, i32>, s: &mut Store<T>, r: &mut [Val]) {
    fn send<X: Send>(_: X) {}
    send(f.call_async(s, &[], r));
    send(t.call_async(s, 0));
}
