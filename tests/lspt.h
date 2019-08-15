#pragma once

#include "lsp.h"

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

static inline void lspt_assert_equal(void) {
    // We loop to compare lists without overflowing the c stack.
    while (true) {
        if (lsp_is_null()) {
            lsp_dup(1);
            lspt_assert(lsp_is_null());
            lsp_pop();

        } else if (lsp_is_cons()) {
            // Check car.
            lsp_dup(1);
            lsp_car();
            lsp_dup(1);
            lsp_car();
            // We can't avoid recursing to compare the cars
            lspt_assert_equal();
            lsp_pop();
            lsp_pop();

            // Replace the two cons cells with their cdrs.
            lsp_dup(1);
            lsp_cdr();
            lsp_dup(1);
            lsp_cdr();
            lsp_store(2);
            lsp_store(2);

            // Loop to compare the cdrs.
            continue;

        } else if (lsp_is_int()) {
            int value_a = lsp_read_int(0);
            int value_b = lsp_read_int(1);

            lspt_assert(value_a == value_b);

        } else if (lsp_is_symbol()) {
            char *value_a = lsp_borrow_symbol(0);
            char *value_b = lsp_borrow_symbol(1);

            lspt_assert(strcmp(value_a, value_b) == 0);

        } else if (lsp_is_string()) {
            char *value_a = lsp_borrow_string(0);
            char *value_b = lsp_borrow_string(1);

            lspt_assert(strcmp(value_a, value_b) == 0);

        } else if (lsp_is_op()) {
            lsp_op_t value_a = lsp_read_op(0);
            lsp_op_t value_b = lsp_read_op(1);

            lspt_assert(value_a == value_b);

        } else {
            // Unknown type.
            lspt_assert(false);
        }

        return;
    }
}

