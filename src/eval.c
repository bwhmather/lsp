#include "lsp.h"

#include <string.h>
#include <assert.h>

/**
 * Helper function that expands the first n elements of a list followed by the
 * tail onto the stack.
 */
static void lsp_unpack(int n) {
    for (int i = 0; i < n; i++) {
        lsp_dup(0);
        lsp_car();
        lsp_swp(1);
        lsp_cdr();
    }
}


/**
 * Evaluates the expression passed as its second argument in the environment
 * passed as its first.
 */
static void lsp_eval_inner(void) {
    if (lsp_is_symbol(1)) {
        // Expression is a name identifying a variable that can be loaded
        // from the environment.  `lookup` will pop the environment and symbol
        // from the stack and replace them with the correct value.
        lsp_lookup();
        return;

    } else if (lsp_is_cons(1)) {
        // Expression is a list representing either a special form or an
        // invocation of a procedure or built-in operator.

        // Unpack the first element from the list to check if it is a symbol.
        lsp_dup(-1);
        lsp_car();
        if (lsp_is_symbol(0)) {
            // The first item in the list is a symbol.  We first check if it
            // represents a special form and if that doesn't work fall through
            // to evaluating as an expression.
            char const *sym = lsp_borrow_symbol(0);

            if (strcmp(sym, "if") == 0) {
                lsp_pop();

                // Duplicate the expression, and strip the leading `if`.
                lsp_dup(1);
                lsp_cdr();

                // Unpack the predicate, subsequent and alternate expressions.
                lsp_unpack(3);

                // Pop the tail of the expression and check that it contains no
                // further elements.
                assert(lsp_is_null(0));
                lsp_pop();

                // Evaluate and check the predicate.
                lsp_dup(-3);  // The predicate expression.
                lsp_dup(-2);  // The environment.
                lsp_eval();

                if (lsp_is_truthy()) {
                    lsp_pop();  // The result.
                    lsp_pop();  // The alternate.
                    lsp_store(-1);  // The subsequent.
                    lsp_pop();  // The predicate.
                    lsp_eval();
                } else {
                    lsp_pop();  // The result.
                    lsp_store(-1);  // The alternate.
                    lsp_pop();  // The subsequent.
                    lsp_pop();  // The predicate.
                    lsp_eval();
                }

                return;
            }
            if (strcmp(sym, "quote") == 0) {
                // Strip the `quote` from the top of the stack and the
                // beginning of the current expression.
                lsp_pop();
                lsp_cdr();

                // Unpack the expression.
                lsp_unpack(1);

                // Pop the tail of the expression and check that it contains no
                // further elements.
                assert(lsp_is_null(0));

                // Return the quoted expression.
                lsp_store(0);
                lsp_pop_to(1);
                return;
            }
            if (strcmp(sym, "define") == 0) {
                // Strip the `define` from the top of the stack and the
                // beginning of the current expression.
                lsp_pop();
                lsp_cdr();

                // Unpack the key and value.
                lsp_unpack(2);

                // Pop the tail of the expression and check that it contains no
                // further elements.
                assert(lsp_is_null(0));

                // Evaluate the value.
                lsp_dup(0);  // The environment.
                lsp_swp(-1);  // Swap the environment and value expression.
                lsp_eval();

                // Bind the evaluated value to the key.
                lsp_define();

                // Return NULL.
                lsp_pop_to(0);
                lsp_push_null();
                return;
            }
            if (strcmp(sym, "set!") == 0) {
                // Strip the `set!` from the top of the stack and the beginning
                // of the current expression.
                lsp_pop();
                lsp_cdr();

                // Unpack the key and value.
                lsp_unpack(2);

                // Pop the tail of the expression and check that it contains no
                // further elements.
                assert(lsp_is_null(0));

                // Evaluate the value.
                lsp_dup(0);  // The environment.
                lsp_swp(-1);  // Swap the environment and value expression.
                lsp_eval();

                // Bind the evaluated value to the key.
                lsp_set();

                // Return NULL.
                lsp_pop_to(0);
                lsp_push_null();
                return;
            }
            if (strcmp(sym, "lambda") == 0) {
                // Strip the `lambda` from the top of the stack and the
                // beginning of the current expression.
                lsp_pop();
                // Strip the `lambda`.
                lsp_cdr();

                // Unpack the argument list and body.
                lsp_unpack(2);

                // Pop the tail of the expression and check that it contains no
                // further elements.
                assert(lsp_is_null(0));

                // Wrap the arg spec and function body up to form a function.
                lsp_cons();

                // Bind the function object and environment together to create
                // a closure.
                lsp_dup(0);
                lsp_cons();

                // Return the closure.
                lsp_store(0);
                lsp_pop_to(1);
                return;
            }
            if (strcmp(sym, "begin") == 0) {
                // Strip the `begin` from the top of the stack and the
                // beginning of the current expression.
                lsp_pop();
                lsp_cdr();

                // Set a default result in case there are no expressions.
                lsp_push_null();

                while (true) {
                    // Check that we haven't reached the end.
                    lsp_dup(2);
                    if (lsp_is_null(0)) {
                        break;
                    }

                    // Discard the previous result.
                    lsp_pop_to(3);

                    // Evaluate the next expression in the list.
                    lsp_dup(0);  // The environment.
                    lsp_dup(2);  // The expression.
                    lsp_car();
                    lsp_eval();
                }

                // Return the result of evaluating the last expression in the
                // list.
                lsp_store(0);
                lsp_pop_to(1);
                return;
            }

            // Strip the unrecognised symbol from the top of the stack.
            lsp_pop();
        }

        int length = 0;

        // Evaluate the whole list of expressions and unpack it to the stack.
        while (lsp_dup(-1), !lsp_is_null(0)) {
            length += 1;

            // Copy the environment to the top of the stack.
            lsp_dup(0);

            // Unpack the next expression in the list on top of it.
            lsp_dup(-2);
            lsp_car();

            // Evaluate it.
            lsp_eval();

            // Save the result.
            lsp_swp(-2);

            // Move to the next item in the list.
            lsp_cdr();
        }

        // Move the function from the bottom of the stack to the top and call
        // it.
        lsp_dup(2);
        lsp_call(length - 1);
    } else {
        // Expression is a literal that does not need to be evaluated.  Discard
        // the environment and return the literal as is.
        lsp_pop();
    }
}


void lsp_eval(void) {
    lsp_fp_t rp = lsp_get_fp();
    lsp_shrink_frame(2);

    lsp_eval_inner();

    lsp_restore_fp(rp);
}
