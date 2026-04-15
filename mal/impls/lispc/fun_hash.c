#include "fun_hash.h"
#include "mal_obj.h"

IMPLEMENT_HASHMAP(fun_table_t, fun_table, string_t, fun_t, NULL, str_hash, str_equals);

void builtin_add(mal_obj_t *x) {
    if (x->type != MAL_LIST) {
        mal_obj_free(x);
        *x = mal_obj_error(ERROR_STR_EVAL_NON_LIST);
        return;
    }
    if (mal_list_is_empty(x->data.list)) {
        mal_obj_free(x);
        *x = mal_obj_error(ERROR_STR_EVAL_EMPTY_LIST);
        return;
    }
    int64_t acc = 0;
    for (int i = 0; i < mal_list_len(x->data.list); i++) {
        mal_obj_t *elem = &x->data.list->items[i];
        if (elem->type == MAL_NUMBER) {
            acc += elem->data.number;
        }
        else if (elem->type == MAL_ERROR) {
            mal_obj_free(x);
            *x = *elem;
            return;
        }
        else {
            mal_obj_free(x);
            *x = mal_obj_error(ERROR_STR_EXPECTED_NUMBER);
            return;
        }
    }
    mal_obj_free(x);
    *x = mal_obj_num(acc);
}

void builtin_sub(mal_obj_t *x) {
}

void builtin_mul(mal_obj_t *x) {
}

void builtin_div(mal_obj_t *x) {
}

fun_table_t *create_env_functions() {
    fun_table_t *ftab = fun_table_init(16);
    fun_table_set(ftab, str_from_cstr("+"), builtin_add);
    //fun_table_set(ftab, str_from_cstr("-"), builtin_sub);
    //fun_table_set(ftab, str_from_cstr("*"), builtin_mul);
    //fun_table_set(ftab, str_from_cstr("/"), builtin_div);
    return ftab;
}
