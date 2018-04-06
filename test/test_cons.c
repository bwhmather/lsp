#include "lsp.h"

#include <string.h>
#include <criterion/criterion.h>



Test(cons, push) {
    lsp_vm_init();

    lsp_push_cons();

    cr_assert_eq(lsp_stats_frame_size(), 1);
    cr_assert(lsp_is_cons());
}

Test(cons, cons_nulls) {
    lsp_vm_init();

    lsp_push_null();
    lsp_push_null();

    lsp_cons();

    cr_assert_eq(lsp_stats_frame_size(), 1);
    cr_assert(lsp_is_cons());
}

Test(cons, car) {
    lsp_vm_init();

    lsp_push_int(2);
    lsp_push_int(1);

    lsp_cons();

    lsp_car();

    cr_assert_eq(lsp_read_int(), 1);
}

Test(cons, cdr) {
    lsp_vm_init();

    lsp_push_int(2);
    lsp_push_int(1);

    lsp_cons();

    lsp_cdr();

    cr_assert_eq(lsp_read_int(), 2);
}

Test(cons, is_cons_pops) {
    lsp_vm_init();

    lsp_push_cons();

    cr_expect(lsp_is_cons());
    cr_assert_eq(lsp_stats_frame_size(), 0);
}

Test(cons, int_is_not_cons) {
    lsp_vm_init();

    lsp_push_int(5);

    cr_assert_not(lsp_is_cons());
}

Test(cons, null_is_not_cons) {
    lsp_vm_init();

    lsp_push_null();

    cr_assert_not(lsp_is_cons());
}

Test(cons, set_car) {
    lsp_vm_init();

    lsp_push_cons();

    lsp_dup(0);
    lsp_enter_frame(1);

    lsp_push_int(5);
    lsp_set_car();

    cr_expect_eq(lsp_stats_frame_size(), 0);
    lsp_exit_frame(0);

    lsp_car();
    cr_assert_eq(lsp_read_int(), 5);
}

Test(cons, set_cdr) {
    lsp_vm_init();

    lsp_push_cons();

    lsp_dup(0);
    lsp_enter_frame(1);

    lsp_push_int(5);
    lsp_set_cdr();

    cr_expect_eq(lsp_stats_frame_size(), 0);
    lsp_exit_frame(0);

    lsp_cdr();
    cr_assert_eq(lsp_read_int(), 5);
}
