//! Functions of Rust types: the values WebAssembly passes ([`WasmTy`]), and host functions of closures over them
//! ([`IntoFunc`], [`IntoPollFunc`]).

use std::sync::Arc;
use std::task::{Context, Poll};

use wah_sys as sys;

use crate::error::{Error, Result};
use crate::instance::Raw;
use crate::store::{Caller, HostFn};
use crate::types::{FuncType, ValType};

type CallCtx = *mut sys::wah_call_context_t;

mod sealed {
    pub trait Sealed {}
}

/// A Rust type of a WebAssembly value: `i32`, `u32`, `i64`, `u64`, `f32`, `f64` and `u128` (for `v128`).
pub trait WasmTy: sealed::Sealed + Copy + Send + 'static {
    #[doc(hidden)]
    const TYPE: ValType;
    #[doc(hidden)]
    unsafe fn param(c: CallCtx, i: usize) -> Self;
    #[doc(hidden)]
    unsafe fn set_result(self, c: CallCtx, i: usize);
    #[doc(hidden)]
    fn to_raw(self) -> sys::wah_value_t;
    #[doc(hidden)]
    unsafe fn from_raw(v: &sys::wah_value_t) -> Self;
}

macro_rules! wasm_ty {
    ($t:ty, $ty:ident, $field:ident, $as:ty, $param:ident, $result:ident) => {
        impl sealed::Sealed for $t {}
        impl WasmTy for $t {
            const TYPE: ValType = ValType::$ty;
            unsafe fn param(c: CallCtx, i: usize) -> $t {
                unsafe { sys::$param(c, i) as $t }
            }
            unsafe fn set_result(self, c: CallCtx, i: usize) {
                unsafe { sys::$result(c, i, self as $as) }
            }
            fn to_raw(self) -> sys::wah_value_t {
                let mut v = sys::wah_value_t::ZERO;
                v.$field = self as $as;
                v
            }
            unsafe fn from_raw(v: &sys::wah_value_t) -> $t {
                unsafe { v.$field as $t }
            }
        }
    };
}

wasm_ty!(i32, I32, i32, i32, wah_param_i32, wah_result_i32);
wasm_ty!(u32, I32, i32, i32, wah_param_i32, wah_result_i32);
wasm_ty!(i64, I64, i64, i64, wah_param_i64, wah_result_i64);
wasm_ty!(u64, I64, i64, i64, wah_param_i64, wah_result_i64);
wasm_ty!(f32, F32, f32, f32, wah_param_f32, wah_result_f32);
wasm_ty!(f64, F64, f64, f64, wah_param_f64, wah_result_f64);

impl sealed::Sealed for u128 {}
impl WasmTy for u128 {
    const TYPE: ValType = ValType::V128;
    unsafe fn param(c: CallCtx, i: usize) -> u128 {
        u128::from_le_bytes(unsafe { sys::wah_param_v128(c, i).u8 })
    }
    unsafe fn set_result(self, c: CallCtx, i: usize) {
        let v = sys::wah_v128_t { u8: self.to_le_bytes() };
        unsafe { sys::wah_result_v128(c, i, &v) }
    }
    fn to_raw(self) -> sys::wah_value_t {
        sys::wah_value_t { _zero: self.to_le_bytes() }
    }
    unsafe fn from_raw(v: &sys::wah_value_t) -> u128 {
        u128::from_le_bytes(unsafe { v._zero })
    }
}

/// The parameters of a function: a [`WasmTy`], or a tuple of them.
pub trait WasmParams: Send + 'static {
    #[doc(hidden)]
    fn types() -> impl Iterator<Item = ValType>;
    #[doc(hidden)]
    fn into_raw(self, out: &mut Vec<Raw>);
}

/// The results of a function: `()`, a [`WasmTy`], or a tuple of them.
pub trait WasmResults: Send + 'static {
    #[doc(hidden)]
    fn types() -> impl Iterator<Item = ValType>;
    #[doc(hidden)]
    unsafe fn set_results(self, c: CallCtx);
    #[doc(hidden)]
    unsafe fn from_raw<'a>(it: &mut impl Iterator<Item = &'a sys::wah_value_t>) -> Self;
}

macro_rules! single {
    ($($t:ty)*) => {$(
        impl WasmParams for $t {
            fn types() -> impl Iterator<Item = ValType> {
                std::iter::once(<$t>::TYPE)
            }
            fn into_raw(self, out: &mut Vec<Raw>) {
                out.push(Raw(self.to_raw()));
            }
        }
        impl WasmResults for $t {
            fn types() -> impl Iterator<Item = ValType> {
                std::iter::once(<$t>::TYPE)
            }
            unsafe fn set_results(self, c: CallCtx) {
                unsafe { self.set_result(c, 0) }
            }
            unsafe fn from_raw<'a>(it: &mut impl Iterator<Item = &'a sys::wah_value_t>) -> Self {
                unsafe { <$t as WasmTy>::from_raw(it.next().expect("a result")) }
            }
        }
    )*};
}

single!(i32 u32 i64 u64 f32 f64 u128);

macro_rules! tuple {
    ($($a:ident)*) => {
        impl<$($a: WasmTy),*> WasmParams for ($($a,)*) {
            fn types() -> impl Iterator<Item = ValType> {
                [$($a::TYPE),*].into_iter()
            }
            #[allow(non_snake_case, unused_variables)]
            fn into_raw(self, out: &mut Vec<Raw>) {
                let ($($a,)*) = self;
                $(out.push(Raw($a.to_raw()));)*
            }
        }
        impl<$($a: WasmTy),*> WasmResults for ($($a,)*) {
            fn types() -> impl Iterator<Item = ValType> {
                [$($a::TYPE),*].into_iter()
            }
            #[allow(non_snake_case, unused_variables, unused_mut, unused_assignments)]
            unsafe fn set_results(self, c: CallCtx) {
                let ($($a,)*) = self;
                let mut i = 0;
                $(unsafe { $a.set_result(c, i) }; i += 1;)*
            }
            #[allow(unused_variables, clippy::unused_unit)]
            unsafe fn from_raw<'a>(it: &mut impl Iterator<Item = &'a sys::wah_value_t>) -> Self {
                ($(unsafe { $a::from_raw(it.next().expect("a result")) },)*)
            }
        }
    };
}

