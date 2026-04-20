#define WAH_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include "../wah.h"
#include "common.c"

static void count_roots_visitor(wah_value_t *slot, wah_type_t type, void *ud) {
    (void)slot; (void)type;
    (*(uint32_t*)ud)++;
}

static void count_ref_roots_visitor(wah_value_t *slot, wah_type_t type, void *ud) {
    (void)slot;
    assert(WAH_TYPE_IS_REF(type));
    (*(uint32_t*)ud)++;
}

static void count_funcref_roots_visitor(wah_value_t *slot, wah_type_t type, void *ud) {
    (void)slot;
    assert(WAH_TYPE_IS_FUNCREF(type));
    (*(uint32_t*)ud)++;
}

static uint32_t g_root_count;

static void host_count_roots(wah_call_context_t *cc, void *ud) {
    (void)ud;
    g_root_count = 0;
    wah_gc_enumerate_roots(cc->exec, count_ref_roots_visitor, &g_root_count);
    wah_return_i32(cc, (int32_t)g_root_count);
}

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

    // --- Root enumeration tests ---
    printf("Testing root enumeration: no roots in scalar-only module...\n");
    {
        const char *spec = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0 ]} \
            code {[ {[] i32.const 5 end } ]}";
        assert_ok(wah_parse_module_from_spec(&module, spec));
        assert_ok(wah_exec_context_create(&ctx, &module));
        uint32_t count = 0;
        wah_gc_enumerate_roots(&ctx, count_roots_visitor, &count);
        assert_eq_u32(count, 0);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Testing root enumeration: funcref global...\n");
    {
        const char *spec = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0 ]} \
            globals {[ funcref mut ref.null funcref end ]} \
            code {[ {[] i32.const 1 end } ]}";
        assert_ok(wah_parse_module_from_spec(&module, spec));
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_instantiate(&ctx));
        uint32_t count = 0;
        wah_gc_enumerate_roots(&ctx, count_ref_roots_visitor, &count);
        assert_eq_u32(count, 1);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Testing root enumeration: funcref table...\n");
    {
        const char *spec = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0 ]} \
            tables {[ funcref limits.i32/2 3 3 ]} \
            code {[ {[] i32.const 1 end } ]}";
        assert_ok(wah_parse_module_from_spec(&module, spec));
        assert_ok(wah_exec_context_create(&ctx, &module));
        uint32_t count = 0;
        wah_gc_enumerate_roots(&ctx, count_funcref_roots_visitor, &count);
        assert_eq_u32(count, 3);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Testing root enumeration: NULL visitor is no-op...\n");
    {
        const char *spec = "wasm \
            types {[ fn [] [] ]} \
            funcs {[ 0 ]} \
            code {[ {[] end } ]}";
        assert_ok(wah_parse_module_from_spec(&module, spec));
        assert_ok(wah_exec_context_create(&ctx, &module));
        wah_gc_enumerate_roots(&ctx, NULL, NULL);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    // --- GC allocation tests ---
    printf("Testing GC alloc basic...\n");
    {
        const char *spec = "wasm \
            types {[ fn [] [] ]} \
            funcs {[ 0 ]} \
            code {[ {[] end } ]}";
        assert_ok(wah_parse_module_from_spec(&module, spec));
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_gc_start(&ctx));

        wah_gc_object_t *obj = wah_gc_alloc(&ctx, WAH_GC_KIND_NONE, 16);
        assert_not_null(obj);
        assert_true(obj->kind == WAH_GC_KIND_NONE);
        assert_true(obj->size == 16);
        assert_false(obj->mark);
        assert_eq_u32(ctx.gc->object_count, 1);
        assert_true(ctx.gc->allocated_bytes == sizeof(wah_gc_object_t) + 16);
        assert_eq_ptr(ctx.gc->all_objects, obj);

        void *payload = wah_gc_payload(obj);
        assert_eq_ptr(wah_gc_header(payload), obj);

        wah_gc_object_t *obj2 = wah_gc_alloc(&ctx, WAH_GC_KIND_NONE, 32);
        assert_not_null(obj2);
        assert_eq_u32(ctx.gc->object_count, 2);
        assert_eq_ptr(ctx.gc->all_objects, obj2);
        assert_eq_ptr(obj2->next, obj);

        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Testing GC alloc without GC state returns NULL...\n");
    {
        const char *spec = "wasm \
            types {[ fn [] [] ]} \
            funcs {[ 0 ]} \
            code {[ {[] end } ]}";
        assert_ok(wah_parse_module_from_spec(&module, spec));
        assert_ok(wah_exec_context_create(&ctx, &module));
        wah_gc_object_t *obj = wah_gc_alloc(&ctx, WAH_GC_KIND_NONE, 8);
        assert_null(obj);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Testing GC alloc triggers gc_pending at threshold...\n");
    {
        const char *spec = "wasm \
            types {[ fn [] [] ]} \
            funcs {[ 0 ]} \
            code {[ {[] end } ]}";
        assert_ok(wah_parse_module_from_spec(&module, spec));
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_gc_start(&ctx));
        ctx.gc->allocation_threshold = sizeof(wah_gc_object_t) + 8;
        assert_false(ctx.gc->gc_pending);
        wah_gc_alloc(&ctx, WAH_GC_KIND_NONE, 8);
        assert_true(ctx.gc->gc_pending);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Testing GC reset frees all objects...\n");
    {
        const char *spec = "wasm \
            types {[ fn [] [] ]} \
            funcs {[ 0 ]} \
            code {[ {[] end } ]}";
        assert_ok(wah_parse_module_from_spec(&module, spec));
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_gc_start(&ctx));
        wah_gc_alloc(&ctx, WAH_GC_KIND_NONE, 16);
        wah_gc_alloc(&ctx, WAH_GC_KIND_NONE, 32);
        assert_eq_u32(ctx.gc->object_count, 2);
        wah_gc_reset(&ctx);
        assert_eq_u32(ctx.gc->object_count, 0);
        assert_true(ctx.gc->allocated_bytes == 0);
        assert_null(ctx.gc->all_objects);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    // --- GC stepping tests ---
    printf("Testing GC step sweeps all unreachable objects...\n");
    {
        const char *spec = "wasm \
            types {[ fn [] [] ]} \
            funcs {[ 0 ]} \
            code {[ {[] end } ]}";
        assert_ok(wah_parse_module_from_spec(&module, spec));
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_gc_start(&ctx));
        wah_gc_alloc(&ctx, WAH_GC_KIND_NONE, 16);
        wah_gc_alloc(&ctx, WAH_GC_KIND_NONE, 32);
        wah_gc_alloc(&ctx, WAH_GC_KIND_NONE, 64);
        assert_eq_u32(ctx.gc->object_count, 3);
        wah_gc_step(&ctx);
        assert_eq_u32(ctx.gc->object_count, 0);
        assert_true(ctx.gc->allocated_bytes == 0);
        assert_true(ctx.gc->phase == WAH_GC_PHASE_IDLE);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Testing GC step on empty heap is no-op...\n");
    {
        const char *spec = "wasm \
            types {[ fn [] [] ]} \
            funcs {[ 0 ]} \
            code {[ {[] end } ]}";
        assert_ok(wah_parse_module_from_spec(&module, spec));
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_gc_start(&ctx));
        wah_gc_step(&ctx);
        assert_eq_u32(ctx.gc->object_count, 0);
        assert_true(ctx.gc->phase == WAH_GC_PHASE_IDLE);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Testing GC step without GC state is no-op...\n");
    {
        const char *spec = "wasm \
            types {[ fn [] [] ]} \
            funcs {[ 0 ]} \
            code {[ {[] end } ]}";
        assert_ok(wah_parse_module_from_spec(&module, spec));
        assert_ok(wah_exec_context_create(&ctx, &module));
        wah_gc_step(&ctx);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Testing GC step via poll handler...\n");
    {
        const char *spec = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0 ]} \
            code {[ {[] i32.const 42 end } ]}";
        assert_ok(wah_parse_module_from_spec(&module, spec));
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_gc_start(&ctx));
        wah_gc_alloc(&ctx, WAH_GC_KIND_NONE, 16);
        wah_gc_alloc(&ctx, WAH_GC_KIND_NONE, 32);
        ctx.gc->gc_pending = true;
        wah_value_t r;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &r));
        assert_eq_i32(r.i32, 42);
        assert_eq_u32(ctx.gc->object_count, 0);
        assert_false(ctx.gc->gc_pending);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    // --- GC diagnostics tests ---
    printf("Testing GC heap stats...\n");
    {
        const char *spec = "wasm \
            types {[ fn [] [] ]} \
            funcs {[ 0 ]} \
            code {[ {[] end } ]}";
        assert_ok(wah_parse_module_from_spec(&module, spec));
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_gc_start(&ctx));
        wah_gc_alloc(&ctx, WAH_GC_KIND_NONE, 16);
        wah_gc_alloc(&ctx, WAH_GC_KIND_NONE, 32);
        wah_gc_heap_stats_t stats;
        wah_gc_heap_stats(&ctx, &stats);
        assert_eq_u32(stats.object_count, 2);
        assert_true(stats.allocated_bytes > 0);
        assert_true(stats.phase == WAH_GC_PHASE_IDLE);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Testing GC heap stats without GC state...\n");
    {
        const char *spec = "wasm \
            types {[ fn [] [] ]} \
            funcs {[ 0 ]} \
            code {[ {[] end } ]}";
        assert_ok(wah_parse_module_from_spec(&module, spec));
        assert_ok(wah_exec_context_create(&ctx, &module));
        wah_gc_heap_stats_t stats;
        wah_gc_heap_stats(&ctx, &stats);
        assert_eq_u32(stats.object_count, 0);
        assert_true(stats.allocated_bytes == 0);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Testing GC verify heap...\n");
    {
        const char *spec = "wasm \
            types {[ fn [] [] ]} \
            funcs {[ 0 ]} \
            code {[ {[] end } ]}";
        assert_ok(wah_parse_module_from_spec(&module, spec));
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_gc_start(&ctx));
        assert_true(wah_gc_verify_heap(&ctx));
        wah_gc_alloc(&ctx, WAH_GC_KIND_NONE, 16);
        wah_gc_alloc(&ctx, WAH_GC_KIND_NONE, 32);
        assert_true(wah_gc_verify_heap(&ctx));
        wah_gc_step(&ctx);
        assert_true(wah_gc_verify_heap(&ctx));
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Testing GC verify heap without GC state...\n");
    {
        const char *spec = "wasm \
            types {[ fn [] [] ]} \
            funcs {[ 0 ]} \
            code {[ {[] end } ]}";
        assert_ok(wah_parse_module_from_spec(&module, spec));
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_true(wah_gc_verify_heap(&ctx));
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    // --- Regression tests for root enumeration gaps ---

    printf("Testing root enumeration: funcref parameter is visited...\n");
    {
        wah_module_t env_mod = {0}, wasm_mod = {0};
        wah_exec_context_t ctx2 = {0};

        assert_ok(wah_new_module(&env_mod));
        wah_type_t i32_result[] = { WAH_TYPE_I32 };
        assert_ok(wah_module_export_funcv(&env_mod, "count_roots", 0, NULL, 1, i32_result, host_count_roots, NULL, NULL));

        const char *spec = "wasm \
            types {[ fn [funcref] [i32], fn [] [i32] ]} \
            imports {[ {'env'} {'count_roots'} fn# 1 ]} \
            funcs {[ 0 ]} \
            exports {[ {'test'} fn# 1 ]} \
            code {[ {[] call 0 end} ]}";
        assert_ok(wah_parse_module_from_spec(&wasm_mod, spec));
        assert_ok(wah_exec_context_create(&ctx2, &wasm_mod));
        assert_ok(wah_link_module(&ctx2, "env", &env_mod));
        assert_ok(wah_instantiate(&ctx2));

        wah_value_t args[1]; args[0].ref = NULL;
        wah_value_t result;
        assert_ok(wah_call(&ctx2, 1, args, 1, &result));
        assert_true(g_root_count >= 1);

        wah_exec_context_destroy(&ctx2);
        wah_free_module(&wasm_mod);
        wah_free_module(&env_mod);
    }

    printf("Testing root enumeration: operand-stack funcref is visited...\n");
    {
        wah_module_t env_mod = {0}, wasm_mod = {0};
        wah_exec_context_t ctx2 = {0};
        assert_ok(wah_new_module(&env_mod));
        wah_type_t i32_result[] = { WAH_TYPE_I32 };
        assert_ok(wah_module_export_funcv(&env_mod, "count_roots", 0, NULL, 1, i32_result, host_count_roots, NULL, NULL));

        // fn takes funcref param, pushes ref.null on operand stack, calls host
        // local 0 = funcref param, local 1 = i32 temp
        const char *spec = "wasm \
            types {[ fn [funcref] [i32], fn [] [i32] ]} \
            imports {[ {'env'} {'count_roots'} fn# 1 ]} \
            funcs {[ 0 ]} \
            exports {[ {'test'} fn# 1 ]} \
            code {[ {[1 i32] ref.null funcref call 0 local.set 1 drop local.get 1 end} ]}";
        assert_ok(wah_parse_module_from_spec(&wasm_mod, spec));
        assert_ok(wah_exec_context_create(&ctx2, &wasm_mod));
        assert_ok(wah_link_module(&ctx2, "env", &env_mod));
        assert_ok(wah_instantiate(&ctx2));

        wah_value_t args[1]; args[0].ref = NULL;
        wah_value_t result;
        assert_ok(wah_call(&ctx2, 1, args, 1, &result));
        // 1 param (funcref) + 1 operand stack (ref.null) = at least 2 ref roots
        assert_true(g_root_count >= 2);

        wah_exec_context_destroy(&ctx2);
        wah_free_module(&wasm_mod);
        wah_free_module(&env_mod);
    }

    printf("Testing root enumeration: linked-module funcref global is visited...\n");
    {
        // Primary module: no ref globals. Linked module: one funcref global.
        wah_module_t linked_mod = {0}, primary_mod = {0};
        wah_exec_context_t ctx2 = {0};

        const char *linked_spec = "wasm \
            types {[ fn [] [] ]} \
            funcs {[ 0 ]} \
            globals {[ funcref mut ref.null funcref end ]} \
            exports {[ {'g'} global# 0 ]} \
            code {[ {[] end } ]}";
        assert_ok(wah_parse_module_from_spec(&linked_mod, linked_spec));

        const char *primary_spec = "wasm \
            types {[ fn [] [] ]} \
            funcs {[ 0 ]} \
            code {[ {[] end } ]}";
        assert_ok(wah_parse_module_from_spec(&primary_mod, primary_spec));

        assert_ok(wah_exec_context_create(&ctx2, &primary_mod));
        assert_ok(wah_link_module(&ctx2, "linked", &linked_mod));
        assert_ok(wah_instantiate(&ctx2));

        uint32_t count = 0;
        wah_gc_enumerate_roots(&ctx2, count_ref_roots_visitor, &count);
        assert_eq_u32(count, 1);

        wah_exec_context_destroy(&ctx2);
        wah_free_module(&primary_mod);
        wah_free_module(&linked_mod);
    }

    printf("All GC tests passed.\n");
    return 0;
}
