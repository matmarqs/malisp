#include "builtin.h"

#include <stdint.h>
#include <stdarg.h>

bool builtin_add(mal_obj_t *x) {
    MAL_ASSERT(x, x->type == MAL_LIST, "Error: Tried to eval a non-list object");
    mal_list_t *list = x->data.list;
    MAL_ASSERT(x, !mal_list_is_empty(list), "Error: Tried to eval empty list");
    int64_t acc = 0;
    for (int i = 0; i < mal_list_len(list); i++) {
        mal_obj_t *elem = mal_list_get(list, i);
        MAL_ASSERT(x, elem->type == MAL_NUMBER, "Error: '%s' is not a number", mal_obj_sprint(elem));
        acc += elem->data.number;
    }
    mal_obj_free(x);
    *x = mal_obj_num(acc);
    return true;
}

bool builtin_sub(mal_obj_t *x) {
    MAL_ASSERT(x, x->type == MAL_LIST, "Error: Tried to eval a non-list object");
    MAL_ASSERT(x, !mal_list_is_empty(x->data.list), "Error: Tried to eval empty list");
    mal_list_t *list = x->data.list;
    mal_obj_t *first = mal_list_get(list, 0);
    MAL_ASSERT(x, first->type == MAL_NUMBER,
               "Error: '%s' is not a number", mal_obj_sprint(first));
    int64_t first_num = first->data.number;
    int64_t acc = mal_list_len(list) == 1 ? -first_num : first_num;
    for (int i = 1; i < mal_list_len(list); i++) {
        mal_obj_t *elem = mal_list_get(list, i);
        MAL_ASSERT(x, elem->type == MAL_NUMBER, "Error: '%s' is not a number", mal_obj_sprint(elem));
        acc -= elem->data.number;
    }
    mal_obj_free(x);
    *x = mal_obj_num(acc);
    return true;
}

bool builtin_mul(mal_obj_t *x) {
    MAL_ASSERT(x, x->type == MAL_LIST, "Error: Tried to eval a non-list object");
    mal_list_t *list = x->data.list;
    MAL_ASSERT(x, !mal_list_is_empty(list), "Error: Tried to eval empty list");
    int64_t acc = 1;
    for (int i = 0; i < mal_list_len(list); i++) {
        mal_obj_t *elem = mal_list_get(list, i);
        MAL_ASSERT(x, elem->type == MAL_NUMBER, "Error: '%s' is not a number", mal_obj_sprint(elem));
        acc *= elem->data.number;
    }
    mal_obj_free(x);
    *x = mal_obj_num(acc);
    return true;
}

bool builtin_div(mal_obj_t *x) {
    MAL_ASSERT(x, x->type == MAL_LIST, "Error: Tried to eval a non-list object");
    mal_list_t *list = x->data.list;
    MAL_ASSERT(x, mal_list_len(list) >= 2, "Error: Division expects at least two arguments. "
               "Got %d", mal_list_len(list));
    mal_obj_t *first = mal_list_get(list, 0);
    MAL_ASSERT(x, first->type == MAL_NUMBER,
               "Error: '%s' is not a number", mal_obj_sprint(first));
    int64_t acc = first->data.number;
    for (int i = 1; i < mal_list_len(list); i++) {
        mal_obj_t *elem = mal_list_get(list, i);
        MAL_ASSERT(x, elem->type == MAL_NUMBER, "Error: '%s' is not a number", mal_obj_sprint(elem));
        MAL_ASSERT(x, elem->data.number != 0, "Error: Encountered division by zero");
        acc /= elem->data.number;
    }
    mal_obj_free(x);
    *x = mal_obj_num(acc);
    return true;
}
