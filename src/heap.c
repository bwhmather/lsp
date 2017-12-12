#include "heap.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

typedef struct lsp_cons_t {
    lsp_expr_t *car;
    lsp_expr_t *cdr;
} lsp_cons_t;


static char *heap_data;
static char *heap_ptr;


void lsp_heap_init() {
    heap_data = malloc(128 * 1024 * 1024);
    heap_ptr = heap_data + sizeof(lsp_type_t);
}


lsp_type_t lsp_type(lsp_expr_t *expr) {
    if (expr == NULL) {
        return LSP_NULL;
    }
    lsp_type_t *type_ptr = (lsp_type_t *) expr;
    return *type_ptr;
}

static char *lsp_data(lsp_expr_t *expr) {
    return (char *) expr + sizeof(lsp_type_t);
}


/**
 * Functions for working with integer objects.
 */
lsp_expr_t *lsp_int(int value) {
    lsp_expr_t *expr_ptr = heap_ptr;

    lsp_type_t *type_ptr = (lsp_type_t *) heap_ptr;
    *type_ptr = LSP_INT;
    heap_ptr += sizeof(lsp_type_t);

    int *int_ptr = (int *) heap_ptr;
    *int_ptr = value;
    heap_ptr += sizeof(int);

    return expr_ptr;
}

int *lsp_as_int(lsp_expr_t *expr) {
    assert(lsp_type(expr) == LSP_INT);
    return (int *) lsp_data(expr);
}


/**
 * Functions for working with symbols.
 */
lsp_expr_t *lsp_symbol_start() {
    lsp_expr_t *expr_ptr = heap_ptr;

    lsp_type_t *type_ptr = (lsp_type_t *) heap_ptr;
    *type_ptr = LSP_SYM;
    heap_ptr += sizeof(lsp_type_t);

    return expr_ptr;
}

void lsp_symbol_push(char character) {
    *heap_ptr = character;
    heap_ptr += 1;
}

void lsp_symbol_stop() {
    lsp_symbol_push('\0');
}

lsp_expr_t *lsp_symbol(char *name) {
    lsp_expr_t *expr = lsp_symbol_start();

    for (int cursor=0; name[cursor] != '\0'; cursor++) {
        lsp_symbol_push(name[cursor]);
    }
    lsp_symbol_stop();
    return expr;
}

char *lsp_as_sym(lsp_expr_t *expr) {
    assert(lsp_type(expr) == LSP_SYM);
    return (char *) lsp_data(expr);
}


/**
 * Functions for working with cons cells.
 */
lsp_expr_t *lsp_cons(lsp_expr_t *car, lsp_expr_t *cdr) {
    lsp_expr_t *expr_ptr = (lsp_expr_t *) heap_ptr;

    lsp_type_t *type_ptr = (lsp_type_t *) heap_ptr;
    *type_ptr = LSP_CONS;
    heap_ptr += sizeof(lsp_type_t);

    lsp_cons_t *cons_ptr = (lsp_cons_t *) heap_ptr;
    cons_ptr->car = car;
    cons_ptr->cdr = cdr;
    heap_ptr += sizeof(lsp_cons_t);

    return expr_ptr;
}

lsp_expr_t *lsp_car(lsp_expr_t *expr) {
    if (lsp_type(expr) != LSP_CONS) {
        assert(false);
    }
    lsp_cons_t *cons = (lsp_cons_t *) lsp_data(expr);

    return cons->car;
}

lsp_expr_t *lsp_caar(lsp_expr_t *expr) {
    return lsp_car(lsp_cdr(expr));
}

lsp_expr_t *lsp_caaar(lsp_expr_t *expr) {
    return lsp_car(lsp_cddr(expr));
}

lsp_expr_t *lsp_cdr(lsp_expr_t *expr) {
    if (lsp_type(expr) != LSP_CONS) {
        assert(false);
    }
    lsp_cons_t *cons = (lsp_cons_t *) lsp_data(expr);

    return cons->cdr;
}

lsp_expr_t *lsp_cddr(lsp_expr_t *expr) {
    return lsp_cdr(lsp_cdr(expr));
}

lsp_expr_t *lsp_cdddr(lsp_expr_t *expr) {
    return lsp_cdr(lsp_cddr(expr));
}

void lsp_set_car(lsp_expr_t *expr, lsp_expr_t *new_car) {
    if (lsp_type(expr) != LSP_CONS) {
        assert(false);
    }
    lsp_cons_t *cons = (lsp_cons_t *) lsp_data(expr);

    cons->car = new_car;
}

void lsp_set_cdr(lsp_expr_t *expr, lsp_expr_t *new_cdr) {
    if (lsp_type(expr) != LSP_CONS) {
        assert(false);
    }
    lsp_cons_t *cons = (lsp_cons_t *) lsp_data(expr);

    cons->cdr = new_cdr;
}


/**
 * Functions for working with expressions representing built-in operations.
 */
lsp_expr_t *lsp_op(lsp_op_t op) {
    lsp_expr_t *expr_ptr = heap_ptr;

    lsp_type_t *type_ptr = (lsp_type_t *) heap_ptr;
    *type_ptr = LSP_OP;
    heap_ptr += sizeof(lsp_type_t);

    lsp_op_t *op_ptr = (lsp_op_t *) heap_ptr;
    *op_ptr = op;
    heap_ptr += sizeof(lsp_op_t);

    return expr_ptr;
}

lsp_op_t *lsp_as_op(lsp_expr_t *expr) {
    assert(lsp_type(expr) == LSP_OP);
    return (lsp_op_t *) lsp_data(expr);
}


/**
 * Miscellaneous helpers.
 */
lsp_expr_t *lsp_reverse(lsp_expr_t *input) {
    lsp_expr_t *output = NULL;
    while (lsp_type(input) == LSP_CONS) {
        output = lsp_cons(lsp_car(input), output);
        input = lsp_cdr(input);
    }
    assert(lsp_type(input) == LSP_NULL);
    return output;
}


bool lsp_is_truthy(lsp_expr_t *expr) {
    if (lsp_type(expr) == LSP_INT) {
        return *lsp_as_int(expr) != 0;
    } else if (lsp_type(expr) == LSP_NULL) {
        return false;
    } else {
        return true;
    }
}


void lsp_print(lsp_expr_t *expr) {
    switch (lsp_type(expr)) {
        case LSP_NULL:
            printf("()");
            break;
        case LSP_CONS:
            printf("(");
            while (lsp_type(expr) == LSP_CONS) {
                lsp_print(lsp_car(expr));
                expr = lsp_cdr(expr);
                if (lsp_type(expr) == LSP_CONS) {
                    printf(" ");
                }
            }
            if (lsp_type(expr) != LSP_NULL) {
                printf(" . ");
                lsp_print(expr);
            }
            printf(")");
            break;
        case LSP_INT:
            printf("%i", *lsp_as_int(expr));
            break;
        case LSP_SYM:
            printf("%s", lsp_as_sym(expr));
            break;
        case LSP_OP:
            printf("<builtin>");
            break;
        default:
            assert(false);
    }
}

