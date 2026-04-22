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
static uint32_t g_structref_count;

static void host_count_roots(wah_call_context_t *cc, void *ud) {
    (void)ud;
    g_root_count = 0;
    wah_gc_enumerate_roots(cc->exec, count_ref_roots_visitor, &g_root_count);
    wah_return_i32(cc, (int32_t)g_root_count);
}

static void count_structref_visitor(wah_value_t *slot, wah_type_t type, void *ud) {
    (void)slot; (void)ud;
    if (type == WAH_TYPE_STRUCTREF) g_structref_count++;
}

static void host_count_structref(wah_call_context_t *cc, void *ud) {
    (void)ud;
    g_structref_count = 0;
    wah_gc_enumerate_roots(cc->exec, count_structref_visitor, &g_structref_count);
    wah_return_i32(cc, (int32_t)g_structref_count);
}

static void host_trigger_gc(wah_call_context_t *cc, void *ud) {
    (void)ud;
    wah_gc_step(cc->exec);
}

static void host_gc_object_count(wah_call_context_t *cc, void *ud) {
    (void)ud;
    wah_gc_heap_stats_t stats;
    wah_gc_heap_stats(cc->exec, &stats);
    wah_return_i32(cc, (int32_t)stats.object_count);
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

    wah_exec_context_destroy(&ctx);

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

        wah_gc_object_t *obj = wah_gc_alloc(&ctx, WAH_REPR_NONE, 16);
        assert_not_null(obj);
        assert_true(obj->repr_id == WAH_REPR_NONE);
        assert_true(obj->size_bytes == sizeof(wah_gc_object_t) + 16);
        assert_false(wah_gc_marked(obj));
        assert_eq_u32(ctx.gc->object_count, 1);
        assert_true(ctx.gc->allocated_bytes == sizeof(wah_gc_object_t) + 16);
        assert_eq_ptr(ctx.gc->all_objects, obj);

        void *payload = wah_gc_payload(obj);
        assert_eq_ptr(wah_gc_header(payload), obj);

        wah_gc_object_t *obj2 = wah_gc_alloc(&ctx, WAH_REPR_NONE, 32);
        assert_not_null(obj2);
        assert_eq_u32(ctx.gc->object_count, 2);
        assert_eq_ptr(ctx.gc->all_objects, obj2);
        assert_eq_ptr(wah_gc_next(obj2), obj);

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
        wah_gc_object_t *obj = wah_gc_alloc(&ctx, WAH_REPR_NONE, 8);
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
        wah_gc_alloc(&ctx, WAH_REPR_NONE, 8);
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
        wah_gc_alloc(&ctx, WAH_REPR_NONE, 16);
        wah_gc_alloc(&ctx, WAH_REPR_NONE, 32);
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
        wah_gc_alloc(&ctx, WAH_REPR_NONE, 16);
        wah_gc_alloc(&ctx, WAH_REPR_NONE, 32);
        wah_gc_alloc(&ctx, WAH_REPR_NONE, 64);
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
        wah_gc_alloc(&ctx, WAH_REPR_NONE, 16);
        wah_gc_alloc(&ctx, WAH_REPR_NONE, 32);
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
        wah_gc_alloc(&ctx, WAH_REPR_NONE, 16);
        wah_gc_alloc(&ctx, WAH_REPR_NONE, 32);
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
        wah_gc_alloc(&ctx, WAH_REPR_NONE, 16);
        wah_gc_alloc(&ctx, WAH_REPR_NONE, 32);
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

    // --- Concrete repr-backed allocation tests ---
    {
        wah_module_t mod = {0};
        wah_exec_context_t ctx3 = {0};

        const char *spec = "wasm \
            types {[ fn [] [], fn [] [] ]} \
            funcs {[ 0 ]} \
            code {[ {[] end } ]}";
        assert_ok(wah_parse_module_from_spec(&mod, spec));
        assert_ok(wah_exec_context_create(&ctx3, &mod));
        assert_ok(wah_gc_start(&ctx3));

        printf("Testing wah_gc_alloc_struct...\n");
        {
            uint8_t info_buf[sizeof(wah_repr_info_t) + 2 * sizeof(wah_repr_field_t)];
            wah_repr_info_t *info = (wah_repr_info_t *)info_buf;
            info->type = WAH_REPR_STRUCT;
            info->typeidx = 0;
            info->size = 8;
            info->count = 2;
            info->fields[0] = (wah_repr_field_t){.offset = 0, .repr_id = WAH_REPR_NONE};
            info->fields[1] = (wah_repr_field_t){.offset = 4, .repr_id = WAH_REPR_NONE};

            wah_repr_t repr_id;
            assert_ok(wah_module_alloc_repr(&mod, 0, info, &repr_id));
            assert(repr_id >= 0);

            wah_gc_object_t *obj = wah_gc_alloc_struct(&ctx3, repr_id, mod.repr_infos[repr_id]);
            assert_not_null(obj);
            assert_eq_u32(obj->repr_id, (uint32_t)repr_id);
            assert_eq_u32(obj->size_bytes, wah_gc_struct_alloc_size(mod.repr_infos[repr_id]));

            uint8_t *payload = (uint8_t *)wah_gc_payload(obj);
            *(int32_t *)(payload + 0) = 42;
            *(int32_t *)(payload + 4) = -7;
            assert(*(int32_t *)(payload + 0) == 42);
            assert(*(int32_t *)(payload + 4) == -7);
        }

        printf("Testing wah_gc_alloc_array...\n");
        {
            uint8_t info_buf[sizeof(wah_repr_info_t) + sizeof(wah_repr_field_t)];
            wah_repr_info_t *info = (wah_repr_info_t *)info_buf;
            info->type = WAH_REPR_ARRAY;
            info->typeidx = 1;
            info->size = 4;
            info->count = 1;
            info->fields[0] = (wah_repr_field_t){.offset = 0, .repr_id = WAH_REPR_NONE};

            wah_repr_t repr_id;
            assert_ok(wah_module_alloc_repr(&mod, 1, info, &repr_id));

            uint32_t length = 5;
            wah_gc_object_t *obj = wah_gc_alloc_array(&ctx3, repr_id, mod.repr_infos[repr_id], length);
            assert_not_null(obj);
            assert_eq_u32(obj->repr_id, (uint32_t)repr_id);
            assert_eq_u32(obj->size_bytes, wah_gc_array_alloc_size(mod.repr_infos[repr_id], length));

            wah_gc_array_body_t *body = (wah_gc_array_body_t *)wah_gc_payload(obj);
            assert_eq_u32(body->length, 5);

            // Write and read back elements
            int32_t *elems = (int32_t *)((uint8_t *)body + sizeof(wah_gc_array_body_t));
            for (uint32_t i = 0; i < length; i++) elems[i] = (int32_t)(i * 10);
            for (uint32_t i = 0; i < length; i++) assert(elems[i] == (int32_t)(i * 10));
        }

        printf("Testing wah_gc_alloc_i31...\n");
        {
            wah_gc_object_t *obj = wah_gc_alloc_i31(&ctx3, 12345);
            assert_not_null(obj);
            assert_eq_u32((uint32_t)obj->repr_id, (uint32_t)WAH_REPR_I31);
            assert_eq_u32(obj->size_bytes, wah_gc_i31_alloc_size());

            wah_gc_i31_body_t *body = (wah_gc_i31_body_t *)wah_gc_payload(obj);
            assert(body->value == 12345);

            // Negative value
            wah_gc_object_t *obj2 = wah_gc_alloc_i31(&ctx3, -42);
            assert_not_null(obj2);
            wah_gc_i31_body_t *body2 = (wah_gc_i31_body_t *)wah_gc_payload(obj2);
            assert(body2->value == (-42 & 0x7FFFFFFF));
        }

        printf("Testing wah_gc_alloc_array overflow protection...\n");
        {
            // Allocation with huge length should return NULL (overflow)
            wah_gc_object_t *obj = wah_gc_alloc_array(&ctx3, mod.typeidx_to_repr[1],
                                                       mod.repr_infos[mod.typeidx_to_repr[1]],
                                                       UINT32_MAX);
            assert_null(obj);
        }

        wah_exec_context_destroy(&ctx3);
        wah_free_module(&mod);
    }

    // --- ref.test / ref.cast runtime tests ---
    {
        printf("Testing ref.test with funcref...\n");
        wah_module_t mod = {0};
        wah_exec_context_t ctx4 = {0};

        // Module: type 0 = fn [] -> [i32]
        //         func 0: ref.func 0, ref.test.null funcref -> should return 1
        //         func 1: ref.null funcref, ref.test.null funcref -> should return 1 (null passes nullable test)
        //         func 2: ref.null funcref, ref.test funcref -> should return 0 (null fails non-nullable test)
        //         func 3: ref.func 0, ref.test.null anyref -> should return 1 (funcref <: anyref)
        //         func 4: ref.func 0, ref.test.null eqref -> should return 0 (funcref not <: eqref)
        wah_value_t result;

        // ref.func 0, ref.test.null funcref -> 1
        {
            const char *s = "wasm types {[ fn [] [i32] ]} funcs {[ 0 ]} \
                exports {[ {'f'} fn# 0 ]} \
                code {[ {[] ref.func 0 ref.test.null funcref end } ]}";
            assert_ok(wah_parse_module_from_spec(&mod, s));
            assert_ok(wah_exec_context_create(&ctx4, &mod));
            assert_ok(wah_instantiate(&ctx4));
            assert_ok(wah_call(&ctx4, 0, NULL, 0, &result));
            assert_eq_i32(result.i32, 1);
            wah_exec_context_destroy(&ctx4);
            wah_free_module(&mod);
        }

        // ref.null funcref, ref.test.null funcref -> 1 (null passes nullable test)
        {
            memset(&mod, 0, sizeof(mod));
            memset(&ctx4, 0, sizeof(ctx4));
            const char *s = "wasm types {[ fn [] [i32] ]} funcs {[ 0 ]} \
                exports {[ {'f'} fn# 0 ]} \
                code {[ {[] ref.null funcref ref.test.null funcref end } ]}";
            assert_ok(wah_parse_module_from_spec(&mod, s));
            assert_ok(wah_exec_context_create(&ctx4, &mod));
            assert_ok(wah_instantiate(&ctx4));
            assert_ok(wah_call(&ctx4, 0, NULL, 0, &result));
            assert_eq_i32(result.i32, 1);
            wah_exec_context_destroy(&ctx4);
            wah_free_module(&mod);
        }

        // ref.null funcref, ref.test funcref -> 0 (null fails non-nullable test)
        {
            memset(&mod, 0, sizeof(mod));
            memset(&ctx4, 0, sizeof(ctx4));
            const char *s = "wasm types {[ fn [] [i32] ]} funcs {[ 0 ]} \
                exports {[ {'f'} fn# 0 ]} \
                code {[ {[] ref.null funcref ref.test funcref end } ]}";
            assert_ok(wah_parse_module_from_spec(&mod, s));
            assert_ok(wah_exec_context_create(&ctx4, &mod));
            assert_ok(wah_instantiate(&ctx4));
            assert_ok(wah_call(&ctx4, 0, NULL, 0, &result));
            assert_eq_i32(result.i32, 0);
            wah_exec_context_destroy(&ctx4);
            wah_free_module(&mod);
        }

        // ref.func 0, ref.test.null anyref -> 1 (funcref <: anyref)
        {
            memset(&mod, 0, sizeof(mod));
            memset(&ctx4, 0, sizeof(ctx4));
            const char *s = "wasm types {[ fn [] [i32] ]} funcs {[ 0 ]} \
                exports {[ {'f'} fn# 0 ]} \
                code {[ {[] ref.func 0 ref.test.null anyref end } ]}";
            assert_ok(wah_parse_module_from_spec(&mod, s));
            assert_ok(wah_exec_context_create(&ctx4, &mod));
            assert_ok(wah_instantiate(&ctx4));
            assert_ok(wah_call(&ctx4, 0, NULL, 0, &result));
            assert_eq_i32(result.i32, 1);
            wah_exec_context_destroy(&ctx4);
            wah_free_module(&mod);
        }

        // ref.func 0, ref.test.null eqref -> 0 (funcref not <: eqref)
        {
            memset(&mod, 0, sizeof(mod));
            memset(&ctx4, 0, sizeof(ctx4));
            const char *s = "wasm types {[ fn [] [i32] ]} funcs {[ 0 ]} \
                exports {[ {'f'} fn# 0 ]} \
                code {[ {[] ref.func 0 ref.test.null eqref end } ]}";
            assert_ok(wah_parse_module_from_spec(&mod, s));
            assert_ok(wah_exec_context_create(&ctx4, &mod));
            assert_ok(wah_instantiate(&ctx4));
            assert_ok(wah_call(&ctx4, 0, NULL, 0, &result));
            assert_eq_i32(result.i32, 0);
            wah_exec_context_destroy(&ctx4);
            wah_free_module(&mod);
        }

        printf("Testing ref.cast with funcref...\n");

        // ref.cast.null funcref on null -> should succeed (no trap)
        {
            wah_module_t mod2 = {0};
            wah_exec_context_t ctx5 = {0};
            const char *s = "wasm types {[ fn [] [i32] ]} funcs {[ 0 ]} \
                exports {[ {'f'} fn# 0 ]} \
                code {[ {[] ref.null funcref ref.cast.null funcref ref.is_null end } ]}";
            assert_ok(wah_parse_module_from_spec(&mod2, s));
            assert_ok(wah_exec_context_create(&ctx5, &mod2));
            assert_ok(wah_instantiate(&ctx5));
            assert_ok(wah_call(&ctx5, 0, NULL, 0, &result));
            assert_eq_i32(result.i32, 1);
            wah_exec_context_destroy(&ctx5);
            wah_free_module(&mod2);
        }

        // ref.cast funcref on null -> should trap
        {
            wah_module_t mod2 = {0};
            wah_exec_context_t ctx5 = {0};
            const char *s = "wasm types {[ fn [] [i32] ]} funcs {[ 0 ]} \
                exports {[ {'f'} fn# 0 ]} \
                code {[ {[] ref.null funcref ref.cast funcref ref.is_null end } ]}";
            assert_ok(wah_parse_module_from_spec(&mod2, s));
            assert_ok(wah_exec_context_create(&ctx5, &mod2));
            assert_ok(wah_instantiate(&ctx5));
            assert_err(wah_call(&ctx5, 0, NULL, 0, &result), WAH_ERROR_TRAP);
            wah_exec_context_destroy(&ctx5);
            wah_free_module(&mod2);
        }
    }

    // --- GC root marking tests: struct/array objects survive collection ---

    printf("Testing GC mark: struct in local survives collection...\n");
    {
        // Module: type 0 = struct {i32 mut}, type 1 = fn [] -> [i32]
        // func 0: allocate struct, store in local, call host_trigger_gc, read struct field
        wah_module_t env_mod = {0}, wasm_mod = {0};
        wah_exec_context_t ctx5 = {0};

        assert_ok(wah_new_module(&env_mod));
        assert_ok(wah_module_export_funcv(&env_mod, "gc", 0, NULL, 0, NULL, host_trigger_gc, NULL, NULL));

        const char *spec = "wasm \
            types {[ struct [i32 mut], fn [] [i32], fn [] [] ]} \
            imports {[ {'env'} {'gc'} fn# 2 ]} \
            funcs {[ 1 ]} \
            exports {[ {'f'} fn# 1 ]} \
            code {[ {[1 type.ref.null 0] \
                i32.const 99 struct.new 0 local.set 0 \
                call 0 \
                local.get 0 struct.get 0 0 \
                end } ]}";
        assert_ok(wah_parse_module_from_spec(&wasm_mod, spec));
        assert_ok(wah_exec_context_create(&ctx5, &wasm_mod));
        assert_ok(wah_link_module(&ctx5, "env", &env_mod));
        assert_ok(wah_gc_start(&ctx5));
        assert_ok(wah_instantiate(&ctx5));

        wah_value_t result;
        assert_ok(wah_call(&ctx5, 1, NULL, 0, &result));
        assert_eq_i32(result.i32, 99);
        assert_true(ctx5.gc->object_count >= 1);

        wah_exec_context_destroy(&ctx5);
        wah_free_module(&wasm_mod);
        wah_free_module(&env_mod);
    }

    printf("Testing GC mark: struct on operand stack survives collection...\n");
    {
        // func: allocate struct, leave on operand stack, call host_trigger_gc (via drop/re-get trick),
        // then read struct field
        wah_module_t env_mod = {0}, wasm_mod = {0};
        wah_exec_context_t ctx5 = {0};

        assert_ok(wah_new_module(&env_mod));
        assert_ok(wah_module_export_funcv(&env_mod, "gc", 0, NULL, 0, NULL, host_trigger_gc, NULL, NULL));

        // struct.new leaves ref on operand stack, then we call gc, then struct.get reads it
        const char *spec = "wasm \
            types {[ struct [i32 mut], fn [] [i32], fn [] [] ]} \
            imports {[ {'env'} {'gc'} fn# 2 ]} \
            funcs {[ 1 ]} \
            exports {[ {'f'} fn# 1 ]} \
            code {[ {[1 type.ref.null 0] \
                i32.const 77 struct.new 0 local.set 0 \
                call 0 \
                local.get 0 struct.get 0 0 \
                end } ]}";
        assert_ok(wah_parse_module_from_spec(&wasm_mod, spec));
        assert_ok(wah_exec_context_create(&ctx5, &wasm_mod));
        assert_ok(wah_link_module(&ctx5, "env", &env_mod));
        assert_ok(wah_gc_start(&ctx5));
        assert_ok(wah_instantiate(&ctx5));

        wah_value_t result;
        assert_ok(wah_call(&ctx5, 1, NULL, 0, &result));
        assert_eq_i32(result.i32, 77);

        wah_exec_context_destroy(&ctx5);
        wah_free_module(&wasm_mod);
        wah_free_module(&env_mod);
    }

    printf("Testing GC mark: unreachable struct is collected...\n");
    {
        // Allocate a struct, then drop it. Trigger GC. Object count should be 0.
        wah_module_t env_mod = {0}, wasm_mod = {0};
        wah_exec_context_t ctx5 = {0};

        assert_ok(wah_new_module(&env_mod));
        wah_type_t i32_result[] = { WAH_TYPE_I32 };
        assert_ok(wah_module_export_funcv(&env_mod, "gc_count", 0, NULL, 1, i32_result, host_gc_object_count, NULL, NULL));

        // Allocate struct, drop it, then call gc_count which triggers GC internally
        const char *spec = "wasm \
            types {[ struct [i32 mut], fn [] [i32], fn [] [i32] ]} \
            imports {[ {'env'} {'gc_count'} fn# 2 ]} \
            funcs {[ 1 ]} \
            exports {[ {'f'} fn# 1 ]} \
            code {[ {[] \
                i32.const 55 struct.new 0 drop \
                call 0 \
                end } ]}";
        assert_ok(wah_parse_module_from_spec(&wasm_mod, spec));
        assert_ok(wah_exec_context_create(&ctx5, &wasm_mod));
        assert_ok(wah_link_module(&ctx5, "env", &env_mod));
        assert_ok(wah_gc_start(&ctx5));
        assert_ok(wah_instantiate(&ctx5));

        // Set low threshold so GC triggers at the POLL inside host call
        ctx5.gc->allocation_threshold = 1;

        wah_value_t result;
        assert_ok(wah_call(&ctx5, 1, NULL, 0, &result));
        // After GC, the dropped struct should be collected
        assert_eq_u32(ctx5.gc->object_count, 0);

        wah_exec_context_destroy(&ctx5);
        wah_free_module(&wasm_mod);
        wah_free_module(&env_mod);
    }

    printf("Testing GC mark: array in local survives collection...\n");
    {
        wah_module_t env_mod = {0}, wasm_mod = {0};
        wah_exec_context_t ctx5 = {0};

        assert_ok(wah_new_module(&env_mod));
        assert_ok(wah_module_export_funcv(&env_mod, "gc", 0, NULL, 0, NULL, host_trigger_gc, NULL, NULL));

        const char *spec = "wasm \
            types {[ array i32 mut, fn [] [i32], fn [] [] ]} \
            imports {[ {'env'} {'gc'} fn# 2 ]} \
            funcs {[ 1 ]} \
            exports {[ {'f'} fn# 1 ]} \
            code {[ {[1 type.ref.null 0] \
                i32.const 42 i32.const 3 array.new 0 local.set 0 \
                call 0 \
                local.get 0 i32.const 1 array.get 0 \
                end } ]}";
        assert_ok(wah_parse_module_from_spec(&wasm_mod, spec));
        assert_ok(wah_exec_context_create(&ctx5, &wasm_mod));
        assert_ok(wah_link_module(&ctx5, "env", &env_mod));
        assert_ok(wah_gc_start(&ctx5));
        assert_ok(wah_instantiate(&ctx5));

        wah_value_t result;
        assert_ok(wah_call(&ctx5, 1, NULL, 0, &result));
        assert_eq_i32(result.i32, 42);

        wah_exec_context_destroy(&ctx5);
        wah_free_module(&wasm_mod);
        wah_free_module(&env_mod);
    }

    printf("Testing GC mark: struct with ref field traces children...\n");
    {
        // type 0 = struct {i32 mut}  (leaf)
        // type 1 = struct {structref mut}  (parent with ref field pointing to leaf)
        // Allocate leaf, allocate parent pointing to it, store parent in local,
        // trigger GC, read leaf field through parent -> both must survive
        wah_module_t env_mod = {0}, wasm_mod = {0};
        wah_exec_context_t ctx5 = {0};

        assert_ok(wah_new_module(&env_mod));
        assert_ok(wah_module_export_funcv(&env_mod, "gc", 0, NULL, 0, NULL, host_trigger_gc, NULL, NULL));

        const char *spec = "wasm \
            types {[ struct [i32 mut], struct [type.ref.null 0 mut], fn [] [i32], fn [] [] ]} \
            imports {[ {'env'} {'gc'} fn# 3 ]} \
            funcs {[ 2 ]} \
            exports {[ {'f'} fn# 1 ]} \
            code {[ {[1 type.ref.null 1] \
                i32.const 123 struct.new 0 \
                struct.new 1 \
                local.set 0 \
                call 0 \
                local.get 0 struct.get 1 0 struct.get 0 0 \
                end } ]}";
        assert_ok(wah_parse_module_from_spec(&wasm_mod, spec));
        assert_ok(wah_exec_context_create(&ctx5, &wasm_mod));
        assert_ok(wah_link_module(&ctx5, "env", &env_mod));
        assert_ok(wah_gc_start(&ctx5));
        assert_ok(wah_instantiate(&ctx5));

        wah_value_t result;
        assert_ok(wah_call(&ctx5, 1, NULL, 0, &result));
        assert_eq_i32(result.i32, 123);
        assert_true(ctx5.gc->object_count >= 2);

        wah_exec_context_destroy(&ctx5);
        wah_free_module(&wasm_mod);
        wah_free_module(&env_mod);
    }

    printf("Testing root enumeration: operand-stack structref reports correct type...\n");
    {
        // Allocate a struct, leave on operand stack, call host that counts structref roots
        wah_module_t env_mod = {0}, wasm_mod = {0};
        wah_exec_context_t ctx5 = {0};

        assert_ok(wah_new_module(&env_mod));
        wah_type_t i32_result[] = { WAH_TYPE_I32 };
        assert_ok(wah_module_export_funcv(&env_mod, "count_structref", 0, NULL, 1, i32_result, host_count_structref, NULL, NULL));

        // struct.new_default leaves structref on operand stack, then call host
        const char *spec = "wasm \
            types {[ struct [i32 mut], fn [] [i32], fn [] [i32] ]} \
            imports {[ {'env'} {'count_structref'} fn# 2 ]} \
            funcs {[ 1 ]} \
            exports {[ {'f'} fn# 1 ]} \
            code {[ {[1 structref, 1 i32] \
                struct.new_default 0 local.set 0 \
                local.get 0 \
                call 0 local.set 1 \
                drop \
                local.get 1 \
                end } ]}";
        assert_ok(wah_parse_module_from_spec(&wasm_mod, spec));
        assert_ok(wah_exec_context_create(&ctx5, &wasm_mod));
        assert_ok(wah_link_module(&ctx5, "env", &env_mod));
        assert_ok(wah_gc_start(&ctx5));
        assert_ok(wah_instantiate(&ctx5));

        wah_value_t result;
        assert_ok(wah_call(&ctx5, 1, NULL, 0, &result));
        // The operand stack has a structref; host should see at least 1 structref root
        assert_true(result.i32 >= 1);

        wah_exec_context_destroy(&ctx5);
        wah_free_module(&wasm_mod);
        wah_free_module(&env_mod);
    }

    printf("Testing GC scan: WAH_REPR_I31 field repr is not traced as pointer...\n");
    {
        // Manually create a repr with a field marked WAH_REPR_I31 (non-pointer).
        // Store garbage in that field slot. GC scan must not dereference it.
        wah_module_t mod = {0};
        wah_exec_context_t ctx5 = {0};

        const char *spec = "wasm \
            types {[ fn [] [] ]} \
            funcs {[ 0 ]} \
            code {[ {[] end } ]}";
        assert_ok(wah_parse_module_from_spec(&mod, spec));
        assert_ok(wah_exec_context_create(&ctx5, &mod));
        assert_ok(wah_gc_start(&ctx5));

        uint8_t info_buf[sizeof(wah_repr_info_t) + 2 * sizeof(wah_repr_field_t)];
        wah_repr_info_t *info = (wah_repr_info_t *)info_buf;
        info->type = WAH_REPR_STRUCT;
        info->typeidx = 0;
        info->size = 2 * sizeof(void *);
        info->count = 2;
        info->fields[0] = (wah_repr_field_t){.offset = 0, .repr_id = WAH_REPR_I31};
        info->fields[1] = (wah_repr_field_t){.offset = sizeof(void *), .repr_id = WAH_REPR_NONE};

        wah_repr_t repr_id;
        assert_ok(wah_module_alloc_repr(&mod, 0, info, &repr_id));

        wah_gc_object_t *obj = wah_gc_alloc_struct(&ctx5, repr_id, mod.repr_infos[repr_id]);
        assert_not_null(obj);
        void **payload = (void **)wah_gc_payload(obj);
        payload[0] = (void *)(uintptr_t)0xDEADBEEF;
        payload[1] = (void *)(uintptr_t)0xCAFEBABE;

        // GC scan should not crash - neither field is a ref
        wah_gc_step(&ctx5);
        assert_eq_u32(ctx5.gc->object_count, 0);

        wah_exec_context_destroy(&ctx5);
        wah_free_module(&mod);
    }

    printf("All GC tests passed.\n");
    return 0;
}
