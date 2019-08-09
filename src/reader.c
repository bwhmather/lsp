#include "lsp.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>


static char lsp_parser_next() {
    const char *buffer = lsp_borrow_string(-1);
    int cursor = lsp_read_int(-2);

    return buffer[cursor];
}


static char lsp_parser_lookahead() {
    const char *buffer = lsp_borrow_string(-1);
    int cursor = lsp_read_int(-2);

    if (buffer[cursor] == '\0') {
        return '\0';
    }

    return buffer[cursor + 1];
}


static void lsp_parser_advance() {
    const char *buffer = lsp_borrow_string(-1);
    int cursor = lsp_read_int(-2);

    if (buffer[cursor] != '\0') {
        lsp_push_int(cursor + 1);
        lsp_store(-2);
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


static void lsp_parse_string() {
    size_t buffer_size = 256;
    char * buffer = (char *) malloc(buffer_size);
    if (buffer == NULL) {
        abort();
    }
    size_t cursor = 0;

    lsp_parser_advance();

    while (true) {
        char next = lsp_parser_next();

        if (next == '\0') {
            // Unexpected end of string.
            abort();
        }

        if (cursor == buffer_size - 2) {
            buffer_size += buffer_size / 2;
            buffer = realloc(buffer, buffer_size);
            if (buffer == NULL) {
                abort();
            }
        }

        if (next == '"') {
            buffer[cursor] = '\0';

            lsp_parser_advance();
            break;
        }

        if (next == '\\') {
            lsp_parser_advance();
            next = lsp_parser_next();

            switch (next) {
                case 'a':
                    next = '\a';
                    break;
                case 'f':
                    next = '\f';
                    break;
                case 'n':
                    next = '\n';
                    break;
                case 'r':
                    next = '\r';
                    break;
                case 't':
                    next = '\t';
                    break;
                case '\\':
                    next = '\\';
                    break;
                case '\'':
                    next = '\'';
                    break;
                case '\"':
                    next = '\"';
                    break;
                case 'x':
                    abort();  // Not implemented.
                default:
                    abort();  // Not supported.
            }
        }

        buffer[cursor] = next;
        cursor++;

        lsp_parser_advance();
    }


    lsp_push_string(buffer);

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


/**
 * Should be called with a single string.
 *
 * Will consume all expressions in the string and return them as a list.
 */
void lsp_parse() {
    lsp_fp_t rp = lsp_get_fp();
    lsp_shrink_frame(1);

    lsp_push_int(0);

    // The next item on the stack, after the cursor, is a list, ordered inner
    // to outer, of pointers to the last cons cell in each list body.  This is
    // the parser stack.
    lsp_push_null();

    // The second item on the stack is a reversed list of elements in the
    // body of the current list in the parse stack.
    lsp_push_null();

    // At the beginning of parsing there are no parent contexts so the parse
    // stack is empty.  If an opening bracket is encountered, the current body
    // is pushed on to the parse stack and a new body is started to parse the
    // nested list.  On hitting the closing bracket, the body list will be
    // reversed and added to the containing list in the parse stack.

    while (true) {
        assert(lsp_stats_frame_size() == 4);

        lsp_consume_whitespace();

        char next = lsp_parser_next();
        char lookahead = lsp_parser_lookahead();

        if (next == '(') {
            // Push the current body onto the stack, consuming it.
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
            lsp_store(-1);
            lsp_pop_to(-2);
            lsp_restore_fp(rp);
            return;
        }

        // Parse the next expression and save it as the third item in this
        // stack frame.
        if (next == ')') {
            // Unwind and reverse the current body list and store it as the
            // current expression.
            lsp_dup(0);
            lsp_reverse();

            // Replace the body list with the next one down the parse stack.
            lsp_dup(2);
            lsp_car();
            lsp_store(2);

            // Pop restored body list from the parse stack.
            lsp_dup(2);
            lsp_cdr();
            lsp_store(3);

            lsp_parser_advance();
        } else if (next == '.') {
            // TODO figure out how to parse non-list cons cells.
            assert(false);
        } else if (
            (next >= '0' && next <= '9') ||
            (next == '-' && lookahead >= '0' && lookahead <= '9')
        ) {
            lsp_parse_number();
        } else if (next == '"') {
            lsp_parse_string();
        } else if (lsp_is_symbol_character(next)) {
            lsp_parse_symbol();
        } else {
            assert(false);
        }

        // Replace the body with a new list containing the new expression as
        // its first element.
        lsp_cons();
    }
}
