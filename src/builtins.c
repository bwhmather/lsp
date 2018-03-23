#include "builtins.h"

#include "vm.h"
#include "interpreter.h"
#include "env.h"

#include <stdlib.h>
#include <assert.h>


static void lsp_op_add() {
    int a = lsp_read_int(0);
    int b = lsp_read_int(1);
    lsp_pop_to(0);
    lsp_push_int(a + b);
}

static void lsp_op_sub() {
    int a = lsp_read_int(0);
    int b = lsp_read_int(1);
    lsp_pop_to(0);
    lsp_push_int(a - b);
}

static void lsp_op_mul() {
    int a = lsp_read_int(0);
    int b = lsp_read_int(1);
    lsp_pop_to(0);
    lsp_push_int(a * b);
}

static void lsp_op_div() {
    int a = lsp_read_int(0);
    int b = lsp_read_int(1);
    lsp_pop_to(0);
    lsp_push_int(a / b);
}

static void lsp_op_cons() {
    lsp_cons();
}

static void lsp_op_car() {
    lsp_car();
}

static void lsp_op_set_car() {
    lsp_set_car();
}

static void lsp_op_cdr() {
    lsp_cdr();
}

static void lsp_op_set_cdr() {
    lsp_set_cdr();
}

void lsp_op_map() {
    // A cons cell used to track the building of the list.  The cdr points to
    // the root of the list.  The car points to the cons that should be
    // appended to next.  It will initially be set to the tracking cons cell
    // but will be updated to point to the end of the list.
    lsp_push_cons();
    lsp_dup(-1);
    lsp_set_car();

    while (lsp_dup(1), !lsp_is_null()) {
        // Extract the next value in the input list.
        lsp_dup(1);
        lsp_car();

        // Call the function on it.
        lsp_dup(0);
        lsp_call(1);

        // Save the result in a new cons cell.
        lsp_push_null();
        lsp_cons();

        // Append the new cons cell to the end of the output list.
        lsp_dup(2);
        lsp_swp(3);
        lsp_set_cdr();

        // Advance to the next cell in the input list.
        lsp_dup(1);
        lsp_cdr();
        lsp_store(1);
    }

    // Move the tracking cell to the bottom of the stack.
    lsp_store(0);
    // Get rid of the input list.
    lsp_pop();

    // Return the output list.
    lsp_cdr();
}

/**
 * Arguments:
 * - op
 * - init
 * - input
 */
lsp_op_fold() {
    while (lsp_dup(2), !lsp_is_null()) {
        // Read the next item in the list.
        lsp_dup(2);
        lsp_car();

        // Copy the accumulator.
        lsp_dup(1);

        // Call the operation on the list value and the previous value of the
        // accumulator.
        lsp_dup(0);
        lsp_call(2);

        // Save the result.
        lsp_store(1);

        // Advance to the next item in the list cell.
        lsp_cdr();
    }

    // Pop the tail of the list from the stack.
    lsp_pop();

    // Return the accumulator as the result.
    lsp_store(0);
}


static void lsp_bind(char *symbol, lsp_op_t operation) {
    lsp_dup(-1);
    lsp_push_symbol(symbol);
    lsp_push_op(operation);
    lsp_define();
}

void lsp_push_default_env() {
    lsp_empty_env();

    lsp_bind("+", &lsp_op_add);
    lsp_bind("-", &lsp_op_sub);
    lsp_bind("*", &lsp_op_mul);
    lsp_bind("/", &lsp_op_div);
    lsp_bind("cons", &lsp_op_cons);
    lsp_bind("car", &lsp_op_car);
    lsp_bind("set-car!", &lsp_op_set_car);
    lsp_bind("cdr", &lsp_op_cdr);
    lsp_bind("set-cdr!", &lsp_op_set_cdr);
    lsp_bind("map", &lsp_op_map);
    lsp_bind("fold", &lsp_op_fold);
}
