#include "mal_obj.h"
#include "types.h"
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

mal_obj_t mal_obj_error_format(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    // Determine size
    int size = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    // Allocate and format
    char *error_str = malloc(size + 1);
    va_start(args, fmt);
    vsnprintf(error_str, size + 1, fmt, args);
    va_end(args);
    return (mal_obj_t){
        .type = MAL_ERROR,
        .data.error = {
            .str = error_str,
            .len = size
        }
    };
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

mal_obj_t mal_obj_boolean(bool boolean) {
    mal_obj_t x = {
        .type = MAL_BOOLEAN,
        .data = {
            .boolean = boolean,
        },
    };
    return x;
}

mal_obj_t mal_obj_nil() {
    mal_obj_t x = {
        .type = MAL_NIL,
        .data = {
            .nil = false,
        },
    };
    return x;
}

mal_obj_t mal_obj_function(mal_obj_t *params, mal_obj_t *body) {
    mal_closure_t *f = malloc(sizeof(mal_closure_t));
    f->params = params;
    f->body = body;
    mal_obj_t x = {
        .type = MAL_FUNCTION,
        .data = {
            .function = f,
        },
    };
    return x;
}


mal_obj_t mal_obj_empty() {
    mal_obj_t x = {
        .type = MAL_EMPTY,
        .data = { 0 },
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
    case MAL_ERROR:
        free(x->data.error.str);
        break;
    case MAL_FUNCTION:
        mal_obj_free(x->data.function->params);
        mal_obj_free(x->data.function->body);
        free(x->data.function);
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
        printf("<builtin>");
        break;
    case MAL_FUNCTION:
        printf("#<function>");
        break;
    case MAL_BOOLEAN:
        printf(mal_object->data.boolean ? "true" : "false");
        break;
    case MAL_NIL:
        printf("nil");
        break;
    default:
        break;
    }
}

char *mal_obj_sprint(mal_obj_t *mal_object) {
    static char _buffer[1024];
    switch (mal_object->type) {
    case MAL_SYMBOL:
        snprintf(_buffer, sizeof(_buffer), "%.*s",
                 mal_object->data.symbol.len, mal_object->data.symbol.str);
        break;
    case MAL_NUMBER:
        snprintf(_buffer, sizeof(_buffer), "%lld", (long long)mal_object->data.number);
        break;
    case MAL_LIST:
        snprintf(_buffer, sizeof(_buffer), "<list>");
        break;
    case MAL_ERROR:
        snprintf(_buffer, sizeof(_buffer), "%.*s",
                 mal_object->data.error.len, mal_object->data.error.str);
        break;
    case MAL_BUILTIN:
        snprintf(_buffer, sizeof(_buffer), "<builtin>");
        break;
    case MAL_FUNCTION:
        snprintf(_buffer, sizeof(_buffer), "#<function>");
        break;
    case MAL_BOOLEAN:
        snprintf(_buffer, sizeof(_buffer), mal_object->data.boolean ? "true" : "false");
        break;
    case MAL_NIL:
        snprintf(_buffer, sizeof(_buffer), "nil");
    default:
        break;
    }
    return _buffer;
}
