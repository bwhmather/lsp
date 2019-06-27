#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <setjmp.h>


#define lspt_expect(lsp_test_condition) do {                                \
    if (!(lsp_test_condition)) {                                            \
        fprintf(                                                            \
            stderr, "%s:%i %s: Expectation `%s' failed",                    \
            __FILE__, __LINE__, __func__, #lsp_test_condition               \
        );                                                                  \
    }                                                                       \
} while (0)

#define lspt_assert(lsp_test_condition) do {                                \
    if (!(lsp_test_condition)) {                                            \
        fprintf(                                                            \
            stderr, "%s:%i %s: Assertion `%s' failed",                      \
            __FILE__, __LINE__, __func__, #lsp_test_condition               \
        );                                                                  \
        abort();                                                            \
    }                                                                       \
} while (0)

static jmp_buf lspt_test_on_sigabrt_jump_target;
static inline void lspt_test_on_sigabrt(int signal) {
    (void) signal;  /* unused */
    longjmp(lspt_test_on_sigabrt_jump_target, 1);
}

#define lspt_assert_aborts(lspt_test_expression) do {                       \
    struct sigaction lspt_test_handler_old;                                 \
    struct sigaction lspt_test_handler_new;                                 \
    memset(&lspt_test_handler_new, 0, sizeof(lspt_test_handler_new));       \
    sigemptyset(&lspt_test_handler_new.sa_mask);                            \
    lspt_test_handler_new.sa_handler = &lspt_test_on_sigabrt;               \
    sigaction(SIGABRT, &lspt_test_handler_new, &lspt_test_handler_old);     \
    int lspt_test_abort_called = setjmp(lspt_test_on_sigabrt_jump_target);  \
    if (!lspt_test_abort_called) {                                          \
        lspt_test_expression;                                               \
    }                                                                       \
    sigaction(SIGABRT, &lspt_test_handler_old, NULL);                       \
    if (!lspt_test_abort_called) {                                          \
        fprintf(                                                            \
            stderr, "%s:%i %s: Expression `%s' did not abort",              \
            __FILE__, __LINE__, __func__, #lspt_test_expression             \
        );                                                                  \
        abort();                                                            \
    }                                                                       \
} while(0)

