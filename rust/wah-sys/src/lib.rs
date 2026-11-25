//! Raw FFI bindings to [WAH](https://github.com/lifthrasiir/wah), a WebAssembly interpreter in a single C header.
//!
//! The declarations mirror the public API of `wah.h` one to one; see the comments there for the semantics.
//! `wah_module_t` and `wah_exec_context_t` are opaque, immovable once used, and 16-byte aligned:
//! keep them boxed. The `wah` crate wraps this safely.
//!
//! The implementation is built from `wahjit.h` with the feature `jit` (default), otherwise from `wah.h`.
//! The proposals compiled in are selected by the features of their names (`wasm3`, all of them, by default; see
//! `WAH_COMPILED_FEATURES`).
//! `wah_define_typev` is left out, as `va_list` can't be passed from stable Rust.

#![no_std]
#![allow(non_camel_case_types, non_snake_case)]

use core::ffi::{c_char, c_int, c_void};
use core::marker::{PhantomData, PhantomPinned};

/// The version of the WAH API this crate was written for (`WAH_VERSION`).
pub const WAH_VERSION: i32 = -2675;

pub type wah_error_t = c_int;
pub const WAH_OK: wah_error_t = 0;
pub const WAH_ERROR_INVALID_MAGIC_NUMBER: wah_error_t = -1;
pub const WAH_ERROR_INVALID_VERSION: wah_error_t = -2;
pub const WAH_ERROR_UNEXPECTED_EOF: wah_error_t = -3;
pub const WAH_ERROR_MALFORMED: wah_error_t = -4;
pub const WAH_ERROR_TOO_LARGE: wah_error_t = -5;
pub const WAH_ERROR_OUT_OF_MEMORY: wah_error_t = -6;
pub const WAH_ERROR_VALIDATION_FAILED: wah_error_t = -7;
pub const WAH_ERROR_TRAP: wah_error_t = -8;
pub const WAH_ERROR_STACK_OVERFLOW: wah_error_t = -9;
pub const WAH_ERROR_MEMORY_OUT_OF_BOUNDS: wah_error_t = -10;
pub const WAH_ERROR_NOT_FOUND: wah_error_t = -11;
pub const WAH_ERROR_MISUSE: wah_error_t = -12;
pub const WAH_ERROR_BAD_SPEC: wah_error_t = -13;
pub const WAH_ERROR_LINK_FAILED: wah_error_t = -14;
pub const WAH_ERROR_EXCEPTION: wah_error_t = -15;
pub const WAH_ERROR_DISABLED_FEATURE: wah_error_t = -16;
pub const WAH_ERROR_MULTI_RETURN: wah_error_t = -17;
pub const WAH_ERROR_UNSUPPORTED: wah_error_t = -18;
pub const WAH_STATUS_FUEL_EXHAUSTED: wah_error_t = 2;
pub const WAH_STATUS_YIELDED: wah_error_t = 3;

pub type wah_exec_state_t = c_int;
pub const WAH_EXEC_READY: wah_exec_state_t = 0;
pub const WAH_EXEC_RUNNING: wah_exec_state_t = 1;
pub const WAH_EXEC_SUSPENDED: wah_exec_state_t = 2;
pub const WAH_EXEC_FINISHED: wah_exec_state_t = 3;
pub const WAH_EXEC_TRAPPED: wah_exec_state_t = 4;

#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct wah_alloc_t {
    pub malloc: Option<unsafe extern "C" fn(size: usize, userdata: *mut c_void) -> *mut c_void>,
    pub realloc: Option<unsafe extern "C" fn(ptr: *mut c_void, size: usize, userdata: *mut c_void) -> *mut c_void>,
    pub free: Option<unsafe extern "C" fn(ptr: *mut c_void, userdata: *mut c_void)>,
    pub userdata: *mut c_void,
    pub min_align: usize,
}

#[repr(C, align(16))]
#[derive(Clone, Copy)]
pub union wah_v128_t {
    pub u8: [u8; 16],
    pub u16: [u16; 8],
    pub u32: [u32; 4],
    pub u64: [u64; 2],
    pub i8: [i8; 16],
    pub i16: [i16; 8],
    pub i32: [i32; 4],
    pub i64: [i64; 2],
    pub f32: [f32; 4],
    pub f64: [f64; 2],
}

