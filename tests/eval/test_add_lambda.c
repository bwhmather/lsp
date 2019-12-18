/**
 * Checks that `lsp_eval` can add two numbers, where one number is contained in
 * a closure.
 */
#include "lsp.h"

#include "lspt.h"


int main(void) {
    lsp_vm_init();

    lsp_push_string("((lambda (x) (+ x 3)) 5)");
    lsp_parse();
    lsp_car();

    lsp_push_default_env();

    lsp_eval();

    lspt_assert(lsp_read_int(0) == 8);

    return 0;
}
