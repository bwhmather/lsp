#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef void (* lsp_op_t)(void);


/**
 * Stack operations.
 */
void lsp_dup(int offset);
void lsp_store(int offset);
void lsp_pop();  // helper
void lsp_pop_to(int offset);
void lsp_swp(int offset);  // helper

void lsp_enter_frame(int nargs);
void lsp_exit_frame(int nret);


/**
 * Null.
 */
void lsp_push_null();
bool lsp_is_null();


/**
 * Builtin function operations.
 */
void lsp_push_op(void (* value)());
bool lsp_is_op();
lsp_op_t lsp_read_op();

bool lsp_is_truthy();
bool lsp_is_equal();


/**
 * Integer operations.
 */
void lsp_push_int(int value);
bool lsp_is_int();
int lsp_read_int();

void lsp_int_add();
void lsp_int_sub();
void lsp_int_mul();
void lsp_int_div();


/**
 * Symbols operations.
 */
void lsp_push_symbol(char *value);
bool lsp_is_symbol();
char *lsp_borrow_symbol();


/**
 * String operations.
 */
void lsp_push_string(char *value);
bool lsp_is_string();
char *lsp_borrow_string();

void lsp_str_len();
void lsp_str_concat();


/**
 * Basic cons cell operations.
 */
void lsp_push_cons();
bool lsp_is_cons();

void lsp_cons();
void lsp_car();
void lsp_cdr();
void lsp_set_car();
void lsp_set_cdr();


/**
 * Higher order list operations.
 */
void lsp_map();
void lsp_fold();
void lsp_reverse();


/**
 * Managing environments for function execution.
 */
void lsp_push_scope();
void lsp_define();
void lsp_lookup();
void lsp_set();
void lsp_push_empty_env();
void lsp_push_default_env();


/**
 * The core of the interpreter.
 */
void lsp_vm_init();

void lsp_parse();

void lsp_eval();

void lsp_print();


/**
 * Interpreter information.
 */
size_t lsp_stats_frame_size();
size_t lsp_stats_stack_size();