#[repr(C, align(16))]
#[derive(Clone, Copy)]
pub union wah_value_t {
    /// Zeroes the whole value (`wah_value_t::ZERO`).
    pub _zero: [u8; 16],
    pub i32: i32,
    pub i64: i64,
    pub f32: f32,
    pub f64: f64,
    pub v128: wah_v128_t,
    pub r#ref: *mut c_void,
}

impl wah_value_t {
    pub const ZERO: wah_value_t = wah_value_t { _zero: [0; 16] };
}

pub type wah_type_t = i32;
pub const WAH_TYPE_I32: wah_type_t = -2;
pub const WAH_TYPE_I64: wah_type_t = -4;
pub const WAH_TYPE_F32: wah_type_t = -6;
pub const WAH_TYPE_F64: wah_type_t = -8;
pub const WAH_TYPE_V128: wah_type_t = -10;
pub const WAH_TYPE_PACKED_I8: wah_type_t = -16;
pub const WAH_TYPE_PACKED_I16: wah_type_t = -18;
pub const WAH_TYPE_FUNC: wah_type_t = -32;
pub const WAH_TYPE_FUNCREF: wah_type_t = -31;
pub const WAH_TYPE_EXTERN: wah_type_t = -34;
pub const WAH_TYPE_EXTERNREF: wah_type_t = -33;
pub const WAH_TYPE_ANY: wah_type_t = -36;
pub const WAH_TYPE_ANYREF: wah_type_t = -35;
pub const WAH_TYPE_EQ: wah_type_t = -38;
pub const WAH_TYPE_EQREF: wah_type_t = -37;
pub const WAH_TYPE_I31: wah_type_t = -40;
pub const WAH_TYPE_I31REF: wah_type_t = -39;
pub const WAH_TYPE_STRUCT: wah_type_t = -42;
pub const WAH_TYPE_STRUCTREF: wah_type_t = -41;
pub const WAH_TYPE_ARRAY: wah_type_t = -44;
pub const WAH_TYPE_ARRAYREF: wah_type_t = -43;
pub const WAH_TYPE_EXN: wah_type_t = -46;
pub const WAH_TYPE_EXNREF: wah_type_t = -45;
pub const WAH_TYPE_NOFUNC: wah_type_t = -48;
pub const WAH_TYPE_NULLFUNCREF: wah_type_t = -47;
pub const WAH_TYPE_NOEXTERN: wah_type_t = -50;
pub const WAH_TYPE_NULLEXTERNREF: wah_type_t = -49;
pub const WAH_TYPE_NONE: wah_type_t = -52;
pub const WAH_TYPE_NULLREF: wah_type_t = -51;
pub const WAH_TYPE_NOEXN: wah_type_t = -54;
pub const WAH_TYPE_NULLEXNREF: wah_type_t = -53;

#[inline]
pub const fn WAH_TYPE_IS_NULLABLE(t: wah_type_t) -> bool {
    t & 1 != 0
}
#[inline]
pub const fn WAH_TYPE_AS_NULLABLE(t: wah_type_t) -> wah_type_t {
    t | 1
}
#[inline]
pub const fn WAH_TYPE_AS_NON_NULL(t: wah_type_t) -> wah_type_t {
    t & !1
}
#[inline]
pub const fn WAH_TYPE_IS_PACKED(t: wah_type_t) -> bool {
    t == WAH_TYPE_PACKED_I8 || t == WAH_TYPE_PACKED_I16
}
#[inline]
pub const fn WAH_TYPE_IS_REF(t: wah_type_t) -> bool {
    (t <= -31 && t >= -54) || t >= 0
}

