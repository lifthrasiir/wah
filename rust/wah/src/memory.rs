use std::ffi::c_void;
use std::panic::{AssertUnwindSafe, catch_unwind};
use std::sync::Arc;

use wah_sys as sys;

use crate::error::{Code, Error, Result, Thrown};
use crate::types::{MemoryType, PAGE_SIZE};

/// A linear memory that a [`MemoryCreator`] made.
///
/// # Safety
/// The memory should be readable and writable for `byte_size` bytes from `as_ptr`, which never changes for its
/// lifetime, and aligned to at least 16 bytes. `grow_to` should make the new bytes readable, writable and zeroed in
/// place. Where the creator was asked to reserve address space (`reserved`), all of it and `guard` bytes after it
/// should be reserved from `as_ptr`, and every byte beyond `byte_size` inaccessible (so that accesses fault).
///
/// While nothing of its store runs, the host may change the contents, or replace its pages by other mappings at the
/// same address and of the same accessibility (e.g. a view copied on write).
pub unsafe trait LinearMemory: Send + Sync + 'static {
    /// The accessible size, in bytes.
    fn byte_size(&self) -> usize;

    /// The most the memory can grow to, in bytes.
    fn byte_capacity(&self) -> usize;

    /// Grows the memory to `new_size` bytes (a multiple of the page size, at most `byte_capacity`). An error makes
    /// `memory.grow` return -1.
    fn grow_to(&mut self, new_size: usize) -> Result<()>;

    /// The base of the memory.
    fn as_ptr(&self) -> *mut u8;
}

/// Makes the memories of a store in place of WAH ([`Store::set_memory_creator`](crate::Store)), like Wasmtime's.
///
/// # Safety
/// The memories made should meet the requirements of [`LinearMemory`], and be of `minimum` bytes of zeros.
pub unsafe trait MemoryCreator: Send + Sync {
    /// Makes a memory of type `ty`, of `minimum` bytes, which may grow to `maximum` bytes (by its type, if any).
    ///
    /// `reserved` is given (with `guard`) where natively compiled code skips bounds checks: 4 GiB for 32-bit memories
    /// once guard pages are enabled (`wah_sys::wah_enable_guard_pages`), with a guard of 2 GiB. Its memory is never
    /// accessed beyond its size then, as faults are caught. Otherwise `guard` is 0, and memories are checked.
    fn new_memory(
        &self,
        ty: MemoryType,
        minimum: usize,
        maximum: Option<usize>,
        reserved: Option<usize>,
        guard: usize,
    ) -> std::result::Result<Box<dyn LinearMemory>, String>;
}

/// The userdata of the creator of a store, which outlives its contexts.
pub(crate) struct CreatorData {
    pub creator: Arc<dyn MemoryCreator>,
    /// `StoreCtx::thrown`, where errors and panics go.
    pub thrown: *mut Option<Thrown>,
}

impl CreatorData {
    pub(crate) fn raw(data: *mut CreatorData) -> sys::wah_memory_creator_t {
        sys::wah_memory_creator_t { create: Some(create), grow: Some(grow), free: Some(free), userdata: data.cast() }
    }
}

const GIB4: u64 = 1 << 32;

unsafe extern "C" fn create(
    userdata: *mut c_void,
    req: *const sys::wah_memory_request_t,
    out: *mut sys::wah_host_memory_t,
) -> sys::wah_error_t {
    // SAFETY: the creator's data lives as long as the store, and WAH gives a valid request and output
    let (d, req) = unsafe { (&*userdata.cast::<CreatorData>(), &*req) };
    let mut desc = unsafe { std::mem::zeroed::<sys::wah_memory_desc_t>() };
    if unsafe { sys::wah_module_memory(req.module, req.mem_idx, &mut desc) } != sys::WAH_OK {
        return sys::WAH_ERROR_MISUSE;
    }
    let max = (desc.max_pages != u64::MAX).then_some(desc.max_pages);
    let ty = match req.is_64 {
        true => MemoryType::new64(desc.min_pages, max),
        false => MemoryType::new(desc.min_pages as u32, max.map(|m| m as u32)),
    };
    let size = |b: u64| usize::try_from(b).ok();
    let Some(minimum) = size(req.min_bytes) else { return sys::WAH_ERROR_TOO_LARGE };
    let maximum = max.map(|m| size(m.saturating_mul(PAGE_SIZE)).unwrap_or(usize::MAX));
    let (reserved, guard) = match req.reserve_bytes {
        0 => (None, 0),
        r => match (size(GIB4), size(r.saturating_sub(GIB4))) {
            (Some(res), Some(g)) => (Some(res), g),
            _ => (None, 0),
        },
    };
    let r = catch_unwind(AssertUnwindSafe(|| d.creator.new_memory(ty, minimum, maximum, reserved, guard)));
    // SAFETY: as above; `thrown` is the store's, which nothing borrows while WAH calls this
    unsafe {
        match r {
            Ok(Ok(m)) => {
                let data = m.as_ptr();
                let handle = Box::into_raw(Box::new(m));
                *out = sys::wah_host_memory_t { data, handle: handle.cast(), guarded: reserved.is_some() };
                sys::WAH_OK
            }
            Ok(Err(msg)) => {
                *d.thrown = Some(Thrown::Error(Error::with_context(Code::OutOfMemory, msg)));
                sys::WAH_ERROR_OUT_OF_MEMORY
            }
            Err(p) => {
                *d.thrown = Some(Thrown::Panic(p));
                sys::WAH_ERROR_TRAP
            }
        }
    }
}

unsafe extern "C" fn grow(userdata: *mut c_void, handle: *mut c_void, _old: u64, new_bytes: u64) -> sys::wah_error_t {
    // SAFETY: of `create`; WAH grows a memory in one thread at a time
    let (d, m) = unsafe { (&*userdata.cast::<CreatorData>(), &mut *handle.cast::<Box<dyn LinearMemory>>()) };
    let Some(new) = usize::try_from(new_bytes).ok().filter(|&n| n <= m.byte_capacity()) else {
        return sys::WAH_ERROR_OUT_OF_MEMORY;
    };
    match catch_unwind(AssertUnwindSafe(|| m.grow_to(new))) {
        Ok(Ok(())) => sys::WAH_OK,
        Ok(Err(_)) => sys::WAH_ERROR_OUT_OF_MEMORY,
        Err(p) => {
            // SAFETY: as in `create`
            unsafe { *d.thrown = Some(Thrown::Panic(p)) };
            sys::WAH_ERROR_TRAP
        }
    }
}

unsafe extern "C" fn free(_userdata: *mut c_void, handle: *mut c_void) {
    // SAFETY: of `create`, freed once
    drop(unsafe { Box::from_raw(handle.cast::<Box<dyn LinearMemory>>()) });
}
