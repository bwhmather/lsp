#include "lsp.h"

#include <string.h>
#include <assert.h>


void lsp_op_eval_lambda(void) {
    // Push new scope onto closure.
    // Bind arguments to local variables.
    // Call `lsp_eval`.
    //
    // Arguments:
    //   - 0: (args body)
    //   - 1: env
    //   - ...: arguments
    //
    // After unpacking
    //   - args
    //   - body
    //   - env
    //   - ...
    //
    // While reading args
    //   - env
    //   - symbol
    //   - value
    //   - args
    //   - body
    //   - env
    //   - ...
    int nargs = lsp_stats_frame_size() - 2;

    // Unpack body and argument list.
    lsp_dup(0);
    lsp_cdr();
    lsp_swp(1);
    lsp_car();

    // Push an empty scope onto the closure and then save it back in its
    // original place on the stack.
    lsp_dup(2);
    lsp_push_scope();
    lsp_store(3);

    // Bind arguments to local variables.
    for (int i = 0; i < nargs; i++) {
        // Push next argument to the top of the stack.
        lsp_dup(i + 3);

        // Read next argument name.
        lsp_dup(1);
        lsp_cdr();  // Could fail if too many arguments.
        lsp_swp(2);
        lsp_car();

        // Bind value to argument name.
        lsp_dup(4);
        lsp_define();
    }

    // Save the body and environment to the bottom of the stack then pop
    // everything else.  We do a bit of a dance in case there was only one
    // argument.
    lsp_pop();
    lsp_dup(1);
    lsp_dup(1);
    lsp_store(-2);
    lsp_store(-1);

    // Remaining arguments.
    while (lsp_stats_frame_size() > 2) {
        lsp_pop();
    }

    lsp_push_null();
    while (!lsp_is_null(1)) {
        lsp_pop();
        lsp_dup(0);
        lsp_car();
        lsp_dup(2);
        lsp_eval();
        lsp_dup(1);
        lsp_cdr();
        lsp_store(2);
    }

    lsp_store(2);
    lsp_pop();
}


void lsp_call_inner(void) {
    // Expand the callable until the top of the stack contains an op.
    while (!lsp_is_op(0)) {
        lsp_dup(0);
        lsp_cdr();
        lsp_swp(1);
        lsp_car();
    }

    // Call the op.
    lsp_op_t op = lsp_read_op(0);
    lsp_pop();
    op();
}


void lsp_call(int nargs) {
    lsp_fp_t rp = lsp_get_fp();
    lsp_shrink_frame(nargs + 1);

    lsp_call_inner();

    lsp_restore_fp(rp);
}


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
                // Pop the `quote` and the environment from the top of the
                // stack.
                lsp_pop();
                lsp_pop();

                // Strip the `quote` from the expression, and split the rest of
                // the list into a head and tail.
                lsp_cdr();
                lsp_unpack(1);

                // Pop the tail of the expression and check that it contains no
                // further elements.
                assert(lsp_is_null(0));
                lsp_pop();

                // Return the quoted expression.
                return;
            }
            if (strcmp(sym, "define") == 0) {
                // Strip the `define` from the top of the stack
                lsp_pop();

                // Drop the `define`, and unpack the symbol and the value onto
                // the stack.
                lsp_dup(1);
                lsp_cdr();
                lsp_unpack(2);

                // Pop the tail of the expression and check that it contains no
                // further elements.
                assert(lsp_is_null(0));
                lsp_pop();

                // Rearrange the stack so that the environment is at the top,
                // followed by the symbol and then the value.
                lsp_store(3);
                lsp_swp(1);

                // Bind the evaluated value to the key.
                lsp_define();

                // Return NULL.
                lsp_push_null();
                return;
            }
            if (strcmp(sym, "set!") == 0) {
                // Strip the `set!` from the top of the stack
                lsp_pop();

                // Drop the `set!` from the current expression and unpack the
                // symbol and the value onto the stack.
                lsp_dup(1);
                lsp_cdr();
                lsp_unpack(2);

                // Pop the tail of the expression and check that it contains no
                // further elements.
                assert(lsp_is_null(0));
                lsp_pop();

                // Rearrange the stack so that the environment is at the top,
                // followed by the symbol and then the value.
                lsp_store(3);
                lsp_swp(1);

                // Re-bind the evaluated value to the key.
                lsp_set();

                // Return NULL.
                lsp_push_null();
                return;
            }
            if (strcmp(sym, "lambda") == 0) {
                // Strip the `lambda` from the top of the stack.
                lsp_pop();

                // Swap the environment and expression.
                lsp_swp(1);

                // Drop the `lambda`, and unpack the arguments and body onto
                // the stack.
                lsp_cdr();

                // TODO Find free variables.
                // TODO Capture free variables from the environment.

                // Bind the function description closure.
                lsp_push_op(lsp_op_eval_lambda);
                lsp_cons();

                // Bind the environment to create the runtime closure.
                lsp_cons();
                return;
            }
            if (strcmp(sym, "begin") == 0) {
                // Strip the `begin` from the top of the stack and the
                // beginning of the current expression.
                lsp_pop();
                lsp_swp(1);
                lsp_cdr();

                lsp_push_null();
                while (!lsp_is_null(1)) {
                    lsp_pop();
                    lsp_dup(0);
                    lsp_car();
                    lsp_dup(2);
                    lsp_eval();
                    lsp_dup(1);
                    lsp_cdr();
                    lsp_store(2);
                }

                lsp_store(2);
                lsp_pop();

                return;
            }
        }
        // Strip the unrecognised symbol from the top of the stack.
        lsp_pop();

        // Evaluate each expression in the list, starting from the callable.
        int length = 0;
        while (!lsp_is_null(-1)) {
            length += 1;

            // Read the next item in the list.
            lsp_dup(-1);
            lsp_car();

            // Remove it from the list.
            lsp_dup(-1);
            lsp_cdr();
            lsp_store(-1);

            // Evaluate it in the current environment.
            lsp_dup(-2);
            lsp_eval();
        }

        // Reverse the results on the stack, including the env and the empty
        // expression list.
        for (int i = 0; i < (length + 2) / 2; i++) {
            lsp_dup(-1 - i);
            lsp_swp(i + 1);
            lsp_store(-1 - i);
        }

        // Pop the empty expression list and the env.
        lsp_pop();
        lsp_pop();

        lsp_call_inner();
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
