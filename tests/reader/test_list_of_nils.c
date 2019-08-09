/**
 * Checks that `lsp_parse` can read a list of nils.
 */
#include "lsp.h"

#include "lspt.h"


int main(int argc, char **argv) {
    lsp_vm_init();

    lsp_push_string("(() ())");
    lsp_parse();

    lspt_expect(lsp_stats_frame_size() == 1);

    lsp_dup(0);
    lsp_car();
    /* BEGIN */
    lsp_dup(0);
    lsp_car();
    lspt_assert(lsp_is_null());
    lsp_pop();

    lsp_cdr();

    lsp_dup(0);
    lsp_car();
    lspt_assert(lsp_is_null());
    lsp_pop();

    lsp_cdr();

    lsp_dup(0);
    lspt_assert(lsp_is_null());
    lsp_pop();
    /* END */
    lsp_pop();

    lsp_cdr();

    lsp_dup(0);
    lspt_assert(lsp_is_null());
    lsp_pop();

    return 0;
}
