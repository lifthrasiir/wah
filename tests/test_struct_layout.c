#include "wah_impl.h"
#include "common.h"
#include <stdio.h>

enum {
    DEBUG_REPR_STRUCT = 1,
    DEBUG_REPR_ARRAY = 2,
};

static void get_repr(const wah_module_t *mod, wah_type_t type,
    wah_debug_repr_info_t *info, wah_debug_repr_field_t *fields, uint32_t field_count)
{
    assert_ok(wah_debug_repr_info(mod, type, info, fields, field_count));
    assert_eq_u32(info->count, field_count);
}

static uint32_t sum_sizes(const uint32_t *sizes, uint32_t count) {
    uint32_t sum = 0;
    for (uint32_t i = 0; i < count; ++i) sum += sizes[i];
    return sum;
}

static void assert_layout(const char *spec, const uint32_t *sizes,
    const uint32_t *expected_offsets, uint32_t field_count, uint32_t expected_size)
{
    wah_module_t mod = {0};
    wah_type_t type;
    wah_debug_repr_info_t info;
    wah_debug_repr_field_t fields[32] = {{0}};

    assert_true(field_count <= 32);
    assert_ok(wah_new_module(&mod, NULL));
    assert_ok(wah_define_type(&mod, &type, spec));
    get_repr(&mod, type, &info, fields, field_count);
    assert_eq_u32(info.type, DEBUG_REPR_STRUCT);
    assert_eq_u32(info.size, expected_size);
    assert_true(info.size >= sum_sizes(sizes, field_count));
    assert_true(info.size - sum_sizes(sizes, field_count) <= 15);
    for (uint32_t i = 0; i < field_count; ++i) {
        assert_eq_u32(fields[i].offset, expected_offsets[i]);
        assert_true((fields[i].offset & (sizes[i] - 1)) == 0);
        assert_true(fields[i].offset + sizes[i] <= info.size);
        for (uint32_t j = i + 1; j < field_count; ++j) {
            uint32_t a0 = fields[i].offset, a1 = a0 + sizes[i];
            uint32_t b0 = fields[j].offset, b1 = b0 + sizes[j];
            assert_true(a1 <= b0 || b1 <= a0);
        }
    }
    wah_free_module(&mod);
}

static void test_fully_packed_preservation(void) {
    printf("Testing fully packed struct layout preservation...\n");

    {
        const uint32_t sizes[] = {1, 1};
        const uint32_t offsets[] = {0, 1};
        assert_layout("struct { i8, i8 }", sizes, offsets, 2, 2);
    }
    {
        const uint32_t sizes[] = {1, 1, 2, 4, 8};
        const uint32_t offsets[] = {0, 1, 2, 4, 8};
        assert_layout("struct { i8, i8, i16, i32, i64 }", sizes, offsets, 5, 16);
    }
    {
        const uint32_t sizes[] = {4, 4, 8};
        const uint32_t offsets[] = {0, 4, 8};
        assert_layout("struct { i32, i32, i64 }", sizes, offsets, 3, 16);
    }
    {
        const uint32_t sizes[] = {16, 8, 4, 2, 1, 1};
        const uint32_t offsets[] = {0, 16, 24, 28, 30, 31};
        assert_layout("struct { v128, i64, i32, i16, i8, i8 }", sizes, offsets, 6, 32);
    }
}

static void test_slot_reuse_adversarial_orders(void) {
    printf("Testing aligned slot reuse for adversarial field orders...\n");

    {
        const uint32_t sizes[] = {1, 16, 1, 16, 1, 16};
        const uint32_t offsets[] = {0, 16, 1, 32, 2, 48};
        assert_layout("struct { i8, v128, i8, v128, i8, v128 }", sizes, offsets, 6, 64);
    }
    {
        const uint32_t sizes[] = {4, 16, 4, 16, 4};
        const uint32_t offsets[] = {0, 16, 4, 32, 8};
        assert_layout("struct { i32, v128, i32, v128, i32 }", sizes, offsets, 5, 48);
    }
    {
        const uint32_t sizes[] = {1, 2, 4, 8};
        const uint32_t offsets[] = {0, 2, 4, 8};
        assert_layout("struct { i8, i16, i32, i64 }", sizes, offsets, 4, 16);
    }
}

static void test_ref_field_layout(void) {
    printf("Testing reference field participates in pointer-sized layout class...\n");

    wah_module_t mod = {0};
    wah_type_t leaf, parent;
    wah_debug_repr_info_t info;
    wah_debug_repr_field_t fields[3] = {{0}};
    uint32_t ref_size = (uint32_t)sizeof(void *);

    assert_ok(wah_new_module(&mod, NULL));
    assert_ok(wah_define_type(&mod, &leaf, "struct { i32 }"));
    assert_ok(wah_define_type(&mod, &parent, "struct { i8, ref null %T, i8 }", leaf));
    get_repr(&mod, parent, &info, fields, 3);
    assert_eq_u32(info.type, DEBUG_REPR_STRUCT);
    assert_eq_u32(fields[0].offset, 0);
    assert_true((fields[1].offset & (ref_size - 1)) == 0);
    assert_eq_u32(fields[2].offset, 1);
    assert_true(info.size >= ref_size + 2);
    assert_true(info.size - (ref_size + 2) <= 15);
    wah_free_module(&mod);
}

static void test_array_stride_unchanged(void) {
    printf("Testing array element layout remains fixed stride...\n");

    wah_module_t mod = {0};
    wah_type_t array_type;
    wah_debug_repr_info_t info;
    wah_debug_repr_field_t field = {0};

    assert_ok(wah_new_module(&mod, NULL));
    assert_ok(wah_define_type(&mod, &array_type, "array i8"));
    get_repr(&mod, array_type, &info, &field, 1);
    assert_eq_u32(info.type, DEBUG_REPR_ARRAY);
    assert_eq_u32(info.size, 1);
    assert_eq_u32(field.offset, 0);
    wah_free_module(&mod);
}

static void test_parse_rebuild_path(void) {
    printf("Testing parsed module metadata uses aligned struct layout...\n");

    const char *spec = "wasm types {[ struct [i8 mut, v128 mut, i8 mut], fn [] [] ]} funcs {[1]} code {[ {[] end} ]}";
    wah_module_t mod = {0};
    wah_debug_repr_info_t info;
    wah_debug_repr_field_t fields[3] = {{0}};

    assert_ok(wah_parse_module_from_spec(&mod, spec));
    get_repr(&mod, wah_debug_type_from_idx(0, false), &info, fields, 3);
    assert_eq_u32(info.type, DEBUG_REPR_STRUCT);
    assert_eq_u32(info.size, 32);
    assert_eq_u32(fields[0].offset, 0);
    assert_eq_u32(fields[1].offset, 16);
    assert_eq_u32(fields[2].offset, 1);
    wah_free_module(&mod);
}

int main(void) {
    test_fully_packed_preservation();
    test_slot_reuse_adversarial_orders();
    test_ref_field_layout();
    test_array_stride_unchanged();
    test_parse_rebuild_path();
    printf("All struct layout tests passed!\n");
    return 0;
}
