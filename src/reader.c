#include "lsp.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

// State for reading from in-memory buffer.
static char *input_buffer;
static size_t input_cursor;

static char lsp_parser_next() {
    return input_buffer[input_cursor];
}

static char lsp_parser_lookahead() {
    if (input_buffer[input_cursor] == '\0') {
        return '\0';
    }
    return input_buffer[input_cursor + 1];
}


static void lsp_parser_advance() {
    if (lsp_parser_next() != '\0') {
        input_cursor++;
    }
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


static void lsp_parse_symbol() {
    size_t buffer_size = 256;
    char *buffer = (char *) malloc(buffer_size);
    if (buffer == NULL) {
        abort();
    }
    size_t cursor = 0;

    char next = lsp_parser_next();

    while (lsp_is_symbol_character(next)) {
        if (cursor == buffer_size - 2) {
            buffer_size += buffer_size / 2;
            buffer = realloc(buffer, buffer_size);
            if (buffer == NULL) {
                abort();
            }
        }

        buffer[cursor] = next;
        cursor++;

        lsp_parser_advance();
        next = lsp_parser_next();
    }

    buffer[cursor] = '\0';

    lsp_push_symbol(buffer);

    free(buffer);
}


static void lsp_parse_number() {
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

    lsp_push_int(negative ? -accumulator : accumulator);
}


void lsp_parse() {
    lsp_fp_t rp = lsp_get_fp();
    lsp_shrink_frame(0);

    lsp_parser_advance();
    lsp_parser_advance();

    // The first item on the stack is a list, ordered inner to outer, of
    // pointers to the last cons cell in each list body.
    lsp_push_null();

    // The second item on the stack is a reversed list of elements in the
    // current body.
    lsp_push_null();

    while (true) {
        lsp_consume_whitespace();

        char next = lsp_parser_next();
        char lookahead = lsp_parser_lookahead();

        if (next == '(') {
            // Push the current body onto the stack, consuming it.
            lsp_swp(0);
            lsp_cons();

            // Replace it with a new empty list.
            lsp_push_null();

            lsp_parser_advance();

            // No expression to add.  Skip logic at end of loop.
            continue;
        }

        if (next == '\0') {
            // Put the body list back in the right order and return it.
            lsp_reverse();
            lsp_store(0);
            lsp_pop_to(1);
            lsp_restore_fp(rp);
            return;
        }

        // Parse the next expression and save it as the third item in this
        // stack frame.
        if (next == ')') {
            // Unwind and reverse the current body list and store it as the
            // current expression.
            lsp_dup(1);
            lsp_reverse();

            // Replace the body list with the next one down the parse stack.
            lsp_dup(0);
            lsp_car();
            lsp_store(1);

            // Pop the parse stack.
            lsp_dup(0);
            lsp_cdr();
            lsp_store(0);

            lsp_parser_advance();
        } else if (next == '.') {
            // TODO figure out how to parse non-list cons cells.
            assert(false);
        } else if (
            (next >= '0' && next <= '9') ||
            (next == '-' && lookahead >= '0' && lookahead <= '9')
        ) {
            lsp_parse_number();
        } else if (lsp_is_symbol_character(next)) {
            lsp_parse_symbol();
        } else {
            assert(false);
        }

        // Replace the body with a new list containing the new expression as
        // its first element.
        lsp_swp(-1);
        lsp_cons();
    }
}

void lsp_read(char *string) {
    assert(string != NULL);

    assert(input_buffer == NULL);
    assert(input_cursor == 0);

    // Set up input state.
    input_buffer = string;
    input_cursor = 0;

    // Read.
    lsp_parse();

    // Tear down input state.
    input_buffer = NULL;
    input_cursor = 0;
}
