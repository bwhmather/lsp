/**
 * Checks that `lsp_parse` will extract the tail from a single element dotted
 * list.
 */
#include "lsp.h"

#include "lspt.h"


int main(void) {
    lsp_vm_init();

    lsp_push_string("(. 1)");
    lsp_parse();

    lspt_expect(lsp_stats_frame_size() == 1);

    lsp_dup(0);
    lsp_car();
    lspt_assert(lsp_read_int(0) == 1);
    lsp_pop();

    lsp_cdr();
    lspt_assert(lsp_is_null(0));

    return 0;
}
