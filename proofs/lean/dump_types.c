// Dumps type sections and canonical maps of the modules in .wast files (or .wasm files) for WahTypesOracle, which
// checks canonical maps against the model in WahProofs/Types.lean (`make lean-diff`). Each module is printed as
//   M <type count>
//   T <start> <size> <kind> <final> <supertype or -1> <count1> <count2> <field count> (<wah_type_t> <mutable>)*
//   C <canonical type>*
// where functions have their params and results as immutable fields and counts, as `TD` in Lean.
// With DUMP_COLLIDE, all rec groups have the same hash, so that every pair of them is compared structurally.
#include <inttypes.h>
#include <stdint.h>
#ifdef DUMP_COLLIDE
static uint64_t dump_collide_mix(uint64_t h, uint64_t v) { (void)h, (void)v; return 0; }
#define WAH_REC_GROUP_HASH_MIX dump_collide_mix
#endif
#define WAH_IMPLEMENTATION
#include "../../wah.h"
#include "../../tests/wast.c"

static void dump_module(const wah_module_t *m) {
    if (m->type_count == 0) return;
    printf("M %" PRIu32 "\n", m->type_count);
    for (uint32_t i = 0; i < m->type_count; ++i) {
        const wah_type_def_t *d = &m->type_defs[i];
        long long super = d->supertype == WAH_NO_SUPERTYPE ? -1 : (long long)d->supertype;
        printf("T %" PRIu32 " %" PRIu32 " %d %d %lld", d->rec_group_start, d->rec_group_size, (int)d->kind,
               (int)d->is_final, super);
        if (d->kind == WAH_COMP_FUNC) {
            const wah_func_type_t *ft = &m->types[i];
            printf(" %" PRIu32 " %" PRIu32 " %" PRIu32, ft->param_count, ft->result_count,
                   ft->param_count + ft->result_count);
            for (uint32_t j = 0; j < ft->param_count; ++j) printf(" %" PRId32 " 0", ft->param_types[j]);
            for (uint32_t j = 0; j < ft->result_count; ++j) printf(" %" PRId32 " 0", ft->result_types[j]);
        } else {
            printf(" %" PRIu32 " 0 %" PRIu32, d->field_count, d->field_count);
            for (uint32_t j = 0; j < d->field_count; ++j)
                printf(" %" PRId32 " %d", d->field_types[j], (int)d->field_mutables[j]);
        }
        printf("\n");
    }
    printf("C");
    for (uint32_t i = 0; i < m->type_count; ++i) printf(" %" PRIu32, m->canonical_map ? m->canonical_map[i] : i);
    printf("\n");
}

static void dump_binary(const uint8_t *bytes, size_t len) {
    static const uint8_t empty = 0;
    wah_module_t m = {0};
    if (wah_parse_module(&m, len ? bytes : &empty, len, NULL) == WAH_OK) {
        dump_module(&m);
        wah_free_module(&m);
    }
}

int main(int argc, char **argv) {
    for (int a = 1; a < argc; ++a) {
        size_t size;
        char *data = wast_read_file(argv[a], &size);
        if (!data) { fprintf(stderr, "cannot read %s\n", argv[a]); return 1; }
        size_t n = strlen(argv[a]);
        if (n < 5 || strcmp(argv[a] + n - 5, ".wast") != 0) {
            dump_binary((const uint8_t *)data, size);
            free(data);
            continue;
        }
        wast_parser_t parser;
        wast_parser_init(&parser, data, size);
        wast_node_t *node;
        while ((node = wast_parse_next(&parser)) != NULL) {
            uint8_t *bytes = NULL;
            size_t len = 0;
            if (!node->is_atom && node->child_count >= 2 && wast_atom_eq(node->children[0], "module") &&
                wast_gather_binary_module(node, &bytes, &len)) {
                dump_binary(bytes, len);
                free(bytes);
            }
            wast_node_free(node);
        }
        free(data);
    }
    return 0;
}
