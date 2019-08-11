/**
 * Checks that `lsp_set_cdr` replaces the second reference in a cons cell.
 */
#include "lsp.h"

#include "lspt.h"


int main(void) {
    lsp_vm_init();

    lsp_push_cons();

    lsp_push_int(5);
    lsp_dup(-1);
    lsp_set_cdr();

    lsp_cdr();
    lspt_assert(lsp_read_int(0) == 5);

    return 0;
}
