/**
 * Checks that `lsp_lookup` is able to read one of many variables in the inner
 * scope.
 */
#include "lsp.h"

#include "lspt.h"


int main(void) {
    lsp_vm_init();

    lsp_push_empty_env();
    
    lsp_push_int(5);
    lsp_push_symbol("five");
    lsp_dup(-1);
    lsp_define();

    lsp_push_int(4);
    lsp_push_symbol("four");
    lsp_dup(-1);
    lsp_define();

    lsp_push_int(3);
    lsp_push_symbol("three");
    lsp_dup(-1);
    lsp_define();

    lsp_push_int(2);
    lsp_push_symbol("two");
    lsp_dup(-1);
    lsp_define();

    lsp_push_int(1);
    lsp_push_symbol("one");
    lsp_dup(-1);
    lsp_define();

    lspt_assert(lsp_stats_frame_size() == 1);

    lsp_push_symbol("three");
    lsp_dup(-1);
    lsp_lookup();

    lspt_assert(lsp_stats_frame_size() == 2);

    lspt_assert(lsp_read_int(0) == 3);

    return 0;
}
