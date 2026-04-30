#define WAH_IMPLEMENTATION
#include "../wah.h"
#include <assert.h>
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

#ifdef WAH_BENCH_KPC_CLOCK
#if !defined(__APPLE__)
#error "WAH_BENCH_KPC_CLOCK is only supported on macOS"
#endif
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define KPC_CLASS_FIXED 0
#define KPC_CLASS_FIXED_MASK (1u << KPC_CLASS_FIXED)

#ifndef WAH_BENCH_KPC_CALIBRATION_FILE
#define WAH_BENCH_KPC_CALIBRATION_FILE "bench/bench_coremark.kpc-rate"
#endif

extern int kpc_force_all_ctrs_set(int val);
extern int kpc_set_counting(uint32_t classes);
extern int kpc_set_thread_counting(uint32_t classes);
extern uint32_t kpc_get_counter_count(uint32_t classes);
extern int kpc_get_thread_counters(uint32_t tid, uint32_t buf_count, uint64_t *buf);

static int kpc_counters_ready;
static int kpc_clock_enabled;
static int kpc_calibrated_from_file;
static uint64_t kpc_base_instructions;
static uint64_t kpc_last_instruction_delta;
static uint64_t kpc_instr_per_ms;
static int kpc_calibration_active;
static uint64_t kpc_calibration_start_instructions;
static uint64_t kpc_calibration_end_instructions;
static uint64_t kpc_calibration_start_us;
static uint64_t kpc_calibration_end_us;

static uint64_t kpc_read_fixed_instructions(void) {
    uint64_t counters[8] = {0};
    uint32_t count = kpc_get_counter_count(KPC_CLASS_FIXED_MASK);
    if (count < 2 || count > 8) return 0;
    if (kpc_get_thread_counters(0, count, counters) != 0) return 0;
    return counters[1];
}

static uint64_t kpc_monotonic_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000 + (uint64_t)ts.tv_nsec / 1000;
}

static int kpc_load_calibration(uint64_t *out_instr_per_ms) {
    char key[64];
    char extra[2];
    unsigned long long value = 0;
    FILE *f = fopen(WAH_BENCH_KPC_CALIBRATION_FILE, "r");
    if (!f) return 0;
    if (fscanf(f, " %63s %llu %1s", key, &value, extra) == 2 &&
        strcmp(key, "coremark_instructions_per_ms") == 0 && value > 0) {
        fclose(f);
        *out_instr_per_ms = (uint64_t)value;
        return 1;
    }
    fclose(f);
    return 0;
}

static void kpc_save_calibration(uint64_t instr_per_ms) {
    FILE *f = fopen(WAH_BENCH_KPC_CALIBRATION_FILE, "w");
    if (!f) {
        fprintf(stderr, "Could not write KPC calibration file %s: %s\n",
                WAH_BENCH_KPC_CALIBRATION_FILE, strerror(errno));
        return;
    }
    fprintf(f, "coremark_instructions_per_ms %llu\n", (unsigned long long)instr_per_ms);
    fclose(f);
}

static void kpc_clock_setup(void) {
    if (geteuid() != 0) return;
    if (kpc_force_all_ctrs_set(1) != 0) goto fallback;
    if (kpc_set_counting(KPC_CLASS_FIXED_MASK) != 0) goto fallback;
    if (kpc_set_thread_counting(KPC_CLASS_FIXED_MASK) != 0) goto fallback;
    kpc_counters_ready = 1;
    if (kpc_load_calibration(&kpc_instr_per_ms)) {
        kpc_calibrated_from_file = 1;
        kpc_base_instructions = kpc_read_fixed_instructions();
        if (kpc_base_instructions == 0) goto fallback;
        kpc_clock_enabled = 1;
    }
    return;

fallback:
    kpc_counters_ready = 0;
    kpc_clock_enabled = 0;
    fprintf(stderr, "KPC clock unavailable, falling back to monotonic clock: %s\n", strerror(errno));
}

static uint64_t bench_clock_ms(void) {
    if (kpc_clock_enabled) {
        uint64_t instructions = kpc_read_fixed_instructions();
        if (instructions >= kpc_base_instructions) {
            uint64_t delta = instructions - kpc_base_instructions;
            kpc_last_instruction_delta = delta;
            return delta / kpc_instr_per_ms;
        }
        kpc_clock_enabled = 0;
    }
    return monotonic_ms();
}
#else
static uint64_t bench_clock_ms(void) {
    return monotonic_ms();
}
#endif

