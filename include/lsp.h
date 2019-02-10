#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef void (* lsp_op_t)(void);


/**
 * Stack operations
 * ----------------
 *
 * Operations on the stack usually involve offset.
 * Offsets less than zero are relative to the stack pointer.  The offset of the
 * value at the top of the heap is -1.
 *
 * Offsets greater than or equal to zero are relative to the frame pointer.
 * The offset of the first reference in the current frame is 0.
 */

/**
 * Copies the reference at `offset` to the top of the reference stack.
 *
 * `lsp_dup(-1)` will duplicate the value at the top of the stack.
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
void lsp_pop();  // helper

/**
 * Pops all references up to and including the value at `offset` from the top
 * of the stack.
 *
 * Will abort if `offset` is not in bounds.
 *
 * It is not safe to call `lsp_pop_to` while holding raw pointers to objects
 * stored on the heap.
 */
void lsp_pop_to(int offset);

/**
 * Swaps the reference at `offset` with the reference at the top of the stack.
 *
 * Will abort if the stack is empty.
 * Will abort if `offset` is not in bounds.
 */
void lsp_swp(int offset);  // helper

void lsp_enter_frame(int nargs);
void lsp_exit_frame(int nret);


/**
 * Null.
 */
void lsp_push_null();
bool lsp_is_null();


/**
 * Builtin function operations.
 */
void lsp_push_op(void (* value)());
bool lsp_is_op();
lsp_op_t lsp_read_op();

bool lsp_is_truthy();
bool lsp_is_equal();


/**
 * Integers
 * --------
 */
void lsp_push_int(int value);
bool lsp_is_int();
int lsp_read_int();

void lsp_int_add();
void lsp_int_sub();
void lsp_int_mul();
void lsp_int_div();


/**
 * Symbols
 * -------
 */
void lsp_push_symbol(char *value);
bool lsp_is_symbol();
char *lsp_borrow_symbol();
bool lsp_symbol_matches_literal(const char *value);


/**
 * String
 * ------
 */

/**
 * Copies a null terminated string onto the heap and pushes a reference to it
 * onto the stack.
 */
void lsp_push_string(char *value);

/**
 * Pops a reference from the top of the stack, returning true if it points to a
 * string and false otherwise.
 *
 * Arguments:
 *   - value: The value to check for stringiness.
 */
bool lsp_is_string();

/**
 * Returns a temporary reference to a null terminated string stored on the
 * heap.
 *
 * The reference pointing to the string will **not** be popped from the stack.
 *
 * Warning:
 *     Calling any lsp function other than `lsp_borrow_string` is could result
 *     in a garbage collection that would invalidate the reference..
 */
char *lsp_borrow_string();

/**
 * Pops a string and replaces it with an integer who's value is the same as the
 * string's length.
 *
 * Arguments:
 *   - string: The string we would like to find the length for.
 *
 * Returns:
 *   - length: The length of the string not including the terminating null.
 */
void lsp_str_len();

/* Pops a pair of strings from the stack and returns a new one containing the
 * bytes of the first string followed by the bytes of the second.
 */
void lsp_str_concat();

/**
 * Cons cells
 * ----------
 */

/**
 * Pushes a new cons cell on to the top of the stack with both car and cdr set
 * to null.
 */
void lsp_push_cons();

/**
 * Pops a value from the stack, returning true if it is a cons cell and false
 * otherwise.
 *
 * Arguments:
 *   - The value we want to check is a cons cell.
 */
bool lsp_is_cons();

/**
 * Pops two values from the top of the stack and wraps them in a cons cell.
 *
 * Arguments:
 *   - car: the value to save as the first value in the cons cell.
 *   - cdr: the value to save as the second value in the cons cell.
 *
 * Returns:
 *   - cons: the new cons cell.
 */
void lsp_cons();

/**
 * Returns the first value in a cons cell.
 *
 * Arguments:
 *   - cons: A cons cell to extract the first element from.
 *
 * Returns:
 *   - The first value in the cons cell.
 */
void lsp_car();

/**
 * Returns the second value in a cons cell.
 *
 * Arguments:
 *   - cons: A cons cell to extract the second element from.
 *
 * Returns:
 *   - The second value in the cons cell.
 */
void lsp_cdr();

/**
 * Sets value pointed to by the car of an existing cons cell and pops both the
 * replacement value and the cons cell from the stack.
 *
 * Arguments:
 *   - cons: The cons cell to modify.
 *   - value: The value to replace the first value in the cons cell with.
 */
void lsp_set_car();
void lsp_set_cdr();


/**
 * Higher order list operations
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
void lsp_map();
void lsp_fold();
void lsp_reverse();


/**
 * Managing environments for function execution.
 */

/**
 * Creates a new empty scope that inherits from the current environment.
 */
void lsp_push_scope();

/**
 * Creates a new binding in the inner-most scope of the current environment.
 */
void lsp_define();

/**
 * Returns the value bound to a symbol in the current scope.
 *
 * Will abort if no binding exists.
 */
void lsp_lookup();
void lsp_set();
void lsp_push_empty_env();
void lsp_push_default_env();


/**
 * The core of the interpreter.
 */
void lsp_vm_init();

void lsp_parse();

void lsp_call();
void lsp_eval();

void lsp_print();


/**
 * Interpreter information.
 */
size_t lsp_stats_frame_size();
size_t lsp_stats_stack_size();


