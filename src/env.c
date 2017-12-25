#include "env.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>


void lsp_define(char *sym, lsp_expr_t *val, lsp_expr_t *env) {
    lsp_set_car(env, lsp_cons(lsp_cons(lsp_symbol(sym), val), lsp_car(env)));
}


lsp_expr_t *lsp_lookup(char *sym, lsp_expr_t *env) {
    // Try to find symbol in local scope.
    lsp_expr_t *scope = lsp_car(env);

    while (lsp_type(scope) == LSP_CONS) {
        if (strcmp(lsp_as_sym(lsp_car(lsp_car(scope))), sym) == 0) {
            return lsp_cdr(lsp_car(scope));
        }
        scope = lsp_cdr(scope);
    }

    // Fallback to searching in parent scope.
    lsp_expr_t *parent_scope = lsp_cdr(env);
    return lsp_lookup(sym, parent_scope);
}


void lsp_set(char *sym, lsp_expr_t *val, lsp_expr_t *env) {
    assert(env != NULL);

    // Try to find symbol in local scope.
    lsp_expr_t *scope = lsp_car(env);

    while (lsp_type(scope) == LSP_CONS) {
        if (strcmp(lsp_as_sym(lsp_car(lsp_car(scope))), sym) == 0) {
            lsp_set_cdr(lsp_car(scope), val);
            return;
        }
        scope = lsp_cdr(scope);
    }

    // Fallback to attempting to set the value in the parent scope.
    lsp_expr_t *parent_scope = lsp_cdr(env);
    lsp_set(sym, val, parent_scope);
}


