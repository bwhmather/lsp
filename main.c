#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

typedef enum lsp_type_t {
    LSP_NULL = 0,
    LSP_CONS,
    LSP_INT,
    LSP_SYM,
    LSP_OP,
} lsp_type_t;

typedef void lsp_expr_t;

typedef struct lsp_cons_t {
    lsp_expr_t *car;
    lsp_expr_t *cdr;
} lsp_cons_t;


typedef lsp_expr_t *(* lsp_op_t)(lsp_expr_t *);

static lsp_type_t lsp_type(lsp_expr_t *expr) {
    if (expr == NULL) {
        return LSP_NULL;
    }
    lsp_type_t *type_ptr = (lsp_type_t *) expr;
    return *type_ptr;
}

static char *heap_data;
static char *heap_ptr;

static char *lsp_data(lsp_expr_t *expr) {
    return (char *) expr + sizeof(lsp_type_t);
}

static lsp_cons_t *lsp_as_cons(lsp_expr_t *expr) {
    if (lsp_type(expr) != LSP_CONS) {
        assert(false);
    }
    return (lsp_cons_t *) lsp_data(expr);
}

static int *lsp_as_int(lsp_expr_t *expr) {
    assert(lsp_type(expr) == LSP_INT);
    return (int *) lsp_data(expr);
}

static char *lsp_as_sym(lsp_expr_t *expr) {
    assert(lsp_type(expr) == LSP_SYM);
    return (char *) lsp_data(expr);
}

static lsp_op_t *lsp_as_op(lsp_expr_t *expr) {
    assert(lsp_type(expr) == LSP_OP);
    return (lsp_op_t *) lsp_data(expr);
}

static lsp_expr_t *lsp_cons(lsp_expr_t *car, lsp_expr_t *cdr) {
    lsp_expr_t *expr_ptr = (lsp_expr_t *) heap_ptr;

    lsp_type_t *type_ptr = (lsp_type_t *) heap_ptr;
    *type_ptr = LSP_CONS;
    heap_ptr += sizeof(lsp_type_t);

    lsp_cons_t *cons_ptr = (lsp_cons_t *) heap_ptr;
    cons_ptr->car = car;
    cons_ptr->cdr = cdr;
    heap_ptr += sizeof(lsp_cons_t);

    return expr_ptr;
}

static lsp_expr_t *lsp_symbol_start() {
    lsp_expr_t *expr_ptr = heap_ptr;

    lsp_type_t *type_ptr = (lsp_type_t *) heap_ptr;
    *type_ptr = LSP_SYM;
    heap_ptr += sizeof(lsp_type_t);

    return expr_ptr;
}

static void lsp_symbol_push(char character) {
    *heap_ptr = character;
    heap_ptr += 1;
}

static void lsp_symbol_stop() {
    lsp_symbol_push('\0');
}

static lsp_expr_t *lsp_symbol(char *name) {
    lsp_expr_t *expr = lsp_symbol_start();

    for (int cursor=0; name[cursor] != '\n'; cursor++) {
        lsp_symbol_push(name[cursor]);
    }
    lsp_symbol_stop();
    return expr;
}

static lsp_expr_t *lsp_int(int value) {
    lsp_expr_t *expr_ptr = heap_ptr;

    lsp_type_t *type_ptr = (lsp_type_t *) heap_ptr;
    *type_ptr = LSP_INT;
    heap_ptr += sizeof(lsp_type_t);

    int *int_ptr = (int *) heap_ptr;
    *int_ptr = value;
    heap_ptr += sizeof(int);

    return expr_ptr;
}

static lsp_expr_t *lsp_op(lsp_op_t op) {
    lsp_expr_t *expr_ptr = heap_ptr;

    lsp_type_t *type_ptr = (lsp_type_t *) heap_ptr;
    *type_ptr = LSP_OP;
    heap_ptr += sizeof(lsp_type_t);

    lsp_op_t *op_ptr = (lsp_op_t *) heap_ptr;
    *op_ptr = op;
    heap_ptr += sizeof(lsp_op_t);

    return expr_ptr;
}

