// This file implements wah_parse_module_from_spec,
// which generates a WebAssembly module from a simple format string specification.
//
// The specification consists of tokens. Tokens can be:
// - Punctuations: `[`, `]`, `{`, `}`, `,`
// - Numbers: `42`, `-123`, `3.14f32`, `2.71828f64`
// - Verbatims: `'foo'`, '%'414243'` etc.
// - Bare words (anything else): `wasm`, `types`, `i32`, `local.get`, etc.
// - Placeholders: `%d32`, `%vi32`, `%f64`, `%v128`, etc.
//
// Bare words must have been defined and are translated to specific byte sequences.
// For example, `wasm` translates to the magic number and version bytes (`\0asm\1\0\0\0`).
//
// Numbers are translated to either an LEB128 varint or a float encoding, depending on their format.
//
// Verbatims are translated to their raw byte content. Prefixing `%` will interpret the content 
// as hexadecimal bytes. No escape character exists in either cases.
//
// Punctuations do not emit any byte sequence, but has the following special meanings:
// `[ ... ]` counts the number of comma-separated items and put that count as a varint in front of them.
// (A trailing comma doesn't count as an item, so both `[a,b,c]` and `[a,b,c,]` are same as `3 a b c`.)
// `{ ... }` counts the total byte length of the content and put that count as a varint in front of it.
//
// Placeholders are used in the test cases to specify a value that will be filled in at runtime.
// `%d32` and `%d64` expect an int32_t or int64_t value and emits a signed LEB128 varint.
// `%i32` and `%i64` expect int32_t and int64_t values respectively, and emits 4- and 8-byte little-endian integers.
// `%f32` and `%f64` expect float and double values respectively, and emits 4- and 8-byte IEEE 754 encodings.
// These placeholders can be prefixed with `v` (e.g. `%vi32`) to indicate that the value is passed as a pointer.
// `%v128` expects a const uint8_t* value and emits 16 bytes.
// `%t` expects a const char* value and recursively interprets it as a DSL specification string.

#include "../wah.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

// Token entry for the keyword mapping table
typedef struct {
    const char *name;
    const uint8_t *bytes;
    size_t len;
} token_entry_t;

// Macro to define a token entry with inline byte array using C99 compound literal
// Usage: TOKEN(name, b1, b2, ...) creates an entry with name and bytes [b1, b2, ...]
#define TOKEN(name, ...) \
    {name, (const uint8_t[]){__VA_ARGS__}, sizeof((const uint8_t[]){__VA_ARGS__})}

