#include "heap.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

/**
 * An offset into the heap array.
 *
 * Used to represent a reference to an object stored on the heap.
 */
typedef uint32_t lsp_heap_ref_t;

typedef struct lsp_heap_meta_t {
    bool is_pointer : 1;
    bool is_continuation : 1;
    unsigned int type : 6;
} lsp_heap_meta_t;


typedef struct lsp_heap_block_t {
    char data[8];
}

static lsp_heap_block_t heap_data[];
static lsp_heap_meta_t heap_metadata[];
static lsp_heap_ref_t heap_cursor;

static lsp_heap_ref_t heap_mark_stack[];
static uint64_t heap_mark_bitset;
static lsp_heap_ref_t heap_offset_cache[];


typedef struct lsp_cons_t {
    lsp_value_t *car;
    lsp_value_t *cdr;
} lsp_cons_t;


void lsp_heap_init() {
    heap_data = malloc(128 * 1024 * 1024);
    heap_ptr = heap_data + sizeof(lsp_type_t);
}


lsp_type_t lsp_type(lsp_value_t *expr) {
    if (expr == NULL) {
        return LSP_NULL;
    }
    lsp_type_t *type_ptr = (lsp_type_t *) expr;
    return *type_ptr;
}

static char *lsp_heap_data(lsp_value_t *expr) {
    return (char *) expr + sizeof(lsp_type_t);
}


void lsp_heap_assert_type(lsp_value_t *expr, lsp_type_t type) {
    if (lsp_heap_type(expr) != type) {
        lsp_heap_print(expr);
        abort();
    }
}


/**
 * Functions for working with integer objects.
 */
lsp_value_t *lsp_heap_int(int value) {
    lsp_value_t *expr_ptr = heap_ptr;

    lsp_type_t *type_ptr = (lsp_type_t *) heap_ptr;
    *type_ptr = LSP_INT;
    heap_ptr += sizeof(lsp_type_t);

    int *int_ptr = (int *) heap_ptr;
    *int_ptr = value;
    heap_ptr += sizeof(int);

    return expr_ptr;
}

int *lsp_heap_as_int(lsp_value_t *expr) {
    assert(lsp_heap_type(expr) == LSP_INT);
    return (int *) lsp_heap_data(expr);
}


/**
 * Functions for working with symbols.
 */
lsp_value_t *lsp_heap_symbol_start() {
    lsp_value_t *expr_ptr = heap_ptr;

    lsp_type_t *type_ptr = (lsp_type_t *) heap_ptr;
    *type_ptr = LSP_SYM;
    heap_ptr += sizeof(lsp_type_t);

    return expr_ptr;
}

void lsp_heap_symbol_push(char character) {
    *heap_ptr = character;
    heap_ptr += 1;
}

void lsp_heap_symbol_stop() {
    lsp_heap_symbol_push('\0');
}

lsp_value_t *lsp_heap_symbol(char *name) {
    lsp_value_t *expr = lsp_heap_symbol_start();

    for (int cursor=0; name[cursor] != '\0'; cursor++) {
        lsp_heap_symbol_push(name[cursor]);
    }
    lsp_heap_symbol_stop();
    return expr;
}

char *lsp_heap_as_sym(lsp_value_t *expr) {
    assert(lsp_heap_type(expr) == LSP_SYM);
    return (char *) lsp_heap_data(expr);
}


/**
 * Functions for working with cons cells.
 */
lsp_value_t *lsp_heap_cons(lsp_value_t *car, lsp_value_t *cdr) {
    lsp_value_t *expr_ptr = (lsp_value_t *) heap_ptr;

    lsp_type_t *type_ptr = (lsp_type_t *) heap_ptr;
    *type_ptr = LSP_CONS;
    heap_ptr += sizeof(lsp_type_t);

    lsp_cons_t *cons_ptr = (lsp_cons_t *) heap_ptr;
    cons_ptr->car = car;
    cons_ptr->cdr = cdr;
    heap_ptr += sizeof(lsp_cons_t);

    return expr_ptr;
}

lsp_value_t *lsp_heap_car(lsp_value_t *expr) {
    lsp_heap_assert_type(expr, LSP_CONS);
    lsp_cons_t *cons = (lsp_cons_t *) lsp_heap_data(expr);

    return cons->car;
}

lsp_value_t *lsp_heap_cdr(lsp_value_t *expr) {
    lsp_heap_assert_type(expr, LSP_CONS);
    lsp_cons_t *cons = (lsp_cons_t *) lsp_heap_data(expr);

    return cons->cdr;
}

void lsp_heap_set_car(lsp_value_t *expr, lsp_value_t *new_car) {
    lsp_heap_assert_type(expr, LSP_CONS);
    lsp_cons_t *cons = (lsp_cons_t *) lsp_heap_data(expr);

    cons->car = new_car;
}

void lsp_heap_set_cdr(lsp_value_t *expr, lsp_value_t *new_cdr) {
    lsp_heap_assert_type(expr, LSP_CONS);
    lsp_cons_t *cons = (lsp_cons_t *) lsp_heap_data(expr);

    cons->cdr = new_cdr;
}


/**
 * Functions for working with expressions representing built-in operations.
 */
lsp_value_t *lsp_heap_op(lsp_op_t op) {
    lsp_value_t *expr_ptr = heap_ptr;

    lsp_type_t *type_ptr = (lsp_type_t *) heap_ptr;
    *type_ptr = LSP_OP;
    heap_ptr += sizeof(lsp_type_t);

    lsp_op_t *op_ptr = (lsp_op_t *) heap_ptr;
    *op_ptr = op;
    heap_ptr += sizeof(lsp_op_t);

    return expr_ptr;
}

lsp_op_t *lsp_heap_as_op(lsp_value_t *expr) {
    assert(lsp_heap_type(expr) == LSP_OP);
    return (lsp_op_t *) lsp_heap_data(expr);
}


/**
 * Miscellaneous helpers.
 */
bool lsp_heap_is_truthy(lsp_value_t *expr) {
    if (lsp_heap_type(expr) == LSP_INT) {
        return *lsp_heap_as_int(expr) != 0;
    } else if (lsp_heap_type(expr) == LSP_NULL) {
        return false;
    } else {
        return true;
    }
}


void lsp_heap_print(lsp_value_t *expr) {
    switch (lsp_heap_type(expr)) {
        case LSP_NULL:
            printf("()");
            break;
        case LSP_CONS:
            printf("(");
            while (lsp_heap_type(expr) == LSP_CONS) {
                lsp_heap_print(lsp_heap_car(expr));
                expr = lsp_heap_cdr(expr);
                if (lsp_heap_type(expr) == LSP_CONS) {
                    printf(" ");
                }
            }
            if (lsp_heap_type(expr) != LSP_NULL) {
                printf(" . ");
                lsp_heap_print(expr);
            }
            printf(")");
            break;
        case LSP_INT:
            printf("%i", *lsp_heap_as_int(expr));
            break;
        case LSP_SYM:
            printf("%s", lsp_heap_as_sym(expr));
            break;
        case LSP_OP:
            printf("<builtin>");
            break;
        default:
            assert(false);
    }
}

