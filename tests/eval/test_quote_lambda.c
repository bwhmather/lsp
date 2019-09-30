/**
 * Checks that `lsp_eval` will not evaluate a lambda expression inside a quote
 * block.
 */
#include "lsp.h"

#include "lspt.h"


int main(void) {
    lsp_vm_init();

    lsp_push_string("(quote (lambda (x) (* x 2)))");
    lsp_parse();
    lsp_car();

    lsp_push_empty_env();
    
    lsp_eval();

    lspt_assert(lsp_stats_frame_size() == 1);

    lsp_push_string("(lambda (x) (* x 2))");
    lsp_parse();
    lsp_car();

    lspt_assert_equal();

    return 0;
}
