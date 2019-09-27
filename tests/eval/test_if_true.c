/**
 * Checks that `lsp_eval` can evaluate a trivial if statement.
 */
#include "lsp.h"

#include "lspt.h"


int main(void) {
    lsp_vm_init();

    lsp_push_null();
    lsp_push_string("alternate");
    lsp_cons();
    lsp_push_string("subsequent");
    lsp_cons();
    lsp_push_int(1);
    lsp_cons();
    lsp_push_symbol("if");
    lsp_cons();

    lsp_push_empty_env();

    lsp_eval();

    lspt_assert(lsp_stats_frame_size() == 1);

    char const *const string = lsp_borrow_string(0);
    lspt_assert(strcmp(string, "subsequent") == 0);

    return 0;
}
