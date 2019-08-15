/**
 * Checks that `lsp_lookup` is able to read a variable that shadows an outer
 * scope.
 */
#include "lsp.h"

#include "lspt.h"


int main(void) {
    lsp_vm_init();

    lsp_push_empty_env();
    
    lsp_push_int(5);
    lsp_push_symbol("variable");
    lsp_dup(-1);
    lsp_define();

    lsp_push_scope();
    
    lsp_push_int(50);
    lsp_push_symbol("variable");
    lsp_dup(-1);
    lsp_define();

    lspt_expect(lsp_stats_frame_size() == 1);

    lsp_push_symbol("variable");
    lsp_dup(-1);
    lsp_lookup();

    lspt_assert(lsp_stats_frame_size() == 2);

    lspt_assert(lsp_read_int(0) == 50);

    return 0;
}
