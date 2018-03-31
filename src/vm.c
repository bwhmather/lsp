#include "vm.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

typedef enum {
    LSP_TYPE_NULL = 0,
    LSP_TYPE_CONS,
    LSP_TYPE_INT,
    LSP_TYPE_SYM,
    LSP_TYPE_STR,
    LSP_TYPE_OP,
} lsp_type_t;


/**
 * An offset into one of the two heap arrays.
 */
typedef unsigned int lsp_offset_t;

/**
 *
 * A reference to either a cons cell or block of memory.
 */
typedef struct {
    bool is_cons : 1;
    lsp_offset_t offset : 31;
} lsp_ref_t;


/**
 * Pair structure that is stored on the cons heap and is used for building
 * other data-structures.
 */
typedef struct {
    lsp_ref_t car;
    lsp_ref_t cdr;
} lsp_cons_t;


/**
 * Header for an object stored on the data heap.
 */
typedef struct {
    lsp_type_t type;
    uint32_t size;
    char data[];
} lsp_header_t;


static const lsp_ref_t LSP_NULL = {
    .is_cons = false,
    .offset = 0,
};


/**
 * The cons heap contains up to 2^31 cons cells.
 *
 * `cons_heap` is a pointer to the root of the heap.  `cons_heap_ptr` is the
 * offset of the next unused cons cell.
 */
static const lsp_offset_t CONS_HEAP_MAX = 0x80000000;
static lsp_cons_t *cons_heap;
static lsp_offset_t cons_heap_ptr;


/**
 * The data heap contains up to 2^31 eight byte blocks for storing arbitrary
 * plain data.
 *
 * References to this data must only be stored on either the reference stack or
 * the cons heap.
 *
 * `data_heap` is a pointer to the root of the heap.  `data_heap_ptr` is equal
 * to the number of 8 byte blocks before the next available blocks.
 */
static const lsp_offset_t DATA_HEAP_MAX = 0x80000000;
static char *data_heap;
static lsp_offset_t data_heap_ptr;


/**
 * The reference stack is a stack of references to data on one of the two heaps
 * that is used as working memory for the process.
 */
static const int REF_STACK_MAX = 0x100000;
static lsp_ref_t *ref_stack;
static int ref_stack_ptr;


/**
 */
static const int FRAME_STACK_MAX = 0x100;
static int *frame_stack;
static int frame_stack_ptr;


/**
 * Arrays used for bookkeeping during garbage collection.
 */
static uint32_t mark_stack;
static char *cons_heap_mark_bitset;
static char *data_heap_mark_bitset;
static uint64_t *cons_heap_offset_cache;
static uint64_t *data_heap_offset_cache;


/**
 * Internal forward declarations.
 */
static void lsp_gc_internal_mark_heap();
static void lsp_gc_internal_rebuild_offset_cache();
static void lsp_gc_internal_compact();
static void lsp_gc_internal_update_stack();
static lsp_cons_t *lsp_heap_get_cons(lsp_ref_t ref);
static lsp_header_t *lsp_heap_get_header(lsp_ref_t ref);
static lsp_type_t lsp_heap_get_type(lsp_ref_t ref);
static char *lsp_heap_get_data(lsp_ref_t ref);
static lsp_ref_t lsp_heap_alloc_null();
static lsp_ref_t lsp_heap_alloc_cons();
static lsp_ref_t lsp_heap_alloc_data(lsp_type_t type, size_t size);
static void lsp_push_ref(lsp_ref_t expr);
static lsp_ref_t lsp_get_at_offset(int offset);
static void lsp_put_at_offset(lsp_ref_t value, int offset);
static void lsp_push_null_terminated(lsp_type_t type, char *value);


void lsp_vm_init() {
    // TODO This doesn't work if overcommit is disabled.
    // Block size times maximum index.
    cons_heap = (lsp_cons_t *) malloc(CONS_HEAP_MAX * sizeof(lsp_cons_t));
    assert(cons_heap != NULL);
    cons_heap_ptr = 0;

    data_heap = (char *) malloc(DATA_HEAP_MAX * 8);
    assert(data_heap != NULL);
    data_heap_ptr = 0;

    ref_stack = (lsp_ref_t *) malloc(REF_STACK_MAX * sizeof(lsp_ref_t));
    assert(ref_stack != NULL);
    ref_stack_ptr = 0;

    frame_stack = (int *) malloc(FRAME_STACK_MAX * sizeof(int));
    assert(frame_stack != NULL);

    frame_stack_ptr = 0;
    lsp_enter_frame(0);

    // The first object allocated on the data stack must always be the null
    // singleton.
    lsp_heap_alloc_null();
}


