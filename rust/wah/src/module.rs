use std::cell::UnsafeCell;
use std::ops::{BitAnd, BitOr, BitOrAssign, Not};
use std::sync::{Arc, Once};

use wah_sys as sys;

use crate::error::{Error, Result, check};
use crate::types::{ExportType, ExternType, ImportType};

/// A set of WebAssembly proposals (`wah_features_t`).
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct Features(u64);

impl Features {
    pub const MVP: Features = Features(sys::WAH_FEATURE_MVP);
    pub const MULTI_VALUE: Features = Features(sys::WAH_FEATURE_MULTI_VALUE);
    pub const MUTABLE_GLOBALS: Features = Features(sys::WAH_FEATURE_MUTABLE_GLOBALS);
    pub const SIGN_EXT: Features = Features(sys::WAH_FEATURE_SIGN_EXT);
    pub const NONTRAPPING_F2I: Features = Features(sys::WAH_FEATURE_NONTRAPPING_F2I);
    pub const BULK_MEMORY: Features = Features(sys::WAH_FEATURE_BULK_MEMORY);
    pub const REF_TYPES: Features = Features(sys::WAH_FEATURE_REF_TYPES);
    pub const SIMD: Features = Features(sys::WAH_FEATURE_SIMD);
    pub const TAIL_CALL: Features = Features(sys::WAH_FEATURE_TAIL_CALL);
    pub const EXCEPTION: Features = Features(sys::WAH_FEATURE_EXCEPTION);
    pub const GC: Features = Features(sys::WAH_FEATURE_GC);
    pub const TYPED_FUNCREF: Features = Features(sys::WAH_FEATURE_TYPED_FUNCREF);
    pub const MEMORY64: Features = Features(sys::WAH_FEATURE_MEMORY64);
    pub const RELAXED_SIMD: Features = Features(sys::WAH_FEATURE_RELAXED_SIMD);
    pub const EXTENDED_CONST: Features = Features(sys::WAH_FEATURE_EXTENDED_CONST);
    pub const MULTI_MEMORY: Features = Features(sys::WAH_FEATURE_MULTI_MEMORY);
    /// WebAssembly 2.0.
    pub const WASM2: Features = Features(sys::WAH_FEATURE_WASM_V2);
    /// WebAssembly 3.0.
    pub const WASM3: Features = Features(sys::WAH_FEATURE_WASM_V3);
    /// Every proposal WAH supports.
    pub const ALL: Features = Features(sys::WAH_FEATURE_ALL);
    /// The proposals compiled in, as the features of this crate (or of `wah-sys`) select them: modules using any
    /// other are rejected with [`Code::DisabledFeature`](crate::Code).
    pub const COMPILED: Features = Features(sys::WAH_COMPILED_FEATURES);

    pub const fn bits(self) -> u64 {
        self.0
    }

    pub const fn from_bits(bits: u64) -> Features {
        Features(bits | sys::WAH_FEATURE_MVP)
    }

    pub const fn contains(self, other: Features) -> bool {
        self.0 & other.0 == other.0
    }
}

impl BitOr for Features {
    type Output = Features;
    fn bitor(self, rhs: Features) -> Features {
        Features(self.0 | rhs.0)
    }
}

impl BitOrAssign for Features {
    fn bitor_assign(&mut self, rhs: Features) {
        self.0 |= rhs.0;
    }
}

impl BitAnd for Features {
    type Output = Features;
    fn bitand(self, rhs: Features) -> Features {
        Features(self.0 & rhs.0 | sys::WAH_FEATURE_MVP)
    }
}

impl Not for Features {
    type Output = Features;
    fn not(self) -> Features {
        Features(!self.0 | sys::WAH_FEATURE_MVP)
    }
}

/// How modules are parsed: the proposals enabled, fuel metering and native compilation.
///
/// WAH always follows the deterministic profile of WebAssembly 3.0 (canonical NaNs, no FP contraction).
#[derive(Debug, Clone)]
pub struct Config {
    features: Features,
    fuel: bool,
    jit: bool,
}

impl Default for Config {
    fn default() -> Config {
        Config::new()
    }
}

impl Config {
    /// Every proposal compiled in ([`Features::COMPILED`]), no fuel metering, native compilation where possible.
    pub fn new() -> Config {
        Config { features: Features::COMPILED, fuel: false, jit: true }
    }

    /// Enables `features` alone, of those compiled in: WAH leaves out the others (and enables the proposals that
    /// these need, e.g. reference types for GC).
    pub fn features(&mut self, features: Features) -> &mut Config {
        self.features = features | Features::MVP;
        self
    }

    /// The proposals enabled, of those compiled in.
    pub fn get_features(&self) -> Features {
        self.features & Features::COMPILED
    }

    /// Enables or disables `feature`.
    pub fn feature(&mut self, feature: Features, enable: bool) -> &mut Config {
        self.features = if enable { self.features | feature } else { self.features & !feature };
        self
    }

    /// Whether modules count fuel (`Store::set_fuel`). Fuel metering keeps modules from being compiled natively.
    pub fn consume_fuel(&mut self, enable: bool) -> &mut Config {
        self.fuel = enable;
        self
    }

    /// Whether eligible modules are compiled to native code (with the feature `jit` of this crate, on the platforms
    /// `wahjit.h` supports); they are interpreted otherwise. Either runs the same.
    pub fn jit(&mut self, enable: bool) -> &mut Config {
        self.jit = enable;
        self
    }
}

