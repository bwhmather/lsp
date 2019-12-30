#include "lsp.h"

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
    lsp_type_t type : 32;
    uint32_t size : 32;
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
#define CONS_HEAP_MAX 1048576  // 0x80000000;
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
#define DATA_HEAP_MAX 1048576  //  0x80000000;
static char *data_heap;
static lsp_offset_t data_heap_ptr;


/**
 * The reference stack is a stack of references to data on one of the two heaps
 * that is used as working memory for the process.
 */
#define REF_STACK_MAX 0x100000
static lsp_ref_t *ref_stack;
static int ref_stack_ptr;
static int ref_frame_ptr;


/**
 * Arrays used for bookkeeping during garbage collection.
 */

/**
 * A stack of offsets into the cons heap.  This is used to keep track of cons
 * cells that need to be visited by the garbage collector.
 */
// TODO Work out real limit.  Should be approximately half CONS_HEAP_MAX;
#define MARK_STACK_MAX CONS_HEAP_MAX
static lsp_ref_t *mark_stack;
static size_t mark_stack_ptr;

/**
 * A bitset with one bit for each pair in the cons heap.  Will be updated by
 * the garbage collector, which will set the corresponding bit for each
 * reachable cons cell.
 */
#define CONS_HEAP_MARK_BITSET_MAX (CONS_HEAP_MAX / 32)
static uint32_t *cons_heap_mark_bitset;

/**
 * A bitset with one bit for each word in the data heap.  Bits corresponding to
 * reachable words will be set to one by the garbage collector.
 */
#define DATA_HEAP_MARK_BITSET_MAX (DATA_HEAP_MAX / 32)
static uint32_t *data_heap_mark_bitset;

/**
 * For each block of 8 bytes in the `cons_heap_mark_bitset`, contains a cache
 * of the sum of the popcount of all preceding bytes.  The offset of a cons
 * cell in the heap after compaction is equal to the number of the bits that
 * are set before it.
 */
#define CONS_HEAP_OFFSET_CACHE_MAX CONS_HEAP_MARK_BITSET_MAX
static uint32_t *cons_heap_offset_cache;

/**
 * For each block of ??? bytes in the `data_heap_mark_bitset`, contains a cache
 * of the sum of the popcount of all preceding bytes.  The offset of a word in
 * the data heap after compaction is equal to the number of the bits that are
 * set before it.
 */
#define DATA_HEAP_OFFSET_CACHE_MAX DATA_HEAP_MARK_BITSET_MAX
static uint32_t *data_heap_offset_cache;

/**
 * Internal forward declarations.
 */
static void lsp_gc_internal_mark_heap(void);
static void lsp_gc_internal_rebuild_offset_cache(void);
static void lsp_gc_internal_compact(void);
static void lsp_gc_internal_update_stack(void);
static lsp_cons_t *lsp_heap_get_cons(lsp_ref_t ref);
static lsp_header_t *lsp_heap_get_header(lsp_ref_t ref);
static lsp_type_t lsp_heap_get_type(lsp_ref_t ref);
static char *lsp_heap_get_data(lsp_ref_t ref);
static lsp_ref_t lsp_heap_alloc_null(void);
static lsp_ref_t lsp_heap_alloc_cons(void);
static lsp_ref_t lsp_heap_alloc_data(lsp_type_t type, size_t size);
static void lsp_push_ref(lsp_ref_t expr);
static lsp_ref_t lsp_get_at_offset(int offset);
static void lsp_put_at_offset(lsp_ref_t value, int offset);
static void lsp_push_null_terminated(lsp_type_t type, char const *value);


static int lsp_popcount(uint32_t x) {
    return __builtin_popcount(x);
}


