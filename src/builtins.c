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
    lsp_expr_t *scope = NULL;

    scope = lsp_cons(lsp_cons(lsp_symbol("+"), lsp_op(&lsp_op_add)), scope);
    scope = lsp_cons(lsp_cons(lsp_symbol("-"), lsp_op(&lsp_op_sub)), scope);
    scope = lsp_cons(lsp_cons(lsp_symbol("*"), lsp_op(&lsp_op_mul)), scope);
    scope = lsp_cons(lsp_cons(lsp_symbol("/"), lsp_op(&lsp_op_div)), scope);

    lsp_expr_t *parent = NULL;

    lsp_expr_t *env = lsp_cons(scope, parent);

    return env;
}

