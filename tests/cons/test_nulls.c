/**
 * Checks that `lsp_cons` pops it's arguments and pushes a cons cell.
 */
#include "lsp.h"

#include "lspt.h"


int main(void) {
    lsp_vm_init();

    lsp_push_null();
    lsp_push_null();

    lsp_cons();

    lspt_assert(lsp_stats_frame_size() == 1);
    lspt_assert(lsp_is_cons(0));

    return 0;
}
