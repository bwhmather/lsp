#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef void (* lsp_op_t)(void);

void lsp_vm_init();

void lsp_push_null();
void lsp_push_cons();
void lsp_push_op(void (* value)());
void lsp_push_int(int value);
void lsp_push_symbol(char *value);
void lsp_push_string(char *value);

int lsp_read_int();
char *lsp_borrow_symbol();
char *lsp_borrow_string();
lsp_op_t lsp_read_op();

void lsp_cons();  // helper
void lsp_car();
void lsp_cdr();
void lsp_set_car();
void lsp_set_cdr();

void lsp_dup(int offset);
void lsp_store(int offset);
void lsp_pop();  // helper
void lsp_pop_to(int offset);
void lsp_swp(int offset);  // helper

void lsp_enter_frame(int nargs);
void lsp_exit_frame(int nret);

bool lsp_is_null();
bool lsp_is_cons();
bool lsp_is_int();
bool lsp_is_symbol();
bool lsp_is_string();
bool lsp_is_op();

bool lsp_is_truthy();
bool lsp_is_equal();

size_t lsp_stats_frame_size();
size_t lsp_stats_stack_size();

