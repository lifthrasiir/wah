// Writes what WAH does with a module as a .wast script, which the reference interpreter of the spec checks
// (`make spec-diff`). Modules that WAH rejects become assert_malformed or assert_invalid, and modules that it accepts
// are instantiated and their exported functions are called in order with zero arguments, each becoming
// assert_return, assert_trap or assert_exception. Anything that depends on the limits of WAH (fuel, memory, stack)
// ends the script, as the spec has no such limits. Traps are not distinguished, as WAH doesn't tell them apart.
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
    fputs("(module binary ", out);
    put_string(bytes, len);
    fputc(')', out);
}

static void put_f32(uint32_t bits) {
    float f;
    memcpy(&f, &bits, 4);
    if ((bits & 0x7f800000) == 0x7f800000 && (bits & 0x7fffff)) {
        if (bits & 0x400000) fputs("nan:arithmetic", out); // The spec lets any arithmetic NaN come out
        else fprintf(out, "%snan:0x%" PRIx32, bits >> 31 ? "-" : "", bits & 0x7fffff);
    } else if (isinf(f)) {
        fputs(f < 0 ? "-inf" : "inf", out);
    } else {
        fprintf(out, "%a", (double)f);
    }
}

static void put_f64(uint64_t bits) {
    double f;
    memcpy(&f, &bits, 8);
    if ((bits & UINT64_C(0x7ff0000000000000)) == UINT64_C(0x7ff0000000000000) && (bits & UINT64_C(0xfffffffffffff))) {
        if (bits & UINT64_C(0x8000000000000)) fputs("nan:arithmetic", out);
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

static bool put_arg(const wah_module_t *m, wah_type_t t) {
    switch (t) {
        case WAH_TYPE_I32: fputs(" (i32.const 0)", out); return true;
        case WAH_TYPE_I64: fputs(" (i64.const 0)", out); return true;
        case WAH_TYPE_F32: fputs(" (f32.const 0)", out); return true;
        case WAH_TYPE_F64: fputs(" (f64.const 0)", out); return true;
        case WAH_TYPE_V128: fputs(" (v128.const i64x2 0 0)", out); return true;
    }
    if (!WAH_TYPE_IS_REF(t) || !WAH_TYPE_IS_NULLABLE(t)) return false; // No values of non-nullable references
    fprintf(out, " %s", null_arg(m, t));
    return true;
}

static bool zero_arg(wah_type_t t, wah_value_t *v) {
    memset(v, 0, sizeof(*v));
    return !WAH_TYPE_IS_REF(t) || WAH_TYPE_IS_NULLABLE(t);
}

static bool put_result(const wah_module_t *m, wah_type_t t, const wah_value_t *v) {
    switch (t) {
        case WAH_TYPE_I32: fprintf(out, " (i32.const %" PRIu32 ")", (uint32_t)v->i32); return true;
        case WAH_TYPE_I64: fprintf(out, " (i64.const %" PRIu64 ")", (uint64_t)v->i64); return true;
        case WAH_TYPE_F32: {
            uint32_t bits;
            memcpy(&bits, &v->f32, 4);
            fputs(" (f32.const ", out), put_f32(bits), fputc(')', out);
            return true;
        }
        case WAH_TYPE_F64: {
            uint64_t bits;
            memcpy(&bits, &v->f64, 8);
            fputs(" (f64.const ", out), put_f64(bits), fputc(')', out);
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
                for (int i = 0; i < 4; ++i) fputc(' ', out), put_f32(lanes[i]);
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
    for (uint32_t i = 0; i < wah_module_export_count(&module) && !stopped; i++) {
        wah_export_desc_t entry;
        if (wah_module_export(&module, i, &entry) != WAH_OK) { stopped = true; break; }
        if (entry.kind != WAH_KIND_FUNCTION) continue;
        const wah_func_desc_t *fn = &entry.u.func;
        if (fn->param_count > MAX_VALUES || fn->result_count > MAX_VALUES) continue;
        wah_value_t params[MAX_VALUES], results[MAX_VALUES];
        bool ok = true;
        for (uint32_t p = 0; p < fn->param_count; ++p) ok &= zero_arg(fn->param_types[p], &params[p]);
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
        for (uint32_t p = 0; p < fn->param_count; ++p) put_arg(&module, fn->param_types[p]);
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
    fclose(out);

done:
    wah_free_exec_context(&ctx);
    wah_free_module(&module);
    return status;
}
