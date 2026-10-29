// CBMC proofs of scalar floating-point operations: for every operand, they agree bit for bit with the spec
// (exec/numerics.html) under the deterministic profile, where every NaN result is the positive canonical NaN.
// The references round with CBMC's own __CPROVER_round_to_integral, while the code under proof uses CBMC's models of
// libm (truncf, roundf, ...), so libm is assumed to be correct. __builtin_roundeven[f] paths aren't covered.
#include "proofs.h"

// Rounding modes of CBMC, which differ from FE_* of the platform
#define ROUND_TO_EVEN 0
#define ROUND_TO_ZERO 3

#ifndef __CPROVER__
float __CPROVER_round_to_integralf(float f, int rounding_mode);
double __CPROVER_round_to_integrald(double f, int rounding_mode);
#endif

float nondet_float(void);
double nondet_double(void);

static uint32_t bits32(float f) { union { float f; uint32_t u; } x = {f}; return x.u; }
static uint64_t bits64(double f) { union { double f; uint64_t u; } x = {f}; return x.u; }

// fmin/fmax: NaN if either is, otherwise the smaller/larger one where -0 < +0
#define MINMAX_HARNESS(N, fty, nondet, canon, min_fn, max_fn) \
    void harness_min_f##N(void) { \
        fty a = nondet(), b = nondet(); \
        fty expected = isnan(a) || isnan(b) ? (canon) : a == b ? (signbit(a) ? a : b) : a < b ? a : b; \
        __CPROVER_assert(bits##N(min_fn(a, b)) == bits##N(expected), "fmin"); \
    } \
    void harness_max_f##N(void) { \
        fty a = nondet(), b = nondet(); \
        fty expected = isnan(a) || isnan(b) ? (canon) : a == b ? (signbit(a) ? b : a) : a > b ? a : b; \
        __CPROVER_assert(bits##N(max_fn(a, b)) == bits##N(expected), "fmax"); \
    }

MINMAX_HARNESS(32, float, nondet_float, WAH_CANONICAL_NAN32.f, wah_minf, wah_maxf)
MINMAX_HARNESS(64, double, nondet_double, WAH_CANONICAL_NAN64.f, wah_min, wah_max)

// fnearest: the integral value nearest to the operand with ties to even, keeping the sign of zeros
void harness_nearest_f32(void) {
    float f = nondet_float();
    float expected = isnan(f) ? WAH_CANONICAL_NAN32.f : __CPROVER_round_to_integralf(f, ROUND_TO_EVEN);
    __CPROVER_assert(bits32(wah_nearest_f32(f)) == bits32(expected), "fnearest");
}

void harness_nearest_f64(void) {
    double f = nondet_double();
    double expected = isnan(f) ? WAH_CANONICAL_NAN64.f : __CPROVER_round_to_integrald(f, ROUND_TO_EVEN);
    __CPROVER_assert(bits64(wah_nearest_f64(f)) == bits64(expected), "fnearest");
}

// CBMC wrongly reports converting -2^(N-1) to intN_t as overflow when -2^(N-1)-1 rounds to it, so such operands are
// left to float_boundary.c, which proves the same without checking conversions.
#ifdef UNCHECKED_CONVERSIONS
#define ASSUME_CHECKABLE(fty, t, lo) (void)0
#else
#define ASSUME_CHECKABLE(fty, t, lo) __CPROVER_assume(t != (lo) || (fty)(lo) - 1 != (fty)(lo))
#endif

// trunc_u/trunc_s trap unless the operand truncates to an integer in the range [lo, hi), and trunc_sat_u/trunc_sat_s
// saturate to the range instead (0 for NaNs). The bounds are powers of two, exact in both formats.
#define TRUNC_HARNESS(N, fty, nondet, round, T, ity, lo, hi, min_val, max_val) \
    void harness_trunc_f##N##_to_##T(void) { \
        fty f = nondet(); \
        fty t = round(f, ROUND_TO_ZERO); \
        ASSUME_CHECKABLE(fty, t, lo); \
        bool ok = isfinite(f) && t >= (lo) && t < (hi); \
        ity got = 0; \
        wah_error_t err = wah_trunc_f##N##_to_##T(f, &got); \
        __CPROVER_assert(err == (ok ? WAH_OK : WAH_ERROR_TRAP), "traps exactly out of range"); \
        if (ok) __CPROVER_assert(got == (ity)t, "trunc"); \
    } \
    void harness_trunc_sat_f##N##_to_##T(void) { \
        fty f = nondet(); \
        fty t = round(f, ROUND_TO_ZERO); \
        ASSUME_CHECKABLE(fty, t, lo); \
        ity expected = isnan(f) ? 0 : t < (lo) ? (min_val) : t >= (hi) ? (max_val) : (ity)t; \
        __CPROVER_assert(wah_trunc_sat_f##N##_to_##T(f) == expected, "trunc_sat"); \
    }

TRUNC_HARNESS(32, float, nondet_float, __CPROVER_round_to_integralf, i32, int32_t, -0x1p31, 0x1p31, INT32_MIN, INT32_MAX)
TRUNC_HARNESS(32, float, nondet_float, __CPROVER_round_to_integralf, u32, uint32_t, 0, 0x1p32, 0, UINT32_MAX)
TRUNC_HARNESS(32, float, nondet_float, __CPROVER_round_to_integralf, i64, int64_t, -0x1p63, 0x1p63, INT64_MIN, INT64_MAX)
TRUNC_HARNESS(32, float, nondet_float, __CPROVER_round_to_integralf, u64, uint64_t, 0, 0x1p64, 0, UINT64_MAX)
TRUNC_HARNESS(64, double, nondet_double, __CPROVER_round_to_integrald, i32, int32_t, -0x1p31, 0x1p31, INT32_MIN, INT32_MAX)
TRUNC_HARNESS(64, double, nondet_double, __CPROVER_round_to_integrald, u32, uint32_t, 0, 0x1p32, 0, UINT32_MAX)
TRUNC_HARNESS(64, double, nondet_double, __CPROVER_round_to_integrald, i64, int64_t, -0x1p63, 0x1p63, INT64_MIN, INT64_MAX)
TRUNC_HARNESS(64, double, nondet_double, __CPROVER_round_to_integrald, u64, uint64_t, 0, 0x1p64, 0, UINT64_MAX)
