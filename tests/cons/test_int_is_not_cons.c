/**
 * Checks that `lsp_is_cons` returns `false` when called on an integer.
 */
#include "lsp.h"

#include "lspt.h"


int main(int argc, char **argv) {
    lsp_vm_init();

    lsp_push_int(5);

    lspt_assert_not(lsp_is_cons());

    return 0;
}
