/**
 * Checks that `lsp_parse` can read a list of nils.
 */
#include "lsp.h"

#include "lspt.h"


int main(void) {
    lsp_vm_init();

    lsp_push_string("(() ())");
    lsp_parse();

    lspt_expect(lsp_stats_frame_size() == 1);

    lsp_dup(0);
    lsp_car();
    /* BEGIN */
    lsp_dup(0);
    lsp_car();
    lspt_assert(lsp_is_null(0));
    lsp_pop();

    lsp_cdr();

    lsp_dup(0);
    lsp_car();
    lspt_assert(lsp_is_null(0));
    lsp_pop();

    lsp_cdr();

    lsp_dup(0);
    lspt_assert(lsp_is_null(0));
    lsp_pop();
    /* END */
    lsp_pop();

    lsp_cdr();

    lsp_dup(0);
    lspt_assert(lsp_is_null(0));
    lsp_pop();

    return 0;
}
