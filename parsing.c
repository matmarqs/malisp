#include <stdio.h>
#include <stdlib.h>

#include <readline/history.h>
#include <readline/readline.h>

#include "mpc.h"

int main(int argc, char **argv) {
    mpc_parser_t *Decimal = mpc_new("decimal");
    mpc_parser_t *Integer = mpc_new("integer");
    mpc_parser_t *Operator = mpc_new("operator");
    mpc_parser_t *Expr = mpc_new("expr");
    mpc_parser_t *Lispy = mpc_new("lispy");

    mpca_lang(MPCA_LANG_DEFAULT,
    "                                                                                    \
        decimal  : /-?[0-9]+\\.[0-9]+/ ;                                                 \
        integer   : /-?[0-9]+/ ;                                                         \
        operator : '+' | '-' | '*' | '/' | '%' | \"div\" | \"add\" | \"sub\" | \"mul\" ; \
        expr     : <decimal> | <integer> | '(' <operator> <expr>+ ')' ;                  \
        lispy    : /^/ <operator> <expr>+ /$/ ;                                          \
    ", Decimal, Integer, Operator, Expr, Lispy);    

    puts("malisp version 0.0.0.1");
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
            mpc_ast_delete(r.output);            
        }
        else {
            // otherwise print the error
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }

    mpc_cleanup(5, Decimal, Integer, Operator, Expr, Lispy);    

    return 0;
}
