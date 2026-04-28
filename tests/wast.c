#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *data;
    size_t len;
    int is_string;
} wast_atom_t;

typedef struct wast_node_s {
    int is_atom;
    int line;
    int column;
    wast_atom_t atom;
    struct wast_node_s **children;
    size_t child_count;
    size_t child_capacity;
} wast_node_t;

typedef struct {
    const char *input;
    size_t length;
    size_t offset;
    int line;
    int column;
    char error[256];
} wast_parser_t;

static void wast_set_error(wast_parser_t *parser, const char *message) {
    if (parser->error[0] != '\0') {
        return;
    }
    (void)snprintf(parser->error, sizeof(parser->error),
                   "line %d, column %d: %s", parser->line, parser->column, message);
}

static int wast_peek(const wast_parser_t *parser) {
    if (parser->offset >= parser->length) {
        return EOF;
    }
    return (unsigned char)parser->input[parser->offset];
}

static int wast_peek_n(const wast_parser_t *parser, size_t n) {
    size_t pos = parser->offset + n;
    if (pos >= parser->length) {
        return EOF;
    }
    return (unsigned char)parser->input[pos];
}

static int wast_next(wast_parser_t *parser) {
    int ch = wast_peek(parser);
    if (ch == EOF) {
        return EOF;
    }
    parser->offset++;
    if (ch == '\n') {
        parser->line++;
        parser->column = 1;
    } else {
        parser->column++;
    }
    return ch;
}

static int wast_buf_push(char **buf, size_t *len, size_t *cap, char byte) {
    if (*len + 1 > *cap) {
        size_t new_cap = (*cap == 0) ? 32 : (*cap * 2);
        char *new_buf = (char *)realloc(*buf, new_cap);
        if (!new_buf) {
            return 0;
        }
        *buf = new_buf;
        *cap = new_cap;
    }
    (*buf)[(*len)++] = byte;
    return 1;
}

static int wast_buf_append_utf8(char **buf, size_t *len, size_t *cap, uint32_t cp) {
    if (cp <= 0x7F) {
        return wast_buf_push(buf, len, cap, (char)cp);
    }
    if (cp <= 0x7FF) {
        return wast_buf_push(buf, len, cap, (char)(0xC0 | (cp >> 6))) &&
               wast_buf_push(buf, len, cap, (char)(0x80 | (cp & 0x3F)));
    }
    if (cp <= 0xFFFF) {
        return wast_buf_push(buf, len, cap, (char)(0xE0 | (cp >> 12))) &&
               wast_buf_push(buf, len, cap, (char)(0x80 | ((cp >> 6) & 0x3F))) &&
               wast_buf_push(buf, len, cap, (char)(0x80 | (cp & 0x3F)));
    }
    if (cp <= 0x10FFFF) {
        return wast_buf_push(buf, len, cap, (char)(0xF0 | (cp >> 18))) &&
               wast_buf_push(buf, len, cap, (char)(0x80 | ((cp >> 12) & 0x3F))) &&
               wast_buf_push(buf, len, cap, (char)(0x80 | ((cp >> 6) & 0x3F))) &&
               wast_buf_push(buf, len, cap, (char)(0x80 | (cp & 0x3F)));
    }
    return 0;
}

static int wast_hex_value(int ch) {
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    return -1;
}

static int wast_skip_block_comment(wast_parser_t *parser) {
    int depth = 1;
    wast_next(parser);
    wast_next(parser);
    while (depth > 0) {
        int ch = wast_peek(parser);
        if (ch == EOF) {
            wast_set_error(parser, "unterminated block comment");
            return 0;
        }
        if (ch == '(' && wast_peek_n(parser, 1) == ';') {
            wast_next(parser);
            wast_next(parser);
            depth++;
            continue;
        }
        if (ch == ';' && wast_peek_n(parser, 1) == ')') {
            wast_next(parser);
            wast_next(parser);
            depth--;
            continue;
        }
        wast_next(parser);
    }
    return 1;
}

static int wast_skip_ws(wast_parser_t *parser) {
    for (;;) {
        int ch = wast_peek(parser);
        if (ch == EOF) {
            return 1;
        }
        if (isspace(ch)) {
            wast_next(parser);
            continue;
        }
        if (ch == ';' && wast_peek_n(parser, 1) == ';') {
            while ((ch = wast_peek(parser)) != EOF && ch != '\n') {
                wast_next(parser);
            }
            continue;
        }
        if (ch == '(' && wast_peek_n(parser, 1) == ';') {
            if (!wast_skip_block_comment(parser)) {
                return 0;
            }
            continue;
        }
        return 1;
    }
}

