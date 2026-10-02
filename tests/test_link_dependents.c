// Regression tests for the dependent lists of linked contexts, which are walked on every memory.grow or
// table.grow. We need WAH_IMPLEMENTATION to inspect them.

#define WAH_IMPLEMENTATION
#include "../wah.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>

// Importing the same memory or table many times used to register the importer once per import, so that each
// grow updated all its imports once per registration.
static void test_repeated_imports(void) {
    printf("Testing repeated imports register the importer once...\n");
    enum { N = 100 };
    wah_module_t pmod = {0}, cmod = {0};
    assert_ok(wah_parse_module_from_spec(&pmod, "wasm types {[ fn [] [i32] ]} funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 1 ]} memories {[ limits.i32/1 1 ]} \
        exports {[ {'m'} mem# 0, {'t'} table# 0, {'grow'} fn# 0 ]} \
        code {[ {[] i32.const 1 memory.grow 0 drop ref.null funcref i32.const 1 table.grow 0 end} ]}"));
    char *spec = malloc(64 + N * 2 * 48), *q = spec;
    q += sprintf(q, "wasm types {[ fn [] [i32] ]} imports {[ ");
    for (int i = 0; i < N; i++) {
        q += sprintf(q, "%s{'p'} {'m'} mem# limits.i32/1 1, {'p'} {'t'} table# funcref limits.i32/1 1", i ? ", " : "");
    }
    q += sprintf(q, " ]} funcs {[ 0 ]} exports {[ {'size'} fn# 0 ]} \
        code {[ {[] memory.size %d table.size %d i32.add end} ]}", N - 1, N - 1);
    assert_ok(wah_parse_module_from_spec(&cmod, spec));
    free(spec);

    wah_exec_context_t p = {0}, c = {0};
    assert_ok(wah_new_exec_context(&p, &pmod, NULL));
    assert_ok(wah_instantiate(&p));
    assert_ok(wah_new_exec_context(&c, &cmod, NULL));
    assert_ok(wah_link_context(&c, "p", &p));
    assert_ok(wah_instantiate(&c));
    assert_eq_u32(p.dependent_count, 1);
    assert_eq_u32(p.gc->gc_dependent_count, 1);

    // All imports still follow the grow
    wah_value_t r;
    assert_ok(wah_call_by_name(&p, "grow", NULL, 0, &r));
    assert_ok(wah_call_by_name(&c, "size", NULL, 0, &r));
    assert_eq_i32(r.i32, 4);

    wah_free_exec_context(&c);
    assert_eq_u32(p.dependent_count, 0);
    assert_eq_u32(p.gc->gc_dependent_count, 0);
    wah_free_exec_context(&p);
    wah_free_module(&cmod);
    wah_free_module(&pmod);
}

int main(void) {
    test_repeated_imports();
    printf("All link dependent tests passed!\n");
    return 0;
}
