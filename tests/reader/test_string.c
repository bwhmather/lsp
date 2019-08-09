/**
 * Checks that `lsp_parse` can read strings.
 */
#include "lsp.h"

#include "lspt.h"


int main(int argc, char **argv) {
    lsp_vm_init();

    lsp_push_string("\"Hello, World!\"");
    lsp_parse();

    lspt_expect(lsp_stats_frame_size() == 1);

    lsp_dup(0);
    lsp_car();
    char const *const string = lsp_borrow_string(0);
    lspt_assert(strcmp(string, "Hello, World!") == 0);
    lsp_pop();

    return 0;
}
