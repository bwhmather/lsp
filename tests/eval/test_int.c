/**
 * Checks that `lsp_eval` will evaluate an integer to itself.
 */
#include "lsp.h"

#include "lspt.h"


int main(void) {
    lsp_vm_init();

    lsp_push_int(12);

    lsp_push_empty_env();
    
    lsp_eval();

    lspt_assert(lsp_stats_frame_size() == 1);

    lspt_assert(lsp_read_int(0) == 12);

    return 0;
}
