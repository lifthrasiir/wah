// Tests for guard pages of wahjit.h. They are enabled for the rest of the process, so this is a separate test.
// test_jit and test_spectest (with JIT=1) can also be run with guard pages by setting WAH_TEST_GUARD_PAGES.
#define WAH_IMPLEMENTATION
#include "../wahjit.h"
#include "common.h"

#ifdef WAH_JIT

#ifndef _WIN32
#include <setjmp.h>
#include <signal.h>
#include <sys/mman.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#define SIGSEGV_OR_ZERO 0
#else
#define SIGSEGV_OR_ZERO SIGSEGV
#endif

// Functions: 0 load32, 1 store32, 2 store16, 3 load16_u, 4 load64 with the largest guarded offset,
// 5 load64 with a checked offset, 6 memory.grow
static const char *MAIN_SPEC = "wasm types {[ fn [i32] [i32], fn [i32, i32] [], fn [i32] [i64] ]} \
    funcs {[ 0, 1, 1, 0, 2, 2, 0 ]} memories {[ limits.i32/2 1 2 ]} exports {[ {'mem'} mem# 0 ]} code {[ \
    {[] local.get 0 i32.load 2 0 end}, \
    {[] local.get 0 local.get 1 i32.store 2 0 end}, \
    {[] local.get 0 local.get 1 i32.store16 1 0 end}, \
    {[] local.get 0 i32.load16_u 1 0 end}, \
    {[] local.get 0 i64.load 3 0x7ffffff8 end}, \
    {[] local.get 0 i64.load 3 0x80000000 end}, \
    {[] local.get 0 memory.grow 0 end} ]}";

static const char *IMPORTER_SPEC = "wasm types {[ fn [i32] [i32] ]} \
    imports {[ {'env'} {'mem'} export.memory limits.i32/1 1 ]} funcs {[ 0 ]} code {[ {[] local.get 0 i32.load 2 0 end} ]}";

// Calls the host function env.inner, then loads from its own memory
static const char *OUTER_SPEC = "wasm types {[ fn [i32] [i32] ]} imports {[ {'env'} {'inner'} fn# 0 ]} funcs {[ 0 ]} \
    memories {[ limits.i32/1 1 ]} code {[ {[] local.get 0 call 0 drop local.get 0 i32.load 2 0 end} ]}";

typedef struct {
    wah_module_t module;
    wah_exec_context_t ctx;
} inst_t;

static void inst_new(inst_t *inst, const char *spec, const char *link_name, wah_module_t *link_mod,
                     wah_exec_context_t *link_ctx) {
    assert_ok(wah_parse_module_from_spec(&inst->module, spec));
    assert_ok(wah_new_exec_context(&inst->ctx, &inst->module, NULL));
    if (link_mod) assert_ok(wah_link_module(&inst->ctx, link_name, link_mod));
    if (link_ctx) assert_ok(wah_link_context(&inst->ctx, link_name, link_ctx));
    assert_ok(wah_instantiate(&inst->ctx));
}

static void inst_free(inst_t *inst) {
    wah_free_exec_context(&inst->ctx);
    wah_free_module(&inst->module);
}

static bool guarded_natively(const inst_t *inst) {
    return inst->module.jit && inst->module.jit->guarded && inst->ctx.jit;
}

static wah_error_t call1(inst_t *inst, uint32_t func_idx, int32_t arg, wah_value_t *result) {
    wah_value_t p = { .i32 = arg };
    return wah_call(&inst->ctx, func_idx, &p, 1, result);
}

static wah_error_t call2(inst_t *inst, uint32_t func_idx, int32_t a, int32_t b) {
    wah_value_t p[2] = { { .i32 = a }, { .i32 = b } };
    return wah_call(&inst->ctx, func_idx, p, 2, NULL);
}

// Faults that reached the handler of the host and were handled by wah_handle_fault
static volatile int host_faults;

#ifdef _WIN32
static LONG CALLBACK host_handler(EXCEPTION_POINTERS *ep) {
    if (!wah_handle_fault(0, ep, NULL)) return EXCEPTION_CONTINUE_SEARCH;
    host_faults++;
    return EXCEPTION_CONTINUE_EXECUTION;
}

