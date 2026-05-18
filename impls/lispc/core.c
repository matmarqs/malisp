#include "core.h"

#include <stdint.h>
#include <stdarg.h>

mal_obj_t *builtin_add(mal_list_t *list)
{
    MAL_OBJ_ASSERT(!mal_list_is_empty(list), "Error: Tried to eval empty list");

    int64_t acc = 0;
    for (int i = 0; i < mal_list_len(list); i++)
    {
        mal_obj_t *elem = *(mal_list_get(list, i));
        MAL_OBJ_ASSERT(elem->type == MAL_NUMBER, "Error: '%s' is not a number", mal_obj_sprint(elem));
        acc += elem->data.number;
    }

    return mal_obj_num(acc);
}

mal_obj_t *builtin_sub(mal_list_t *list) {
    MAL_OBJ_ASSERT(!mal_list_is_empty(list), "Error: Tried to eval empty list");

    mal_obj_t *first = *(mal_list_get(list, 0));
    MAL_OBJ_ASSERT(first->type == MAL_NUMBER,
                   "Error: '%s' is not a number", mal_obj_sprint(first));

    int64_t first_num = first->data.number;
    int64_t acc = mal_list_len(list) == 1 ? -first_num : first_num;

    for (int i = 1; i < mal_list_len(list); i++) {
        mal_obj_t *elem = *(mal_list_get(list, i));
        MAL_OBJ_ASSERT(elem->type == MAL_NUMBER, "Error: '%s' is not a number", mal_obj_sprint(elem));
        acc -= elem->data.number;
    }

    return mal_obj_num(acc);
}

mal_obj_t *builtin_mul(mal_list_t *list) {
    MAL_OBJ_ASSERT(!mal_list_is_empty(list), "Error: Tried to eval empty list");

    int64_t acc = 1;
    for (int i = 0; i < mal_list_len(list); i++) {
        mal_obj_t *elem = *(mal_list_get(list, i));
        MAL_OBJ_ASSERT(elem->type == MAL_NUMBER, "Error: '%s' is not a number", mal_obj_sprint(elem));
        acc *= elem->data.number;
    }

    return mal_obj_num(acc);
}

mal_obj_t *builtin_div(mal_list_t *list) {
    MAL_OBJ_ASSERT(mal_list_len(list) >= 2, "Error: Division expects at least two arguments. "
               "Got %d", mal_list_len(list));

    mal_obj_t *first = *(mal_list_get(list, 0));
    MAL_OBJ_ASSERT(first->type == MAL_NUMBER,
                   "Error: '%s' is not a number", mal_obj_sprint(first));

    int64_t acc = first->data.number;
    for (int i = 1; i < mal_list_len(list); i++) {
        mal_obj_t *elem = *(mal_list_get(list, i));
        MAL_OBJ_ASSERT(elem->type == MAL_NUMBER, "Error: '%s' is not a number", mal_obj_sprint(elem));
        MAL_OBJ_ASSERT(elem->data.number != 0, "Error: Encountered division by zero");
        acc /= elem->data.number;
    }

    return mal_obj_num(acc);
}

mal_obj_t *builtin_prn(mal_list_t *list) {
    int num_args = mal_list_len(list);
    MAL_OBJ_ASSERT(num_args == 1, "Error: 'prn' only accepts 1 argument. Got %d", num_args);

    mal_obj_t *first = *mal_list_get(list, 0);
    mal_obj_println(first, true);

    return mal_obj_nil();
}

mal_obj_t *builtin_list(mal_list_t *list) {
    int len = mal_list_len(list);
    mal_obj_t *new_obj = mal_obj_list((3 * len) / 2);
    mal_list_t *new_list = new_obj->data.list;
    for (int i = 0; i < len; i++) {
        mal_obj_t *arg = *mal_list_get(list, i);
        mal_obj_retain(arg);
        mal_list_push_back(new_list, arg);
    }
    return new_obj;
}

mal_obj_t *builtin_list_question_mark(mal_list_t *list) {
    int num_args = mal_list_len(list);
    MAL_OBJ_ASSERT(num_args == 1, "Error: 'list?' only accepts 1 argument. Got %d", num_args);

    mal_obj_t *first = *mal_list_get(list, 0);

    if (first->type == MAL_LIST) {
        return mal_obj_boolean(true);
    }
    else {
        return mal_obj_boolean(false);
    }
}

