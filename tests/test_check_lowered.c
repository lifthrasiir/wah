// The lowered code checker should accept what the lowering emits, and reject it once any part
// the interpreter relies on is broken.
#ifndef WAH_CHECK_LOWERED
#define WAH_CHECK_LOWERED
#endif
#define WAH_IMPLEMENTATION
#include "common.h"

// func 0 (a: i32, b: i32) -> i32 with an i64 local, which returns 25 for (8, 3); func 1 adds its params.
static const char spec[] = "wasm \
    types {[fn [i32, i32] [i32]]} funcs {[0, 0]} \
    tables {[funcref limits.i32/1 1]} memories {[limits.i32/1 1]} \
    globals {[i32 mut i32.const 7 end, i64 immut i64.const 1 end]} \
    elements {[elem.active.table#0 i32.const 0 end [1]]} \
    code {[ \
        {[1 i64] \
            local.get 0 i32.const 9 i32.store align=4 offset=0 \
            block void \
                block void \
                    local.get 1 br_table [0, 1] 1 \
                end \
                i32.const 5 global.set 0 \
            end \
            loop void \
                local.get 1 i32.const 1 i32.sub local.tee 1 br_if 0 \
            end \
            i64.const 3 local.set 2 \
            local.get 0 i32.load align=4 offset=0 \
            global.get 0 \
            local.get 0 if i32 i32.const 1 else i32.const 2 end \
            select \
            block i32 \
                local.get 0 local.get 1 call 1 \
                local.get 0 local.get 1 i32.const 0 call_indirect 0 0 \
                i32.add br 0 \
            end \
            i32.add \
            memory.size 0 drop \
            local.get 2 i32.wrap_i64 drop \
            return \
        end}, \
        {[] local.get 0 local.get 1 i32.add end} \
    ]}";

static wah_error_t check_func(const wah_module_t *mod, uint32_t i) {
    const wah_code_body_t *body = &mod->code_bodies[i];
    return wah_check_lowered_code(mod, &body->parsed_code, &mod->types[mod->function_type_indices[i]], body,
                                  body->max_stack_depth, WAH_TYPE_BOT);
}

// Returns the offset of the n-th instruction with `op`.
static uint32_t find_instr(const wah_parsed_code_t *pc, uint16_t op, uint32_t n) {
    wah_check_instr_t in;
    for (uint32_t off = 0; off < pc->bytecode_size; off += in.len) {
        assert_ok(wah_check_decode(pc->bytecode, pc->bytecode_size, off, &in));
        if (in.op == op && n-- == 0) return off;
    }
    assert_true(!"instruction not found");
    return 0;
}

// Overwrites a u8, u16 or u32 at `pos` of func 0, expects the check to fail, and restores it.
static void expect_rejected(wah_module_t *mod, uint32_t pos, uint32_t size, uint32_t value, const char *what) {
    printf("  %s\n", what);
    uint8_t *code = mod->code_bodies[0].parsed_code.bytecode, saved[4];
    memcpy(saved, code + pos, size);
    if (size == 1) code[pos] = (uint8_t)value;
    else if (size == 2) wah_write_u16_le(code + pos, (uint16_t)value);
    else wah_write_u32_le(code + pos, value);
    assert_err(check_func(mod, 0), WAH_ERROR_VALIDATION_FAILED);
    memcpy(code + pos, saved, size);
    assert_ok(check_func(mod, 0));
}

#define OP_AT(op, n) find_instr(pc, WAH_OP_##op, (n))
#define IMM_AT(op, n, i) (OP_AT(op, n) + sizeof(uint16_t) + (i) * sizeof(uint32_t))

