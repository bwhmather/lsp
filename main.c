#include "parser.h"
#include "eval.h"
#include "vm.h"
#include "builtins.h"
#include "env.h"
#include "interpreter.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>


int main(int argc, char **argv) {
    (void) argc;
    (void) argv;

    lsp_vm_init();

    // Load the default environment.
    lsp_push_default_env();

    // Parse a list of expression from stdin.
    lsp_parse();

    // Evaluate the first expression in the list (TODO evaluate everything).
    lsp_eval();

    // Dump the result to stdout.
    lsp_print();
}

