use std::cell::UnsafeCell;
use std::ffi::{CString, c_void};
use std::marker::PhantomData;
use std::panic::{AssertUnwindSafe, catch_unwind};
use std::ptr::{NonNull, null_mut};
use std::sync::Arc;
use std::sync::atomic::{AtomicU64, Ordering};
use std::task::{Context, Poll, Waker};
use std::time::Duration;

use wah_sys as sys;

use crate::error::{Code, Error, Result, Thrown, check};
use crate::instance::{Instance, Memory};
use crate::memory::{CreatorData, MemoryCreator};
use crate::module::Module;
use crate::types::{ExternType, PAGE_SIZE};

/// Limits the growth of memories, as [`Store::limiter`] gives it.
pub trait ResourceLimiter {
    /// Called before a memory grows from `current` to `desired` bytes (`maximum`: the most its type allows), and
    /// before a memory is created (`current` = 0) as a host module defines it or a module is instantiated.
    /// `Ok(false)` makes `memory.grow` fail (returning -1) and the creation fail; an error traps with it.
    fn memory_growing(&mut self, current: u64, desired: u64, maximum: Option<u64>) -> Result<bool>;
}

type Limiter<T> = Box<dyn Send + FnMut(&mut T) -> &mut dyn ResourceLimiter>;

/// The function a host function runs, which may wait (`Poll::Pending`) for its `Context`'s waker: the activation is
/// suspended then, and the function called again with the same parameters once resumed.
pub(crate) type HostFn<T> = dyn Fn(Caller<'_, T>, &mut Context<'_>) -> Poll<Result<()>> + Send + Sync;

struct HookData<T: 'static> {
    store: *mut StoreCtx<T>,
    ctx: u32,
}

/// An exec context of a store, freed with it, and what it is of.
struct Ctx {
    raw: NonNull<UnsafeCell<sys::wah_exec_context_t>>,
    module: CtxModule,
    /// Whether it is an instance that has been instantiated (where the deadline goes).
    instance: bool,
}

enum CtxModule {
    Wasm(Module),
    /// A module of host functions and memories, built by a `Linker`.
    Host(Module),
}

impl CtxModule {
    fn module(&self) -> &Module {
        match self {
            CtxModule::Wasm(m) | CtxModule::Host(m) => m,
        }
    }
}

fn boxed<T>(v: T) -> NonNull<UnsafeCell<T>> {
    NonNull::from(Box::leak(Box::new(UnsafeCell::new(v))))
}

/// What a [`Store`] holds: its data and its instances. Host functions and limiters see it through [`Caller`] and
/// [`AsContext`].
pub struct StoreCtx<T: 'static> {
    data: T,
    id: u64,
    limiter: Option<Limiter<T>>,
    max_stack_bytes: u64,
    deadline_us: u64,
    fuel: Option<u64>,
    contexts: Vec<Ctx>,
    /// The contexts linkable by name.
    names: Vec<(String, u32)>,
    hooks: Vec<NonNull<HookData<T>>>,
    /// The memory creator of contexts made from now on, of the last of `creators` (which outlive the contexts).
    creator: Option<sys::wah_memory_creator_t>,
    creators: Vec<NonNull<CreatorData>>,
    /// While WAH runs: the context of the poll, whether a host function waits, and what to propagate.
    cx: *mut Context<'static>,
    host_pending: bool,
    thrown: Option<Thrown>,
}

/// Every instance, host module and memory of an application of modules, with the data `T` its host functions
/// work on. Contexts of a store are a link domain of WAH: freed together, and never used by two threads at once.
pub struct Store<T: 'static> {
    ptr: NonNull<StoreCtx<T>>,
    _marker: PhantomData<StoreCtx<T>>,
}

