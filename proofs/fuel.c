// CBMC proofs of resource accounting: bulk operations make progress with any fuel left, take as many items as they
// can afford, and leave fuel at -1 or above, and charges within the memory budget never exceed it.
#include "proofs.h"

bool nondet_bool(void);
int64_t nondet_i64(void);
uint64_t nondet_u64(void);
uint32_t nondet_u32(void);

static wah_module_t module;
static wah_exec_context_t ctx;

static uint64_t fuel_units(uint64_t items) { // Units charged for items, without overflowing
    return items / WAH_BULK_ITEMS_PER_FUEL + (items % WAH_BULK_ITEMS_PER_FUEL != 0);
}

static void setup(void) {
    module.fuel_metering = nondet_bool();
    ctx.module = &module;
    ctx.fuel = nondet_i64();
}

void harness_bulk_limit(void) {
    setup();
    int64_t fuel = ctx.fuel;
    uint64_t chunk = nondet_u64(), limit = wah_bulk_fuel_limit(&ctx, chunk);
    __CPROVER_assert(limit <= chunk, "never more than asked");
    if (!module.fuel_metering) {
        __CPROVER_assert(limit == chunk, "unlimited without metering");
        wah_bulk_fuel_charge(&ctx, limit);
        __CPROVER_assert(ctx.fuel == fuel, "free without metering");
        return;
    }
    if (fuel < 0) {
        __CPROVER_assert(limit == 0, "nothing without fuel");
        return;
    }
    __CPROVER_assert(chunk == 0 || limit > 0, "progress with any fuel left");
    __CPROVER_assert(limit == chunk || fuel_units(limit + 1) > (uint64_t)fuel + 1, "as many as affordable");
    wah_bulk_fuel_charge(&ctx, limit);
    __CPROVER_assert(ctx.fuel == fuel - (int64_t)fuel_units(limit), "charged by items");
    __CPROVER_assert(ctx.fuel >= -1, "fuel stays at -1 or above");
}

void harness_budget(void) {
    ctx.max_memory_bytes = nondet_u64();
    ctx.memory_bytes_committed = nondet_u64();
    uint64_t committed = ctx.memory_bytes_committed, bytes = nondet_u64();
    bool ok = wah_budget_check(&ctx, bytes);
    if (ctx.max_memory_bytes == UINT64_MAX) {
        __CPROVER_assert(ok, "unlimited");
        return;
    }
    __CPROVER_assert(ok == ((unsigned __int128)committed + bytes <= ctx.max_memory_bytes), "within the budget");
    if (!ok) return;
    wah_budget_charge(&ctx, bytes);
    __CPROVER_assert(ctx.memory_bytes_committed >= committed && ctx.memory_bytes_committed <= ctx.max_memory_bytes,
                     "charges stay within the budget");
    wah_budget_release(&ctx, bytes);
    __CPROVER_assert(ctx.memory_bytes_committed == committed, "releases what was charged");
}
