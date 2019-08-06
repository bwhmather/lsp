/**
 * Checks that `lsp_reverse` correctly reverses a three element list.
 */
#include "lsp.h"

#include "lspt.h"


int main(int argc, char **argv) {
    lsp_vm_init();

    lsp_push_null();
    lsp_push_int(2);
    lsp_cons();

    lsp_push_int(1);
    lsp_cons();

    lsp_reverse();

    lsp_dup(0);
    lspt_assert(lsp_is_cons());

    lsp_dup(0);
    lsp_car();
    lspt_assert(lsp_read_int(0) == 2);
    lsp_pop();

    lsp_dup(0);
    lsp_cdr();
    lsp_car();
    lspt_assert(lsp_read_int(0) == 1);
    lsp_pop();

    lsp_dup(0);
    lsp_cdr();
    lsp_cdr();
    lspt_assert(lsp_is_null());

    return 0;
}

