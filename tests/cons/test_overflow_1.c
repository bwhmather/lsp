/**
 * Checks that `lsp_cons` aborts when called with only one argument.
 */
#include "lsp.h"

#include "lspt.h"


int main(int argc, char **argv) {
    lsp_vm_init();

    lsp_push_int(1);

    lspt_assert_aborts(lsp_cons());
}