pub type wah_features_t = u64;
pub const WAH_FEATURE_MVP: wah_features_t = 1 << 0;
pub const WAH_FEATURE_MULTI_VALUE: wah_features_t = 1 << 1;
pub const WAH_FEATURE_MUTABLE_GLOBALS: wah_features_t = 1 << 2;
pub const WAH_FEATURE_SIGN_EXT: wah_features_t = 1 << 3;
pub const WAH_FEATURE_NONTRAPPING_F2I: wah_features_t = 1 << 4;
pub const WAH_FEATURE_BULK_MEMORY: wah_features_t = 1 << 5;
pub const WAH_FEATURE_REF_TYPES: wah_features_t = 1 << 6;
pub const WAH_FEATURE_SIMD: wah_features_t = 1 << 7;
pub const WAH_FEATURE_TAIL_CALL: wah_features_t = 1 << 8;
pub const WAH_FEATURE_EXCEPTION: wah_features_t = 1 << 9;
pub const WAH_FEATURE_GC: wah_features_t = 1 << 10;
pub const WAH_FEATURE_TYPED_FUNCREF: wah_features_t = 1 << 11;
pub const WAH_FEATURE_MEMORY64: wah_features_t = 1 << 12;
pub const WAH_FEATURE_RELAXED_SIMD: wah_features_t = 1 << 13;
pub const WAH_FEATURE_EXTENDED_CONST: wah_features_t = 1 << 14;
pub const WAH_FEATURE_MULTI_MEMORY: wah_features_t = 1 << 15;
pub const WAH_FEATURE_WASM_V2: wah_features_t = WAH_FEATURE_MVP
    | WAH_FEATURE_MULTI_VALUE
    | WAH_FEATURE_MUTABLE_GLOBALS
    | WAH_FEATURE_SIGN_EXT
    | WAH_FEATURE_NONTRAPPING_F2I
    | WAH_FEATURE_BULK_MEMORY
    | WAH_FEATURE_REF_TYPES
    | WAH_FEATURE_SIMD;
pub const WAH_FEATURE_WASM_V3: wah_features_t = WAH_FEATURE_WASM_V2
    | WAH_FEATURE_TAIL_CALL
    | WAH_FEATURE_EXCEPTION
    | WAH_FEATURE_GC
    | WAH_FEATURE_TYPED_FUNCREF
    | WAH_FEATURE_MEMORY64
    | WAH_FEATURE_RELAXED_SIMD
    | WAH_FEATURE_EXTENDED_CONST
    | WAH_FEATURE_MULTI_MEMORY;
pub const WAH_FEATURE_ALL: wah_features_t = WAH_FEATURE_WASM_V3;

/// The proposals compiled in (`WAH_COMPILED_FEATURES`), as the features of this crate select them.
pub const WAH_COMPILED_FEATURES: wah_features_t = {
    const fn bit(on: bool, f: wah_features_t) -> wah_features_t {
        if on { f } else { 0 }
    }
    WAH_FEATURE_MVP
        | bit(cfg!(feature = "multi-value"), WAH_FEATURE_MULTI_VALUE)
        | bit(cfg!(feature = "mutable-globals"), WAH_FEATURE_MUTABLE_GLOBALS)
        | bit(cfg!(feature = "sign-ext"), WAH_FEATURE_SIGN_EXT)
        | bit(cfg!(feature = "nontrapping-f2i"), WAH_FEATURE_NONTRAPPING_F2I)
        | bit(cfg!(feature = "bulk-memory"), WAH_FEATURE_BULK_MEMORY)
        | bit(cfg!(feature = "ref-types"), WAH_FEATURE_REF_TYPES)
        | bit(cfg!(feature = "simd"), WAH_FEATURE_SIMD)
        | bit(cfg!(feature = "tail-call"), WAH_FEATURE_TAIL_CALL)
        | bit(cfg!(feature = "exception"), WAH_FEATURE_EXCEPTION)
        | bit(cfg!(feature = "gc"), WAH_FEATURE_GC)
        | bit(cfg!(feature = "typed-funcref"), WAH_FEATURE_TYPED_FUNCREF)
        | bit(cfg!(feature = "memory64"), WAH_FEATURE_MEMORY64)
        | bit(cfg!(feature = "relaxed-simd"), WAH_FEATURE_RELAXED_SIMD)
        | bit(cfg!(feature = "extended-const"), WAH_FEATURE_EXTENDED_CONST)
        | bit(cfg!(feature = "multi-memory"), WAH_FEATURE_MULTI_MEMORY)
};

pub type wah_comp_type_kind_t = c_int;
pub const WAH_COMP_FUNC: wah_comp_type_kind_t = 0x60;
pub const WAH_COMP_STRUCT: wah_comp_type_kind_t = 0x5F;
pub const WAH_COMP_ARRAY: wah_comp_type_kind_t = 0x5E;

pub const WAH_KIND_FUNCTION: u8 = 0;
pub const WAH_KIND_TABLE: u8 = 1;
pub const WAH_KIND_MEMORY: u8 = 2;
pub const WAH_KIND_GLOBAL: u8 = 3;
pub const WAH_KIND_TAG: u8 = 4;

