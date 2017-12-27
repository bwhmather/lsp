#pragma once

#include "heap.h"

lsp_expr_t *lsp_empty_env();
lsp_expr_t *lsp_push_scope(lsp_expr_t *env);
void lsp_define(char *sym, lsp_expr_t *val, lsp_expr_t *env);
lsp_expr_t *lsp_lookup(char *sym, lsp_expr_t *env);
void lsp_set(char *sym, lsp_expr_t *val, lsp_expr_t *env);
