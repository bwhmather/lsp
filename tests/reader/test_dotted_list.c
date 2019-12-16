/**
 * Checks that `lsp_parse` can read a list terminated by a pair rather than a
 * null.
 */
#include "lsp.h"

#include "lspt.h"


int main(void) {
    lsp_vm_init();

    lsp_push_string("(1 2 . 3)");
    lsp_parse();

    lspt_expect(lsp_stats_frame_size() == 1);

    lsp_dup(0);
    lsp_cdr();
    lspt_expect(lsp_is_null(0));
    lsp_pop();

    lsp_car();

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
    lspt_assert(lsp_read_int(0) == 3);

    return 0;
}
