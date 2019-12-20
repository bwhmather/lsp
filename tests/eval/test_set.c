/**
 * Checks that `lsp_eval` can replace an existing binding.
 */
#include "lsp.h"

#include "lspt.h"


int main(void) {
    lsp_vm_init();

    lsp_push_empty_env();
    
    // Create a definition in the current scope.
    lsp_push_int(4);
    lsp_push_symbol("x");
    lsp_dup(-1);
    lsp_define();

    lsp_push_string("(set! x 6)");
    lsp_parse();
    lsp_car();

    lsp_dup(-1);
    lsp_eval();

    lspt_assert(lsp_stats_frame_size() == 2);
    lspt_assert(lsp_is_null(0));
    lsp_pop();

    lsp_push_symbol("x");
    lsp_swp(1);
    lsp_lookup();

    lspt_assert(lsp_read_int(0) == 6);

    return 0;
}
