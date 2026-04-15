#include "mal_obj.h"
IMPLEMENT_STACK(mal_obj_t, mal_list, mal_list_t);

#include <stdio.h>

mal_obj_t mal_obj_symbol(char *token, int token_sz) {
    mal_obj_t x = {
        .type = MAL_SYMBOL,
        .data = {
            .symbol = {
                .str = token,
                .len = token_sz,
            },
        },
    };
    return x;
}

mal_obj_t mal_obj_num(int64_t num) {
    mal_obj_t x = {
        .type = MAL_NUMBER,
        .data = {
            .number = num,
        },
    };
    return x;
}

mal_obj_t mal_obj_list(void) {
    mal_obj_t x = {
        .type = MAL_LIST,
        .data = {
            .list = mal_list_create(4), // alloc list in the heap
        },
    };
    return x;
}

void mal_obj_free(mal_obj_t *x) {
    switch (x->type) {
    case MAL_SYMBOL:
    case MAL_NUMBER:
        break;
    case MAL_LIST:
        for (int i = 0; i < mal_list_len(x->data.list); i++) {
            mal_obj_free(&x->data.list->items[i]);
        }
        mal_list_free(x->data.list);
        break;
    }
}

void mal_obj_print(mal_obj_t *mal_object) {
    switch (mal_object->type) {
    case MAL_SYMBOL:
        printf("%.*s", mal_object->data.symbol.len, mal_object->data.symbol.str);
        break;
    case MAL_NUMBER:
        printf("%lld", (long long)mal_object->data.number); break;
    case MAL_LIST:
        putchar('(');
        for (int i = 0; i < mal_list_len(mal_object->data.list); i++) {
            mal_obj_print(&mal_object->data.list->items[i]);
            if (i != mal_list_len(mal_object->data.list)-1) {
                putchar(' ');
            }
        }
        putchar(')');
        break;
    }
}
