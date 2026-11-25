use std::ffi::{CString, c_void};
use std::ptr::NonNull;
use std::sync::Arc;
use std::task::{Context, Poll};

use wah_sys as sys;

use crate::error::{Error, Result, check};
use crate::func::{IntoFunc, IntoPollFunc};
use crate::instance::Instance;
use crate::module::Module;
use crate::store::{self, Caller, HostFn, HostFuncData, Store};
use crate::types::{FuncType, MemoryType, Val};

enum Item<T: 'static> {
    Memory(MemoryType),
    Global(Val, bool),
    Func(FuncType, Arc<HostFn<T>>),
}

impl<T: 'static> Clone for Item<T> {
    fn clone(&self) -> Self {
        match self {
            Item::Memory(m) => Item::Memory(*m),
            Item::Global(v, m) => Item::Global(*v, *m),
            Item::Func(t, f) => Item::Func(t.clone(), f.clone()),
        }
    }
}

/// Host modules to instantiate in stores: memories, globals and host functions under module names.
///
/// Each module name becomes a host module of its own in a store (an exec context of WAH that the instances link
/// to), whose memories are counted by the store's limiter. Cloning a linker is cheap for its functions.
pub struct Linker<T: 'static> {
    modules: Vec<(String, Items<T>)>,
}

/// The items of a host module, by name.
type Items<T> = Vec<(String, Item<T>)>;

impl<T: 'static> Clone for Linker<T> {
    fn clone(&self) -> Self {
        Linker { modules: self.modules.clone() }
    }
}

impl<T: 'static> Default for Linker<T> {
    fn default() -> Self {
        Linker::new()
    }
}

