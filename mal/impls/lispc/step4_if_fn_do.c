#include "mal_obj.h"
#include "reader.h"
#include "env.h"
#include "types.h"

#include <stdio.h>
#include <editline.h>
#include <stdlib.h>
#include <stdbool.h>

mal_obj_t mal_eval(mal_env_t *env, mal_obj_t *node);

char *mal_read() {
    char *input = readline("user> ");
    if (input) {
        add_history(input);
    }
    return input;
}

void mal_print(mal_obj_t *root) {
    if (root->type != MAL_EMPTY) {
        mal_obj_print(root);
        putchar('\n');
    }
}

mal_obj_t mal_eval_noop(mal_obj_t *node)
{
    return *node; // shallow copy
}

// assumes that node->type == MAL_SYMBOL
mal_obj_t mal_eval_symbol(mal_env_t *env, mal_obj_t *node)
{
    mal_obj_t value;
    MAL_OBJ_ASSERT(mal_env_get(env, node->data.symbol, &value),
                   "Error: Symbol '%s' not found", mal_obj_sprint(node));
    return value;
}

mal_obj_t mal_handle_apply(mal_env_t *env, mal_obj_t *list_obj)
{
    mal_list_t *list = list_obj->data.list;
    int list_len = mal_list_len(list);
    mal_obj_t *zeroth = mal_list_get(list, 0);

    MAL_OBJ_ASSERT(zeroth->type == MAL_SYMBOL,
                   "First element '%s' of expression is not a symbol", mal_obj_sprint(zeroth));
    mal_obj_t func = mal_eval(env, zeroth);
    if (func.type == MAL_ERROR)
        return func;

    mal_list_t *list_to_eval = mal_list_create(8);
    for (int i = 0; i < list_len; i++)
    {
        mal_obj_t result = mal_eval(env, mal_list_get(list, i));
        if (result.type == MAL_ERROR)
        {
            mal_list_free(list_to_eval);
            return result;
        }
        mal_list_push_back(list_to_eval, result);
    }

    if (func.type == MAL_BUILTIN)
    {
        return func.data.builtin_fn(list_to_eval);
    }
    else if (func.type == MAL_FUNCTION)
    {
        mal_closure_t *f = &func.data.function;
        mal_env_t *call_env = mal_env_create(env);
        if (!mal_env_bind(call_env, f->params, list_to_eval,
    }


    // from the loop above, the first symbol was already evaluated to MAL_BUILTIN or MAL_ERROR
    mal_obj_t func_symbol;
    mal_list_peek_front(list, &func_symbol);
    MAL_OBJ_ASSERT(list_obj, func_symbol.type != MAL_ERROR,
                   "%s", mal_obj_sprint(&func_symbol));
    // pop the symbol, and let it act on the rest of the list
    mal_list_pop_front(list, NULL);
    if (func_symbol.type == MAL_BUILTIN) { /* builtin function */
        func_symbol.data.builtin_fn(list_obj);
    }
    else if (func_symbol.type == MAL_FUNCTION) { /* user defined function, a.k.a. closure */
        /* FIXME: THERE IS A BIG PROBLEM HERE IF THE MAL_FUNCTION IS IN "def!" */
        mal_closure_t *f = func_symbol.data.function;
        mal_env_t *call_env = mal_env_create(env);
        if (!mal_env_bind(call_env, f->params, list_obj, list_obj)) {
            /* mal_env_bind frees the upper node and puts an error in it */
            /* we need to free the closure (that was popped from the upper node) */
            mal_obj_free(&func_symbol); /* should not free if the MAL_FUNCTION comes from def!, we want to reuse it */
            return false;
        }
        mal_eval(f->body, call_env); /* this is problematic, because we actually lose the function body (because the evaluation is in-place) */
        mal_obj_free(list_obj);
        *list_obj = *(f->body);
        mal_env_free(call_env);
        mal_obj_free(&func_symbol); /* we actually don't want to do that if the MAL_FUNCTION comes from "def!" */
    }
    return true;
    break;
}

mal_obj_t mal_apply(mal_env_t *env, mal_obj_t *node)
{
}

// assumes 0th element of MAL_LIST is the symbol "def!"
// we expect (def! symbol_name (expr arg1 arg2))
mal_obj_t mal_handle_def(mal_env_t *env, mal_obj_t *list_obj)
{
    mal_list_t *list = list_obj->data.list;
    MAL_OBJ_ASSERT(mal_list_len(list) == 3,
                   "Error: def! expects 2 arguments. Got %d", mal_list_len(list)-1);

    mal_obj_t *first = mal_list_get(list, 1);
    MAL_OBJ_ASSERT(first->type == MAL_SYMBOL,
                   "First element '%s' of def! is not a symbol", mal_obj_sprint(first));

    mal_obj_t *second = mal_list_get(list, 2);
    mal_obj_t result = mal_eval(env, second);

    /* never set errors in the environment */
    if (result.type != MAL_ERROR) {
        string_t key = first->data.symbol;
        mal_env_set(env, key, result);
    }
    return result;
}

mal_obj_t mal_handle_let(mal_env_t *env, mal_obj_t *list_obj)
{
    mal_list_t *list = list_obj->data.list;
    MAL_OBJ_ASSERT(mal_list_len(list) == 3,
                   "Error: let* expects 2 arguments. "
                   "Got %d", mal_list_len(list));

    mal_obj_t *first = mal_list_get(list, 1);
    MAL_OBJ_ASSERT(first->type == MAL_LIST && mal_list_len(first->data.list) % 2 == 0,
                   "Error: The first argument of let* must be like "
                   "(key1 expr1 key2 expr2 ...)");

    mal_list_t *bindings = first->data.list;
    mal_env_t *new_env = mal_env_create(env);

    for (int i = 0; i < mal_list_len(bindings); i += 2)
    {
        mal_obj_t *symbol = mal_list_get(bindings, i);
        mal_obj_t bounded_symbol;

        bool can_bind = symbol->type == MAL_SYMBOL &&
            !mal_env_get(env, symbol->data.symbol, &bounded_symbol) ||
            bounded_symbol.type != MAL_BUILTIN;

        if (!can_bind)
        {
            mal_env_free(new_env);
            return mal_obj_error_format("Error: Invalid symbol '%s' found in let* expression",
                                        mal_obj_sprint(symbol));
        }

        mal_obj_t value = mal_eval(new_env, mal_list_get(bindings, i+1));
        if (value.type == MAL_ERROR) {
            mal_env_free(new_env);
            return value;
        }

        mal_env_set(new_env, symbol->data.symbol, value);
    }

    mal_obj_t result = mal_eval(new_env, mal_list_get(list, 2));
    mal_env_free(new_env);
    return result;
}

mal_obj_t mal_handle_fn(mal_env_t *env, mal_obj_t *list_obj)
{
    mal_list_t *list = list_obj->data.list;
    MAL_OBJ_ASSERT(mal_list_len(list) == 3,
                   "Error: fn* expects 2 arguments. Got %d", mal_list_len(list));

    mal_obj_t *binds = mal_list_get(list, 1);
    mal_obj_t *body = mal_list_get(list, 2);
    return mal_obj_function(binds, body);
}

// assumes node->type == MAL_LIST
mal_obj_t mal_eval_list(mal_env_t *env, mal_obj_t *node)
{
    mal_list_t *list = node->data.list;
    if (mal_list_is_empty(list)) // return new empty list
        return mal_obj_list(0);

    mal_obj_t *first = mal_list_get(list, 0);
    if (first->type == MAL_SYMBOL) {
        char *symbol = first->data.symbol.str;
        if (strncmp(symbol, "def!", 4) == 0)
        {
            return mal_handle_def(env, node);
        }
        if (strncmp(symbol, "let*", 4) == 0)
        {
            return mal_handle_let(env, node);
        }
        if (strncmp(symbol, "fn*", 3) == 0)
        {
            return mal_handle_fn(env, node);
        }
    }

    /* standard apply case here */
    return mal_handle_apply(env, node);
}

mal_obj_t mal_eval(mal_env_t *env, mal_obj_t *node) {
    switch (node->type) {
    case MAL_SYMBOL:
        return mal_eval_symbol(env, node);
    case MAL_LIST:
        return mal_eval_list(env, node);
    default:
        break;
    }
    return mal_eval_noop(node);
}

bool mal_rep(mal_reader_t *reader, mal_env_t *env) {
    char *input = mal_read();
    if (!input) {
        puts("");
        return false;
    }
    mal_obj_t root = read_str(reader, input);
    mal_eval(env, &root);
    mal_print(&root);
    mal_obj_free(&root);
    free(input);
    return true;
}

int main() {
    mal_reader_t reader;
    mal_reader_regex_init(&reader);
    mal_env_t *env = mal_env_create(NULL); // create env, alloc hashmap for symbols
    mal_env_register_builtins(env); // register the builtin functions: add, sub, mul, div, etc.

    bool running = true;
    while (running) {
        running = mal_rep(&reader, env);
    }

    mal_env_free(env);          /* this cleans MAL_FUNCTION's */
    mal_reader_regex_free(&reader);
    return 0;
}