#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct wah_type_desc_t {
    pub kind: wah_comp_type_kind_t,
    pub is_final: bool,
    pub supertype: u32,
    pub param_count: u32,
    pub result_count: u32,
    pub param_types: *const wah_type_t,
    pub result_types: *const wah_type_t,
    pub field_count: u32,
    pub field_types: *const wah_type_t,
    pub field_mutables: *const bool,
}

#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct wah_func_desc_t {
    pub is_import: bool,
    pub is_host: bool,
    pub type_index: u32,
    pub param_count: u32,
    pub result_count: u32,
    pub param_types: *const wah_type_t,
    pub result_types: *const wah_type_t,
}

#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct wah_global_desc_t {
    pub r#type: wah_type_t,
    pub is_mutable: bool,
}

#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct wah_memory_desc_t {
    pub addr_type: wah_type_t,
    pub min_pages: u64,
    pub max_pages: u64,
}

#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct wah_table_desc_t {
    pub elem_type: wah_type_t,
    pub addr_type: wah_type_t,
    pub min_elements: u64,
    pub max_elements: u64,
}

#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct wah_tag_desc_t {
    pub type_index: u32,
    pub param_count: u32,
    pub param_types: *const wah_type_t,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub union wah_extern_desc_t {
    pub func: wah_func_desc_t,
    pub table: wah_table_desc_t,
    pub memory: wah_memory_desc_t,
    pub global: wah_global_desc_t,
    pub tag: wah_tag_desc_t,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct wah_import_desc_t {
    pub kind: u8,
    pub index: u32,
    pub module: *const c_char,
    pub module_len: usize,
    pub name: *const c_char,
    pub name_len: usize,
    pub u: wah_extern_desc_t,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct wah_export_desc_t {
    pub kind: u8,
    pub index: u32,
    pub name: *const c_char,
    pub name_len: usize,
    pub u: wah_extern_desc_t,
}

/// Opaque; only exists behind pointers given to host functions.
#[repr(C)]
pub struct wah_call_context_t {
    _private: [u8; 0],
    _marker: PhantomData<(*mut u8, PhantomPinned)>,
}

pub type wah_func_t = Option<unsafe extern "C" fn(ctx: *mut wah_call_context_t, userdata: *mut c_void)>;
pub type wah_finalize_t = Option<unsafe extern "C" fn(userdata: *mut c_void)>;

/// Opaque and immovable once given to WAH. Zero it before use (`wah_module_t::ZERO`).
#[repr(C, align(16))]
pub struct wah_module_t {
    _reserved: [u8; 384],
    _marker: PhantomData<(*mut u8, PhantomPinned)>,
}

/// Opaque and immovable once given to WAH. Zero it before use (`wah_exec_context_t::ZERO`).
#[repr(C, align(16))]
pub struct wah_exec_context_t {
    _reserved: [u8; 384],
    _marker: PhantomData<(*mut u8, PhantomPinned)>,
}

impl wah_module_t {
    pub const ZERO: wah_module_t = wah_module_t { _reserved: [0; 384], _marker: PhantomData };
}

impl wah_exec_context_t {
    pub const ZERO: wah_exec_context_t = wah_exec_context_t { _reserved: [0; 384], _marker: PhantomData };
}

pub type wah_fault_handling_t = c_int;
pub const WAH_HOST_HANDLES_FAULT: wah_fault_handling_t = 2;
pub const WAH_HANDLES_FAULT: wah_fault_handling_t = 3;

#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct wah_parse_options_t {
    pub features: wah_features_t,
    pub enable_fuel_metering: bool,
    pub alloc: *const wah_alloc_t,
    pub disable_jit: bool,
}

#[repr(C)]
#[derive(Debug, Clone, Copy, Default)]
pub struct wah_limits_t {
    pub max_stack_bytes: u64,
    pub max_memory_bytes: u64,
    pub fuel: u64,
    pub deadline_us: u64,
    pub no_memory_bytes: bool,
}

#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct wah_exec_options_t {
    pub limits: wah_limits_t,
    pub alloc: *const wah_alloc_t,
    pub memory_creator: *const wah_memory_creator_t,
    pub userdata: *mut c_void,
}

#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct wah_memory_request_t {
    pub module: *const wah_module_t,
    pub mem_idx: u32,
    pub is_64: bool,
    pub min_bytes: u64,
    pub max_bytes: u64,
    pub reserve_bytes: u64,
}

#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct wah_host_memory_t {
    pub data: *mut u8,
    pub handle: *mut c_void,
    pub guarded: bool,
}

