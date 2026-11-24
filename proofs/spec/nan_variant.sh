#!/bin/sh
# Writes a copy of wah.h whose promote and demote conversions keep NaN payloads, which the spec allows but which
# `wasm-tools smith --canonicalize-nans` doesn't canonicalize. Disagreements that this copy resolves are NaNs.
# Usage: nan_variant.sh <wah.h> <out.h>
set -e
sed -e 's/sp\[-1\]\.f32 = wah_canonicalize_f32((float)sp\[-1\]\.f64);/sp[-1].f32 = (float)sp[-1].f64;/' \
    -e 's/sp\[-1\]\.f64 = wah_canonicalize_f64((double)sp\[-1\]\.f32);/sp[-1].f64 = (double)sp[-1].f32;/' \
    -e 's/{wah_canonicalize_f32((float)val\.f64\[0\]), wah_canonicalize_f32((float)val\.f64\[1\])}/{(float)val.f64[0], (float)val.f64[1]}/' \
    -e 's/{wah_canonicalize_f64((double)val\.f32\[0\]), wah_canonicalize_f64((double)val\.f32\[1\])}/{(double)val.f32[0], (double)val.f32[1]}/' \
    -e 's/wah_canonicalize_f32x4_sse2(_mm_cvtpd_ps(sp\[-1\]\._m128d))/_mm_cvtpd_ps(sp[-1]._m128d)/' \
    -e 's/wah_canonicalize_f64x2_sse2(_mm_cvtps_pd(sp\[-1\]\._m128))/_mm_cvtps_pd(sp[-1]._m128)/' "$1" > "$2"
n=$(diff "$1" "$2" | grep -c '^>' || true)
[ "$n" -eq 6 ] || { echo "nan_variant.sh: patched $n lines instead of 6, update it for wah.h" >&2; rm -f "$2"; exit 1; }
