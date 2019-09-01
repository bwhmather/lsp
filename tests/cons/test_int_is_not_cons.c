/**
 * Checks that `lsp_is_cons` returns `false` when called on an integer.
 */
#include "lsp.h"

#include "lspt.h"


int main(void) {
    lsp_vm_init();

    lsp_push_int(5);

    lspt_assert(!lsp_is_cons(0));

    return 0;
}
