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


/**
 * Structure for storing metadata related to a block.
 */
typedef struct lsp_heap_meta_t {
    bool is_pointer : 1;
    bool is_continuation : 1;
    unsigned int type : 6;
} lsp_heap_meta_t;


/**
 * Structure representing the smallest unit of memory that can be allocated on
 * the heap.
 */
typedef struct lsp_heap_block_t {
    char data[8];
}

static lsp_heap_block_t heap_data[];
static lsp_heap_meta_t heap_metadata[];
static lsp_heap_ref_t heap_cursor;

static lsp_heap_ref_t stack[];
static lsp_heap_ref_t *frame_ptr;
static lsp_heap_ref_t *stack_ptr;

static lsp_heap_ref_t heap_mark_stack[];
static uint64_t heap_mark_bitset;
static lsp_heap_ref_t heap_offset_cache[];



/**
 * Internal functions for garbage collection.
 */
static void lsp_gc_mark_heap() {
    // TODO
}

static void lsp_gc_update_offset_cache() {
    // TODO
}

static void lsp_gc_compact() {
    // TODO
}

static void lsp_gc_update_stack() {
    // TODO
}

static void lsp_gc() {
    lsp_gc_mark_heap();
    lsp_gc_rebuild_offset_cache();
    lsp_gc_compact();
    lsp_gc_update_stack();
}


/**
 * Aborts the program if `ref` does not point to the start of a valid object
 * on the heap.
 */
static void lsp_check_ref(lsp_heap_ref_t ref) {
    assert(ref >= heap_cursor);
    assert(!lsp_heap_is_continuation(ref));
}

/**
 * Returns the type of the object stored at offset `ref`.
 *
 * Will abort if `ref` does not point to the start of an object on the heap.
 */
unsigned int lsp_heap_type(int offset) {
    return (lsp_type_t)heap_metadata[ref].type;
}

/**
 * Returns a pointer to area of memory allocated on the heap for the object
 * stored at offset `ref`.
 */
char *lsp_heap_data(int offset) {
    return (char *)(&heap_data[ref]);
}

/**
 * Allocates a contiguous area of memory for an object.
 *
 * Currently all blocks making up an object must be marked with the same type,
 * and must all be pointers or all be literals values.
 *
 * All bytes within the allocated area of memory will be initialised to zero.
 *
 * .. warning::
 *     Allocation may trigger a garbage collection cycle.  All non-stack
 *     references to objects stored on the heap should be released before
 *     calling this function.
 */
void lsp_heap_allocate(
    unsigned int size, unsigned int type, bool is_pointer,
) {
    // Figure out how many blocks we need to claim to be able to fit the
    // requested number of bytes.
    unsigned int num_blocks = size / sizeof(lsp_heap_block_t);
    if (size % sizeof(lsp_heap_block_t)) {
        num_blocks += 1;
    }

    // TODO check available space and possibly trigger garbage collection.

    // Zero out allocated data.
    memset(
        (char *)&heap_data[heap_cursor],
        0, num_blocks * sizeof(lsp_heap_block_t)
    );

    // Write metadata for header block.
    heap_metadata[heap_cursor].is_pointer = is_pointer;
    heap_metadata[heap_cursor].is_continuation = false;
    heap_metadata[heap_cursor].type = type;

    // Write metadata for each of the allocated blocks.
    for (int i = 1; i < num_blocks; i++) {
        heap_metadata[heap_cursor + i].is_pointer = is_pointer;
        heap_metadata[heap_cursor + i].is_continuation = true;
        heap_metadata[heap_cursor + i].type = type;
    }

    lsp_heap_ ref = heap_cursor;

    // Update cursor.
    heap_cursor += num_blocks;
}






void lsp_dup(int offset) {
    lsp_value_t *value = lsp_get_at_offset(offset);
    lsp_push_expr(value);
}

void lsp_store(int offset) {
    lsp_value_t *value = lsp_get_at_offset(-1);
    lsp_put_at_offset(value, offset);
    lsp_pop_to(-1);
}

void lsp_pop_to(int offset) {
    lsp_value_t **tgt = offset < 0 ? stack_ptr + offset : frame_ptr + offset;
    assert(tgt >= frame_ptr && tgt < stack_ptr);
    stack_ptr = tgt;
}

void lsp_swp(int offset) {
    lsp_value_t *tgt = lsp_get_at_offset(offset);
    lsp_value_t *top = lsp_get_at_offset(-1);

    lsp_put_at_offset(top, offset);
    lsp_put_at_offset(tgt, -1);
}




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

