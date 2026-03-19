#define WAH_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // For memcpy, memset
#include "wah.h"

// Define maximum WASM input size to prevent excessive memory allocation.
// afl-fuzz typically handles this by limiting input size.
#define MAX_WASM_INPUT_SIZE (10 * 1024 * 1024) // 10 MB

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
    err = wah_parse_module(wasm_binary, current_size, &module);
    if (err != WAH_OK) {
        goto cleanup_binary; // Return non-zero for afl-fuzz to detect a crash/bug
    }

    // 2. Create execution context
    memset(&exec_ctx, 0, sizeof(wah_exec_context_t)); // Initialize context struct
    err = wah_exec_context_create(&exec_ctx, &module);
    if (err != WAH_OK) {
        goto cleanup_module;
    }

    // 3. Attempt to call the _start function if it exists
    // This is a common entry point for WASM modules.
    wah_entry_t start_func_entry;
    wah_error_t find_start_err = wah_module_export_by_name(&module, "_start", &start_func_entry);

    if (find_start_err == WAH_OK && start_func_entry.type == WAH_TYPE_FUNCTION) {
        err = wah_call(&exec_ctx, WAH_GET_ENTRY_INDEX(start_func_entry.id), NULL, 0, NULL);
        if (err != WAH_OK) {
            goto cleanup_exec_ctx;
        }
    } else if (find_start_err != WAH_ERROR_NOT_FOUND) {
        // If another error occurred while finding _start, or it's not a function
        err = find_start_err;
        goto cleanup_exec_ctx;
    }
    // If _start is not found, it's not necessarily an error for fuzzing,
    // as we're primarily testing parsing and context creation.

cleanup_exec_ctx:
    wah_exec_context_destroy(&exec_ctx);
cleanup_module:
    wah_free_module(&module);
cleanup_binary:
    free(wasm_binary);

    // Return 0 for success, non-zero for any error/trap
    return (err == WAH_OK) ? 0 : 1;
}