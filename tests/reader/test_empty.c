/**
 * Checks that `lsp_parse` won't blow up on an empty string
 */
#include "lsp.h"

#include "lspt.h"


int main(void) {
    lsp_vm_init();

    lsp_push_string("");
    lsp_parse();

    lspt_assert(lsp_stats_frame_size() == 1);
    lspt_assert(lsp_is_null(0));

    return 0;
}