static void wast_node_free(wast_node_t *node) {
    size_t i;
    if (!node) {
        return;
    }
    if (node->is_atom) {
        free(node->atom.data);
    } else {
        for (i = 0; i < node->child_count; ++i) {
            wast_node_free(node->children[i]);
        }
        free(node->children);
    }
    free(node);
}

static wast_node_t *wast_new_atom(wast_parser_t *parser, char *data, size_t len, int is_string) {
    wast_node_t *node = (wast_node_t *)calloc(1, sizeof(*node));
    if (!node) {
        free(data);
        wast_set_error(parser, "out of memory");
        return NULL;
    }
    node->is_atom = 1;
    node->line = parser->line;
    node->column = parser->column;
    node->atom.data = data;
    node->atom.len = len;
    node->atom.is_string = is_string;
    return node;
}

static wast_node_t *wast_new_list(wast_parser_t *parser) {
    wast_node_t *node = (wast_node_t *)calloc(1, sizeof(*node));
    if (!node) {
        wast_set_error(parser, "out of memory");
        return NULL;
    }
    node->is_atom = 0;
    node->line = parser->line;
    node->column = parser->column;
    return node;
}

static int wast_list_push(wast_parser_t *parser, wast_node_t *list, wast_node_t *child) {
    if (list->child_count == list->child_capacity) {
        size_t new_cap = list->child_capacity == 0 ? 8 : list->child_capacity * 2;
        wast_node_t **new_children =
            (wast_node_t **)realloc(list->children, new_cap * sizeof(*new_children));
        if (!new_children) {
            wast_set_error(parser, "out of memory");
            return 0;
        }
        list->children = new_children;
        list->child_capacity = new_cap;
    }
    list->children[list->child_count++] = child;
    return 1;
}

static wast_node_t *wast_parse_expr(wast_parser_t *parser);

static wast_node_t *wast_parse_string(wast_parser_t *parser) {
    char *buf = NULL;
    size_t len = 0;
    size_t cap = 0;
    wast_next(parser);
    for (;;) {
        int ch = wast_peek(parser);
        if (ch == EOF) {
            free(buf);
            wast_set_error(parser, "unterminated string literal");
            return NULL;
        }
        if (ch == '"') {
            wast_next(parser);
            break;
        }
        if (ch != '\\') {
            if (!wast_buf_push(&buf, &len, &cap, (char)wast_next(parser))) {
                free(buf);
                wast_set_error(parser, "out of memory");
                return NULL;
            }
            continue;
        }

        wast_next(parser);
        ch = wast_peek(parser);
        if (ch == EOF) {
            free(buf);
            wast_set_error(parser, "unterminated string escape");
            return NULL;
        }

        if (ch == 'n' || ch == 'r' || ch == 't' || ch == '\\' || ch == '"' || ch == '\'') {
            char mapped = (ch == 'n') ? '\n' :
                          (ch == 'r') ? '\r' :
                          (ch == 't') ? '\t' : (char)ch;
            wast_next(parser);
            if (!wast_buf_push(&buf, &len, &cap, mapped)) {
                free(buf);
                wast_set_error(parser, "out of memory");
                return NULL;
            }
            continue;
        }

        if (ch == 'u' && wast_peek_n(parser, 1) == '{') {
            uint32_t cp = 0;
            wast_next(parser);
            wast_next(parser);
            if (wast_peek(parser) == '}') {
                free(buf);
                wast_set_error(parser, "empty unicode escape");
                return NULL;
            }
            while ((ch = wast_peek(parser)) != EOF && ch != '}') {
                int hv = wast_hex_value(ch);
                if (hv < 0) {
                    free(buf);
                    wast_set_error(parser, "invalid unicode escape");
                    return NULL;
                }
                cp = (cp << 4) | (uint32_t)hv;
                wast_next(parser);
            }
            if (wast_peek(parser) != '}') {
                free(buf);
                wast_set_error(parser, "unterminated unicode escape");
                return NULL;
            }
            wast_next(parser);
            if (!wast_buf_append_utf8(&buf, &len, &cap, cp)) {
                free(buf);
                wast_set_error(parser, "invalid unicode code point");
                return NULL;
            }
            continue;
        }

        {
            int hi = wast_hex_value(ch);
            int lo = wast_hex_value(wast_peek_n(parser, 1));
            if (hi >= 0 && lo >= 0) {
                wast_next(parser);
                wast_next(parser);
                if (!wast_buf_push(&buf, &len, &cap, (char)((hi << 4) | lo))) {
                    free(buf);
                    wast_set_error(parser, "out of memory");
                    return NULL;
                }
                continue;
            }
        }

        free(buf);
        wast_set_error(parser, "unsupported string escape");
        return NULL;
    }
    return wast_new_atom(parser, buf, len, 1);
}

