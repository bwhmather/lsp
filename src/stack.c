#include <stdlib.h>
#include <assert.h>

#include "stack.h"

#include "heap.h"


static lsp_value_t **stack_data;
static lsp_value_t **frame_ptr;
static lsp_value_t **stack_ptr;


static void lsp_push_expr(lsp_value_t *expr) {
    *stack_ptr = expr;
    stack_ptr++;
}

static lsp_value_t *lsp_get_at_offset(int offset) {
    lsp_value_t **src = offset < 0 ? stack_ptr + offset : frame_ptr + offset;
    assert(src >= frame_ptr && src < stack_ptr);
    return *src;
}

static void lsp_put_at_offset(lsp_value_t *value, int offset) {
    lsp_value_t **tgt = offset < 0 ? stack_ptr + offset : frame_ptr + offset;
    assert(tgt >= frame_ptr && tgt < stack_ptr);
    *tgt = value;
}

void lsp_push_null() {
    lsp_push_expr(NULL);
}

void lsp_push_cons() {
    lsp_value_t *expr = lsp_heap_cons(NULL, NULL);
    lsp_push_expr(expr);
}

void lsp_push_op(void (* value)());
void lsp_push_int(int value);
void lsp_push_symbol(char *value);
void lsp_push_string(char *value);

int lsp_read_int() {
    lsp_value_t *value = lsp_get_at_offset(-1);
    return lsp_heap_as_int(value);
}

char *lsp_read_symbol();
char *lsp_read_string();

void lsp_cons() {
    lsp_value_t *car = lsp_get_at_offset(-2);
    lsp_value_t *cdr = lsp_get_at_offset(-1);
    lsp_value_t *cons = lsp_heap_cons(car, cdr);
    lsp_pop_to(-2);
    lsp_push_expr(cons);
}

void lsp_car() {
    lsp_value_t *cons = lsp_get_at_offset(-1);
    lsp_value_t *car = lsp_heap_car(cons);
    lsp_pop_to(-1);
    lsp_push_expr(car);
}

void lsp_cdr() {
    lsp_value_t *cons = lsp_get_at_offset(-1);
    lsp_value_t *cdr = lsp_heap_car(cons);
    lsp_pop_to(-1);
    lsp_push_expr(cdr);
}

void lsp_set_car() {
    lsp_value_t *cons = lsp_get_at_offset(-2);
    lsp_value_t *car = lsp_get_at_offset(-1);
    lsp_heap_set_car(cons, car);
    lsp_pop_to(-2);
}
void lsp_set_cdr() {
    lsp_value_t *cons = lsp_get_at_offset(-2);
    lsp_value_t *cdr = lsp_get_at_offset(-1);
    lsp_heap_set_cdr(cons, cdr);
    lsp_pop_to(-2);
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

void lsp_call(int nargs);

bool lsp_is_null();
bool lsp_is_cons();
bool lsp_is_op();
bool lsp_is_int();
bool lsp_is_symbol();
bool lsp_is_string();

bool lsp_is_truthy();
bool lsp_is_equal();