static void test_accepted(void) {
    printf("Testing lowered code is accepted...\n");
    wah_module_t mod;
    wah_exec_context_t ctx;
    wah_value_t params[2] = { {.i32 = 8}, {.i32 = 3} }, result;
    assert_ok(wah_parse_module_from_spec(&mod, spec));
    assert_ok(check_func(&mod, 0));
    assert_ok(check_func(&mod, 1));
    assert_ok(wah_check_lowered_code(&mod, &mod.globals[0].init_expr, NULL, NULL, WAH_MAX_TYPE_STACK_SIZE, WAH_TYPE_I32));
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_call(&ctx, 0, params, 2, &result));
    assert_eq_i32(result.i32, 25);
    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

static void test_rejected(void) {
    printf("Testing broken lowered code is rejected...\n");
    wah_module_t mod;
    assert_ok(wah_parse_module_from_spec(&mod, spec));
    wah_code_body_t *body = &mod.code_bodies[0];
    const wah_parsed_code_t *pc = &body->parsed_code;

    expect_rejected(&mod, IMM_AT(LOCAL_GET, 0, 0), 4, 3, "local index out of range");
    expect_rejected(&mod, IMM_AT(LOCAL_SET, 0, 0), 4, 0, "local of a wrong type");
    expect_rejected(&mod, IMM_AT(GLOBAL_SET, 0, 0), 4, 1, "immutable global set");
    expect_rejected(&mod, OP_AT(GLOBAL_GET, 0), 2, WAH_OP_GLOBAL_GET_INDIRECT, "indirect access to a direct global");
    expect_rejected(&mod, IMM_AT(CALL, 0, 0), 4, 2, "function index out of range");
    expect_rejected(&mod, IMM_AT(CALL_INDIRECT, 0, 0), 4, 1, "type index out of range");
    expect_rejected(&mod, OP_AT(I32_ADD, 0), 2, WAH_OP_I64_ADD, "operands of a wrong type");
    expect_rejected(&mod, OP_AT(DROP, 0), 2, WAH_OP_NOP, "instruction that is never emitted");
    expect_rejected(&mod, OP_AT(DROP, 0), 2, WAH_NUM_OPCODES, "unknown opcode");
    expect_rejected(&mod, OP_AT(RETURN, 0), 2, WAH_OP_DROP, "returning without a result");
    expect_rejected(&mod, IMM_AT(BR, 0, 0), 4, OP_AT(BR, 0) + 1, "branch into the middle of an instruction");
    expect_rejected(&mod, IMM_AT(BR, 0, 0), 4, pc->bytecode_size, "branch out of the code");
    expect_rejected(&mod, IMM_AT(BR, 0, 1), 4, 1000, "branch keeping too many values");
    expect_rejected(&mod, IMM_AT(BR, 0, 2), 4, 1, "branch dropping a value the target expects");
    expect_rejected(&mod, IMM_AT(BR_IF, 0, 2), 4, 1, "branch dropping a value missing");
    expect_rejected(&mod, IMM_AT(IF, 0, 0), 4, 0, "branch to a target with another stack");

    printf("  br_table record dropping a value missing\n");
    uint32_t br_table = OP_AT(BR_TABLE, 0);
    uint32_t target_count = wah_read_u32_le(pc->bytecode + br_table + sizeof(uint16_t));
    uint32_t records = br_table + sizeof(uint16_t) + 2 * sizeof(uint32_t) + (target_count + 1) * sizeof(uint16_t);
    expect_rejected(&mod, records + sizeof(uint32_t), 4, 1, "  (drop of the first record)");
    expect_rejected(&mod, records, 4, records, "  (target of the first record)");

    printf("  truncated code\n");
    body->parsed_code.bytecode_size--;
    assert_err(check_func(&mod, 0), WAH_ERROR_VALIDATION_FAILED);
    body->parsed_code.bytecode_size++;

    printf("  operand stack deeper than reserved\n");
    body->max_stack_depth--;
    assert_err(check_func(&mod, 0), WAH_ERROR_VALIDATION_FAILED);
    body->max_stack_depth++;

    printf("  const expression of a wrong type\n");
    assert_err(wah_check_lowered_code(&mod, &mod.globals[0].init_expr, NULL, NULL, WAH_MAX_TYPE_STACK_SIZE, WAH_TYPE_I64),
               WAH_ERROR_VALIDATION_FAILED);

    assert_ok(check_func(&mod, 0));
    wah_free_module(&mod);
}

