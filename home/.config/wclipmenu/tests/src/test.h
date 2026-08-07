#ifndef KT_TEST_H
#define KT_TEST_H

/* test.h - minimal C test framework for the kaprica CLI harness.
 * Zero dependencies beyond libc (stdio/string/stdlib).
 *
 * Usage:
 *     TEST(foo) {
 *         KT_ASSERT(1 == 1);
 *         KT_ASSERT_STR_EQ("a", "a");
 *     }
 *     KT_MAIN()
 *
 * Each binary is one suite: test_*.c defines the registry globals below,
 * then KT_MAIN() supplies main(). Assertions are non-fatal: every one
 * prints "  [PASS]/[FAIL] file:line cond" and the test keeps running.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define KT_MAX_TESTS 256

typedef void (*kt_test_fn)(void);

typedef struct {
    const char *name;
    kt_test_fn fn;
} kt_test;

extern kt_test kt_tests[KT_MAX_TESTS];
extern int kt_test_count;
extern int kt_pass;
extern int kt_fail;

/* Print one [PASS]/[FAIL] line and bump the counters. */
static inline void kt_report(int ok, const char *file, int line, const char *cond) {
    if (ok) {
        kt_pass++;
        printf("  [PASS] %s:%d %s\n", file, line, cond);
    } else {
        kt_fail++;
        printf("  [FAIL] %s:%d %s\n", file, line, cond);
    }
    fflush(stdout);
}

/* TEST(name) - declare + register a test at load time.
 * Expands to: forward declaration, constructor that appends
 * { "name", name } to the registry, and the opening of the function
 * definition, so the body is written right after the macro. */
#if defined(__GNUC__)
#define TEST(name)                                                     \
    static void name(void);                                            \
    __attribute__((constructor)) static void kt_reg_##name(void) {     \
        if (kt_test_count < KT_MAX_TESTS) {                            \
            kt_tests[kt_test_count].name = #name;                      \
            kt_tests[kt_test_count].fn = name;                         \
            kt_test_count++;                                           \
        }                                                              \
    }                                                                  \
    static void name(void)
#else
#define TEST(name) static void name(void)
#endif

#define KT_ASSERT(cond) \
    do { kt_report(((cond) != 0), __FILE__, __LINE__, #cond); } while (0)

#define KT_ASSERT_STR_EQ(a, b) \
    do { \
        const char *kt_a_ = (a); \
        const char *kt_b_ = (b); \
        int kt_ok_ = (kt_a_ && kt_b_) ? (strcmp(kt_a_, kt_b_) == 0) \
                                      : (kt_a_ == kt_b_); \
        kt_report(kt_ok_, __FILE__, __LINE__, "str_eq(" #a ", " #b ")"); \
    } while (0)

#define KT_ASSERT_STR_NE(a, b) \
    do { \
        const char *kt_a_ = (a); \
        const char *kt_b_ = (b); \
        int kt_ok_ = !((kt_a_ == NULL && kt_b_ == NULL) || \
                       (kt_a_ && kt_b_ && strcmp(kt_a_, kt_b_) == 0)); \
        kt_report(kt_ok_, __FILE__, __LINE__, "str_ne(" #a ", " #b ")"); \
    } while (0)

#define KT_ASSERT_STR_CONTAINS(haystack, needle) \
    do { \
        const char *kt_h_ = (haystack); \
        const char *kt_n_ = (needle); \
        int kt_ok_ = (kt_h_ != NULL && kt_n_ != NULL && \
                      strstr(kt_h_, kt_n_) != NULL); \
        kt_report(kt_ok_, __FILE__, __LINE__, \
                  "str_contains(" #haystack ", " #needle ")"); \
    } while (0)

#define KT_ASSERT_EQ_INT(a, b) \
    do { kt_report(((a) == (b)) != 0, __FILE__, __LINE__, \
                   #a " == " #b); } while (0)

#define KT_ASSERT_MEM_EQ(a, b, len) \
    do { \
        int kt_ok_ = (memcmp((a), (b), (len)) == 0); \
        kt_report(kt_ok_, __FILE__, __LINE__, \
                  "mem_eq(" #a ", " #b ", " #len ")"); \
    } while (0)

#define KT_FAIL(msg) \
    do { kt_report(0, __FILE__, __LINE__, (msg)); } while (0)

/* KT_MAIN() - run all registered tests in order, print the summary
 * "passed=N failed=N", and exit 0 iff nothing failed. */
#define KT_MAIN()                                                      \
    int main(void) {                                                   \
        int i;                                                         \
        kt_pass = 0;                                                   \
        kt_fail = 0;                                                   \
        for (i = 0; i < kt_test_count; i++) {                          \
            printf("[ RUN ] %s\n", kt_tests[i].name);                  \
            fflush(stdout);                                            \
            kt_tests[i].fn();                                          \
        }                                                              \
        printf("passed=%d failed=%d\n", kt_pass, kt_fail);             \
        return (kt_fail == 0) ? 0 : 1;                                 \
    }

#endif /* KT_TEST_H */
