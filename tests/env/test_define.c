/**
 * Checks that `lsp_define` can add a single binding to an empty env.
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

    lspt_assert(lsp_stats_frame_size() == 1);

    lsp_push_null();
    lsp_push_null();
    lsp_push_int(5);
    lsp_push_symbol("variable");
    lsp_cons();
    lsp_cons();
    lsp_cons();

    lspt_assert_equal();

    return 0;
}