static void test_fuel_metering(void) {
    printf("Testing lowered code with fuel metering...\n");
    wah_parse_options_t opts = { .enable_fuel_metering = true };
    wah_module_t mod;
    assert_ok(wah_parse_module_from_spec_ex(&mod, &opts, spec));
    assert_ok(check_func(&mod, 0));
    const wah_parsed_code_t *pc = &mod.code_bodies[0].parsed_code;
    expect_rejected(&mod, OP_AT(METER, 0) + 2 * sizeof(uint16_t), 4, OP_AT(METER, 0) + 1, "slow island into an instruction");
    expect_rejected(&mod, IMM_AT(ELSE, 0, 0), 4, 0, "jump to a target with another stack");
    wah_free_module(&mod);
}

static void test_bulk_memory(void) {
    printf("Testing lowered bulk memory operations...\n");
    wah_module_t mod;
    wah_exec_context_t ctx;
    wah_value_t param = {.i32 = 4};
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [i32] []]} funcs {[0]} \
        tables {[funcref limits.i32/1 4, externref limits.i32/1 4]} memories {[limits.i32/1 1]} \
        elements {[elem.passive elem.funcref [0]]} \
        datacount {1} \
        code {[{[] \
            local.get 0 i32.const 1 i32.const 2 memory.fill 0 \
            i32.const 0 local.get 0 i32.const 2 memory.copy 0 0 \
            i32.const 8 i32.const 0 i32.const 2 memory.init 0 0 data.drop 0 \
            i32.const 0 i32.const 0 i32.const 1 table.init 0 0 elem.drop 0 \
            i32.const 1 i32.const 0 i32.const 1 table.copy 0 0 \
        end}]} \
        data {[data.passive {%'4142'}]}"));
    assert_ok(check_func(&mod, 0));
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_call(&ctx, 0, &param, 1, NULL));
    wah_free_exec_context(&ctx);

    const wah_parsed_code_t *pc = &mod.code_bodies[0].parsed_code;
    expect_rejected(&mod, IMM_AT(DATA_DROP, 0, 0), 4, 1, "data segment out of range");
    expect_rejected(&mod, IMM_AT(MEMORY_INIT, 0, 0), 4, 1, "data segment out of range to initialize from");
    expect_rejected(&mod, IMM_AT(ELEM_DROP, 0, 0), 4, 1, "element segment out of range");
    expect_rejected(&mod, IMM_AT(TABLE_INIT, 0, 1), 4, 1, "table of a wrong type to initialize");
    expect_rejected(&mod, IMM_AT(TABLE_COPY, 0, 0), 4, 1, "table of a wrong type to copy into");
    expect_rejected(&mod, IMM_AT(MEMORY_COPY, 0, 1), 4, 1, "memory out of range");
    expect_rejected(&mod, IMM_AT(MEMORY_FILL, 0, 0), 4, 1, "memory out of range to fill");
    wah_free_module(&mod);
}