/**
 * Internal functions for actually performing a garbage collection.
 */
static void lsp_gc_internal_mark_heap() {
    // TODO
}

static void lsp_gc_internal_rebuild_offset_cache() {
    // TODO
}

static void lsp_gc_internal_compact() {
    // TODO
}

static void lsp_gc_internal_update_stack() {
    // TODO
}

void lsp_gc_collect() {
    lsp_gc_internal_mark_heap();
    lsp_gc_internal_rebuild_offset_cache();
    lsp_gc_internal_compact();
    lsp_gc_internal_update_stack();
}


/**
 * Heap operations.
 */
static lsp_cons_t *lsp_heap_get_cons(lsp_ref_t ref) {
    assert(ref.is_cons);
    assert(ref.offset <= CONS_HEAP_MAX);
    assert(ref.offset < cons_heap_ptr);

    return &cons_heap[ref.offset];
}


static lsp_header_t *lsp_heap_get_header(lsp_ref_t ref) {
    assert(!ref.is_cons);
    assert(ref.offset <= DATA_HEAP_MAX);
    assert(ref.offset < data_heap_ptr);

    return (lsp_header_t *) &data_heap[ref.offset << 4];
}


static lsp_type_t lsp_heap_get_type(lsp_ref_t ref) {
    if (ref.is_cons) {
        return LSP_TYPE_CONS;
    }

    if (ref.offset == 0) {
        return LSP_TYPE_NULL;
    }

    lsp_header_t *header = lsp_heap_get_header(ref);
    return header->type;
}


static char *lsp_heap_get_data(lsp_ref_t ref) {
    lsp_header_t *header = lsp_heap_get_header(ref);
    return header->data;
}


static lsp_ref_t lsp_heap_alloc_null() {
    // Null can only be initialised as the first item on the data-heap.
    assert(data_heap_ptr == 0);

    // Construct a reference to the data pointed to by ptr.
    lsp_ref_t ref;
    ref.is_cons = false;
    ref.offset = 0;

    // Bump the ptr;
    data_heap_ptr += 1;

    // Initialise the header.
    // TODO might be worth clearing the data.
    lsp_header_t *header = lsp_heap_get_header(ref);
    header->type = LSP_TYPE_NULL;
    header->size = 0;

    return ref;
}


static lsp_ref_t lsp_heap_alloc_cons() {
    assert(cons_heap_ptr < CONS_HEAP_MAX);

    // Construct a reference.
    lsp_ref_t ref;
    ref.is_cons = true;
    ref.offset = cons_heap_ptr;

    // Bump the ptr.
    cons_heap_ptr += 1;

    // Initialise the cons cell.
    lsp_cons_t *cons = lsp_heap_get_cons(ref);
    cons->car = LSP_NULL;
    cons->cdr = LSP_NULL;

    return ref;
}


static lsp_ref_t lsp_heap_alloc_data(lsp_type_t type, size_t size) {
    // Offset zero is reserved for null.
    assert(data_heap_ptr >= 1);
    // TODO check upper bound.


    // Construct a reference to the data pointed to by ptr.
    lsp_ref_t ref;
    ref.is_cons = false;
    ref.offset = data_heap_ptr;

    // Bump the ptr;
    data_heap_ptr += ((sizeof(lsp_header_t) + size - 1) / 8) + 1;

    // Initialise the header.
    // TODO might be worth clearing the data.
    lsp_header_t *header = lsp_heap_get_header(ref);
    header->type = type;
    header->size = ((size - 1) / 8) + 1;

    return ref;
}

/**
 * Stack operations.
 */
void lsp_enter_frame(int nargs) {
    frame_stack[frame_stack_ptr] = ref_stack_ptr - nargs;
    frame_stack_ptr++;
}

void lsp_exit_frame(int nret) {
    frame_stack_ptr--;
}

static void lsp_push_ref(lsp_ref_t ref) {
    ref_stack[ref_stack_ptr] = ref;
    ref_stack_ptr++;
}

static lsp_ref_t lsp_get_at_offset(int offset) {
    int frame_ptr = frame_stack[frame_stack_ptr - 1];
    int abs_offset;
    if (offset < 0) {
        assert(frame_ptr - ref_stack_ptr <= offset);
        abs_offset = ref_stack_ptr + offset;
    } else {
        assert(ref_stack_ptr - frame_ptr > offset);
        abs_offset = frame_ptr + offset;
    }
    return ref_stack[abs_offset];
}

