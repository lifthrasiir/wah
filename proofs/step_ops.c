// Lists the harnesses of proofs/step.c with the opcodes they run, as `<harness> <opcode> <flags>*` lines, for
// `make cbmc-step`.
#define WAH_FORCE_SWITCH
#define WAH_FORCE_PORTABLE
#define WAH_CHECK_LOWERED // For wah_check_decode
#define WAH_IMPLEMENTATION
#include "../wah.h"
#include <stdio.h>

int main(void) {
#define WAH_STEP_OP(name, cls, val, feat) WAH_IF_##feat({ \
        uint16_t op = WAH_OP_##name; \
        uint8_t c = wah_opclasses[op]; \
        if (op < WAH_FE && c >= WAH_OPCLASS__I_I && c <= WAH_OPCLASS__VVV_V) printf("numeric %s\n", #name); \
    })
    WAH_OPCODES(WAH_STEP_OP)
#undef WAH_STEP_OP
    // Loads and stores, as wah_check_decode finds them, including the variants for memory 0 and 64-bit addresses
    static const char *names[WAH_NUM_OPCODES];
#define WAH_STEP_NAME(name, cls, val, feat) WAH_IF_##feat(names[WAH_OP_##name] = #name;)
#define WAH_STEP_EXTRA_NAME(name, suffix) names[WAH_OP_##name##_##suffix] = #name "_" #suffix;
    WAH_OPCODES(WAH_STEP_NAME)
    WAH_EXTRA_OPCODES(WAH_STEP_EXTRA_NAME)