// SAFETY: WAH keeps no thread-bound state in exec contexts between calls, and a store is used by one thread at a time
// (`&mut`); host functions and limiters are `Send` (and host functions `Sync`, as linkers share them).
unsafe impl<T: Send> Send for Store<T> {}
unsafe impl<T: Sync> Sync for Store<T> {}

static NEXT_ID: AtomicU64 = AtomicU64::new(1);

impl<T: 'static> Store<T> {
    pub fn new(data: T) -> Store<T> {
        let ptr = NonNull::from(Box::leak(Box::new(StoreCtx {
            data,
            id: NEXT_ID.fetch_add(1, Ordering::Relaxed),
            limiter: None,
            max_stack_bytes: 0,
            deadline_us: 0,
            fuel: None,
            contexts: Vec::new(),
            names: Vec::new(),
            hooks: Vec::new(),
            creator: None,
            creators: Vec::new(),
            cx: null_mut(),
            host_pending: false,
            thrown: None,
        })));
        Store { ptr, _marker: PhantomData }
    }

    pub(crate) fn raw(&mut self) -> *mut StoreCtx<T> {
        self.ptr.as_ptr()
    }

    pub fn data(&self) -> &T {
        &self.as_context().data
    }

    pub fn data_mut(&mut self) -> &mut T {
        &mut self.as_context_mut().data
    }

    /// Sets what limits the memories of the store: `f` gives the limiter out of the data.
    pub fn limiter(&mut self, f: impl Send + 'static + FnMut(&mut T) -> &mut dyn ResourceLimiter) {
        self.as_context_mut().limiter = Some(Box::new(f));
    }

    /// Makes the memories of instances and host modules made from now on by `creator`, in place of WAH.
    pub fn set_memory_creator(&mut self, creator: Arc<dyn MemoryCreator>) {
        let s = self.ptr.as_ptr();
        // SAFETY: the store is ours; `thrown` stays where it is as long as the store
        unsafe {
            let data = NonNull::from(Box::leak(Box::new(CreatorData { creator, thrown: &raw mut (*s).thrown })));
            (*s).creators.push(data);
            (*s).creator = Some(CreatorData::raw(data.as_ptr()));
        }
    }

    /// Bounds the stack (values and call frames) of each instance and host module made from now on, in bytes.
    /// WAH's default (1 to 2 MiB) is enough for about a thousand nested calls.
    pub fn set_max_stack_bytes(&mut self, bytes: u64) {
        self.as_context_mut().max_stack_bytes = bytes;
    }

    /// Gives the store `fuel` to run modules that consume fuel ([`Config::consume_fuel`](crate::Config)): every
    /// instruction takes one, and calls fail with [`Code::OutOfFuel`] once it runs out. Unlimited by default.
    pub fn set_fuel(&mut self, fuel: u64) {
        self.as_context_mut().fuel = Some(fuel);
    }

    /// The fuel left, `None` if unlimited.
    pub fn get_fuel(&self) -> Option<u64> {
        self.as_context().fuel
    }

    /// Sets the time the instances of the store may run (`None`: no deadline), counted while they run, after which
    /// calls fail with [`Code::Interrupted`]. It is checked cooperatively, at loops and calls.
    pub fn set_deadline(&mut self, deadline: Option<Duration>) -> Result<()> {
        let us = match deadline {
            Some(d) => u64::try_from(d.as_micros()).unwrap_or(u64::MAX - 1).max(1),
            None => u64::MAX,
        };
        let s = self.as_context_mut();
        s.deadline_us = us;
        for c in s.contexts.iter().filter(|c| c.instance) {
            let limits = sys::wah_limits_t { deadline_us: us, ..Default::default() };
            // SAFETY: the context is the store's and not running
            check(unsafe { sys::wah_set_limits(c.raw.as_ref().get(), &limits) })?;
        }
        Ok(())
    }

    /// A memory that a host module of the store defines (`Linker::instantiate_hosts`).
    pub fn get_memory(&self, module: &str, name: &str) -> Option<Memory> {
        let s = self.as_context();
        let &(_, ctx) = s.names.iter().find(|(n, _)| n == module)?;
        let CtxModule::Host(m) = &s.contexts[ctx as usize].module else { return None };
        // SAFETY: the module is the store's
        let index = unsafe { export_memory(m.raw(), name)? };
        Some(Memory { store: s.id, ctx, index })
    }

    /// Makes `instance` importable by other modules of the store under `name`.
    pub fn register(&mut self, name: &str, instance: &Instance) -> Result<()> {
        let s = self.as_context_mut();
        s.check(instance.store);
        if s.names.iter().any(|(n, _)| n == name) {
            return Err(Error::misuse(format!("{name} defined twice in the store")));
        }
        s.names.push((name.to_owned(), instance.ctx));
        Ok(())
    }
}

