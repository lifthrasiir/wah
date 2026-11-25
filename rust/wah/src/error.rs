use std::any::Any;
use std::error::Error as StdError;
use std::ffi::CStr;
use std::fmt;

use wah_sys as sys;

/// What WAH reported, by its error codes (`wah_error_t`).
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
#[non_exhaustive]
pub enum Code {
    InvalidMagicNumber,
    InvalidVersion,
    UnexpectedEof,
    Malformed,
    TooLarge,
    /// An allocation failed, or a memory budget was exceeded.
    OutOfMemory,
    ValidationFailed,
    /// A trap other than those below, e.g. `unreachable`, an integer division by zero or an indirect call of a
    /// wrong type.
    Trap,
    /// The call stack (`Store::set_max_stack_bytes`) was exhausted.
    StackOverflow,
    MemoryOutOfBounds,
    NotFound,
    Misuse,
    BadSpec,
    /// An import was missing or of an incompatible type.
    LinkFailed,
    /// An exception was thrown and not caught.
    Exception,
    DisabledFeature,
    MultiReturn,
    Unsupported,
    /// The fuel of the store ran out (`Store::set_fuel`).
    OutOfFuel,
    /// The deadline of the store passed (`Store::set_deadline`), or the execution was otherwise interrupted.
    Interrupted,
    /// A code unknown to this crate.
    Other(i32),
}

impl Code {
    pub(crate) fn from_raw(err: sys::wah_error_t) -> Code {
        match err {
            sys::WAH_ERROR_INVALID_MAGIC_NUMBER => Code::InvalidMagicNumber,
            sys::WAH_ERROR_INVALID_VERSION => Code::InvalidVersion,
            sys::WAH_ERROR_UNEXPECTED_EOF => Code::UnexpectedEof,
            sys::WAH_ERROR_MALFORMED => Code::Malformed,
            sys::WAH_ERROR_TOO_LARGE => Code::TooLarge,
            sys::WAH_ERROR_OUT_OF_MEMORY => Code::OutOfMemory,
            sys::WAH_ERROR_VALIDATION_FAILED => Code::ValidationFailed,
            sys::WAH_ERROR_TRAP => Code::Trap,
            sys::WAH_ERROR_STACK_OVERFLOW => Code::StackOverflow,
            sys::WAH_ERROR_MEMORY_OUT_OF_BOUNDS => Code::MemoryOutOfBounds,
            sys::WAH_ERROR_NOT_FOUND => Code::NotFound,
            sys::WAH_ERROR_MISUSE => Code::Misuse,
            sys::WAH_ERROR_BAD_SPEC => Code::BadSpec,
            sys::WAH_ERROR_LINK_FAILED => Code::LinkFailed,
            sys::WAH_ERROR_EXCEPTION => Code::Exception,
            sys::WAH_ERROR_DISABLED_FEATURE => Code::DisabledFeature,
            sys::WAH_ERROR_MULTI_RETURN => Code::MultiReturn,
            sys::WAH_ERROR_UNSUPPORTED => Code::Unsupported,
            sys::WAH_STATUS_FUEL_EXHAUSTED => Code::OutOfFuel,
            sys::WAH_STATUS_YIELDED => Code::Interrupted,
            other => Code::Other(other),
        }
    }

    /// Whether this is a trap of the execution (as opposed to a failure of parsing, linking or the API).
    pub fn is_trap(self) -> bool {
        matches!(
            self,
            Code::Trap
                | Code::StackOverflow
                | Code::MemoryOutOfBounds
                | Code::Exception
                | Code::OutOfFuel
                | Code::Interrupted
                | Code::OutOfMemory
        )
    }

    fn describe(self) -> String {
        let raw = match self {
            Code::OutOfFuel => return "fuel exhausted".into(),
            Code::Interrupted => return "execution interrupted".into(),
            Code::Other(c) => c,
            c => c.raw(),
        };
        // SAFETY: wah_strerror returns a static string for any code
        unsafe { CStr::from_ptr(sys::wah_strerror(raw)) }.to_string_lossy().into_owned()
    }

