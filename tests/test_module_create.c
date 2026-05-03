// Test for programmatic module creation

#include "../wah.h"
#include "common.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

// Test host function
void test_host_func(wah_call_context_t *ctx, void *userdata) {
    (void)ctx;
    (void)userdata;
    printf("  Host function called\n");
}

// Test userdata cleanup
static int cleanup_called = 0;
void test_cleanup(void *userdata) {
    (void)userdata;
    cleanup_called = 1;
    printf("  Cleanup called\n");
}

int main() {
    // Test 1: Create and destroy empty module
    printf("Test 1: Create and destroy empty module\n");
    wah_module_t mod1 = {0};
    assert_ok(wah_new_module(&mod1, NULL));
    wah_free_module(&mod1);

    // Test 2: Export a function
    printf("Test 2: Export a function\n");
    wah_module_t mod2 = {0};
    assert_ok(wah_new_module(&mod2, NULL));

    // Export function with no parameters and no results
    assert_ok(wah_export_func(&mod2, "test_func", "()", test_host_func, NULL, NULL));
    assert_ok(wah_export_func(&mod2, "test_func_explicit", "fn () -> ()", test_host_func, NULL, NULL));
    assert_err(wah_export_func(&mod2, "bad_placeholder", "fn (%T) -> ()", test_host_func, NULL, NULL), WAH_ERROR_BAD_SPEC);
    assert_err(wah_export_func(&mod2, "bad_struct", "struct { i32 }", test_host_func, NULL, NULL), WAH_ERROR_BAD_SPEC);

    // Verify export exists
    wah_export_desc_t entry;
    assert_ok(wah_export_by_name(&mod2, "test_func", &entry));

    // Check it's a function
    assert_eq_i32(entry.kind, WAH_KIND_FUNCTION);

    wah_free_module(&mod2);

    // Test 3: Export with userdata and cleanup
    printf("Test 3: Export with userdata and cleanup\n");
    wah_module_t mod3 = {0};
    assert_ok(wah_new_module(&mod3, NULL));

    int test_data = 42;
    assert_ok(wah_export_func(&mod3, "test_func2", "()", test_host_func, &test_data, test_cleanup));

    cleanup_called = 0;
    wah_free_module(&mod3);
    assert_true(cleanup_called);

    // Test 4: Multiple exports
    printf("Test 4: Multiple exports\n");
    wah_module_t mod4 = {0};
    assert_ok(wah_new_module(&mod4, NULL));

    assert_ok(wah_export_func(&mod4, "func1", "()", test_host_func, NULL, NULL));
    assert_ok(wah_export_func(&mod4, "func2", "()", test_host_func, NULL, NULL));
    assert_ok(wah_export_func(&mod4, "func3", "()", test_host_func, NULL, NULL));

    // Verify all exports exist
    for (int i = 1; i <= 3; i++) {
        char name[16];
        snprintf(name, sizeof(name), "func%d", i);
        assert_ok(wah_export_by_name(&mod4, name, &entry));
    }

    // Check export count
    assert_eq_u32(wah_module_export_count(&mod4), 3);

    wah_free_module(&mod4);

    printf("\nAll tests passed!\n");
    return 0;
}