/// The memory export `name` of `m`.
///
/// # Safety
/// `m` should be a valid module.
unsafe fn export_memory(m: *const sys::wah_module_t, name: &str) -> Option<u32> {
    unsafe {
        let mut d = std::mem::zeroed::<sys::wah_export_desc_t>();
        let r = sys::wah_export_by_name_len(m, name.as_ptr().cast(), name.len(), &mut d);
        (r == sys::WAH_OK && d.kind == sys::WAH_KIND_MEMORY).then_some(d.index)
    }
}

impl<T: 'static> Drop for Store<T> {
    fn drop(&mut self) {
        // SAFETY: the store is ours, and nothing of it runs
        unsafe { drop(Box::from_raw(self.ptr.as_ptr())) }
    }
}

impl<T: 'static> Drop for StoreCtx<T> {
    fn drop(&mut self) {
        // SAFETY: the contexts are freed first (in any order, as a link domain), then the modules they use (as
        // `Module`s); the data of hooks and creators last, which they point to
        unsafe {
            for c in &self.contexts {
                sys::wah_free_exec_context(c.raw.as_ref().get());
            }
            for c in self.contexts.drain(..) {
                drop(Box::from_raw(c.raw.as_ptr()));
            }
            for h in self.hooks.drain(..) {
                drop(Box::from_raw(h.as_ptr()));
            }
            for c in self.creators.drain(..) {
                drop(Box::from_raw(c.as_ptr()));
            }
        }
    }
}