static lsp_expr_t *lsp_car(lsp_expr_t *expr) {
    lsp_cons_t *cons = lsp_as_cons(expr);
    return cons->car;
}

static lsp_expr_t *lsp_cdr(lsp_expr_t *expr) {
    lsp_cons_t *cons = lsp_as_cons(expr);
    return cons->cdr;
}

static lsp_expr_t *lsp_reverse(lsp_expr_t *input) {
    lsp_expr_t *output = NULL;
    while (lsp_type(input) == LSP_CONS) {
        output = lsp_cons(lsp_car(input), output);
        input = lsp_cdr(input);
    }
    assert(lsp_type(input) == LSP_NULL);
    return output;
}

static lsp_expr_t *lsp_op_add(lsp_expr_t *args) {
    int a = *lsp_as_int(lsp_car(args));
    int b = *lsp_as_int(lsp_car(lsp_cdr(args)));
    return lsp_int(a + b);
}

static lsp_expr_t *lsp_op_sub(lsp_expr_t *args) {
    int a = *lsp_as_int(lsp_car(args));
    int b = *lsp_as_int(lsp_car(lsp_cdr(args)));
    return lsp_int(a - b);
}

static lsp_expr_t *lsp_op_mul(lsp_expr_t *args) {
    int a = *lsp_as_int(lsp_car(args));
    int b = *lsp_as_int(lsp_car(lsp_cdr(args)));
    return lsp_int(a * b);
}

static lsp_expr_t *lsp_op_div(lsp_expr_t *args) {
    int a = *lsp_as_int(lsp_car(args));
    int b = *lsp_as_int(lsp_car(lsp_cdr(args)));

    return lsp_int(a / b);
}

lsp_expr_t *lsp_default_env() {
    lsp_expr_t *env = NULL;

    env = lsp_cons(lsp_cons(lsp_symbol("+"), lsp_op(&lsp_op_add)), env);
    env = lsp_cons(lsp_cons(lsp_symbol("-"), lsp_op(&lsp_op_sub)), env);
    env = lsp_cons(lsp_cons(lsp_symbol("*"), lsp_op(&lsp_op_mul)), env);
    env = lsp_cons(lsp_cons(lsp_symbol("/"), lsp_op(&lsp_op_div)), env);

    return env;
}

static lsp_expr_t *lsp_lookup(char *sym, lsp_expr_t *env) {
    while (lsp_type(env) == LSP_CONS) {
        if (strcmp(lsp_as_sym(lsp_car(lsp_car(env))), lsp_as_sym(sym)) == 0) {
            return lsp_cdr(lsp_car(env));
        }
        env = lsp_cdr(env);
    }
    assert(0);
}

static lsp_expr_t *lsp_parse() {
    char next;

    // The reversed list of elements in the current body.
    lsp_expr_t *body = NULL;

    // stack is a list, ordered inner to outer, of pointers to the last cons
    // cell in each list body.
    lsp_expr_t *stack = NULL;

    next = getchar();

    while (true) {
        lsp_expr_t *expression = NULL;

        // Consume whitespace whitespace.
        if (next == ' ' || next == '\n') {
            next = getchar();
            continue;
        } else if (next == '(') {
            next = getchar();

            // Push the current body onto the stack, and start a new one.
            stack = lsp_cons(body, stack);
            body = NULL;

            // No expression to add.  Skip logic at end of loop.
            continue;

        } else if (next == ')' || next == EOF) {
            // Unwind and reverse the current body list and store it as the
            // current expression.
            while (lsp_type(body) == LSP_CONS) {
                expression = lsp_cons(lsp_car(body), expression);
                body = lsp_cdr(body);
            }

            if (next == EOF) {
                return expression;
            }

            // Pop the containing body from the stack and set it as current.
            body = lsp_car(stack);
            stack = lsp_cdr(stack);

            next = getchar();
        } else if (next == '.') {
            // TODO figure out how to parse non-list cons cells.
        } else if (next >= '0' && next <= '9') {
            int accumulator = 0;

            // Parse number.
            while (true) {
                if (next >= '0' && next <= '9') {
                    accumulator *= 10;
                    accumulator += (int) (next - '0');
                } else if (
                    next == '(' || next == ')' ||
                    next == ' ' || next == '\n' ||
                    next == EOF
                ) {
                    expression = lsp_int(accumulator);
                    break;
                } else {
                    assert(false);
                }
                next = getchar();
            }
        } else {
            expression = lsp_symbol_start();
            // Parse symbol.
            while (true) {
                if (
                    next == '(' || next == ')' ||
                    next == ' ' || next == '\n'
                ) {
                    break;
                }

                lsp_symbol_push(next);
                next = getchar();
            }
            lsp_symbol_stop();
        }

        // Push expression.
        body = lsp_cons(expression, body);
    }
}

