/**
 * Checks that `lsp_cons` aborts when called on an empty stack.
 */
#include "lsp.h"

#include "lspt.h"


int main(void) {
    lsp_vm_init();

    lspt_assert_aborts(lsp_cons());
}
