#include "builtins.h"

#include "heap.h"

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

lsp_expr_t *lsp_default_env() {
    lsp_expr_t *env = NULL;

    env = lsp_cons(lsp_cons(lsp_symbol("+"), lsp_op(&lsp_op_add)), env);
    env = lsp_cons(lsp_cons(lsp_symbol("-"), lsp_op(&lsp_op_sub)), env);
    env = lsp_cons(lsp_cons(lsp_symbol("*"), lsp_op(&lsp_op_mul)), env);
    env = lsp_cons(lsp_cons(lsp_symbol("/"), lsp_op(&lsp_op_div)), env);

    return env;
}