static void test_references(void) {
    printf("Testing lowered reference operations...\n");
    wah_module_t mod;
    wah_exec_context_t ctx;
    wah_value_t param = {.i32 = 1}, result;
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [i32] [i32], fn [] [i32], fn [] []]} funcs {[0, 1, 2]} \
        tables {[funcref limits.i32/1 2, externref limits.i32/1 2]} \
        elements {[elem.declarative elem.funcref [1]]} \
        code {[ \
            {[] \
                i32.const 0 ref.func 1 table.set 0 \
                i32.const 0 table.get 0 ref.is_null drop \
                i32.const 1 ref.null externref table.set 1 \
                ref.func 1 call_ref 1 \
                block void ref.null funcref br_on_null 0 drop end \
                local.get 0 i32.add \
                return_call 1 \
            end}, \
            {[] i32.const 7 end}, \
            {[] end} \
        ]}"));
    assert_ok(check_func(&mod, 0));
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_call(&ctx, 0, &param, 1, &result));
    assert_eq_i32(result.i32, 7);
    wah_free_exec_context(&ctx);

    const wah_parsed_code_t *pc = &mod.code_bodies[0].parsed_code;
    expect_rejected(&mod, IMM_AT(TABLE_SET, 0, 0), 4, 1, "function into a table of externs");
    expect_rejected(&mod, IMM_AT(REF_NULL, 0, 0), 4, (uint32_t)WAH_TYPE_FUNC, "null function into a table of externs");
    expect_rejected(&mod, IMM_AT(REF_FUNC, 0, 0), 4, 3, "function out of range to refer to");
    expect_rejected(&mod, IMM_AT(CALL_REF, 0, 0), 4, 3, "type out of range to call by reference");
    expect_rejected(&mod, IMM_AT(BR_ON_NULL, 0, 1), 4, 5, "branch on null keeping too many values");
    expect_rejected(&mod, IMM_AT(RETURN_CALL, 0, 0), 4, 2, "tail call returning other results");
    wah_free_module(&mod);
}

static void test_simd(void) {
    printf("Testing lowered SIMD operations...\n");
    wah_module_t mod;
    wah_exec_context_t ctx;
    wah_value_t param = {.i32 = 0x01020304}, result;
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [i32] [i32]]} funcs {[0]} memories {[limits.i32/1 1]} \
        code {[{[] \
            i32.const 16 local.get 0 i32x4.splat v128.store align=1 offset=0 \
            i32.const 16 v128.const %'000102030405060708090a0b0c0d0e0f' v128.load8_lane align=1 offset=0 3 \
            local.get 0 i32x4.splat \
            i8x16.shuffle %'00100111021203130414051506160717' \
            i32.const 7 i32x4.replace_lane 2 \
            i8x16.extract_lane_s 3 \
        end}]}"));
    assert_ok(check_func(&mod, 0));
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_call(&ctx, 0, &param, 1, &result));
    assert_eq_i32(result.i32, 3); // Lane 3 of the shuffle is byte 1 of the splat
    wah_free_exec_context(&ctx);

    const wah_parsed_code_t *pc = &mod.code_bodies[0].parsed_code;
    expect_rejected(&mod, OP_AT(I8X16_SHUFFLE, 0) + sizeof(uint16_t), 1, 32, "shuffle lane out of range");
    expect_rejected(&mod, OP_AT(I8X16_EXTRACT_LANE_S, 0) + sizeof(uint16_t), 1, 16, "lane out of range to extract");
    expect_rejected(&mod, OP_AT(I32X4_REPLACE_LANE, 0) + sizeof(uint16_t), 1, 4, "lane out of range to replace");
    expect_rejected(&mod, OP_AT(V128_LOAD8_LANE_i32_mem0, 0) + sizeof(uint16_t) + sizeof(uint64_t), 1, 16,
                    "lane out of range to load into");
    expect_rejected(&mod, OP_AT(V128_STORE_i32_mem0, 0), 2, WAH_OP_I32_STORE_i32_mem0, "vector stored as a scalar");
    expect_rejected(&mod, OP_AT(I32X4_SPLAT, 0), 2, WAH_OP_I32_EQZ, "scalar operation producing a vector");
    wah_free_module(&mod);
}

