#include "builtin.h"

#include <stdint.h>
//#include <stdio.h>

void builtin_add(mal_obj_t *x) {
    if (x->type != MAL_LIST) {
        mal_obj_free_list(x);
        *x = mal_obj_error(ERROR_STR_EVAL_NON_LIST);
        return;
    }
    if (mal_list_is_empty(x->data.list)) {
        mal_obj_free_list(x);
        *x = mal_obj_error(ERROR_STR_EVAL_EMPTY_LIST);
        return;
    }
    int64_t acc = 0;
    for (int i = 0; i < mal_list_len(x->data.list); i++) {
        mal_obj_t *elem = mal_list_get(x->data.list, i);
        if (elem->type == MAL_NUMBER) {
            acc += elem->data.number;
        }
        else if (elem->type == MAL_ERROR) {
            mal_obj_free_list(x);
            *x = *elem;
            return;
        }
        else {
            mal_obj_free_list(x);
            *x = mal_obj_error(ERROR_STR_EXPECTED_NUMBER);
            return;
        }
    }
    mal_obj_free_list(x);
    *x = mal_obj_num(acc);
}

void builtin_sub(mal_obj_t *x) {
    if (x->type != MAL_LIST) {
        mal_obj_free_list(x);
        *x = mal_obj_error(ERROR_STR_EVAL_NON_LIST);
        return;
    }
    if (mal_list_is_empty(x->data.list)) {
        mal_obj_free_list(x);
        *x = mal_obj_error(ERROR_STR_EVAL_EMPTY_LIST);
        return;
    }
    mal_list_t *list = x->data.list;
    if (mal_list_get(list, 0)->type != MAL_NUMBER) {
        mal_obj_free_list(x);
        *x = mal_obj_error(ERROR_STR_EXPECTED_NUMBER);
        return;
    }
    int64_t first_num = mal_list_get(list, 0)->data.number;
    int64_t acc = mal_list_len(list) == 1 ? -first_num : first_num;
    for (int i = 1; i < mal_list_len(list); i++) {
        mal_obj_t *elem = mal_list_get(list, i);
        if (elem->type == MAL_NUMBER) {
            acc -= elem->data.number;
        }
        else if (elem->type == MAL_ERROR) {
            mal_obj_free_list(x);
            *x = *elem;
            return;
        }
        else {
            mal_obj_free_list(x);
            *x = mal_obj_error(ERROR_STR_EXPECTED_NUMBER);
            return;
        }
    }
    mal_obj_free_list(x);
    *x = mal_obj_num(acc);
}

void builtin_mul(mal_obj_t *x) {
    if (x->type != MAL_LIST) {
        mal_obj_free_list(x);
        *x = mal_obj_error(ERROR_STR_EVAL_NON_LIST);
        return;
    }
    if (mal_list_is_empty(x->data.list)) {
        mal_obj_free_list(x);
        *x = mal_obj_error(ERROR_STR_EVAL_EMPTY_LIST);
        return;
    }
    int64_t acc = 1;
    for (int i = 0; i < mal_list_len(x->data.list); i++) {
        mal_obj_t *elem = mal_list_get(x->data.list, i);
        if (elem->type == MAL_NUMBER) {
            acc *= elem->data.number;
        }
        else if (elem->type == MAL_ERROR) {
            mal_obj_free_list(x);
            *x = *elem;
            return;
        }
        else {
            mal_obj_free_list(x);
            *x = mal_obj_error(ERROR_STR_EXPECTED_NUMBER);
            return;
        }
    }
    mal_obj_free_list(x);
    *x = mal_obj_num(acc);
}

void builtin_div(mal_obj_t *x) {
    if (x->type != MAL_LIST) {
        mal_obj_free_list(x);
        *x = mal_obj_error(ERROR_STR_EVAL_NON_LIST);
        return;
    }
    if (mal_list_is_empty(x->data.list)) {
        mal_obj_free_list(x);
        *x = mal_obj_error(ERROR_STR_EVAL_EMPTY_LIST);
        return;
    }
    if (mal_list_len(x->data.list) < 2) {
        mal_obj_free_list(x);
        *x = mal_obj_error(ERROR_STR_NEED_AT_LEAST_2_ARGS);
        return;
    }
    mal_list_t *list = x->data.list;
    if (mal_list_get(list, 0)->type != MAL_NUMBER) {
        mal_obj_free_list(x);
        *x = mal_obj_error(ERROR_STR_EXPECTED_NUMBER);
        return;
    }
    int64_t acc = mal_list_get(list, 0)->data.number;
    for (int i = 1; i < mal_list_len(list); i++) {
        mal_obj_t *elem = mal_list_get(list, i);
        if (elem->type == MAL_NUMBER) {
            if (elem->data.number == 0) {
                mal_obj_free_list(x);
                *x = mal_obj_error(ERROR_STR_DIVISION_BY_ZERO);
                return;
            }
            acc /= elem->data.number;
        }
        else if (elem->type == MAL_ERROR) {
            mal_obj_free_list(x);
            *x = *elem;
            return;
        }
        else {
            mal_obj_free_list(x);
            *x = mal_obj_error(ERROR_STR_EXPECTED_NUMBER);
            return;
        }
    }
    mal_obj_free_list(x);
    *x = mal_obj_num(acc);
}
