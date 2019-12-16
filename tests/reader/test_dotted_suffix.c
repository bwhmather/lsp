/**
 * Checks that `lsp_parse` will abort when passed a list terminated by a `.`.
 */
#include "lsp.h"

#include "lspt.h"


int main(void) {
    lsp_vm_init();

    lsp_push_string("(1. )");

    lspt_assert_aborts(lsp_parse());

    return 0;
}
