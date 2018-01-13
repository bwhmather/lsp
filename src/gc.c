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
 * Internal functions for actually performing a garbage collection.
 */
static void lsp_gc_internal_mark_heap() {
    // TODO
}

static void lsp_gc_internal_update_offset_cache() {
    // TODO
}

static void lsp_gc_internal_compact() {
    // TODO
}

static void lsp_gc_internal_update_stack() {
    // TODO
}

static void lsp_gc_collect() {
    lsp_gc_internal_mark_heap();
    lsp_gc_internal_rebuild_offset_cache();
    lsp_gc_internal_compact();
    lsp_gc_internal_update_stack();
}


/**
 * Stack operations.
 */
static lsp_heap_ref_t lsp_gc_internal_offset_to_ptr(int offset) {
    lsp_heap_ref_t *ptr = offset < 0 ? stack_ptr + offset : frame_ptr + offset;
    assert(ptr >= frame_ptr && ptr < stack_ptr);
    return ptr;
}

static lsp_heap_ref_t lsp_gc_internal_get(int offset) {
    lsp_heap_ref_t *src = lsp_gc_internal_offset_to_ptr(offset);
    return *src;
}

static void lsp_gc_internal_put(lsp_value_t *value, int offset) {
    lsp_heap_ref_t *tgt = lsp_gc_internal_offset_to_ptr(offset);
    *tgt = value;
}

static void lsp_gc_internal_push(lsp_value_t *value) {
    // TODO check for stack overflow.
    stack_ptr++;
    lsp_gc_internal_put(value, -1);
}

void lsp_gc_dup(int offset) {
    lsp_heap_ref_t ref = lsp_gc_internal_get(offset);
    lsp_gc_internal_push(ref);
}

void lsp_gc_store(int offset) {
    lsp_heap_ref_t ref = lsp_gc_internal_get(-1);
    lsp_gc_internal_put(ref, offset);
    lsp_gc_pop_to(-1);
}

void lsp_gc_pop_to(int offset) {
    lsp_heap_ref_t *tgt = lsp_gc_internal_offset_to_ptr(offset);
    stack_ptr = tgt;
}

void lsp_gc_swp(int offset) {
    lsp_heap_ref_t tgt = lsp_gc_internal_get(offset);
    lsp_heap_ref_t top = lsp_gc_internal_get(-1);

    lsp_gc_internal_put(top, offset);
    lsp_gc_internal_put(tgt, -1);
}

/**
 * Aborts the program if `ref` does not point to the start of a valid object
 * on the heap.
 */
static void lsp_gc_check_ref(lsp_heap_ref_t ref) {
    assert(ref >= heap_cursor);
    assert(!lsp_heap_is_continuation(ref));
}

/**
 * Returns the type of the object stored at offset `ref`.
 *
 * Will abort if `ref` does not point to the start of an object on the heap.
 */
unsigned int lsp_gc_type(int offset) {
    lsp_heap_ref_t ref = lsp_gc_internal_get(offset);
    lsp_gc_check_ref(ref);
    return (lsp_type_t)heap_metadata[ref].type;
}

/**
 * Returns a pointer to area of memory allocated on the heap for the object
 * stored at offset `ref`.
 */
char *lsp_gc_data(int offset) {
    lsp_heap_ref_t ref = lsp_gc_internal_get(offset);
    lsp_gc_check_ref(ref);
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
void lsp_gc_allocate(
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

    lsp_gc_internal_push(heap_cursor);

    // Update cursor.
    heap_cursor += num_blocks;
}