mal_obj_t *builtin_empty_question_mark(mal_list_t *list) {
    int num_args = mal_list_len(list);
    MAL_OBJ_ASSERT(num_args == 1, "Error: 'empty?' only accepts 1 argument. Got %d", num_args);

    mal_obj_t *first = *mal_list_get(list, 0);

    if (first->type == MAL_LIST) {
        if (mal_list_is_empty(first->data.list)) {
            return mal_obj_boolean(true);
        }
        else {
            return mal_obj_boolean(false);
        }
    }
    else {
        return mal_obj_error_format("Error: '%s' parameter of 'empty?' is not a list",
                                    mal_obj_sprint(first));
    }
}

mal_obj_t *builtin_count(mal_list_t *list) {
    int num_args = mal_list_len(list);
    MAL_OBJ_ASSERT(num_args == 1, "Error: 'count' only accepts 1 argument. Got %d", num_args);

    mal_obj_t *first = *mal_list_get(list, 0);

    if (first->type == MAL_LIST) {
        return mal_obj_num(mal_list_len(first->data.list));
    }
    else {
        return mal_obj_num(0);
    }
}

mal_obj_t *builtin_equal(mal_list_t *list) {
    int num_args = mal_list_len(list);
    MAL_OBJ_ASSERT(num_args == 2, "Error: '=' only accepts 2 argument. Got %d", num_args);

    mal_obj_t *first = *mal_list_get(list, 0);
    mal_obj_t *second = *mal_list_get(list, 1);

    return mal_obj_boolean(mal_obj_equals(first, second));
}

mal_obj_t *builtin_less(mal_list_t *list) {
    int num_args = mal_list_len(list);
    MAL_OBJ_ASSERT(num_args == 2, "Error: '<' only accepts 2 argument. Got %d", num_args);

    mal_obj_t *fst = *mal_list_get(list, 0);
    mal_obj_t *snd = *mal_list_get(list, 1);

    MAL_OBJ_ASSERT(fst->type == (snd->type == MAL_NUMBER), "Error: '<' can only compare numbers");

    return mal_obj_boolean((fst->data.number) < (snd->data.number));
}

mal_obj_t *builtin_less_or_equal(mal_list_t *list) {
    int num_args = mal_list_len(list);
    MAL_OBJ_ASSERT(num_args == 2, "Error: '<=' only accepts 2 argument. Got %d", num_args);

    mal_obj_t *fst = *mal_list_get(list, 0);
    mal_obj_t *snd = *mal_list_get(list, 1);

    MAL_OBJ_ASSERT(fst->type == (snd->type == MAL_NUMBER), "Error: '<=' can only compare numbers");

    return mal_obj_boolean((fst->data.number) <= (snd->data.number));
}

mal_obj_t *builtin_greater(mal_list_t *list) {
    int num_args = mal_list_len(list);
    MAL_OBJ_ASSERT(num_args == 2, "Error: '>' only accepts 2 argument. Got %d", num_args);

    mal_obj_t *fst = *mal_list_get(list, 0);
    mal_obj_t *snd = *mal_list_get(list, 1);

    MAL_OBJ_ASSERT(fst->type == (snd->type == MAL_NUMBER), "Error: '>' can only compare numbers");

    return mal_obj_boolean((fst->data.number) > (snd->data.number));
}

mal_obj_t *builtin_greater_or_equal(mal_list_t *list) {
    int num_args = mal_list_len(list);
    MAL_OBJ_ASSERT(num_args == 2, "Error: '>=' only accepts 2 argument. Got %d", num_args);

    mal_obj_t *fst = *mal_list_get(list, 0);
    mal_obj_t *snd = *mal_list_get(list, 1);

    MAL_OBJ_ASSERT(fst->type == (snd->type == MAL_NUMBER), "Error: '>=' can only compare numbers");

    return mal_obj_boolean((fst->data.number) >= (snd->data.number));
}

core_ns_t core_ns[] = {
    { "+", builtin_add },
    { "-", builtin_sub },
    { "*", builtin_mul },
    { "/", builtin_div },
    { "prn", builtin_prn },
    { "list", builtin_list },
    { "list?", builtin_list_question_mark },
    { "empty?", builtin_empty_question_mark },
    { "count", builtin_count },
    { "=", builtin_equal },
    { "<", builtin_less },
    { "<=", builtin_less_or_equal },
    { ">", builtin_greater },
    { ">=", builtin_greater_or_equal },
};

const int core_ns_len = sizeof(core_ns) / sizeof(core_ns[0]);