impl<T: 'static> StoreCtx<T> {
    pub fn data(&self) -> &T {
        &self.data
    }

    pub fn data_mut(&mut self) -> &mut T {
        &mut self.data
    }

    pub(crate) fn id(&self) -> u64 {
        self.id
    }

    /// Panics where a handle is of another store.
    pub(crate) fn check(&self, store: u64) {
        assert_eq!(store, self.id, "a handle used with a store other than its own");
    }

    pub(crate) fn ctx_ptr(&self, ctx: u32) -> *mut sys::wah_exec_context_t {
        // SAFETY: the context lives as long as the store
        unsafe { self.contexts[ctx as usize].raw.as_ref().get() }
    }

    pub(crate) fn module_ptr(&self, ctx: u32) -> *const sys::wah_module_t {
        self.contexts[ctx as usize].module.module().raw()
    }

    pub(crate) fn wasm_module(&self, ctx: u32) -> Option<&Module> {
        match &self.contexts[ctx as usize].module {
            CtxModule::Wasm(m) => Some(m),
            CtxModule::Host(_) => None,
        }
    }

    pub(crate) fn name_of(&self, name: &str) -> Option<u32> {
        self.names.iter().find(|(n, _)| n == name).map(|&(_, c)| c)
    }

    pub(crate) fn has_name(&self, name: &str) -> bool {
        self.name_of(name).is_some()
    }

    pub(crate) fn fuel(&self) -> Option<u64> {
        self.fuel
    }

    pub(crate) fn set_fuel(&mut self, fuel: u64) {
        self.fuel = Some(fuel);
    }

    /// The maximum of memory `index` of the module of `ctx`, in bytes.
    fn memory_max(&self, ctx: u32, index: u32) -> Option<u64> {
        // SAFETY: the module is the store's
        unsafe {
            let mut d = std::mem::zeroed::<sys::wah_memory_desc_t>();
            if sys::wah_module_memory(self.module_ptr(ctx), index, &mut d) != sys::WAH_OK || d.max_pages == u64::MAX {
                return None;
            }
            d.max_pages.checked_mul(PAGE_SIZE)
        }
    }

    /// Asks the limiter for memory `index` of `ctx` of `minimum` pages to be created.
    fn account_initial(&mut self, ctx: u32, index: u32, minimum: u64) -> Result<()> {
        let max = self.memory_max(ctx, index);
        let Some(limiter) = self.limiter.as_mut() else { return Ok(()) };
        let bytes = minimum.checked_mul(PAGE_SIZE).ok_or_else(|| Error::code(Code::TooLarge))?;
        match limiter(&mut self.data).memory_growing(0, bytes, max)? {
            true => Ok(()),
            false => Err(Error::with_context(Code::OutOfMemory, "memory not created (limiter)")),
        }
    }

    /// The pointer and size of memory `m`.
    pub(crate) fn memory(&self, m: &Memory) -> (*mut u8, usize) {
        self.check(m.store);
        let (mut data, mut size) = (null_mut(), 0u64);
        // SAFETY: the context is the store's and instantiated
        let r = unsafe { sys::wah_memory_data(self.ctx_ptr(m.ctx), m.index, &mut data, &mut size) };
        assert_eq!(r, sys::WAH_OK, "a memory of the store");
        if data.is_null() {
            return (NonNull::dangling().as_ptr(), 0);
        }
        (data, usize::try_from(size).expect("a memory within the address space"))
    }
}

/// A new exec context of `module` in the store `s`, kept by it (freed with it), whose host functions find the store
/// by its userdata.
///
/// # Safety
/// `s` should be the store's own pointer, with nothing of the store borrowed.
unsafe fn new_ctx<T: 'static>(s: *mut StoreCtx<T>, module: CtxModule, instance: bool) -> Result<u32> {
    unsafe {
        let st = &mut *s;
        let raw = boxed(sys::wah_exec_context_t::ZERO);
        let ctx = st.contexts.len() as u32;
        let m = module.module().clone();
        st.contexts.push(Ctx { raw, module, instance });
        let limits = sys::wah_limits_t {
            max_stack_bytes: st.max_stack_bytes,
            deadline_us: if instance { st.deadline_us } else { 0 },
            ..Default::default()
        };
        let opts = sys::wah_exec_options_t {
            limits,
            alloc: std::ptr::null(),
            memory_creator: st.creator.as_ref().map_or(std::ptr::null(), |c| c),
            userdata: s.cast(),
        };
        st.thrown = None;
        // (the context is zeroed and stays where it is until the store frees it, after which the module)
        match m.new_exec_context(raw.as_ref().get(), &opts) {
            Ok(()) => Ok(ctx),
            Err(e) => Err(match (*s).thrown.take() {
                Some(Thrown::Panic(p)) => std::panic::resume_unwind(p),
                Some(Thrown::Error(e)) => e,
                None => e,
            }),
        }
    }
}

/// Instantiates a host module `module` (built by a linker of `T`, for stores of `T`) in the store `s` as `name`.
///
/// # Safety
/// `s` should be the store's own pointer, with nothing of the store borrowed.
pub(crate) unsafe fn new_host<T: 'static>(s: *mut StoreCtx<T>, name: &str, module: &Module) -> Result<()> {
    unsafe {
        if (*s).has_name(name) {
            return Err(Error::misuse(format!("{name} defined twice in the store")));
        }
        let ctx = new_ctx(s, CtxModule::Host(module.clone()), false)?;
        let st = &mut *s;
        let mp = module.raw();
        for i in 0..sys::wah_module_memory_count(mp) {
            let mut d = std::mem::zeroed::<sys::wah_memory_desc_t>();
            check(sys::wah_module_memory(mp, i, &mut d))?;
            st.account_initial(ctx, i, d.min_pages)?;
        }
        instantiate(s, ctx)?;
        (*s).names.push((name.to_owned(), ctx));
        Ok(())
    }
}

