/**
 * Checks that `lsp_parse` can read nested lists.
 */
#include "lsp.h"

#include "lspt.h"


int main(void) {
    lsp_vm_init();

    lsp_push_string("((1 2) () 3 (4))");
    lsp_parse();

    lspt_expect(lsp_stats_frame_size() == 1);

    lsp_dup(0);
    lsp_car();
    /* BEGIN */
    lsp_dup(0);
    lsp_car();
    /* BEGIN */
    lsp_dup(0);
    lsp_car();
    lspt_assert(lsp_read_int(0) == 1);
    lsp_pop();

    lsp_cdr();

    lsp_dup(0);
    lsp_car();
    lspt_assert(lsp_read_int(0) == 2);
    lsp_pop();

    lsp_cdr();

    lsp_dup(0);
    lspt_assert(lsp_is_null());
    lsp_pop();
    /* END */
    lsp_pop();

    lsp_cdr();

    lsp_dup(0);
    lsp_car();
    lspt_assert(lsp_is_null());
    lsp_pop();

    lsp_cdr();

    lsp_dup(0);
    lsp_car();
    lspt_assert(lsp_read_int(0) == 3);
    lsp_pop();

    lsp_cdr();

    lsp_dup(0);
    lsp_car();
    /* BEGIN */
    lsp_dup(0);
    lsp_car();
    lspt_assert(lsp_read_int(0) == 4);
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
    /* END */
    lsp_pop();

    lsp_cdr();

    lsp_dup(0);
    lspt_expect(lsp_is_null());
    lsp_pop();

    return 0;
}
