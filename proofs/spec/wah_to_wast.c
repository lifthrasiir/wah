// Writes what WAH does with a module as a .wast script, which the reference interpreter of the spec checks
// (`make spec-diff`). Modules that WAH rejects become assert_malformed or assert_invalid, and modules that it accepts
// are instantiated and their exported functions are called in order, first with zero arguments and then for some more
// rounds with pseudo-random ones, each call becoming assert_return, assert_trap or assert_exception. Then the final
// values of exported globals and the sizes and hashes of exported memories are checked, the latter by modules
// importing them. Anything that depends on the limits of WAH (fuel, memory, stack) ends the script, as the spec has no
// such limits. Traps are not distinguished, as WAH doesn't tell them apart.
//
// Usage: wah_to_wast <in.wasm> <out.wast>; exits with 0 if a script was written, 1 if the module was skipped.
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define WAH_IMPLEMENTATION
#ifdef WAH_TO_WAST_HEADER // A variant from nan_variant.sh
#include WAH_TO_WAST_HEADER
#else
#include "../../wah.h"
#endif

#define CALL_FUEL 100000
#define MAX_BLOCK_BYTES ((size_t)512 << 20) // Linear memories and tables are single blocks
#define MAX_VALUES 64
#define ARG_ROUNDS 4 // The first one with zero arguments
#define MAX_HASHED_BYTES ((uint64_t)128 << 16) // The reference interpreter hashes 100 pages a second
#define HASH_INIT UINT64_C(0xcbf29ce484222325) // FNV-1a over 64-bit words
#define HASH_PRIME UINT64_C(0x100000001b3)

static FILE *out;
static bool grow_refused;

static void put_string(const uint8_t *s, size_t len) {
    fputc('"', out);
    for (size_t i = 0; i < len; ++i) {
        if (s[i] >= 0x20 && s[i] < 0x7f && s[i] != '"' && s[i] != '\\') fputc(s[i], out);
        else fprintf(out, "\\%02x", s[i]);
    }
    fputc('"', out);
}

static void put_module(const uint8_t *bytes, size_t len) {
    fputs("(module $M binary ", out);
    put_string(bytes, len);
    fputc(')', out);
}

static void put_f32(uint32_t bits, bool exact) {
    float f;
    memcpy(&f, &bits, 4);
    if ((bits & 0x7f800000) == 0x7f800000 && (bits & 0x7fffff)) {
        if ((bits & 0x400000) && !exact) fputs("nan:arithmetic", out); // The spec lets any arithmetic NaN come out
        else fprintf(out, "%snan:0x%" PRIx32, bits >> 31 ? "-" : "", bits & 0x7fffff);
    } else if (isinf(f)) {
        fputs(f < 0 ? "-inf" : "inf", out);
    } else {
        fprintf(out, "%a", (double)f);
    }
}

static void put_f64(uint64_t bits, bool exact) {
    double f;
    memcpy(&f, &bits, 8);
    if ((bits & UINT64_C(0x7ff0000000000000)) == UINT64_C(0x7ff0000000000000) && (bits & UINT64_C(0xfffffffffffff))) {
        if ((bits & UINT64_C(0x8000000000000)) && !exact) fputs("nan:arithmetic", out);
        else fprintf(out, "%snan:0x%" PRIx64, bits >> 63 ? "-" : "", bits & UINT64_C(0xfffffffffffff));
    } else if (isinf(f)) {
        fputs(f < 0 ? "-inf" : "inf", out);
    } else {
        fprintf(out, "%a", f);
    }
}

// The most general pattern of non-null references of type t
static const char *ref_pattern(const wah_module_t *m, wah_type_t t) {
    t = WAH_TYPE_AS_NON_NULL(t);
    if (t >= 0) {
        switch (m->type_defs[WAH_TYIDX(t)].kind) {
            case WAH_COMP_FUNC: return "(ref.func)";
            case WAH_COMP_STRUCT: return "(ref.struct)";
            case WAH_COMP_ARRAY: return "(ref.array)";
        }
        return NULL;
    }
    switch (t) {
        case WAH_TYPE_FUNC: return "(ref.func)";
        case WAH_TYPE_EXTERN: return "(ref.extern)";
        case WAH_TYPE_ANY: return "(ref)";
        case WAH_TYPE_EQ: return "(ref.eq)";
        case WAH_TYPE_I31: return "(ref.i31)";
        case WAH_TYPE_STRUCT: return "(ref.struct)";
        case WAH_TYPE_ARRAY: return "(ref.array)";
        case WAH_TYPE_EXN: return "(ref.exn)";
    }
    return NULL;
}