static void lsp_put_at_offset(lsp_ref_t value, int offset) {
    int frame_ptr = frame_stack[frame_stack_ptr - 1];
    int abs_offset;
    if (offset < 0) {
        assert(frame_ptr - ref_stack_ptr <= offset);
        abs_offset = ref_stack_ptr + offset;
    } else {
        assert(ref_stack_ptr - frame_ptr > offset);
        abs_offset = frame_ptr + offset;
    }
    assert(abs_offset >= frame_ptr && abs_offset < ref_stack_ptr);
    ref_stack[abs_offset] = value;
}

void lsp_push_null() {
    lsp_push_ref(LSP_NULL);
}

void lsp_push_cons() {
    lsp_ref_t expr = lsp_heap_alloc_cons();
    lsp_push_ref(expr);
}

void lsp_push_int(int value) {
    // Allocate space.
    lsp_ref_t ref = lsp_heap_alloc_data(LSP_TYPE_INT, sizeof(value));

    // Copy the value into the allocated space.
    char *data = lsp_heap_get_data(ref);
    memcpy(data, &value, sizeof(value));

    // Save the reference to the stack.
    lsp_push_ref(ref);
}

static void lsp_push_null_terminated(lsp_type_t type, char *value) {
    // Space required is equal to the length of the string plus one for the
    // terminating null byte.
    size_t size = strlen(value) + 1;

    // Allocate space.
    lsp_ref_t ref = lsp_heap_alloc_data(type, size);

    // Copy the string, including the terminating null byte, into the allocated
    // space.
    char *data = lsp_heap_get_data(ref);
    memcpy(data, value, size);

    // Save the reference to the stack.
    lsp_push_ref(ref);
}

void lsp_push_symbol(char *value) {
    lsp_push_null_terminated(LSP_TYPE_SYM, value);
}

void lsp_push_string(char *value) {
    lsp_push_null_terminated(LSP_TYPE_STR, value);
}

void lsp_push_op(lsp_op_t op) {
    // Allocate space.
    lsp_ref_t ref = lsp_heap_alloc_data(LSP_TYPE_OP, sizeof(op));

    // Copy the value into the allocated space.
    char *data = lsp_heap_get_data(ref);
    memcpy(data, &op, sizeof(op));

    // Save the reference to the stack.
    lsp_push_ref(ref);
}

int lsp_read_int() {
    lsp_ref_t ref = lsp_get_at_offset(-1);
    assert(lsp_heap_get_type(ref) == LSP_TYPE_INT);
    int *data = (int *) lsp_heap_get_data(ref);
    return *data;
}

char *lsp_read_symbol() {
    lsp_ref_t ref = lsp_get_at_offset(-1);
    assert(lsp_heap_get_type(ref) == LSP_TYPE_SYM);
    return lsp_heap_get_data(ref);
}

char *lsp_read_string() {
    lsp_ref_t ref = lsp_get_at_offset(-1);
    assert(lsp_heap_get_type(ref) == LSP_TYPE_SYM);
    return lsp_heap_get_data(ref);
}

lsp_op_t lsp_read_op() {
    lsp_ref_t ref = lsp_get_at_offset(-1);
    assert(lsp_heap_get_type(ref) == LSP_TYPE_OP);
    lsp_op_t *data = (lsp_op_t *) lsp_heap_get_data(ref);
    return *data;
}

void lsp_cons() {
    lsp_ref_t car_ref = lsp_get_at_offset(-2);
    lsp_ref_t cdr_ref = lsp_get_at_offset(-1);
    lsp_ref_t cons_ref = lsp_heap_alloc_cons();

    lsp_cons_t *cons = lsp_heap_get_cons(cons_ref);
    cons->car = car_ref;
    cons->cdr = cdr_ref;

    lsp_pop_to(-2);
    lsp_push_ref(cons_ref);
}

void lsp_car() {
    lsp_ref_t cons_ref = lsp_get_at_offset(-1);

    lsp_cons_t *cons = lsp_heap_get_cons(cons_ref);
    lsp_ref_t car_ref = cons->car;

    lsp_pop_to(-1);
    lsp_push_ref(car_ref);
}

