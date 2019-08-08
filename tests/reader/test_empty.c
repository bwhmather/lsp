/**
 * Checks that `lsp_parse` won't blow up on an empty string
 */
#include "lsp.h"

#include "lspt.h"


int main(int argc, char **argv) {
    lsp_vm_init();

    lsp_push_string("");
    lsp_parse();

    lspt_assert(lsp_stats_frame_size() == 1);
    lspt_assert(lsp_is_null());

    return 0;
}