// Keyword mapping table (associative array)
// Each entry combines the token name and its byte representation in one place
static const token_entry_t token_table[] = {
    // Magic and sections
    TOKEN("wasm", 0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00),  // magic + version
    TOKEN("types", 0x01),     // Type section
    TOKEN("imports", 0x02),   // Import section
    TOKEN("funcs", 0x03),      // Function section
    TOKEN("tables", 0x04),    // Table section
    TOKEN("memories", 0x05),  // Memory section
    TOKEN("globals", 0x06),   // Global section
    TOKEN("exports", 0x07),   // Export section
    TOKEN("start", 0x08),     // Start section
    TOKEN("elements", 0x09),  // Element section
    TOKEN("code", 0x0a),      // Code section
    TOKEN("data", 0x0b),      // Data section
    TOKEN("datacount", 0x0c), // Data count section
    TOKEN("tags", 0x0d),      // Tag section

    // Types
    TOKEN("fn", 0x60),        // Function type
    TOKEN("i32", 0x7f),       // i32 type
    TOKEN("i64", 0x7e),       // i64 type
    TOKEN("f32", 0x7d),       // f32 type
    TOKEN("f64", 0x7c),       // f64 type
    TOKEN("v128", 0x7b),      // v128 type
    TOKEN("i8", 0x78),        // Packed i8 type
    TOKEN("i16", 0x77),       // Packed i16 type
    TOKEN("void", 0x40),      // Void return
    TOKEN("nullexnref", 0x74), // (ref null noexn)
    TOKEN("nullfuncref", 0x73), // (ref null nofunc)
    TOKEN("nullexternref", 0x72), // (ref null noextern)
    TOKEN("nullref", 0x71),   // (ref null none)
    TOKEN("funcref", 0x70),   // (ref null func) = funcref type
    TOKEN("externref", 0x6f), // (ref null extern) = externref type
    TOKEN("anyref", 0x6e),    // (ref null any)
    TOKEN("eqref", 0x6d),     // (ref null eq)
    TOKEN("i31ref", 0x6c),    // (ref null i31)
    TOKEN("structref", 0x6b), // (ref null struct)
    TOKEN("arrayref", 0x6a),  // (ref null array)
    TOKEN("exnref", 0x69),    // (ref null exn)
    TOKEN("type.ref.noexn", 0x64, 0x74),    // (ref noexn)
    TOKEN("type.ref.nofunc", 0x64, 0x73),   // (ref nofunc)
    TOKEN("type.ref.noextern", 0x64, 0x72), // (ref noextern)
    TOKEN("type.ref.none", 0x64, 0x71),     // (ref none)
    TOKEN("type.ref.func", 0x64, 0x70),     // (ref func)
    TOKEN("type.ref.extern", 0x64, 0x6f),   // (ref extern)
    TOKEN("type.ref.any", 0x64, 0x6e),      // (ref any)
    TOKEN("type.ref.eq", 0x64, 0x6d),       // (ref eq)
    TOKEN("type.ref.i31", 0x64, 0x6c),      // (ref i31)
    TOKEN("type.ref.struct", 0x64, 0x6b),   // (ref struct)
    TOKEN("type.ref.array", 0x64, 0x6a),    // (ref array)
    TOKEN("type.ref.exn", 0x64, 0x69),      // (ref exn)
    TOKEN("type.ref.null.noexn", 0x63, 0x74), // Alternative encodings of (ref null ...)
    TOKEN("type.ref.null.nofunc", 0x63, 0x73),
    TOKEN("type.ref.null.noextern", 0x63, 0x72),
    TOKEN("type.ref.null.none", 0x63, 0x71),
    TOKEN("type.ref.null.func", 0x63, 0x70),
    TOKEN("type.ref.null.extern", 0x63, 0x6f),
    TOKEN("type.ref.null.any", 0x63, 0x6e),
    TOKEN("type.ref.null.eq", 0x63, 0x6d),
    TOKEN("type.ref.null.i31", 0x63, 0x6c),
    TOKEN("type.ref.null.struct", 0x63, 0x6b),
    TOKEN("type.ref.null.array", 0x63, 0x6a),
    TOKEN("type.ref.null.exn", 0x63, 0x69),

    // Type parts
    TOKEN("type.ref.null", 0x63), // (ref null ...);  type.ref.null <type>
    TOKEN("type.ref", 0x64),      // (ref ...);       type.ref <type>
    TOKEN("array", 0x5e),         // (array ...);     array <type> {mut|immut}
    TOKEN("struct", 0x5f),        // (struct ...);    struct [<type> {mut|immut}, ...]
    TOKEN("rec", 0x4e),           // (rec ...);       rec [<type>, ...]
    TOKEN("sub.final", 0x4f),     // (sub final ...); sub.final [<supertype idx>, ...] <type>
    TOKEN("sub", 0x50),           // (sub ...);       sub [<supertype idx>, ...] <type>
    TOKEN("immut", 0x00),         // Immutable flag
    TOKEN("mut", 0x01),           // Mutable flag

    // Indices
    TOKEN("fn#", 0x00),       // Function index space
    TOKEN("table#", 0x01),    // Table index space
    TOKEN("mem#", 0x02),      // Memory index space
    TOKEN("global#", 0x03),   // Global index space
    TOKEN("tag#", 0x04),      // Tag index space

    // Exports
    TOKEN("export.type#", 0x00), // Or use `fn#`
    TOKEN("export.table", 0x01),
    TOKEN("export.memory", 0x02),
    TOKEN("export.global", 0x03),
    TOKEN("export.tag", 0x04),

    // Element kinds (for elemkind field)
    TOKEN("elem.funcref", 0x00),  // funcref as element kind

    // Elements
    TOKEN("elem.active.table#0", 0x00),      // elem.active.table#0 <offset_expr> [<funcidx>...]
    TOKEN("elem.passive", 0x01),             // elem.passive <elemkind> [<funcidx>...]
    TOKEN("elem.active.table#", 0x02),       // elem.active.table# <tableidx> <offset_expr> <elemkind> [<funcidx>...]
    TOKEN("elem.declarative", 0x03),         // elem.declarative <elemkind> [<funcidx>...]
    TOKEN("elem.active.expr.table#0", 0x04), // elem.active.expr.table#0 <offset_expr> [<expr>...]
    TOKEN("elem.passive.expr", 0x05),        // elem.passive.expr <reftype> [<expr>...]
    TOKEN("elem.active.expr.table#", 0x06),  // elem.active.expr.table# <tableidx> <offset_expr> <reftype> [<expr>...]
    TOKEN("elem.declarative.expr", 0x07),    // elem.declarative.expr <reftype> [<expr>...]

    // Tables
    TOKEN("table.w/init", 0x40, 0x00),

    // Data
    TOKEN("data.active.table#0", 0x00),
    TOKEN("data.passive", 0x01),
    TOKEN("data.active.table#", 0x02),

    // Instructions
    TOKEN("unreachable", 0x00),
    TOKEN("nop", 0x01),
    TOKEN("block", 0x02),
    TOKEN("loop", 0x03),
    TOKEN("if", 0x04),
    TOKEN("else", 0x05),
    TOKEN("throw", 0x08),
    TOKEN("throw_ref", 0x0a),
    TOKEN("end", 0x0b),
    TOKEN("br", 0x0c),
    TOKEN("br_if", 0x0d),
    TOKEN("br_table", 0x0e),
    TOKEN("return", 0x0f),
    TOKEN("call", 0x10),
    TOKEN("call_indirect", 0x11),
    TOKEN("return_call", 0x12),
    TOKEN("return_call_indirect", 0x13),
    TOKEN("call_ref", 0x14),
    TOKEN("return_call_ref", 0x15),
    TOKEN("try_table", 0x1f),
    TOKEN("drop", 0x1a),
    TOKEN("select", 0x1b),
    TOKEN("local.get", 0x20),
    TOKEN("local.set", 0x21),
    TOKEN("local.tee", 0x22),
    TOKEN("global.get", 0x23),
    TOKEN("global.set", 0x24),
    TOKEN("table.get", 0x25),
    TOKEN("table.set", 0x26),
    TOKEN("i32.load", 0x28),
    TOKEN("i64.load", 0x29),
    TOKEN("f32.load", 0x2a),
    TOKEN("f64.load", 0x2b),
    TOKEN("i32.load8_s", 0x2c),
    TOKEN("i32.load8_u", 0x2d),
    TOKEN("i32.load16_s", 0x2e),
    TOKEN("i32.load16_u", 0x2f),
    TOKEN("i64.load8_s", 0x30),
    TOKEN("i64.load8_u", 0x31),
    TOKEN("i64.load16_s", 0x32),
    TOKEN("i64.load16_u", 0x33),
    TOKEN("i64.load32_s", 0x34),
    TOKEN("i64.load32_u", 0x35),
    TOKEN("i32.store", 0x36),
    TOKEN("i64.store", 0x37),
    TOKEN("f32.store", 0x38),
    TOKEN("f64.store", 0x39),
    TOKEN("i32.store8", 0x3a),
    TOKEN("i32.store16", 0x3b),
    TOKEN("i64.store8", 0x3c),
    TOKEN("i64.store16", 0x3d),
    TOKEN("i64.store32", 0x3e),
    TOKEN("memory.size", 0x3f),
    TOKEN("memory.grow", 0x40),
    TOKEN("i32.const", 0x41),
    TOKEN("i64.const", 0x42),
    TOKEN("f32.const", 0x43),
    TOKEN("f64.const", 0x44),
    TOKEN("i32.eqz", 0x45),
    TOKEN("i32.eq", 0x46),
    TOKEN("i32.ne", 0x47),
    TOKEN("i32.lt_s", 0x48),
    TOKEN("i32.lt_u", 0x49),
    TOKEN("i32.gt_s", 0x4a),
    TOKEN("i32.gt_u", 0x4b),
    TOKEN("i32.le_s", 0x4c),
    TOKEN("i32.le_u", 0x4d),
    TOKEN("i32.ge_s", 0x4e),
    TOKEN("i32.ge_u", 0x4f),
    TOKEN("i64.eqz", 0x50),
    TOKEN("i64.eq", 0x51),
    TOKEN("i64.ne", 0x52),
    TOKEN("i64.lt_s", 0x53),
    TOKEN("i64.lt_u", 0x54),
    TOKEN("i64.gt_s", 0x55),
    TOKEN("i64.gt_u", 0x56),
    TOKEN("i64.le_s", 0x57),
    TOKEN("i64.le_u", 0x58),
    TOKEN("i64.ge_s", 0x59),
    TOKEN("i64.ge_u", 0x5a),
    TOKEN("i32.popcnt", 0x69),
    TOKEN("i32.add", 0x6a),
    TOKEN("i32.sub", 0x6b),
    TOKEN("i32.mul", 0x6c),
    TOKEN("i32.div_s", 0x6d),
    TOKEN("i32.div_u", 0x6e),
    TOKEN("i32.rem_s", 0x6f),
    TOKEN("i32.rem_u", 0x70),
    TOKEN("i32.and", 0x71),
    TOKEN("i32.or", 0x72),
    TOKEN("i32.xor", 0x73),
    TOKEN("i32.rotl", 0x77),
    TOKEN("i64.clz", 0x79),
    TOKEN("i64.add", 0x7c),
    TOKEN("i64.sub", 0x7d),
    TOKEN("i64.mul", 0x7e),
    TOKEN("i64.div_s", 0x7f),
    TOKEN("i64.div_u", 0x80),
    TOKEN("i64.rem_s", 0x81),
    TOKEN("i64.rem_u", 0x82),
    TOKEN("i64.and", 0x83),
    TOKEN("i64.or", 0x84),
    TOKEN("i64.xor", 0x85),
    TOKEN("f32.abs", 0x8b),
    TOKEN("f32.neg", 0x8c),
    TOKEN("f32.ceil", 0x8d),
    TOKEN("f32.floor", 0x8e),
    TOKEN("f32.trunc", 0x8f),
    TOKEN("f32.nearest", 0x90),
    TOKEN("f32.sqrt", 0x91),
    TOKEN("f32.add", 0x92),
    TOKEN("f32.sub", 0x93),
    TOKEN("f32.mul", 0x94),
    TOKEN("f32.div", 0x95),
    TOKEN("f32.min", 0x96),
    TOKEN("f32.max", 0x97),
    TOKEN("f32.copysign", 0x98),
    TOKEN("f64.abs", 0x99),
    TOKEN("f64.neg", 0x9a),
    TOKEN("f64.ceil", 0x9b),
    TOKEN("f64.floor", 0x9c),
    TOKEN("f64.trunc", 0x9d),
    TOKEN("f64.nearest", 0x9e),
    TOKEN("f64.sqrt", 0x9f),
    TOKEN("f64.add", 0xa0),
    TOKEN("f64.sub", 0xa1),
    TOKEN("f64.mul", 0xa2),
    TOKEN("f64.div", 0xa3),
    TOKEN("f64.min", 0xa4),
    TOKEN("f64.max", 0xa5),
    TOKEN("f64.copysign", 0xa6),
    TOKEN("i32.wrap_i64", 0xa7),
    TOKEN("i32.trunc_f32_s", 0xa8),
    TOKEN("i32.trunc_f32_u", 0xa9),
    TOKEN("i32.trunc_f64_s", 0xaa),
    TOKEN("i32.trunc_f64_u", 0xab),
    TOKEN("i64.extend_i32_s", 0xac),
    TOKEN("i64.extend_i32_u", 0xad),
    TOKEN("i64.trunc_f32_s", 0xae),
    TOKEN("i64.trunc_f32_u", 0xaf),
    TOKEN("i64.trunc_f64_s", 0xb0),
    TOKEN("i64.trunc_f64_u", 0xb1),
    TOKEN("f32.convert_i32_s", 0xb2),
    TOKEN("f32.convert_i32_u", 0xb3),
    TOKEN("f32.convert_i64_s", 0xb4),
    TOKEN("f32.convert_i64_u", 0xb5),
    TOKEN("f32.demote_f64", 0xb6),
    TOKEN("f64.convert_i32_s", 0xb7),
    TOKEN("f64.convert_i32_u", 0xb8),
    TOKEN("f64.convert_i64_s", 0xb9),
    TOKEN("f64.convert_i64_u", 0xba),
    TOKEN("f64.promote_f32", 0xbb),
    TOKEN("i32.reinterpret_f32", 0xbc),
    TOKEN("i64.reinterpret_f64", 0xbd),
    TOKEN("f32.reinterpret_i32", 0xbe),
    TOKEN("f64.reinterpret_i64", 0xbf),
    TOKEN("i32.extend8_s", 0xc0),
    TOKEN("i32.extend16_s", 0xc1),
    TOKEN("i64.extend8_s", 0xc2),
    TOKEN("i64.extend16_s", 0xc3),
    TOKEN("i64.extend32_s", 0xc4),
    TOKEN("ref.null", 0xd0),
    TOKEN("ref.is_null", 0xd1),
    TOKEN("ref.func", 0xd2),
    TOKEN("struct.new", 0xfb, 0x00),
    TOKEN("struct.new_default", 0xfb, 0x01),
    TOKEN("struct.get", 0xfb, 0x02),
    TOKEN("struct.get_s", 0xfb, 0x03),
    TOKEN("struct.get_u", 0xfb, 0x04),
    TOKEN("struct.set", 0xfb, 0x05),
    TOKEN("array.new", 0xfb, 0x06),
    TOKEN("array.new_default", 0xfb, 0x07),
    TOKEN("array.new_fixed", 0xfb, 0x08),
    TOKEN("array.new_data", 0xfb, 0x09),
    TOKEN("array.new_elem", 0xfb, 0x0a),
    TOKEN("array.get", 0xfb, 0x0b),
    TOKEN("array.get_s", 0xfb, 0x0c),
    TOKEN("array.get_u", 0xfb, 0x0d),
    TOKEN("array.set", 0xfb, 0x0e),
    TOKEN("array.len", 0xfb, 0x0f),
    TOKEN("array.fill", 0xfb, 0x10),
    TOKEN("array.copy", 0xfb, 0x11),
    TOKEN("array.init_data", 0xfb, 0x12),
    TOKEN("array.init_elem", 0xfb, 0x13),
    TOKEN("ref.test", 0xfb, 0x14),
    TOKEN("ref.test.null", 0xfb, 0x15),
    TOKEN("ref.cast", 0xfb, 0x16),
    TOKEN("ref.cast.null", 0xfb, 0x17),
    TOKEN("ref.eq", 0xd3),
    TOKEN("ref.as_non_null", 0xd4),
    TOKEN("br_on_null", 0xd5),
    TOKEN("br_on_non_null", 0xd6),
    TOKEN("br_on_cast", 0xfb, 0x18),
    TOKEN("br_on_cast_fail", 0xfb, 0x19),
    TOKEN("any.convert_extern", 0xfb, 0x1a),
    TOKEN("extern.convert_any", 0xfb, 0x1b),
    TOKEN("ref.i31", 0xfb, 0x1c),
    TOKEN("i31.get_s", 0xfb, 0x1d),
    TOKEN("i31.get_u", 0xfb, 0x1e),
    TOKEN("i32.trunc_sat_f32_s", 0xfc, 0x00),
    TOKEN("i32.trunc_sat_f32_u", 0xfc, 0x01),
    TOKEN("i32.trunc_sat_f64_s", 0xfc, 0x02),
    TOKEN("i32.trunc_sat_f64_u", 0xfc, 0x03),
    TOKEN("i64.trunc_sat_f32_s", 0xfc, 0x04),
    TOKEN("i64.trunc_sat_f32_u", 0xfc, 0x05),
    TOKEN("i64.trunc_sat_f64_s", 0xfc, 0x06),
    TOKEN("i64.trunc_sat_f64_u", 0xfc, 0x07),
    TOKEN("memory.init", 0xfc, 0x08),
    TOKEN("memory.copy", 0xfc, 0x0a),
    TOKEN("memory.fill", 0xfc, 0x0b),
    TOKEN("table.init", 0xfc, 0x0c),
    TOKEN("elem.drop", 0xfc, 0x0d),
    TOKEN("table.copy", 0xfc, 0x0e),
    TOKEN("table.grow", 0xfc, 0x0f),
    TOKEN("table.size", 0xfc, 0x10),
    TOKEN("table.fill", 0xfc, 0x11),
    TOKEN("v128.load", 0xfd, 0x00),
    TOKEN("v128.load8x8_s", 0xfd, 0x01),
    TOKEN("v128.load8x8_u", 0xfd, 0x02),
    TOKEN("v128.load16x4_s", 0xfd, 0x03),
    TOKEN("v128.load16x4_u", 0xfd, 0x04),
    TOKEN("v128.load32x2_s", 0xfd, 0x05),
    TOKEN("v128.load32x2_u", 0xfd, 0x06),
    TOKEN("v128.load8_splat", 0xfd, 0x07),
    TOKEN("v128.load16_splat", 0xfd, 0x08),
    TOKEN("v128.load32_splat", 0xfd, 0x09),
    TOKEN("v128.load64_splat", 0xfd, 0x0a),
    TOKEN("v128.store", 0xfd, 0x0b),
    TOKEN("v128.const", 0xfd, 0x0c),
    TOKEN("i8x16.shuffle", 0xfd, 0x0d),
    TOKEN("i8x16.swizzle", 0xfd, 0x0e),
    TOKEN("i8x16.splat", 0xfd, 0x0f),
    TOKEN("i16x8.splat", 0xfd, 0x10),
    TOKEN("i32x4.splat", 0xfd, 0x11),
    TOKEN("i64x2.splat", 0xfd, 0x12),
    TOKEN("f32x4.splat", 0xfd, 0x13),
    TOKEN("f64x2.splat", 0xfd, 0x14),
    TOKEN("i8x16.extract_lane_s", 0xfd, 0x15),
    TOKEN("i8x16.replace_lane", 0xfd, 0x17),
    TOKEN("i32x4.extract_lane", 0xfd, 0x1b),
    TOKEN("i8x16.eq", 0xfd, 0x23),
    TOKEN("i8x16.ne", 0xfd, 0x24),
    TOKEN("i8x16.lt_s", 0xfd, 0x25),
    TOKEN("i8x16.lt_u", 0xfd, 0x26),
    TOKEN("i8x16.gt_s", 0xfd, 0x27),
    TOKEN("i8x16.gt_u", 0xfd, 0x28),
    TOKEN("i8x16.le_s", 0xfd, 0x29),
    TOKEN("i8x16.le_u", 0xfd, 0x2a),
    TOKEN("i8x16.ge_s", 0xfd, 0x2b),
    TOKEN("i8x16.ge_u", 0xfd, 0x2c),
    TOKEN("i16x8.eq", 0xfd, 0x2d),
    TOKEN("i16x8.ne", 0xfd, 0x2e),
    TOKEN("i16x8.lt_s", 0xfd, 0x2f),
    TOKEN("i16x8.lt_u", 0xfd, 0x30),
    TOKEN("i16x8.gt_s", 0xfd, 0x31),
    TOKEN("i16x8.gt_u", 0xfd, 0x32),
    TOKEN("i16x8.le_s", 0xfd, 0x33),
    TOKEN("i16x8.le_u", 0xfd, 0x34),
    TOKEN("i16x8.ge_s", 0xfd, 0x35),
    TOKEN("i16x8.ge_u", 0xfd, 0x36),
    TOKEN("i32x4.eq", 0xfd, 0x37),
    TOKEN("i32x4.ne", 0xfd, 0x38),
    TOKEN("i32x4.lt_s", 0xfd, 0x39),
    TOKEN("i32x4.lt_u", 0xfd, 0x3a),
    TOKEN("i32x4.gt_s", 0xfd, 0x3b),
    TOKEN("i32x4.gt_u", 0xfd, 0x3c),
    TOKEN("i32x4.le_s", 0xfd, 0x3d),
    TOKEN("i32x4.le_u", 0xfd, 0x3e),
    TOKEN("i32x4.ge_s", 0xfd, 0x3f),
    TOKEN("i32x4.ge_u", 0xfd, 0x40),
    TOKEN("f32x4.eq", 0xfd, 0x41),
    TOKEN("f32x4.ne", 0xfd, 0x42),
    TOKEN("f32x4.lt", 0xfd, 0x43),
    TOKEN("f32x4.gt", 0xfd, 0x44),
    TOKEN("f32x4.le", 0xfd, 0x45),
    TOKEN("f32x4.ge", 0xfd, 0x46),
    TOKEN("f64x2.eq", 0xfd, 0x47),
    TOKEN("f64x2.ne", 0xfd, 0x48),
    TOKEN("f64x2.lt", 0xfd, 0x49),
    TOKEN("f64x2.gt", 0xfd, 0x4a),
    TOKEN("f64x2.le", 0xfd, 0x4b),
    TOKEN("f64x2.ge", 0xfd, 0x4c),
    TOKEN("v128.not", 0xfd, 0x4d),
    TOKEN("v128.and", 0xfd, 0x4e),
    TOKEN("v128.andnot", 0xfd, 0x4f),
    TOKEN("v128.or", 0xfd, 0x50),
    TOKEN("v128.xor", 0xfd, 0x51),
    TOKEN("v128.bitselect", 0xfd, 0x52),
    TOKEN("v128.any_true", 0xfd, 0x53),
    TOKEN("v128.load8_lane", 0xfd, 0x54),
    TOKEN("v128.load16_lane", 0xfd, 0x55),
    TOKEN("v128.load32_lane", 0xfd, 0x56),
    TOKEN("v128.load64_lane", 0xfd, 0x57),
    TOKEN("v128.load32_zero", 0xfd, 0x5c),
    TOKEN("v128.load64_zero", 0xfd, 0x5d),
    TOKEN("f32x4.demote_f64x2_zero", 0xfd, 0x5e),
    TOKEN("f64x2.promote_low_f32x4", 0xfd, 0x5f),
    TOKEN("i8x16.abs", 0xfd, 0x60),
    TOKEN("f32x4.ceil", 0xfd, 0x67),
    TOKEN("i8x16.shl", 0xfd, 0x6b),
    TOKEN("i8x16.add", 0xfd, 0x6e),
    TOKEN("i8x16.add_sat_s", 0xfd, 0x6f),
    TOKEN("i8x16.add_sat_u", 0xfd, 0x70),
    TOKEN("i8x16.sub", 0xfd, 0x71),
    TOKEN("i8x16.sub_sat_s", 0xfd, 0x72),
    TOKEN("i8x16.sub_sat_u", 0xfd, 0x73),
    TOKEN("i8x16.min_s", 0xfd, 0x76),
    TOKEN("i8x16.avgr_u", 0xfd, 0x7b),
    TOKEN("i16x8.q15mulr_sat_s", 0xfd, 0x82, 0x01),
    TOKEN("i16x8.narrow_i32x4_s", 0xfd, 0x85, 0x01),
    TOKEN("i16x8.extend_low_i8x16_s", 0xfd, 0x87, 0x01),
    TOKEN("i16x8.sub", 0xfd, 0x91, 0x01),
    TOKEN("i32x4.mul", 0xfd, 0xb5, 0x01),
    TOKEN("i32x4.dot_i16x8_s", 0xfd, 0xba, 0x01),
    TOKEN("i32x4.extmul_low_i16x8_s", 0xfd, 0xbc, 0x01),
    TOKEN("i64x2.abs", 0xfd, 0xc0, 0x01),
    TOKEN("i64x2.extend_high_i32x4_u", 0xfd, 0xca, 0x01),
    TOKEN("i64x2.eq", 0xfd, 0xd6, 0x01),
    TOKEN("i64x2.ne", 0xfd, 0xd7, 0x01),
    TOKEN("i64x2.lt_s", 0xfd, 0xd8, 0x01),
    TOKEN("i64x2.gt_s", 0xfd, 0xd9, 0x01),
    TOKEN("i64x2.le_s", 0xfd, 0xda, 0x01),
    TOKEN("i64x2.ge_s", 0xfd, 0xdb, 0x01),
    TOKEN("f32x4.abs", 0xfd, 0xe0, 0x01),
    TOKEN("f32x4.add", 0xfd, 0xe4, 0x01),
    TOKEN("f32x4.min", 0xfd, 0xe8, 0x01),
    TOKEN("f32x4.pmax", 0xfd, 0xeb, 0x01),
    TOKEN("f64x2.neg", 0xfd, 0xed, 0x01),
    TOKEN("f64x2.sqrt", 0xfd, 0xef, 0x01),
    TOKEN("i32x4.trunc_sat_f32x4_s", 0xfd, 0xf8, 0x01),
    TOKEN("i32x4.trunc_sat_f32x4_u", 0xfd, 0xf9, 0x01),
    TOKEN("f32x4.convert_i32x4_s", 0xfd, 0xfa, 0x01),
    TOKEN("f32x4.convert_i32x4_u", 0xfd, 0xfb, 0x01),
    TOKEN("i32x4.trunc_sat_f64x2_s_zero", 0xfd, 0xfc, 0x01),
    TOKEN("i32x4.trunc_sat_f64x2_u_zero", 0xfd, 0xfd, 0x01),
    TOKEN("f64x2.convert_low_i32x4_s", 0xfd, 0xfe, 0x01),
    TOKEN("f64x2.convert_low_i32x4_u", 0xfd, 0xff, 0x01),
    TOKEN("i8x16.relaxed_swizzle", 0xfd, 0x80, 0x02),
    TOKEN("i32x4.relaxed_trunc_f32x4_s", 0xfd, 0x81, 0x02),
    TOKEN("i32x4.relaxed_trunc_f32x4_u", 0xfd, 0x82, 0x02),
    TOKEN("f32x4.relaxed_madd", 0xfd, 0x85, 0x02),
    TOKEN("i8x16.relaxed_laneselect", 0xfd, 0x89, 0x02),
    TOKEN("f32x4.relaxed_min", 0xfd, 0x8d, 0x02),
    TOKEN("i16x8.relaxed_q15mulr_s", 0xfd, 0x91, 0x02),
    TOKEN("i16x8.relaxed_dot_i8x16_i7x16_s", 0xfd, 0x92, 0x02),
    TOKEN("i32x4.relaxed_dot_i8x16_i7x16_add_s", 0xfd, 0x93, 0x02),

    // Opcode parts
    TOKEN("offset=0", 0x00),
    TOKEN("offset=0x1233", 0xb3, 0x24),
    TOKEN("offset=0x1234", 0xb4, 0x24),
    TOKEN("offset=0xffffffff", 0xff, 0xff, 0xff, 0xff, 0x0f),
    TOKEN("offset=0x100000000", 0x80, 0x80, 0x80, 0x80, 0x10),
    TOKEN("align=1", 0x00),
    TOKEN("align=2", 0x01),
    TOKEN("align=4", 0x02),
    TOKEN("align=8", 0x03),
    TOKEN("align=1.mem#", 0x40),
    TOKEN("align=2.mem#", 0x41),
    TOKEN("align=4.mem#", 0x42),
    TOKEN("align=8.mem#", 0x43),

    // Miscellaneous
    TOKEN("limits.i32/1", 0x00),  // limits.i32/1 <min> (max is infinite)
    TOKEN("limits.i32/2", 0x01),  // limits.i32/2 <min> <max>
    TOKEN("limits.i64/1", 0x04),  // limits.i64/1 <min> (max is infinite)
    TOKEN("limits.i64/2", 0x05),  // limits.i64/2 <min> <max>

    {NULL, NULL, 0}  // Sentinel
};