/// Makes an instance of `module` in the store `s`, linked to its contexts by the names it imports.
///
/// # Safety
/// `s` should be the store's own pointer, with nothing of the store borrowed.
pub(crate) unsafe fn new_instance<T: 'static>(s: *mut StoreCtx<T>, module: &Module) -> Result<u32> {
    unsafe {
        let st = &mut *s;
        let mut names: Vec<&str> = Vec::new();
        let mut imported_memories = 0;
        for i in module.imports() {
            if !names.contains(&&*i.module) {
                names.push(&i.module);
            }
            imported_memories += matches!(i.ty, ExternType::Memory(_)) as u32;
        }
        let mut linked = Vec::with_capacity(names.len());
        for &n in &names {
            let Some(c) = st.name_of(n) else {
                return Err(Error::with_context(Code::LinkFailed, format!("no module {n} in the store")));
            };
            linked.push((CString::new(n).map_err(|_| Error::misuse("a name with a null byte"))?, c));
        }
        let ctx = new_ctx(s, CtxModule::Wasm(module.clone()), true)?;
        let st = &mut *s;
        for (n, c) in &linked {
            // (both contexts are the store's, and live as long as it)
            let r = sys::wah_link_context(st.ctx_ptr(ctx), n.as_ptr(), st.ctx_ptr(*c));
            check(r).map_err(|e| {
                Error::with_context(e.wah_code().unwrap(), format!("{} not linked", n.to_string_lossy()))
            })?;
        }
        let mp = module.raw();
        for i in imported_memories..sys::wah_module_memory_count(mp) {
            let mut d = std::mem::zeroed::<sys::wah_memory_desc_t>();
            check(sys::wah_module_memory(mp, i, &mut d))?;
            st.account_initial(ctx, i, d.min_pages)?;
        }
        instantiate(s, ctx)?;
        Ok(ctx)
    }
}

/// Instantiates `ctx` of the store `s` (running start functions), and sets the hook of the limiter on it.
///
/// # Safety
/// `s` should be the store's own pointer, with nothing of the store borrowed.
unsafe fn instantiate<T: 'static>(s: *mut StoreCtx<T>, ctx: u32) -> Result<()> {
    unsafe {
        let p = (*s).ctx_ptr(ctx);
        let mut cx = Context::from_waker(Waker::noop());
        match run(s, &mut cx, || sys::wah_instantiate(p)) {
            (sys::WAH_OK, _) => {}
            (sys::WAH_STATUS_YIELDED, true) => {
                return Err(Error::with_context(Code::Interrupted, "a host function waited during instantiation"));
            }
            (r, _) => {
                return Err(match thrown(s) {
                    Some(Thrown::Panic(p)) => std::panic::resume_unwind(p),
                    Some(Thrown::Error(e)) => e,
                    None => Error::with_context(Code::from_raw(r), "not instantiated"),
                });
            }
        }
        let hook = NonNull::from(Box::leak(Box::new(HookData { store: s, ctx })));
        (*s).hooks.push(hook);
        check(sys::wah_set_memory_grow_hook(p, Some(grow_hook::<T>), hook.as_ptr().cast()))
    }
}

