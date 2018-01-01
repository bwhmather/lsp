#pragma once

typedef offset_t lsp_offset_t;

void lsp_push_null();
void lsp_push_cons();
void lsp_push_op(void (* value)());
void lsp_push_int(int value);
void lsp_push_symbol(char *value);
void lsp_push_string(char *value);

int lsp_read_int();
char *lsp_read_symbol();
char *lsp_read_string();

void lsp_cons();  // helper
void lsp_car();
void lsp_cdr();
void lsp_set_car();
void lsp_set_cdr();

void lsp_dup(int offset);
void lsp_store(int offset);
void lsp_pop();
void lsp_pop_to(int offset);

void lsp_del(int offset);
void lsp_swp(int offset);

void lsp_call(int nargs);

bool lsp_is_null();
bool lsp_is_cons();
bool lsp_is_op();
bool lsp_is_int();
bool lsp_is_symbol();
bool lsp_is_string();

bool lsp_is_truthy();
bool lsp_is_equal();

