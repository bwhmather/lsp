#include "env.h"

#include "vm.h"
#include "interpreter.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>


void lsp_empty_env() {
    lsp_push_null();
    lsp_push_null();
    lsp_cons();
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
    lsp_swp(-1);
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
    lsp_dup(0);
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
    lsp_dup(0);
    if (lsp_is_null()) {
        lsp_abort("undefined variable");
    }

    // Extract the list of local bindings and save at the top of the stack.
    lsp_dup(0);
    lsp_car();

    // Search the list for the symbol.
    while (lsp_dup(-1), lsp_is_cons()) {
        // Read the symbol from the first entry.
        lsp_dup(-1);
        lsp_car();  // The first binding in the list.
        lsp_car();  // The key for the binding.

        // Copy the target symbol.
        lsp_dup(1);

        // Compare it to the symbol we are interested in.
        char *target = lsp_read_symbol();
        char *current = lsp_read_symbol();

        if (strcmp(current, target) == 0) {
            // If equal then we have found what we are looking for.  Extract
            // the value from the corresponding entry in the scope and return
            // it.
            lsp_car();  // The first binding in the list.
            lsp_cdr();  // The value from the binding.

            lsp_store(0);
            lsp_pop_to(1);
            return;
        }

        // Advance to the next entry in the list.
        lsp_cdr();
    }

    // Remove the empty inner scope from the stack.
    lsp_pop_to(2);

    // Replace the current environment with the parent environment.
    lsp_dup(0);
    lsp_cdr();
    lsp_store(0);

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
    lsp_dup(0);
    if (lsp_is_null()) {
        lsp_abort("undefined variable");
    }

    // Extract the list of local bindings and save it at the top of the stack.
    lsp_dup(0);
    lsp_car();

    // Search the list for the symbol we want o edit.
    while (lsp_dup(-1), lsp_is_cons()) {
        // Read the symbol from the first entry.
        lsp_dup(-1);
        lsp_car();  // The first binding in the list.
        lsp_car();  // The key for the binding.

        // Copy the target symbol.
        lsp_dup(1);

        // Compare it to the symbol we are interested in.
        char *target = lsp_read_symbol();
        char *current = lsp_read_symbol();

        if (strcmp(current, target) == 0) {
            // Load a reference to the binding and replace its cdr with the new
            // value.
            lsp_car();
            lsp_dup(2);

            // Return null.
            lsp_pop_to(0);
            lsp_push_null();
            return;
        }

        // Advance to the next entry in the list.
        lsp_cdr();
    }


    // Remove the empty inner scope from the stack.
    lsp_pop_to(3);

    // Replace the current environment with the parent environment.
    lsp_dup(0);
    lsp_cdr();
    lsp_store(0);

    // Search for the symbol in the parent environment.
    lsp_set();
}


void lsp_set() {
    lsp_push_op(lsp_op_set);
    lsp_call(3);
}
