#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <readline/history.h>
#include <readline/readline.h>

#include "mpc.h"

#define VERSION "0.0.0.4"

/* enumeration of possible lval types */
enum { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR };

/* enumeration of possible error types */
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

typedef struct lval {
    int type;
    int64_t num;
    // error and symbol types have some string data
    char *err;
    char *sym;
    // count and pointer to a list of "lval*"
    int count;
    struct lval **cell;
} lval;

lval *lval_num(int64_t x) {
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->num = x;
    return v;
}

lval *lval_err(char *m) {
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_ERR;
    v->err = malloc(strlen(m) + 1);
    strcpy(v->err, m);
    return v;
}

lval *lval_sym(char *s) {
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_SYM;
    v->sym = malloc(strlen(s) + 1);
    strcpy(v->sym, s);
    return v;
}

lval *lval_sexpr(void) {
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

void lval_del(lval *v) {
    switch (v->type) {
    case LVAL_NUM: break;
    case LVAL_ERR: free(v->err); break;
    case LVAL_SYM: free(v->sym); break;
    case LVAL_SEXPR:
        // delete all elements inside, recursively
        for (int i = 0; i < v->count; i++) {
            lval_del(v->cell[i]);
        }
        // free the memory allocated to contain the pointers
        free(v->cell);
        break;
    }
    free(v);
}

lval *lval_read_num(mpc_ast_t *t) {
    errno = 0;
    uint64_t x = strtoll(t->contents, NULL, 10);
    return errno != ERANGE ? lval_num(x) : lval_err("invalid number");
}

lval *lval_add(lval *x, lval *v) {
    x->count++;
    x->cell = realloc(x->cell, sizeof(lval *) * x->count); // horrible performance
    x->cell[x->count-1] = v;
    return x;
}

lval *lval_read(mpc_ast_t *t) {
    if (strstr(t->tag, "number")) { return lval_read_num(t); }
    if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }

    // if root (>) or sexpr then create empty list
    lval *x = NULL;    
    if (strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
    if (strstr(t->tag, "sexpr")) { x = lval_sexpr(); }

    for (int i = 0; i < t->children_num; i++) {    
        if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
        if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
        if (strcmp(t->children[i]->tag, "regex") == 0) { continue; }
        x = lval_add(x, lval_read(t->children[i]));
    }

    return x;
}

void lval_expr_print(lval *v, char open, char close);
void lval_print(lval *v);
void lval_println(lval *v) { lval_print(v); putchar('\n'); }

void lval_print(lval *v) {
    switch (v->type) {
    case LVAL_NUM: printf("%li", v->num); break;
    case LVAL_ERR: printf("Error: %s", v->err); break;
    case LVAL_SYM: printf("%s", v->sym); break;
    case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
    }
}

void lval_expr_print(lval *v, char open, char close) {
    putchar(open);
    for (int i = 0; i < v->count; i++) {
        lval_print(v->cell[i]);
        // don't print trailing space if last element
        if (i != (v->count-1)) {
            putchar(' ');
        }
    }
    putchar(close);
}

lval *lval_eval_sexpr(lval *v) {
    for (int i = 0; i < v->count; i++) {
        v->cell[i]
    }
}

//lval eval_op(lval x, char *op, lval y) {
//    // if either value is an error
//    if (x.type == LVAL_ERR) { return x; }
//    if (y.type == LVAL_ERR) { return y; }
//
//    if (strcmp(op, "+") == 0) { return lval_num(x.num + y.num); }
//    if (strcmp(op, "-") == 0) { return lval_num(x.num - y.num); }
//    if (strcmp(op, "*") == 0) { return lval_num(x.num * y.num); }
//    if (strcmp(op, "/") == 0) {
//        return y.num == 0 ? lval_err(LERR_DIV_ZERO) : lval_num(x.num / y.num);
//    }
//    return lval_err(LERR_BAD_OP);
//}
//
//lval eval(mpc_ast_t *t) {
//    // if it is a number, return directly (base case)
//    if (strstr(t->tag, "number")) {
//    }
//
//    // the symbol is always the second child    
//    char *op = t->children[1]->contents;
//
//    lval acc = eval(t->children[2]);
//    for (int i = 3; strstr(t->children[i]->tag, "expr"); i++) {
//        acc = eval_op(acc, op, eval(t->children[i]));
//    }
//    return acc;
//}

int main(int argc, char **argv) {
    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Symbol = mpc_new("symbol");
    mpc_parser_t *Sexpr = mpc_new("sexpr");
    mpc_parser_t *Expr = mpc_new("expr");
    mpc_parser_t *Lispy = mpc_new("lispy");

    mpca_lang(MPCA_LANG_DEFAULT,
    "                                                       \
        number   : /-?[0-9]+/ ;                             \
        symbol   : '+' | '-' | '*' | '/' ;                  \
        sexpr    : '(' <expr>* ')' ;                        \
        expr     : <number> | <symbol> | <sexpr> ;          \
        lispy    : /^/ <expr>* /$/ ;                        \
    ", Number, Symbol, Sexpr, Expr, Lispy);    

    puts("malisp version " VERSION);
    puts("Press Ctrl+c to exit");

    while (1) {    
        char *input = readline("lisp> ");
        if (!input)
            break;          
        add_history(input);

        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r)) {
            //mpc_ast_print(r.output);
            lval *v = lval_read(r.output);
            lval_println(v);
            lval_del(v);
            mpc_ast_delete(r.output);            
        }
        else {
            // otherwise print the error
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }

    mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lispy);    

    return 0;
}