// Dynamic buffer for building the binary
typedef struct {
    uint8_t *data;
    size_t size;
    size_t capacity;
} buffer_t;

static void buffer_init(buffer_t *buf) {
    buf->capacity = 1024;
    buf->size = 0;
    buf->data = (uint8_t *)malloc(buf->capacity);
}

static void buffer_append(buffer_t *buf, const uint8_t *data, size_t len) {
    while (buf->size + len > buf->capacity) {
        buf->capacity *= 2;
        buf->data = (uint8_t *)realloc(buf->data, buf->capacity);
    }
    memcpy(buf->data + buf->size, data, len);
    buf->size += len;
}

static void buffer_free(buffer_t *buf) {
    free(buf->data);
    buf->data = NULL;
    buf->size = buf->capacity = 0;
}

// Write unsigned LEB128 (varint)
static void append_varint(buffer_t *buf, uint64_t value) {
    uint8_t byte;
    do {
        byte = value & 0x7f;
        value >>= 7;
        if (value != 0) {
            byte |= 0x80;
        }
        buffer_append(buf, &byte, 1);
    } while (value != 0);
}

// Write signed LEB128 (varint for signed numbers)
static void append_varint_signed(buffer_t *buf, int64_t value) {
    uint8_t byte;
    int more;
    do {
        byte = value & 0x7f;
        value >>= 7;  // Arithmetic shift (fill with sign bit)
        more = !((value == 0 && (byte & 0x40) == 0) ||
                 (value == -1 && (byte & 0x40) != 0));
        if (more) {
            byte |= 0x80;
        }
        buffer_append(buf, &byte, 1);
    } while (more);
}

