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

void mal_eval(mal_obj_t *node, fun_table_t *ftab) {
    fun_t f;
    mal_obj_t first_symbol;
    switch (node->type) {
    case MAL_SYMBOL:
        f = fun_table_get(ftab, node->data.symbol);
        if (f) {
            return; // it's okay
        }
        else {
            // not okay, change the node to be an MAL_ERROR
            printf("type = %d, str = %.*s, len = %d\n", node->type, node->data.symbol.len, node->data.symbol.str, node->data.symbol.len); // debugging line
            *node = mal_obj_error(ERROR_STR_SYMBOL_NOT_FOUND);
        }
        break;
    case MAL_LIST:
        if (!mal_list_is_empty(node->data.list)) {
            for (int i = 0; i < mal_list_len(node->data.list); i++) {
                mal_eval(&node->data.list->items[i], ftab);
            }
            if (node->data.list->items[0].type == MAL_SYMBOL) {
                mal_list_pop_front(node->data.list, &first_symbol);
                f = fun_table_get(ftab, first_symbol.data.symbol);
                f(node);
            }
            else {
                mal_obj_free(node);
                *node = mal_obj_error(ERROR_STR_FIRST_NOT_SYMBOL);
            }
        }
        break;
    default:
        break;
    }
}

void mal_print(mal_obj_t *root) {
    mal_obj_print(root);
    putchar('\n');
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
