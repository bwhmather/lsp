/**
 * Checks that `lsp_eval` can evaluate a simple begin expression.
 */
#include "lsp.h"

#include "lspt.h"

void lsp_op_eval_lambda(void);


int main(void) {
    lsp_vm_init();

    lsp_push_string("(begin (define x 4) (set! x 3) (define y 5) (+ x y))");
    lsp_parse();
    lsp_car();

    lsp_push_default_env();

    lsp_eval();

    lspt_assert(lsp_stats_frame_size() == 1);

    lsp_print_stack();

    lspt_assert(lsp_read_int(0) == 8);

    return 0;
}
