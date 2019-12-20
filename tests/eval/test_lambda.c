/**
 * Checks that `lsp_eval` can evaluate a lambda expression to a closure.
 */
#include "lsp.h"

#include "lspt.h"

void lsp_op_eval_lambda(void);


int main(void) {
    lsp_vm_init();

    lsp_push_string("(lambda (x) (* 2 x))");
    lsp_parse();
    lsp_car();

    lsp_push_empty_env();

    lsp_eval();

    lspt_assert(lsp_stats_frame_size() == 1);

    lsp_push_string("((<builtin> . ((x) (* 2 x))) . (() . ()))");
    lsp_parse();
    lsp_car();

    // Swap `<builtin>` for real reference to `lsp_op_eval_lambda`.
    lsp_push_op(lsp_op_eval_lambda);
    lsp_dup(1);
    lsp_car();
    lsp_set_car();

    lsp_print_stack();
    lspt_assert_equal();
    lsp_pop();
    lsp_pop();

    return 0;
}