static int wast_is_atom_delim(int ch) {
    return ch == EOF || isspace(ch) || ch == '(' || ch == ')' || ch == '"' ||
           (ch == ';');
}

static wast_node_t *wast_parse_symbol(wast_parser_t *parser) {
    char *buf = NULL;
    size_t len = 0;
    size_t cap = 0;
    while (!wast_is_atom_delim(wast_peek(parser))) {
        if (!wast_buf_push(&buf, &len, &cap, (char)wast_next(parser))) {
            free(buf);
            wast_set_error(parser, "out of memory");
            return NULL;
        }
    }
    if (len == 0) {
        free(buf);
        wast_set_error(parser, "expected atom");
        return NULL;
    }
    return wast_new_atom(parser, buf, len, 0);
}

static wast_node_t *wast_parse_list(wast_parser_t *parser) {
    wast_node_t *node = wast_new_list(parser);
    if (!node) {
        return NULL;
    }
    wast_next(parser);
    if (!wast_skip_ws(parser)) {
        wast_node_free(node);
        return NULL;
    }
    while (wast_peek(parser) != ')') {
        wast_node_t *child;
        if (wast_peek(parser) == EOF) {
            wast_set_error(parser, "unterminated list");
            wast_node_free(node);
            return NULL;
        }
        child = wast_parse_expr(parser);
        if (!child) {
            wast_node_free(node);
            return NULL;
        }
        if (!wast_list_push(parser, node, child)) {
            wast_node_free(child);
            wast_node_free(node);
            return NULL;
        }
        if (!wast_skip_ws(parser)) {
            wast_node_free(node);
            return NULL;
        }
    }
    wast_next(parser);
    return node;
}

static wast_node_t *wast_parse_expr(wast_parser_t *parser) {
    if (!wast_skip_ws(parser)) {
        return NULL;
    }
    if (wast_peek(parser) == EOF) {
        return NULL;
    }
    if (wast_peek(parser) == '(') {
        return wast_parse_list(parser);
    }
    if (wast_peek(parser) == '"') {
        return wast_parse_string(parser);
    }
    if (wast_peek(parser) == ')') {
        wast_set_error(parser, "unexpected ')'");
        return NULL;
    }
    return wast_parse_symbol(parser);
}

static void wast_parser_init(wast_parser_t *parser, const char *input, size_t length) {
    memset(parser, 0, sizeof(*parser));
    parser->input = input;
    parser->length = length;
    parser->line = 1;
    parser->column = 1;
}

static wast_node_t *wast_parse_next(wast_parser_t *parser) {
    if (!wast_skip_ws(parser)) {
        return NULL;
    }
    if (wast_peek(parser) == EOF) {
        return NULL;
    }
    return wast_parse_expr(parser);
}

static int wast_atom_eq(const wast_node_t *node, const char *text) {
    size_t len;
    if (!node || !node->is_atom) {
        return 0;
    }
    len = strlen(text);
    return node->atom.len == len && memcmp(node->atom.data, text, len) == 0;
}

static char *wast_atom_dup_cstr(const wast_node_t *node) {
    char *text;
    if (!node || !node->is_atom) {
        return NULL;
    }
    text = (char *)malloc(node->atom.len + 1);
    if (!text) {
        return NULL;
    }
    if (node->atom.len > 0) {
        memcpy(text, node->atom.data, node->atom.len);
    }
    text[node->atom.len] = '\0';
    return text;
}

static int wast_atom_to_cstr(const wast_node_t *node, char *buf, size_t buf_size) {
    if (!node || !node->is_atom || node->atom.len + 1 > buf_size) {
        return 0;
    }
    memcpy(buf, node->atom.data, node->atom.len);
    buf[node->atom.len] = '\0';
    return 1;
}

static uint32_t wast_read_u32_le(const uint8_t *bytes) {
    return ((uint32_t)bytes[0]) |
           ((uint32_t)bytes[1] << 8) |
           ((uint32_t)bytes[2] << 16) |
           ((uint32_t)bytes[3] << 24);
}

