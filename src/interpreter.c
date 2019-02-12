#include "lsp.h"

#include <assert.h>

/**
 * Calls the function at the top of the stack with the next `nargs` items as
 * its arguments.
 */
void lsp_call(int nargs) {
    if (lsp_dup(-1), lsp_is_op()) {
        // Pop the operation from the top of the stack.
        lsp_op_t op = lsp_read_op();

        // Evaluate it and then pop everything but the return value.
        lsp_fp_t rp = lsp_get_fp();
        lsp_shrink_frame(nargs);
        op();
        lsp_pop_to(1);
        lsp_restore_fp(rp);
        return;
    }

    if (lsp_dup(-1), lsp_is_cons()) {
        assert(false);
        // // Break the callable up into its component pieces.
        // lsp_expr_t *function = lsp_car(callable);
        // lsp_expr_t *arg_spec = lsp_car(function);
        // lsp_expr_t *body = lsp_cdr(function);
        // lsp_expr_t *closure = lsp_cdr(callable);

        // // Bind function arguments to a new environment.
        // lsp_expr_t *function_env = lsp_push_scope(closure);
        // while (lsp_type(arg_spec) == LSP_CONS) {
        //     lsp_define(
        //         lsp_as_sym(lsp_car(arg_spec)),
        //         lsp_car(args),
        //         function_env
        //     );
        //     arg_spec = lsp_cdr(arg_spec);
        //     args = lsp_cdr(args);
        // }

        // // Evaluate the function.
        // lsp_expr_t * result = lsp_eval(body, function_env);
    }

    assert(false);
}


