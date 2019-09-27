/**
 * Checks that `lsp_eval` can read the value of a variable from its environment
 */
#include "lsp.h"

#include "lspt.h"


int main(void) {
    lsp_vm_init();

    lsp_push_symbol("symbol");

    lsp_push_empty_env();

    lsp_push_int(42);
    lsp_dup(-1);
    lsp_dup(-2);
    lsp_define();
    
    lsp_eval();

    lspt_assert(lsp_stats_frame_size() == 1);

    lspt_assert(lsp_read_int(0) == 42);

    return 0;
}
