/**
 * Checks that `lsp_eval` can add a new binding to a nested scope.
 */
#include "lsp.h"

#include "lspt.h"


int main(void) {
    lsp_vm_init();
    
    lsp_push_default_env();
    lsp_dup(0);
    lsp_push_scope();

    lsp_push_string("(define x 4)");
    lsp_parse();
    lsp_car();

    lsp_dup(1);

    lsp_eval();

    lspt_assert(lsp_stats_frame_size() == 3);

    // Check that variable is bound in nested scope.
    lspt_assert(lsp_is_null(0));
    lsp_pop();

    lsp_push_string("x");
    lsp_parse();
    lsp_car();

    lsp_dup(1);

    lsp_eval();

    lspt_assert(lsp_read_int(0) == 4);

    // Check that variable is not bound in outer scope.
    lsp_pop();
    lsp_pop();

    lsp_push_string("x");
    lsp_parse();
    lsp_car();

    lsp_dup(1);

    lspt_assert_aborts(lsp_eval());

    return 0;
}
