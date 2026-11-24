#!/bin/sh
# Writes a copy of wah.h that makes NaNs as the reference interpreter does, which the spec allows: scalar operations
# return their first NaN operand made quiet (or the positive canonical NaN), and promote and demote conversions keep
# NaN payloads (which `wasm-tools smith --canonicalize-nans` doesn't canonicalize). Disagreements that this copy
# resolves are NaNs. SIMD lanes are still canonicalized.
# Usage: nan_variant.sh <wah.h> <out.h>
set -e
nan2='static inline float wah_ref_nan2_f32(float r, float a, float b) { union { float f; uint32_t u; } x = {a}, y = {b}; if (r == r) return r; if (a != a) { x.u |= 0x7fc00000U; return x.f; } if (b != b) { y.u |= 0x7fc00000U; return y.f; } return WAH_CANONICAL_NAN32.f; } static inline double wah_ref_nan2_f64(double r, double a, double b) { union { double f; uint64_t u; } x = {a}, y = {b}; if (r == r) return r; if (a != a) { x.u |= 0x7ff8000000000000ULL; return x.f; } if (b != b) { y.u |= 0x7ff8000000000000ULL; return y.f; } return WAH_CANONICAL_NAN64.f; }'
sed -e 's/sp\[-1\]\.f32 = wah_canonicalize_f32((float)sp\[-1\]\.f64);/sp[-1].f32 = (float)sp[-1].f64;/' \
    -e 's/sp\[-1\]\.f64 = wah_canonicalize_f64((double)sp\[-1\]\.f32);/sp[-1].f64 = (double)sp[-1].f32;/' \
    -e 's/{wah_canonicalize_f32((float)val\.f64\[0\]), wah_canonicalize_f32((float)val\.f64\[1\])}/{(float)val.f64[0], (float)val.f64[1]}/' \
    -e 's/{wah_canonicalize_f64((double)val\.f32\[0\]), wah_canonicalize_f64((double)val\.f32\[1\])}/{(double)val.f32[0], (double)val.f32[1]}/' \
    -e 's/wah_canonicalize_f32x4_sse2(_mm_cvtpd_ps(sp\[-1\]\._m128d))/_mm_cvtpd_ps(sp[-1]._m128d)/' \
    -e 's/wah_canonicalize_f64x2_sse2(_mm_cvtps_pd(sp\[-1\]\._m128))/_mm_cvtps_pd(sp[-1]._m128)/' \
    -e "s/^static inline double wah_canonicalize_f64(double val) { .* }\$/& $nan2/" \
    -e 's/wah_canonicalize_f##N(sp\[-2\]\.f##N op sp\[-1\]\.f##N)/wah_ref_nan2_f##N(sp[-2].f##N op sp[-1].f##N, sp[-2].f##N, sp[-1].f##N)/' \
    -e 's/wah_canonicalize_f##N(fn(sp\[-2\]\.f##N, sp\[-1\]\.f##N))/wah_ref_nan2_f##N(fn(sp[-2].f##N, sp[-1].f##N), sp[-2].f##N, sp[-1].f##N)/' \
    -e 's/wah_canonicalize_f##N(fn(sp\[-1\]\.f##N))/wah_ref_nan2_f##N(fn(sp[-1].f##N), sp[-1].f##N, sp[-1].f##N)/' \
    "$1" > "$2"
n=$(diff "$1" "$2" | grep -c '^>' || true)
[ "$n" -eq 10 ] || { echo "nan_variant.sh: patched $n lines instead of 10, update it for wah.h" >&2; rm -f "$2"; exit 1; }