/// Runs `f` (a call into WAH that may run WebAssembly) for the store `s` in `cx`: what it returns and whether a host
/// function waits. What a host function or the limiter threw is returned as the error (a panic resumed).
///
/// # Safety
/// `s` should be the store's own pointer, with nothing of the store borrowed.
pub(crate) unsafe fn run<T: 'static>(
    s: *mut StoreCtx<T>,
    cx: &mut Context<'_>,
    f: impl FnOnce() -> sys::wah_error_t,
) -> (sys::wah_error_t, bool) {
    unsafe {
        (*s).cx = (cx as *mut Context<'_>).cast();
        (*s).host_pending = false;
        (*s).thrown = None;
        let r = f();
        (*s).cx = null_mut();
        (r, (*s).host_pending)
    }
}

/// What a host function or the limiter threw in the last `run`.
///
/// # Safety
/// As `run`.
pub(crate) unsafe fn thrown<T: 'static>(s: *mut StoreCtx<T>) -> Option<Thrown> {
    unsafe { (*s).thrown.take() }
}

/// The error of `r` that `run` returned, as thrown (a panic resumed).
///
/// # Safety
/// As `run`.
pub(crate) unsafe fn failure<T: 'static>(s: *mut StoreCtx<T>, r: sys::wah_error_t) -> Error {
    match unsafe { thrown(s) } {
        Some(Thrown::Panic(p)) => std::panic::resume_unwind(p),
        Some(Thrown::Error(e)) => e,
        None => Error::raw(r),
    }
}

unsafe extern "C" fn grow_hook<T: 'static>(
    userdata: *mut c_void,
    mem_idx: u32,
    old_bytes: u64,
    new_bytes: u64,
) -> sys::wah_error_t {
    // SAFETY: WAH calls this while running for the store, which nothing else borrows then
    let h = unsafe { &*userdata.cast::<HookData<T>>() };
    let s = unsafe { &mut *h.store };
    let max = s.memory_max(h.ctx, mem_idx);
    let Some(limiter) = s.limiter.as_mut() else { return sys::WAH_OK };
    let data = &mut s.data;
    let r = catch_unwind(AssertUnwindSafe(|| limiter(data).memory_growing(old_bytes, new_bytes, max)));
    match r {
        Ok(Ok(true)) => sys::WAH_OK,
        Ok(Ok(false)) => sys::WAH_ERROR_OUT_OF_MEMORY,
        Ok(Err(e)) => {
            s.thrown = Some(Thrown::Error(e));
            sys::WAH_ERROR_TRAP
        }
        Err(p) => {
            s.thrown = Some(Thrown::Panic(p));
            sys::WAH_ERROR_TRAP
        }
    }
}

/// The userdata of a host function: what it runs, dropped with its module (by `drop_host_fn`).
pub(crate) fn host_fn<T: 'static>(f: Arc<HostFn<T>>) -> *mut c_void {
    Box::into_raw(Box::new(f)).cast()
}

/// Drops the userdata of `host_fn`.
///
/// # Safety
/// Only once, by WAH.
pub(crate) unsafe extern "C" fn drop_host_fn<T: 'static>(userdata: *mut c_void) {
    drop(unsafe { Box::from_raw(userdata.cast::<Arc<HostFn<T>>>()) });
}

/// Calls the host function `userdata` (of `host_fn`) for the store of its context.
///
/// # Safety
/// Only for WAH, with host functions of modules that linkers of `T` built, instantiated only in stores of `T`.
pub(crate) unsafe extern "C" fn trampoline<T: 'static>(cctx: *mut sys::wah_call_context_t, userdata: *mut c_void) {
    // SAFETY: WAH calls this while running for the store (`run`), which nothing else borrows then
    let f = unsafe { &*userdata.cast::<Arc<HostFn<T>>>() };
    let s = unsafe { sys::wah_exec_userdata(cctx).cast::<StoreCtx<T>>() };
    let cx = if s.is_null() { null_mut() } else { unsafe { (*s).cx } };
    if cx.is_null() {
        // (not through `run`: not expected)
        unsafe { sys::wah_trap(cctx, sys::WAH_ERROR_MISUSE) };
        return;
    }
    let r = catch_unwind(AssertUnwindSafe(|| {
        let caller = Caller { store: unsafe { &mut *s }, cctx };
        f(caller, unsafe { &mut *cx })
    }));
    let s = unsafe { &mut *s };
    match r {
        Ok(Poll::Ready(Ok(()))) => {}
        Ok(Poll::Pending) => {
            s.host_pending = true;
            unsafe { sys::wah_suspend(cctx) };
        }
        Ok(Poll::Ready(Err(e))) => {
            s.thrown = Some(Thrown::Error(e));
            unsafe { sys::wah_trap(cctx, sys::WAH_ERROR_TRAP) };
        }
        Err(p) => {
            s.thrown = Some(Thrown::Panic(p));
            unsafe { sys::wah_trap(cctx, sys::WAH_ERROR_TRAP) };
        }
    }
}

