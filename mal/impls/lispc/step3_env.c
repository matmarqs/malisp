#include "mal_obj.h"
#include "reader.h"
#include "env.h"

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


/* TODO:
 * Update step3_env.qx to use the new Env type to create the repl_env (with a nil outer value) and use the set method to add the numeric functions.
 * Modify EVAL to call the get method on the env parameter.
*/

void mal_eval(mal_obj_t *node, mal_env_t *env) {
    fun_t f;
    mal_obj_t temp_obj;
    mal_obj_t first;
    mal_list_t *list;
    switch (node->type) {
    case MAL_SYMBOL:
        if (mal_env_get(env, node->data.symbol, &temp_obj)) {
            mal_obj_free(node);
            *node = temp_obj;
            return; // evaluate to object correspondent with symbol in the environment
        }
        else {
            // not okay, change the node to be an MAL_ERROR
            //printf("type = %d, str = %.*s, len = %d\n", node->type, node->data.symbol.len, node->data.symbol.str, node->data.symbol.len); // debugging line
            mal_obj_free(node);
            *node = mal_obj_error(ERROR_STR_SYMBOL_NOT_FOUND);
        }
        break;
    case MAL_LIST:
        list = node->data.list;
        if (!mal_list_is_empty(list)) {
            // HERE is for special symbols, such as 'def!', 'let*', etc.
            mal_list_peek_front(list, &first);
            if (first.type != MAL_SYMBOL) {
                mal_obj_free(node);
                *node = mal_obj_error(ERROR_STR_FIRST_NOT_SYMBOL);
                return;
            }
            // def!
            if (strncmp(first.data.symbol.str, "def!", 4)) {
                if (mal_list_len(list) != 3) {
                    mal_obj_free(node);
                    *node = mal_obj_error(ERROR_STR_WRONG_NUM_OF_ARGS);
                    return;
                }
                mal_eval(mal_list_get(list, 2), env);
                mal_env_set(env, mal_obj_symbol_persistent(mal_list_get(list, 1)->data.symbol), *mal_list_get(list, 2));
                mal_obj_free(node);
                return;
            }
            // let*
            if (strncmp(first.data.symbol.str, "let*", 4)) {
                // handle let*
            }
            // BELOW is for standard functions
            // first, eval all elements of the list
            for (int i = 0; i < mal_list_len(list); i++) {
                mal_eval(mal_list_get(list, i), env);
            }
            mal_list_peek_front(list, &first);
            if (first.type != MAL_SYMBOL) {
                mal_obj_free(node);
                *node = mal_obj_error(ERROR_STR_FIRST_NOT_SYMBOL);
                return;
            }
            // pop the symbol, and let it act on the rest of the list
            mal_list_pop_front(list, &first);
            if (mal_env_get(env, first.data.symbol, &first)) {
                if (first.type == MAL_BUILTIN) {
                    first.data.builtin_fn(node);
                    return;
                }
                else {
                    mal_obj_free(node);
                    *node = mal_obj_error(ERROR_STR_SYMBOL_NOT_FOUND);
                    return;
                }
            }
        }
        break;
    default:
        break;
    }
}

bool mal_rep(mal_env_t *env) {
    char *input = mal_read();
    if (!input) {
        puts("");
        return false;
    }
    mal_obj_t root = read_str(input);
    mal_eval(&root, env);
    mal_print(&root);
    mal_obj_free(&root);
    free(input);
    return true;
}

int main() {
    mal_env_t *env = mal_env_create(NULL); // create env, alloc hashmap for symbols
    mal_env_register_builtins(env); // register the builtin functions: add, sub, mul, div, etc.
    bool running = true;
    while (running) {
        running = mal_rep(env);
    }
    mal_env_free(env);
    return 0;
}