static void test_stack_maps(void) {
    printf("Testing stack maps of lowered code...\n");
    wah_module_t mod;
    wah_exec_context_t ctx;
    wah_value_t param = {.i32 = 1}, result;
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32], fn [i32] [i32]]} funcs {[1, 0]} \
        elements {[elem.declarative elem.funcref [1]]} \
        code {[ \
            {[] \
                block type.ref.null 0 \
                    ref.func 1 local.get 0 br_if 0 drop ref.null 0 \
                end \
                call_ref 0 \
                ref.func 1 ref.null funcref local.get 0 select.typed [funcref] ref.is_null \
                i32.add \
            end}, \
            {[] i32.const 7 end} \
        ]}"));
    assert_ok(check_func(&mod, 0));
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_call(&ctx, 0, &param, 1, &result));
    assert_eq_i32(result.i32, 7);
    wah_free_exec_context(&ctx);

    // The block joins (ref $0) from br_if and (ref null $0) from ref.null, which is fine only with its stack map
    wah_parsed_code_t *pc = &mod.code_bodies[0].parsed_code;
    assert_eq_u32(pc->stack_map_count, 1);
    wah_stack_map_t saved = pc->stack_maps[0];
    wah_type_t saved_type = pc->stack_map_types[saved.start];
    #define EXPECT_REJECTED(what, mutation) do { \
        printf("  %s\n", what); \
        mutation; \
        assert_err(check_func(&mod, 0), WAH_ERROR_VALIDATION_FAILED); \
        pc->stack_map_count = 1; \
        pc->stack_maps[0] = saved; \
        pc->stack_map_types[saved.start] = saved_type; \
        assert_ok(check_func(&mod, 0)); \
    } while (0)
    EXPECT_REJECTED("stack map dropped", pc->stack_map_count = 0);
    EXPECT_REJECTED("stack map with a supertype not accepted later", pc->stack_map_types[saved.start] = WAH_TYPE_FUNCREF);
    EXPECT_REJECTED("stack map with a subtype not met by every path",
                    pc->stack_map_types[saved.start] = WAH_TYPE_FROM_IDX(0, 0));
    EXPECT_REJECTED("stack map at the middle of an instruction", pc->stack_maps[0].offset++);
    EXPECT_REJECTED("stack map with types out of range", pc->stack_maps[0].count = 100);
    #undef EXPECT_REJECTED
    expect_rejected(&mod, IMM_AT(SELECT_T, 0, 0), 4, (uint32_t)WAH_TYPE_I32, "typed select of a wrong type");
    wah_free_module(&mod);

    printf("  block ending at the end of the function\n");
    // Both labels are at the END, and the branch to the function label doesn't fit the stack map of the block
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32], fn [i32] [funcref]]} funcs {[1, 0]} \
        elements {[elem.declarative elem.funcref [1]]} \
        code {[ \
            {[] block type.ref.null 0 ref.func 1 ref.null funcref local.get 0 br_if 1 drop end end}, \
            {[] i32.const 7 end} \
        ]}"));
    assert_ok(check_func(&mod, 0));
    wah_free_module(&mod);
}

