#include "builtins.h"

#include "heap.h"
#include "env.h"

#include <stdlib.h>
#include <assert.h>


static void lsp_op_add() {
    int a = lsp_read_int(0);
    int b = lsp_read_int(1);
    lsp_remove(0);
    lsp_push_int(a + b);
}

static void lsp_op_sub() {
    int a = lsp_read_int(0);
    int b = lsp_read_int(1);
    lsp_remove(0);
    lsp_push_int(a - b);
}

static void lsp_op_mul() {
    int a = lsp_read_int(0);
    int b = lsp_read_int(1);
    lsp_remove(0);
    lsp_push_int(a * b);
}

static void lsp_op_div() {
    int a = lsp_read_int(0);
    int b = lsp_read_int(1);
    lsp_remove(0);
    lsp_push_int(a / b);
}

static void lsp_op_cons() {
    int a = lsp_read_int(0);
    int b = lsp_read_int(1);
    lsp_remove(0);
    lsp_push_int(a / b);
    lsp_push_cons()
}
static lsp_expr_t *lsp_op_cons(lsp_expr_t *args) {
    lsp_expr_t *car = lsp_car(args);
    lsp_expr_t *cdr = lsp_caar(args);
    return lsp_cons(car, cdr);
}

static lsp_expr_t *lsp_op_car(lsp_expr_t *args) {
    lsp_expr_t *cons = lsp_car(args);
    return lsp_car(cons);
}

static lsp_expr_t *lsp_op_set_car(lsp_expr_t *args) {
    lsp_expr_t *cons = lsp_car(args);
    lsp_expr_t *value = lsp_caar(args);
    lsp_set_car(cons, value);
    return NULL;
}

static lsp_expr_t *lsp_op_cdr(lsp_expr_t *args) {
    lsp_expr_t *cons = lsp_car(args);
    return lsp_cdr(cons);
}

static lsp_expr_t *lsp_op_set_cdr(lsp_expr_t *args) {
    lsp_expr_t *cons = lsp_car(args);
    lsp_expr_t *value = lsp_caar(args);
    lsp_set_cdr(cons, value);
    return NULL;
}


lsp_expr_t *lsp_default_env() {
    lsp_expr_t *env = lsp_empty_env();

    lsp_define("+", lsp_op(&lsp_op_add), env);
    lsp_define("-", lsp_op(&lsp_op_sub), env);
    lsp_define("*", lsp_op(&lsp_op_mul), env);
    lsp_define("/", lsp_op(&lsp_op_div), env);
    lsp_define("cons", lsp_op(&lsp_op_cons), env);
    lsp_define("car", lsp_op(&lsp_op_car), env);
    lsp_define("set-car!", lsp_op(&lsp_op_set_car), env);
    lsp_define("cdr", lsp_op(&lsp_op_cdr), env);
    lsp_define("set-cdr!", lsp_op(&lsp_op_set_cdr), env);

    return env;
}

