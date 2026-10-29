// Common header of CBMC proof harnesses (`make cbmc`). Each harness_* function is checked for all inputs, which come
// from the nondet_* functions without bodies, within the unwinding bound given in the Makefile.
#ifndef WAH_PROOFS_H
#define WAH_PROOFS_H

// Neither computed gotos, musttail nor inline assembly, which CBMC doesn't model
#define WAH_FORCE_SWITCH
#define WAH_FORCE_PORTABLE
#define WAH_IMPLEMENTATION
#include "../wah.h"

uint8_t nondet_u8(void);
size_t nondet_size(void);

#endif // WAH_PROOFS_H
