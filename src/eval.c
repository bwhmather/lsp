#include "eval.h"

#include "heap.h"
#include "env.h"

#include <string.h>
#include <assert.h>


lsp_expr_t *lsp_eval(lsp_expr_t *expr, lsp_expr_t *env) {
    if (lsp_type(expr) == LSP_SYM) {
        // Expression is a name identifying a variable that can be loaded
        // from the environment.
        return lsp_lookup(lsp_as_sym(expr), env);

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
                lsp_define(lsp_as_sym(lsp_caar(expr)), lsp_caaar(expr), env);
                return NULL;
            }
            if (strcmp(sym, "set!") == 0) {
                lsp_set(lsp_as_sym(lsp_caar(expr)), lsp_caaar(expr), env);
                return NULL;
            }
            if (strcmp(sym, "lambda") == 0) {
                lsp_expr_t *arg_spec = lsp_caar(expr);
                lsp_expr_t *body = lsp_caaar(expr);
                lsp_expr_t *result = lsp_cons(lsp_cons(arg_spec, body), env);
                return result;
            }
            if (strcmp(sym, "begin") == 0) {
                lsp_expr_t *result = NULL;
                for (
                    lsp_expr_t *statements = lsp_cdr(expr);
                    statements != NULL;
                    statements = lsp_cdr(statements)
                ) {
                    result = lsp_eval(lsp_car(statements), env);
                }
                return result;
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
        } else if (lsp_type(callable) == LSP_CONS) {
            lsp_expr_t *function = lsp_car(callable);
            lsp_expr_t *arg_spec = lsp_car(function);
            lsp_expr_t *body = lsp_cdr(function);

            lsp_expr_t *bindings = NULL;
            while (lsp_type(arg_spec) == LSP_CONS) {
                bindings = lsp_cons(
                    lsp_cons(lsp_car(arg_spec), lsp_car(args)),
                    bindings
                );
                arg_spec = lsp_cdr(arg_spec);
                args = lsp_cdr(args);
            }
            bindings = lsp_reverse(bindings);

            lsp_expr_t *closure = lsp_cdr(callable);

            lsp_expr_t *function_env = lsp_cons(bindings, closure);

            lsp_expr_t * result = lsp_eval(body, function_env);

            return result;
        } else {
            assert(false);
        }

    } else {
        // Expression is a literal that can be returned as-is.
        return expr;
    }
}

