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

    // --- POLL insertion tests ---
    printf("Testing POLL at function entry...\n");
    {
        // Simple function: just returns. Should have POLL at entry.
        const char *spec = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0 ]} \
            code {[ {[] i32.const 42 end } ]}";
        assert_ok(wah_parse_module_from_spec(&module, spec));
        // First opcode in preparsed bytecode should be POLL
        uint16_t first_op = module.code_bodies[0].parsed_code.bytecode[0]
                          | (module.code_bodies[0].parsed_code.bytecode[1] << 8);
        assert_true(first_op == WAH_OP_POLL);
        // Execution still works
        assert_ok(wah_exec_context_create(&ctx, &module));
        wah_value_t r;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &r));
        assert_eq_i32(r.i32, 42);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Testing POLL at loop header...\n");
    {
        // Function with a loop that counts down
        const char *spec = "wasm \
            types {[ fn [i32] [i32] ]} \
            funcs {[ 0 ]} \
            code {[ {[] \
                block void \
                    loop void \
                        local.get 0 \
                        i32.eqz \
                        br_if 1 \
                        local.get 0 \
                        i32.const 1 \
                        i32.sub \
                        local.set 0 \
                        br 0 \
                    end \
                end \
                local.get 0 \
            end } ]}";
        assert_ok(wah_parse_module_from_spec(&module, spec));
        assert_ok(wah_exec_context_create(&ctx, &module));
        wah_value_t params[1] = {{.i32 = 10}};
        wah_value_t r;
        assert_ok(wah_call(&ctx, 0, params, 1, &r));
        assert_eq_i32(r.i32, 0);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Testing POLL after call site...\n");
    {
        // Two functions: callee returns 7, caller calls callee
        const char *spec = "wasm \
            types {[ fn [] [i32], fn [] [i32] ]} \
            funcs {[ 0, 1 ]} \
            code {[ \
                {[] i32.const 7 end }, \
                {[] call 0 end } \
            ]}";
        assert_ok(wah_parse_module_from_spec(&module, spec));
        assert_ok(wah_exec_context_create(&ctx, &module));
        wah_value_t r;
        assert_ok(wah_call(&ctx, 1, NULL, 0, &r));
        assert_eq_i32(r.i32, 7);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Testing POLL not in constexpr...\n");
    {
        const char *spec = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0 ]} \
            globals {[ i32 mut i32.const 99 end ]} \
            code {[ {[] global.get 0 end } ]}";
        assert_ok(wah_parse_module_from_spec(&module, spec));
        // Constexpr should NOT start with POLL
        uint16_t first_op = module.globals[0].init_expr.bytecode[0]
                          | (module.globals[0].init_expr.bytecode[1] << 8);
        assert_true(first_op != WAH_OP_POLL);
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_instantiate(&ctx));
        wah_value_t r;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &r));
        assert_eq_i32(r.i32, 99);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    // --- POLL handler tests ---
    printf("Testing POLL fast path (no GC state)...\n");
    {
        const char *spec = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0 ]} \
            code {[ {[] i32.const 1 end } ]}";
        assert_ok(wah_parse_module_from_spec(&module, spec));
        assert_ok(wah_exec_context_create(&ctx, &module));
        wah_value_t r;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &r));
        assert_eq_i32(r.i32, 1);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Testing POLL with gc_pending (clears flag)...\n");
    {
        const char *spec = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0 ]} \
            code {[ {[] i32.const 2 end } ]}";
        assert_ok(wah_parse_module_from_spec(&module, spec));
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_gc_start(&ctx));
        ctx.gc->gc_pending = true;
        wah_value_t r;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &r));
        assert_eq_i32(r.i32, 2);
        assert_false(ctx.gc->gc_pending);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Testing POLL with interrupt_pending (traps)...\n");
    {
        const char *spec = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0 ]} \
            code {[ {[] i32.const 3 end } ]}";
        assert_ok(wah_parse_module_from_spec(&module, spec));
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_gc_start(&ctx));
        ctx.gc->interrupt_pending = true;
        wah_value_t r;
        assert_err(wah_call(&ctx, 0, NULL, 0, &r), WAH_ERROR_TRAP);
        assert_false(ctx.gc->interrupt_pending);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("All GC tests passed.\n");
    return 0;
}