// Look up a token in the keyword table
static const token_entry_t *lookup_token(const char *token, size_t token_len) {
    for (int i = 0; token_table[i].name != NULL; i++) {
        if (strncmp(token, token_table[i].name, token_len) == 0 &&
            token_table[i].name[token_len] == '\0') {
            return &token_table[i];
        }
    }
    return NULL;
}

// Check if a string is a valid number
// Returns: 0 = not a number, 1 = integer, 2 = float
// If float, sets suffix_offset to the offset of the suffix (e.g., "f32")
static int classify_number(const char *token, size_t token_len, size_t *suffix_offset) {
    *suffix_offset = 0;

    const char *p = token;
    const char *end = token + token_len;
    if (p < end && (*p == '-' || *p == '+')) p++;

    int has_digits = 0;
    int has_dot = 0;

    while (p < end) {
        if (*p == '.') {
            if (has_dot) return 0;  // Multiple dots
            has_dot = 1;
            p++;
        } else if (isdigit(*p)) {
            has_digits = 1;
            p++;
        } else if (!has_dot && isalpha(*p)) {
            // Might be a number followed by suffix like "123f32"
            break;
        } else {
            break;
        }
    }

    if (!has_digits) return 0;
    if (p < end) {
        *suffix_offset = p - token;
    }

    return has_dot ? 2 : 1;
}

