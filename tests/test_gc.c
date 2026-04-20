#define WAH_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include "../wah.h"
#include "common.c"

int main() {
    wah_module_t module;
    wah_exec_context_t ctx;

    // Minimal module for context creation
    const char *nop_wasm = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        code {[ {[] end } ]}";
    assert_ok(wah_parse_module_from_spec(&module, nop_wasm));
    assert_ok(wah_exec_context_create(&ctx, &module));

    printf("Testing GC state: initially NULL...\n");
    assert_null(ctx.gc);

    printf("Testing wah_gc_start...\n");
    assert_ok(wah_gc_start(&ctx));
    assert_not_null(ctx.gc);
    assert_true(ctx.gc->phase == WAH_GC_PHASE_IDLE);
    assert_true(ctx.gc->object_count == 0);
    assert_true(ctx.gc->allocated_bytes == 0);
    assert_true(ctx.gc->all_objects == NULL);
    assert_false(ctx.gc->gc_pending);
    assert_false(ctx.gc->interrupt_pending);
    assert_true(ctx.gc->allocation_threshold > 0);

    printf("Testing wah_gc_start idempotent...\n");
    wah_gc_state_t *saved = ctx.gc;
    assert_ok(wah_gc_start(&ctx));
    assert_eq_ptr(ctx.gc, saved);

    printf("Testing wah_gc_reset...\n");
    ctx.gc->gc_pending = true;
    ctx.gc->phase = WAH_GC_PHASE_MARK;
    wah_gc_reset(&ctx);
    assert_true(ctx.gc->phase == WAH_GC_PHASE_IDLE);
    assert_false(ctx.gc->gc_pending);
    assert_false(ctx.gc->interrupt_pending);
    assert_true(ctx.gc->object_count == 0);

    printf("Testing wah_gc_destroy...\n");
    wah_gc_destroy(&ctx);
    assert_null(ctx.gc);

    printf("Testing wah_gc_destroy idempotent...\n");
    wah_gc_destroy(&ctx);
    assert_null(ctx.gc);

    printf("Testing wah_gc_reset on NULL gc (no-op)...\n");
    wah_gc_reset(&ctx);
    assert_null(ctx.gc);

    printf("Testing exec context destroy cleans up GC...\n");
    assert_ok(wah_exec_context_create(&ctx, &module));
    assert_ok(wah_gc_start(&ctx));
    assert_not_null(ctx.gc);
    wah_exec_context_destroy(&ctx);
    assert_null(ctx.gc);

    printf("Testing non-GC execution unaffected...\n");
    assert_ok(wah_exec_context_create(&ctx, &module));
    wah_value_t result;
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    wah_exec_context_destroy(&ctx);

    wah_free_module(&module);

    printf("All GC state tests passed.\n");
    return 0;
}
