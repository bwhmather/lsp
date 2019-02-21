/**
 * Checks that `lsp_car` returns the value added as the car of a cons cell.
 */
#include "lsp.h"

#include "lspt.h"


int main(int argc, char **argv) {
    lsp_vm_init();

    lsp_push_int(1);
    lsp_push_int(2);

    lsp_cons();

    lsp_car();

    lspt_assert_eq(lsp_read_int(), 1);

    return 0;
}