#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct wah_memory_creator_t {
    pub create: Option<
        unsafe extern "C" fn(
            userdata: *mut c_void,
            req: *const wah_memory_request_t,
            out: *mut wah_host_memory_t,
        ) -> wah_error_t,
    >,
    pub grow: Option<
        unsafe extern "C" fn(userdata: *mut c_void, handle: *mut c_void, old_bytes: u64, new_bytes: u64) -> wah_error_t,
    >,
    pub free: Option<unsafe extern "C" fn(userdata: *mut c_void, handle: *mut c_void)>,
    pub userdata: *mut c_void,
}

pub type wah_memory_grow_hook_t =
    Option<unsafe extern "C" fn(userdata: *mut c_void, mem_idx: u32, old_bytes: u64, new_bytes: u64) -> wah_error_t>;

pub type wah_gc_phase_t = c_int;
pub const WAH_GC_PHASE_IDLE: wah_gc_phase_t = 0;
pub const WAH_GC_PHASE_MARK: wah_gc_phase_t = 1;
pub const WAH_GC_PHASE_SWEEP: wah_gc_phase_t = 2;

#[repr(C)]
#[derive(Debug, Clone, Copy, Default)]
pub struct wah_gc_heap_stats_t {
    pub object_count: u32,
    pub allocated_bytes: usize,
    pub allocation_threshold: usize,
    pub phase: wah_gc_phase_t,
    pub total_collections: u32,
    pub total_allocations: u32,
    pub total_frees: u32,
    pub total_polls: u32,
}

