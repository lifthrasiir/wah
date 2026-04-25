#ifndef WAH_TEST_COMMON_H
#define WAH_TEST_COMMON_H

#include "../wah.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int wah_build_spec_binaryv(uint8_t **out_data, size_t *out_size, const char *fmt, va_list args);
int wah_build_spec_binary(uint8_t **out_data, size_t *out_size, const char *fmt, ...);

static inline wah_error_t wah_parse_module_from_specv(wah_module_t *module, const char *fmt, va_list args) {
    uint8_t *data; size_t size;
    va_list args_copy;
    va_copy(args_copy, args);
    int ok = wah_build_spec_binaryv(&data, &size, fmt, args_copy);
    va_end(args_copy);
    if (!ok) return WAH_ERROR_BAD_SPEC;
    wah_error_t err = wah_parse_module(data, size, module);
    free(data);
    return err;
}

static inline wah_error_t wah_parse_module_from_spec_impl_(wah_module_t *module, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    wah_error_t err = wah_parse_module_from_specv(module, fmt, args);
    va_end(args);
    return err;
}
#define wah_parse_module_from_spec(module, fmt, ...) \
    wah_parse_module_from_spec_impl_(module, fmt, ##__VA_ARGS__)

void assert_eq_i32(const char *file, int line, int32_t actual, int32_t expected, const char *expr);
void assert_eq_u32(const char *file, int line, uint32_t actual, uint32_t expected, const char *expr);
void assert_eq_i64(const char *file, int line, int64_t actual, int64_t expected, const char *expr);
void assert_eq_u64(const char *file, int line, uint64_t actual, uint64_t expected, const char *expr);
void assert_eq_f32(const char *file, int line, float actual, float expected, float epsilon, const char *expr);
void assert_eq_f64(const char *file, int line, double actual, double expected, double epsilon, const char *expr);
void assert_eq_str(const char *file, int line, const char *actual, const char *expected, const char *expr);
void assert_true(const char *file, int line, bool condition, const char *expr);
void assert_false(const char *file, int line, bool condition, const char *expr);
void assert_null(const char *file, int line, const void *ptr, const char *expr);
void assert_not_null(const char *file, int line, const void *ptr, const char *expr);
void assert_eq_ptr(const char *file, int line, const void *actual, const void *expected, const char *expr);

#define assert_eq_i32(actual, expected) assert_eq_i32(__FILE__, __LINE__, (actual), (expected), #actual " == " #expected)
#define assert_eq_u32(actual, expected) assert_eq_u32(__FILE__, __LINE__, (actual), (expected), #actual " == " #expected)
#define assert_eq_i64(actual, expected) assert_eq_i64(__FILE__, __LINE__, (actual), (expected), #actual " == " #expected)
#define assert_eq_u64(actual, expected) assert_eq_u64(__FILE__, __LINE__, (actual), (expected), #actual " == " #expected)
#define assert_eq_f32(actual, expected, epsilon) assert_eq_f32(__FILE__, __LINE__, (actual), (expected), (epsilon), #actual " == " #expected " within " #epsilon)
#define assert_eq_f64(actual, expected, epsilon) assert_eq_f64(__FILE__, __LINE__, (actual), (expected), (epsilon), #actual " == " #expected " within " #epsilon)
#define assert_eq_str(actual, expected) assert_eq_str(__FILE__, __LINE__, (actual), (expected), #actual " == " #expected)
#define assert_true(condition) assert_true(__FILE__, __LINE__, (condition), #condition)
#define assert_false(condition) assert_false(__FILE__, __LINE__, (condition), #condition)
#define assert_null(ptr) assert_null(__FILE__, __LINE__, (ptr), #ptr " == NULL")
#define assert_not_null(ptr) assert_not_null(__FILE__, __LINE__, (ptr), #ptr " != NULL")
#define assert_eq_ptr(actual, expected) assert_eq_ptr(__FILE__, __LINE__, (actual), (expected), #actual " == " #expected)

#define assert_err(actual, expected) do { \
    wah_error_t assert_err_a_ = (actual), assert_err_e_ = (expected); \
    if (assert_err_a_ != assert_err_e_) { \
        fprintf(stderr, "%s:%d: Assertion failed: %s (expected %s, got %s)\n", \
                __FILE__, __LINE__, #actual " == " #expected, \
                wah_strerror(assert_err_e_), wah_strerror(assert_err_a_)); \
        exit(1); \
    } \
} while(0)

#define assert_ok(err) do { \
    wah_error_t assert_ok_e_ = (err); \
    if (assert_ok_e_ != WAH_OK) { \
        fprintf(stderr, "%s:%d: Assertion failed: %s (expected WAH_OK, got %s)\n", \
                __FILE__, __LINE__, #err " == WAH_OK", wah_strerror(assert_ok_e_)); \
        exit(1); \
    } \
} while(0)

#endif
