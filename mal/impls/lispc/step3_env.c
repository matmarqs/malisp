#include "mal_obj.h"
#include "reader.h"
#include "fun_hash.h"

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

void mal_eval(mal_obj_t *node, fun_table_t *ftab) {
    fun_t f;
    mal_obj_t first_symbol;
    switch (node->type) {
    case MAL_SYMBOL:
        if (fun_table_get(ftab, node->data.symbol, NULL)) {
            return; // it's okay
        }
        else {
            // not okay, change the node to be an MAL_ERROR
            //printf("type = %d, str = %.*s, len = %d\n", node->type, node->data.symbol.len, node->data.symbol.str, node->data.symbol.len); // debugging line
            *node = mal_obj_error(ERROR_STR_SYMBOL_NOT_FOUND);
        }
        break;
    case MAL_LIST:
        if (!mal_list_is_empty(node->data.list)) {
            // HERE is for special symbols, such as 'def!', 'let*', etc.
            mal_list_peek_front(node->data.list, &first_symbol);
            if (first_symbol.type != MAL_SYMBOL) {
                mal_obj_free(node);
                *node = mal_obj_error(ERROR_STR_FIRST_NOT_SYMBOL);
                return;
            }
            // BELOW is for standard functions
            // first, eval all elements of the list
            for (int i = 0; i < mal_list_len(node->data.list); i++) {
                mal_eval(mal_list_get(node->data.list, i), ftab);
            }
            mal_list_peek_front(node->data.list, &first_symbol);
            if (first_symbol.type != MAL_SYMBOL) {
                mal_obj_free(node);
                *node = mal_obj_error(ERROR_STR_FIRST_NOT_SYMBOL);
                return;
            }
            // pop the symbol, and let it act on the rest of the list
            mal_list_pop_front(node->data.list, NULL);
            if (fun_table_get(ftab, first_symbol.data.symbol, &f)) {
                f(node);
            }
        }
        break;
    default:
        break;
    }
}

bool mal_rep(fun_table_t *ftab) {
    char *input = mal_read();
    if (!input) {
        puts("");
        return false;
    }
    mal_obj_t root = read_str(input);
    mal_eval(&root, ftab);
    mal_print(&root);
    mal_obj_free(&root);
    free(input);
    return true;
}

int main() {
    fun_table_t *ftab = create_env_functions();
    bool running = true;
    while (running) {
        running = mal_rep(ftab);
    }
    fun_table_free(ftab);
    return 0;
}
