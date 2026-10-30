// Common header of CBMC proof harnesses (`make cbmc`). Each harness_* function is checked for all inputs, which come
// from the nondet_* functions without bodies, within the unwinding bound given in the Makefile.
#ifndef WAH_PROOFS_H
#define WAH_PROOFS_H

// Library functions are plain functions that harnesses can replace
#undef _FORTIFY_SOURCE
#define _FORTIFY_SOURCE 0

// Neither computed gotos, musttail nor inline assembly, which CBMC doesn't model
#define WAH_FORCE_SWITCH
#define WAH_FORCE_PORTABLE
#define WAH_IMPLEMENTATION
#include "../wah.h"

#ifndef __CPROVER__ // Built into CBMC, declared for other tools like editors
void __CPROVER_assume(bool cond);
void __CPROVER_assert(bool cond, const char *desc);
bool __CPROVER_same_object(const void *p, const void *q);
bool __CPROVER_w_ok(const void *p, size_t size);
#endif

uint8_t nondet_u8(void);
size_t nondet_size(void);

#endif // WAH_PROOFS_H
