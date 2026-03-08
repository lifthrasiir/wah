#define WAH_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include "wah.h"
#include "wah_testutils.c"

int main() {
    wah_module_t module;
    wah_exec_context_t ctx;
    wah_error_t err;

    printf("--- Testing Valid Module (simple_add_wasm) ---\n");
    printf("Parsing simple_add_wasm module...\n");
    err = wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] local.get 0 local.get 1 i32.add end } ]}");
    if (err != WAH_OK) {
        fprintf(stderr, "Error parsing valid module: %s\n", wah_strerror(err));
        return 1;
    }
    printf("Module parsed successfully.\n");

    // Create execution context
    err = wah_exec_context_create(&ctx, &module);
    if (err != WAH_OK) {
        fprintf(stderr, "Error creating execution context: %s\n", wah_strerror(err));
        wah_free_module(&module);
        return 1;
    }

    printf("Function max stack depth: %u\n", module.code_bodies[0].max_stack_depth);

    uint32_t func_idx = 0;
    wah_value_t params[2];
    wah_value_t result;

    params[0].i32 = 10;
    params[1].i32 = 20;

    printf("Interpreting function %u with params %d and %d...\n", func_idx, params[0].i32, params[1].i32);
    err = wah_call(&ctx, &module, func_idx, params, 2, &result);
    if (err != WAH_OK) {
        fprintf(stderr, "Error interpreting function: %s\n", wah_strerror(err));
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
        return 1;
    }
    printf("Function interpreted successfully.\n");
    printf("Result: %d\n", result.i32);

    params[0].i32 = 5;
    params[1].i32 = 7;
    printf("Interpreting function %u with params %d and %d...\n", func_idx, params[0].i32, params[1].i32);
    err = wah_call(&ctx, &module, func_idx, params, 2, &result);
    if (err != WAH_OK) {
        fprintf(stderr, "Error interpreting function: %s\n", wah_strerror(err));
        wah_exec_context_destroy(&ctx);
        wah_free_module(&module);
        return 1;
    }
    printf("Function interpreted successfully.\n");
    printf("Result: %d\n", result.i32);

    wah_exec_context_destroy(&ctx);
    wah_free_module(&module);
    printf("Valid module freed.\n");

    printf("\n--- Testing Invalid Module (invalid_local_get_wasm) ---\n");
    printf("Parsing invalid_local_get_wasm module...\n");
    err = wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [i32, i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] local.get 2 local.get 1 i32.add end } ]}");
    if (err == WAH_ERROR_VALIDATION_FAILED) {
        printf("Successfully detected invalid module during parsing (expected WAH_ERROR_VALIDATION_FAILED).\n");
    } else if (err != WAH_OK) {
        fprintf(stderr, "Error parsing invalid module: %s (Expected parsing to succeed)\n", wah_strerror(err));
        return 1;
    } else {
        fprintf(stderr, "Invalid module parsed successfully (Expected WAH_ERROR_VALIDATION_FAILED)\n");
        wah_free_module(&module);
        return 1;
    }

    printf("\n--- Testing Stack Underflow Module (stack_underflow_wasm) ---\n");
    printf("Parsing stack_underflow_wasm module...\n");
    err = wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [i32] [i32] ]} \
        funcs {[ 0 ]} \
        code {[ {[] local.get 0 i32.add end } ]}");
    if (err == WAH_ERROR_VALIDATION_FAILED) {
        printf("Successfully detected stack underflow during parsing (expected WAH_ERROR_VALIDATION_FAILED).\n");
    } else if (err != WAH_OK) {
        printf("Successfully detected stack underflow during parsing (%s).\n", wah_strerror(err));
    } else {
        fprintf(stderr, "Stack underflow module parsed successfully (Expected WAH_ERROR_VALIDATION_FAILED)\n");
        wah_free_module(&module);
        return 1;
    }

    return 0;
}