static void test_ref_maps(void) {
    printf("Testing ref maps of lowered code...\n");
    // A reference stays on the stack across the call, so that it is a GC root inside the callee
    static const char ref_spec[] = "wasm \
        types {[fn [] [i32], fn [] []]} funcs {[1, 0]} \
        elements {[elem.declarative elem.funcref [1]]} \
        code {[{[] ref.func 1 call 1 drop drop end}, {[] i32.const 7 end}]}";
    wah_module_t mod;
    assert_ok(wah_parse_module_from_spec(&mod, ref_spec));
    assert_ok(check_func(&mod, 0));
    wah_parsed_code_t *pc = &mod.code_bodies[0].parsed_code;
    uint32_t entry_map = wah_read_u32_le(pc->bytecode + IMM_AT(POLL, 0, 0));
    expect_rejected(&mod, IMM_AT(POLL, 1, 0), 4, entry_map, "ref map missing a reference");
    expect_rejected(&mod, OP_AT(POLL, 1), 2, WAH_OP_TICK, "call not preceded by a POLL");
    printf("  ref map listing a slot above the stack\n");
    uint32_t node = pc->poll_ref_tops[wah_read_u32_le(pc->bytecode + IMM_AT(POLL, 1, 0))] - 1;
    pc->ref_nodes[node].pos++;
    assert_err(check_func(&mod, 0), WAH_ERROR_VALIDATION_FAILED);
    pc->ref_nodes[node].pos--;
    assert_ok(check_func(&mod, 0));
    wah_free_module(&mod);

    wah_parse_options_t opts = { .enable_fuel_metering = true };
    assert_ok(wah_parse_module_from_spec_ex(&mod, &opts, ref_spec));
    assert_ok(check_func(&mod, 0));
    pc = &mod.code_bodies[0].parsed_code;
    for (uint32_t i = 0; ; i++) { // The TICK in the slow island, which may suspend with the reference
        uint32_t tick = OP_AT(TICK, i);
        if (wah_read_u16_le(pc->bytecode + tick + sizeof(uint16_t) + sizeof(uint32_t)) != WAH_OP_POLL) continue;
        if (wah_read_u32_le(pc->bytecode + tick + sizeof(uint16_t)) == entry_map) continue;
        expect_rejected(&mod, tick + sizeof(uint16_t), 4, entry_map, "ref map of a slow island missing a reference");
        break;
    }
    wah_free_module(&mod);
}

static void test_gc(void) {
    printf("Testing lowered GC operations...\n");
    wah_module_t mod;
    wah_exec_context_t ctx;
    wah_value_t result;
    // $1 extends $0, and the object of $1 is accessed through both
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[sub [] struct [i32 mut, i8 mut], sub [0] struct [i32 mut, i8 mut, i64 immut], array i32 mut, fn [] [i32]]} \
        funcs {[3]} \
        code {[{[1 type.ref.null 0, 1 type.ref.null 2] \
            i32.const 5 i32.const 6 i64.const 7 struct.new 1 local.set 0 \
            local.get 0 i32.const 9 struct.set 0 0 \
            i32.const 1 i32.const 3 array.new 2 local.set 1 \
            local.get 1 i32.const 2 i32.const 4 array.set 2 \
            local.get 0 struct.get_s 0 1 \
            local.get 0 struct.get 0 0 i32.add \
            local.get 1 i32.const 2 array.get 2 i32.add \
            local.get 1 array.len i32.add \
            local.get 0 ref.cast 1 struct.get 1 2 i32.wrap_i64 i32.add \
        end}]}"));
    assert_ok(check_func(&mod, 0));
    assert_true(wah_check_layouts(&mod));
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 6 + 9 + 4 + 3 + 7);
    wah_free_exec_context(&ctx);

    const wah_parsed_code_t *pc = &mod.code_bodies[0].parsed_code;
    expect_rejected(&mod, IMM_AT(STRUCT_GET_32, 0, 0), 4, wah_read_u32_le(pc->bytecode + IMM_AT(STRUCT_GET_32, 0, 0)) + 1,
                    "struct field at no offset");
    expect_rejected(&mod, OP_AT(STRUCT_GET_S8, 0), 2, WAH_OP_STRUCT_GET_32, "struct field of another size");
    expect_rejected(&mod, OP_AT(ARRAY_GET_32, 0), 2, WAH_OP_ARRAY_GET_64, "array element of another size");
    expect_rejected(&mod, IMM_AT(REF_CAST, 0, 0), 4, (uint32_t)WAH_TYPE_FROM_IDX(2, 0), "struct access to a cast array");

    printf("  immutable struct field set\n");
    mod.type_defs[0].field_mutables[0] = false;
    assert_err(check_func(&mod, 0), WAH_ERROR_VALIDATION_FAILED);
    mod.type_defs[0].field_mutables[0] = true;
    assert_ok(check_func(&mod, 0));

    printf("  subtype laid out unlike its supertype\n");
    wah_repr_info_t *ri = (wah_repr_info_t *)wah_repr_info_get(&mod, wah_module_typeidx_to_repr(&mod, 1));
    uint32_t saved = ri->fields[1].offset;
    ri->fields[1].offset = ri->fields[0].offset;
    assert_true(!wah_check_layouts(&mod));
    ri->fields[1].offset = saved;
    assert_true(wah_check_layouts(&mod));
    wah_free_module(&mod);
}

