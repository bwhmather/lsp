#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

typedef enum lsp_type_t {
    LSP_CONS = 1,
    LSP_INT,
    LSP_SYM,
} lsp_type_t;

typedef void lsp_expr_t;

typedef struct lsp_cons_t {
    lsp_expr_t *car;
    lsp_expr_t *cdr;
} lsp_cons_t;


static lsp_type_t lsp_type(lsp_expr_t *expr) {
    lsp_type_t *type_ptr = (lsp_type_t *) expr;
    return *type_ptr;
}

static char *heap_data;
static char *heap_ptr;

static char *lsp_data(lsp_expr_t *expr) {
    return (char *) expr + sizeof(lsp_type_t);
}

static lsp_cons_t *lsp_as_cons(lsp_expr_t *expr) {
    assert(lsp_type(expr) == LSP_CONS);
    return (lsp_cons_t *) expr;
}

static int *lsp_as_int(lsp_expr_t *expr) {
    assert(lsp_type(expr) == LSP_INT);
    return (int *) expr;
}

static char *lsp_as_sym(lsp_expr_t *expr) {
    assert(lsp_type(expr) == LSP_SYM);
}

static lsp_expr_t *lsp_car(lsp_expr_t *expr) {
    lsp_cons_t *cons = lsp_as_cons(expr);
    return cons->car;
}

static lsp_expr_t *lsp_cdr(lsp_expr_t *expr) {
    lsp_cons_t *cons = lsp_as_cons(expr);
    return cons->cdr;
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
}

static void lsp_symbol_push(char character) {
    *heap_ptr = character;
    heap_ptr += 1;
}

static void lsp_symbol_stop() {
    lsp_symbol_push('\0');
}

static lsp_expr_t *lsp_lookup(char *sym, lsp_expr_t *env) {
    for (
        lsp_cons_t *cons = lsp_as_cons(env);
        cons->cdr != NULL;
        cons = lsp_as_cons(cons->cdr)
    ) {
        lsp_cons_t *entry = lsp_as_cons(cons->car);
        char *key = lsp_as_sym(entry->car);
        lsp_expr_t *value = entry->cdr;
        if (strcmp(sym, key) == 0) {
            return value;
        }
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


    // yield an expression each time around the while loop.
    // stack is a list of frames
    // frame 
    //
    // Alternatives:
    //   - Mutate cdr to grow frame
    //   - Build frame in reverse then reverse on )

    // finish:
    //   reverse

    next = getchar();

    while (true) {
        lsp_expr_t *expression = NULL;

        // Consume whitespace whitespace.
        if (next == ' ' || next == '\n') {
            next = getchar();
        } else if (next == '(') {
            next = getchar();

            // Push the current body onto the stack, and start a new one.
            stack = lsp_cons(body, stack);
            body = NULL;

            // No expression to add.  Skip logic at end of loop.
            continue;

        } else if (next == ')') {
            next = getchar();

            // Unwind and reverse the current body list and store it as the
            // current expression.
            while (lsp_type(body) == LSP_CONS) {
                lsp_cons_t *body_head = lsp_as_cons(body);
                expression = lsp_cons(lsp_car(body), expression);
                body = lsp_cdr(body);
            }

            // Pop the containing body from the stack and set it as current.
            body = lsp_car(stack);
            stack = lsp_cdr(stack);
        } else if (next >= '0' && next <= '9') {
            int accumulator = 0;

            // Parse number.
            while (true) {
                if (next >= '0' && next <= '9') {
                    accumulator *= 10;
                    accumulator += (int) (next - '0');
                } else if (
                    next == '(' || next == ')' ||
                    next == ' ' || next == '\n'
                ) {

                } else {
                    assert(false);
                }
            }
        } else {
            // Parse symbol.
            while (true) {


            }
        }

        // Push expression.
        body = lsp_cons(expression, body);
    }
}

static lsp_expr_t *lsp_invoke(lsp_expr_t *proc, lsp_expr_t *env) {

}

lsp_expr_t *lsp_eval(lsp_expr_t *expr, lsp_expr_t *env) {
    switch (lsp_type(expr)) {
        case LSP_SYM:
            return lsp_lookup(expr, env);
        case LSP_CONS:
            if (lsp_type(lsp_car(expr)) == LSP_SYM) {
                char *sym = lsp_as_sym(lsp_car(expr));
                if (strcmp(sym, "if") == 0) {
                } else if (strcmp(sym, "quote") == 0) {
                    return;
                } else if (strcmp(sym, "define") == 0) {
                    return;
                } else if (strcmp(sym, "set!") == 0) {
                    return;
                } else if (strcmp(sym, "lambda") == 0) {
                    return;
                }
            } else {
                lsp_expr_t *proc = eval(expr, env);
                return lsp_invoke(proc, cdr(expr));
            }

        default:
            return expr;
    }

}



int main(int argc, char **argv) {
    heap_data = malloc(128 * 1024 * 1024);
    heap_ptr = heap_data;

    lsp_expr_t *ast = lsp_parse();

    lsp_expr_t *result = lsp_eval(ast, lsp_default_env());
    lsp_dump(result);
}
