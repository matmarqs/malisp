#include "mal_obj.h"
#include "reader.h"
#include "env.h"
#include "types.h"

#include <stdio.h>
#include <editline.h>
#include <stdlib.h>
#include <stdbool.h>

char *mal_read() {
    char *input = readline("user> ");
    if (input) {
        add_history(input);
    }
    return input;
}

void mal_print(mal_obj_t *root) {
    mal_obj_print(root);
    putchar('\n');
}

bool mal_eval(mal_obj_t *node, mal_env_t *env) {
    mal_obj_t temp_obj;
    mal_obj_t first;
    mal_list_t *list;
    switch (node->type) {
    case MAL_SYMBOL:
        MAL_ASSERT(node, mal_env_get(env, node->data.symbol, &temp_obj),
                   "Error: Symbol '%s' not found", mal_obj_sprint(node));
        MAL_ASSERT(node, temp_obj.type != MAL_ERROR, "%s", mal_obj_sprint(&temp_obj));
        mal_obj_free(node);  // there is a problem here, if temp_obj is an error
        *node = temp_obj;    // because we only do a shallow copy
        return true;
        break;
    case MAL_LIST:
        list = node->data.list;
        if (!mal_list_is_empty(list)) {
            // HERE is for special symbols, such as 'def!', 'let*', etc.
            mal_list_peek_front(list, &first);
            if (first.type != MAL_SYMBOL) {
                goto standard_apply;
            }
            // def!
            if (strncmp(first.data.symbol.str, "def!", 4) == 0) {
                MAL_ASSERT(node, mal_list_len(list) == 3,
                           "Error: def! expects 2 arguments. Got %d", mal_list_len(list)-1);
                string_t key = mal_list_get(list, 1)->data.symbol;
                mal_obj_t *value = mal_list_get(list, 2);
                mal_eval(value, env);
                mal_obj_t result = *value; // need to get before free
                MAL_ASSERT(node, value->type != MAL_ERROR, "%s", mal_obj_sprint(value));
                mal_env_set(env, key, result);
                mal_list_pop_back(list, NULL); /* this handles the case where value == MAL_FUNCTION, so it would not double free */
                mal_obj_free(node);
                *node = result;
                return true;
            }
            // let*
            if (strncmp(first.data.symbol.str, "let*", 4) == 0) {
                MAL_ASSERT(node, mal_list_len(list) == 3, "Error: let* expects 2 arguments. "
                           "Got %d", mal_list_len(list));
                mal_obj_t *bind_obj = mal_list_get(list, 1);
                MAL_ASSERT(node, bind_obj->type == MAL_LIST && mal_list_len(bind_obj->data.list) % 2 == 0,
                           "Error: The first argument of let* must be like "
                           "(key1 expr1 key2 expr2 ...)");
                mal_list_t *bindings = bind_obj->data.list;
                mal_env_t *new_env = mal_env_create(env);
                for (int i = 0; i < mal_list_len(bindings); i += 2) {
                    mal_obj_t *symbol = mal_list_get(bindings, i);
                    mal_obj_t *value = mal_list_get(bindings, i+1);
                    mal_obj_t temp;
                    MAL_ASSERT_FREE_ENV(node, new_env, symbol->type == MAL_SYMBOL &&
                               (!mal_env_get(env, symbol->data.symbol, &temp) ||
                                temp.type != MAL_BUILTIN),
                               "Error: Invalid symbol '%s' found in let* expression",
                               mal_obj_sprint(symbol));
                    mal_eval(value, new_env);
                    MAL_ASSERT_FREE_ENV(node, new_env, value->type != MAL_ERROR, "%s", mal_obj_sprint(value));
                    mal_env_set(new_env, symbol->data.symbol, *value);
                }
                mal_obj_t *result = mal_list_get(list, 2);
                mal_eval(result, new_env);
                // if result is an error, freeing node will free the MAL_ERROR twice
                MAL_ASSERT_FREE_ENV(node, new_env, result->type != MAL_ERROR, "%s", mal_obj_sprint(result));
                mal_obj_t result_obj = *result; // need to get before free
                mal_obj_free(node);
                *node = result_obj;
                mal_env_free(new_env);
                return true;
            }
            // fn*
            if (strncmp(first.data.symbol.str, "fn*", 3) == 0) {
                MAL_ASSERT(node, mal_list_len(list) == 3, "Error: fn* expects 2 arguments. "
                           "Got %d", mal_list_len(list));
                mal_env_t *new_env = mal_env_create(env);
                mal_obj_t *binds = mal_list_get(list, 1);
                mal_obj_t *body = mal_list_get(list, 2);
                mal_obj_t closure = mal_obj_function(new_env, binds, body);
                /* here we do not free the node directly, because it would free the function
                   parameters and the body, instead we just free its internal list */
                mal_list_free(node->data.list);
                node->data.list = NULL;
                *node = closure; /* evaluates to a MAL_FUNCTION */
                return true;
            }
        standard_apply:
            // BELOW is for standard functions
            // first, eval all elements of the list
            for (int i = 0; i < mal_list_len(list); i++) {
                mal_eval(mal_list_get(list, i), env);
            }
            // from the loop above, the first symbol was already evaluated to MAL_BUILTIN or MAL_ERROR
            mal_obj_t func_symbol;
            mal_list_peek_front(list, &func_symbol);
            MAL_ASSERT(node, func_symbol.type != MAL_ERROR,
                       "%s", mal_obj_sprint(&func_symbol));
            // pop the symbol, and let it act on the rest of the list
            mal_list_pop_front(list, NULL);
            if (func_symbol.type == MAL_BUILTIN) { /* builtin function */
                func_symbol.data.builtin_fn(node);
            }
            else if (func_symbol.type == MAL_FUNCTION) { /* user defined function, a.k.a. closure */
                mal_closure_t *f = func_symbol.data.function;
                if (!mal_env_bind(f->env, f->params, node, node)) {
                    /* mal_env_bind frees the upper node and puts an error in it */
                    /* we need to free the closure (that was popped from the upper node) */
                    mal_obj_free(&func_symbol);
                    return false;
                }
                mal_eval(f->body, f->env);
                mal_obj_free(node);
                *node = *(f->body);
                mal_obj_free(&func_symbol);
            }
            return true;
        }
        break;
    default:
        break;
    }
    return true;
}

bool mal_rep(mal_reader_t *reader, mal_env_t *env) {
    char *input = mal_read();
    if (!input) {
        puts("");
        return false;
    }
    mal_obj_t root = read_str(reader, input);
    mal_eval(&root, env);
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

    mal_env_free(env);
    mal_reader_regex_free(&reader);
    return 0;
}