unsafe extern "C" {
    pub fn wah_strerror(err: wah_error_t) -> *const c_char;

    pub fn wah_enable_guard_pages(mode: wah_fault_handling_t) -> wah_error_t;
    pub fn wah_handle_fault(signo: c_int, info: *mut c_void, uctx: *mut c_void) -> bool;

    pub fn wah_parse_module(
        module: *mut wah_module_t,
        binary: *const u8,
        binary_size: usize,
        options: *const wah_parse_options_t,
    ) -> wah_error_t;

    pub fn wah_module_type_count(module: *const wah_module_t) -> u32;
    pub fn wah_module_function_count(module: *const wah_module_t) -> u32;
    pub fn wah_module_global_count(module: *const wah_module_t) -> u32;
    pub fn wah_module_memory_count(module: *const wah_module_t) -> u32;
    pub fn wah_module_table_count(module: *const wah_module_t) -> u32;
    pub fn wah_module_tag_count(module: *const wah_module_t) -> u32;
    pub fn wah_module_import_count(module: *const wah_module_t) -> u32;
    pub fn wah_module_export_count(module: *const wah_module_t) -> u32;

    pub fn wah_module_type(module: *const wah_module_t, idx: u32, out: *mut wah_type_desc_t) -> wah_error_t;
    pub fn wah_module_function(module: *const wah_module_t, idx: u32, out: *mut wah_func_desc_t) -> wah_error_t;
    pub fn wah_module_global(module: *const wah_module_t, idx: u32, out: *mut wah_global_desc_t) -> wah_error_t;
    pub fn wah_module_memory(module: *const wah_module_t, idx: u32, out: *mut wah_memory_desc_t) -> wah_error_t;
    pub fn wah_module_table(module: *const wah_module_t, idx: u32, out: *mut wah_table_desc_t) -> wah_error_t;
    pub fn wah_module_tag(module: *const wah_module_t, idx: u32, out: *mut wah_tag_desc_t) -> wah_error_t;
    pub fn wah_module_import(module: *const wah_module_t, idx: u32, out: *mut wah_import_desc_t) -> wah_error_t;
    pub fn wah_module_export(module: *const wah_module_t, idx: usize, out: *mut wah_export_desc_t) -> wah_error_t;

    pub fn wah_export_by_name(
        module: *const wah_module_t,
        name: *const c_char,
        out: *mut wah_export_desc_t,
    ) -> wah_error_t;
    pub fn wah_export_by_name_len(
        module: *const wah_module_t,
        name: *const c_char,
        name_len: usize,
        out: *mut wah_export_desc_t,
    ) -> wah_error_t;

    pub fn wah_new_exec_context(
        exec_ctx: *mut wah_exec_context_t,
        module: *const wah_module_t,
        options: *const wah_exec_options_t,
    ) -> wah_error_t;
    pub fn wah_set_limits(exec_ctx: *mut wah_exec_context_t, limits: *const wah_limits_t) -> wah_error_t;
    pub fn wah_get_limits(exec_ctx: *const wah_exec_context_t, out: *mut wah_limits_t);
    pub fn wah_free_exec_context(exec_ctx: *mut wah_exec_context_t);

    pub fn wah_call(
        exec_ctx: *mut wah_exec_context_t,
        func_idx: u64,
        params: *const wah_value_t,
        param_count: u32,
        result: *mut wah_value_t,
    ) -> wah_error_t;
    pub fn wah_call_multi(
        exec_ctx: *mut wah_exec_context_t,
        func_idx: u64,
        params: *const wah_value_t,
        param_count: u32,
        results: *mut wah_value_t,
        max_result_count: u32,
        actual_result_count: *mut u32,
    ) -> wah_error_t;
    pub fn wah_call_by_name(
        exec_ctx: *mut wah_exec_context_t,
        name: *const c_char,
        params: *const wah_value_t,
        param_count: u32,
        result: *mut wah_value_t,
    ) -> wah_error_t;
    pub fn wah_call_pin(
        exec_ctx: *mut wah_exec_context_t,
        func_idx: u64,
        params: *const wah_value_t,
        param_count: u32,
        result: *mut wah_value_t,
    ) -> wah_error_t;

    pub fn wah_set_fuel(ctx: *mut wah_exec_context_t, fuel: i64) -> wah_error_t;
    pub fn wah_get_fuel(ctx: *const wah_exec_context_t) -> i64;

    pub fn wah_start(
        ctx: *mut wah_exec_context_t,
        func_idx: u64,
        params: *const wah_value_t,
        param_count: u32,
    ) -> wah_error_t;
    pub fn wah_resume(ctx: *mut wah_exec_context_t) -> wah_error_t;
    pub fn wah_finish(
        ctx: *mut wah_exec_context_t,
        results: *mut wah_value_t,
        max_result_count: u32,
        actual_result_count: *mut u32,
    ) -> wah_error_t;
    pub fn wah_finish_pin(
        ctx: *mut wah_exec_context_t,
        results: *mut wah_value_t,
        max_result_count: u32,
        actual_result_count: *mut u32,
    ) -> wah_error_t;
    pub fn wah_unpin_ref(ctx: *mut wah_exec_context_t, pinned_ref: *mut c_void) -> wah_error_t;
    pub fn wah_cancel(ctx: *mut wah_exec_context_t);
    pub fn wah_is_suspended(ctx: *const wah_exec_context_t) -> bool;
    pub fn wah_exec_state(ctx: *const wah_exec_context_t) -> wah_exec_state_t;

    pub fn wah_free_module(module: *mut wah_module_t);

    pub fn wah_new_module(module: *mut wah_module_t, alloc: *const wah_alloc_t) -> wah_error_t;
    pub fn wah_define_type(
        module: *mut wah_module_t,
        out_type: *mut wah_type_t,
        spec: *const c_char,
        ...
    ) -> wah_error_t;
    pub fn wah_export_func(
        module: *mut wah_module_t,
        name: *const c_char,
        types: *const c_char,
        func: wah_func_t,
        userdata: *mut c_void,
        finalize: wah_finalize_t,
    ) -> wah_error_t;
    pub fn wah_export_typed_func(
        module: *mut wah_module_t,
        name: *const c_char,
        r#type: wah_type_t,
        func: wah_func_t,
        userdata: *mut c_void,
        finalize: wah_finalize_t,
    ) -> wah_error_t;
    pub fn wah_export_memory(
        module: *mut wah_module_t,
        name: *const c_char,
        min_pages: u64,
        max_pages: u64,
    ) -> wah_error_t;
    pub fn wah_export_memory64(
        module: *mut wah_module_t,
        name: *const c_char,
        min_pages: u64,
        max_pages: u64,
    ) -> wah_error_t;
    pub fn wah_export_global_i32(
        module: *mut wah_module_t,
        name: *const c_char,
        is_mutable: bool,
        init_value: i32,
    ) -> wah_error_t;
    pub fn wah_export_global_i64(
        module: *mut wah_module_t,
        name: *const c_char,
        is_mutable: bool,
        init_value: i64,
    ) -> wah_error_t;
    pub fn wah_export_global_f32(
        module: *mut wah_module_t,
        name: *const c_char,
        is_mutable: bool,
        init_value: f32,
    ) -> wah_error_t;
    pub fn wah_export_global_f64(
        module: *mut wah_module_t,
        name: *const c_char,
        is_mutable: bool,
        init_value: f64,
    ) -> wah_error_t;
    pub fn wah_export_global_v128(
        module: *mut wah_module_t,
        name: *const c_char,
        is_mutable: bool,
        init_value: *const wah_v128_t,
    ) -> wah_error_t;

    pub fn wah_param_count(ctx: *const wah_call_context_t) -> usize;
    pub fn wah_param_type(ctx: *const wah_call_context_t, index: usize) -> wah_type_t;
    pub fn wah_param_i32(ctx: *const wah_call_context_t, index: usize) -> i32;
    pub fn wah_param_i64(ctx: *const wah_call_context_t, index: usize) -> i64;
    pub fn wah_param_f32(ctx: *const wah_call_context_t, index: usize) -> f32;
    pub fn wah_param_f64(ctx: *const wah_call_context_t, index: usize) -> f64;
    pub fn wah_param_v128(ctx: *const wah_call_context_t, index: usize) -> wah_v128_t;
    pub fn wah_param_ref(ctx: *const wah_call_context_t, index: usize) -> *mut c_void;
    pub fn wah_param_pinned_ref(ctx: *mut wah_call_context_t, index: usize) -> *mut c_void;
    pub fn wah_unpin_ref_from_host(ctx: *mut wah_call_context_t, pinned_ref: *mut c_void);

    pub fn wah_result_count(ctx: *const wah_call_context_t) -> usize;
    pub fn wah_result_type(ctx: *const wah_call_context_t, index: usize) -> wah_type_t;
    pub fn wah_result_i32(ctx: *mut wah_call_context_t, index: usize, value: i32);
    pub fn wah_result_i64(ctx: *mut wah_call_context_t, index: usize, value: i64);
    pub fn wah_result_f32(ctx: *mut wah_call_context_t, index: usize, value: f32);
    pub fn wah_result_f64(ctx: *mut wah_call_context_t, index: usize, value: f64);
    pub fn wah_result_v128(ctx: *mut wah_call_context_t, index: usize, value: *const wah_v128_t);
    pub fn wah_result_ref(ctx: *mut wah_call_context_t, index: usize, value: *mut c_void);

    pub fn wah_trap(ctx: *mut wah_call_context_t, reason: wah_error_t);
    pub fn wah_suspend(ctx: *mut wah_call_context_t);
    pub fn wah_exec_userdata(ctx: *const wah_call_context_t) -> *mut c_void;

    pub fn wah_link_module(
        ctx: *mut wah_exec_context_t,
        name: *const c_char,
        module: *const wah_module_t,
    ) -> wah_error_t;
    pub fn wah_link_context(
        ctx: *mut wah_exec_context_t,
        name: *const c_char,
        linked_ctx: *mut wah_exec_context_t,
    ) -> wah_error_t;
    pub fn wah_instantiate(ctx: *mut wah_exec_context_t) -> wah_error_t;

    pub fn wah_memory_data(
        ctx: *mut wah_exec_context_t,
        mem_idx: u32,
        data: *mut *mut u8,
        size: *mut u64,
    ) -> wah_error_t;
    pub fn wah_set_memory_grow_hook(
        ctx: *mut wah_exec_context_t,
        hook: wah_memory_grow_hook_t,
        userdata: *mut c_void,
    ) -> wah_error_t;

    pub fn wah_gc_start(ctx: *mut wah_exec_context_t) -> wah_error_t;
    pub fn wah_gc_heap_stats(ctx: *const wah_exec_context_t, stats: *mut wah_gc_heap_stats_t);
    pub fn wah_gc_heap_stats_from_host(ctx: *const wah_call_context_t, stats: *mut wah_gc_heap_stats_t);
    pub fn wah_gc_verify_heap(ctx: *const wah_exec_context_t) -> bool;
    pub fn wah_gc_alloc_host(ctx: *mut wah_exec_context_t, size: usize) -> *mut c_void;
    pub fn wah_pin_ref(
        ctx: *mut wah_exec_context_t,
        host_ref: *mut c_void,
        pinned_ref: *mut *mut c_void,
    ) -> wah_error_t;

    pub fn wah_request_interrupt(ctx: *mut wah_exec_context_t);
    pub fn wah_request_interrupt_from_host(ctx: *mut wah_call_context_t);
    pub fn wah_is_interrupted(ctx: *const wah_exec_context_t) -> bool;
}

