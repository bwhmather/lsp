#include "gc.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>


/**
 * An offset into the heap array.
 *
 * Used to represent a reference to an object stored on the heap.
 */
typedef uint32_t lsp_ref_t;


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
} lsp_heap_block_t;

static lsp_heap_block_t *heap_data;
static lsp_heap_meta_t *heap_metadata;
static lsp_ref_t heap_cursor;

static lsp_ref_t *stack;
static lsp_ref_t *frame_ptr;
static lsp_ref_t *stack_ptr;

static lsp_ref_t *heap_mark_stack;
static uint64_t *heap_mark_bitset;
static lsp_ref_t *heap_offset_cache;


void lsp_heap_init() {
    // TODO This doesn't work if overcommit is disabled.
    // Block size times maximum index.
    heap_data = (lsp_heap_block_t *)malloc(8 * 4294967296);
    // Metadata block size times maximum index.
    heap_metadata = (lsp_heap_meta_t *)malloc(
        sizeof(lsp_heap_meta_t) * 4294967296
    );
    heap_cursor = 0;

    // Size of a reference times an arbitrary stack size.
    stack = (lsp_ref_t *malloc(sizeof(lsp_ref_t) * 1024);
    frame_ptr = stack;
    stack_ptr = stack;

    // TODO I think the worst case is much lower.
    heap_mark_stack = (lsp_ref_t *)malloc(sizeof(lsp_ref_t) * 4294967296);
    // One bit times the maximum number of blocks.
    heap_mark_bitset = (uint64_t *)malloc(4294967296 / 8)
    // One offset for every bit in the bitset.
    heap_offset_cache = (heap_ref_t *)malloc(
        sizeof(heap_ref_t) * (4294967296 / 8)
    )

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
 * Stack operations.
 */
static lsp_ref_t *lsp_gc_internal_offset_to_ptr(int offset) {
    lsp_ref_t *ptr = offset < 0 ? stack_ptr + offset : frame_ptr + offset;
    assert(ptr >= frame_ptr && ptr < stack_ptr);
    return ptr;
}

static lsp_ref_t lsp_gc_internal_get(int offset) {
    lsp_ref_t *src = lsp_gc_internal_offset_to_ptr(offset);
    return *src;
}

static void lsp_gc_internal_put(lsp_ref_t value, int offset) {
    lsp_ref_t *tgt = lsp_gc_internal_offset_to_ptr(offset);
    *tgt = value;
}

static void lsp_gc_internal_push(lsp_ref_t value) {
    // TODO check for stack overflow.
    stack_ptr++;
    lsp_gc_internal_put(value, -1);
}

void lsp_gc_dup(int offset) {
    lsp_ref_t ref = lsp_gc_internal_get(offset);
    lsp_gc_internal_push(ref);
}

void lsp_gc_store(int offset) {
    lsp_ref_t ref = lsp_gc_internal_get(-1);
    lsp_gc_internal_put(ref, offset);
    lsp_gc_pop_to(-1);
}

void lsp_gc_pop_to(int offset) {
    lsp_ref_t *tgt = lsp_gc_internal_offset_to_ptr(offset);
    stack_ptr = tgt;
}

void lsp_gc_swp(int offset) {
    lsp_ref_t tgt = lsp_gc_internal_get(offset);
    lsp_ref_t top = lsp_gc_internal_get(-1);

    lsp_gc_internal_put(top, offset);
    lsp_gc_internal_put(tgt, -1);
}

/**
 * Aborts the program if `ref` does not point to the start of a valid object
 * on the heap.
 */
static void lsp_gc_check_ref(lsp_ref_t ref) {
    assert(ref <= heap_cursor);
    assert(!heap_metadata[ref].is_continuation);
}

/**
 * Returns the type of the object stored at offset `ref`.
 *
 * Will abort if `ref` does not point to the start of an object on the heap.
 */
unsigned int lsp_gc_type(int offset) {
    lsp_ref_t ref = lsp_gc_internal_get(offset);
    lsp_gc_check_ref(ref);
    return heap_metadata[ref].type;
}

/**
 * Returns a pointer to area of memory allocated on the heap for the object
 * stored at offset `ref`.
 */
char *lsp_gc_data(int offset) {
    lsp_ref_t ref = lsp_gc_internal_get(offset);
    lsp_gc_check_ref(ref);
    return (char *)(&heap_data[ref]);
}

/**
 * Allocates a contiguous area of memory for an object and pushes a reference
 * to it onto the gc stack.
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
    unsigned int size, unsigned int type, bool is_pointer
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
    for (lsp_ref_t i = 1; i < num_blocks; i++) {
        heap_metadata[heap_cursor + i].is_pointer = is_pointer;
        heap_metadata[heap_cursor + i].is_continuation = true;
        heap_metadata[heap_cursor + i].type = type;
    }

    lsp_gc_internal_push(heap_cursor);

    // Update cursor.
    heap_cursor += num_blocks;
}

