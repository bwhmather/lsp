/**
 * Checks that `lsp_eval` can add two numbers.
 */
#include "lsp.h"

#include "lspt.h"


int main(void) {
    lsp_vm_init();

    lsp_push_null();
    lsp_push_int(2);
    lsp_cons();
    lsp_push_int(1);
    lsp_cons();
    lsp_push_symbol("+");
    lsp_cons();

    lsp_push_default_env();
    
    lsp_eval();

    lspt_assert(lsp_stats_frame_size() == 1);

    lspt_assert(lsp_read_int(0) == 3);

    return 0;
}