void lsp_vm_init(void) {
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
    ref_frame_ptr = 0;

    mark_stack = (lsp_ref_t *) malloc(MARK_STACK_MAX * sizeof(lsp_ref_t));
    assert(mark_stack != NULL);
    mark_stack_ptr = 0;

    cons_heap_offset_cache = (uint32_t *) malloc(
        CONS_HEAP_OFFSET_CACHE_MAX * sizeof(uint32_t)
    );
    assert(cons_heap_offset_cache != NULL);

    data_heap_offset_cache = (uint32_t *) malloc(
        DATA_HEAP_OFFSET_CACHE_MAX * sizeof(uint32_t)
    );
    assert(data_heap_offset_cache != NULL);

    cons_heap_mark_bitset = (uint32_t *) malloc(
        CONS_HEAP_MARK_BITSET_MAX * sizeof(uint32_t)
    );
    assert(cons_heap_mark_bitset != NULL);

    data_heap_mark_bitset = (uint32_t *) malloc(
        DATA_HEAP_MARK_BITSET_MAX * sizeof(uint32_t)
    );
    assert(data_heap_mark_bitset != NULL);

    // The first object allocated on the data stack must always be the null
    // singleton.
    lsp_heap_alloc_null();
}


static void lsp_gc_internal_reset(void) {
    mark_stack_ptr = 0;
}

static void lsp_gc_internal_mark_ref(lsp_ref_t ref) {
    if (ref.is_cons) {
        off_t word = ref.offset >> 5;
        int bit = ref.offset & 0x1f;
        uint32_t bitmask = 0x01 << bit;

        if (cons_heap_mark_bitset[word] & bitmask) {
            return;
        }

        cons_heap_mark_bitset[word] |= bitmask;

        mark_stack[mark_stack_ptr] = ref;
        mark_stack_ptr++;
    } else {
        size_t size = lsp_heap_get_header(ref)->size;

        for (
            uint32_t offset = ref.offset;
            offset < ref.offset + size;
            offset++
        ) {
            off_t word = offset >> 5;
            int bit = offset & 0x1f;

            data_heap_mark_bitset[word] |= 0x01 << bit;
        }
    }
}


/**
 * Internal functions for actually performing a garbage collection.
 */
static void lsp_gc_internal_mark_heap(void) {
    lsp_gc_internal_mark_ref(LSP_NULL);

    for (off_t i = 0; i < ref_stack_ptr; i++) {
        lsp_ref_t ref = ref_stack[i];
        lsp_gc_internal_mark_ref(ref);
    }

    while (mark_stack_ptr) {
        mark_stack_ptr--;
        lsp_ref_t ref = mark_stack[mark_stack_ptr];
        lsp_gc_internal_mark_ref(ref);
    }
}

static void lsp_gc_internal_rebuild_offset_cache(void) {
    uint32_t offset = 0;
    for (unsigned int i = 0; i < cons_heap_ptr; i++) {
        cons_heap_offset_cache[i] = offset;
        offset += lsp_popcount(cons_heap_mark_bitset[i]);
    }

    offset = 0;
    for (unsigned int i = 0; i < data_heap_ptr; i++) {
        data_heap_offset_cache[i] = offset;
        offset += lsp_popcount(data_heap_mark_bitset[i]);
    }
}

static void lsp_gc_internal_compact(void) {
    // TODO
}

static void lsp_gc_internal_update_heap(void) {
    // Iterates over the cons heap, and updates each pointer to point to its
    // new location.
}

static void lsp_gc_internal_update_stack(void) {
    // Updates each reference on the stack to point to the new location of the
    // data.
}

void lsp_gc_collect(void) {
    lsp_gc_internal_reset();
    lsp_gc_internal_mark_heap();
    lsp_gc_internal_rebuild_offset_cache();
    lsp_gc_internal_compact();
    lsp_gc_internal_update_heap();
    lsp_gc_internal_update_stack();
}

