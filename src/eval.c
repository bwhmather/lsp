#include "eval.h"

#include "stack.h"
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


void lsp_op_eval() {
    int env = 0;
    int expr = 1;

    if (lsp_dup(-1), lsp_is_symbol()) {
        // Expression is a name identifying a variable that can be loaded
        // from the environment.  `lookup` will po the environment and symbol
        // from the stack and replace them with the correct value.
        lsp_lookup();
        return;
    } else if (lsp_dup(-1), lsp_is_cons()) {
        // Expression is a list representing either a special form or an
        // invocation of a procedure or built-in operator.
        if (lsp_dup(-1), lsp_is_symbol()) {
            char *sym = lsp_as_sym(lsp_car(expr));
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

        // Expression is not a special form.  We evaluate all items in the
        // list and then pass the tail to the built-in or procedure
        // represented by the first item.
        lsp_expr_t *evaled = NULL;
        lsp_expr_t *cursor = expr;
        while (lsp_type(cursor) == LSP_CONS) {
            evaled = lsp_cons(lsp_eval(lsp_car(cursor), env), evaled);
            cursor = lsp_cdr(cursor);
        }
        evaled = lsp_reverse(evaled);

        lsp_expr_t *callable = lsp_car(evaled);
        lsp_expr_t *args = lsp_cdr(evaled);

        if (lsp_type(callable) == LSP_OP) {
            // Expression is a call to a built-in procedure represented by a
            // function pointer.
            lsp_op_t op = *lsp_as_op(callable);
            return op(args);
        } else if (lsp_type(callable) == LSP_CONS) {
            // Break the callable up into its component pieces.
            lsp_expr_t *function = lsp_car(callable);
            lsp_expr_t *arg_spec = lsp_car(function);
            lsp_expr_t *body = lsp_cdr(function);
            lsp_expr_t *closure = lsp_cdr(callable);

            // Bind function arguments to a new environment.
            lsp_expr_t *function_env = lsp_push_scope(closure);
            while (lsp_type(arg_spec) == LSP_CONS) {
                lsp_define(
                    lsp_as_sym(lsp_car(arg_spec)),
                    lsp_car(args),
                    function_env
                );
                arg_spec = lsp_cdr(arg_spec);
                args = lsp_cdr(args);
            }

            // Evaluate the function.
            lsp_expr_t * result = lsp_eval(body, function_env);

            return result;
        } else {
            assert(false);
        }

    } else {
        // Expression is a literal that can be returned as-is.
        return expr;
    }
}


void lsp_eval() {
    lsp_push_op(lsp_op_eval);
    lsp_call(2);
}
