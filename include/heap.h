#pragma once

#include <stdbool.h>

typedef enum lsp_type_t {
    LSP_NULL = 0,
    LSP_CONS,
    LSP_INT,
    LSP_SYM,
    LSP_OP,
} lsp_type_t;

typedef void lsp_value_t;

typedef lsp_value_t *(* lsp_op_t)(lsp_value_t *);

void lsp_heap_init();

lsp_type_t lsp_heap_type(lsp_value_t *expr);

void lsp_assert_type(lsp_value_t *expr, lsp_type_t type);

/**
 * Functions for working with integer objects.
 */
lsp_value_t *lsp_heap_int(int value);
int *lsp_heap_as_int(lsp_value_t *expr);


/**
 * Functions for working with symbols.
 */
lsp_value_t *lsp_symbol_start();
void lsp_heap_symbol_push(char character);
void lsp_heap_symbol_stop();

lsp_value_t *lsp_heap_symbol(char *name);

char *lsp_heap_as_sym(lsp_value_t *expr);


/**
 * Functions for working with cons cells.
 */
lsp_value_t *lsp_heap_cons(lsp_value_t *car, lsp_value_t *cdr);

lsp_value_t *lsp_heap_car(lsp_value_t *expr);
lsp_value_t *lsp_heap_cdr(lsp_value_t *expr);

void lsp_heap_set_car(lsp_value_t *expr, lsp_value_t *new_car);
void lsp_heap_set_cdr(lsp_value_t *expr, lsp_value_t *new_cdr);


/**
 * Functions for working with expressions representing built-in operations.
 */
lsp_value_t *lsp_heap_op(lsp_op_t op);
lsp_op_t *lsp_heap_as_op(lsp_value_t *expr);


/**
 * Miscellaneous helpers.
 */
bool lsp_heap_is_truthy(lsp_value_t *expr);

void lsp_heap_print(lsp_value_t *expr);

