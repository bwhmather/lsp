#include "lsp.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

static void lsp_parse_one(void);


static char lsp_parser_next(void) {
    lsp_dup(0);
    lsp_cdr();
    int cursor = lsp_read_int(0);
    lsp_pop();

    lsp_dup(0);
    lsp_car();
    const char *buffer = lsp_borrow_string(0);
    char character = buffer[cursor];
    lsp_pop();

    return character;
}


static char lsp_parser_lookahead(void) {
    lsp_dup(0);
    lsp_cdr();
    int cursor = lsp_read_int(0);
    lsp_pop();

    lsp_dup(0);
    lsp_car();
    const char *buffer = lsp_borrow_string(0);
    char character = buffer[cursor] == '\0' ? '\0': buffer[cursor + 1];
    lsp_pop();

    return character;
}


static void lsp_parser_advance(void) {
    lsp_dup(0);
    lsp_cdr();
    int cursor = lsp_read_int(0);
    lsp_pop();

    lsp_dup(0);
    lsp_car();
    const char *buffer = lsp_borrow_string(0);
    char character = buffer[cursor];
    lsp_pop();

    if (character != '\0') {
        lsp_push_int(cursor + 1);
        lsp_swp(1);
        lsp_set_cdr();
    }
}


static void lsp_consume_whitespace(void) {
    char next = lsp_parser_next();

    while (next == ' ' || next == '\n' || next == '\t') {
        lsp_dup(0);
        lsp_parser_advance();

        next = lsp_parser_next();
    }

    lsp_pop();
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


static void lsp_parse_symbol(void) {
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

        lsp_dup(-1);
        lsp_parser_advance();

        lsp_dup(-1);
        next = lsp_parser_next();
        lsp_pop();
    }
    lsp_pop();

    buffer[cursor] = '\0';

    lsp_push_symbol(buffer);

    free(buffer);
}


static void lsp_parse_string(void) {
    size_t buffer_size = 256;
    char * buffer = (char *) malloc(buffer_size);
    if (buffer == NULL) {
        abort();
    }
    size_t cursor = 0;

    lsp_dup(0);
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

            lsp_dup(0);
            lsp_parser_advance();
            break;
        }

        if (next == '\\') {
            lsp_dup(0);
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

        lsp_dup(0);
        lsp_parser_advance();
    }
    lsp_pop();

    lsp_push_string(buffer);

    free(buffer);
}


static void lsp_parse_number(void) {
    bool negative = false;
    int accumulator = 0;

    char next = lsp_parser_next();
    if (next == '-') {
        negative = true;

        lsp_dup(0);
        lsp_parser_advance();

        next = lsp_parser_next();
    }

    while (next >= '0' && next <= '9') {
        accumulator *= 10;
        accumulator += (int) (next - '0');

        lsp_dup(0);
        lsp_parser_advance();

        next = lsp_parser_next();
    }
    lsp_pop();

    lsp_push_int(negative ? -accumulator : accumulator);
}


static void lsp_parse_list(void) {
    // Cons cell where the cdr points to the head of the list, and the car is
    // ignored.
    lsp_push_cons();

    // Reference to the tail of the list.
    lsp_dup(0);

    // Consume the leading '('.
    lsp_dup(-1);
    char next = lsp_parser_next();
    assert(next == '(');
    lsp_parser_advance();

    while (true) {
        lsp_dup(-1);
        lsp_consume_whitespace();

        lsp_dup(-1);
        char next = lsp_parser_next();
        lsp_pop();

        if (next == ')') {
            lsp_dup(-1);
            lsp_parser_advance();

            break;
        }

        if (next == '.') {
            lsp_dup(-1);
            lsp_parser_advance();

            lsp_dup(-1);
            lsp_consume_whitespace();

            lsp_dup(-1);
            lsp_parse_one();

            // Insert the value following the dot as the cdr of the tail.
            lsp_dup(1);
            lsp_set_cdr();

            lsp_dup(-1);
            lsp_consume_whitespace();

            lsp_dup(-1);
            assert(lsp_parser_next() == ')');
            lsp_parser_advance();

            break;
        }

        lsp_dup(-1);
        lsp_parse_one();

        // Create a new tail pair.
        lsp_push_null();
        lsp_swp(1);

        lsp_cons();

        // Insert it as the cdr of the current tail.
        lsp_dup(1);
        lsp_set_cdr();

        // Replace the old tail with the new tail.
        lsp_cdr();
    }

    // Discard the tail reference.
    lsp_pop();

    // Strip the dummy head and save the rest of the list.
    lsp_cdr();
    lsp_store(1);
}

static void lsp_parse_one(void) {
    char next = lsp_parser_next();
    char lookahead = lsp_parser_lookahead();

    if (next == '(') {
        lsp_parse_list();
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
}

/**
 * Should be called with a single string.
 *
 * Will consume all expressions in the string and return them as a list.
 */
void lsp_parse(void) {
    lsp_fp_t rp = lsp_get_fp();
    lsp_shrink_frame(1);

    lsp_push_int(0);
    lsp_swp(1);
    lsp_cons();

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
        lsp_dup(2);
        lsp_consume_whitespace();

        lsp_dup(2);
        char next = lsp_parser_next();
        lsp_pop();

        if (next == '\0') {
            // Put the body list back in the right order and return it.
            lsp_reverse();
            lsp_store(-1);
            lsp_pop();
            lsp_restore_fp(rp);
            return;
        }

        lsp_dup(-1);
        lsp_parse_one();

        // Replace the body with a new list containing the new expression as
        // its first element.
        lsp_cons();
    }
}
