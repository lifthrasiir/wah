// Lists the harnesses of proofs/step.c with the opcodes they run, as `<harness> <opcode> <flags>*` lines, for
// `make cbmc-step`.
#define WAH_FORCE_SWITCH
#define WAH_FORCE_PORTABLE
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
    static const char *const basic[] = { "I32_CONST", "I64_CONST", "F32_CONST", "F64_CONST", "V128_CONST", "DROP",
                                         "SELECT", "SELECT_T", "LOCAL_GET", "LOCAL_SET", "LOCAL_TEE" };
    for (size_t i = 0; i < sizeof(basic) / sizeof(*basic); i++) printf("basic %s\n", basic[i]);
    // Branches for both ways and some values kept and dropped, which are constant (see harness_branch)
    for (int k = 0; k < 3; k++) {
        for (int d = 0; d < 3; d++) {
            for (int taken = 0; taken < 2; taken++) {
                const char *flags = "-DSTEP_TAKEN=%d -DSTEP_KEEP=%d -DSTEP_DROP=%d";
                printf("branch BR_IF "); printf(flags, taken, k, d); printf("\n");
                if (taken) { printf("branch BR "); printf(flags, taken, k, d); printf("\n"); }
            }
        }
    }
    printf("branch IF -DSTEP_TAKEN=0\nbranch IF -DSTEP_TAKEN=1\nbranch ELSE -DSTEP_TAKEN=1\n");
    return 0;
}