lsp_expr_t *lsp_eval(lsp_expr_t *expr, lsp_expr_t *env) {
    if (lsp_type(expr) == LSP_SYM) {
        // Expression is a name identifying a variable that can be loaded
        // from the environment.
        return lsp_lookup(expr, env);

    } else if (lsp_type(expr) == LSP_CONS) {
        // Expression is a list representing either a special form or an
        // invocation of a procedure or built-in operator.
        if (lsp_type(lsp_car(expr)) == LSP_SYM) {
            char *sym = lsp_as_sym(lsp_car(expr));
            if (strcmp(sym, "if") == 0) {
                assert(false);
            }
            if (strcmp(sym, "quote") == 0) {
                return lsp_car(lsp_cdr(expr));
            }
            if (strcmp(sym, "define") == 0) {
                assert(false);
            }
            if (strcmp(sym, "set!") == 0) {
                assert(false);
            }
            if (strcmp(sym, "lambda") == 0) {
                assert(false);
            }
        }

        // Expression is not a special form.  We evaluate all items in the
        // list and then pass the tail to the built-in or procedure
        // represented by the first item.
        lsp_expr_t *evaled = NULL;
        lsp_expr_t *cursor = expr;
        while (lsp_type(cursor) == LSP_CONS) {
            evaled = lsp_cons(lsp_eval(lsp_car(cursor), env), evaled);
            cursor = lsp_cdr(cursor);
        }
        evaled = lsp_reverse(evaled);

        lsp_expr_t *callable = lsp_car(evaled);
        lsp_expr_t *args = lsp_cdr(evaled);

        if (lsp_type(callable) == LSP_OP) {
            // Expression is a call to a built-in procedure represented by a
            // function pointer.
            lsp_op_t op = *lsp_as_op(callable);
            return op(args);
        } else {
            assert(false);
        }

    } else {
        // Expression is a literal that can be returned as-is.
        return expr;
    }
}

static void lsp_print(lsp_expr_t *expr) {
    switch (lsp_type(expr)) {
        case LSP_NULL:
            printf("()");
            break;
        case LSP_CONS:
            printf("(");
            while (lsp_type(expr) == LSP_CONS) {
                lsp_print(lsp_car(expr));
                expr = lsp_cdr(expr);
                if (lsp_type(expr) == LSP_CONS) {
                    printf(" ");
                }
            }
            if (lsp_type(expr) != LSP_NULL) {
                printf(" . ");
                lsp_print(expr);
            }
            printf(")");
            break;
        case LSP_INT:
            printf("%i", *lsp_as_int(expr));
            break;
        case LSP_SYM:
            printf("%s", lsp_as_sym(expr));
            break;
        case LSP_OP:
            printf("<builtin>");
            break;
        default:
            assert(false);
    }
}

int main(int argc, char **argv) {
    heap_data = malloc(128 * 1024 * 1024);
    heap_ptr = heap_data + sizeof(lsp_type_t);

    lsp_expr_t *ast = lsp_parse();
    lsp_expr_t *result = lsp_eval(lsp_car(ast), lsp_default_env());
    lsp_print(result);
}