static void test_exceptions(void) {
    printf("Testing lowered exception handling...\n");
    wah_module_t mod;
    wah_exec_context_t ctx;
    wah_value_t result;
    // The try_table takes 42, which the callee takes and throws, so that the handler expects the stack below it
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [i32] [], fn [] [i32]]} funcs {[1, 0]} tags {[tag.type# 0]} \
        code {[ \
            {[] block i32 i32.const 42 try_table 0 [catch 0 0] call 1 end i32.const -1 end end}, \
            {[] local.get 0 throw 0 end} \
        ]}"));
    assert_ok(check_func(&mod, 0));
    assert_ok(check_func(&mod, 1));
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 42);
    wah_free_exec_context(&ctx);

    const wah_parsed_code_t *pc = &mod.code_bodies[0].parsed_code;
    uint32_t catch0 = OP_AT(TRY_TABLE, 0) + sizeof(uint16_t) + sizeof(uint32_t); // Kind, tag, target and drop
    expect_rejected(&mod, catch0 + 1 + 2 * sizeof(uint32_t), 4, 0, "handler expecting arguments the callee may overwrite");
    expect_rejected(&mod, catch0 + 1, 4, 1, "catch of a tag out of range");
    expect_rejected(&mod, catch0, 1, WAH_CATCH_KIND_CATCH_REF, "catch with a payload the label doesn't take");
    expect_rejected(&mod, catch0 + 1 + sizeof(uint32_t), 4, OP_AT(TRY_TABLE, 0) + 1, "catch into the middle of an instruction");

    printf("  throw of a tag out of range\n");
    uint8_t *code = mod.code_bodies[1].parsed_code.bytecode;
    pc = &mod.code_bodies[1].parsed_code;
    uint32_t tag_pos = IMM_AT(THROW, 0, 0);
    wah_write_u32_le(code + tag_pos, 1);
    assert_err(check_func(&mod, 1), WAH_ERROR_VALIDATION_FAILED);
    wah_write_u32_le(code + tag_pos, 0);
    assert_ok(check_func(&mod, 1));
    wah_free_module(&mod);

    // The branch leaves the try_table, so that the label trims the handler that the other path has already ended
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [i32] []]} funcs {[0]} \
        code {[{[] block void try_table void [] local.get 0 br_if 1 end end end}]}"));
    assert_ok(check_func(&mod, 0));
    pc = &mod.code_bodies[0].parsed_code;
    expect_rejected(&mod, IMM_AT(TRIM_HANDLERS, 0, 0), 4, 1, "label keeping a handler ended on another path");
    wah_free_module(&mod);

    // Both handlers return from the function, where one of them is still active for the other, which doesn't matter
    assert_ok(wah_parse_module_from_spec(&mod, "wasm \
        types {[fn [] [i32], fn [i32] []]} funcs {[0]} tags {[tag.type# 1, tag.type# 1]} \
        code {[{[] try_table void [catch 0 0] try_table void [catch 1 1] i32.const 55 throw 0 end end \
                   i32.const -1 end}]}"));
    assert_ok(check_func(&mod, 0));
    assert_ok(wah_new_exec_context(&ctx, &mod, NULL));
    assert_ok(wah_call(&ctx, 0, NULL, 0, &result));
    assert_eq_i32(result.i32, 55);
    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
}

int main(void) {
    test_accepted();
    test_rejected();
    test_fuel_metering();
    test_bulk_memory();
    test_references();
    test_simd();
    test_stack_maps();
    test_ref_maps();
    test_gc();
    test_exceptions();
    printf("All lowered code checker tests passed!\n");
    return 0;
}
