/**
 * Checks that `lsp_set_cdr` replaces the second reference in a cons cell.
 */
#include "lsp.h"

#include "lspt.h"


int main(int argc, char **argv) {
    lsp_vm_init();

    lsp_push_cons();

    lsp_dup(0);
    lsp_fp_t rp = lsp_get_fp();
    lsp_shrink_frame(1);

    lsp_push_int(5);
    lsp_set_cdr();

    lspt_expect_eq(lsp_stats_frame_size(), 0);
    lsp_restore_fp(rp);

    lsp_cdr();
    lspt_assert_eq(lsp_read_int(), 5);

    return 0;
}
