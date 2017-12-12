#pragma once

#include <stdbool.h>

typedef enum lsp_type_t {
    LSP_NULL = 0,
    LSP_CONS,
    LSP_INT,
    LSP_SYM,
    LSP_OP,
} lsp_type_t;

typedef void lsp_expr_t;

typedef lsp_expr_t *(* lsp_op_t)(lsp_expr_t *);

void lsp_heap_init();

lsp_type_t lsp_type(lsp_expr_t *expr);


/**
 * Functions for working with integer objects.
 */
lsp_expr_t *lsp_int(int value);
int *lsp_as_int(lsp_expr_t *expr);


/**
 * Functions for working with symbols.
 */
lsp_expr_t *lsp_symbol_start();
void lsp_symbol_push(char character);
void lsp_symbol_stop();

lsp_expr_t *lsp_symbol(char *name);

char *lsp_as_sym(lsp_expr_t *expr);


/**
 * Functions for working with cons cells.
 */
lsp_expr_t *lsp_cons(lsp_expr_t *car, lsp_expr_t *cdr);

lsp_expr_t *lsp_car(lsp_expr_t *expr);
lsp_expr_t *lsp_caar(lsp_expr_t *expr);
lsp_expr_t *lsp_caaar(lsp_expr_t *expr);

lsp_expr_t *lsp_cdr(lsp_expr_t *expr);
lsp_expr_t *lsp_cddr(lsp_expr_t *expr);
lsp_expr_t *lsp_cdddr(lsp_expr_t *expr);

void lsp_set_car(lsp_expr_t *expr, lsp_expr_t *new_car);
void lsp_set_cdr(lsp_expr_t *expr, lsp_expr_t *new_cdr);


/**
 * Functions for working with expressions representing built-in operations.
 */
lsp_expr_t *lsp_op(lsp_op_t op);
lsp_op_t *lsp_as_op(lsp_expr_t *expr);


/**
 * Miscellaneous helpers.
 */
lsp_expr_t *lsp_reverse(lsp_expr_t *input);

bool lsp_is_truthy(lsp_expr_t *expr);

void lsp_print(lsp_expr_t *expr);

