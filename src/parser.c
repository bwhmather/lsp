#include "heap.h"

#include <stdio.h>
#include <assert.h>


static char lookahead_buffer[2] = {'\0', '\0'};


static char lsp_parser_next() {
    return lookahead_buffer[0];
}


static char lsp_parser_lookahead() {
    return lookahead_buffer[1];
}


static void lsp_parser_advance() {
    int next = getc(stdin);

    lookahead_buffer[0] = lookahead_buffer[1];
    lookahead_buffer[1] = next == EOF ? '\0': next;
}


static void lsp_consume_whitespace() {
    char next = lsp_parser_next();
    while (next == ' ' || next == '\n' || next == '\t') {
        lsp_parser_advance();
        next = lsp_parser_next();
    }
}


static bool lsp_is_symbol_character(char next) {
    return (
        (next >= 'a' && next <= 'z') ||
        (next >= 'A' && next <= 'Z') ||
        next == '+' || next == '-' || next == '*' ||
        next == '/' || next == '%' ||
        next == '<' || next == '=' || next == '>' ||
        next == '~' || next == '!' ||
        next == '$' || next == '@' ||
        next == '&' || next == '|' || next == '^' ||
        next == '?' ||
        next == ':' || next == '_'
    );
}


static lsp_expr_t *lsp_parse_symbol() {
    lsp_expr_t *symbol = lsp_symbol_start();

    char next = lsp_parser_next();

    while (lsp_is_symbol_character(next)) {
        lsp_symbol_push(next);
        lsp_parser_advance();
        next = lsp_parser_next();
    }

    lsp_symbol_stop();

    return symbol;
}


static lsp_expr_t *lsp_parse_number() {
    bool negative = false;
    int accumulator = 0;

    char next = lsp_parser_next();
    if (next == '-') {
        negative = true;
        lsp_parser_advance();
        next = lsp_parser_next();
    }

    while (next >= '0' && next <= '9') {
        accumulator *= 10;
        accumulator += (int) (next - '0');

        lsp_parser_advance();
        next = lsp_parser_next();
    }

    return lsp_int(negative ? -accumulator : accumulator);
}


lsp_expr_t *lsp_parse() {
    lsp_parser_advance();
    lsp_parser_advance();

    // The reversed list of elements in the current body.
    lsp_expr_t *body = NULL;

    // stack is a list, ordered inner to outer, of pointers to the last cons
    // cell in each list body.
    lsp_expr_t *stack = NULL;

    while (true) {
        lsp_consume_whitespace();
        
        char next = lsp_parser_next();
        char lookahead = lsp_parser_lookahead();

        if (next == '(') {
            // Push the current body onto the stack, and start a new one.
            stack = lsp_cons(body, stack);
            body = NULL;

            lsp_parser_advance();

            // No expression to add.  Skip logic at end of loop.
            continue;
        }

        // Parse the next expression and save it in the current environment. 
        lsp_expr_t *expression;

        if (next == ')' || next == '\0') {
            // Unwind and reverse the current body list and store it as the
            // current expression.
            while (lsp_type(body) == LSP_CONS) {
                expression = lsp_cons(lsp_car(body), expression);
                body = lsp_cdr(body);
            }

            if (next == '\0') {
                return expression;
            }

            // Pop the containing body from the stack and set it as current.
            body = lsp_car(stack);
            stack = lsp_cdr(stack);

            lsp_parser_advance();
        } else if (next == '.') {
            // TODO figure out how to parse non-list cons cells.
        } else if (
            (next >= '0' && next <= '9') ||
            (next == '-' && lookahead >= '0' && lookahead <= '9')
        ) {
            expression = lsp_parse_number();
        } else if (lsp_is_symbol_character(next)) {
            expression = lsp_parse_symbol();
        } else {
            assert(false);
        }

        // Push expression.
        body = lsp_cons(expression, body);
    }
}

