/**
 * Checks that `lsp_eval` can add a new binding to the current environment.
 */
#include "lsp.h"

#include "lspt.h"


int main(void) {
    lsp_vm_init();

    lsp_push_empty_env();
    
    lsp_push_null();
    lsp_push_int(5);
    lsp_cons();
    lsp_push_symbol("variable");
    lsp_cons();
    lsp_push_symbol("define");
    lsp_cons();

    lsp_dup(-1);
    lsp_eval();

    lspt_assert(lsp_stats_frame_size() == 2);
    lspt_assert(lsp_is_null(0));
    lsp_pop();

    lsp_push_null();
    lsp_push_null();
    lsp_push_int(5);
    lsp_push_symbol("variable");
    lsp_cons();
    lsp_cons();
    lsp_cons();

    lspt_assert_equal();

    return 0;
}
