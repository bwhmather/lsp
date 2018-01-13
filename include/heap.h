#pragma once

#include <stdbool.h>


void lsp_heap_init();

/**
 * Stack operations.
 */
void lsp_dup(int offset);
void lsp_store(int offset);
void lsp_pop_to(int offset);
void lsp_swp(int offset);

/**
 * Allocates a contiguous area of memory for an object.
 *
 * Currently all blocks making up an object must be marked with the same type,
 * and must all be pointers or all be literals values.
 *
 * All bytes within the allocated area of memory will be initialised to zero.
 *
 * Returns the frame offset of the reference to the object on the stack.
 *
 * .. warning::
 *     Allocation may trigger a garbage collection cycle.  All non-stack
 *     references to objects stored on the heap should be released before
 *     calling this function.
 */
int lsp_heap_allocate(
    unsigned int size, unsigned int type, bool is_pointer,
);

/**
 * Returns the type of the object pointed to the stack entry at `offset`.
 */
unsigned int lsp_heap_type(int offset);

/**
 * Returns a pointer to area of memory allocated on the heap for the object
 * pointed to by the stack entry at `offset`.
 */
char *lsp_heap_data(int offset);
