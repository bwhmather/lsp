#include "builtins.h"

#include "heap.h"
#include "env.h"

#include <stdlib.h>
#include <assert.h>


static lsp_expr_t *lsp_op_add(lsp_expr_t *args) {
    int a = *lsp_as_int(lsp_car(args));
    int b = *lsp_as_int(lsp_car(lsp_cdr(args)));
    return lsp_int(a + b);
}

static lsp_expr_t *lsp_op_sub(lsp_expr_t *args) {
    int a = *lsp_as_int(lsp_car(args));
    int b = *lsp_as_int(lsp_car(lsp_cdr(args)));
    return lsp_int(a - b);
}

static lsp_expr_t *lsp_op_mul(lsp_expr_t *args) {
    int a = *lsp_as_int(lsp_car(args));
    int b = *lsp_as_int(lsp_car(lsp_cdr(args)));
    return lsp_int(a * b);
}

static lsp_expr_t *lsp_op_div(lsp_expr_t *args) {
    int a = *lsp_as_int(lsp_car(args));
    int b = *lsp_as_int(lsp_car(lsp_cdr(args)));
    return lsp_int(a / b);
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

