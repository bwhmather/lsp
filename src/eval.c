#include "eval.h"

#include "vm.h"
#include "env.h"

#include <string.h>
#include <assert.h>

/**
 * Helper function that expands the first n elements of a list followed by the
 * tail onto the stack.
 */
static void lsp_unpack(int n) {
    for (int i=0; i < n; i++) {
        // Read the current head of the list.
        lsp_dup(-1);
        lsp_car();
        // Save it.
        lsp_swp(-2);
        // Advance the head.
        lsp_cdr();
    }
}

/**
 * Calls the function at the top of the stack with the next `nargs` items as
 * its arguments.
 */
void lsp_call(int nargs) {
    if (lsp_dup(-1), lsp_is_op()) {
        // Pop the operation from the top of the stack.
        lsp_op_t op = lsp_read_op();

        // Evaluate it and then pop everything but the return value.
        lsp_enter_frame(nargs);
        op();
        lsp_exit_frame(1);
        return;
    }

    if (lsp_dup(-1), lsp_is_cons()) {
        assert(false);
        // // Break the callable up into its component pieces.
        // lsp_expr_t *function = lsp_car(callable);
        // lsp_expr_t *arg_spec = lsp_car(function);
        // lsp_expr_t *body = lsp_cdr(function);
        // lsp_expr_t *closure = lsp_cdr(callable);

        // // Bind function arguments to a new environment.
        // lsp_expr_t *function_env = lsp_push_scope(closure);
        // while (lsp_type(arg_spec) == LSP_CONS) {
        //     lsp_define(
        //         lsp_as_sym(lsp_car(arg_spec)),
        //         lsp_car(args),
        //         function_env
        //     );
        //     arg_spec = lsp_cdr(arg_spec);
        //     args = lsp_cdr(args);
        // }

        // // Evaluate the function.
        // lsp_expr_t * result = lsp_eval(body, function_env);
    }

    assert(false);
}


/**
 * Evaluates the expression passed as its second argument in the environment
 * passed as its first.
 */
void lsp_op_eval() {
    if (lsp_dup(-1), lsp_is_symbol()) {
        // Expression is a name identifying a variable that can be loaded
        // from the environment.  `lookup` will po the environment and symbol
        // from the stack and replace them with the correct value.
        lsp_lookup();
        return;
    } else if (lsp_dup(-1), lsp_is_cons()) {
        // Expression is a list representing either a special form or an
        // invocation of a procedure or built-in operator.

        // Unpack the first element from the list to check if it is a symbol.
        lsp_dup(-1);
        lsp_car();
        if (lsp_is_symbol()) {
            // The first item in the list is a symbol.  We first check if it
            // represents a special form and if that doesn't work fall through
            // to evaluating as an expression.

            lsp_dup(-1);
            lsp_car();
            char *sym = lsp_read_sym();
            if (strcmp(sym, "if") == 0) {
                // Strip the `if`.
                lsp_cdr();

                // Unpack the predicate, subsequent and alternate expressions.
                lsp_unpack(3);

                // Pop the tail of the expression and check that it contains no
                // further elements.
                assert(lsp_is_null());

                // Evaluate and check the predicate.
                lsp_dup(0);  // The environment.
                lsp_dup(2);  // The predicate expression.
                lsp_eval();

                if (lsp_is_truthy()) {
                    lsp_dup(0);  // The environment.
                    lsp_dup(3);  // The subsequent.
                    lsp_eval();
                } else {
                    lsp_dup(0);  // The environment.
                    lsp_dup(3);  // The alternate.
                    lsp_eval();
                }

                lsp_store(0);
                lsp_pop_to(1);
                return;
            }
            if (strcmp(sym, "quote") == 0) {
                // Strip the `quote`.
                lsp_cdr();

                // Unpack the expression.
                lsp_unpack(1);

                // Pop the tail of the expression and check that it contains no
                // further elements.
                assert(lsp_is_null());

                // Return the quoted expression.
                lsp_store(0);
                lsp_pop_to(1);
                return;
            }
            if (strcmp(sym, "define") == 0) {
                // Strip the `define`.
                lsp_cdr();

                // Unpack the key and value.
                lsp_unpack(2);

                // Pop the tail of the expression and check that it contains no
                // further elements.
                assert(lsp_is_null());

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
                // Strip the `define`.
                lsp_cdr();

                // Unpack the key and value.
                lsp_unpack(2);

                // Pop the tail of the expression and check that it contains no
                // further elements.
                assert(lsp_is_null());

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
                // Strip the `define`.
                lsp_cdr();

                // Unpack the argument list and body..
                lsp_unpack(2);

                // Pop the tail of the expression and check that it contains no
                // further elements.
                assert(lsp_is_null());

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
                // Strip the `begin`.
                lsp_cdr();

                // Set a default result in case there are no expressions.
                lsp_push_null();

                while (true) {
                    // Check that we haven't reached the end.
                    lsp_dup(2);
                    if (lsp_is_null()) {
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
        }

        int length = 0;

        // Evaluate the whole list of expressions and unpack it to the stack.
        while (lsp_dup(-1), !lsp_is_null()) {
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
        // the environment and return it as is.
        lsp_swp(-1);
        lsp_pop();
    }
}


void lsp_eval() {
    lsp_push_op(lsp_op_eval);
    lsp_call(2);
}


