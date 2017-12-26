#include "parser.h"
#include "eval.h"
#include "heap.h"
#include "builtins.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>


int main(int argc, char **argv) {
    (void) argc;
    (void) argv;

    lsp_heap_init();
    lsp_expr_t *ast = lsp_parse();
    lsp_expr_t *result = lsp_eval(lsp_car(ast), lsp_default_env());
    lsp_print(result);
}

