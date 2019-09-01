/**
 * Checks that `lsp_push_scope` will add a new frame to the environment.
 */
#include "lsp.h"

#include "lspt.h"


int main(void) {
    lsp_vm_init();

    lsp_push_empty_env();

    lspt_assert(lsp_stats_frame_size() == 1);

    lsp_push_scope();

    // Current scope should be empty.
    lsp_dup(0);
    lsp_car();
    lspt_assert(lsp_is_null(0));
    lsp_pop();

    lsp_cdr();

    // Parent scope should be empty.
    lsp_dup(0);
    lsp_car();
    lspt_assert(lsp_is_null(0));
    lsp_pop();

    lsp_cdr();

    // Terminating null.
    lsp_dup(0);
    lspt_assert(lsp_is_null(0));
    lsp_pop();

    return 0;
}