// Append int32 value
static void append_i32(buffer_t *buf, int32_t value) {
    buffer_append(buf, (uint8_t *)&value, sizeof(value));
}

// Append int64 value
static void append_i64(buffer_t *buf, int64_t value) {
    buffer_append(buf, (uint8_t *)&value, sizeof(value));
}

// Append float32 value
static void append_f32(buffer_t *buf, float value) {
    uint32_t bits;
    memcpy(&bits, &value, sizeof(bits));
    buffer_append(buf, (uint8_t *)&bits, sizeof(bits));
}

// Append float64 value
static void append_f64(buffer_t *buf, double value) {
    uint64_t bits;
    memcpy(&bits, &value, sizeof(bits));
    buffer_append(buf, (uint8_t *)&bits, sizeof(bits));
}

// Append v128 value (16 bytes)
static void append_v128(buffer_t *buf, const uint8_t *value) {
    buffer_append(buf, value, 16);
}


// Forward declarations
static size_t parse_tokens(buffer_t *buf, const char *fmt, va_list *args, bool *error);
static size_t parse_bracket_expr(buffer_t *buf, const char *fmt, char bracket, va_list *args, bool *error);

// Parse a bracket expression: [...] or {...}
// Returns number of characters consumed
static size_t parse_bracket_expr(buffer_t *buf, const char *fmt, char bracket, va_list *args, bool *error) {
    const char *original_fmt = fmt;  // Save original position for return value
    fmt++;  // Skip opening bracket

    if (bracket == '[') {
        // [...] counts commas+1 as varint prefix
        buffer_t content_buf;
        buffer_init(&content_buf);
        int comma_count = 0;
        int has_content = 0;  // Track if we have any non-comma content

        while (*fmt != ']' && *fmt != '\0') {
            // Skip whitespace
            if (isspace(*fmt)) {
                fmt++;
                continue;
            }

            // Handle nested brackets directly - parse as single items
            if (*fmt == '[') {
                size_t consumed = parse_bracket_expr(&content_buf, fmt, '[', args, error);
                if (*error) {
                    buffer_free(&content_buf);
                    return 0;
                }
                if (consumed == 0) {
                    buffer_free(&content_buf);
                    return 0;
                }
                has_content = 1;
                fmt += consumed;
                continue;
            }

            if (*fmt == ',') {
                // Check if comma is followed by ] (ignoring whitespace)
                const char *next = fmt + 1;
                while (*next && isspace(*next)) next++;
                if (*next != ']') {
                    comma_count++;
                }
                fmt++;
                continue;
            }

            // Parse other tokens (keywords, numbers, etc.)
            size_t consumed = parse_tokens(&content_buf, fmt, args, error);
            if (*error) {
                buffer_free(&content_buf);
                return 0;
            }
            if (consumed == 0) {
                // Unknown token - return error
                *error = true;
                buffer_free(&content_buf);
                return 0;
            }
            has_content = 1;
            fmt += consumed;
        }

        if (*fmt == ']') fmt++;

        // Append: (comma_count + 1) as varint, then content
        // But if empty brackets [], should be 0 not 1
        uint32_t count = has_content ? (comma_count + 1) : 0;
        append_varint(buf, count);
        buffer_append(buf, content_buf.data, content_buf.size);
        buffer_free(&content_buf);

    } else {  // bracket == '{'
        // {...} counts bytes as varint prefix
        buffer_t content_buf;
        buffer_init(&content_buf);

        while (*fmt != '}' && *fmt != '\0') {
            size_t consumed = parse_tokens(&content_buf, fmt, args, error);
            if (*error) {
                buffer_free(&content_buf);
                return 0;
            }
            if (consumed == 0) {
                // Unknown token - return error
                *error = true;
                buffer_free(&content_buf);
                return 0;
            }
            fmt += consumed;
        }

        if (*fmt == '}') fmt++;

        // Append: byte count as varint, then content
        append_varint(buf, content_buf.size);
        buffer_append(buf, content_buf.data, content_buf.size);
        buffer_free(&content_buf);
    }

    return fmt - original_fmt;
}

