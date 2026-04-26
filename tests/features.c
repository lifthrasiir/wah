// Compile-time feature flag tests.
// This file is included by test_feature_*.c after defining WAH_COMPILED_FEATURES
// and including wah.h. It tests that runtime feature detection behaves correctly
// when certain features are compiled out.

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

static const char detect_mvp_add[] = "wasm \
    types {[fn [i32, i32] [i32]]} funcs {[0]} \
    code {[{[] local.get 0 local.get 1 i32.add end}]}";

struct feature_detect_test {
    const char *name;
    wah_features_t feature;
    wah_features_t also_disable;
    const char *spec;
};

static const struct feature_detect_test detect_tests[] = {
    { "bulk-memory",             WAH_FEATURE_BULK_MEMORY,     0, detect_bulk_memory },
    { "multi-value",             WAH_FEATURE_MULTI_VALUE,     0, detect_multi_value },
    { "mutable-globals",         WAH_FEATURE_MUTABLE_GLOBALS, 0, detect_mutable_globals },
    { "reference-types",         WAH_FEATURE_REF_TYPES,       WAH_FEATURE_GC | WAH_FEATURE_TYPED_FUNCREF, detect_reference_types },
    { "relaxed-simd",            WAH_FEATURE_RELAXED_SIMD,    0, detect_relaxed_simd },
    { "saturated-float-to-int",  WAH_FEATURE_NONTRAPPING_F2I, 0, detect_nontrapping_f2i },
    { "sign-extensions",         WAH_FEATURE_SIGN_EXT,        0, detect_sign_ext },
    { "simd",                    WAH_FEATURE_SIMD,            WAH_FEATURE_RELAXED_SIMD, detect_simd },
    { "tail-call",               WAH_FEATURE_TAIL_CALL,       0, detect_tail_call },
    { "memory64",                WAH_FEATURE_MEMORY64,        0, detect_memory64 },
    { "exceptions-final",        WAH_FEATURE_EXCEPTION,       0, detect_exception },
    { "gc",                      WAH_FEATURE_GC,              0, detect_gc },
    { "typed-function-references", WAH_FEATURE_TYPED_FUNCREF, 0, detect_typed_funcref },
};
#define NUM_DETECT_TESTS (sizeof(detect_tests) / sizeof(detect_tests[0]))

static int run_feature_tests(void) {
    wah_module_t mod;
    wah_error_t err;

    printf("WAH_COMPILED_FEATURES = 0x%llx\n", (unsigned long long)WAH_COMPILED_FEATURES);

    // Test 1: features within WAH_COMPILED_FEATURES parse OK at runtime
    for (size_t i = 0; i < NUM_DETECT_TESTS; i++) {
        const struct feature_detect_test *t = &detect_tests[i];
        wah_features_t compiled = WAH_COMPILED_FEATURES;
        // Skip if this feature is not compiled in
        wah_features_t needed = wah_feature_closure(t->feature);
        if ((needed & compiled) != needed) continue;

        printf("Testing %s: parses with compiled features...\n", t->name);
        err = wah_parse_module_from_spec_ex(&mod, NULL, t->spec);
        if (err != WAH_OK) {
            fprintf(stderr, "  FAIL: expected WAH_OK, got %s\n", wah_strerror(err));
            return 1;
        }
        assert_true(mod.required_features & t->feature);
        wah_free_module(&mod);
    }

    // Test 2: features within WAH_COMPILED_FEATURES can be disabled at runtime
    for (size_t i = 0; i < NUM_DETECT_TESTS; i++) {
        const struct feature_detect_test *t = &detect_tests[i];
        wah_features_t compiled = WAH_COMPILED_FEATURES;
        wah_features_t needed = wah_feature_closure(t->feature);
        if ((needed & compiled) != needed) continue;

        printf("Testing %s: rejected when runtime-disabled...\n", t->name);
        wah_features_t disabled = compiled & ~(t->feature | t->also_disable);
        wah_parse_options_t opts = { .features = disabled };
        err = wah_parse_module_from_spec_ex(&mod, &opts, t->spec);
        if (err != WAH_ERROR_DISABLED_FEATURE) {
            fprintf(stderr, "  FAIL: expected WAH_ERROR_DISABLED_FEATURE, got %s\n", wah_strerror(err));
            return 1;
        }
    }

    // Test 3: features NOT in WAH_COMPILED_FEATURES always fail,
    // even when explicitly requested at runtime
    for (size_t i = 0; i < NUM_DETECT_TESTS; i++) {
        const struct feature_detect_test *t = &detect_tests[i];
        wah_features_t compiled = WAH_COMPILED_FEATURES;
        wah_features_t needed = wah_feature_closure(t->feature);
        if ((needed & compiled) == needed) continue;

        printf("Testing %s: rejected when compiled out (even if runtime-enabled)...\n", t->name);
        wah_parse_options_t opts = { .features = WAH_FEATURE_ALL };
        err = wah_parse_module_from_spec_ex(&mod, &opts, t->spec);
        if (err != WAH_ERROR_DISABLED_FEATURE) {
            fprintf(stderr, "  FAIL: expected WAH_ERROR_DISABLED_FEATURE, got %s\n", wah_strerror(err));
            return 1;
        }
    }

    // Test 4: MVP module always works regardless of compiled features
    printf("Testing MVP module with zero optional features...\n");
    {
        wah_parse_options_t opts = { .features = 0 };
        assert_ok(wah_parse_module_from_spec_ex(&mod, &opts, detect_mvp_add));
        assert_eq_u64(mod.required_features, 0);
        wah_free_module(&mod);
    }

    // Test 5: feature closure
    printf("Testing feature closure: RELAXED_SIMD implies SIMD...\n");
    assert_true(wah_feature_closure(WAH_FEATURE_RELAXED_SIMD) & WAH_FEATURE_SIMD);

    printf("Testing feature closure: GC implies REF_TYPES...\n");
    assert_true(wah_feature_closure(WAH_FEATURE_GC) & WAH_FEATURE_REF_TYPES);

    // Test 6: strerror
    printf("Testing wah_strerror(WAH_ERROR_DISABLED_FEATURE)...\n");
    assert_eq_str(wah_strerror(WAH_ERROR_DISABLED_FEATURE), "Feature not enabled");

    printf("All feature flag tests passed!\n");
    return 0;
}