static uint64_t wast_read_u64_le(const uint8_t *bytes) {
    return ((uint64_t)bytes[0]) |
           ((uint64_t)bytes[1] << 8) |
           ((uint64_t)bytes[2] << 16) |
           ((uint64_t)bytes[3] << 24) |
           ((uint64_t)bytes[4] << 32) |
           ((uint64_t)bytes[5] << 40) |
           ((uint64_t)bytes[6] << 48) |
           ((uint64_t)bytes[7] << 56);
}

static int wast_parse_u64(const wast_node_t *node, uint64_t *out) {
    char *text;
    char *clean;
    size_t i, j;
    char *endptr;
    unsigned long long value;
    if (!node || !node->is_atom) {
        return 0;
    }
    text = wast_atom_dup_cstr(node);
    if (!text) {
        return 0;
    }
    clean = (char *)malloc(node->atom.len + 1);
    if (!clean) {
        free(text);
        return 0;
    }
    for (i = 0, j = 0; i < node->atom.len; ++i) {
        if (text[i] != '_') {
            clean[j++] = text[i];
        }
    }
    clean[j] = '\0';
    errno = 0;
    value = strtoull(clean, &endptr, 0);
    free(text);
    {
        int ok = (errno == 0 && endptr && *endptr == '\0');
        free(clean);
        if (!ok) {
            return 0;
        }
    }
    *out = (uint64_t)value;
    return 1;
}

static int wast_parse_i64(const wast_node_t *node, int64_t *out) {
    char *text;
    char *clean;
    size_t i, j;
    int negative = 0;
    uint64_t value = 0;
    if (!node || !node->is_atom) {
        return 0;
    }
    text = wast_atom_dup_cstr(node);
    if (!text) {
        return 0;
    }
    clean = (char *)malloc(node->atom.len + 1);
    if (!clean) {
        free(text);
        return 0;
    }
    for (i = 0, j = 0; i < node->atom.len; ++i) {
        if (text[i] != '_') {
            clean[j++] = text[i];
        }
    }
    clean[j] = '\0';
    if (clean[0] == '-') {
        negative = 1;
        memmove(clean, clean + 1, strlen(clean));
    } else if (clean[0] == '+') {
        memmove(clean, clean + 1, strlen(clean));
    }
    if (clean[0] == '\0') {
        free(text);
        free(clean);
        return 0;
    }
    errno = 0;
    value = strtoull(clean, NULL, 0);
    free(text);
    free(clean);
    if (errno != 0) {
        return 0;
    }
    *out = negative ? -(int64_t)value : (int64_t)value;
    return 1;
}

static int wast_parse_f32_bits(const wast_node_t *node, uint32_t *out) {
    char *text;
    char *clean;
    size_t i, j;
    float f;
    if (!node || !node->is_atom) {
        return 0;
    }
    if (wast_atom_eq(node, "inf") || wast_atom_eq(node, "+inf")) {
        *out = 0x7f800000u;
        return 1;
    }
    if (wast_atom_eq(node, "-inf")) {
        *out = 0xff800000u;
        return 1;
    }
    if (wast_atom_eq(node, "nan:canonical")) {
        *out = 0x7fc00000u;
        return 1;
    }
    if (wast_atom_eq(node, "nan:arithmetic")) {
        *out = 0x7fc00000u;
        return 1;
    }
    text = wast_atom_dup_cstr(node);
    if (!text) {
        return 0;
    }
    clean = (char *)malloc(node->atom.len + 1);
    if (!clean) {
        free(text);
        return 0;
    }
    for (i = 0, j = 0; i < node->atom.len; ++i) {
        if (text[i] != '_') {
            clean[j++] = text[i];
        }
    }
    clean[j] = '\0';
    if ((strncmp(clean, "-nan:0x", 7) == 0) || (strncmp(clean, "+nan:0x", 7) == 0) ||
        (strncmp(clean, "nan:0x", 6) == 0)) {
        uint32_t payload = 0;
        char *payload_text = strchr(clean, ':');
        int negative = clean[0] == '-';
        if (!payload_text) {
            free(text);
            free(clean);
            return 0;
        }
        payload_text++;
        payload = (uint32_t)strtoul(payload_text, NULL, 0);
        *out = (negative ? 0x80000000u : 0) | 0x7f800000u | payload;
        free(text);
        free(clean);
        return 1;
    }
    errno = 0;
    f = strtof(clean, NULL);
    memcpy(out, &f, sizeof(*out));
    free(text);
    free(clean);
    return errno == 0;
}

