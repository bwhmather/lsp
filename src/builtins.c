#include "builtins.h"

#include "vm.h"
#include "interpreter.h"
#include "env.h"

#include <stdlib.h>
#include <assert.h>


static void lsp_op_add() {
    int a = lsp_read_int(0);
    int b = lsp_read_int(1);
    lsp_pop_to(0);
    lsp_push_int(a + b);
}

static void lsp_op_sub() {
    int a = lsp_read_int(0);
    int b = lsp_read_int(1);
    lsp_pop_to(0);
    lsp_push_int(a - b);
}

static void lsp_op_mul() {
    int a = lsp_read_int(0);
    int b = lsp_read_int(1);
    lsp_pop_to(0);
    lsp_push_int(a * b);
}

static void lsp_op_div() {
    int a = lsp_read_int(0);
    int b = lsp_read_int(1);
    lsp_pop_to(0);
    lsp_push_int(a / b);
}

static void lsp_op_cons() {
    lsp_cons();
}

static void lsp_op_car() {
    lsp_car();
}

static void lsp_op_set_car() {
    lsp_set_car();
}

static void lsp_op_cdr() {
    lsp_cdr();
}

static void lsp_op_set_cdr() {
    lsp_set_cdr();
}

static void lsp_bind(char *symbol, lsp_op_t operation) {
    lsp_dup(-1);
    lsp_push_symbol(symbol);
    lsp_push_op(operation);
    lsp_define();
}

void lsp_push_default_env() {
    lsp_empty_env();

    lsp_bind("+", &lsp_op_add);
    lsp_bind("-", &lsp_op_sub);
    lsp_bind("*", &lsp_op_mul);
    lsp_bind("/", &lsp_op_div);
    lsp_bind("cons", &lsp_op_cons);
    lsp_bind("car", &lsp_op_car);
    lsp_bind("set-car!", &lsp_op_set_car);
    lsp_bind("cdr", &lsp_op_cdr);
    lsp_bind("set-cdr!", &lsp_op_set_cdr);
}

