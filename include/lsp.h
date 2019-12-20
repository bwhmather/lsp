#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef void (* lsp_op_t)(void);
typedef int lsp_fp_t;


/**
 * Stack operations
 * ================
 * Functions for manipulating the reference stack.
 */

/**
 * Copies the reference at `offset` to the top of the reference stack.
 *
 * `lsp_dup(0)` will duplicate the value at the top of the stack.
 *
 * Will abort if `offset` is not in bounds.
 */
void lsp_dup(int offset);

/**
 * Pops the reference at the top of the stack and saves it at offset.
 *
 * Will abort if the stack is empty.
 * Will abort if `offset` is not in bounds.
 *
 * It is not safe to call `lsp_store` while holding raw pointers to objects
 * stored on the heap.
 */
void lsp_store(int offset);

/**
 * Pops and discards the reference at the top of the stack.
 *
 * Will abort if the stack is empty.
 * Will abort if `offset` is not in bounds.
 *
 * It is not safe to call `lsp_pop` while holding raw pointers to objects
 * stored on the heap.
 */
void lsp_pop(void);

/**
 * Swaps the reference at `offset` with the reference at the top of the stack.
 *
 * Will abort if the stack is empty.
 * Will abort if `offset` is not in bounds.
 */
void lsp_swp(int offset);

/**
 * Returns an opaque copy of the current value of the frame pointer that can
 * be used to restore it at a later stage.
 */
lsp_fp_t lsp_get_fp(void);

/**
 * Advances the frame pointer so that the current frame only contains the
 * requested number of arguments.
 *
 * Callers are responsible for restoring the frame pointer to its original
 * value.  This should be done using `lsp_get_fp` and `lsp_restore_fp` instead
 * of attempting to calculate a value to pass to `lsp_shrink_frame`.
 *
 * Will abort if passed a value larger than the number of references in the
 * current frame.
 * Will abort if passed a value less than zero.
 */
void lsp_shrink_frame(int nargs);

/**
 * Restores the frame pointer to a previous value.
 *
 * This must be less than or equal to the current stack pointer.
 * Will abort if the stack has been popped too far.
 *
 * It is not required that the frame pointer be less than or equal to the
 * current frame pointer, but doing this suggests a mistake.
 */
void lsp_restore_fp(lsp_fp_t fp);

/**
 * Heap operations
 * ===============
 */

/**
 * Null
 * ----
 */
void lsp_push_null(void);
bool lsp_is_null(int offset);


/**
 * Foreign Function Interface
 * --------------------------
 */
void lsp_push_op(void (* value)(void));
bool lsp_is_op(int offset);
lsp_op_t lsp_read_op(int offset);

bool lsp_is_truthy(void);
bool lsp_is_equal(void);


/**
 * Integers
 * --------
 */
void lsp_push_int(int value);
bool lsp_is_int(int offset);
int lsp_read_int(int offset);

void lsp_int_add(void);
void lsp_int_sub(void);
void lsp_int_mul(void);
void lsp_int_div(void);

/**
 * Symbols
 * -------
 */
void lsp_push_symbol(char const *value);
bool lsp_is_symbol(int offset);
char const *lsp_borrow_symbol(int offset);
bool lsp_symbol_matches_literal(char const *value);


/**
 * Strings
 * -------
 */

/**
 * Copies a null terminated string onto the heap and pushes a reference to it
 * onto the stack.
 */
void lsp_push_string(char const *value);

/**
 * Returns true if the reference at the given stack offset points to a string.
 *
 * Does not modify the heap or the stack.
 */
bool lsp_is_string(int offset);

/**
 * Returns a temporary reference to a null terminated string stored on the
 * heap.
 *
 * The reference pointing to the string will **not** be popped from the stack.
 *
 * Warning:
 *     Calling any mutating lsp function could trigger a garbage collection
 *     that would invalidate the reference.
 */
char const *lsp_borrow_string(int offset);


/**
 * Cons cells
 * ----------
 */

/**
 * Pushes a new cons cell on to the top of the stack with both car and cdr set
 * to null.
 *
 * Returns:
 *   - A new cons cell.
 */
void lsp_push_cons(void);

/**
 * Returns true if the ref at offset points to a cons cell, false otherwise.
 */
bool lsp_is_cons(int offset);

/**
 * Pops two values from the top of the stack and wraps them in a cons cell.
 *
 * Arguments:
 *   - car: The value to save as the first value in the cons cell.
 *   - cdr: The value to save as the second value in the cons cell.
 *
 * Returns:
 *   - The new cons cell.
 */
void lsp_cons(void);

/**
 * Returns the first value in a cons cell.
 *
 * Arguments:
 *   - cons: A cons cell to extract the first element from.
 *
 * Returns:
 *   - The first value in the cons cell.
 */
void lsp_car(void);

/**
 * Returns the second value in a cons cell.
 *
 * Arguments:
 *   - cons: A cons cell to extract the second element from.
 *
 * Returns:
 *   - The second value in the cons cell.
 */
void lsp_cdr(void);

/**
 * Sets value pointed to by the car of an existing cons cell and pops both the
 * replacement value and the cons cell from the stack.
 *
 * Arguments:
 *   - cons: The cons cell to modify.
 *   - value: The value to replace the first value in the cons cell with.
 */
void lsp_set_car(void);
void lsp_set_cdr(void);


/**
 * Higher Level Helper Functions
 * =============================
 * Utility functions implemented using the public API for performing common
 * tasks.
 */

/**
 * List Operations
 * ---------------
 * Higher order functions for operating on lists constructed from cons cells.
 */

/**
 * Applies a callable object to each item in a list and returns a new list
 * containing the results.
 *
 * Arguments:
 *   - fn: The operation to apply.
 *   - list: The list of values to apply the operation to.
 *
 * Returns:
 *   - A new list of the return values..
 */
void lsp_map(void);
void lsp_fold(void);
void lsp_reverse(void);


/**
 * Environments
 * ------------
 * Functions for adding and reading bindings from scopes.
 */

/**
 * Creates a new empty scope that inherits from the current environment.
 *
 * Arguments:
 *   - parent: The scope to inherit from.
 *
 * Returns:
 *   - A new scope datastructure.
 */
void lsp_push_scope(void);

/**
 * Creates a new binding in the inner-most scope of the current environment.
 */
void lsp_define(void);

/**
 * Returns the value bound to a symbol in the current scope.
 *
 * Will abort if no binding exists.
 */
void lsp_lookup(void);
void lsp_set(void);
void lsp_push_empty_env(void);
void lsp_push_default_env(void);
void lsp_capture(void);

/**
 * Core Operations
 * ===============
 * The core of the interpreter.
 */
void lsp_abort(void);

void lsp_vm_init(void);

void lsp_parse(void);

void lsp_call(int nargs);
void lsp_eval(void);

void lsp_print(void);
void lsp_print_stack(void);

/**
 * Interpreter information.
 */
size_t lsp_stats_frame_size(void);
size_t lsp_stats_stack_size(void);