/// What a host function is called with: the store, and the call.
pub struct Caller<'a, T: 'static> {
    pub(crate) store: &'a mut StoreCtx<T>,
    pub(crate) cctx: *mut sys::wah_call_context_t,
}

impl<T: 'static> Caller<'_, T> {
    pub fn data(&self) -> &T {
        &self.store.data
    }

    pub fn data_mut(&mut self) -> &mut T {
        &mut self.store.data
    }

    /// A memory that a host module of the store defines (see [`Store::get_memory`]).
    pub fn get_memory(&self, module: &str, name: &str) -> Option<Memory> {
        let &(_, ctx) = self.store.names.iter().find(|(n, _)| n == module)?;
        let CtxModule::Host(m) = &self.store.contexts[ctx as usize].module else { return None };
        // SAFETY: the module is the store's
        let index = unsafe { export_memory(m.raw(), name)? };
        Some(Memory { store: self.store.id, ctx, index })
    }
}

/// Gives access to a store.
pub trait AsContext {
    type Data: 'static;
    fn as_context(&self) -> &StoreCtx<Self::Data>;
}

/// Gives mutable access to a store.
pub trait AsContextMut: AsContext {
    fn as_context_mut(&mut self) -> &mut StoreCtx<Self::Data>;
}

impl<T: 'static> AsContext for Store<T> {
    type Data = T;
    fn as_context(&self) -> &StoreCtx<T> {
        // SAFETY: the store is ours
        unsafe { self.ptr.as_ref() }
    }
}

impl<T: 'static> AsContextMut for Store<T> {
    fn as_context_mut(&mut self) -> &mut StoreCtx<T> {
        // SAFETY: the store is ours
        unsafe { self.ptr.as_mut() }
    }
}

impl<T: 'static> AsContext for StoreCtx<T> {
    type Data = T;
    fn as_context(&self) -> &StoreCtx<T> {
        self
    }
}

impl<T: 'static> AsContextMut for StoreCtx<T> {
    fn as_context_mut(&mut self) -> &mut StoreCtx<T> {
        self
    }
}

impl<T: 'static> AsContext for Caller<'_, T> {
    type Data = T;
    fn as_context(&self) -> &StoreCtx<T> {
        self.store
    }
}

impl<T: 'static> AsContextMut for Caller<'_, T> {
    fn as_context_mut(&mut self) -> &mut StoreCtx<T> {
        self.store
    }
}

impl<S: AsContext + ?Sized> AsContext for &S {
    type Data = S::Data;
    fn as_context(&self) -> &StoreCtx<S::Data> {
        (**self).as_context()
    }
}

impl<S: AsContext + ?Sized> AsContext for &mut S {
    type Data = S::Data;
    fn as_context(&self) -> &StoreCtx<S::Data> {
        (**self).as_context()
    }
}

impl<S: AsContextMut + ?Sized> AsContextMut for &mut S {
    fn as_context_mut(&mut self) -> &mut StoreCtx<S::Data> {
        (**self).as_context_mut()
    }
}
