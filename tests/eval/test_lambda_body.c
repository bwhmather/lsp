/**
 * Checks that `lsp_eval` can call a function with multiple body bits.
 */
#include "lsp.h"

#include "lspt.h"


int main(void) {
    lsp_vm_init();

    lsp_push_default_env();

    lsp_push_string("((lambda (x) (define y 4) (set! y 5) (+ x y)) 2)");
    lsp_parse();
    lsp_car();

    lsp_dup(1);
    lsp_eval();

    lspt_assert(lsp_read_int(0) == 7);

    return 0;
}
