/**
 * Checks that `lsp_reverse` doesn't choke when called on an empty list.
 */
#include "lsp.h"

#include "lspt.h"


int main(void) {
    lsp_vm_init();

    lsp_push_null();

    lsp_reverse();

    lspt_assert(lsp_stats_frame_size() == 1);
    lspt_assert(lsp_is_null());

    return 0;
}
