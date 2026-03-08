#include <stdio.h>
#include <string.h>
#include <assert.h>

#define WAH_IMPLEMENTATION
#include "wah.h"
#include "wah_testutils.c"

int main(void) {
    wah_module_t module;
    wah_error_t err;

    printf("--- Running Overflow Tests ---\n");

    printf("1. Testing ULEB128 value overflow (5th byte has continuation bit)...\n");
    err = wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        code {[ 0x100000000 unreachable end ]}");
    if (err != WAH_ERROR_TOO_LARGE) {
        printf("  - FAILED: Expected WAH_ERROR_TOO_LARGE, got: %s\n", wah_strerror(err));
        return 1;
    }
    printf("  - PASSED\n");

    printf("2. Testing SLEB128 value overflow...\n");
    err = wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        code {[ {[] i32.const 0x80000000 end} ]}");
    if (err != WAH_ERROR_TOO_LARGE) {
        printf("  - FAILED: Expected WAH_ERROR_TOO_LARGE, got: %s\n", wah_strerror(err));
        return 1;
    }
    printf("  - PASSED\n");

    printf("3. Testing SLEB128 value underflow...\n");
    err = wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        code {[ {[] i32.const -0x80000001 end} ]}");
    if (err != WAH_ERROR_TOO_LARGE) {
        printf("  - FAILED: Expected WAH_ERROR_TOO_LARGE, got: %s\n", wah_strerror(err));
        return 1;
    }
    printf("  - PASSED\n");

    // This tests if (offset + num_elems) overflows the table bounds.
    // The offset is a valid sleb128, but the resulting address is out of bounds.
    printf("4. Testing element segment address overflow...\n");
    err = wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 1000 ]} \
        elements {[ elem.active.table#0 0 i32.const 990 end [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0] ]}");
    if (err != WAH_ERROR_VALIDATION_FAILED) {
        printf("  - FAILED: Expected WAH_ERROR_VALIDATION_FAILED, got: %s\n", wah_strerror(err));
        if (err == WAH_OK) {
            wah_free_module(&module);
        }
        return 1;
    }
    printf("  - PASSED\n");

    // This tests if current_local_count += local_type_count overflows uint32_t
    // leading to an incorrect local_count and subsequent allocation failure or
    // out-of-bounds access.
    printf("5. Testing local count overflow...\n");
    err = wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        code {[ {[0xffffffff i32, 1 i32] end} ]}");
    if (err != WAH_ERROR_TOO_LARGE) {
        printf("  - FAILED: Expected WAH_ERROR_TOO_LARGE, got: %s\n", wah_strerror(err));
        return 1;
    }
    printf("  - PASSED\n");

    // This tests a ULEB128 sequence that ends prematurely,
    // before the continuation bit is cleared. This should
    // trigger WAH_ERROR_UNEXPECTED_EOF.
    printf("6. Testing ULEB128 truncated (input ends prematurely)...\n");
    err = wah_parse_module_from_spec(&module, "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        code 7 1 %'80808080'");
    if (err != WAH_ERROR_UNEXPECTED_EOF) {
        printf("  - FAILED: Expected WAH_ERROR_UNEXPECTED_EOF, got: %s\n", wah_strerror(err));
        return 1;
    }
    printf("  - PASSED\n");

    printf("--- All Overflow Tests Passed ---\n");

    return 0;
}
