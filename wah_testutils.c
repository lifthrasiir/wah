// This file implements wah_parse_module_from_spec,
// which generates a WebAssembly module from a simple format string specification.
//
// The specification consists of tokens. Tokens can be:
// - Punctuations: `[`, `]`, `{`, `}`, `,`
// - Numbers: `42`, `-123`, `3.14f32`, `2.71828f64`
// - Verbatims: `'foo'`, '%'414243'` etc.
// - Bare words (anything else): `wasm`, `types`, `i32`, `local.get`, etc.
// - Placeholders: `%d`, `%i32`, `%f64`
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
// `%d` expects an int64_t value and emits a signed LEB128 varint.
// `%i32` and `%i64` expect int32_t and int64_t values respectively, and emits 4- and 8-byte little-endian integers.
// `%f32` and `%f64` expect float and double values respectively, and emits 4- and 8-byte IEEE 754 encodings.

#include "wah.h"
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

    // Types
    TOKEN("fn", 0x60),        // Function type
    TOKEN("i32", 0x7f),       // i32 type
    TOKEN("i64", 0x7e),       // i64 type
    TOKEN("f32", 0x7d),       // f32 type
    TOKEN("f64", 0x7c),       // f64 type
    TOKEN("void", 0x40),      // Void return
    TOKEN("funcref", 0x70),   // funcref type
    TOKEN("externref", 0x6f), // externref type

    // Indices
    TOKEN("fn#", 0x00),       // Function index space
    TOKEN("table#", 0x01),    // Table index space
    TOKEN("mem#", 0x02),      // Memory index space
    TOKEN("global#", 0x03),   // Global index space

    // Elements
    TOKEN("elem.passive", 0x00),
    TOKEN("elem.active.table#0", 0x01),
    TOKEN("elem.declarative", 0x02),
    TOKEN("elem.active.table#", 0x03),
    TOKEN("elem.passive.expr", 0x04),
    TOKEN("elem.active.expr.table#0", 0x05),
    TOKEN("elem.declarative.expr", 0x06),
    TOKEN("elem.active.expr.table#", 0x07),

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
    TOKEN("end", 0x0b),
    TOKEN("br", 0x0c),
    TOKEN("br_if", 0x0d),
    TOKEN("br_table", 0x0e),
    TOKEN("return", 0x0f),
    TOKEN("call", 0x10),
    TOKEN("call_indirect", 0x11),
    TOKEN("drop", 0x1a),
    TOKEN("select", 0x1b),
    TOKEN("local.get", 0x20),
    TOKEN("local.set", 0x21),
    TOKEN("local.tee", 0x22),
    TOKEN("global.get", 0x23),
    TOKEN("global.set", 0x24),
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
    TOKEN("memory.init", 0xfc, 0x08),
    TOKEN("memory.copy", 0xfc, 0x0a),
    TOKEN("memory.fill", 0xfc, 0x0b),

    // Opcode parts
    TOKEN("offset=0", 0x00),
    TOKEN("align=1", 0x00),
    TOKEN("align=2", 0x01),
    TOKEN("align=4", 0x02),
    TOKEN("align=8", 0x03),

    // Miscellaneous
    TOKEN("mut", 0x01),       // Mutable flag for globals
    TOKEN("limits.i32/1", 0x00),
    TOKEN("limits.i32/2", 0x01),

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
static const token_entry_t *lookup_token(const char *token) {
    for (int i = 0; token_table[i].name != NULL; i++) {
        if (strcmp(token, token_table[i].name) == 0) {
            return &token_table[i];
        }
    }
    return NULL;
}

// Check if a string is a valid number
// Returns: 0 = not a number, 1 = integer, 2 = float
// If float, sets suffix_end to point to the suffix (e.g., "f32")
static int classify_number(const char *token, const char **suffix_end) {
    *suffix_end = NULL;

    const char *p = token;
    if (*p == '-' || *p == '+') p++;

    int has_digits = 0;
    int has_dot = 0;

    while (*p) {
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
    if (*p != '\0') {
        *suffix_end = p;
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

            if (strncmp(fmt, "d", 1) == 0) {
                int64_t val = va_arg(*args, int64_t);
                append_varint_signed(buf, val);
                fmt += 1;
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
            } else if (strncmp(fmt, "f64", 4) == 0) {
                double val = va_arg(*args, double);
                append_f64(buf, val);
                fmt += 4;
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
                // Unknown specifier, skip it
                continue;
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
            // Copy token to null-terminated string
            char *token = (char *)malloc(token_len + 1);
            memcpy(token, token_start, token_len);
            token[token_len] = '\0';

            // Check if it's a keyword
            const token_entry_t *entry = lookup_token(token);
            if (entry != NULL) {
                buffer_append(buf, entry->bytes, entry->len);
            } else {
                // Check if it's a number
                const char *suffix;
                int num_type = classify_number(token, &suffix);

                if (num_type == 1) {
                    // Integer: parse as varint
                    int64_t val = strtoll(token, NULL, 0);
                    append_varint_signed(buf, val);
                } else if (num_type == 2) {
                    // Float
                    if (suffix && strcmp(suffix, "f32") == 0) {
                        float val = strtof(token, NULL);
                        append_f32(buf, val);
                    } else if (suffix && strcmp(suffix, "f64") == 0) {
                        double val = strtod(token, NULL);
                        append_f64(buf, val);
                    } else {
                        // f32/f64 suffix is mandatory
                        *error = true;
                        free(token);
                        return 0;
                    }
                } else {
                    // Unknown token - return error
                    *error = true;
                    free(token);
                    return 0;
                }
            }

            free(token);
        }
    }

    return fmt - start;
}

// Main function
wah_error_t wah_parse_module_from_spec(wah_module_t *module, const char *fmt, ...) {
    if (!module || !fmt) {
        return WAH_ERROR_BAD_SPEC;
    }

    buffer_t buf;
    buffer_init(&buf);

    va_list args;
    va_start(args, fmt);

    bool error = false;
    parse_tokens(&buf, fmt, &args, &error);

    va_end(args);

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
