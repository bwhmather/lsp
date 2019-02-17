#include "lsp.h"

#include "lspt.h"


int main(int argc, char **argv) {
    lsp_vm_init();

    lsp_push_null();

    lspt_assert_not(lsp_is_cons());

    return 0;
}
