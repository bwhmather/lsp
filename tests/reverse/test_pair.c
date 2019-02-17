#include "lsp.h"

#include "lspt.h"


int main(int argc, char **argv) {
    lsp_vm_init();

    lsp_push_int(12);
    lsp_push_null();
    lsp_cons();

    lsp_reverse();

    lspt_assert_eq(lsp_stats_frame_size(), 1);

    lsp_dup(0);
    lspt_assert(lsp_is_cons());

    lsp_dup(0);
    lsp_car();
    lspt_assert_eq(lsp_read_int(), 12);

    lsp_dup(0);
    lsp_cdr();
    lspt_assert(lsp_is_null());

    return 0;
}

