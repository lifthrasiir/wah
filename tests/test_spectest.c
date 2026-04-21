#define WAH_IMPLEMENTATION
#include "../wah.h"
#include "wast.c"
#include "spectest.c"

static const char *SPECTEST_FILES[] = {
    "tests/spectest/address.bin.wast",
    "tests/spectest/address0.bin.wast",
    "tests/spectest/address1.bin.wast",
    "tests/spectest/address64.bin.wast",
    "tests/spectest/align.bin.wast",
    "tests/spectest/align0.bin.wast",
    "tests/spectest/align64.bin.wast",
    //"tests/spectest/annotations.bin.wast",     // annotations proposal
    //"tests/spectest/array.bin.wast",           // GC proposal
    //"tests/spectest/array_copy.bin.wast",      // GC proposal
    //"tests/spectest/array_fill.bin.wast",      // GC proposal
    //"tests/spectest/array_init_data.bin.wast", // GC proposal
    //"tests/spectest/array_init_elem.bin.wast", // GC proposal
    //"tests/spectest/array_new_data.bin.wast",  // GC proposal
    //"tests/spectest/array_new_elem.bin.wast",  // GC proposal
    //"tests/spectest/binary.bin.wast",          // needs extended ref types
    "tests/spectest/binary_leb128_64.bin.wast",
    "tests/spectest/binary0.bin.wast",
    //"tests/spectest/binary-gc.bin.wast",       // GC proposal
    "tests/spectest/binary-leb128.bin.wast",
    "tests/spectest/block.bin.wast",
    "tests/spectest/br.bin.wast",
    "tests/spectest/br_if.bin.wast",
    //"tests/spectest/br_on_cast.bin.wast",      // GC proposal
    //"tests/spectest/br_on_cast_fail.bin.wast", // GC proposal
    //"tests/spectest/br_on_non_null.bin.wast",  // needs extended ref types
    //"tests/spectest/br_on_null.bin.wast",      // needs extended ref types
    "tests/spectest/br_table.bin.wast",
    "tests/spectest/bulk.bin.wast",
    "tests/spectest/bulk64.bin.wast",
    "tests/spectest/call.bin.wast",
    "tests/spectest/call_indirect.bin.wast",
    "tests/spectest/call_indirect64.bin.wast",
    "tests/spectest/call_ref.bin.wast",
    "tests/spectest/const.bin.wast",
    "tests/spectest/conversions.bin.wast",
    //"tests/spectest/custom.bin.wast",          // needs extended ref types
    "tests/spectest/data.bin.wast",
    "tests/spectest/data_drop0.bin.wast",
    "tests/spectest/data0.bin.wast",
    "tests/spectest/data1.bin.wast",
    //"tests/spectest/elem.bin.wast",             // needs table-with-init-expr (0x40)
    "tests/spectest/endianness.bin.wast",
    "tests/spectest/endianness64.bin.wast",
    "tests/spectest/exports.bin.wast",
    "tests/spectest/exports0.bin.wast",
    //"tests/spectest/extern.bin.wast",          // GC proposal
    "tests/spectest/f32.bin.wast",
    "tests/spectest/f32_bitwise.bin.wast",
    "tests/spectest/f32_cmp.bin.wast",
    "tests/spectest/f64.bin.wast",
    "tests/spectest/f64_bitwise.bin.wast",
    "tests/spectest/f64_cmp.bin.wast",
    "tests/spectest/fac.bin.wast",
    "tests/spectest/float_exprs.bin.wast",
    "tests/spectest/float_exprs0.bin.wast",
    "tests/spectest/float_exprs1.bin.wast",
    "tests/spectest/float_literals.bin.wast",
    "tests/spectest/float_memory.bin.wast",
    "tests/spectest/float_memory0.bin.wast",
    "tests/spectest/float_memory64.bin.wast",
    "tests/spectest/float_misc.bin.wast",
    "tests/spectest/forward.bin.wast",
    "tests/spectest/func.bin.wast",
    "tests/spectest/func_ptrs.bin.wast",
    "tests/spectest/global.bin.wast",
    "tests/spectest/i16x8_relaxed_q15mulr_s.bin.wast",
    //"tests/spectest/i31.bin.wast",             // GC proposal
    "tests/spectest/i32.bin.wast",
    "tests/spectest/i32x4_relaxed_trunc.bin.wast",
    "tests/spectest/i64.bin.wast",
    "tests/spectest/i8x16_relaxed_swizzle.bin.wast",
    "tests/spectest/id.bin.wast",
    "tests/spectest/if.bin.wast",
    //"tests/spectest/imports.bin.wast",         // needs extended ref types
    "tests/spectest/imports0.bin.wast",
    "tests/spectest/imports1.bin.wast",
    "tests/spectest/imports2.bin.wast",
    "tests/spectest/imports3.bin.wast",
    "tests/spectest/imports4.bin.wast",
    "tests/spectest/inline-module.bin.wast",
    //"tests/spectest/instance.bin.wast",        // GC proposal
    "tests/spectest/int_exprs.bin.wast",
    "tests/spectest/int_literals.bin.wast",
    "tests/spectest/labels.bin.wast",
    "tests/spectest/left-to-right.bin.wast",
    //"tests/spectest/linking.bin.wast",         // needs extended ref types
    "tests/spectest/linking0.bin.wast",
    "tests/spectest/linking1.bin.wast",
    "tests/spectest/linking2.bin.wast",
    "tests/spectest/linking3.bin.wast",
    "tests/spectest/load.bin.wast",
    "tests/spectest/load0.bin.wast",
    "tests/spectest/load1.bin.wast",
    "tests/spectest/load2.bin.wast",
    "tests/spectest/load64.bin.wast",
    "tests/spectest/local_get.bin.wast",
    "tests/spectest/local_init.bin.wast",
    "tests/spectest/local_set.bin.wast",
    "tests/spectest/local_tee.bin.wast",
    "tests/spectest/loop.bin.wast",
    "tests/spectest/memory.bin.wast",
    "tests/spectest/memory_copy.bin.wast",
    "tests/spectest/memory_copy0.bin.wast",
    "tests/spectest/memory_copy1.bin.wast",
    "tests/spectest/memory_copy64.bin.wast",
    "tests/spectest/memory_fill.bin.wast",
    "tests/spectest/memory_fill0.bin.wast",
    "tests/spectest/memory_fill64.bin.wast",
    "tests/spectest/memory_grow.bin.wast",
    "tests/spectest/memory_grow64.bin.wast",
    "tests/spectest/memory_init.bin.wast",
    "tests/spectest/memory_init0.bin.wast",
    "tests/spectest/memory_init64.bin.wast",
    "tests/spectest/memory_redundancy.bin.wast",
    "tests/spectest/memory_redundancy64.bin.wast",
    "tests/spectest/memory_size.bin.wast",
    "tests/spectest/memory_size_import.bin.wast",
    "tests/spectest/memory_size0.bin.wast",
    "tests/spectest/memory_size1.bin.wast",
    "tests/spectest/memory_size2.bin.wast",
    "tests/spectest/memory_size3.bin.wast",
    "tests/spectest/memory_trap.bin.wast",
    "tests/spectest/memory_trap0.bin.wast",
    "tests/spectest/memory_trap1.bin.wast",
    "tests/spectest/memory_trap64.bin.wast",
    "tests/spectest/memory64.bin.wast",
    "tests/spectest/memory64-imports.bin.wast",
    "tests/spectest/memory-multi.bin.wast",
    "tests/spectest/names.bin.wast",
    "tests/spectest/nop.bin.wast",
    //"tests/spectest/obsolete-keywords.bin.wast", // obsolete keywords
    "tests/spectest/ref.bin.wast",
    //"tests/spectest/ref_as_non_null.bin.wast", // needs extended ref types
    //"tests/spectest/ref_cast.bin.wast",        // GC proposal
    //"tests/spectest/ref_eq.bin.wast",          // GC proposal
    "tests/spectest/ref_func.bin.wast",
    "tests/spectest/ref_is_null.bin.wast",
    "tests/spectest/ref_null.bin.wast",
    //"tests/spectest/ref_test.bin.wast",        // GC proposal
    "tests/spectest/relaxed_dot_product.bin.wast",
    "tests/spectest/relaxed_laneselect.bin.wast",
    "tests/spectest/relaxed_madd_nmadd.bin.wast",
    "tests/spectest/relaxed_min_max.bin.wast",
    "tests/spectest/return.bin.wast",
    //"tests/spectest/return_call.bin.wast",     // unimplemented
    //"tests/spectest/return_call_indirect.bin.wast", // unimplemented
    //"tests/spectest/return_call_ref.bin.wast", // GC proposal
    "tests/spectest/select.bin.wast",
    "tests/spectest/simd_address.bin.wast",
    "tests/spectest/simd_align.bin.wast",
    "tests/spectest/simd_bit_shift.bin.wast",
    "tests/spectest/simd_bitwise.bin.wast",
    "tests/spectest/simd_boolean.bin.wast",
    "tests/spectest/simd_const.bin.wast",
    "tests/spectest/simd_conversions.bin.wast",
    "tests/spectest/simd_f32x4.bin.wast",
    "tests/spectest/simd_f32x4_arith.bin.wast",
    "tests/spectest/simd_f32x4_cmp.bin.wast",
    "tests/spectest/simd_f32x4_pmin_pmax.bin.wast",
    "tests/spectest/simd_f32x4_rounding.bin.wast",
    "tests/spectest/simd_f64x2.bin.wast",
    "tests/spectest/simd_f64x2_arith.bin.wast",
    "tests/spectest/simd_f64x2_cmp.bin.wast",
    "tests/spectest/simd_f64x2_pmin_pmax.bin.wast",
    "tests/spectest/simd_f64x2_rounding.bin.wast",
    "tests/spectest/simd_i16x8_arith.bin.wast",
    "tests/spectest/simd_i16x8_arith2.bin.wast",
    "tests/spectest/simd_i16x8_cmp.bin.wast",
    "tests/spectest/simd_i16x8_extadd_pairwise_i8x16.bin.wast",
    "tests/spectest/simd_i16x8_extmul_i8x16.bin.wast",
    "tests/spectest/simd_i16x8_q15mulr_sat_s.bin.wast",
    "tests/spectest/simd_i16x8_sat_arith.bin.wast",
    "tests/spectest/simd_i32x4_arith.bin.wast",
    "tests/spectest/simd_i32x4_arith2.bin.wast",
    "tests/spectest/simd_i32x4_cmp.bin.wast",
    "tests/spectest/simd_i32x4_dot_i16x8.bin.wast",
    "tests/spectest/simd_i32x4_extadd_pairwise_i16x8.bin.wast",
    "tests/spectest/simd_i32x4_extmul_i16x8.bin.wast",
    "tests/spectest/simd_i32x4_trunc_sat_f32x4.bin.wast",
    "tests/spectest/simd_i32x4_trunc_sat_f64x2.bin.wast",
    "tests/spectest/simd_i64x2_arith.bin.wast",
    "tests/spectest/simd_i64x2_arith2.bin.wast",
    "tests/spectest/simd_i64x2_cmp.bin.wast",
    "tests/spectest/simd_i64x2_extmul_i32x4.bin.wast",
    "tests/spectest/simd_i8x16_arith.bin.wast",
    "tests/spectest/simd_i8x16_arith2.bin.wast",
    "tests/spectest/simd_i8x16_cmp.bin.wast",
    "tests/spectest/simd_i8x16_sat_arith.bin.wast",
    "tests/spectest/simd_int_to_int_extend.bin.wast",
    "tests/spectest/simd_lane.bin.wast",
    "tests/spectest/simd_linking.bin.wast",
    "tests/spectest/simd_load.bin.wast",
    "tests/spectest/simd_load_extend.bin.wast",
    "tests/spectest/simd_load_splat.bin.wast",
    "tests/spectest/simd_load_zero.bin.wast",
    "tests/spectest/simd_load16_lane.bin.wast",
    "tests/spectest/simd_load32_lane.bin.wast",
    "tests/spectest/simd_load64_lane.bin.wast",
    "tests/spectest/simd_load8_lane.bin.wast",
    "tests/spectest/simd_memory-multi.bin.wast",
    "tests/spectest/simd_select.bin.wast",
    "tests/spectest/simd_splat.bin.wast",
    "tests/spectest/simd_store.bin.wast",
    "tests/spectest/simd_store16_lane.bin.wast",
    "tests/spectest/simd_store32_lane.bin.wast",
    "tests/spectest/simd_store64_lane.bin.wast",
    "tests/spectest/simd_store8_lane.bin.wast",
    //"tests/spectest/skip-stack-guard-page.bin.wast", // OS stack guard behavior
    "tests/spectest/stack.bin.wast",
    "tests/spectest/start.bin.wast",
    "tests/spectest/start0.bin.wast",
    "tests/spectest/store.bin.wast",
    "tests/spectest/store0.bin.wast",
    "tests/spectest/store1.bin.wast",
    "tests/spectest/store2.bin.wast",
    "tests/spectest/struct.bin.wast",
    "tests/spectest/switch.bin.wast",
    "tests/spectest/table.bin.wast",
    "tests/spectest/table_copy.bin.wast",
    "tests/spectest/table_copy_mixed.bin.wast",
    "tests/spectest/table_copy64.bin.wast",
    "tests/spectest/table_fill.bin.wast",
    "tests/spectest/table_fill64.bin.wast",
    "tests/spectest/table_get.bin.wast",
    "tests/spectest/table_get64.bin.wast",
    "tests/spectest/table_grow.bin.wast",
    "tests/spectest/table_grow64.bin.wast",
    "tests/spectest/table_init.bin.wast",
    "tests/spectest/table_init64.bin.wast",
    "tests/spectest/table_set.bin.wast",
    "tests/spectest/table_set64.bin.wast",
    "tests/spectest/table_size.bin.wast",
    "tests/spectest/table_size64.bin.wast",
    "tests/spectest/table64.bin.wast",
    "tests/spectest/table-sub.bin.wast",
    //"tests/spectest/tag.bin.wast",             // exception handling proposal
    //"tests/spectest/throw.bin.wast",           // exception handling proposal
    //"tests/spectest/throw_ref.bin.wast",       // exception handling proposal
    //"tests/spectest/token.bin.wast",           // token/text format tests
    "tests/spectest/traps.bin.wast",
    "tests/spectest/traps0.bin.wast",
    //"tests/spectest/try_table.bin.wast",       // exception handling proposal
    "tests/spectest/type.bin.wast",
    "tests/spectest/type-canon.bin.wast",
    "tests/spectest/type-equivalence.bin.wast",
    "tests/spectest/type-rec.bin.wast",
    "tests/spectest/type-subtyping.bin.wast",
    "tests/spectest/unreachable.bin.wast",
    //"tests/spectest/unreached-invalid.bin.wast",// needs extended ref types
    //"tests/spectest/unreached-valid.bin.wast",  // needs extended ref types
    "tests/spectest/unwind.bin.wast",
    "tests/spectest/utf8-custom-section-id.bin.wast",
    "tests/spectest/utf8-import-field.bin.wast",
    "tests/spectest/utf8-import-module.bin.wast",
    "tests/spectest/utf8-invalid-encoding.bin.wast",
};

int main(int argc, char **argv) {
    size_t i;
    const char *filter = argc > 1 ? argv[1] : NULL;
    tally_t tally = {0};
    if (!run_parser_sanity()) {
        return 1;
    }
    for (i = 0; i < sizeof(SPECTEST_FILES) / sizeof(SPECTEST_FILES[0]); ++i) {
        if (filter && !strstr(SPECTEST_FILES[i], filter)) continue;
        run_file(SPECTEST_FILES[i], &tally);
    }
    printf("\nSpectest tally: %u passed, %u failed, %u total checks across %u files (%u files with failures, %u files skipped)\n",
           tally.passed_checks, tally.failed_checks, tally.total_checks,
           tally.files_run, tally.files_failed, tally.files_skipped);
    return tally.failed_checks == 0 ? 0 : 1;
}
