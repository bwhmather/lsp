#include <stdlib.h>
#include <signal.h>

static void on_sigabrt(int signal) {
    exit(0);
}

#define lspt_assert_aborts(expr) do { \
    signal(SIGABRT, &on_sigabrt); \
    expr; \
    exit(1); \
} while(0)


#define lspt_assert(condition)
#define lspt_assert_not(condition)
#define lspt_assert_eq(actual, expected)


#define lspt_expect(condition) lspt_assert(condition)
#define lspt_expect_not(condition) lspt_expect(condition)
#define lspt_expect_eq(actual, expected) lspt_expect_eq(a, b)
