#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <readline/history.h>
#include <readline/readline.h>

#include "mpc.h"

#define VERSION "0.0.0.4"

/* enumeration of possible lval types */
enum { LVAL_NUM, LVAL_ERR };

/* enumeration of possible error types */
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

typedef struct {
    int type;
    int64_t num;
    int err;
} lval;

lval lval_num(int64_t x) {
    lval v;
    v.type = LVAL_NUM;
    v.num = x;
    return v;
}

lval lval_err(int x) {
    lval v;
    v.type = LVAL_ERR;
    v.err = x;
    return v;
}

void lval_print(lval v) {
    switch (v.type) {
    case LVAL_NUM:
        printf("%li\n", v.num);
        break;
    case LVAL_ERR:
        switch(v.err) {
        case LERR_DIV_ZERO:
            printf("Error: Division by zero!\n");
            break;
        case LERR_BAD_OP:
            printf("Error: Invalid operator!\n");
            break;
        case LERR_BAD_NUM:
            printf("Error: Invalid number!\n");
            break;
        }
        break;
    }
}

lval eval_op(lval x, char *op, lval y) {
    // if either value is an error
    if (x.type == LVAL_ERR) { return x; }
    if (y.type == LVAL_ERR) { return y; }

    if (strcmp(op, "+") == 0) { return lval_num(x.num + y.num); }
    if (strcmp(op, "-") == 0) { return lval_num(x.num - y.num); }
    if (strcmp(op, "*") == 0) { return lval_num(x.num * y.num); }
    if (strcmp(op, "/") == 0) {
        return y.num == 0 ? lval_err(LERR_DIV_ZERO) : lval_num(x.num / y.num);
    }
    return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t *t) {
    // if it is a number, return directly (base case)
    if (strstr(t->tag, "number")) {
        errno = 0;
        uint64_t x = strtoll(t->contents, NULL, 10);
        return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
    }

    // the operator is always the second child    
    char *op = t->children[1]->contents;

    lval acc = eval(t->children[2]);
    for (int i = 3; strstr(t->children[i]->tag, "expr"); i++) {
        acc = eval_op(acc, op, eval(t->children[i]));
    }
    return acc;
}

int main(int argc, char **argv) {
    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Operator = mpc_new("operator");
    mpc_parser_t *Expr = mpc_new("expr");
    mpc_parser_t *Lispy = mpc_new("lispy");

    mpca_lang(MPCA_LANG_DEFAULT,
    "                                                         \
        number   : /-?[0-9]+/ ;                               \
        operator : '+' | '-' | '*' | '/' ;                    \
        expr     : <number> | '(' <operator> <expr>+ ')' ;    \
        lispy    : /^/ <operator> <expr>+ /$/ ;               \
    ", Number, Operator, Expr, Lispy);    

    puts("malisp version " VERSION);
    puts("Press Ctrl+c to exit");

    while (1) {    
        char *input = readline("lisp> ");
        if (!input)
            break;          
        add_history(input);

        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r)) {
            // On success print the AST
            //mpc_ast_print(r.output);
            lval v = eval(r.output);
            //printf("\nResult: ");
            lval_print(v);
            mpc_ast_delete(r.output);            
        }
        else {
            // otherwise print the error
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }

    mpc_cleanup(4, Number, Operator, Expr, Lispy);    

    return 0;
}