// Parse tokens from format string until delimiter or special character
// Returns number of characters consumed
static size_t parse_tokens(buffer_t *buf, const char *fmt, va_list *args, bool *error) {
    const char *start = fmt;

    while (*fmt != '\0') {
        // Skip whitespace
        if (isspace(*fmt)) {
            fmt++;
            continue;
        }

        // Check for brackets
        if (*fmt == '[') {
            size_t consumed = parse_bracket_expr(buf, fmt, '[', args, error);
            if (*error) return 0;
            fmt += consumed;
            continue;
        }

        if (*fmt == '{') {
            size_t consumed = parse_bracket_expr(buf, fmt, '{', args, error);
            if (*error) return 0;
            fmt += consumed;
            continue;
        }

        // Stop at closing brackets
        if (*fmt == ']' || *fmt == '}') {
            break;
        }

        // Stop at comma (only in bracket context)
        if (*fmt == ',') {
            break;
        }

        // Check for single-quoted strings (raw bytes)
        if (*fmt == '\'') {
            fmt++;  // Skip opening quote
            const char *str_start = fmt;
            while (*fmt != '\'' && *fmt != '\0') {
                fmt++;
            }
            if (*fmt == '\'') {
                // Append the string content as raw bytes
                buffer_append(buf, (const uint8_t *)str_start, fmt - str_start);
                fmt++;  // Skip closing quote
            }
            continue;
        }

        // Check for format specifiers
        if (*fmt == '%') {
            fmt++;  // Skip '%'

            if (strncmp(fmt, "d32", 3) == 0) {
                int32_t val = va_arg(*args, int32_t);
                append_varint_signed(buf, val);
                fmt += 3;
            } else if (strncmp(fmt, "d64", 3) == 0) {
                int64_t val = va_arg(*args, int64_t);
                append_varint_signed(buf, val);
                fmt += 3;
            } else if (strncmp(fmt, "i32", 3) == 0) {
                int32_t val = va_arg(*args, int32_t);
                append_i32(buf, val);
                fmt += 3;
            } else if (strncmp(fmt, "i64", 3) == 0) {
                int64_t val = va_arg(*args, int64_t);
                append_i64(buf, val);
                fmt += 3;
            } else if (strncmp(fmt, "f32", 3) == 0) {
                float val = (float)va_arg(*args, double);
                append_f32(buf, val);
                fmt += 3;
            } else if (strncmp(fmt, "f64", 3) == 0) {
                double val = va_arg(*args, double);
                append_f64(buf, val);
                fmt += 3;
            } else if (strncmp(fmt, "vi32", 4) == 0) {
                const int32_t *val = va_arg(*args, const int32_t*);
                append_i32(buf, *val);
                fmt += 4;
            } else if (strncmp(fmt, "vi64", 4) == 0) {
                const int64_t *val = va_arg(*args, const int64_t*);
                append_i64(buf, *val);
                fmt += 4;
            } else if (strncmp(fmt, "vd32", 4) == 0) {
                const int32_t *val = va_arg(*args, const int32_t*);
                append_varint_signed(buf, *val);
                fmt += 4;
            } else if (strncmp(fmt, "vd64", 4) == 0) {
                const int64_t *val = va_arg(*args, const int64_t*);
                append_varint_signed(buf, *val);
                fmt += 4;
            } else if (strncmp(fmt, "vf32", 4) == 0) {
                const float *val = va_arg(*args, const float*);
                append_f32(buf, *val);
                fmt += 4;
            } else if (strncmp(fmt, "vf64", 4) == 0) {
                const double *val = va_arg(*args, const double*);
                append_f64(buf, *val);
                fmt += 4;
            } else if (strncmp(fmt, "v128", 4) == 0) {
                const uint8_t *val = va_arg(*args, const uint8_t*);
                append_v128(buf, val);
                fmt += 4;
            } else if (strncmp(fmt, "t", 1) == 0) {
                const char *spec = va_arg(*args, const char*);
                size_t consumed = parse_tokens(buf, spec, args, error);
                if (*error) {
                    return 0;
                }
                (void)consumed;  // Unused, parse_tokens processes the entire spec string
                fmt += 1;
            } else if (*fmt == '\'') {
                // Hex string in single quotes: %'414243' -> bytes 0x41 0x42 0x43
                fmt++;  // Skip opening quote
                const char *hex_start = fmt;
                while (*fmt != '\'' && *fmt != '\0') {
                    fmt++;
                }
                size_t hex_len = fmt - hex_start;
                // Parse hex string
                for (size_t i = 0; i < hex_len; i += 2) {
                    if (i + 1 < hex_len) {
                        char byte_str[3] = {hex_start[i], hex_start[i+1], '\0'};
                        uint8_t byte = (uint8_t)strtol(byte_str, NULL, 16);
                        buffer_append(buf, &byte, 1);
                    }
                }
                if (*fmt == '\'') fmt++;  // Skip closing quote
            } else {
                // Unknown specifier
                *error = true;
                return 0;
            }
            continue;
        }

        // Extract a single token (whitespace or special char delimited)
        const char *token_start = fmt;
        while (*fmt != '\0' && !isspace(*fmt) && *fmt != '[' && *fmt != ']' &&
               *fmt != '{' && *fmt != '}' && *fmt != ',' && *fmt != '%' && *fmt != '\'') {
            fmt++;
        }
        size_t token_len = fmt - token_start;

        if (token_len > 0) {
            // Check if it's a keyword
            const token_entry_t *entry = lookup_token(token_start, token_len);
            if (entry != NULL) {
                buffer_append(buf, entry->bytes, entry->len);
            } else {
                // Check if it's a number
                size_t suffix_offset;
                int num_type = classify_number(token_start, token_len, &suffix_offset);

                if (num_type == 1) {
                    // Integer: parse as varint
                    // Need null-terminated string for strtoll, so use stack buffer
                    char numbuf[32];
                    size_t copy_len = token_len < sizeof(numbuf) - 1 ? token_len : sizeof(numbuf) - 1;
                    memcpy(numbuf, token_start, copy_len);
                    numbuf[copy_len] = '\0';
                    int64_t val = strtoll(numbuf, NULL, 0);
                    append_varint_signed(buf, val);
                } else if (num_type == 2) {
                    // Float - need null-terminated string
                    // Use stack buffer to avoid allocation
                    char numbuf[32];
                    size_t copy_len = token_len < sizeof(numbuf) - 1 ? token_len : sizeof(numbuf) - 1;
                    memcpy(numbuf, token_start, copy_len);
                    numbuf[copy_len] = '\0';

                    if (suffix_offset > 0) {
                        const char *suffix = numbuf + suffix_offset;
                        if (strcmp(suffix, "f32") == 0) {
                            float val = strtof(numbuf, NULL);
                            append_f32(buf, val);
                        } else if (strcmp(suffix, "f64") == 0) {
                            double val = strtod(numbuf, NULL);
                            append_f64(buf, val);
                        } else {
                            // f32/f64 suffix is mandatory
                            *error = true;
                            return 0;
                        }
                    } else {
                        *error = true;
                        return 0;
                    }
                } else {
                    // Unknown token - return error
                    *error = true;
                    return 0;
                }
            }
        }
    }

    return fmt - start;
}

