/**
 * Checks that `lsp_is_cons` returns `false` when called on a null reference.
 */
#include "lsp.h"

#include "lspt.h"


int main(int argc, char **argv) {
    lsp_vm_init();

    lsp_push_null();

    lspt_assert(!lsp_is_cons());

    return 0;
}
