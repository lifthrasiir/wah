#define WAH_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include "../wah.h"
#include "common.h"

static const char detect_bulk_memory[] = "wasm \
    types {[fn [] []]} funcs {[0]} memories {[limits.i32/1 1]} \
    code {[{[] i32.const 0 i32.const 0 i32.const 0 memory.copy 0 0 end}]}";

static const char detect_multi_value[] = "wasm \
    types {[fn [] [i32, i32]]} funcs {[0]} \
    code {[{[] i32.const 0 i32.const 0 end}]}";

static const char detect_mutable_globals[] = "wasm \
    imports {[{'a'} {'b'} global# i32 mut]} \
    globals {[i32 mut i32.const 0 end]} \
    exports {[{'a'} global# 0]}";

static const char detect_reference_types[] = "wasm \
    types {[fn [] []]} funcs {[0]} \
    code {[{[] ref.null funcref drop end}]}";

static const char detect_relaxed_simd[] = "wasm \
    types {[fn [] [v128]]} funcs {[0]} \
    code {[{[] i32.const 1 i8x16.splat i32.const 2 i8x16.splat i8x16.relaxed_swizzle end}]}";

static const char detect_nontrapping_f2i[] = "wasm \
    types {[fn [] []]} funcs {[0]} \
    code {[{[] f32.const 0.0f32 i32.trunc_sat_f32_s drop end}]}";

static const char detect_sign_ext[] = "wasm \
    types {[fn [] []]} funcs {[0]} \
    code {[{[] i32.const 0 i32.extend8_s drop end}]}";

static const char detect_simd[] = "wasm \
    types {[fn [] [v128]]} funcs {[0]} \
    code {[{[] i32.const 0 i8x16.splat i32.const 0 i8x16.splat i8x16.add end}]}";

static const char detect_tail_call[] = "wasm \
    types {[fn [] []]} funcs {[0]} \
    code {[{[] return_call 0 end}]}";

static const char detect_memory64[] = "wasm \
    types {[]} memories {[limits.i64/1 1]}";

static const char detect_exception[] = "wasm \
    types {[fn [] []]} funcs {[0]} \
    code {[{[] block exnref \
        try_table void [catch_all_ref 0 0] end unreachable end \
    drop end}]}";

static const char detect_gc[] = "wasm \
    types {[struct [i8 immut]]}";

static const char detect_typed_funcref[] = "wasm \
    types {[fn [i32] [i32], fn [type.ref 0] [i32], fn [] [i32]]} \
    funcs {[1, 0, 2]} \
    elements {[elem.declarative elem.funcref [1]]} \
    code {[ \
        {[] i32.const 10 i32.const 42 local.get 0 call_ref 0 i32.add end}, \
        {[] local.get 0 i32.const 1 i32.add end}, \
        {[] ref.func 1 call 0 end}, \
    ]}";

// --------------------------------------------------------------------------

struct feature_detect_test {
    const char *name;
    wah_features_t feature;
    wah_features_t also_disable; // additional features to disable (for dependency chains)
    const char *spec;
};