static void install_host_handler(void) { assert_not_null(AddVectoredExceptionHandler(1, host_handler)); }
#else
static sigjmp_buf *volatile expected_fault; // Where other faults jump to

static void host_handler(int signo, siginfo_t *info, void *uctx) {
    if (wah_handle_fault(signo, info, uctx)) {
        host_faults++;
        return;
    }
    if (expected_fault) siglongjmp(*expected_fault, 1);
    signal(signo, SIG_DFL); // Crashes when retried
}

static void install_host_handler(void) {
    struct sigaction sa = {0};
    sa.sa_sigaction = host_handler;
    sa.sa_flags = SA_SIGINFO | SA_NODEFER;
    sigemptyset(&sa.sa_mask);
    assert_true(sigaction(SIGSEGV, &sa, NULL) == 0);
    assert_true(sigaction(SIGBUS, &sa, NULL) == 0);
}
#endif

static void expect_oob(inst_t *inst, uint32_t func_idx, int32_t arg, int faults) {
    int before = host_faults;
    wah_value_t r;
    assert_err(call1(inst, func_idx, arg, &r), WAH_ERROR_MEMORY_OUT_OF_BOUNDS);
    assert_eq_i32(host_faults - before, faults);
}

// Accesses of MAIN_SPEC, where `faults` is how many faults reach the host per guarded out-of-bounds access
static void check_main(inst_t *m, int faults) {
    wah_value_t r;
    assert_ok(call2(m, 1, 65532, 0x11223344));
    assert_ok(call1(m, 0, 65532, &r));
    assert_eq_u32((uint32_t)r.i32, 0x11223344);
    expect_oob(m, 0, 65533, faults);
    expect_oob(m, 0, 65536, faults);
    expect_oob(m, 0, -1, faults);

    // A store straddling the end writes nothing
    assert_ok(call2(m, 2, 65534, 0xabcd));
    int before = host_faults;
    assert_err(call2(m, 1, 65534, -1), WAH_ERROR_MEMORY_OUT_OF_BOUNDS);
    assert_eq_i32(host_faults - before, faults);
    assert_ok(call1(m, 3, 65534, &r));
    assert_eq_u32((uint32_t)r.i32, 0xabcd);

    // The far end of guard pages, then an offset beyond them which is checked
    expect_oob(m, 4, -1, faults);
    expect_oob(m, 4, 0, faults);
    expect_oob(m, 5, 0, 0);

    // Grown pages are accessible and zeroed, up to the maximum
    assert_ok(call1(m, 6, 1, &r));
    assert_eq_i32(r.i32, 1);
    assert_ok(call1(m, 0, 65536, &r));
    assert_eq_i32(r.i32, 0);
    assert_ok(call2(m, 1, 131068, 42));
    assert_ok(call1(m, 0, 131068, &r));
    assert_eq_i32(r.i32, 42);
    expect_oob(m, 0, 131069, faults);
    assert_ok(call1(m, 6, 1, &r));
    assert_eq_i32(r.i32, -1);
}

static void inner_call(wah_call_context_t *call, void *userdata) {
    wah_value_t p = { .i32 = wah_param_i32(call, 0) }, r;
    wah_error_t err = wah_call((wah_exec_context_t *)userdata, 0, &p, 1, &r);
    wah_result_i32(call, 0, err);
}

static void test_before_enabling(inst_t *early) {
    printf("Testing before enabling guard pages...\n");
    assert_err(wah_enable_guard_pages((wah_fault_handling_t)0), WAH_ERROR_MISUSE);
    assert_err(wah_enable_guard_pages((wah_fault_handling_t)1), WAH_ERROR_MISUSE);
    assert_err(wah_enable_guard_pages((wah_fault_handling_t)4), WAH_ERROR_MISUSE);
    assert_false(wah_handle_fault(SIGSEGV_OR_ZERO, NULL, NULL));
    inst_new(early, MAIN_SPEC, NULL, NULL, NULL);
    assert_false(early->ctx.memories[0].guarded);
    assert_false(guarded_natively(early));
}

