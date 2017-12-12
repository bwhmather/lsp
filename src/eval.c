#include "eval.h"

#include <string.h>
#include <assert.h>


static lsp_expr_t *lsp_lookup(char *sym, lsp_expr_t *env) {
    // Try to find symbol in local scope.
    lsp_expr_t *scope = lsp_car(env);

    while (lsp_type(scope) == LSP_CONS) {
        if (strcmp(
            lsp_as_sym(lsp_car(lsp_car(scope))),
            lsp_as_sym(sym)
        ) == 0) {
            return lsp_cdr(lsp_car(scope));
        }
        scope = lsp_cdr(scope);
    }

    // Fallback to searching in parent scope.
    lsp_expr_t *parent_scope = lsp_cdr(env);
    return lsp_lookup(sym, parent_scope);
}


lsp_expr_t *lsp_eval(lsp_expr_t *expr, lsp_expr_t *env) {
    if (lsp_type(expr) == LSP_SYM) {
        // Expression is a name identifying a variable that can be loaded
        // from the environment.
        return lsp_lookup(expr, env);

    } else if (lsp_type(expr) == LSP_CONS) {
        // Expression is a list representing either a special form or an
        // invocation of a procedure or built-in operator.
        if (lsp_type(lsp_car(expr)) == LSP_SYM) {
            char *sym = lsp_as_sym(lsp_car(expr));
            if (strcmp(sym, "if") == 0) {
                lsp_expr_t *cursor = lsp_cdr(expr);

                lsp_expr_t *predicate = lsp_car(cursor);
                cursor = lsp_cdr(cursor);

                lsp_expr_t *subsequent = lsp_car(cursor);
                cursor = lsp_cdr(cursor);

                lsp_expr_t *alternate = lsp_car(cursor);
                cursor = lsp_cdr(cursor);

                if (lsp_is_truthy(lsp_eval(predicate, env))) {
                    return lsp_eval(subsequent, env);
                } else {
                    return lsp_eval(alternate, env);
                }
            }
            if (strcmp(sym, "quote") == 0) {
                return lsp_car(lsp_cdr(expr));
            }
            if (strcmp(sym, "define") == 0) {
                assert(false);
            }
            if (strcmp(sym, "set!") == 0) {
                assert(false);
            }
            if (strcmp(sym, "lambda") == 0) {
                assert(false);
            }
        }

        // Expression is not a special form.  We evaluate all items in the
        // list and then pass the tail to the built-in or procedure
        // represented by the first item.
        lsp_expr_t *evaled = NULL;
        lsp_expr_t *cursor = expr;
        while (lsp_type(cursor) == LSP_CONS) {
            evaled = lsp_cons(lsp_eval(lsp_car(cursor), env), evaled);
            cursor = lsp_cdr(cursor);
        }
        evaled = lsp_reverse(evaled);

        lsp_expr_t *callable = lsp_car(evaled);
        lsp_expr_t *args = lsp_cdr(evaled);

        if (lsp_type(callable) == LSP_OP) {
            // Expression is a call to a built-in procedure represented by a
            // function pointer.
            lsp_op_t op = *lsp_as_op(callable);
            return op(args);
        } else {
            assert(false);
        }

    } else {
        // Expression is a literal that can be returned as-is.
        return expr;
    }
}