#[cfg(test)]
mod tests {
    extern crate std;
    use super::*;
    use core::mem::{align_of, offset_of, size_of};

    unsafe extern "C" {
        static wah_sys_layout: [usize; 0];
        static wah_sys_layout_len: usize;
        static wah_sys_compiled_features: u64;
    }

    #[test]
    fn compiled_features_match_c() {
        assert_eq!(unsafe { wah_sys_compiled_features } | WAH_FEATURE_MVP, WAH_COMPILED_FEATURES);
    }

    macro_rules! layout {
        ($t:ty) => {
            [size_of::<$t>(), align_of::<$t>()]
        };
    }

    #[test]
    fn layouts_match_c() {
        let rust: &[&[usize]] = &[
            &layout!(wah_module_t),
            &layout!(wah_exec_context_t),
            &layout!(wah_value_t),
            &layout!(wah_v128_t),
            &layout!(wah_alloc_t),
            &layout!(wah_type_desc_t),
            &[offset_of!(wah_type_desc_t, field_mutables)],
            &layout!(wah_func_desc_t),
            &[offset_of!(wah_func_desc_t, result_types)],
            &layout!(wah_global_desc_t),
            &layout!(wah_memory_desc_t),
            &layout!(wah_table_desc_t),
            &layout!(wah_tag_desc_t),
            &layout!(wah_extern_desc_t),
            &layout!(wah_import_desc_t),
            &[offset_of!(wah_import_desc_t, name_len), offset_of!(wah_import_desc_t, u)],
            &layout!(wah_export_desc_t),
            &[offset_of!(wah_export_desc_t, u)],
            &layout!(wah_parse_options_t),
            &[offset_of!(wah_parse_options_t, disable_jit)],
            &layout!(wah_limits_t),
            &[offset_of!(wah_limits_t, no_memory_bytes)],
            &layout!(wah_exec_options_t),
            &[offset_of!(wah_exec_options_t, memory_creator), offset_of!(wah_exec_options_t, userdata)],
            &layout!(wah_memory_request_t),
            &[offset_of!(wah_memory_request_t, is_64), offset_of!(wah_memory_request_t, reserve_bytes)],
            &layout!(wah_host_memory_t),
            &[offset_of!(wah_host_memory_t, guarded)],
            &layout!(wah_memory_creator_t),
            &layout!(wah_gc_heap_stats_t),
            &[offset_of!(wah_gc_heap_stats_t, total_polls)],
            &[size_of::<wah_error_t>(), size_of::<wah_exec_state_t>(), size_of::<wah_comp_type_kind_t>()],
            &[size_of::<wah_fault_handling_t>(), size_of::<wah_gc_phase_t>()],
        ];
        let rust: std::vec::Vec<usize> = rust.iter().flat_map(|s| s.iter().copied()).collect();
        let c = unsafe { core::slice::from_raw_parts(wah_sys_layout.as_ptr(), wah_sys_layout_len) };
        assert_eq!(rust, c);
    }

