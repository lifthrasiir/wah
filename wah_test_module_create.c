// Test for programmatic module creation

#define WAH_IMPLEMENTATION
#include "wah.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

// Test host function
void test_host_func(wah_call_context_t *ctx, void *userdata) {
    (void)ctx;
    (void)userdata;
    printf("Host function called\n");
}

// Test userdata cleanup
static int cleanup_called = 0;
void test_cleanup(void *userdata) {
    (void)userdata;
    cleanup_called = 1;
    printf("Cleanup called\n");
}

int main() {
    printf("Testing programmatic module creation...\n\n");

    // Test 1: Create and destroy empty module
    printf("Test 1: Create and destroy empty module\n");
    wah_module_t mod1 = {0};
    wah_error_t err = wah_new_module(&mod1);
    if (err != WAH_OK) {
        printf("FAIL: wah_new_module returned %s\n", wah_strerror(err));
        return 1;
    }
    wah_free_module(&mod1);
    printf("PASS\n\n");

    // Test 2: Export a function
    printf("Test 2: Export a function\n");
    wah_module_t mod2 = {0};
    err = wah_new_module(&mod2);
    if (err != WAH_OK) {
        printf("FAIL: wah_new_module returned %s\n", wah_strerror(err));
        return 1;
    }

    // Export function with no parameters and no results
    err = wah_module_export_funcv(&mod2, "test_func", 0, NULL, 0, NULL, test_host_func, NULL, NULL);
    if (err != WAH_OK) {
        printf("FAIL: wah_module_export_func returned %s\n", wah_strerror(err));
        return 1;
    }

    // Verify export exists
    wah_entry_t entry;
    err = wah_module_export_by_name(&mod2, "test_func", &entry);
    if (err != WAH_OK) {
        printf("FAIL: wah_module_export_by_name returned %s\n", wah_strerror(err));
        return 1;
    }

    // Check it's a function
    if (!WAH_TYPE_IS_FUNCTION(entry.type)) {
        printf("FAIL: export is not a function type\n");
        return 1;
    }

    wah_free_module(&mod2);
    printf("PASS\n\n");

    // Test 3: Export with userdata and cleanup
    printf("Test 3: Export with userdata and cleanup\n");
    wah_module_t mod3 = {0};
    err = wah_new_module(&mod3);
    if (err != WAH_OK) {
        printf("FAIL: wah_new_module returned %s\n", wah_strerror(err));
        return 1;
    }

    int test_data = 42;
    err = wah_module_export_funcv(&mod3, "test_func2", 0, NULL, 0, NULL, test_host_func, &test_data, test_cleanup);
    if (err != WAH_OK) {
        printf("FAIL: wah_module_export_func returned %s\n", wah_strerror(err));
        return 1;
    }

    cleanup_called = 0;
    wah_free_module(&mod3);
    if (!cleanup_called) {
        printf("FAIL: cleanup was not called\n");
        return 1;
    }
    printf("PASS\n\n");

    // Test 4: Multiple exports
    printf("Test 4: Multiple exports\n");
    wah_module_t mod4 = {0};
    err = wah_new_module(&mod4);
    if (err != WAH_OK) {
        printf("FAIL: wah_new_module returned %s\n", wah_strerror(err));
        return 1;
    }

    err = wah_module_export_funcv(&mod4, "func1", 0, NULL, 0, NULL, test_host_func, NULL, NULL);
    if (err != WAH_OK) {
        printf("FAIL: wah_module_export_funcv returned %s\n", wah_strerror(err));
        return 1;
    }

    err = wah_module_export_funcv(&mod4, "func2", 0, NULL, 0, NULL, test_host_func, NULL, NULL);
    if (err != WAH_OK) {
        printf("FAIL: wah_module_export_funcv returned %s\n", wah_strerror(err));
        return 1;
    }

    err = wah_module_export_funcv(&mod4, "func3", 0, NULL, 0, NULL, test_host_func, NULL, NULL);
    if (err != WAH_OK) {
        printf("FAIL: wah_module_export_func returned %s\n", wah_strerror(err));
        return 1;
    }

    // Verify all exports exist
    for (int i = 1; i <= 3; i++) {
        char name[16];
        snprintf(name, sizeof(name), "func%d", i);
        err = wah_module_export_by_name(&mod4, name, &entry);
        if (err != WAH_OK) {
            printf("FAIL: wah_module_export_by_name(%s) returned %s\n", name, wah_strerror(err));
            return 1;
        }
    }

    // Check export count
    size_t count = wah_module_num_exports(&mod4);
    if (count != 3) {
        printf("FAIL: expected 3 exports, got %zu\n", count);
        return 1;
    }

    wah_free_module(&mod4);
    printf("PASS\n\n");

    printf("All tests passed!\n");
    return 0;
}
