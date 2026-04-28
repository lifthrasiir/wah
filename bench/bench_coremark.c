#define WAH_IMPLEMENTATION
#include "../wah.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
static uint64_t monotonic_ms(void) {
    LARGE_INTEGER freq, now;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&now);
    return (uint64_t)(now.QuadPart * 1000 / freq.QuadPart);
}
#else
#include <time.h>
static uint64_t monotonic_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}
#endif

static void clock_ms_host(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    wah_return_i32(ctx, (int32_t)monotonic_ms());
}

static uint8_t *read_file(const char *path, size_t *out_size) {
    FILE *f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "Cannot open %s\n", path); return NULL; }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *buf = (uint8_t *)malloc((size_t)len);
    if (!buf) { fclose(f); return NULL; }
    fread(buf, 1, (size_t)len, f);
    fclose(f);
    *out_size = (size_t)len;
    return buf;
}

int main(void) {
    size_t wasm_size = 0;
    uint8_t *wasm = read_file("bench/coremark-minimal.wasm", &wasm_size);
    if (!wasm) return 1;

    wah_module_t mod = {0};
    wah_error_t err = wah_parse_module(wasm, wasm_size, &mod);
    if (err) { fprintf(stderr, "Parse error: %s\n", wah_strerror(err)); free(wasm); return 1; }

    wah_module_t env = {0};
    wah_new_module(&env);
    wah_module_export_func(&env, "clock_ms", "() -> i32", clock_ms_host, NULL, NULL);

    wah_exec_context_t ctx = {0};
    wah_exec_context_create(&ctx, &mod);
    wah_link_module(&ctx, "env", &env);

    err = wah_instantiate(&ctx);
    if (err) { fprintf(stderr, "Instantiate error: %s\n", wah_strerror(err)); goto cleanup; }

    wah_export_desc_t run_entry;
    err = wah_module_export_by_name(&mod, "run", &run_entry);
    if (err) { fprintf(stderr, "Cannot find 'run' export: %s\n", wah_strerror(err)); goto cleanup; }

    printf("Running CoreMark...\n");
    wah_value_t result;
    err = wah_call(&ctx, run_entry.index, NULL, 0, &result);
    if (err) { fprintf(stderr, "Call error: %s\n", wah_strerror(err)); goto cleanup; }

    printf("CoreMark result: %.2f\n", result.f32);

cleanup:
    wah_exec_context_destroy(&ctx);
    wah_free_module(&env);
    wah_free_module(&mod);
    free(wasm);
    return err ? 1 : 0;
}
