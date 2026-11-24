// Subtyping across modules (wah_cross_module_subtype), which needs WAH_IMPLEMENTATION to be called directly.
// Its model in proofs/lean/WahProofs/Cross.lean agrees with the subtyping of the spec.

#define WAH_IMPLEMENTATION
#include "../wah.h"
#include "common.h"
#include <stdio.h>

int main(void) {
    wah_module_t a = {0}, b = {0};
    const char *spec = "wasm types {[ fn [] [] ]}";
    assert_ok(wah_parse_module_from_spec(&a, spec));
    assert_ok(wah_parse_module_from_spec(&b, spec));

    printf("Testing the bottom type across modules...\n");
    // The bottom type of validation is a subtype of anything, although it has the nullable bit
    assert_true(wah_cross_module_subtype(&a.alloc, &a, WAH_TYPE_BOT, &b, WAH_TYPE_I32));
    assert_true(wah_cross_module_subtype(&a.alloc, &a, WAH_TYPE_BOT, &b, WAH_TYPE_FROM_IDX(0, 0)));
    assert_true(wah_cross_module_subtype(&a.alloc, &a, WAH_TYPE_BOT, &a, WAH_TYPE_FUNC));
    assert_true(!wah_cross_module_subtype(&a.alloc, &a, WAH_TYPE_I32, &b, WAH_TYPE_BOT));

    printf("Testing types across modules...\n");
    assert_true(wah_cross_module_subtype(&a.alloc, &a, WAH_TYPE_FROM_IDX(0, 0), &b, WAH_TYPE_FROM_IDX(0, 1)));
    assert_true(!wah_cross_module_subtype(&a.alloc, &a, WAH_TYPE_FROM_IDX(0, 1), &b, WAH_TYPE_FROM_IDX(0, 0)));
    assert_true(wah_cross_module_subtype(&a.alloc, &a, WAH_TYPE_FROM_IDX(0, 0), &b, WAH_TYPE_FUNCREF));
    assert_true(!wah_cross_module_subtype(&a.alloc, &a, WAH_TYPE_I32, &b, WAH_TYPE_I64));

    wah_free_module(&a);
    wah_free_module(&b);
    printf("All cross-module subtype tests passed!\n");
    return 0;
}
