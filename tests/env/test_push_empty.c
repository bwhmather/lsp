/**
 * Checks that `lsp_parse` can read nested lists.
 */
#include "lsp.h"

#include "lspt.h"


int main(void) {
    lsp_vm_init();

    lsp_push_empty_env();

    lspt_assert(lsp_stats_frame_size() == 1);

    lsp_dup(0);
    lsp_car();
    lspt_assert(lsp_is_null());
    lsp_pop();

    lsp_cdr();

    lsp_dup(0);
    lspt_assert(lsp_is_null());
    lsp_pop();

    return 0;
}