static void clock_ms_host(wah_call_context_t *ctx, void *userdata) {
    (void)userdata;
    wah_return_i32(ctx, (int32_t)bench_clock_ms());
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

static wah_error_t run_coremark(uint8_t *wasm, size_t wasm_size, wah_value_t *result) {
    wah_module_t mod = {0};
    wah_error_t err = wah_parse_module(&mod, wasm, wasm_size, NULL);
    if (err) { fprintf(stderr, "Parse error: %s\n", wah_strerror(err)); return err; }

    wah_module_t env = {0};
    wah_new_module(&env, NULL);
    wah_module_export_func(&env, "clock_ms", "() -> i32", clock_ms_host, NULL, NULL);

    wah_exec_context_t ctx = {0};
    wah_exec_context_create(&ctx, &mod, NULL);
    wah_link_module(&ctx, "env", &env);

    err = wah_instantiate(&ctx);
    if (err) { fprintf(stderr, "Instantiate error: %s\n", wah_strerror(err)); goto cleanup; }

    wah_export_desc_t run_entry;
    err = wah_module_export_by_name(&mod, "run", &run_entry);
    if (err) { fprintf(stderr, "Cannot find 'run' export: %s\n", wah_strerror(err)); goto cleanup; }

#ifdef WAH_BENCH_KPC_CLOCK
    if (kpc_calibration_active) {
        kpc_calibration_start_instructions = kpc_read_fixed_instructions();
        kpc_calibration_start_us = kpc_monotonic_us();
    }
#endif
    err = wah_call(&ctx, run_entry.index, NULL, 0, result);
#ifdef WAH_BENCH_KPC_CLOCK
    if (kpc_calibration_active) {
        kpc_calibration_end_us = kpc_monotonic_us();
        kpc_calibration_end_instructions = kpc_read_fixed_instructions();
    }
#endif
    if (err) { fprintf(stderr, "Call error: %s\n", wah_strerror(err)); goto cleanup; }

cleanup:
    wah_exec_context_destroy(&ctx);
    wah_free_module(&env);
    wah_free_module(&mod);
    return err;
}

#ifdef WAH_BENCH_KPC_CLOCK
static void kpc_calibrate_with_coremark(uint8_t *wasm, size_t wasm_size) {
    uint64_t elapsed_us;
    uint64_t delta;
    wah_value_t ignored;

    if (!kpc_counters_ready || kpc_clock_enabled) return;
    kpc_calibration_active = 1;
    if (run_coremark(wasm, wasm_size, &ignored) != WAH_OK) {
        kpc_calibration_active = 0;
        return;
    }
    kpc_calibration_active = 0;
    if (kpc_calibration_start_instructions == 0) return;
    if (kpc_calibration_end_instructions < kpc_calibration_start_instructions) return;
    elapsed_us = kpc_calibration_end_us - kpc_calibration_start_us;
    delta = kpc_calibration_end_instructions - kpc_calibration_start_instructions;
    if (elapsed_us == 0 || delta == 0) return;

    kpc_instr_per_ms = delta * 1000 / elapsed_us;
    if (kpc_instr_per_ms == 0) return;
    if (kpc_load_calibration(&kpc_instr_per_ms)) {
        kpc_calibrated_from_file = 1;
    } else {
        kpc_save_calibration(kpc_instr_per_ms);
    }
    kpc_base_instructions = kpc_read_fixed_instructions();
    if (kpc_base_instructions == 0) return;
    kpc_clock_enabled = 1;
}
#endif

int main(void) {
    size_t wasm_size = 0;
    uint8_t *wasm = read_file("bench/coremark-minimal.wasm", &wasm_size);
    wah_value_t result;
    wah_error_t err;
    if (!wasm) return 1;

#ifdef WAH_BENCH_KPC_CLOCK
    kpc_clock_setup();
    kpc_calibrate_with_coremark(wasm, wasm_size);
#endif

    printf("Running CoreMark");
#ifdef WAH_BENCH_KPC_CLOCK
    if (kpc_clock_enabled) {
        printf(" with KPC fixed-instruction clock");
        if (kpc_calibrated_from_file) {
            printf(" (calibrated from %s", WAH_BENCH_KPC_CALIBRATION_FILE);
        } else {
            printf(" (calibrated at startup");
        }
        printf(", %llu instructions/ms)", (unsigned long long)kpc_instr_per_ms);
    } else {
        printf(" with monotonic clock");
    }
#endif
    printf("...\n");
    err = run_coremark(wasm, wasm_size, &result);
    if (err) goto cleanup;

    printf("CoreMark result: %.2f\n", result.f32);
#ifdef WAH_BENCH_KPC_CLOCK
    if (kpc_clock_enabled) {
        printf("Host retired instructions observed by clock: %llu\n",
               (unsigned long long)kpc_last_instruction_delta);
    }
#endif

cleanup:
    free(wasm);
    return err ? 1 : 0;
}
