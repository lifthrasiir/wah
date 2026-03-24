#include <stdio.h>
#include <string.h>
#include <assert.h>

#define WAH_IMPLEMENTATION
#include "wah.h"
#include "wah_testutils.c"

int main(void) {
    wah_module_t module;

    printf("1. Testing ULEB128 value overflow (5th byte has continuation bit)...\n");
    const char *overflow_uleb_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        code {[ 0x100000000 unreachable end ]}";
    assert_err(wah_parse_module_from_spec(&module, overflow_uleb_spec), WAH_ERROR_TOO_LARGE);

    printf("2. Testing SLEB128 value overflow...\n");
    const char *sleb_overflow_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        code {[ {[] i32.const 0x80000000 end} ]}";
    assert_err(wah_parse_module_from_spec(&module, sleb_overflow_spec), WAH_ERROR_TOO_LARGE);

    printf("3. Testing SLEB128 value underflow...\n");
    const char *sleb_underflow_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        code {[ {[] i32.const -0x80000001 end} ]}";
    assert_err(wah_parse_module_from_spec(&module, sleb_underflow_spec), WAH_ERROR_TOO_LARGE);

    // This tests if (offset + num_elems) overflows the table bounds.
    // The offset is a valid sleb128, but the resulting address is out of bounds.
    printf("4. Testing element segment address overflow...\n");
    const char *elem_addr_overflow_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        tables {[ funcref limits.i32/1 1000 ]} \
        elements {[ elem.active.table#0 0 i32.const 990 end [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0] ]}";
    assert_err(wah_parse_module_from_spec(&module, elem_addr_overflow_spec), WAH_ERROR_VALIDATION_FAILED);

    // This tests if current_local_count += local_type_count overflows uint32_t
    // leading to an incorrect local_count and subsequent allocation failure or
    // out-of-bounds access.
    printf("5. Testing local count overflow...\n");
    const char *locals_count_overflow_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        code {[ {[0xffffffff i32, 1 i32] end} ]}";
    assert_err(wah_parse_module_from_spec(&module, locals_count_overflow_spec), WAH_ERROR_TOO_LARGE);

    // This tests a ULEB128 sequence that ends prematurely,
    // before the continuation bit is cleared. This should
    // trigger WAH_ERROR_UNEXPECTED_EOF.
    printf("6. Testing ULEB128 truncated (input ends prematurely)...\n");
    const char *uleb_truncated_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        code 7 1 %'80808080'";
    assert_err(wah_parse_module_from_spec(&module, uleb_truncated_spec), WAH_ERROR_UNEXPECTED_EOF);

    #define REPEAT8(x) x x x x x x x x
    #define REPEAT16(x) REPEAT8(x x)
    #define REPEAT64(x) REPEAT8(REPEAT8(x))
    #define REPEAT256(x) REPEAT64(x x x x)
    #define REPEAT1024(x) REPEAT256(x x x x)

    // This tests if the control stack depth exceeds WAH_MAX_CONTROL_DEPTH (256).
    // We create a function with 257 nested blocks, which should trigger WAH_ERROR_TOO_LARGE.
    printf("7. Testing control stack overflow (257 nested blocks)...\n");
    const char *control_stack_overflow_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        code {[ \
            {[] \
                " REPEAT256("block void ") "\
                    block void end \
                " REPEAT256("end ") "\
            end} \
        ]}";
    assert_err(wah_parse_module_from_spec(&module, control_stack_overflow_spec), WAH_ERROR_TOO_LARGE);

    // This tests if the type stack exceeds WAH_MAX_TYPE_STACK_SIZE (1024).
    // We create a block that contains many i32.const instructions followed by i32.add,
    // which pushes values to the type stack and should trigger WAH_ERROR_TOO_LARGE.
    printf("8. Testing type stack overflow (>1024 types)...\n");
    const char *type_stack_overflow_spec = "wasm \
        types {[ fn [] [] ]} \
        funcs {[ 0 ]} \
        code {[ \
            {[] \
                " REPEAT1024("i32.const 0 ") "i32.const 0 \
                " REPEAT1024("i32.add ") "\
                drop \
            end} \
        ]}";
    assert_err(wah_parse_module_from_spec(&module, type_stack_overflow_spec), WAH_ERROR_TOO_LARGE);

    // This tests if the constant expression stack exceeds its limit (16).
    // We create a global with a constant expression that tries to push more than 16 values.
    printf("9. Testing constant expression stack overflow (>16 items)...\n");
    const char *const_expr_stack_overflow_spec = "wasm \
        types {[ fn [] [] ]} \
        globals {[ i32 mut \
            " REPEAT16("i32.const 0 ") "i32.const 0 \
            " REPEAT16("i32.add ") "\
        end ]}";
    // This pushes 17 i32.const values before END.
    // The 17th push should trigger WAH_ERROR_TOO_LARGE.
    // The final stack size is still 1, so it should not trigger the END check.
    assert_err(wah_parse_module_from_spec(&module, const_expr_stack_overflow_spec), WAH_ERROR_TOO_LARGE);

    printf("--- All Overflow Tests Passed ---\n");
    return 0;
}
