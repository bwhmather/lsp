#include "stack.h"

#include "heap.h"


static lsp_expr_t *stack_data;
static lsp_expr_t *frame_ptr;
static lsp_expr_t *stack_ptr;


static void lsp_push_expr(lsp_expr_t *expr) {
    *stack_ptr = expr;
    stack_ptr++;
}

void lsp_push_null() {
    lsp_push_expr(NULL);
}

void lsp_push_cons() {
    expr = lsp_heap_cons(NULL, NULL);
    lsp_push_expr(expr);
}

void lsp_push_op(void (* value)());
void lsp_push_int(int value);
void lsp_push_symbol(char *value);
void lsp_push_string(char *value);

int lsp_read_int() {

}

char *lsp_read_symbol();
char *lsp_read_string();

void lsp_cons() {
    lsp_expr_t *car = stack_ptr - 1;
    lsp_expr_t *cdr = stack_ptr - 2;
    lsp_pop_to(-2);
    lsp_push_expr(lsp_heap_cons(car, cdr));
}

void lsp_car() {
    lsp_expr_t *car = lsp_heap_car(stack_ptr - 1);
    lsp_pop_to(-1);
    lsp_push_expr(car);
}

void lsp_cdr() {
    lsp_expr_t *car = lsp_heap_car(stack_ptr - 1);
    lsp_pop_to(-1);
    lsp_push_expr(car);
}

void lsp_set_car() {
    lsp_expr_t *cons = stack_ptr - 1;
    lsp_expr_t *car = stack_ptr - 1;
    lsp_heap_set_car(cons, car);
    lsp_pop_to(-2);
}
void lsp_set_cdr() {
    lsp_expr_t *cons = stack_ptr - 1;
    lsp_expr_t *cdr = stack_ptr - 1;
    lsp_heap_set_cdr(cons, cdr);
    lsp_pop_to(-2);
}

void lsp_dup(int offset);
void lsp_store(int offset);
void lsp_pop()
void lsp_pop_to(int offset);

void lsp_call(int nargs);

bool lsp_is_null();
bool lsp_is_cons();
bool lsp_is_op();
bool lsp_is_int();
bool lsp_is_symbol();
bool lsp_is_string();

bool lsp_is_truthy();
bool lsp_is_equal();

