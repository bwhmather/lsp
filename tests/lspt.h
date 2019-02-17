#include <stdlib.h>
#include <signal.h>

#define LSPT_EXPAND(x) x

#define lspt_assert(condition) do { \
    bool lspt_internal_condition = !!(condition); \
    /* char *lspt_internal_lineno = __LINE__; */ \
    /* char *lspt_internal_file = __FILE__; */ \
    if (!condition) { \
        abort(); \
    } \
} while (0)


static void lspt_internal_on_sigabrt(int signal) {
    exit(0);
}

#define lspt_assert_aborts(expr) do { \
    signal(SIGABRT, &lspt_internal_on_sigabrt); \
    expr; \
    exit(1); \
} while(0)

#define lspt_assert_not(condition) LSPT_EXPAND(lspt_assert(!(condition)))
#define lspt_assert_eq(actual, expected) LSPT_EXPAND(lspt_assert(((actual) == (expected))))

#define lspt_expect(condition) LSPT_EXPAND(lspt_assert((condition)))
#define lspt_expect_not(condition) LSPT_EXPAND(lspt_assert((condition)))
#define lspt_expect_eq(actual, expected) LSPT_EXPAND(lspt_assert_eq((actual), (expected)))
