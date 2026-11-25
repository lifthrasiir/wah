// Builds the implementation of WAH from `wah.h` (or `wahjit.h` with the feature `jit`), found in `WAH_SRC_DIR` if set,
// else at the root of the WAH repository (two directories up) when built within it, else in `vendor/` (copied there
// by `rust/vendor.py` for packaging).

use std::env;
use std::fs;
use std::path::{Path, PathBuf};

const HEADERS: [&str; 2] = ["wah.h", "wahjit.h"];

/// The features of the crate for the proposals, by their bits in `wah_features_t` (as `WAH_COMPILED_FEATURES` in
/// `src/lib.rs`, which a test checks against the C).
const PROPOSALS: [(&str, u32); 15] = [
    ("MULTI_VALUE", 1),
    ("MUTABLE_GLOBALS", 2),
    ("SIGN_EXT", 3),
    ("NONTRAPPING_F2I", 4),
    ("BULK_MEMORY", 5),
    ("REF_TYPES", 6),
    ("SIMD", 7),
    ("TAIL_CALL", 8),
    ("EXCEPTION", 9),
    ("GC", 10),
    ("TYPED_FUNCREF", 11),
    ("MEMORY64", 12),
    ("RELAXED_SIMD", 13),
    ("EXTENDED_CONST", 14),
    ("MULTI_MEMORY", 15),
];

fn has_headers(dir: &Path) -> bool {
    HEADERS.iter().all(|f| dir.join(f).is_file())
}

/// The `WAH_VERSION` that `text` (of a header, or of the bindings) defines.
fn version(text: &str, prefix: &str) -> Option<i64> {
    let line = text.lines().find_map(|l| l.trim().strip_prefix(prefix))?;
    line.trim().trim_end_matches(';').trim().parse().ok()
}

fn main() {
    let manifest = PathBuf::from(env::var_os("CARGO_MANIFEST_DIR").unwrap());
    let repo = manifest.join("../..");
    let vendor = manifest.join("vendor");
    println!("cargo:rerun-if-env-changed=WAH_SRC_DIR");
    let src = if let Some(dir) = env::var_os("WAH_SRC_DIR") {
        PathBuf::from(dir)
    } else if has_headers(&repo) {
        if has_headers(&vendor) {
            for f in HEADERS {
                if fs::read(repo.join(f)).ok() != fs::read(vendor.join(f)).ok() {
                    println!("cargo:warning=vendor/{f} differs from the repository's; run rust/vendor.py again");
                }
                println!("cargo:rerun-if-changed={}", vendor.join(f).display());
            }
        }
        repo
    } else {
        vendor
    };
    if !has_headers(&src) {
        panic!("wah.h and wahjit.h not found in {} (set WAH_SRC_DIR, or run rust/vendor.py)", src.display());
    }
    for f in HEADERS {
        println!("cargo:rerun-if-changed={}", src.join(f).display());
    }
    println!("cargo:rerun-if-changed=src/lib.rs");
    println!("cargo:rerun-if-changed=src/wah_impl.c");
    println!("cargo:rerun-if-changed=src/layout.c");
    println!("cargo:include={}", src.display());

    // The bindings are written for one version of the API, which changes with any change of it.
    let header = fs::read_to_string(src.join("wah.h")).unwrap();
    let bindings = fs::read_to_string(manifest.join("src/lib.rs")).unwrap();
    let (h, b) = (version(&header, "#define WAH_VERSION"), version(&bindings, "pub const WAH_VERSION: i32 ="));
    if h.is_none() || h != b {
        panic!("wah.h is of WAH_VERSION {h:?}, but the bindings of {b:?}: update src/lib.rs for the API changed");
    }

    let mut b = cc::Build::new();
    b.include(&src).file("src/wah_impl.c").file("src/layout.c");
    if env::var_os("CARGO_FEATURE_JIT").is_some() {
        b.define("WAH_SYS_JIT", None);
    }
    let compiled = (PROPOSALS.iter())
        .filter(|(name, _)| env::var_os(format!("CARGO_FEATURE_{name}")).is_some())
        .fold(1u64, |bits, &(_, bit)| bits | 1 << bit);
    b.define("WAH_COMPILED_FEATURES", format!("{compiled}u").as_str());
    if env::var_os("CARGO_FEATURE_DEBUG").is_some() {
        b.define("WAH_DEBUG", None);
    }
    // The interpreter is unusably slow unoptimized (musttail dispatch at -O0 is ~15x slower than the switch).
    let opt: u32 = env::var("OPT_LEVEL").ok().and_then(|o| o.parse().ok()).unwrap_or(0);
    b.opt_level(opt.max(2));
    if b.get_compiler().is_like_msvc() {
        b.flag("/std:c11").flag("/wd5105").define("_CRT_SECURE_NO_WARNINGS", None);
    }
    b.warnings(false);
    b.compile("wah");
}
