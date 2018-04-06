#include <criterion/criterion.h>

#include "lsp.h"


Test(reverse, null) {
    lsp_vm_init();

    lsp_push_null();

    lsp_reverse();

    cr_assert_eq(lsp_stats_frame_size(), 1);
    cr_assert(lsp_is_null());
}

Test(reverse, pair) {
    lsp_vm_init();

    lsp_push_int(12);
    lsp_push_null();
    lsp_cons();

    lsp_reverse();

    cr_assert_eq(lsp_stats_frame_size(), 1);

    lsp_dup(0);
    cr_assert(lsp_is_cons());

    lsp_dup(0);
    lsp_car();
    cr_assert_eq(lsp_read_int(), 12);

    lsp_dup(0);
    lsp_cdr();
    cr_assert(lsp_is_null());
}

Test(reverse, triple) {
    lsp_vm_init();

    lsp_push_int(1);
    lsp_push_int(2);
    lsp_push_null();
    lsp_cons();
    lsp_cons();

    lsp_reverse();

    cr_assert_eq(lsp_stats_frame_size(), 1);

    lsp_dup(0);
    cr_assert(lsp_is_cons());

    lsp_dup(0);
    lsp_car();
    cr_assert_eq(lsp_stats_frame_size(), 2);
    cr_assert_eq(lsp_read_int(), 2);

    cr_assert_eq(lsp_stats_frame_size(), 1);
    lsp_cdr();
    lsp_dup(0);

    lsp_car();
    cr_assert_eq(lsp_read_int(), 1);

    lsp_cdr();
    cr_assert(lsp_is_null());
}