#undef WAH_STEP_NAME
#undef WAH_STEP_EXTRA_NAME
    for (uint32_t op = 0; op < WAH_NUM_OPCODES; op++) {
        uint8_t buf[32] = {(uint8_t)op, (uint8_t)(op >> 8)};
        wah_check_instr_t in;
        if (names[op] && wah_check_decode(buf, sizeof(buf), 0, &in) == WAH_OK && in.mem != WAH_CHECK_MEM_NONE)
            printf("memory %s\n", names[op]);
    }
    static const char *const basic[] = { "I32_CONST", "I64_CONST", "F32_CONST", "F64_CONST", "V128_CONST", "DROP",
                                         "SELECT", "SELECT_T", "LOCAL_GET", "LOCAL_SET", "LOCAL_TEE", "GLOBAL_GET",
                                         "GLOBAL_SET", "GLOBAL_GET_INDIRECT", "GLOBAL_SET_INDIRECT", "REF_NULL",
                                         "REF_IS_NULL", "REF_AS_NON_NULL", "REF_EQ", "REF_I31", "I31_GET_S",
                                         "I31_GET_U", "REF_FUNC" };
    for (size_t i = 0; i < sizeof(basic) / sizeof(*basic); i++) printf("basic %s\n", basic[i]);
    // Branches for both ways and some values kept and dropped, which are constant (see harness_branch)
    for (int k = 0; k < 3; k++) {
        for (int d = 0; d < 3; d++) {
            for (int taken = 0; taken < 2; taken++) {
                const char *flags = "-DSTEP_TAKEN=%d -DSTEP_KEEP=%d -DSTEP_DROP=%d";
                printf("branch BR_IF "); printf(flags, taken, k, d); printf("\n");
                printf("branch BR_ON_NULL "); printf(flags, taken, k, d); printf("\n");
                if (k > 0) { printf("branch BR_ON_NON_NULL "); printf(flags, taken, k, d); printf("\n"); }
                if (taken) { printf("branch BR "); printf(flags, taken, k, d); printf("\n"); }
            }
        }
    }
    printf("branch IF -DSTEP_TAKEN=0\nbranch IF -DSTEP_TAKEN=1\nbranch ELSE -DSTEP_TAKEN=1\n");
    // Calls with room for the callee or not (one value below the arguments, 8 values in all), and returns
    for (int p = 0; p < 3; p++) {
        for (int r = 0; r < 3; r++) {
            for (int l = 0; l < 3; l++) {
                int room = 8 - 1 - p - l;
                printf("call CALL -DSTEP_PARAMS=%d -DSTEP_RESULTS=%d -DSTEP_LOCALS=%d -DSTEP_SLACK=%d\n", p, r, l, room);
                printf("call CALL -DSTEP_PARAMS=%d -DSTEP_RESULTS=%d -DSTEP_LOCALS=%d -DSTEP_SLACK=%d\n", p, r, l, room + 1);
            }
        }
    }
    for (int r = 0; r < 3; r++) {
        for (int d = 0; d < 3; d++) {
            printf("return RETURN -DSTEP_RESULTS=%d -DSTEP_DROP=%d\n", r, d);
            printf("return END -DSTEP_RESULTS=%d -DSTEP_DROP=%d\n", r, d);
        }
    }
    printf("table TABLE_GET\ntable TABLE_SET\ntable TABLE_SIZE\ntable TABLE_GET_i64\ntable TABLE_SET_i64\n"
           "table TABLE_SIZE_i64\n");
    printf("gc STRUCT_GET_S8\n");
    printf("gc STRUCT_GET_U8\n");
    printf("gc STRUCT_GET_S16\n");
    printf("gc STRUCT_GET_U16\n");
    printf("gc STRUCT_GET_32\n");
    printf("gc STRUCT_GET_64\n");
    printf("gc STRUCT_GET_128\n");
    printf("gc STRUCT_SET_8\n");
    printf("gc STRUCT_SET_16\n");
    printf("gc STRUCT_SET_32\n");
    printf("gc STRUCT_SET_64\n");
    printf("gc STRUCT_SET_128\n");
    printf("gc ARRAY_GET_S8\n");
    printf("gc ARRAY_GET_U8\n");
    printf("gc ARRAY_GET_S16\n");
    printf("gc ARRAY_GET_U16\n");
    printf("gc ARRAY_GET_32\n");
    printf("gc ARRAY_GET_64\n");
    printf("gc ARRAY_GET_128\n");
    printf("gc ARRAY_SET_8\n");
    printf("gc ARRAY_SET_16\n");
    printf("gc ARRAY_SET_32\n");
    printf("gc ARRAY_SET_64\n");
    printf("gc ARRAY_SET_128\n");
    printf("gc ARRAY_LEN\n");
    printf("internal POLL\ninternal METER -DSTEP_TAKEN=0\ninternal METER -DSTEP_TAKEN=1\n"
           "internal TICK -DSTEP_INDEX=0\ninternal TICK -DSTEP_INDEX=1\ninternal TICK -DSTEP_INDEX=2\n");
    // Casts of null and i31 references against heap types that i31s have or not
    static const char *heaps[] = {"WAH_TYPE_ANY", "WAH_TYPE_I31", "WAH_TYPE_STRUCT"};
    static const char *casts[] = {"REF_TEST", "REF_TEST_NULL", "REF_CAST", "REF_CAST_NULL"};
    for (int r = 0; r < 2; r++) {
        for (int h = 0; h < 3; h++) {
            for (int c = 0; c < 4; c++) printf("cast %s -DSTEP_REF=%d -DSTEP_HEAP=%s\n", casts[c], r, heaps[h]);
            for (int f = 0; f < 2; f++) {
                printf("cast BR_ON_CAST -DSTEP_REF=%d -DSTEP_HEAP=%s -DSTEP_TAKEN=%d -DSTEP_DROP=1\n", r, heaps[h], f);
                printf("cast BR_ON_CAST_FAIL -DSTEP_REF=%d -DSTEP_HEAP=%s -DSTEP_TAKEN=%d -DSTEP_DROP=1\n", r, heaps[h], f);
            }
        }
    }
    // Fills in and out of 4 items, to the end or suspended by fuel with one item affordable
    static const char *fills[] = {"TABLE_FILL", "TABLE_FILL_i64", "MEMORY_FILL", "MEMORY_FILL_i64"};
    static const int ranges[][2] = {{0, 0}, {1, 2}, {0, 4}, {4, 0}, {3, 2}, {5, 0}};
    for (int f = 0; f < 4; f++) {
        for (int r = 0; r < 6; r++) {
            printf("fill %s -DSTEP_DST=%d -DSTEP_COUNT=%d -DSTEP_TAKEN=0\n", fills[f], ranges[r][0], ranges[r][1]);
            printf("fill %s -DSTEP_DST=%d -DSTEP_COUNT=%d -DSTEP_TAKEN=1 -DWAH_BULK_ITEMS_PER_FUEL=1\n", fills[f],
                   ranges[r][0], ranges[r][1]);
        }
    }
    // Copies within 4 items, overlapping forward and backward or out of bounds, to the end or suspended
    static const char *copies[] = {"TABLE_COPY", "TABLE_COPY_i32_to_i64", "TABLE_COPY_i64_to_i32",
                                   "TABLE_COPY_i64_to_i64", "MEMORY_COPY", "MEMORY_COPY_i32_to_i64",
                                   "MEMORY_COPY_i64_to_i32", "MEMORY_COPY_i64_to_i64"};
    static const int cranges[][3] = {{0, 1, 3}, {1, 0, 3}, {0, 0, 4}, {2, 0, 2}, {3, 0, 2}, {0, 3, 2}};
    for (int c = 0; c < 8; c++) {
        for (int r = 0; r < 6; r++) {
            printf("copy %s -DSTEP_DST=%d -DSTEP_SRC=%d -DSTEP_COUNT=%d -DSTEP_TAKEN=0\n", copies[c],
                   cranges[r][0], cranges[r][1], cranges[r][2]);
            printf("copy %s -DSTEP_DST=%d -DSTEP_SRC=%d -DSTEP_COUNT=%d -DSTEP_TAKEN=1 -DWAH_BULK_ITEMS_PER_FUEL=1\n",
                   copies[c], cranges[r][0], cranges[r][1], cranges[r][2]);
        }
    }
    // br_table for each target and beyond, with some values kept and dropped
    for (int k = 0; k < 3; k++) {
        for (int d = 0; d < 3; d++) {
            for (int i = 0; i < 4; i++)
                printf("br_table BR_TABLE -DSTEP_KEEP=%d -DSTEP_DROP=%d -DSTEP_INDEX=%d\n", k, d, i == 3 ? 7 : i);
        }
    }
    return 0;
}