static int wast_parse_f64_bits(const wast_node_t *node, uint64_t *out) {
    char *text;
    char *clean;
    size_t i, j;
    double d;
    if (!node || !node->is_atom) {
        return 0;
    }
    if (wast_atom_eq(node, "inf") || wast_atom_eq(node, "+inf")) {
        *out = UINT64_C(0x7ff0000000000000);
        return 1;
    }
    if (wast_atom_eq(node, "-inf")) {
        *out = UINT64_C(0xfff0000000000000);
        return 1;
    }
    if (wast_atom_eq(node, "nan:canonical")) {
        *out = UINT64_C(0x7ff8000000000000);
        return 1;
    }
    if (wast_atom_eq(node, "nan:arithmetic")) {
        *out = UINT64_C(0x7ff8000000000000);
        return 1;
    }
    text = wast_atom_dup_cstr(node);
    if (!text) {
        return 0;
    }
    clean = (char *)malloc(node->atom.len + 1);
    if (!clean) {
        free(text);
        return 0;
    }
    for (i = 0, j = 0; i < node->atom.len; ++i) {
        if (text[i] != '_') {
            clean[j++] = text[i];
        }
    }
    clean[j] = '\0';
    if ((strncmp(clean, "-nan:0x", 7) == 0) || (strncmp(clean, "+nan:0x", 7) == 0) ||
        (strncmp(clean, "nan:0x", 6) == 0)) {
        uint64_t payload = 0;
        char *payload_text = strchr(clean, ':');
        int negative = clean[0] == '-';
        if (!payload_text) {
            free(text);
            free(clean);
            return 0;
        }
        payload_text++;
        payload = strtoull(payload_text, NULL, 0);
        *out = (negative ? UINT64_C(0x8000000000000000) : 0) |
               UINT64_C(0x7ff0000000000000) | payload;
        free(text);
        free(clean);
        return 1;
    }
    errno = 0;
    d = strtod(clean, NULL);
    memcpy(out, &d, sizeof(*out));
    free(text);
    free(clean);
    return errno == 0;
}

static int wast_append_bytes(uint8_t **out, size_t *out_len, size_t *out_cap,
                             const void *bytes, size_t len) {
    uint8_t *new_buf;
    size_t new_cap;
    if (*out_len + len > *out_cap) {
        new_cap = (*out_cap == 0) ? 256 : (*out_cap * 2);
        while (*out_len + len > new_cap) {
            new_cap *= 2;
        }
        new_buf = (uint8_t *)realloc(*out, new_cap);
        if (!new_buf) {
            return 0;
        }
        *out = new_buf;
        *out_cap = new_cap;
    }
    memcpy(*out + *out_len, bytes, len);
    *out_len += len;
    return 1;
}

static int wast_gather_binary_module(const wast_node_t *module_node, uint8_t **out_bytes, size_t *out_len) {
    size_t i;
    size_t cap = 0;
    uint8_t *bytes = NULL;
    *out_bytes = NULL;
    *out_len = 0;
    if (!module_node || module_node->is_atom || module_node->child_count < 2 ||
        !wast_atom_eq(module_node->children[0], "module")) {
        return 0;
    }
    for (i = 1; i < module_node->child_count; ++i) {
        const wast_node_t *child = module_node->children[i];
        if (!child->is_atom) {
            continue;
        }
        if (wast_atom_eq(child, "definition") || wast_atom_eq(child, "binary") ||
            (!child->atom.is_string && child->atom.len > 0 && child->atom.data[0] == '$')) {
            continue;
        }
        if (!child->atom.is_string) {
            free(bytes);
            return 0;
        }
        if (!wast_append_bytes(&bytes, out_len, &cap, child->atom.data, child->atom.len)) {
            free(bytes);
            return 0;
        }
    }
    *out_bytes = bytes;
    return 1;
}

typedef struct {
    wah_value_t value;
    wah_type_t type;
} wast_const_value_t;

#define WAST_MAX_RESULTS 16

typedef struct {
    wast_const_value_t values[WAST_MAX_RESULTS];
    uint32_t count;
} wast_const_result_t;

typedef enum {
    WAST_NUM_LITERAL = 0,
    WAST_NUM_NAN_CANONICAL,
    WAST_NUM_NAN_ARITHMETIC,
} wast_num_pat_kind_t;

