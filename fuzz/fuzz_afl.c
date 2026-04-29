#define WAH_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // For memcpy, memset
#include "../wah.h"

// Define maximum WASM input size to prevent excessive memory allocation.
// afl-fuzz typically handles this by limiting input size.
#define MAX_WASM_INPUT_SIZE (10 * 1024 * 1024) // 10 MB
#define WAH_FUZZ_CALL_FUEL 2048

int main(void) {
    wah_module_t module;
    wah_exec_context_t exec_ctx;
    wah_error_t err = WAH_OK;

    // afl-fuzz typically provides input via standard input (stdin).
    // Here, we read the WASM binary from stdin until EOF.
    size_t current_size = 0;
    size_t capacity = 4096; // Initial buffer capacity
    uint8_t *wasm_binary = (uint8_t *)malloc(capacity);
    if (!wasm_binary) {
        fprintf(stderr, "Error: Failed to allocate initial buffer for WASM.\n");
        return 1;
    }

    int c;
    while ((c = getchar()) != EOF) {
        if (current_size >= capacity) {
            if (capacity >= MAX_WASM_INPUT_SIZE) {
                fprintf(stderr, "Error: WASM input size exceeds maximum allowed (%u bytes).\n", MAX_WASM_INPUT_SIZE);
                err = WAH_ERROR_TOO_LARGE;
                goto cleanup_binary;
            }
            capacity *= 2;
            if (capacity > MAX_WASM_INPUT_SIZE) { // Adjust to not exceed MAX_WASM_INPUT_SIZE
                capacity = MAX_WASM_INPUT_SIZE;
            }
            uint8_t *new_wasm_binary = (uint8_t *)realloc(wasm_binary, capacity);
            if (!new_wasm_binary) {
                fprintf(stderr, "Error: Failed to reallocate buffer for WASM.\n");
                err = WAH_ERROR_OUT_OF_MEMORY;
                goto cleanup_binary;
            }
            wasm_binary = new_wasm_binary;
        }
        wasm_binary[current_size++] = (uint8_t)c;
    }

    if (current_size == 0) {
        // fprintf(stderr, "Error: No WASM input provided.\n"); // afl-fuzz does not require verbose output.
        err = WAH_ERROR_UNEXPECTED_EOF;
        goto cleanup_binary;
    }

    // 1. Parse the WASM module
    memset(&module, 0, sizeof(wah_module_t)); // Initialize module struct
    wah_parse_options_t parse_options = {
        .features = WAH_DEFAULT_FEATURES,
        .enable_fuel_metering = true,
    };
    err = wah_parse_module_ex(wasm_binary, current_size, &parse_options, &module);
    if (err != WAH_OK) {
        goto cleanup_module;
    }

    // 2. Create execution context
    memset(&exec_ctx, 0, sizeof(wah_exec_context_t)); // Initialize context struct
    err = wah_exec_context_create(&exec_ctx, &module);
    if (err != WAH_OK) {
        goto cleanup_module;
    }

    err = wah_instantiate(&exec_ctx);
    if (err != WAH_OK) {
        goto cleanup_exec_ctx;
    }

    // 3. Call every exported function that does not require parameters.
    for (uint32_t i = 0; i < wah_module_export_count(&module); i++) {
        wah_export_desc_t entry;
        err = wah_module_export(&module, i, &entry);
        if (err != WAH_OK) {
            goto cleanup_exec_ctx;
        }

        if (entry.kind != WAH_KIND_FUNCTION || entry.u.func.param_count != 0) {
            continue;
        }

        wah_value_t results[16];
        uint32_t actual_results = 0;
        uint32_t max_results = entry.u.func.result_count <= 16 ? entry.u.func.result_count : 16;

        wah_set_fuel(&exec_ctx, WAH_FUZZ_CALL_FUEL);
        err = wah_call_multi(&exec_ctx, entry.index, NULL, 0, results, max_results, &actual_results);
        if (err == WAH_STATUS_FUEL_EXHAUSTED) {
            err = WAH_OK;
            continue;
        }
        if (err != WAH_OK) {
            goto cleanup_exec_ctx;
        }
    }

cleanup_exec_ctx:
    wah_exec_context_destroy(&exec_ctx);
cleanup_module:
    wah_free_module(&module);
cleanup_binary:
    free(wasm_binary);

    // Return 0 for success, non-zero for any error/trap
    return (err == WAH_OK) ? 0 : 1;
}