    fn raw(self) -> sys::wah_error_t {
        match self {
            Code::InvalidMagicNumber => sys::WAH_ERROR_INVALID_MAGIC_NUMBER,
            Code::InvalidVersion => sys::WAH_ERROR_INVALID_VERSION,
            Code::UnexpectedEof => sys::WAH_ERROR_UNEXPECTED_EOF,
            Code::Malformed => sys::WAH_ERROR_MALFORMED,
            Code::TooLarge => sys::WAH_ERROR_TOO_LARGE,
            Code::OutOfMemory => sys::WAH_ERROR_OUT_OF_MEMORY,
            Code::ValidationFailed => sys::WAH_ERROR_VALIDATION_FAILED,
            Code::Trap => sys::WAH_ERROR_TRAP,
            Code::StackOverflow => sys::WAH_ERROR_STACK_OVERFLOW,
            Code::MemoryOutOfBounds => sys::WAH_ERROR_MEMORY_OUT_OF_BOUNDS,
            Code::NotFound => sys::WAH_ERROR_NOT_FOUND,
            Code::Misuse => sys::WAH_ERROR_MISUSE,
            Code::BadSpec => sys::WAH_ERROR_BAD_SPEC,
            Code::LinkFailed => sys::WAH_ERROR_LINK_FAILED,
            Code::Exception => sys::WAH_ERROR_EXCEPTION,
            Code::DisabledFeature => sys::WAH_ERROR_DISABLED_FEATURE,
            Code::MultiReturn => sys::WAH_ERROR_MULTI_RETURN,
            Code::Unsupported => sys::WAH_ERROR_UNSUPPORTED,
            Code::OutOfFuel => sys::WAH_STATUS_FUEL_EXHAUSTED,
            Code::Interrupted => sys::WAH_STATUS_YIELDED,
            Code::Other(c) => c,
        }
    }
}

/// An error of WAH (with its [`Code`]), or one given by the host: returned by a host function or a
/// [`ResourceLimiter`](crate::ResourceLimiter), and passed through the execution unchanged.
pub struct Error(Box<Inner>);

enum Inner {
    Wah { code: Code, context: Option<String> },
    Host(Box<dyn StdError + Send + Sync>),
    Msg(String),
}

pub type Result<T, E = Error> = std::result::Result<T, E>;

impl Error {
    /// An error of the host, which can be told apart by [`Error::downcast_ref`].
    pub fn new<E: StdError + Send + Sync + 'static>(e: E) -> Error {
        Error(Box::new(Inner::Host(Box::new(e))))
    }

    /// An error of the host of a message alone.
    pub fn msg(msg: impl Into<String>) -> Error {
        Error(Box::new(Inner::Msg(msg.into())))
    }

    pub(crate) fn code(code: Code) -> Error {
        Error(Box::new(Inner::Wah { code, context: None }))
    }

    pub(crate) fn raw(err: sys::wah_error_t) -> Error {
        Error::code(Code::from_raw(err))
    }

    pub(crate) fn with_context(code: Code, context: impl Into<String>) -> Error {
        Error(Box::new(Inner::Wah { code, context: Some(context.into()) }))
    }

    pub(crate) fn misuse(context: impl Into<String>) -> Error {
        Error::with_context(Code::Misuse, context)
    }

    /// The code of an error of WAH, `None` for an error of the host.
    pub fn wah_code(&self) -> Option<Code> {
        match &*self.0 {
            Inner::Wah { code, .. } => Some(*code),
            _ => None,
        }
    }

    /// Whether this is a trap of the execution by WAH (see [`Code::is_trap`]).
    pub fn is_trap(&self) -> bool {
        self.wah_code().is_some_and(Code::is_trap)
    }

    pub fn is<E: StdError + 'static>(&self) -> bool {
        self.downcast_ref::<E>().is_some()
    }

    /// The error of the host given to [`Error::new`], if of type `E`.
    pub fn downcast_ref<E: StdError + 'static>(&self) -> Option<&E> {
        match &*self.0 {
            Inner::Host(e) => e.downcast_ref(),
            _ => None,
        }
    }

    /// The error of the host given to [`Error::new`], if of type `E`, or the error itself.
    pub fn downcast<E: StdError + 'static>(self) -> Result<E, Error> {
        match *self.0 {
            Inner::Host(e) if e.is::<E>() => Ok(*e.downcast().unwrap()),
            inner => Err(Error(Box::new(inner))),
        }
    }
}

impl fmt::Display for Error {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match &*self.0 {
            Inner::Wah { code, context: None } => f.write_str(&code.describe()),
            Inner::Wah { code, context: Some(c) } => write!(f, "{c}: {}", code.describe()),
            Inner::Host(e) => fmt::Display::fmt(e, f),
            Inner::Msg(m) => f.write_str(m),
        }
    }
}

impl fmt::Debug for Error {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match &*self.0 {
            Inner::Wah { code, context } => {
                f.debug_struct("Error").field("code", code).field("context", context).finish()
            }
            Inner::Host(e) => f.debug_tuple("Error").field(e).finish(),
            Inner::Msg(m) => f.debug_tuple("Error").field(m).finish(),
        }
    }
}

impl StdError for Error {
    fn source(&self) -> Option<&(dyn StdError + 'static)> {
        match &*self.0 {
            Inner::Host(e) => e.source(),
            _ => None,
        }
    }
}

/// Converts a status of WAH into a result.
pub(crate) fn check(err: sys::wah_error_t) -> Result<()> {
    if err == sys::WAH_OK { Ok(()) } else { Err(Error::raw(err)) }
}

/// What a host function or a limiter left in the store to propagate, once WAH returns.
pub(crate) enum Thrown {
    Error(Error),
    Panic(Box<dyn Any + Send>),
}
