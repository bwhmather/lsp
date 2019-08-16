/**
 * Checks that `lsp_set` is able to replace a single variable in a single
 * scope.
 */
#include "lsp.h"

#include "lspt.h"


int main(void) {
    lsp_vm_init();

    lsp_push_empty_env();
    
    lsp_push_string("initial");
    lsp_push_symbol("variable");
    lsp_dup(-1);
    lsp_define();

    lsp_push_string("updated");
    lsp_push_symbol("variable");
    lsp_dup(-1);
    lsp_set();

    lspt_expect(lsp_stats_frame_size() == 1);

    lsp_dup(0);
    lsp_print();

    lsp_push_null();
    lsp_push_null();
    lsp_push_string("updated");
    lsp_push_symbol("variable");
    lsp_cons();
    lsp_cons();
    lsp_cons();

    lspt_assert_equal();

    return 0;
}