typedef struct {
    wast_num_pat_kind_t kind;
    union {
        int32_t i32;
        int64_t i64;
        uint32_t f32_bits;
        uint64_t f64_bits;
    } literal;
} wast_num_pat_t;

typedef enum {
    WAST_PAT_I32 = 0,
    WAST_PAT_I64,
    WAST_PAT_F32,
    WAST_PAT_F64,
    WAST_PAT_V128,
    WAST_PAT_REF_ANY,
    WAST_PAT_REF_NULL,
    WAST_PAT_REF_FUNC,
    WAST_PAT_REF_EXTERN,
    WAST_PAT_REF_HOST,
    WAST_PAT_REF_OTHER,
    WAST_PAT_EITHER,
} wast_result_pat_kind_t;

typedef struct wast_result_pat_s wast_result_pat_t;

struct wast_result_pat_s {
    wast_result_pat_kind_t kind;
    char *ref_kind_name;
    uint32_t host_ref_id;
    char shape[16];
    uint32_t lane_count;
    wast_num_pat_t lanes[16];
    wast_num_pat_t scalar;
    wast_result_pat_t *alternatives;
    size_t alternative_count;
};

typedef void *(*wast_resolve_ref_fn)(void *userdata, uint32_t id);

static int wast_parse_num_pattern(const wast_node_t *node, wast_num_pat_t *out, int float_bits) {
    if (wast_atom_eq(node, "nan:canonical")) {
        out->kind = WAST_NUM_NAN_CANONICAL;
        return 1;
    }
    if (wast_atom_eq(node, "nan:arithmetic")) {
        out->kind = WAST_NUM_NAN_ARITHMETIC;
        return 1;
    }
    out->kind = WAST_NUM_LITERAL;
    if (float_bits == 32) {
        return wast_parse_f32_bits(node, &out->literal.f32_bits);
    }
    if (float_bits == 64) {
        return wast_parse_f64_bits(node, &out->literal.f64_bits);
    }
    if (float_bits == -32) {
        return wast_parse_i64(node, (int64_t *)&out->literal.i32);
    }
    return wast_parse_i64(node, &out->literal.i64);
}

