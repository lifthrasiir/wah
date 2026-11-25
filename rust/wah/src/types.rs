use std::fmt;
use std::sync::Arc;

use wah_sys as sys;

use crate::error::{Error, Result};

/// The type of a value.
#[derive(Clone, Copy, PartialEq, Eq, Hash)]
pub enum ValType {
    I32,
    I64,
    F32,
    F64,
    V128,
    /// A reference type, as WAH encodes it (`wah_type_t`: negative for the built-in ones, a type index of the module
    /// otherwise, odd if nullable). Values of reference types can't be passed through this crate yet.
    Ref(i32),
}

impl ValType {
    pub const FUNCREF: ValType = ValType::Ref(sys::WAH_TYPE_FUNCREF);
    pub const EXTERNREF: ValType = ValType::Ref(sys::WAH_TYPE_EXTERNREF);

    pub(crate) fn from_raw(t: sys::wah_type_t) -> ValType {
        match t {
            sys::WAH_TYPE_I32 => ValType::I32,
            sys::WAH_TYPE_I64 => ValType::I64,
            sys::WAH_TYPE_F32 => ValType::F32,
            sys::WAH_TYPE_F64 => ValType::F64,
            sys::WAH_TYPE_V128 => ValType::V128,
            t => ValType::Ref(t),
        }
    }

    pub fn is_num(self) -> bool {
        !matches!(self, ValType::Ref(_))
    }

    /// The name in the type syntax of `wah_export_func`, for the types host functions can have.
    pub(crate) fn spec(self) -> Option<&'static str> {
        Some(match self {
            ValType::I32 => "i32",
            ValType::I64 => "i64",
            ValType::F32 => "f32",
            ValType::F64 => "f64",
            ValType::V128 => "v128",
            ValType::Ref(_) => return None,
        })
    }
}

impl fmt::Debug for ValType {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            ValType::I32 => f.write_str("i32"),
            ValType::I64 => f.write_str("i64"),
            ValType::F32 => f.write_str("f32"),
            ValType::F64 => f.write_str("f64"),
            ValType::V128 => f.write_str("v128"),
            &ValType::Ref(sys::WAH_TYPE_FUNCREF) => f.write_str("funcref"),
            &ValType::Ref(sys::WAH_TYPE_EXTERNREF) => f.write_str("externref"),
            ValType::Ref(t) => write!(f, "ref({t})"),
        }
    }
}

/// A value of a numeric or vector type. Floats are kept as bits, so that NaNs keep their payloads.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum Val {
    I32(i32),
    I64(i64),
    F32(u32),
    F64(u64),
    V128(u128),
}

impl Val {
    pub fn ty(&self) -> ValType {
        match self {
            Val::I32(_) => ValType::I32,
            Val::I64(_) => ValType::I64,
            Val::F32(_) => ValType::F32,
            Val::F64(_) => ValType::F64,
            Val::V128(_) => ValType::V128,
        }
    }

    /// The zero of `ty`, `None` for reference types.
    pub fn default_for(ty: ValType) -> Option<Val> {
        Some(match ty {
            ValType::I32 => Val::I32(0),
            ValType::I64 => Val::I64(0),
            ValType::F32 => Val::F32(0),
            ValType::F64 => Val::F64(0),
            ValType::V128 => Val::V128(0),
            ValType::Ref(_) => return None,
        })
    }

    pub fn i32(&self) -> Option<i32> {
        if let Val::I32(v) = *self { Some(v) } else { None }
    }

    pub fn i64(&self) -> Option<i64> {
        if let Val::I64(v) = *self { Some(v) } else { None }
    }

    pub fn f32(&self) -> Option<f32> {
        if let Val::F32(v) = *self { Some(f32::from_bits(v)) } else { None }
    }

    pub fn f64(&self) -> Option<f64> {
        if let Val::F64(v) = *self { Some(f64::from_bits(v)) } else { None }
    }

    pub fn v128(&self) -> Option<u128> {
        if let Val::V128(v) = *self { Some(v) } else { None }
    }

    pub(crate) fn to_raw(self) -> sys::wah_value_t {
        let mut v = sys::wah_value_t::ZERO;
        match self {
            Val::I32(x) => v.i32 = x,
            Val::I64(x) => v.i64 = x,
            Val::F32(x) => v.i32 = x as i32,
            Val::F64(x) => v.i64 = x as i64,
            Val::V128(x) => v._zero = x.to_le_bytes(),
        }
        v
    }

    /// # Safety
    /// `v` should hold a value of `ty`, which is not a reference type.
    pub(crate) unsafe fn from_raw(v: &sys::wah_value_t, ty: ValType) -> Val {
        unsafe {
            match ty {
                ValType::I32 => Val::I32(v.i32),
                ValType::I64 => Val::I64(v.i64),
                ValType::F32 => Val::F32(v.i32 as u32),
                ValType::F64 => Val::F64(v.i64 as u64),
                ValType::V128 => Val::V128(u128::from_le_bytes(v._zero)),
                ValType::Ref(_) => unreachable!("a reference value"),
            }
        }
    }
}

impl From<i32> for Val {
    fn from(v: i32) -> Val {
        Val::I32(v)
    }
}

impl From<i64> for Val {
    fn from(v: i64) -> Val {
        Val::I64(v)
    }
}

impl From<f32> for Val {
    fn from(v: f32) -> Val {
        Val::F32(v.to_bits())
    }
}