    #[test]
    fn parses_and_calls() {
        // (func (export "add") (param i32 i32) (result i32) local.get 0 local.get 1 i32.add)
        const ADD: &[u8] = b"\0asm\x01\0\0\0\x01\x07\x01\x60\x02\x7f\x7f\x01\x7f\x03\x02\x01\0\
            \x07\x07\x01\x03add\0\0\x0a\x09\x01\x07\0\x20\0\x20\x01\x6a\x0b";
        unsafe {
            let mut m = std::boxed::Box::new(wah_module_t::ZERO);
            assert_eq!(wah_parse_module(&mut *m, ADD.as_ptr(), ADD.len(), core::ptr::null()), WAH_OK);
            let mut ctx = std::boxed::Box::new(wah_exec_context_t::ZERO);
            assert_eq!(wah_new_exec_context(&mut *ctx, &*m, core::ptr::null()), WAH_OK);
            let params = [wah_value_t { i32: 40 }, wah_value_t { i32: 2 }];
            let mut result = wah_value_t::ZERO;
            assert_eq!(wah_call_by_name(&mut *ctx, c"add".as_ptr(), params.as_ptr(), 2, &mut result), WAH_OK);
            assert_eq!(result.i32, 42);
            wah_free_exec_context(&mut *ctx);
            wah_free_module(&mut *m);
        }
    }
}