static int wast_parse_const(const wast_node_t *node, wast_const_value_t *out,
                            wast_resolve_ref_fn resolve_ref, void *ref_userdata) {
    uint64_t u64;
    int64_t i64;
    if (!node || node->is_atom || node->child_count == 0) {
        return 0;
    }
    if (wast_atom_eq(node->children[0], "i32.const") && node->child_count == 2) {
        if (!wast_parse_i64(node->children[1], &i64)) return 0;
        out->type = WAH_TYPE_I32;
        out->value.i32 = (int32_t)i64;
        return 1;
    }
    if (wast_atom_eq(node->children[0], "i64.const") && node->child_count == 2) {
        if (!wast_parse_i64(node->children[1], &i64)) return 0;
        out->type = WAH_TYPE_I64;
        out->value.i64 = i64;
        return 1;
    }
    if (wast_atom_eq(node->children[0], "f32.const") && node->child_count == 2) {
        out->type = WAH_TYPE_F32;
        return wast_parse_f32_bits(node->children[1], (uint32_t *)&out->value.f32);
    }
    if (wast_atom_eq(node->children[0], "f64.const") && node->child_count == 2) {
        out->type = WAH_TYPE_F64;
        return wast_parse_f64_bits(node->children[1], (uint64_t *)&out->value.f64);
    }
    if (wast_atom_eq(node->children[0], "ref.null")) {
        out->type = WAH_TYPE_EXTERNREF;
        out->value.ref = NULL;
        return 1;
    }
    if (wast_atom_eq(node->children[0], "ref.extern") && node->child_count == 2) {
        if (!wast_parse_u64(node->children[1], &u64)) return 0;
        out->type = WAH_TYPE_EXTERNREF;
        out->value.ref = resolve_ref(ref_userdata, (uint32_t)u64);
        return out->value.ref != NULL;
    }
    if (wast_atom_eq(node->children[0], "ref.host") && node->child_count == 2) {
        if (!wast_parse_u64(node->children[1], &u64)) return 0;
        out->type = WAH_TYPE_ANYREF;
        void *wrapper = resolve_ref(ref_userdata, (uint32_t)u64);
        if (!wrapper) return 0;
        out->value.ref = wrapper;
        return out->value.ref != NULL;
    }
    if (wast_atom_eq(node->children[0], "v128.const") && node->child_count >= 3) {
        wah_v128_t tmp;
        uint8_t bytes[16] = {0};
        size_t i;
        char shape[16];
        if (!wast_atom_to_cstr(node->children[1], shape, sizeof(shape))) {
            return 0;
        }
        memset(&tmp, 0, sizeof(tmp));
        if (strcmp(shape, "i8x16") == 0 && node->child_count == 18) {
            for (i = 0; i < 16; ++i) {
                if (!wast_parse_i64(node->children[i + 2], &i64)) return 0;
                bytes[i] = (uint8_t)i64;
            }
        } else if (strcmp(shape, "i16x8") == 0 && node->child_count == 10) {
            for (i = 0; i < 8; ++i) {
                uint16_t v;
                if (!wast_parse_i64(node->children[i + 2], &i64)) return 0;
                v = (uint16_t)i64;
                bytes[i * 2 + 0] = (uint8_t)(v & 0xff);
                bytes[i * 2 + 1] = (uint8_t)(v >> 8);
            }
        } else if ((strcmp(shape, "i32x4") == 0 || strcmp(shape, "f32x4") == 0) && node->child_count == 6) {
            for (i = 0; i < 4; ++i) {
                uint32_t v32;
                if (shape[0] == 'f') {
                    if (!wast_parse_f32_bits(node->children[i + 2], &v32)) return 0;
                } else {
                    if (!wast_parse_u64(node->children[i + 2], &u64)) return 0;
                    v32 = (uint32_t)u64;
                }
                bytes[i * 4 + 0] = (uint8_t)(v32 & 0xff);
                bytes[i * 4 + 1] = (uint8_t)((v32 >> 8) & 0xff);
                bytes[i * 4 + 2] = (uint8_t)((v32 >> 16) & 0xff);
                bytes[i * 4 + 3] = (uint8_t)((v32 >> 24) & 0xff);
            }
        } else if ((strcmp(shape, "i64x2") == 0 || strcmp(shape, "f64x2") == 0) && node->child_count == 4) {
            for (i = 0; i < 2; ++i) {
                uint64_t v64;
                if (shape[0] == 'f') {
                    if (!wast_parse_f64_bits(node->children[i + 2], &v64)) return 0;
                } else {
                    if (!wast_parse_u64(node->children[i + 2], &u64)) return 0;
                    v64 = u64;
                }
                bytes[i * 8 + 0] = (uint8_t)(v64 & 0xff);
                bytes[i * 8 + 1] = (uint8_t)((v64 >> 8) & 0xff);
                bytes[i * 8 + 2] = (uint8_t)((v64 >> 16) & 0xff);
                bytes[i * 8 + 3] = (uint8_t)((v64 >> 24) & 0xff);
                bytes[i * 8 + 4] = (uint8_t)((v64 >> 32) & 0xff);
                bytes[i * 8 + 5] = (uint8_t)((v64 >> 40) & 0xff);
                bytes[i * 8 + 6] = (uint8_t)((v64 >> 48) & 0xff);
                bytes[i * 8 + 7] = (uint8_t)((v64 >> 56) & 0xff);
            }
        } else {
            return 0;
        }
        memcpy(&tmp, bytes, sizeof(bytes));
        out->type = WAH_TYPE_V128;
        out->value.v128 = tmp;
        return 1;
    }
    return 0;
}

static void wast_free_result_pattern(wast_result_pat_t *pat) {
    size_t i;
    if (!pat) {
        return;
    }
    free(pat->ref_kind_name);
    for (i = 0; i < pat->alternative_count; ++i) {
        wast_free_result_pattern(&pat->alternatives[i]);
    }
    free(pat->alternatives);
    memset(pat, 0, sizeof(*pat));
}

