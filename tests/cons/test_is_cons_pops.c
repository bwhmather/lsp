/**
 * Checks that `lsp_is_cons` will pop its argument from the stack.
 */
#include "lsp.h"

#include "lspt.h"


int main(int argc, char **argv) {
    lsp_vm_init();

    lsp_push_cons();

    lspt_expect(lsp_is_cons());
    lspt_assert_eq(lsp_stats_frame_size(), 0);

    return 0;
}
