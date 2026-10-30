// CBMC proof of the pin table: after any sequence of pins and unpins (of any references, including stale and forged
// ones), exactly the references pinned and not yet unpinned resolve, to their pinned values, and only they unpin.
// Handles are never reused, so stale ones stay rejected, and freed slots are reused before the table grows.
// Generations wrap around only after 2^30 reuses of a slot (2^10 on 32-bit targets), which is beyond the bound.
#include "proofs.h"

bool nondet_bool(void);
uintptr_t nondet_uintptr(void);
uint8_t nondet_u8(void);

#define MAX_OPS 6

// The table is preallocated, which keeps symbolic reallocations out of the proof
static wah_exec_context_t ctx;
static wah_pin_slot_t slots[MAX_OPS];
static wah_pin_table_t pins = { slots, 0, MAX_OPS, 0 };
static void *handles[MAX_OPS], *values[MAX_OPS];
static bool live[MAX_OPS];
static int handle_count, live_count, peak_live_count;

// Any reference: a handle given before (live or stale), or anything else
static void *nondet_ref(void) {
    uint8_t i = nondet_u8();
    if (i < handle_count) return handles[i];
    return (void *)nondet_uintptr();
}

static int find_live(const void *ref) {
    int found = -1;
    for (int i = 0; i < MAX_OPS; i++) {
        if (i < handle_count && live[i] && handles[i] == ref) found = i;
    }
    return found;
}

static void check_resolve(void) {
    void *ref = nondet_ref();
    const wah_pin_slot_t *s = wah_pinned_slot(&ctx, ref);
    int i = find_live(ref);
    __CPROVER_assert((s != NULL) == (i >= 0), "resolves exactly live references");
    if (s) __CPROVER_assert(s->value.ref == values[i], "resolves to the pinned value");
}

void harness_ops(void) {
    ctx.pins = &pins;
    for (int op = 0; op < MAX_OPS; op++) {
        if (nondet_bool()) {
            void *value = (void *)nondet_uintptr(), *handle = NULL;
            __CPROVER_assume(value);
            if (wah_pin(&ctx, (wah_value_t){ .ref = value }, NULL, WAH_TYPE_ANY, &handle) != WAH_OK) continue;
            __CPROVER_assert(((uintptr_t)handle & 3) == WAH_PIN_TAG, "makes a tagged handle");
            for (int i = 0; i < MAX_OPS; i++) {
                if (i < handle_count) __CPROVER_assert(handles[i] != handle, "makes a handle never given before");
            }
            if (++live_count > peak_live_count) peak_live_count = live_count;
            __CPROVER_assert(pins.slot_count == peak_live_count, "reuses freed slots");
            handles[handle_count] = handle, values[handle_count] = value, live[handle_count] = true;
            handle_count++;
        } else {
            void *ref = nondet_ref();
            int i = find_live(ref);
            bool ok = wah_unpin(&ctx, ref);
            __CPROVER_assert(ok == (i >= 0 || !ref), "unpins exactly live references");
            if (i >= 0) live[i] = false, live_count--;
        }
        check_resolve();
    }
}
