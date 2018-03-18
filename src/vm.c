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


typedef struct {
    lsp_offset_t frame_ptr;
    // TODO return address.
} lsp_frame_t;


static const lsp_ref_t LSP_NULL = {
    .is_cons = false,
    .offset = 0,
};


static lsp_cons_t *cons_heap;
static lsp_offset_t cons_heap_ptr;

static char *data_heap;
static lsp_offset_t data_heap_ptr;

static lsp_ref_t *ref_stack;
static int ref_stack_ptr;

static lsp_frame_t *call_stack;
static int call_stack_ptr;

static uint32_t mark_stack;
static char *cons_heap_mark_bitset;
static char *data_heap_mark_bitset;
static uint64_t *heap_offset_cache;


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
static lsp_ref_t lsp_heap_alloc_cons(car, cdr);
static lsp_ref_t lsp_heap_alloc_data(lsp_type_t type, size_t size);
static void lsp_push_ref(lsp_ref_t expr);
static lsp_ref_t lsp_get_at_offset(int offset);
static void lsp_put_at_offset(lsp_ref_t value, int offset);
static void lsp_push_null_terminated(lsp_type_t type, char *value);



void lsp_vm_init() {
    // TODO This doesn't work if overcommit is disabled.
    // Block size times maximum index.
    cons_heap = (lsp_cons_t *) malloc(0x80000000 * sizeof(lsp_cons_t));
    cons_heap_ptr = 0;

    data_heap = (char *) malloc(0x80000000 * 8);
    data_heap_ptr = 0;

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
    assert(ref.offset <= 0x80000000);
    assert(ref.offset < cons_heap_ptr);

    return &cons_heap[ref.offset];
}


