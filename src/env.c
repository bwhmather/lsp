#include "lsp.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>


void lsp_push_empty_env(void) {
    lsp_push_null();
    lsp_push_null();
    lsp_cons();
}

/**
 * :param cons env:
 *     The environment to wrap in a new scope.
 */
void lsp_push_scope(void) {
    // Create an empty list of bindings.
    lsp_push_null();

    // Store it as the first entry in a pair containing a list of bindings as
    // its car, and a parent scope as its cdr.
    lsp_cons();
}


/**
 * Arguments:
 *   - environment
 *   - symbol
 *   - value
 */
void lsp_define(void) {
    lsp_fp_t rp = lsp_get_fp();
    lsp_shrink_frame(3);

    // Extract the current locals from the environment.
    lsp_dup(-3);
    lsp_car();

    // Wrap the symbol and value in a new cons cell.
    lsp_dup(-1);
    lsp_dup(-2);
    lsp_cons();

    // Add the binding to the list of locals.
    lsp_cons();

    // Replace the list of locals stored in the environment with the new list.
    lsp_swp(1);
    lsp_set_car();

    // Environment has been mutated in place, so don't return.
    lsp_pop();
    lsp_pop();

    lsp_restore_fp(rp);
}


/**
 * Arguments:
 *   - environment
 *   - symbol
 */
void lsp_lookup(void) {
    lsp_fp_t rp = lsp_get_fp();
    lsp_shrink_frame(2);

    // Check that the current environment is not NULL.
    if (lsp_is_null(0)) {
        assert(false);
        // lsp_abort("undefined variable");
    }

    // Extract the list of local bindings and save at the top of the stack.
    lsp_dup(0);
    lsp_car();

    // Search the list for the symbol.
    while (lsp_is_cons(0)) {
        assert(lsp_stats_frame_size() == 3);

        // Read the symbol from the first entry.
        lsp_dup(0);
        lsp_car();  // The first binding in the list.
        lsp_car();  // The key for the binding.

        // Compare it to the symbol we are interested in.
        char const *target = lsp_borrow_symbol(-1);
        char const *current = lsp_borrow_symbol(0);
        int cmp_result = strcmp(current, target);
        lsp_pop();

        if (cmp_result == 0) {
            // If equal then we have found what we are looking for.  Extract
            // the value from the corresponding entry in the scope and return
            // it.
            lsp_car();  // The first binding in the list.
            lsp_cdr();  // The value from the binding.

            lsp_store(-1);
            lsp_pop();

            lsp_restore_fp(rp);

            return;
        }

        // Advance to the next entry in the list.
        lsp_cdr();
    }

    // Remove the empty inner scope from the stack.
    lsp_pop();

    // Replace the current environment with the parent environment.
    lsp_cdr();

    // Search for the symbol in the parent environment.
    lsp_lookup();

    lsp_restore_fp(rp);
}

/**
 * Arguments:
 *   - environment
 *   - symbol
 *   - value
 */
void lsp_set(void) {
    lsp_fp_t rp = lsp_get_fp();
    lsp_shrink_frame(3);

    // Check that the current environment is not NULL.
    if (lsp_is_null(0)) {
        assert(false);
        // lsp_abort("undefined variable");
    }

    // Extract the list of local bindings and save at the top of the stack.
    lsp_dup(0);
    lsp_car();

    // Search the list for the symbol.
    while (lsp_is_cons(0)) {
        assert(lsp_stats_frame_size() == 4);

        // Read the symbol from the first entry.
        lsp_dup(0);
        lsp_car();  // The first binding in the list.
        lsp_car();  // The key for the binding.

        // Compare it to the symbol we are interested in.
        char const *target = lsp_borrow_symbol(-2);
        char const *current = lsp_borrow_symbol(0);
        int cmp_result = strcmp(current, target);
        lsp_pop();

        if (cmp_result == 0) {
            // Load a reference to the binding and replace its cdr with the new
            // value.
            lsp_car();
            lsp_store(2);
            lsp_pop();

            lsp_set_cdr();

            lsp_restore_fp(rp);

            return;
        }

        // Advance to the next entry in the list.
        lsp_cdr();
    }

    // Remove the empty inner scope from the stack.
    lsp_pop();

    // Replace the current environment with the parent environment.
    lsp_cdr();

    // Search for the symbol in the parent environment.
    lsp_lookup();

    lsp_restore_fp(rp);
}

/**
 * Extracts the innermost binding for each of the requested variables into the
 * inner scope of a new env.
 *
 * Arguments:
 *   - A list of symbols to capture.
 *   - The environment to capture them from.
 */
void lsp_capture(void) {
    abort();
}


static void lsp_bind(char *symbol, lsp_op_t operation) {
    lsp_push_op(operation);
    lsp_push_symbol(symbol);
    lsp_dup(2);
    lsp_define();
}


void lsp_push_default_env(void) {
    lsp_push_empty_env();

    lsp_bind("+", &lsp_int_add);
    lsp_bind("-", &lsp_int_sub);
    lsp_bind("*", &lsp_int_mul);
    lsp_bind("/", &lsp_int_div);
    lsp_bind("cons", &lsp_cons);
    lsp_bind("car", &lsp_car);
    lsp_bind("set-car!", &lsp_set_car);
    lsp_bind("cdr", &lsp_cdr);
    lsp_bind("set-cdr!", &lsp_set_cdr);
    lsp_bind("map", &lsp_map);
    lsp_bind("fold", &lsp_fold);
}

