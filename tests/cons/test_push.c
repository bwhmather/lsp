/**
 * Checks that `lsp_push_cons` pushes an empty cons cell.
 */
#include "lsp.h"

#include "lspt.h"


int main(void) {
    lsp_vm_init();

    lsp_push_cons();

    lspt_assert(lsp_stats_frame_size() == 1);

    lsp_dup(-1);
    lspt_assert(lsp_is_cons(0));

    lsp_dup(-1);
    lsp_car();
    lspt_assert(lsp_is_null(0));

    lsp_dup(-1);
    lsp_cdr();
    lspt_assert(lsp_is_null(0));

    return 0;
}