static lsp_header_t *lsp_heap_get_header(lsp_ref_t ref) {
    assert(!ref.is_cons);
    assert(ref.offset >= 1);
    assert(ref.offset <= 0x80000000);
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


static lsp_ref_t lsp_heap_alloc_cons(car, cdr) {
    assert(cons_heap_ptr <= 0x80000000);

    // Construct a reference.
    lsp_ref_t ref;
    ref.is_cons = true;
    ref.offset = cons_heap_ptr;
    return ref;

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
    assert(data_heap_ptr <= 0x80000000);

    // Construct a reference to the data pointed to by ptr.
    lsp_ref_t ref;
    ref.is_cons = false;
    ref.offset = data_heap_ptr;

    // Bump the ptr;
    data_heap_ptr += (sizeof(lsp_header_t) + size) >> 4;

    // Initialise the header.
    // TODO might be worth clearing the data.
    lsp_header_t *header = lsp_heap_get_header(ref);
    header->type = type;
    header->size = size >> 4;

    return ref;
}


/**
 * Stack operations.
 */
void lsp_call(int nargs) {
    call_stack_ptr++;
    call_stack[call_stack_ptr].frame_ptr = ref_stack_ptr - nargs;
}

static void lsp_return(int nret) {
    ref_stack_ptr = call_stack[call_stack_ptr].frame_ptr + nret;
    call_stack_ptr--;
}

static void lsp_push_ref(lsp_ref_t ref) {
    ref_stack[ref_stack_ptr] = ref;
    ref_stack_ptr++;
}

static lsp_ref_t lsp_get_at_offset(int offset) {
    lsp_frame_t *frame = &call_stack[call_stack_ptr - 1];
    int abs_offset;
    if (offset < 0) {
        abs_offset = ref_stack_ptr + offset;
    } else {
        abs_offset = frame->frame_ptr + offset;
    }
    assert(abs_offset >= frame->frame_ptr && abs_offset < ref_stack_ptr);
    return ref_stack[abs_offset];
}

static void lsp_put_at_offset(lsp_ref_t value, int offset) {
    lsp_ref_t *tgt = offset < 0 ? stack_ptr + offset : frame_ptr + offset;
    assert(tgt >= frame_ptr && tgt < stack_ptr);
    *tgt = value;
}

void lsp_push_null() {
    lsp_push_ref(LSP_NULL);
}

void lsp_push_cons() {
    lsp_ref_t expr = lsp_heap_cons(LSP_NULL, LSP_NULL);
    lsp_push_ref(expr);
}

void lsp_push_op(void (* value)());
void lsp_push_int(int value) {
    // Allocate space.
    lsp_ref_t ref = lsp_heap_alloc_data(LSP_TYPE_INT, sizeof(value));

    // Copy the value into the allocated space.
    char *data = lsp_heap_get_data(ref);
    memcpy(data, &value, sizeof(value));

    return ref;
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
    memcpy(data, value, sizeof(value));

    lsp_push_ref(ref);
}

void lsp_push_symbol(char *value) {
    lsp_push_null_terminated(LSP_TYPE_SYM, value);
}

void lsp_push_string(char *value) {
    lsp_push_null_terminated(LSP_TYPE_STR, value);
}

int lsp_read_int() {
    lsp_ref_t ref = lsp_get_at_offset(-1);
    assert(lsp_heap_get_type(ref) == LSP_TYPE_INT);
    int *data = (int *) lsp_heap_get_data(ref);
    return *data;
}

char *lsp_read_symbol() {
    lsp_ref_t ref = lsp_get_at_offset(-1)
    assert(lsp_heap_get_type(ref) == LSP_TYPE_SYM);
    return lsp_heap_get_data(ref);
}

char *lsp_read_string() {
    lsp_ref_t ref = lsp_get_at_offset(-1)
    assert(lsp_heap_get_type(ref) == LSP_TYPE_SYM);
    return lsp_heap_get_data(ref);
}

void lsp_cons() {
    lsp_ref_t car = lsp_get_at_offset(-2);
    lsp_ref_t cdr = lsp_get_at_offset(-1);
    lsp_ref_t cons = lsp_heap_alloc_cons(car, cdr);
    lsp_pop_to(-2);
    lsp_push_ref(cons);
}

void lsp_car() {
    lsp_ref_t cons = lsp_get_at_offset(-1);

    lsp_cons_t *cons = lsp_heap_get_cons(cons);
    lsp_ref_t car = cons->car;

    lsp_pop_to(-1);
    lsp_push_ref(car);
}

void lsp_cdr() {
    lsp_ref_t cons = lsp_get_at_offset(-1);

    lsp_cons_t *cons = lsp_heap_get_cons(cons);
    lsp_ref_t cdr = cons->cdr;

    lsp_pop_to(-1);
    lsp_push_ref(cdr);
}

void lsp_set_car() {
    lsp_ref_t cons = lsp_get_at_offset(-2);
    lsp_ref_t car = lsp_get_at_offset(-1);

    lsp_cons_t *cons_data = lsp_heap_get_cons(cons);
    cons_data->car = car;

    lsp_pop_to(-2);
}
void lsp_set_cdr() {
    lsp_ref_t cons = lsp_get_at_offset(-2);
    lsp_ref_t cdr = lsp_get_at_offset(-1);
    lsp_heap_set_cdr(cons, cdr);
    lsp_pop_to(-2);
}

void lsp_dup(int offset) {
    lsp_ref_t value = lsp_get_at_offset(offset);
    lsp_push_ref(value);
}

void lsp_store(int offset) {
    lsp_ref_t value = lsp_get_at_offset(-1);
    lsp_put_at_offset(value, offset);
    lsp_pop_to(-1);
}

void lsp_pop_to(int offset) {
    lsp_ref_t *tgt = offset < 0 ? stack_ptr + offset : frame_ptr + offset;
    assert(tgt >= frame_ptr && tgt < stack_ptr);
    stack_ptr = tgt;
}

void lsp_swp(int offset) {
    lsp_ref_t tgt = lsp_get_at_offset(offset);
    lsp_ref_t top = lsp_get_at_offset(-1);

    lsp_put_at_offset(top, offset);
    lsp_put_at_offset(tgt, -1);
}

void lsp_call(int nargs);

bool lsp_is_null();
bool lsp_is_cons();
bool lsp_is_op();
bool lsp_is_int();
bool lsp_is_symbol();
bool lsp_is_string();

bool lsp_is_truthy();
bool lsp_is_equal();