static int wast_parse_result_pattern(const wast_node_t *node, wast_result_pat_t *out) {
    size_t i;
    memset(out, 0, sizeof(*out));
    if (!node || node->is_atom || node->child_count == 0) {
        return 0;
    }
    if (wast_atom_eq(node->children[0], "either")) {
        out->kind = WAST_PAT_EITHER;
        out->alternative_count = node->child_count - 1;
        out->alternatives = (wast_result_pat_t *)calloc(out->alternative_count, sizeof(*out->alternatives));
        if (!out->alternatives) {
            return 0;
        }
        for (i = 0; i < out->alternative_count; ++i) {
            if (!wast_parse_result_pattern(node->children[i + 1], &out->alternatives[i])) {
                return 0;
            }
        }
        return 1;
    }
    if (wast_atom_eq(node->children[0], "i32.const") && node->child_count == 2) {
        out->kind = WAST_PAT_I32;
        return wast_parse_num_pattern(node->children[1], &out->scalar, -32);
    }
    if (wast_atom_eq(node->children[0], "i64.const") && node->child_count == 2) {
        out->kind = WAST_PAT_I64;
        return wast_parse_num_pattern(node->children[1], &out->scalar, -64);
    }
    if (wast_atom_eq(node->children[0], "f32.const") && node->child_count == 2) {
        out->kind = WAST_PAT_F32;
        return wast_parse_num_pattern(node->children[1], &out->scalar, 32);
    }
    if (wast_atom_eq(node->children[0], "f64.const") && node->child_count == 2) {
        out->kind = WAST_PAT_F64;
        return wast_parse_num_pattern(node->children[1], &out->scalar, 64);
    }
    if (wast_atom_eq(node->children[0], "v128.const") && node->child_count >= 3) {
        out->kind = WAST_PAT_V128;
        if (!wast_atom_to_cstr(node->children[1], out->shape, sizeof(out->shape))) {
            return 0;
        }
        if (strcmp(out->shape, "i8x16") == 0) out->lane_count = 16;
        else if (strcmp(out->shape, "i16x8") == 0) out->lane_count = 8;
        else if (strcmp(out->shape, "i32x4") == 0 || strcmp(out->shape, "f32x4") == 0) out->lane_count = 4;
        else if (strcmp(out->shape, "i64x2") == 0 || strcmp(out->shape, "f64x2") == 0) out->lane_count = 2;
        else return 0;
        if (node->child_count != out->lane_count + 2) {
            return 0;
        }
        for (i = 0; i < out->lane_count; ++i) {
            int float_bits = 0;
            if (out->shape[0] == 'f') {
                float_bits = (out->shape[1] == '3') ? 32 : 64;
            } else {
                float_bits = (out->shape[1] == '8') ? -32 : (out->shape[1] == '6' && out->shape[2] == '4') ? -64 : -32;
                if (strcmp(out->shape, "i16x8") == 0) float_bits = -32;
                if (strcmp(out->shape, "i32x4") == 0) float_bits = -32;
                if (strcmp(out->shape, "i64x2") == 0) float_bits = -64;
            }
            if (!wast_parse_num_pattern(node->children[i + 2], &out->lanes[i], float_bits)) {
                return 0;
            }
        }
        return 1;
    }
    if (wast_atom_eq(node->children[0], "ref") && node->child_count == 1) {
        out->kind = WAST_PAT_REF_ANY;
        return 1;
    }
    if (wast_atom_eq(node->children[0], "ref.null")) {
        out->kind = WAST_PAT_REF_NULL;
        return 1;
    }
    if (wast_atom_eq(node->children[0], "ref.func")) {
        out->kind = WAST_PAT_REF_FUNC;
        return 1;
    }
    if (wast_atom_eq(node->children[0], "ref.extern")) {
        out->kind = WAST_PAT_REF_EXTERN;
        return 1;
    }
    if (wast_atom_eq(node->children[0], "ref.host")) {
        uint64_t id;
        out->kind = WAST_PAT_REF_HOST;
        if (node->child_count == 2 && wast_parse_u64(node->children[1], &id)) {
            out->host_ref_id = (uint32_t)id;
        }
        return 1;
    }
    if (!node->children[0]->is_atom || node->children[0]->atom.len < 4 ||
        memcmp(node->children[0]->atom.data, "ref.", 4) != 0) {
        return 0;
    }
    out->kind = WAST_PAT_REF_OTHER;
    out->ref_kind_name = wast_atom_dup_cstr(node->children[0]);
    return out->ref_kind_name != NULL;
}

static char *wast_read_file(const char *path, size_t *out_size) {
    FILE *fp;
    long size;
    char *data;
    if (out_size) {
        *out_size = 0;
    }
    fp = fopen(path, "rb");
    if (!fp) {
        return NULL;
    }
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return NULL;
    }
    size = ftell(fp);
    if (size < 0) {
        fclose(fp);
        return NULL;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return NULL;
    }
    data = (char *)malloc((size_t)size + 1);
    if (!data) {
        fclose(fp);
        return NULL;
    }
    if (size > 0 && fread(data, 1, (size_t)size, fp) != (size_t)size) {
        free(data);
        fclose(fp);
        return NULL;
    }
    data[size] = '\0';
    fclose(fp);
    if (out_size) {
        *out_size = (size_t)size;
    }
    return data;
}
