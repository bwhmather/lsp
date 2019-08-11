/**
 * Checks that `lsp_is_cons` does not pop its argument from the stack.
 */
#include "lsp.h"

#include "lspt.h"


int main(void) {
    lsp_vm_init();

    lsp_push_cons();

    lspt_expect(lsp_is_cons());
    lspt_assert(lsp_stats_frame_size() == 1);

    return 0;
}
