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
        ;
    }
    return input;
}

void mal_print(mal_obj_t *root) {
    mal_obj_print(root);
    putchar('\n');
}

void mal_eval(mal_obj_t *node, mal_env_t *env) {
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
            if (strncmp(first.data.symbol.str, "def!", 4) == 0) {
                if (mal_list_len(list) != 3) {
                    mal_obj_free(node);
                    *node = mal_obj_error(ERROR_STR_WRONG_NUM_OF_ARGS);
                    return;
                }
                string_t key = mal_list_get(list, 1)->data.symbol;
                mal_obj_t *value_node = mal_list_get(list, 2);
                mal_eval(value_node, env);
                mal_obj_t value = *value_node;
                mal_env_set(env, key, value);
                mal_obj_free(node);
                *node = value;
                return;
            }
            //// let*
            //if (strncmp(first.data.symbol.str, "let*", 4)) {
            //    // handle let*
            //}
            // BELOW is for standard functions
            // first, eval all elements of the list
            for (int i = 0; i < mal_list_len(list); i++) {
                mal_eval(mal_list_get(list, i), env);
            }
            // from the loop above, the first symbol was already evaluated to MAL_BUILTIN (maybe?)
            mal_obj_t func_symbol;
            mal_list_peek_front(list, &func_symbol);
            if (func_symbol.type != MAL_BUILTIN) {
                mal_obj_free(node);
                *node = mal_obj_error(ERROR_STR_SYMBOL_NOT_FOUND);
                return;
            }
            // pop the symbol, and let it act on the rest of the list
            mal_list_pop_front(list, NULL);
            func_symbol.data.builtin_fn(node);
            return;
        }
        break;
    default:
        break;
    }
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
