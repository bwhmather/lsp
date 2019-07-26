/**
 * Checks that `lsp_cdr` returns the second reference in a cons cell.
 */
#include "lsp.h"

#include "lspt.h"


int main(int argc, char **argv) {
    lsp_vm_init();

    lsp_push_int(2);
    lsp_push_int(1);

    lsp_cons();

    lsp_cdr();

    lspt_assert(lsp_read_int() == 2);

    return 0;
}
