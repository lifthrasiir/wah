//! Safe bindings to [WAH](https://github.com/lifthrasiir/wah), a WebAssembly interpreter (with an optional native
//! compiler) in a single C header.
//!
//! The API is close to Wasmtime's where WAH allows:
//!
//! - A [`Module`] is parsed and validated once ([`Config`] gives the proposals enabled), and instantiated by any
//!   number of stores, from any thread.
//! - A [`Store`] holds the data of the host functions (`T`) and every instance and host module made in it.
//!   Its memories are limited by a [`ResourceLimiter`] (also asked as memories are created).
//! - A [`Linker`] defines host modules: memories, globals and host functions under module names, instantiated in a
//!   store before the instances that import from them. Instances import from the store by module names: so memories
//!   that a host makes are written before instantiation by [`Linker::instantiate_hosts`], [`Store::get_memory`] and
//!   then [`Instance::new`].
//! - Host functions may wait: one of [`Linker::func_wrap_poll`] returns `Poll::Pending` (having arranged for the
//!   waker to be woken), which suspends the activation in WAH (`wah_suspend`) and makes the future of
//!   [`Func::call_async`] pending, without a fiber; the host function is called again with the same parameters once
//!   polled again. [`Func::call`] waits in its thread instead.
//! - Errors of host functions and limiters ([`Error::new`]) and panics pass through WebAssembly unchanged.
//!
//! Values of reference types can't be passed between the host and WebAssembly yet, and the host can't grow memories.

mod error;
mod func;
mod instance;
mod linker;
mod module;
mod store;
mod types;

pub use error::{Code, Error, Result};
pub use func::{IntoFunc, IntoPollFunc, WasmParams, WasmResults, WasmRet, WasmTy};
pub use instance::{Func, Instance, Memory, TypedFunc};
pub use linker::Linker;
pub use module::{Config, Features, Module};
pub use store::{AsContext, AsContextMut, Caller, ResourceLimiter, Store, StoreCtx};
pub use types::{
    ExportType, ExternType, FuncType, GlobalType, ImportType, MemoryType, PAGE_SIZE, TableType, Val, ValType,
};

pub use wah_sys as sys;
