#include "lsp.h"

#include "lspt.h"


int main(int argc, char **argv) {
    lsp_vm_init();

    lsp_push_null();

    lsp_reverse();

    lspt_assert_eq(lsp_stats_frame_size(), 1);
    lspt_assert(lsp_is_null());

    return 0;
}