impl<T: 'static> Linker<T> {
    pub fn new() -> Linker<T> {
        Linker { modules: Vec::new() }
    }

    fn define(&mut self, module: &str, name: &str, item: Item<T>) -> Result<&mut Self> {
        let i = match self.modules.iter().position(|(m, _)| m == module) {
            Some(i) => i,
            None => {
                self.modules.push((module.to_owned(), Vec::new()));
                self.modules.len() - 1
            }
        };
        let items = &mut self.modules[i].1;
        if items.iter().any(|(n, _)| n == name) {
            return Err(Error::misuse(format!("{module}.{name} defined twice")));
        }
        items.push((name.to_owned(), item));
        Ok(self)
    }

    /// Defines a memory of type `ty`, made as the host module is instantiated in a store.
    pub fn memory(&mut self, module: &str, name: &str, ty: MemoryType) -> Result<&mut Self> {
        self.define(module, name, Item::Memory(ty))
    }

    /// Defines a global of the value `init`.
    pub fn global(&mut self, module: &str, name: &str, init: Val, mutable: bool) -> Result<&mut Self> {
        self.define(module, name, Item::Global(init, mutable))
    }

    /// Defines a host function of type `ty`, of dynamically typed values.
    pub fn func_new(
        &mut self,
        module: &str,
        name: &str,
        ty: FuncType,
        f: impl Fn(Caller<'_, T>, &[Val], &mut [Val]) -> Result<()> + Send + Sync + 'static,
    ) -> Result<&mut Self> {
        self.func_new_poll(module, name, ty, move |caller, _, params, results| Poll::Ready(f(caller, params, results)))
    }

    /// Defines a host function of type `ty`, of dynamically typed values, that may wait (see [`IntoPollFunc`]).
    pub fn func_new_poll(
        &mut self,
        module: &str,
        name: &str,
        ty: FuncType,
        f: impl Fn(Caller<'_, T>, &mut Context<'_>, &[Val], &mut [Val]) -> Poll<Result<()>> + Send + Sync + 'static,
    ) -> Result<&mut Self> {
        ty.spec()?;
        let t = ty.clone();
        let host = move |caller: Caller<'_, T>, cx: &mut Context<'_>| -> Poll<Result<()>> {
            let c = caller.cctx;
            // SAFETY: the parameters and results of the function's type, during the call
            let params: Vec<Val> = (t.params().iter().enumerate())
                .map(|(i, &ty)| unsafe {
                    match ty {
                        crate::ValType::I32 => Val::I32(sys::wah_param_i32(c, i)),
                        crate::ValType::I64 => Val::I64(sys::wah_param_i64(c, i)),
                        crate::ValType::F32 => Val::F32(sys::wah_param_f32(c, i).to_bits()),
                        crate::ValType::F64 => Val::F64(sys::wah_param_f64(c, i).to_bits()),
                        crate::ValType::V128 => Val::V128(u128::from_le_bytes(sys::wah_param_v128(c, i).u8)),
                        crate::ValType::Ref(_) => unreachable!("checked by FuncType::spec"),
                    }
                })
                .collect();
            let mut results: Vec<Val> = t.results().iter().map(|&ty| Val::default_for(ty).unwrap()).collect();
            match f(caller, cx, &params, &mut results) {
                Poll::Pending => Poll::Pending,
                Poll::Ready(Err(e)) => Poll::Ready(Err(e)),
                Poll::Ready(Ok(())) => {
                    for (i, (v, &ty)) in results.iter().zip(t.results()).enumerate() {
                        if v.ty() != ty {
                            return Poll::Ready(Err(Error::misuse(format!("a result {v:?} for a function of {t:?}"))));
                        }
                        unsafe {
                            match *v {
                                Val::I32(x) => sys::wah_result_i32(c, i, x),
                                Val::I64(x) => sys::wah_result_i64(c, i, x),
                                Val::F32(x) => sys::wah_result_f32(c, i, f32::from_bits(x)),
                                Val::F64(x) => sys::wah_result_f64(c, i, f64::from_bits(x)),
                                Val::V128(x) => sys::wah_result_v128(c, i, &sys::wah_v128_t { u8: x.to_le_bytes() }),
                            }
                        }
                    }
                    Poll::Ready(Ok(()))
                }
            }
        };
        self.define(module, name, Item::Func(ty, Arc::new(host)))
    }

    /// Defines a host function of a closure `Fn(Caller<'_, T>, A1, A2, ...) -> R` (see [`IntoFunc`]).
    pub fn func_wrap<Params, Results>(
        &mut self,
        module: &str,
        name: &str,
        f: impl IntoFunc<T, Params, Results>,
    ) -> Result<&mut Self> {
        let (ty, f) = f.into_func();
        self.define(module, name, Item::Func(ty, f))
    }

    /// Defines a host function of a closure that may wait,
    /// `Fn(Caller<'_, T>, &mut Context<'_>, A1, A2, ...) -> Poll<R>` (see [`IntoPollFunc`]).
    pub fn func_wrap_poll<Params, Results>(
        &mut self,
        module: &str,
        name: &str,
        f: impl IntoPollFunc<T, Params, Results>,
    ) -> Result<&mut Self> {
        let (ty, f) = f.into_func();
        self.define(module, name, Item::Func(ty, f))
    }

    /// Instantiates every host module of the linker in `store` (making their memories, which
    /// [`Store::get_memory`] then gives), so that instances of the store can import them.
    pub fn instantiate_hosts(&self, store: &mut Store<T>) -> Result<()> {
        for (name, _) in &self.modules {
            self.instantiate_host(store, name)?;
        }
        Ok(())
    }

    fn instantiate_host(&self, store: &mut Store<T>, module: &str) -> Result<()> {
        let Some((_, items)) = self.modules.iter().find(|(m, _)| m == module) else { return Ok(()) };
        let s = store.raw();
        let build = |m: *mut sys::wah_module_t,
                     funcs: &mut Vec<NonNull<HostFuncData<T>>>,
                     s: *mut store::StoreCtx<T>|
         -> Result<()> {
            for (name, item) in items {
                let n = CString::new(name.as_str()).map_err(|_| Error::misuse("a name with a null byte"))?;
                let n = n.as_ptr();
                // SAFETY: the module is being built, and the data of functions lives as long as the store
                let r = unsafe {
                    match item {
                        Item::Memory(ty) => {
                            let max = ty.maximum().unwrap_or(u64::MAX);
                            match ty.is_64() {
                                false => sys::wah_export_memory(m, n, ty.minimum(), max),
                                true => sys::wah_export_memory64(m, n, ty.minimum(), max),
                            }
                        }
                        &Item::Global(v, mutable) => match v {
                            Val::I32(x) => sys::wah_export_global_i32(m, n, mutable, x),
                            Val::I64(x) => sys::wah_export_global_i64(m, n, mutable, x),
                            Val::F32(x) => sys::wah_export_global_f32(m, n, mutable, f32::from_bits(x)),
                            Val::F64(x) => sys::wah_export_global_f64(m, n, mutable, f64::from_bits(x)),
                            Val::V128(x) => {
                                let v = sys::wah_v128_t { u8: x.to_le_bytes() };
                                sys::wah_export_global_v128(m, n, mutable, &v)
                            }
                        },
                        Item::Func(ty, f) => {
                            let spec = CString::new(ty.spec()?).unwrap();
                            let data = NonNull::from(Box::leak(Box::new(HostFuncData { store: s, f: f.clone() })));
                            funcs.push(data);
                            let trampoline = store::trampoline::<T>;
                            let ud = data.as_ptr().cast::<c_void>();
                            sys::wah_export_func(m, n, spec.as_ptr(), Some(trampoline), ud, None)
                        }
                    }
                };
                check(r)
                    .map_err(|e| Error::with_context(e.wah_code().unwrap(), format!("{module}.{name} not defined")))?;
            }
            Ok(())
        };
        // SAFETY: `s` is the store's, which nothing borrows
        unsafe { store::new_host(s, module, build) }
    }

    /// Instantiates `module` in `store` ([`Instance::new`]), with the host modules it imports from instantiated
    /// first where the store has none of their names yet.
    pub fn instantiate(&self, store: &mut Store<T>, module: &Module) -> Result<Instance> {
        for i in module.imports() {
            let name = &i.module;
            let s = store.raw();
            // SAFETY: a short borrow of the store
            if !unsafe { (*s).has_name(name) } {
                self.instantiate_host(store, name)?;
            }
        }
        Instance::new(store, module)
    }
}