// Main function
wah_error_t wah_parse_module_from_specv(wah_module_t *module, const char *fmt, va_list args) {
    if (!module || !fmt) {
        return WAH_ERROR_BAD_SPEC;
    }

    buffer_t buf;
    buffer_init(&buf);

    bool error = false;
    parse_tokens(&buf, fmt, &args, &error);

    if (error) {
        buffer_free(&buf);
        return WAH_ERROR_BAD_SPEC;
    }

#ifdef WAH_DEBUG
    printf("Generated binary (%zu bytes):\n", buf.size);
    for (size_t i = 0; i < buf.size; i++) {
        printf("%02x ", buf.data[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    if (buf.size % 16 != 0) printf("\n");
#endif

    // Parse the module
    wah_error_t err = wah_parse_module(buf.data, buf.size, module);

    buffer_free(&buf);
    return err;
}

wah_error_t wah_parse_module_from_spec(wah_module_t *module, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    wah_error_t err = wah_parse_module_from_specv(module, fmt, args);

    va_end(args);
    return err;
}

// ---------------------------------------------------------------------------
// Assertion Utilities for Tests
// ---------------------------------------------------------------------------

#include <inttypes.h>

// Internal implementation functions (with file/line context)
void assert_eq_i32(const char *file, int line, int32_t actual, int32_t expected, const char *expr) {
    if (actual != expected) {
        fprintf(stderr, "%s:%d: Assertion failed: %s (expected %" PRId32 ", got %" PRId32 ")\n",
                file, line, expr, expected, actual);
        exit(1);
    }
}

void assert_eq_u32(const char *file, int line, uint32_t actual, uint32_t expected, const char *expr) {
    if (actual != expected) {
        fprintf(stderr, "%s:%d: Assertion failed: %s (expected %" PRIu32 ", got %" PRIu32 ")\n",
                file, line, expr, expected, actual);
        exit(1);
    }
}

void assert_eq_i64(const char *file, int line, int64_t actual, int64_t expected, const char *expr) {
    if (actual != expected) {
        fprintf(stderr, "%s:%d: Assertion failed: %s (expected %" PRId64 ", got %" PRId64 ")\n",
                file, line, expr, expected, actual);
        exit(1);
    }
}

void assert_eq_u64(const char *file, int line, uint64_t actual, uint64_t expected, const char *expr) {
    if (actual != expected) {
        fprintf(stderr, "%s:%d: Assertion failed: %s (expected %" PRIu64 ", got %" PRIu64 ")\n",
                file, line, expr, expected, actual);
        exit(1);
    }
}

void assert_eq_f32(const char *file, int line, float actual, float expected, float epsilon, const char *expr) {
    if (fabsf(actual - expected) > epsilon) {
        fprintf(stderr, "%s:%d: Assertion failed: %s (expected %f ± %f, got %f)\n",
                file, line, expr, expected, epsilon, actual);
        exit(1);
    }
}

void assert_eq_f64(const char *file, int line, double actual, double expected, double epsilon, const char *expr) {
    if (fabs(actual - expected) > epsilon) {
        fprintf(stderr, "%s:%d: Assertion failed: %s (expected %f ± %f, got %f)\n",
                file, line, expr, expected, epsilon, actual);
        exit(1);
    }
}

void assert_eq_str(const char *file, int line, const char *actual, const char *expected, const char *expr) {
    if (strcmp(actual, expected) != 0) {
        fprintf(stderr, "%s:%d: Assertion failed: %s (expected \"%s\", got \"%s\")\n",
                file, line, expr, expected, actual);
        exit(1);
    }
}

void assert_err(const char *file, int line, wah_error_t actual, wah_error_t expected, const char *expr) {
    if (actual != expected) {
        fprintf(stderr, "%s:%d: Assertion failed: %s (expected %s, got %s)\n",
                file, line, expr, wah_strerror(expected), wah_strerror(actual));
        exit(1);
    }
}

void assert_ok(const char *file, int line, wah_error_t err, const char *expr) {
    if (err != WAH_OK) {
        fprintf(stderr, "%s:%d: Assertion failed: %s (expected WAH_OK, got %s)\n",
                file, line, expr, wah_strerror(err));
        exit(1);
    }
}

void assert_true(const char *file, int line, bool condition, const char *expr) {
    if (!condition) {
        fprintf(stderr, "%s:%d: Assertion failed: %s (expected true)\n",
                file, line, expr);
        exit(1);
    }
}

void assert_false(const char *file, int line, bool condition, const char *expr) {
    if (condition) {
        fprintf(stderr, "%s:%d: Assertion failed: %s (expected false)\n",
                file, line, expr);
        exit(1);
    }
}

void assert_null(const char *file, int line, const void *ptr, const char *expr) {
    if (ptr != NULL) {
        fprintf(stderr, "%s:%d: Assertion failed: %s (expected NULL)\n",
                file, line, expr);
        exit(1);
    }
}

void assert_not_null(const char *file, int line, const void *ptr, const char *expr) {
    if (ptr == NULL) {
        fprintf(stderr, "%s:%d: Assertion failed: %s (expected non-NULL)\n",
                file, line, expr);
        exit(1);
    }
}

void assert_eq_ptr(const char *file, int line, const void *actual, const void *expected, const char *expr) {
    if (actual != expected) {
        fprintf(stderr, "%s:%d: Assertion failed: %s (expected %p, got %p)\n",
                file, line, expr, expected, actual);
        exit(1);
    }
}

// Public macros that inject __FILE__ and __LINE__
#define assert_eq_i32(actual, expected) assert_eq_i32(__FILE__, __LINE__, (actual), (expected), #actual " == " #expected)
#define assert_eq_u32(actual, expected) assert_eq_u32(__FILE__, __LINE__, (actual), (expected), #actual " == " #expected)
#define assert_eq_i64(actual, expected) assert_eq_i64(__FILE__, __LINE__, (actual), (expected), #actual " == " #expected)
#define assert_eq_u64(actual, expected) assert_eq_u64(__FILE__, __LINE__, (actual), (expected), #actual " == " #expected)
#define assert_eq_f32(actual, expected, epsilon) assert_eq_f32(__FILE__, __LINE__, (actual), (expected), (epsilon), #actual " == " #expected " within " #epsilon)
#define assert_eq_f64(actual, expected, epsilon) assert_eq_f64(__FILE__, __LINE__, (actual), (expected), (epsilon), #actual " == " #expected " within " #epsilon)
#define assert_eq_str(actual, expected) assert_eq_str(__FILE__, __LINE__, (actual), (expected), #actual " == " #expected)
#define assert_err(actual, expected) assert_err(__FILE__, __LINE__, (actual), (expected), #actual " == " #expected)
#define assert_ok(err) assert_ok(__FILE__, __LINE__, (err), #err " == WAH_OK")
#define assert_true(condition) assert_true(__FILE__, __LINE__, (condition), #condition)
#define assert_false(condition) assert_false(__FILE__, __LINE__, (condition), #condition)
#define assert_null(ptr) assert_null(__FILE__, __LINE__, (ptr), #ptr " == NULL")
#define assert_not_null(ptr) assert_not_null(__FILE__, __LINE__, (ptr), #ptr " != NULL")
#define assert_eq_ptr(actual, expected) assert_eq_ptr(__FILE__, __LINE__, (actual), (expected), #actual " == " #expected)
