#include "builtin.h"

#include <stdint.h>
#include <stdarg.h>

// all the builtin functions assume the elements of the input list were already evaluated
// as such, they cannot be errors (because they were already filtered out by mal_handle_apply)

mal_obj_t builtin_add(mal_list_t *list)
{
    MAL_OBJ_ASSERT(!mal_list_is_empty(list), "Error: Tried to eval empty list");

    int64_t acc = 0;
    for (int i = 0; i < mal_list_len(list); i++)
    {
        mal_obj_t *elem = mal_list_get(list, i);
        MAL_OBJ_ASSERT(elem->type == MAL_NUMBER, "Error: '%s' is not a number", mal_obj_sprint(elem));
        acc += elem->data.number;
    }

    return mal_obj_num(acc);
}

mal_obj_t builtin_sub(mal_list_t *list) {
    MAL_OBJ_ASSERT(!mal_list_is_empty(list), "Error: Tried to eval empty list");

    mal_obj_t *first = mal_list_get(list, 0);
    MAL_OBJ_ASSERT(first->type == MAL_NUMBER,
                   "Error: '%s' is not a number", mal_obj_sprint(first));

    int64_t first_num = first->data.number;
    int64_t acc = mal_list_len(list) == 1 ? -first_num : first_num;

    for (int i = 1; i < mal_list_len(list); i++) {
        mal_obj_t *elem = mal_list_get(list, i);
        MAL_OBJ_ASSERT(elem->type == MAL_NUMBER, "Error: '%s' is not a number", mal_obj_sprint(elem));
        acc -= elem->data.number;
    }

    return mal_obj_num(acc);
}

mal_obj_t builtin_mul(mal_list_t *list) {
    MAL_OBJ_ASSERT(!mal_list_is_empty(list), "Error: Tried to eval empty list");

    int64_t acc = 1;
    for (int i = 0; i < mal_list_len(list); i++) {
        mal_obj_t *elem = mal_list_get(list, i);
        MAL_OBJ_ASSERT(elem->type == MAL_NUMBER, "Error: '%s' is not a number", mal_obj_sprint(elem));
        acc *= elem->data.number;
    }

    return mal_obj_num(acc);
}

mal_obj_t builtin_div(mal_list_t *list) {
    MAL_OBJ_ASSERT(mal_list_len(list) >= 2, "Error: Division expects at least two arguments. "
               "Got %d", mal_list_len(list));

    mal_obj_t *first = mal_list_get(list, 0);
    MAL_OBJ_ASSERT(first->type == MAL_NUMBER,
                   "Error: '%s' is not a number", mal_obj_sprint(first));

    int64_t acc = first->data.number;
    for (int i = 1; i < mal_list_len(list); i++) {
        mal_obj_t *elem = mal_list_get(list, i);
        MAL_OBJ_ASSERT(elem->type == MAL_NUMBER, "Error: '%s' is not a number", mal_obj_sprint(elem));
        MAL_OBJ_ASSERT(elem->data.number != 0, "Error: Encountered division by zero");
        acc /= elem->data.number;
    }

    return mal_obj_num(acc);
}
