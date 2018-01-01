#include "env.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>


void lsp_empty_env() {
    lsp_push_null();
    lsp_push_null();
    lsp_cons()
}

/**
 * :param cons env:
 *     The environment to wrap in a new scope.
 */
void lsp_op_push_scope() {
    // Create an empty list of bindings.
    lsp_push_null();

    // Store it as the first entry in a pair containing a list of bindings as
    // its car, and a parent scope as its cdr.
    lsp_rotate(0);
    lsp_cons();
}


void lsp_push_scope() {
    lsp_push_op(lsp_op_push_scope);
    lsp_call(1);
}


/**
 * Arguments:
 *   - environment
 *   - symbol
 *   - value
 */
void lsp_op_define() {
    // Wrap the symbol and value at the top of the stack in a cons cell.
    lsp_cons();

    // Read the inner most scope from the environment and store it behind the
    // cons cell for the new entry.
    lsp_push(0);
    lsp_car();

    // Create a new scope with the new binding as its first entry.
    lsp_cons();

    // Replace the old scope with the new one.
    lsp_set_car();
}

void lsp_define() {
    lsp_push_op(lsp_op_define);
    lsp_call(3);
}


/**
 * Arguments:
 *   - environment
 *   - symbol
 */
void lsp_op_lookup() {
    // Check that the current environment is not NULL.
    lsp_push(0);
    if (lsp_is_null()) {
        lsp_abort("undefined variable");
    }

    // Extract the list of local bindings and save at the top of the stack.
    lsp_push(0);
    lsp_car();

    // Search the list for the symbol.
    while (lsp_is_type_nopop(LSP_CONS)) {
        // Read the symbol from the first entry.
        lsp_push(-1);
        lsp_car();
        lsp_car();

        // Compare it to the symbol we are interested in.
        lsp_push(1);
        if (lsp_is_eq()) {
            // If equal then we have found what we are looking for.  Extract
            // the value from the corresponding entry in the scope and return
            // it.
            lsp_car();
            lsp_cdr();
            lsp_replace(0);
            lsp_pop(-1);

            return;
        }
    }

    // Remove the empty inner scope from the stack.
    lsp_pop(1);

    // Replace the current environment with the parent environment.
    lsp_push(0);
    lsp_cdr();
    lsp_replace(0);

    // Search for the symbol in the parent environment.
    lsp_lookup();
}

void lsp_lookup() {
    lsp_push_op(lsp_op_lookup);
    lsp_call(2);
}


/**
 * Arguments:
 *   - environment
 *   - symbol
 *   - value
 */
void lsp_op_set() {
    assert(env != NULL);

    // Try to find symbol in local scope.
    lsp_expr_t *scope = lsp_car(env);

    while (lsp_type(scope) == LSP_CONS) {
        if (strcmp(lsp_as_sym(lsp_car(lsp_car(scope))), sym) == 0) {
            lsp_set_cdr(lsp_car(scope), val);
            return;
        }
        scope = lsp_cdr(scope);
    }

    // Fallback to attempting to set the value in the parent scope.
    lsp_expr_t *parent_scope = lsp_cdr(env);
    lsp_set(sym, val, parent_scope);
}


void lsp_set() {
    lsp_push_op(lsp_op_set);
    lsp_call(3);
}
