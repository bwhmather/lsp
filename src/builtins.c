#include "lsp.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>


/**
 * Integer operations.
 */
void lsp_int_add(void) {
    int a = lsp_read_int(-2);
    int b = lsp_read_int(-1);
    lsp_push_int(a + b);
}

void lsp_int_sub(void) {
    int a = lsp_read_int(-2);
    int b = lsp_read_int(-1);
    lsp_push_int(a - b);
}

void lsp_int_mul(void) {
    int a = lsp_read_int(-2);
    int b = lsp_read_int(-1);
    lsp_push_int(a * b);
}

void lsp_int_div(void) {
    int a = lsp_read_int(-2);
    int b = lsp_read_int(-1);
    lsp_push_int(a / b);
}

void lsp_map(void) {
    lsp_fp_t rp = lsp_get_fp();
    lsp_shrink_frame(2);

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

    lsp_pop_to(1);
    lsp_restore_fp(rp);
}

/**
 * Arguments:
 * - op
 * - init
 * - input
 */
void lsp_fold(void) {
    lsp_fp_t rp = lsp_get_fp();
    lsp_shrink_frame(3);

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

    lsp_pop_to(1);
    lsp_restore_fp(rp);
}


void lsp_reverse(void) {
    lsp_fp_t rp = lsp_get_fp();
    lsp_shrink_frame(1);

    lsp_push_null();
    lsp_swp(1);

    while (!lsp_is_null()) {
        // Replace the output list with a new one starting with the next item
        // in the input list.
        lsp_dup(1);  // Output list
        lsp_dup(1);  // Input list
        lsp_car();
        lsp_cons();
        lsp_store(2);

        // Pop the value from the input list.
        lsp_cdr();
    }

    lsp_pop();
    lsp_restore_fp(rp);
}


void lsp_print(void) {
    lsp_fp_t rp = lsp_get_fp();
    lsp_shrink_frame(1);

    if (lsp_is_null()) {
        printf("()");

        lsp_pop();

    } else if (lsp_is_cons()) {
        printf("(");

        // Print the first element.
        lsp_dup(0);
        lsp_car();
        lsp_print();

        lsp_cdr();

        while (lsp_is_cons()) {
            printf(" ");

            // Print the next element in the array.
            lsp_dup(0);
            lsp_car();
            lsp_print();

            // Move to the next element.
            lsp_cdr();
        }

        // If the list is terminated with something other than null, print it
        // after printing a dot.
        if (!lsp_is_null()) {
            printf(" . ");
            lsp_print();
        } else {
            lsp_pop();
        }

        printf(")");

    } else if (lsp_is_int()) {
        int value = lsp_read_int(0);
        printf("%i", value);

        lsp_pop();

    } else if (lsp_is_symbol()) {
        char *str = lsp_borrow_symbol(0);
        printf("%s", str);

        lsp_pop();

    } else if (lsp_is_string()) {
        char *str = lsp_borrow_string(0);
        printf("\"%s\"", str);  // TODO escape

        lsp_pop();

    } else if (lsp_is_op()) {
        printf("<builtin>");

        lsp_pop();

    } else {
        assert(false);
    }

    assert(lsp_stats_frame_size() == 0);
    lsp_restore_fp(rp);
}

