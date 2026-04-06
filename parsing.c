#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <readline/history.h>
#include <readline/readline.h>

#include "mpc.h"

int64_t eval_op(int64_t x, char *op, int64_t y) {
    if (strcmp(op, "+") == 0) { return x + y; }
    if (strcmp(op, "-") == 0) { return x - y; }
    if (strcmp(op, "*") == 0) { return x * y; }
    if (strcmp(op, "/") == 0) { return x / y; }
    return 0;
}

int64_t eval(mpc_ast_t *t) {
    // if it is a number, return directly (base case)
    if (strstr(t->tag, "number")) {
        return atoi(t->contents);
    }

    // the operator is always the second child    
    char *op = t->children[1]->contents;

    uint64_t acc = eval(t->children[2]);
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

    puts("malisp version 0.0.0.3");
    puts("Press Ctrl+c to exit");

    while (1) {    
        char *input = readline("lisp> ");
        if (!input)
            break;          
        add_history(input);

        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r)) {
            // On success print the AST
            mpc_ast_print(r.output);
            printf("\nResult: %lld\n", (long long) eval(r.output));
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