tuple!();
tuple!(A1);
tuple!(A1 A2);
tuple!(A1 A2 A3);
tuple!(A1 A2 A3 A4);
tuple!(A1 A2 A3 A4 A5);
tuple!(A1 A2 A3 A4 A5 A6);
tuple!(A1 A2 A3 A4 A5 A6 A7);
tuple!(A1 A2 A3 A4 A5 A6 A7 A8);
tuple!(A1 A2 A3 A4 A5 A6 A7 A8 A9);
tuple!(A1 A2 A3 A4 A5 A6 A7 A8 A9 A10);
tuple!(A1 A2 A3 A4 A5 A6 A7 A8 A9 A10 A11);
tuple!(A1 A2 A3 A4 A5 A6 A7 A8 A9 A10 A11 A12);

/// What a host function returns: its results, or them in a `Result` (an error traps with it).
pub trait WasmRet {
    #[doc(hidden)]
    type Results: WasmResults;
    #[doc(hidden)]
    fn into_results(self) -> Result<Self::Results>;
}

impl<R: WasmResults> WasmRet for R {
    type Results = R;
    fn into_results(self) -> Result<R> {
        Ok(self)
    }
}

impl<R: WasmResults> WasmRet for Result<R, Error> {
    type Results = R;
    fn into_results(self) -> Result<R> {
        self
    }
}

/// A closure that is a host function: `Fn(Caller<'_, T>, A1, A2, ...) -> R`, of parameters of [`WasmTy`] and
/// what [`WasmRet`] is.
pub trait IntoFunc<T: 'static, Params, Results>: Send + Sync + 'static {
    #[doc(hidden)]
    fn into_func(self) -> (FuncType, Arc<HostFn<T>>);
}

/// A closure that is a host function that may wait:
/// `Fn(Caller<'_, T>, &mut Context<'_>, A1, A2, ...) -> Poll<R>`. On `Poll::Pending`, the activation is suspended
/// (after the closure arranged for the context's waker to be woken), and the closure is called again with the same
/// parameters once the call is polled again.
pub trait IntoPollFunc<T: 'static, Params, Results>: Send + Sync + 'static {
    #[doc(hidden)]
    fn into_func(self) -> (FuncType, Arc<HostFn<T>>);
}

macro_rules! into_func {
    ($($a:ident)*) => {
        impl<T: 'static, F, R, $($a: WasmTy),*> IntoFunc<T, ($($a,)*), R> for F
        where
            F: Fn(Caller<'_, T>, $($a),*) -> R + Send + Sync + 'static,
            R: WasmRet,
        {
            #[allow(non_snake_case, unused_mut, unused_assignments, unused_variables)]
            fn into_func(self) -> (FuncType, Arc<HostFn<T>>) {
                let ty = FuncType::new([$($a::TYPE),*], R::Results::types());
                let f = move |caller: Caller<'_, T>, _: &mut Context<'_>| -> Poll<Result<()>> {
                    let c = caller.cctx;
                    let mut i = 0;
                    // SAFETY: the parameters of the function's type, during the call
                    $(let $a = unsafe { $a::param(c, i) }; i += 1;)*
                    Poll::Ready(self(caller, $($a),*).into_results().map(|r| unsafe { r.set_results(c) }))
                };
                (ty, Arc::new(f))
            }
        }

        impl<T: 'static, F, R, $($a: WasmTy),*> IntoPollFunc<T, ($($a,)*), R> for F
        where
            F: Fn(Caller<'_, T>, &mut Context<'_>, $($a),*) -> Poll<R> + Send + Sync + 'static,
            R: WasmRet,
        {
            #[allow(non_snake_case, unused_mut, unused_assignments, unused_variables)]
            fn into_func(self) -> (FuncType, Arc<HostFn<T>>) {
                let ty = FuncType::new([$($a::TYPE),*], R::Results::types());
                let f = move |caller: Caller<'_, T>, cx: &mut Context<'_>| -> Poll<Result<()>> {
                    let c = caller.cctx;
                    let mut i = 0;
                    // SAFETY: as above
                    $(let $a = unsafe { $a::param(c, i) }; i += 1;)*
                    match self(caller, cx, $($a),*) {
                        Poll::Ready(r) => Poll::Ready(r.into_results().map(|r| unsafe { r.set_results(c) })),
                        Poll::Pending => Poll::Pending,
                    }
                };
                (ty, Arc::new(f))
            }
        }
    };
}

into_func!();
into_func!(A1);
into_func!(A1 A2);
into_func!(A1 A2 A3);
into_func!(A1 A2 A3 A4);
into_func!(A1 A2 A3 A4 A5);
into_func!(A1 A2 A3 A4 A5 A6);
into_func!(A1 A2 A3 A4 A5 A6 A7);
into_func!(A1 A2 A3 A4 A5 A6 A7 A8);
into_func!(A1 A2 A3 A4 A5 A6 A7 A8 A9);
into_func!(A1 A2 A3 A4 A5 A6 A7 A8 A9 A10);
into_func!(A1 A2 A3 A4 A5 A6 A7 A8 A9 A10 A11);
into_func!(A1 A2 A3 A4 A5 A6 A7 A8 A9 A10 A11 A12);