/// A module parsed, validated and compiled, which any number of stores can instantiate (from any thread).
/// Cloning it is cheap.
#[derive(Clone)]
pub struct Module(Arc<ModuleInner>);

pub(crate) struct ModuleInner {
    raw: Box<UnsafeCell<sys::wah_module_t>>,
    /// The first exec context of the module, which freezes it (the only write after parsing).
    frozen: Once,
    imports: Vec<ImportType>,
    exports: Vec<ExportType>,
    fuel: bool,
}

// SAFETY: WAH doesn't change a module after parsing, but for `frozen` (written once, under `ModuleInner::frozen`).
unsafe impl Send for ModuleInner {}
unsafe impl Sync for ModuleInner {}

impl Drop for ModuleInner {
    fn drop(&mut self) {
        // SAFETY: no exec context of the module remains, as each holds a `Module`
        unsafe { sys::wah_free_module(self.raw.get()) }
    }
}

/// A string WAH gave as a pointer and length.
///
/// # Safety
/// The pointer should be valid for the length.
pub(crate) unsafe fn string(p: *const std::ffi::c_char, len: usize) -> String {
    if len == 0 {
        return String::new();
    }
    String::from_utf8_lossy(unsafe { std::slice::from_raw_parts(p.cast::<u8>(), len) }).into_owned()
}

/// The imports and exports of `m`.
///
/// # Safety
/// `m` should be a valid module.
unsafe fn describe(m: *const sys::wah_module_t) -> Result<(Vec<ImportType>, Vec<ExportType>)> {
    // SAFETY: the descriptions are copied before the next call
    unsafe {
        let mut imports = Vec::new();
        for i in 0..sys::wah_module_import_count(m) {
            let mut d = std::mem::zeroed::<sys::wah_import_desc_t>();
            check(sys::wah_module_import(m, i, &mut d))?;
            imports.push(ImportType {
                module: string(d.module, d.module_len),
                name: string(d.name, d.name_len),
                ty: ExternType::from_raw(d.kind, &d.u),
            });
        }
        let mut exports = Vec::new();
        for i in 0..sys::wah_module_export_count(m) {
            let mut d = std::mem::zeroed::<sys::wah_export_desc_t>();
            check(sys::wah_module_export(m, i as usize, &mut d))?;
            exports.push(ExportType {
                name: string(d.name, d.name_len),
                ty: ExternType::from_raw(d.kind, &d.u),
                index: d.index,
            });
        }
        Ok((imports, exports))
    }
}

impl Module {
    /// Parses and validates `binary`.
    pub fn new(config: &Config, binary: &[u8]) -> Result<Module> {
        let raw = Box::new(UnsafeCell::new(sys::wah_module_t::ZERO));
        let opts = sys::wah_parse_options_t {
            features: config.features.0,
            enable_fuel_metering: config.fuel,
            alloc: std::ptr::null(),
            disable_jit: !config.jit,
        };
        // (freed by its drop on failure too, as WAH allows)
        let mut module =
            ModuleInner { raw, frozen: Once::new(), imports: Vec::new(), exports: Vec::new(), fuel: config.fuel };
        // SAFETY: the module is ours
        unsafe {
            check(sys::wah_parse_module(module.raw.get(), binary.as_ptr(), binary.len(), &opts))?;
            (module.imports, module.exports) = describe(module.raw.get())?;
        }
        Ok(Module(Arc::new(module)))
    }

    /// A module that the host built (`wah_new_module`), which its drop frees.
    ///
    /// # Safety
    /// `raw` should be a module built, and not changed any more.
    pub(crate) unsafe fn from_built(raw: Box<UnsafeCell<sys::wah_module_t>>) -> Result<Module> {
        let mut module =
            ModuleInner { raw, frozen: Once::new(), imports: Vec::new(), exports: Vec::new(), fuel: false };
        (module.imports, module.exports) = unsafe { describe(module.raw.get())? };
        Ok(Module(Arc::new(module)))
    }

    pub fn imports(&self) -> &[ImportType] {
        &self.0.imports
    }

    pub fn exports(&self) -> &[ExportType] {
        &self.0.exports
    }

    pub fn get_export(&self, name: &str) -> Option<&ExportType> {
        self.0.exports.iter().find(|e| e.name == name)
    }

    /// Whether the module was parsed with fuel metering.
    pub fn consumes_fuel(&self) -> bool {
        self.0.fuel
    }

    pub(crate) fn raw(&self) -> *const sys::wah_module_t {
        self.0.raw.get()
    }

    /// Initializes `ctx` for the module.
    ///
    /// # Safety
    /// `ctx` should be zeroed and stay where it is until freed, before the module is dropped.
    pub(crate) unsafe fn new_exec_context(
        &self,
        ctx: *mut sys::wah_exec_context_t,
        opts: &sys::wah_exec_options_t,
    ) -> Result<()> {
        let mut r = None;
        self.0.frozen.call_once(|| r = Some(unsafe { sys::wah_new_exec_context(ctx, self.raw(), opts) }));
        let r = r.unwrap_or_else(|| unsafe { sys::wah_new_exec_context(ctx, self.raw(), opts) });
        check(r).map_err(|e| Error::with_context(e.wah_code().unwrap(), "exec context not made"))
    }
}

impl std::fmt::Debug for Module {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("Module").field("imports", &self.0.imports).field("exports", &self.0.exports).finish()
    }
}