static const struct feature_detect_test detect_tests[] = {
    { .name = "bulk-memory",             .spec = detect_bulk_memory,     .feature = WAH_FEATURE_BULK_MEMORY },
    { .name = "multi-value",             .spec = detect_multi_value,     .feature = WAH_FEATURE_MULTI_VALUE },
    { .name = "mutable-globals",         .spec = detect_mutable_globals, .feature = WAH_FEATURE_MUTABLE_GLOBALS },
    { .name = "reference-types",         .spec = detect_reference_types, .feature = WAH_FEATURE_REF_TYPES,
                                                                         .also_disable = WAH_FEATURE_GC | WAH_FEATURE_TYPED_FUNCREF },
    { .name = "relaxed-simd",            .spec = detect_relaxed_simd,    .feature = WAH_FEATURE_RELAXED_SIMD },
    { .name = "saturated-float-to-int",  .spec = detect_nontrapping_f2i, .feature = WAH_FEATURE_NONTRAPPING_F2I },
    { .name = "sign-extensions",         .spec = detect_sign_ext,        .feature = WAH_FEATURE_SIGN_EXT },
    { .name = "simd",                    .spec = detect_simd,            .feature = WAH_FEATURE_SIMD,
                                                                         .also_disable = WAH_FEATURE_RELAXED_SIMD },
    { .name = "tail-call",               .spec = detect_tail_call,       .feature = WAH_FEATURE_TAIL_CALL },
    { .name = "memory64",                .spec = detect_memory64,        .feature = WAH_FEATURE_MEMORY64 },
    { .name = "exceptions-final",        .spec = detect_exception,       .feature = WAH_FEATURE_EXCEPTION },
    { .name = "gc",                      .spec = detect_gc,              .feature = WAH_FEATURE_GC },
    { .name = "typed-function-references", .spec = detect_typed_funcref, .feature = WAH_FEATURE_TYPED_FUNCREF },
};
#define NUM_DETECT_TESTS (sizeof(detect_tests) / sizeof(detect_tests[0]))

int main(void) {
    wah_module_t mod;
    wah_error_t err;

    // --- wasm-feature-detect: each proposal parses OK with all features ---
    for (size_t i = 0; i < NUM_DETECT_TESTS; i++) {
        const struct feature_detect_test *t = &detect_tests[i];
        printf("Testing %s: parses with all features...\n", t->name);
        err = wah_parse_module_from_spec_ex(&mod, NULL, t->spec);
        if (err != WAH_OK) {
            fprintf(stderr, "  FAIL: expected WAH_OK, got %s\n", wah_strerror(err));
            return 1;
        }
        assert_true(mod.required_features & t->feature);
        wah_free_module(&mod);
    }

    // --- wasm-feature-detect: each proposal fails when its feature is disabled ---
    for (size_t i = 0; i < NUM_DETECT_TESTS; i++) {
        const struct feature_detect_test *t = &detect_tests[i];
        printf("Testing %s: rejected when disabled...\n", t->name);
        wah_features_t disabled = WAH_FEATURE_ALL & ~(t->feature | t->also_disable);
        wah_parse_options_t opts = { .features = disabled };
        err = wah_parse_module_from_spec_ex(&mod, &opts, t->spec);
        if (err != WAH_ERROR_DISABLED_FEATURE) {
            fprintf(stderr, "  FAIL: expected WAH_ERROR_DISABLED_FEATURE, got %s\n", wah_strerror(err));
            return 1;
        }
    }

    // --- MVP module with zero optional features ---
    printf("Testing MVP module with zero optional features...\n");
    {
        const char *mvp_add =
            "wasm types {[fn [i32, i32] [i32]]} funcs {[0]} "
            "code {[{[] local.get 0 local.get 1 i32.add end}]}";
        wah_parse_options_t opts = { .features = 0 };
        assert_ok(wah_parse_module_from_spec_ex(&mod, &opts, mvp_add));
        assert_eq_u64(mod.required_features, 0);
        wah_free_module(&mod);
    }

    // --- Feature closure tests ---
    printf("Testing feature closure: RELAXED_SIMD implies SIMD...\n");
    {
        wah_features_t closed = wah_feature_closure(WAH_FEATURE_RELAXED_SIMD);
        assert_true(closed & WAH_FEATURE_SIMD);
    }

    printf("Testing feature closure: GC implies REF_TYPES...\n");
    {
        wah_features_t closed = wah_feature_closure(WAH_FEATURE_GC);
        assert_true(closed & WAH_FEATURE_REF_TYPES);
    }

    // --- wah_strerror ---
    printf("Testing wah_strerror(WAH_ERROR_DISABLED_FEATURE)...\n");
    assert_eq_str(wah_strerror(WAH_ERROR_DISABLED_FEATURE), "Feature not enabled");

    printf("All feature flag tests passed!\n");
    return 0;
}
