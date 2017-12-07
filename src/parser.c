#include "heap.h"

#include <stdio.h>
#include <assert.h>


lsp_expr_t *lsp_parse() {
    char next;

    // The reversed list of elements in the current body.
    lsp_expr_t *body = NULL;

    // stack is a list, ordered inner to outer, of pointers to the last cons
    // cell in each list body.
    lsp_expr_t *stack = NULL;

    next = getchar();

    while (true) {
        lsp_expr_t *expression = NULL;

        // Consume whitespace whitespace.
        if (next == ' ' || next == '\n') {
            next = getchar();
            continue;
        } else if (next == '(') {
            next = getchar();

            // Push the current body onto the stack, and start a new one.
            stack = lsp_cons(body, stack);
            body = NULL;

            // No expression to add.  Skip logic at end of loop.
            continue;

        } else if (next == ')' || next == EOF) {
            // Unwind and reverse the current body list and store it as the
            // current expression.
            while (lsp_type(body) == LSP_CONS) {
                expression = lsp_cons(lsp_car(body), expression);
                body = lsp_cdr(body);
            }

            if (next == EOF) {
                return expression;
            }

            // Pop the containing body from the stack and set it as current.
            body = lsp_car(stack);
            stack = lsp_cdr(stack);

            next = getchar();
        } else if (next == '.') {
            // TODO figure out how to parse non-list cons cells.
        } else if (next >= '0' && next <= '9') {
            int accumulator = 0;

            // Parse number.
            while (true) {
                if (next >= '0' && next <= '9') {
                    accumulator *= 10;
                    accumulator += (int) (next - '0');
                } else if (
                    next == '(' || next == ')' ||
                    next == ' ' || next == '\n' ||
                    next == EOF
                ) {
                    expression = lsp_int(accumulator);
                    break;
                } else {
                    assert(false);
                }
                next = getchar();
            }
        } else {
            expression = lsp_symbol_start();
            // Parse symbol.
            while (true) {
                if (
                    next == '(' || next == ')' ||
                    next == ' ' || next == '\n'
                ) {
                    break;
                }

                lsp_symbol_push(next);
                next = getchar();
            }
            lsp_symbol_stop();
        }

        // Push expression.
        body = lsp_cons(expression, body);
    }
}