// The null of the bottom type in the hierarchy of t, which can be passed for any nullable t
static const char *null_arg(const wah_module_t *m, wah_type_t t) {
    t = WAH_TYPE_AS_NON_NULL(t);
    if (t >= 0) return m->type_defs[WAH_TYIDX(t)].kind == WAH_COMP_FUNC ? "(ref.null nofunc)" : "(ref.null none)";
    switch (t) {
        case WAH_TYPE_FUNC: case WAH_TYPE_NOFUNC: return "(ref.null nofunc)";
        case WAH_TYPE_EXTERN: case WAH_TYPE_NOEXTERN: return "(ref.null noextern)";
        case WAH_TYPE_EXN: case WAH_TYPE_NOEXN: return "(ref.null noexn)";
        default: return "(ref.null none)";
    }
}

static uint64_t rng_state;
static uint64_t rng(void) { // splitmix64
    uint64_t z = (rng_state += UINT64_C(0x9e3779b97f4a7c15));
    z = (z ^ (z >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    z = (z ^ (z >> 27)) * UINT64_C(0x94d049bb133111eb);
    return z ^ (z >> 31);
}

// Integers near edges, or random bits of random widths
static uint64_t random_int(void) {
    static const uint64_t edges[] = { 0, 1, 2, 7, 8, 31, 32, 33, 63, 64, 0x7f, 0x80, 0xff, 0x7fff, 0x8000, 0xffff,
                                      0x7fffffff, 0x80000000, UINT64_C(0x7fffffffffffffff) };
    uint64_t r = rng();
    switch (r % 4) {
        case 0: return edges[(r >> 8) % (sizeof(edges) / sizeof(*edges))];
        case 1: return -edges[(r >> 8) % (sizeof(edges) / sizeof(*edges))];
        default: return rng() >> (r >> 8) % 64;
    }
}

// Floats that are not NaNs except the positive canonical one: the spec lets other NaNs come out of operations taking
// them (in a way that the reference interpreter takes from the hardware), and WAH canonicalizes them
static uint64_t random_f64(void) {
    static const double edges[] = { 0.0, -0.0, 1.0, -1.0, 0.5, -0.5, 1.5, 2.5, -2.5, 1e300, -1e300, 4.9e-324,
                                    2.2250738585072014e-308, 4294967296.0, -2147483649.0, 9.2233720368547758e18,
                                    INFINITY, -INFINITY, NAN };
    uint64_t r = rng(), bits;
    double d = r % 2 ? edges[(r >> 8) % (sizeof(edges) / sizeof(*edges))] : ldexp((double)(int64_t)rng(), -(int)((r >> 8) % 64));
    if (isnan(d)) return UINT64_C(0x7ff8000000000000);
    memcpy(&bits, &d, 8);
    return bits;
}
static uint32_t random_f32(void) {
    double d;
    uint64_t bits = random_f64();
    memcpy(&d, &bits, 8);
    float f = (float)d;
    if (isnan(f)) return 0x7fc00000;
    uint32_t b;
    memcpy(&b, &f, 4);
    return b;
}

// Arguments of round 0 are zeros (or nulls); references are always nulls, as there are no others to pass
static bool make_arg(wah_type_t t, uint32_t round, wah_value_t *v) {
    memset(v, 0, sizeof(*v));
    if (WAH_TYPE_IS_REF(t)) return WAH_TYPE_IS_NULLABLE(t);
    if (round == 0) return true;
    switch (t) {
        case WAH_TYPE_I32: v->i32 = (int32_t)random_int(); break;
        case WAH_TYPE_I64: v->i64 = (int64_t)random_int(); break;
        case WAH_TYPE_F32: { uint32_t b = random_f32(); memcpy(&v->f32, &b, 4); break; }
        case WAH_TYPE_F64: { uint64_t b = random_f64(); memcpy(&v->f64, &b, 8); break; }
        case WAH_TYPE_V128: {
            uint32_t lanes[4];
            for (int i = 0; i < 4; ++i) lanes[i] = (uint32_t)random_int() & ~UINT32_C(0x40000000); // No NaNs
            memcpy(&v->v128, lanes, 16);
            break;
        }
    }
    return true;
}

static void put_arg(const wah_module_t *m, wah_type_t t, const wah_value_t *v) {
    switch (t) {
        case WAH_TYPE_I32: fprintf(out, " (i32.const %" PRIu32 ")", (uint32_t)v->i32); return;
        case WAH_TYPE_I64: fprintf(out, " (i64.const %" PRIu64 ")", (uint64_t)v->i64); return;
        case WAH_TYPE_F32: {
            uint32_t bits;
            memcpy(&bits, &v->f32, 4);
            fputs(" (f32.const ", out), put_f32(bits, true), fputc(')', out);
            return;
        }
        case WAH_TYPE_F64: {
            uint64_t bits;
            memcpy(&bits, &v->f64, 8);
            fputs(" (f64.const ", out), put_f64(bits, true), fputc(')', out);
            return;
        }
        case WAH_TYPE_V128: {
            uint32_t lanes[4];
            memcpy(lanes, &v->v128, 16);
            fprintf(out, " (v128.const i32x4 0x%" PRIx32 " 0x%" PRIx32 " 0x%" PRIx32 " 0x%" PRIx32 ")",
                    lanes[0], lanes[1], lanes[2], lanes[3]);
            return;
        }
    }
    fprintf(out, " %s", null_arg(m, t));
}

static bool put_result(const wah_module_t *m, wah_type_t t, const wah_value_t *v) {
    switch (t) {
        case WAH_TYPE_I32: fprintf(out, " (i32.const %" PRIu32 ")", (uint32_t)v->i32); return true;
        case WAH_TYPE_I64: fprintf(out, " (i64.const %" PRIu64 ")", (uint64_t)v->i64); return true;
        case WAH_TYPE_F32: {
            uint32_t bits;
            memcpy(&bits, &v->f32, 4);
            fputs(" (f32.const ", out), put_f32(bits, false), fputc(')', out);
            return true;
        }
        case WAH_TYPE_F64: {
            uint64_t bits;
            memcpy(&bits, &v->f64, 8);
            fputs(" (f64.const ", out), put_f64(bits, false), fputc(')', out);
            return true;
        }
        case WAH_TYPE_V128: {
            uint32_t lanes[4];
            memcpy(lanes, &v->v128, 16);
            bool nan = false;
            for (int i = 0; i < 4; ++i) nan |= (lanes[i] & 0x7f800000) == 0x7f800000 && (lanes[i] & 0x7fffff);
            if (!nan) {
                fprintf(out, " (v128.const i32x4 0x%" PRIx32 " 0x%" PRIx32 " 0x%" PRIx32 " 0x%" PRIx32 ")",
                        lanes[0], lanes[1], lanes[2], lanes[3]);
            } else { // Probably float lanes, with any NaN allowed
                fputs(" (v128.const f32x4", out);
                for (int i = 0; i < 4; ++i) fputc(' ', out), put_f32(lanes[i], false);
                fputc(')', out);
            }
            return true;
        }
    }
    if (!WAH_TYPE_IS_REF(t)) return false;
    if (!v->ref) {
        fputs(" (ref.null)", out);
        return true;
    }
    const char *pattern = ref_pattern(m, t);
    if (!pattern) return false;
    fprintf(out, " %s", pattern);
    return true;
}

// Allocations of execution, which refuse large blocks, as growing memories or tables beyond them makes memory.grow
// and table.grow return -1, which the spec does only if the host refuses
static void *exec_malloc(size_t size, void *userdata) {
    (void)userdata;
    if (size > MAX_BLOCK_BYTES) { grow_refused = true; return NULL; }
    return malloc(size);
}
static void *exec_realloc(void *ptr, size_t size, void *userdata) {
    (void)userdata;
    if (size > MAX_BLOCK_BYTES) { grow_refused = true; return NULL; }
    return realloc(ptr, size);
}
static void exec_free(void *ptr, void *userdata) { (void)userdata; free(ptr); }

static bool is_malformed(wah_error_t err) {
    return err == WAH_ERROR_INVALID_MAGIC_NUMBER || err == WAH_ERROR_INVALID_VERSION ||
           err == WAH_ERROR_UNEXPECTED_EOF || err == WAH_ERROR_MALFORMED;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s <in.wasm> <out.wast>\n", argv[0]);
        return 2;
    }
    FILE *in = fopen(argv[1], "rb");
    if (!in) return 2;
    static uint8_t bytes[16 << 20];
    size_t len = fread(bytes, 1, sizeof(bytes), in);
    fclose(in);
    if (len == sizeof(bytes)) return 1;

    wah_module_t module = {0};
    wah_parse_options_t parse_options = { .features = WAH_FEATURE_WASM_V3, .enable_fuel_metering = true };
    wah_error_t err = wah_parse_module(&module, len ? bytes : (const uint8_t *)"", len, &parse_options);
    if (err != WAH_OK) {
        wah_free_module(&module);
        if (!is_malformed(err) && err != WAH_ERROR_VALIDATION_FAILED) return 1; // Limits or disabled features
        out = fopen(argv[2], "w");
        if (!out) return 2;
        fputs(is_malformed(err) ? "(assert_malformed " : "(assert_invalid ", out);
        put_module(bytes, len);
        fprintf(out, " \"\") ;; %s\n", wah_strerror(err));
        fclose(out);
        return 0;
    }
    if (wah_module_import_count(&module) > 0) { // No imports to link with
        wah_free_module(&module);
        return 1;
    }

    int status = 1;
    wah_exec_context_t ctx = {0};
    wah_alloc_t exec_alloc = { .malloc = exec_malloc, .realloc = exec_realloc, .free = exec_free };
    wah_exec_options_t exec_options = { .alloc = &exec_alloc, .limits = { .fuel = CALL_FUEL, .max_memory_bytes = UINT64_MAX } };
    if (wah_new_exec_context(&ctx, &module, &exec_options) != WAH_OK) goto done;
    err = wah_instantiate(&ctx);
    if (err == WAH_ERROR_TRAP || err == WAH_ERROR_MEMORY_OUT_OF_BOUNDS) {
        out = fopen(argv[2], "w");
        if (!out) goto done;
        fputs("(assert_trap ", out);
        put_module(bytes, len);
        fputs(" \"\")\n", out);
        fclose(out);
        status = 0;
        goto done;
    }
    if (err != WAH_OK || grow_refused) goto done;

    out = fopen(argv[2], "w");
    if (!out) goto done;
    put_module(bytes, len);
    fputc('\n', out);
    status = 0;
    bool stopped = false;
    for (uint32_t round = 0; round < ARG_ROUNDS && !stopped; ++round)
    for (uint32_t i = 0; i < wah_module_export_count(&module) && !stopped; i++) {
        wah_export_desc_t entry;
        if (wah_module_export(&module, i, &entry) != WAH_OK) { stopped = true; break; }
        if (entry.kind != WAH_KIND_FUNCTION) continue;
        const wah_func_desc_t *fn = &entry.u.func;
        if (fn->param_count > MAX_VALUES || fn->result_count > MAX_VALUES) continue;
        if (round > 0 && fn->param_count == 0) continue; // Repeating these is unlikely to be worth it
        wah_value_t params[MAX_VALUES], results[MAX_VALUES];
        bool ok = true;
        rng_state = (uint64_t)round << 32 | i;
        for (uint32_t p = 0; p < fn->param_count; ++p) ok &= make_arg(fn->param_types[p], round, &params[p]);
        if (!ok) continue;

        grow_refused = false;
        uint32_t actual = 0;
        if (wah_set_fuel(&ctx, CALL_FUEL) != WAH_OK) { stopped = true; break; }
        err = wah_call_multi(&ctx, entry.index, params, fn->param_count, results, MAX_VALUES, &actual);
        if (grow_refused) { fputs(";; Growing a memory or table was refused\n", out); stopped = true; break; }

        // The call, which can be printed only once its outcome is known
        char *call = NULL;
        size_t call_len = 0;
        FILE *saved = out;
        out = open_memstream(&call, &call_len);
        fputs("(invoke ", out);
        put_string((const uint8_t *)entry.name, entry.name_len);
        for (uint32_t p = 0; p < fn->param_count; ++p) put_arg(&module, fn->param_types[p], &params[p]);
        fputc(')', out);
        bool printable = true;
        if (err == WAH_OK) {
            fputs(" ", out);
            for (uint32_t r = 0; r < actual && printable; ++r) printable = put_result(&module, fn->result_types[r], &results[r]);
        }
        fclose(out);
        out = saved;
        if (err == WAH_OK && printable) {
            fprintf(out, "(assert_return %s)\n", call);
        } else if (err == WAH_ERROR_TRAP || err == WAH_ERROR_MEMORY_OUT_OF_BOUNDS) {
            fprintf(out, "(assert_trap %s \"\")\n", call);
        } else if (err == WAH_ERROR_EXCEPTION) {
            fprintf(out, "(assert_exception %s)\n", call);
        } else {
            fprintf(out, ";; %s: %s\n", call, err == WAH_OK ? "unprintable results" : wah_strerror(err));
            free(call);
            stopped = true; // The rest may depend on what the call did
            break;
        }
        free(call);
        if (err != WAH_OK) wah_cancel(&ctx); // Trapped activations are discarded, as in the spec
    }
    // Then the final values of exported globals, which are all defined as there are no imports
    for (uint32_t i = 0; i < wah_module_export_count(&module) && !stopped; i++) {
        wah_export_desc_t entry;
        if (wah_module_export(&module, i, &entry) != WAH_OK || entry.kind != WAH_KIND_GLOBAL) continue;
        char *line = NULL;
        size_t line_len = 0;
        FILE *saved = out;
        out = open_memstream(&line, &line_len);
        fputs("(assert_return (get ", out);
        put_string((const uint8_t *)entry.name, entry.name_len);
        fputc(')', out);
        bool printable = put_result(&module, entry.u.global.type, &ctx.globals[entry.index]);
        fputs(")\n", out);
        fclose(out);
        out = saved;
        if (printable) fputs(line, out);
        free(line);
    }
    // Then the sizes and contents of exported memories, through modules importing them
    bool registered = false;
    for (uint32_t i = 0; i < wah_module_export_count(&module) && !stopped; i++) {
        wah_export_desc_t entry;
        uint8_t *data;
        uint64_t size;
        if (wah_module_export(&module, i, &entry) != WAH_OK || entry.kind != WAH_KIND_MEMORY) continue;
        if (wah_memory_data(&ctx, entry.index, &data, &size) != WAH_OK) continue;
        if (!registered) fputs("(register \"M\" $M)\n", out), registered = true;
        bool m64 = entry.u.memory.addr_type == WAH_TYPE_I64;
        const char *a = m64 ? "i64" : "i32";
        fputs("(module (import \"M\" ", out);
        put_string((const uint8_t *)entry.name, entry.name_len);
        fprintf(out, " (memory %s 0))\n", a);
        fprintf(out, "  (func (export \"size\") (result i64) (%s))\n",
                m64 ? "memory.size" : "i64.extend_i32_u (memory.size)");
        fprintf(out, "  (func (export \"hash\") (result i64) (local $i %s) (local $h i64)\n"
                     "    (local.set $h (i64.const 0x%" PRIx64 "))\n"
                     "    (block (loop (br_if 1 (%s.ge_u (local.get $i) (%s.shl (memory.size) (%s.const 16))))\n"
                     "      (local.set $h (i64.mul (i64.xor (local.get $h) (i64.load (local.get $i))) (i64.const 0x%" PRIx64 ")))\n"
                     "      (local.set $i (%s.add (local.get $i) (%s.const 8))) (br 0)))\n"
                     "    (local.get $h)))\n",
                a, HASH_INIT, a, a, a, HASH_PRIME, a, a);
        fprintf(out, "(assert_return (invoke \"size\") (i64.const %" PRIu64 "))\n", size >> 16);
        if (size > MAX_HASHED_BYTES) continue;
        uint64_t h = HASH_INIT;
        for (uint64_t off = 0; off < size; off += 8) {
            uint64_t word = 0;
            for (int b = 0; b < 8; ++b) word |= (uint64_t)data[off + b] << 8 * b;
            h = (h ^ word) * HASH_PRIME;
        }
        fprintf(out, "(assert_return (invoke \"hash\") (i64.const %" PRIu64 "))\n", h);
    }
    fclose(out);

done:
    wah_free_exec_context(&ctx);
    wah_free_module(&module);
    return status;
}