impl From<f64> for Val {
    fn from(v: f64) -> Val {
        Val::F64(v.to_bits())
    }
}

impl From<u128> for Val {
    fn from(v: u128) -> Val {
        Val::V128(v)
    }
}

/// The type of a function.
#[derive(Clone, PartialEq, Eq, Hash)]
pub struct FuncType(Arc<(Box<[ValType]>, usize)>);

impl FuncType {
    pub fn new(params: impl IntoIterator<Item = ValType>, results: impl IntoIterator<Item = ValType>) -> FuncType {
        let mut types: Vec<ValType> = params.into_iter().collect();
        let n = types.len();
        types.extend(results);
        FuncType(Arc::new((types.into_boxed_slice(), n)))
    }

    pub fn params(&self) -> &[ValType] {
        &self.0.0[..self.0.1]
    }

    pub fn results(&self) -> &[ValType] {
        &self.0.0[self.0.1..]
    }

    /// # Safety
    /// The pointers should be valid for the counts.
    pub(crate) unsafe fn from_raw(params: *const i32, np: u32, results: *const i32, nr: u32) -> FuncType {
        let slice = |p: *const i32, n: u32| {
            if n == 0 { &[][..] } else { unsafe { std::slice::from_raw_parts(p, n as usize) } }
        };
        FuncType::new(
            slice(params, np).iter().map(|&t| ValType::from_raw(t)),
            slice(results, nr).iter().map(|&t| ValType::from_raw(t)),
        )
    }

    /// The type in the syntax of `wah_export_func`, if host functions can have it.
    pub(crate) fn spec(&self) -> Result<String> {
        let list = |ts: &[ValType]| -> Result<String> {
            let names: Option<Vec<&str>> = ts.iter().map(|t| t.spec()).collect();
            let names = names.ok_or_else(|| Error::msg("host functions of reference types are not supported"))?;
            Ok(names.join(", "))
        };
        Ok(format!("fn ({}) -> ({})", list(self.params())?, list(self.results())?))
    }
}

impl fmt::Debug for FuncType {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "fn {:?} -> {:?}", self.params(), self.results())
    }
}

/// The type of a linear memory, in pages of 64 KiB.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct MemoryType {
    memory64: bool,
    minimum: u64,
    maximum: Option<u64>,
}

/// The size of a page of linear memory.
pub const PAGE_SIZE: u64 = 65536;

impl MemoryType {
    /// A 32-bit memory.
    pub fn new(minimum: u32, maximum: Option<u32>) -> MemoryType {
        MemoryType { memory64: false, minimum: minimum.into(), maximum: maximum.map(Into::into) }
    }

    /// A 64-bit memory (memory64).
    pub fn new64(minimum: u64, maximum: Option<u64>) -> MemoryType {
        MemoryType { memory64: true, minimum, maximum }
    }

    pub fn is_64(&self) -> bool {
        self.memory64
    }

    pub fn minimum(&self) -> u64 {
        self.minimum
    }

    pub fn maximum(&self) -> Option<u64> {
        self.maximum
    }

    pub(crate) fn from_raw(d: &sys::wah_memory_desc_t) -> MemoryType {
        MemoryType {
            memory64: d.addr_type == sys::WAH_TYPE_I64,
            minimum: d.min_pages,
            maximum: (d.max_pages != u64::MAX).then_some(d.max_pages),
        }
    }
}

/// The type of a global.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct GlobalType {
    pub content: ValType,
    pub mutable: bool,
}

/// The type of a table.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct TableType {
    pub element: ValType,
    pub table64: bool,
    pub minimum: u64,
    pub maximum: Option<u64>,
}

/// The type of an import or an export.
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum ExternType {
    Func(FuncType),
    Memory(MemoryType),
    Global(GlobalType),
    Table(TableType),
    /// An exception tag, of its parameters.
    Tag(FuncType),
}

impl ExternType {
    /// # Safety
    /// `u` should be the description of `kind`, valid as WAH gave it.
    pub(crate) unsafe fn from_raw(kind: u8, u: &sys::wah_extern_desc_t) -> ExternType {
        unsafe {
            match kind {
                sys::WAH_KIND_FUNCTION => {
                    let f = &u.func;
                    ExternType::Func(FuncType::from_raw(f.param_types, f.param_count, f.result_types, f.result_count))
                }
                sys::WAH_KIND_MEMORY => ExternType::Memory(MemoryType::from_raw(&u.memory)),
                sys::WAH_KIND_GLOBAL => ExternType::Global(GlobalType {
                    content: ValType::from_raw(u.global.r#type),
                    mutable: u.global.is_mutable,
                }),
                sys::WAH_KIND_TABLE => {
                    let t = &u.table;
                    ExternType::Table(TableType {
                        element: ValType::from_raw(t.elem_type),
                        table64: t.addr_type == sys::WAH_TYPE_I64,
                        minimum: t.min_elements,
                        maximum: (t.max_elements != u64::MAX).then_some(t.max_elements),
                    })
                }
                _ => {
                    let t = &u.tag;
                    ExternType::Tag(FuncType::from_raw(t.param_types, t.param_count, std::ptr::null(), 0))
                }
            }
        }
    }
}

/// An import of a module.
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct ImportType {
    pub module: String,
    pub name: String,
    pub ty: ExternType,
}

/// An export of a module: its name, type, and index in the index space of its kind.
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct ExportType {
    pub name: String,
    pub ty: ExternType,
    pub index: u32,
}
