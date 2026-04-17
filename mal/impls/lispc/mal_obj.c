#include "mal_obj.h"
IMPLEMENT_DEQUE(mal_obj_t, mal_list, mal_list_t);

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

mal_obj_t mal_obj_error(char *error_cstr) {
    mal_obj_t x = {
        .type = MAL_ERROR,
        .data = {
            .error = {
                .str = error_cstr,
                .len = strlen(error_cstr),
            },
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

mal_obj_t mal_obj_builtin(fun_t fn_ptr) {
    mal_obj_t x = {
        .type = MAL_BUILTIN,
        .data = {
            .builtin_fn = fn_ptr,
        },
    };
    return x;
}

void mal_obj_free(mal_obj_t *x) {
    switch (x->type) {
    case MAL_LIST:
        for (int i = 0; i < mal_list_len(x->data.list); i++) {
            mal_obj_free(mal_list_get(x->data.list, i));
        }
        mal_list_free(x->data.list);
        break;
    default:
        break;
    }
}

void mal_obj_print(mal_obj_t *mal_object) {
    switch (mal_object->type) {
    case MAL_SYMBOL:
        printf("%.*s", mal_object->data.symbol.len, mal_object->data.symbol.str);
        break;
    case MAL_NUMBER:
        printf("%lld", (long long)mal_object->data.number);
        break;
    case MAL_LIST:
        putchar('(');
        for (int i = 0; i < mal_list_len(mal_object->data.list); i++) {
            mal_obj_print(mal_list_get(mal_object->data.list, i));
            if (i != mal_list_len(mal_object->data.list)-1) {
                putchar(' ');
            }
        }
        putchar(')');
        break;
    case MAL_ERROR:
        printf("%.*s", mal_object->data.error.len, mal_object->data.error.str);
        break;
    case MAL_BUILTIN:
        printf("<builtin_function>");
        break;
    default:
        break;
    }
}
