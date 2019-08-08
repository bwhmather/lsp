/**
 * Checks that `lsp_parse` can read a list containing a single number.
 */
#include "lsp.h"

#include "lspt.h"


int main(int argc, char **argv) {
    lsp_vm_init();

    lsp_push_string("(4)");
    lsp_parse();

    lspt_expect(lsp_stats_frame_size() == 1);

    lsp_dup(0);
    lsp_cdr();
    lspt_expect(lsp_is_null());
    lsp_pop();

    lsp_car();

    lsp_dup(0);
    lsp_cdr();
    lspt_expect(lsp_is_null());
    lsp_pop();
        
    lsp_car();
    lspt_assert(lsp_read_int(0) == 4);

    return 0;
}