void lsp_gc_maybe_collect(void) {
    // Don't collect unless there have been a good number of allocations.

    // If number of objects in data heap is greater than stack size plus the
    // number of cons cells then some of them must be inaccessible.

    // If the heap has grown by more than a certain amount since the last
    // collection then it should be worth re-scanning it.


    lsp_gc_collect();
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

    return (lsp_header_t *) &data_heap[ref.offset << 3];
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


static lsp_ref_t lsp_heap_alloc_null(void) {
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


static lsp_ref_t lsp_heap_alloc_cons(void) {
    assert(cons_heap_ptr < CONS_HEAP_MAX);

    lsp_gc_maybe_collect();

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
    assert(size < DATA_HEAP_MAX);

    lsp_gc_maybe_collect();

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
lsp_fp_t lsp_get_fp(void) {
    return (lsp_fp_t) ref_frame_ptr;
}

void lsp_shrink_frame(int nargs) {
    if (nargs < 0) {
        assert(false);
        // lsp_abort("new frame cannot contain a negative number of references");
    }

    if (nargs > (ref_stack_ptr - ref_frame_ptr)) {
        assert(false);
        // lsp_abort("not enough values to create new frame");
    }

    ref_frame_ptr = ref_stack_ptr - nargs;
}

void lsp_restore_fp(lsp_fp_t fp) {
    if (fp < 0 || fp > REF_STACK_MAX) {
        assert(false);
        // lsp_abort("cannot restore frame pointer to invalid value");
    }

    if (fp > ref_stack_ptr) {
        assert(false);
        // lsp_abort("cannot restore frame that has been completely popped");
    }

    ref_frame_ptr = fp;
}

static void lsp_push_ref(lsp_ref_t ref) {
    ref_stack[ref_stack_ptr] = ref;
    ref_stack_ptr++;
}

static lsp_ref_t lsp_get_at_offset(int offset) {
    int abs_offset;
    if (offset < 0) {
        // Offset is less than zero so is relative to the frame pointer.
        assert(ref_frame_ptr - ref_stack_ptr <= offset);
        abs_offset = ref_frame_ptr - offset - 1;
    } else {
        // Offset is greater than zero so is relative to the stack pointer.
        assert(ref_stack_ptr - ref_frame_ptr > offset);
        abs_offset = ref_stack_ptr - offset - 1;
    }
    assert(abs_offset >= ref_frame_ptr);
    assert(abs_offset < ref_stack_ptr);
    return ref_stack[abs_offset];
}

static void lsp_put_at_offset(lsp_ref_t value, int offset) {
    int abs_offset;
    if (offset < 0) {
        // Offset is less than zero so is relative to the frame pointer.
        assert(ref_frame_ptr - ref_stack_ptr <= offset);
        abs_offset = ref_frame_ptr - offset - 1;
    } else {
        // Offset is greater than zero so is relative to the stack pointer.
        assert(ref_stack_ptr - ref_frame_ptr > offset);
        abs_offset = ref_stack_ptr - offset - 1;
    }
    assert(abs_offset >= ref_frame_ptr && abs_offset < ref_stack_ptr);
    ref_stack[abs_offset] = value;
}

void lsp_push_null(void) {
    lsp_push_ref(LSP_NULL);
}

void lsp_push_cons(void) {
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

static void lsp_push_null_terminated(lsp_type_t type, char const *value) {
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

void lsp_push_symbol(char const *value) {
    lsp_push_null_terminated(LSP_TYPE_SYM, value);
}

void lsp_push_string(char const *value) {
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

int lsp_read_int(int offset) {
    lsp_ref_t ref = lsp_get_at_offset(offset);
    assert(lsp_heap_get_type(ref) == LSP_TYPE_INT);
    int *data = (int *) lsp_heap_get_data(ref);
    return *data;
}

char const *lsp_borrow_symbol(int offset) {
    lsp_ref_t ref = lsp_get_at_offset(offset);
    assert(lsp_heap_get_type(ref) == LSP_TYPE_SYM);
    return lsp_heap_get_data(ref);
}

char const *lsp_borrow_string(int offset) {
    lsp_ref_t ref = lsp_get_at_offset(offset);
    assert(lsp_heap_get_type(ref) == LSP_TYPE_STR);
    return lsp_heap_get_data(ref);
}

lsp_op_t lsp_read_op(int offset) {
    lsp_ref_t ref = lsp_get_at_offset(offset);
    assert(lsp_heap_get_type(ref) == LSP_TYPE_OP);
    lsp_op_t *data = (lsp_op_t *) lsp_heap_get_data(ref);
    return *data;
}

void lsp_cons(void) {
    lsp_ref_t cons_ref = lsp_heap_alloc_cons();
    lsp_ref_t car_ref = lsp_get_at_offset(0);
    lsp_ref_t cdr_ref = lsp_get_at_offset(1);

    lsp_cons_t *cons = lsp_heap_get_cons(cons_ref);
    cons->car = car_ref;
    cons->cdr = cdr_ref;

    lsp_pop();
    lsp_pop();
    lsp_push_ref(cons_ref);
}

void lsp_car(void) {
    lsp_ref_t cons_ref = lsp_get_at_offset(0);

    lsp_cons_t *cons = lsp_heap_get_cons(cons_ref);
    lsp_ref_t car_ref = cons->car;

    lsp_pop();
    lsp_push_ref(car_ref);
}

void lsp_cdr(void) {
    lsp_ref_t cons_ref = lsp_get_at_offset(0);

    lsp_cons_t *cons = lsp_heap_get_cons(cons_ref);
    lsp_ref_t cdr_ref = cons->cdr;

    lsp_pop();
    lsp_push_ref(cdr_ref);
}

void lsp_set_car(void) {
    lsp_ref_t cons_ref = lsp_get_at_offset(0);
    lsp_ref_t car_ref = lsp_get_at_offset(1);

    lsp_cons_t *cons = lsp_heap_get_cons(cons_ref);
    cons->car = car_ref;

    lsp_pop();
    lsp_pop();
}

void lsp_set_cdr(void) {
    lsp_ref_t cons_ref = lsp_get_at_offset(0);
    lsp_ref_t cdr_ref = lsp_get_at_offset(1);

    lsp_cons_t *cons = lsp_heap_get_cons(cons_ref);
    cons->cdr = cdr_ref;

    lsp_pop();
    lsp_pop();
}

void lsp_dup(int offset) {
    lsp_ref_t ref = lsp_get_at_offset(offset);
    lsp_push_ref(ref);
}

void lsp_store(int offset) {
    lsp_ref_t ref = lsp_get_at_offset(0);
    lsp_put_at_offset(ref, offset);

    lsp_pop();
}

void lsp_pop(void) {
    assert(ref_stack_ptr > ref_frame_ptr);
    ref_stack_ptr--;
}

void lsp_swp(int offset) {
    lsp_ref_t tgt = lsp_get_at_offset(offset);
    lsp_ref_t top = lsp_get_at_offset(0);

    lsp_put_at_offset(top, offset);
    lsp_put_at_offset(tgt, 0);
}

bool lsp_is_null(int offset) {
    lsp_ref_t ref = lsp_get_at_offset(offset);
    return lsp_heap_get_type(ref) == LSP_TYPE_NULL;
}

bool lsp_is_cons(int offset) {
    lsp_ref_t ref = lsp_get_at_offset(offset);
    return lsp_heap_get_type(ref) == LSP_TYPE_CONS;
}

bool lsp_is_int(int offset) {
    lsp_ref_t ref = lsp_get_at_offset(offset);
    return lsp_heap_get_type(ref) == LSP_TYPE_INT;
}

bool lsp_is_symbol(int offset) {
    lsp_ref_t ref = lsp_get_at_offset(offset);
    return lsp_heap_get_type(ref) == LSP_TYPE_SYM;
}

bool lsp_is_string(int offset) {
    lsp_ref_t ref = lsp_get_at_offset(offset);
    return lsp_heap_get_type(ref) == LSP_TYPE_STR;
}

bool lsp_is_op(int offset) {
    lsp_ref_t ref = lsp_get_at_offset(offset);
    return lsp_heap_get_type(ref) == LSP_TYPE_OP;
}

bool lsp_is_truthy(void) {
    lsp_ref_t ref = lsp_get_at_offset(0);
    switch (lsp_heap_get_type(ref)) {
        case LSP_TYPE_NULL:
            return false;
        case LSP_TYPE_CONS:
            return true;
        case LSP_TYPE_INT:
            return lsp_read_int(0) != 0;
        case LSP_TYPE_SYM:
            return true;
        case LSP_TYPE_STR:
            return strlen(lsp_borrow_string(0)) > 0;
        default:
            assert(false);
    }
}

size_t lsp_stats_frame_size(void) {
    assert(ref_frame_ptr >= 0);
    assert(ref_stack_ptr >= 0);
    assert(ref_frame_ptr <= ref_stack_ptr);
    return (size_t) (ref_stack_ptr - ref_frame_ptr);
}

size_t lsp_stats_stack_size(void) {
    assert(ref_stack_ptr >= 0);
    return (size_t) ref_stack_ptr;
}

