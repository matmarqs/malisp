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

mal_obj_t *mal_eval(mal_obj_t *root, fun_table_t *ftab) {
    fun_t f;
    switch (root->type) {
    case MAL_SYMBOL:
        f = fun_table_get(ftab, root->data.symbol);
        if (f) {
            return root;
        }
        else {
            // throw error "not found"
        }
        break;
    case MAL_LIST:
        for (int i = 0; i < mal_list_len(root->data.list); i++) {

        }
        if (!mal_list_is_empty(root->data.list)) {
        }
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
    //mal_obj_println(&root);
    //input = mal_print(input);
    mal_obj_t evald = mal_eval(&root, ftab);
    mal_print(&evald);
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
