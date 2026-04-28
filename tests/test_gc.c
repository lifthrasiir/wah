#define WAH_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include "../wah.h"
#include "common.h"

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
    assert_false(wah_is_interrupted(&ctx));
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
    assert_false(wah_is_interrupted(&ctx));
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
    { wah_value_t result; assert_ok(wah_call(&ctx, 0, NULL, 0, &result)); }
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
        WAH_POLL_FLAG_STORE(ctx.poll_flag, 1);
        wah_value_t r;
        assert_ok(wah_call(&ctx, 0, NULL, 0, &r));
        assert_eq_i32(r.i32, 2);
        assert_false(ctx.gc->gc_pending);
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
    }

    printf("Testing POLL with interrupt flag (yields)...\n");
    {
        const char *spec = "wasm \
            types {[ fn [] [i32] ]} \
            funcs {[ 0 ]} \
            code {[ {[] i32.const 3 end } ]}";
        assert_ok(wah_parse_module_from_spec(&module, spec));
        assert_ok(wah_exec_context_create(&ctx, &module));
        assert_ok(wah_gc_start(&ctx));
        assert_ok(wah_instantiate(&ctx));
        wah_request_interrupt(&ctx);
        wah_value_t r;
        assert_err(wah_call(&ctx, 0, NULL, 0, &r), WAH_STATUS_YIELDED);
        assert_false(wah_is_interrupted(&ctx));
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

        void *obj = wah_gc_alloc(&ctx, WAH_REPR_NONE, 16);
        assert_not_null(obj);
        wah_gc_object_t *hdr = wah_gc_header(obj);
        assert_true(hdr->repr_id == WAH_REPR_NONE);
        assert_true(hdr->size_bytes == sizeof(wah_gc_object_t) + 16);
        assert_false(wah_gc_marked(hdr));
        assert_eq_u32(ctx.gc->object_count, 1);
        assert_true(ctx.gc->allocated_bytes == sizeof(wah_gc_object_t) + 16);
        assert_eq_ptr(ctx.gc->all_objects, hdr);

        assert_eq_ptr(wah_gc_payload(hdr), obj);
        assert_eq_ptr(wah_gc_header(obj), hdr);

        void *obj2 = wah_gc_alloc(&ctx, WAH_REPR_NONE, 32);
        assert_not_null(obj2);
        wah_gc_object_t *hdr2 = wah_gc_header(obj2);
        assert_eq_u32(ctx.gc->object_count, 2);
        assert_eq_ptr(ctx.gc->all_objects, hdr2);
        assert_eq_ptr(wah_gc_next(hdr2), hdr);

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
        void *obj = wah_gc_alloc(&ctx, WAH_REPR_NONE, 8);
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
        WAH_POLL_FLAG_STORE(ctx.poll_flag, 1);
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
        assert_ok(wah_module_export_func(&env_mod, "count_roots", "() -> i32", host_count_roots, NULL, NULL));

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
        assert_ok(wah_module_export_func(&env_mod, "count_roots", "() -> i32", host_count_roots, NULL, NULL));

        // fn takes funcref param, pushes ref.null on operand stack, calls host
        // local 0 = funcref param, local 1 = i32 temp
        // NOTE: this relies on POLL being emitted before CALL (so ref_map_offset
        // is current when the host function calls wah_gc_enumerate_roots).
        // If POLL is optimized to skip the ref_map_offset store, this test breaks.
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

            void *obj = wah_gc_alloc_struct(&ctx3, repr_id, mod.repr_infos[repr_id]);
            assert_not_null(obj);
            wah_gc_object_t *hdr = wah_gc_header(obj);
            assert_eq_u32(hdr->repr_id, (uint32_t)repr_id);
            assert_eq_u32(hdr->size_bytes, wah_gc_struct_alloc_size(mod.repr_infos[repr_id]));

            uint8_t *payload = (uint8_t *)obj;
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
            void *obj = wah_gc_alloc_array(&ctx3, repr_id, mod.repr_infos[repr_id], length);
            assert_not_null(obj);
            wah_gc_object_t *hdr = wah_gc_header(obj);
            assert_eq_u32(hdr->repr_id, (uint32_t)repr_id);
            assert_eq_u32(hdr->size_bytes, wah_gc_array_alloc_size(mod.repr_infos[repr_id], length));

            wah_gc_array_body_t *body = (wah_gc_array_body_t *)obj;
            assert_eq_u32(body->length, 5);

            // Write and read back elements
            int32_t *elems = (int32_t *)((uint8_t *)body + sizeof(wah_gc_array_body_t));
            for (uint32_t i = 0; i < length; i++) elems[i] = (int32_t)(i * 10);
            for (uint32_t i = 0; i < length; i++) assert(elems[i] == (int32_t)(i * 10));
        }

        printf("Testing i31 tagged pointers...\n");
        {
            void *ref = wah_ref_make_i31(12345);
            assert(wah_ref_is_i31(ref));
            assert(wah_ref_i31_get_u(ref) == 12345);
            assert(wah_ref_i31_get_s(ref) == 12345);

            void *ref2 = wah_ref_make_i31(-42);
            assert(wah_ref_is_i31(ref2));
            assert(wah_ref_i31_get_u(ref2) == (-42 & 0x7FFFFFFF));
            assert(wah_ref_i31_get_s(ref2) == -42);

            // Same value produces same tagged pointer
            void *ref3 = wah_ref_make_i31(12345);
            assert(ref == ref3);

            // NULL is not i31
            assert(!wah_ref_is_i31(NULL));
        }

        printf("Testing wah_gc_alloc_array overflow protection...\n");
        {
            // Allocation with huge length should return NULL (overflow)
            void *obj = wah_gc_alloc_array(&ctx3, mod.typeidx_to_repr[1],
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

        // ref.func 0, ref.test.null anyref -> 0 (funcref not <: anyref in GC spec)
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
            assert_eq_i32(result.i32, 0);
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
        assert_ok(wah_module_export_func(&env_mod, "gc", "()", host_trigger_gc, NULL, NULL));

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
        assert_ok(wah_module_export_func(&env_mod, "gc", "()", host_trigger_gc, NULL, NULL));

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
        assert_ok(wah_module_export_func(&env_mod, "gc_count", "() -> i32", host_gc_object_count, NULL, NULL));

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
        assert_ok(wah_module_export_func(&env_mod, "gc", "()", host_trigger_gc, NULL, NULL));

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
        assert_ok(wah_module_export_func(&env_mod, "gc", "()", host_trigger_gc, NULL, NULL));

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
        assert_ok(wah_module_export_func(&env_mod, "count_structref", "() -> i32", host_count_structref, NULL, NULL));

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
        info->fields[0] = (wah_repr_field_t){.offset = 0, .repr_id = WAH_REPR_REF};
        info->fields[1] = (wah_repr_field_t){.offset = sizeof(void *), .repr_id = WAH_REPR_NONE};

        wah_repr_t repr_id;
        assert_ok(wah_module_alloc_repr(&mod, 0, info, &repr_id));

        void *obj = wah_gc_alloc_struct(&ctx5, repr_id, mod.repr_infos[repr_id]);
        assert_not_null(obj);
        void **payload = (void **)obj;
        payload[0] = wah_ref_make_i31(0xDEADBEEF);
        payload[1] = (void *)(uintptr_t)0xCAFEBABE;

        // GC scan should not crash
        wah_gc_step(&ctx5);
        assert_eq_u32(ctx5.gc->object_count, 0);

        wah_exec_context_destroy(&ctx5);
        wah_free_module(&mod);
    }

    printf("Testing GC mark: struct inside exnref survives collection...\n");
    {
        // Throw exception carrying a (ref null $0) struct, catch with catch,
        // store the struct ref in a local, trigger GC via host call, then
        // struct.get the field to verify the struct survived collection.
        // The exception's value_types metadata allows GC to trace the ref
        // inside the pending exception during collection.
        //
        // types:
        //   0: struct [i32 mut]
        //   1: fn [type.ref.null 0] []   -- tag: carries (ref null $0)
        //   2: fn [] [i32]               -- main func
        //   3: fn [] []                  -- host gc
        wah_module_t env_mod = {0}, wasm_mod = {0};
        wah_exec_context_t ctx5 = {0};

        assert_ok(wah_new_module(&env_mod));
        assert_ok(wah_module_export_func(&env_mod, "gc", "()", host_trigger_gc, NULL, NULL));

        const char *spec = "wasm \
            types {[ struct [i32 mut], fn [type.ref.null 0] [], fn [] [i32], fn [] [], fn [] [type.ref.null 0] ]} \
            imports {[ {'env'} {'gc'} fn# 3 ]} \
            funcs {[ 2 ]} \
            tags {[ tag.type# 1 ]} \
            exports {[ {'f'} fn# 1 ]} \
            code {[ {[1 type.ref.null 0] \
                block 4 \
                    try_table void [catch 0 0] \
                        i32.const 42 struct.new 0 \
                        throw 0 \
                    end \
                    ref.null 0 \
                end \
                local.set 0 \
                call 0 \
                local.get 0 \
                struct.get 0 0 \
            end } ]}";
        assert_ok(wah_parse_module_from_spec(&wasm_mod, spec));
        assert_ok(wah_exec_context_create(&ctx5, &wasm_mod));
        assert_ok(wah_link_module(&ctx5, "env", &env_mod));
        assert_ok(wah_gc_start(&ctx5));
        assert_ok(wah_instantiate(&ctx5));

        ctx5.gc->allocation_threshold = 1;

        wah_value_t result;
        assert_ok(wah_call(&ctx5, 1, NULL, 0, &result));
        assert_eq_i32(result.i32, 42);

        wah_exec_context_destroy(&ctx5);
        wah_free_module(&wasm_mod);
        wah_free_module(&env_mod);
    }

    printf("Testing GC mark: pending exception refs survive collection...\n");
    {
        // Throw exception carrying a GC struct from a callee function.
        // The exception propagates through the call stack as pending_exception.
        // The outer catch catches it and reads the struct field.
        // GC is triggered via low threshold during the call sequence.
        //
        // types:
        //   0: struct [i32 mut]
        //   1: fn [type.ref.null 0] []   -- tag: carries (ref null $0)
        //   2: fn [] [i32]               -- main func
        //   3: fn [] []                  -- host gc / thrower
        //   4: fn [] [type.ref.null 0]   -- block type for catch
        wah_module_t env_mod = {0}, wasm_mod = {0};
        wah_exec_context_t ctx5 = {0};

        assert_ok(wah_new_module(&env_mod));
        assert_ok(wah_module_export_func(&env_mod, "gc", "()", host_trigger_gc, NULL, NULL));

        const char *spec = "wasm \
            types {[ struct [i32 mut], fn [type.ref.null 0] [], fn [] [i32], fn [] [], fn [] [type.ref.null 0] ]} \
            imports {[ {'env'} {'gc'} fn# 3 ]} \
            funcs {[ 2, 3 ]} \
            tags {[ tag.type# 1 ]} \
            exports {[ {'f'} fn# 1 ]} \
            code {[ \
                {[] \
                    block 4 \
                        try_table void [catch 0 0] \
                            call 2 \
                        end \
                        ref.null 0 \
                    end \
                    struct.get 0 0 \
                end }, \
                {[] \
                    i32.const 99 struct.new 0 \
                    throw 0 \
                end } \
            ]}";
        assert_ok(wah_parse_module_from_spec(&wasm_mod, spec));
        assert_ok(wah_exec_context_create(&ctx5, &wasm_mod));
        assert_ok(wah_link_module(&ctx5, "env", &env_mod));
        assert_ok(wah_gc_start(&ctx5));
        assert_ok(wah_instantiate(&ctx5));

        ctx5.gc->allocation_threshold = 1;

        wah_value_t result;
        assert_ok(wah_call(&ctx5, 1, NULL, 0, &result));
        assert_eq_i32(result.i32, 99);

        wah_exec_context_destroy(&ctx5);
        wah_free_module(&wasm_mod);
        wah_free_module(&env_mod);
    }

    printf("Testing GC mark: nested exnref payload traces refs...\n");
    {
        wah_module_t wasm_mod = {0};
        wah_exec_context_t ctx5 = {0};

        const char *spec = "wasm \
            types {[ struct [i32 mut], fn [] [] ]} \
            funcs {[ 1 ]} \
            code {[ {[] end } ]}";

        assert_ok(wah_parse_module_from_spec(&wasm_mod, spec));
        assert_ok(wah_exec_context_create(&ctx5, &wasm_mod));
        assert_ok(wah_gc_start(&ctx5));
        assert_ok(wah_instantiate(&ctx5));

        wah_repr_t struct_repr = wasm_mod.typeidx_to_repr[0];
        const wah_repr_info_t *struct_info = wasm_mod.repr_infos[struct_repr];
        void *struct_ref = wah_gc_alloc_struct(&ctx5, struct_repr, struct_info);
        assert_not_null(struct_ref);
        wah_value_t field_value = { .i32 = 123 };
        wah_gc_store_field(WAH_TYPE_I32, (uint8_t *)struct_ref + struct_info->fields[0].offset, &field_value);

        wah_exception_t *inner = (wah_exception_t *)calloc(1, sizeof(*inner));
        wah_exception_t *outer = (wah_exception_t *)calloc(1, sizeof(*outer));
        assert_not_null(inner);
        assert_not_null(outer);
        inner->value_count = 1;
        inner->values = (wah_value_t *)calloc(1, sizeof(wah_value_t));
        inner->value_types = (wah_type_t *)calloc(1, sizeof(wah_type_t));
        outer->value_count = 1;
        outer->values = (wah_value_t *)calloc(1, sizeof(wah_value_t));
        outer->value_types = (wah_type_t *)calloc(1, sizeof(wah_type_t));
        assert_not_null(inner->values);
        assert_not_null(inner->value_types);
        assert_not_null(outer->values);
        assert_not_null(outer->value_types);

        inner->values[0].ref = struct_ref;
        inner->value_types[0] = 0; // (ref null type 0), the struct type.
        outer->values[0].ref = inner;
        outer->value_types[0] = WAH_TYPE_EXNREF;
        wah_exception_track(&ctx5, inner);
        wah_exception_track(&ctx5, outer);
        ctx5.pending_exception = outer;

        wah_gc_step(&ctx5);
        assert_eq_u32(ctx5.gc->object_count, 1);
        assert_true(wah_gc_verify_heap(&ctx5));

        ctx5.pending_exception = NULL;
        wah_exception_free_all(&ctx5);
        wah_gc_step(&ctx5);
        assert_eq_u32(ctx5.gc->object_count, 0);

        wah_exec_context_destroy(&ctx5);
        wah_free_module(&wasm_mod);
    }

    printf("Testing pending exception replacement and destroy cleanup...\n");
    {
        wah_module_t wasm_mod = {0};
        wah_exec_context_t ctx5 = {0};

        const char *spec = "wasm \
            types {[ fn [] [] ]} \
            funcs {[ 0 ]} \
            code {[ {[] end } ]}";

        assert_ok(wah_parse_module_from_spec(&wasm_mod, spec));
        assert_ok(wah_exec_context_create(&ctx5, &wasm_mod));
        assert_ok(wah_gc_start(&ctx5));
        assert_ok(wah_instantiate(&ctx5));

        wah_exception_t *old_exc = (wah_exception_t *)calloc(1, sizeof(*old_exc));
        wah_exception_t *new_exc = (wah_exception_t *)calloc(1, sizeof(*new_exc));
        assert_not_null(old_exc);
        assert_not_null(new_exc);

        old_exc->value_count = 1;
        old_exc->values = (wah_value_t *)calloc(1, sizeof(wah_value_t));
        old_exc->value_types = (wah_type_t *)calloc(1, sizeof(wah_type_t));
        new_exc->value_count = 1;
        new_exc->values = (wah_value_t *)calloc(1, sizeof(wah_value_t));
        new_exc->value_types = (wah_type_t *)calloc(1, sizeof(wah_type_t));
        assert_not_null(old_exc->values);
        assert_not_null(old_exc->value_types);
        assert_not_null(new_exc->values);
        assert_not_null(new_exc->value_types);
        old_exc->values[0].i32 = 11;
        old_exc->value_types[0] = WAH_TYPE_I32;
        new_exc->values[0].i32 = 22;
        new_exc->value_types[0] = WAH_TYPE_I32;

        wah_exception_track(&ctx5, old_exc);
        wah_exception_track(&ctx5, new_exc);
        ctx5.pending_exception = old_exc;

        assert_err(wah_throw_exception(&ctx5, new_exc), WAH_ERROR_EXCEPTION);
        assert_eq_ptr(ctx5.pending_exception, new_exc);
        assert_eq_i32(ctx5.pending_exception->values[0].i32, 22);

        uint32_t tracked = 0;
        for (wah_exception_t *e = ctx5.exceptions; e; e = e->next) tracked++;
        assert_eq_u32(tracked, 1);

        wah_exec_context_destroy(&ctx5);
        wah_free_module(&wasm_mod);
    }

    printf("Testing GC mark: array ref elements survive collection...\n");
    {
        // Keep only the array itself in a local across a host-triggered GC.
        // The struct stored in array[0] is reachable only through
        // wah_gc_scan_object's array-element reference branch.
        wah_module_t env_mod = {0}, wasm_mod = {0};

        assert_ok(wah_new_module(&env_mod));
        assert_ok(wah_module_export_func(&env_mod, "gc", "()", host_trigger_gc, NULL, NULL));
        assert_ok(wah_module_export_func(&env_mod, "count", "() -> i32", host_gc_object_count, NULL, NULL));

        const char *spec = "wasm \
            types {[ struct [i32 mut], array type.ref.null 0 mut, fn [] [], fn [] [i32] ]} \
            imports {[ {'env'} {'gc'} fn# 2, {'env'} {'count'} fn# 3 ]} \
            funcs {[ 3 ]} \
            exports {[ {'f'} fn# 2 ]} \
            code {[ {[1 type.ref.null 1] \
                ref.null 0 \
                i32.const 1 \
                array.new 1 \
                local.set 0 \
                local.get 0 \
                i32.const 0 \
                i32.const 77 \
                struct.new 0 \
                array.set 1 \
                call 0 \
                local.get 0 \
                i32.const 0 \
                array.get 1 \
                struct.get 0 0 \
            end } ]}";

        assert_ok(wah_parse_module_from_spec(&wasm_mod, spec));
        wah_exec_context_t ctx6 = {0};
        assert_ok(wah_exec_context_create(&ctx6, &wasm_mod));
        assert_ok(wah_link_module(&ctx6, "env", &env_mod));
        assert_ok(wah_gc_start(&ctx6));
        assert_ok(wah_instantiate(&ctx6));

        ctx6.gc->allocation_threshold = 1;

        wah_value_t result;
        assert_ok(wah_call(&ctx6, 2, NULL, 0, &result));
        assert_eq_i32(result.i32, 77);

        assert_ok(wah_call(&ctx6, 0, NULL, 0, NULL));
        assert_ok(wah_call(&ctx6, 1, NULL, 0, &result));
        assert_eq_i32(result.i32, 0);

        wah_exec_context_destroy(&ctx6);
        wah_free_module(&wasm_mod);
        wah_free_module(&env_mod);
    }

    printf("Testing GC sweep unlink (dead object from middle of list)...\n");
    {
        wah_module_t env_mod = {0}, wasm_mod = {0};
        assert_ok(wah_new_module(&env_mod));
        assert_ok(wah_module_export_func(&env_mod, "gc", "()", host_trigger_gc, NULL, NULL));
        assert_ok(wah_module_export_func(&env_mod, "count", "() -> i32", host_gc_object_count, NULL, NULL));

        const char *spec = "wasm \
            types {[ struct [i32 mut], fn [] [], fn [] [i32] ]} \
            imports {[ {'env'} {'gc'} fn# 1, {'env'} {'count'} fn# 2 ]} \
            funcs {[ 2 ]} \
            code {[ {[1 type.ref.null 0] \
                i32.const 10 struct.new 0 \
                local.set 0 \
                i32.const 20 struct.new 0 \
                drop \
                call 0 \
                local.get 0 struct.get 0 0 \
            end } ]}";

        assert_ok(wah_parse_module_from_spec(&wasm_mod, spec));
        wah_exec_context_t ctx6 = {0};
        assert_ok(wah_exec_context_create(&ctx6, &wasm_mod));
        assert_ok(wah_link_module(&ctx6, "env", &env_mod));
        assert_ok(wah_gc_start(&ctx6));
        assert_ok(wah_instantiate(&ctx6));

        ctx6.gc->allocation_threshold = 1;

        wah_value_t result;
        assert_ok(wah_call(&ctx6, 2, NULL, 0, &result));
        assert_eq_i32(result.i32, 10);

        assert_ok(wah_call(&ctx6, 1, NULL, 0, &result));
        assert_eq_i32(result.i32, 1);

        wah_exec_context_destroy(&ctx6);
        wah_free_module(&wasm_mod);
        wah_free_module(&env_mod);
    }

    printf("Testing GC phase transitions (MARK -> SWEEP -> IDLE)...\n");
    {
        wah_module_t wasm_mod = {0};
        const char *spec = "wasm \
            types {[ struct [i32 mut], fn [] [i32] ]} \
            funcs {[ 1 ]} \
            code {[ {[] i32.const 42 struct.new 0 struct.get 0 0 end } ]}";
        assert_ok(wah_parse_module_from_spec(&wasm_mod, spec));
        wah_exec_context_t ctx7 = {0};
        assert_ok(wah_exec_context_create(&ctx7, &wasm_mod));
        assert_ok(wah_gc_start(&ctx7));
        assert_ok(wah_instantiate(&ctx7));

        wah_value_t result;
        assert_ok(wah_call(&ctx7, 0, NULL, 0, &result));
        assert_eq_i32(result.i32, 42);

        assert_not_null(ctx7.gc);
        assert_true(ctx7.gc->phase == WAH_GC_PHASE_IDLE);
        wah_gc_step(&ctx7);
        assert_true(ctx7.gc->phase == WAH_GC_PHASE_IDLE);

        wah_exec_context_destroy(&ctx7);
        wah_free_module(&wasm_mod);
    }

    printf("All GC tests passed.\n");
    return 0;
}