void lsp_cdr() {
    lsp_ref_t cons_ref = lsp_get_at_offset(-1);

    lsp_cons_t *cons = lsp_heap_get_cons(cons_ref);
    lsp_ref_t cdr_ref = cons->cdr;

    lsp_pop_to(-1);
    lsp_push_ref(cdr_ref);
}

void lsp_set_car() {
    lsp_ref_t cons_ref = lsp_get_at_offset(-2);
    lsp_ref_t car_ref = lsp_get_at_offset(-1);

    lsp_cons_t *cons = lsp_heap_get_cons(cons_ref);
    cons->car = car_ref;

    lsp_pop_to(-2);
}

void lsp_set_cdr() {
    lsp_ref_t cons_ref = lsp_get_at_offset(-2);
    lsp_ref_t cdr_ref = lsp_get_at_offset(-1);

    lsp_cons_t *cons = lsp_heap_get_cons(cons_ref);
    cons->cdr = cdr_ref;

    lsp_pop_to(-2);
}

void lsp_dup(int offset) {
    lsp_ref_t ref = lsp_get_at_offset(offset);
    lsp_push_ref(ref);
}

void lsp_store(int offset) {
    lsp_ref_t ref = lsp_get_at_offset(-1);
    lsp_put_at_offset(ref, offset);
    lsp_pop_to(-1);
}

void lsp_pop_to(int offset) {
    int frame_ptr = frame_stack[frame_stack_ptr - 1];
    int abs_offset;
    if (offset < 0) {
        assert(frame_ptr - ref_stack_ptr <= offset);
        abs_offset = ref_stack_ptr + offset;
    } else {
        assert(ref_stack_ptr - frame_ptr > offset);
        abs_offset = frame_ptr + offset;
    }

    ref_stack_ptr = abs_offset;
}

void lsp_pop() {
    lsp_pop_to(-1);
}

void lsp_swp(int offset) {
    lsp_ref_t tgt = lsp_get_at_offset(offset);
    lsp_ref_t top = lsp_get_at_offset(-1);

    lsp_put_at_offset(top, offset);
    lsp_put_at_offset(tgt, -1);
}

bool lsp_is_null() {
    lsp_ref_t ref = lsp_get_at_offset(-1);
    lsp_pop_to(-1);
    return lsp_heap_get_type(ref) == LSP_TYPE_NULL;
}

bool lsp_is_cons() {
    lsp_ref_t ref = lsp_get_at_offset(-1);
    lsp_pop_to(-1);
    return lsp_heap_get_type(ref) == LSP_TYPE_CONS;
}

bool lsp_is_int() {
    lsp_ref_t ref = lsp_get_at_offset(-1);
    lsp_pop_to(-1);
    return lsp_heap_get_type(ref) == LSP_TYPE_INT;
}

bool lsp_is_symbol() {
    lsp_ref_t ref = lsp_get_at_offset(-1);
    lsp_pop_to(-1);
    return lsp_heap_get_type(ref) == LSP_TYPE_SYM;
}

bool lsp_is_string() {
    lsp_ref_t ref = lsp_get_at_offset(-1);
    lsp_pop_to(-1);
    return lsp_heap_get_type(ref) == LSP_TYPE_STR;
}

bool lsp_is_op() {
    lsp_ref_t ref = lsp_get_at_offset(-1);
    lsp_pop_to(-1);
    return lsp_heap_get_type(ref) == LSP_TYPE_OP;
}

bool lsp_is_truthy() {
    lsp_ref_t ref = lsp_get_at_offset(-1);
    switch (lsp_heap_get_type(ref)) {
        case LSP_TYPE_NULL:
            lsp_pop_to(-1);
            return false;
        case LSP_TYPE_CONS:
            lsp_pop_to(-1);
            return true;
        case LSP_TYPE_INT:
            return lsp_read_int() != 0;
        case LSP_TYPE_SYM:
            return true;
        case LSP_TYPE_STR:
            return strlen(lsp_read_string()) > 0;
        default:
            assert(false);
    }
}

bool lsp_is_equal();

size_t lsp_stats_frame_size() {
    assert(frame_stack_ptr > 0);
    int frame_ptr = frame_stack[frame_stack_ptr - 1];
    assert(frame_ptr >= 0);
    assert(ref_stack_ptr >= 0);
    assert(frame_ptr <= ref_stack_ptr);
    return (size_t) (ref_stack_ptr - frame_ptr);
}

size_t lsp_stats_stack_size() {
    assert(ref_stack_ptr >= 0);
    return (size_t) ref_stack_ptr;
}

