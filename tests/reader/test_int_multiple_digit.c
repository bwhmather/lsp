/**
 * Checks that `lsp_parse` can read an integer with multiple digits.
 */
#include "lsp.h"

#include "lspt.h"


int main(void) {
    lsp_vm_init();

    lsp_push_string("123456");
    lsp_parse();

    lspt_assert(lsp_stats_frame_size() == 1);

    lsp_car();

    lspt_assert(lsp_read_int(0) == 123456);

    return 0;
}
