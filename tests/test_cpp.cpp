#include "../wah.h"

#include <cstdlib>
#include <new>
#include <utility>

static void fail(const char *expr, wah_error_t err, int line) {
    (void)expr;
    (void)err;
    (void)line;
    std::abort();
}

#define CHECK_OK(expr) do { \
    wah_error_t err_ = (expr); \
    if (err_ != WAH_OK) fail(#expr, err_, __LINE__); \
} while (0)

#define CHECK_TRUE(expr) do { \
    if (!(expr)) std::abort(); \
} while (0)

template <typename F>
static void trampoline(wah_call_context_t *ctx, void *userdata) {
    (*static_cast<F *>(userdata))(ctx);
}

template <typename F>
static void finalize(void *userdata) {
    delete static_cast<F *>(userdata);
}

template <typename F>
static wah_error_t export_cpp_func(wah_module_t *mod, const char *name, const char *types, F fn) {
    F *boxed = new (std::nothrow) F(std::move(fn));
    if (!boxed) return WAH_ERROR_OUT_OF_MEMORY;

    wah_error_t err = wah_export_func(mod, name, types, trampoline<F>, boxed, finalize<F>);
    if (err != WAH_OK) delete boxed;
    return err;
}

int main() {
    wah_module_t mod = {};
    CHECK_OK(wah_new_module(&mod, NULL));

    int captured = 41;
    CHECK_OK(export_cpp_func(&mod, "add_captured", "(i32) -> i32",
        [captured](wah_call_context_t *ctx) {
            int32_t value = wah_param_i32(ctx, 0);
            wah_return_i32(ctx, value + captured);
        }));

    wah_exec_context_t ctx = {};
    CHECK_OK(wah_new_exec_context(&ctx, &mod, NULL));

    wah_value_t param = {};
    param.i32 = 1;
    wah_value_t result = {};
    CHECK_OK(wah_call_by_name(&ctx, "add_captured", &param, 1, &result));
    CHECK_TRUE(result.i32 == 42);

    wah_free_exec_context(&ctx);
    wah_free_module(&mod);
    return 0;
}