static void test_host_handles_fault(inst_t *early) {
    printf("Testing WAH_HOST_HANDLES_FAULT...\n");
    install_host_handler();
    assert_ok(wah_enable_guard_pages(WAH_HOST_HANDLES_FAULT));
    assert_ok(wah_enable_guard_pages(WAH_HOST_HANDLES_FAULT));
    assert_eq_i32(wah_jit_guard_load(), WAH_HOST_HANDLES_FAULT);

    inst_t m;
    inst_new(&m, MAIN_SPEC, NULL, NULL, NULL);
    assert_true(m.ctx.memories[0].guarded);
    assert_true(guarded_natively(&m));
    check_main(&m, 1);

    // Objects made before keep working with explicit checks
    check_main(early, 0);
    inst_t late;
    inst_new(&late, IMPORTER_SPEC, "env", NULL, &early->ctx);
    assert_false(guarded_natively(&late));
    expect_oob(&late, 0, 131069, 0);

    // A guarded memory exported to another context, and one from a host module
    inst_t imp;
    inst_new(&imp, IMPORTER_SPEC, "env", NULL, &m.ctx);
    assert_true(guarded_natively(&imp));
    wah_value_t r;
    assert_ok(call1(&imp, 0, 131068, &r));
    assert_eq_i32(r.i32, 42);
    expect_oob(&imp, 0, 131069, 1);
    wah_module_t env;
    assert_ok(wah_new_module(&env, NULL));
    assert_ok(wah_export_memory(&env, "mem", 1, 1));
    inst_t himp;
    inst_new(&himp, IMPORTER_SPEC, "env", &env, NULL);
    assert_true(guarded_natively(&himp));
    expect_oob(&himp, 0, 65533, 1);
    inst_free(&himp);
    wah_free_module(&env);

    // A nested activation restores the outer one, whose faults are still handled
    wah_module_t host;
    assert_ok(wah_new_module(&host, NULL));
    assert_ok(wah_export_func(&host, "inner", "(i32) -> i32", inner_call, &m.ctx, NULL));
    inst_t outer;
    inst_new(&outer, OUTER_SPEC, "env", &host, NULL);
    assert_true(guarded_natively(&outer));
    expect_oob(&outer, 1, 140000, 2);
    inst_free(&outer);
    wah_free_module(&host);

    inst_free(&imp);
    inst_free(&late);
    inst_free(&m);
}

static void test_wah_handles_fault(void) {
    printf("Testing WAH_HANDLES_FAULT...\n");
    assert_ok(wah_enable_guard_pages(WAH_HANDLES_FAULT));
    assert_ok(wah_enable_guard_pages(WAH_HOST_HANDLES_FAULT)); // Can't be downgraded
    assert_ok(wah_enable_guard_pages(WAH_HANDLES_FAULT));
    assert_eq_i32(wah_jit_guard_load(), WAH_HANDLES_FAULT);

    inst_t m;
    inst_new(&m, MAIN_SPEC, NULL, NULL, NULL);
    assert_true(guarded_natively(&m));
    check_main(&m, 0); // Handled before reaching the host
    inst_free(&m);

#ifndef _WIN32
    // Other faults are passed to the previous handler
    long page = sysconf(_SC_PAGESIZE);
    void *p = mmap(NULL, (size_t)page, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    assert_true(p != MAP_FAILED);
    sigjmp_buf jb;
    volatile bool faulted = false;
    if (sigsetjmp(jb, 1) == 0) {
        expected_fault = &jb;
        *(volatile int *)p = 1;
    } else {
        faulted = true;
    }
    expected_fault = NULL;
    assert_true(faulted);
    munmap(p, (size_t)page);
#endif
}

int main(void) {
    inst_t early;
    test_before_enabling(&early);
    test_host_handles_fault(&early);
    test_wah_handles_fault();
    inst_free(&early);
    printf("All guard page tests passed.\n");
    return 0;
}

#else // !WAH_JIT

int main(void) {
    printf("Testing guard pages without the JIT...\n");
    assert_err(wah_enable_guard_pages((wah_fault_handling_t)1), WAH_ERROR_MISUSE);
    assert_err(wah_enable_guard_pages(WAH_HANDLES_FAULT), WAH_ERROR_UNSUPPORTED);
    assert_err(wah_enable_guard_pages(WAH_HOST_HANDLES_FAULT), WAH_ERROR_UNSUPPORTED);
    assert_false(wah_handle_fault(0, NULL, NULL));
    printf("All guard page tests passed.\n");
    return 0;
}

#endif
