#include "fun_hash.h"
#include "mal_obj.h"

IMPLEMENT_HASHMAP(fun_table_t, fun_table, string_t, fun_t, NULL, str_hash, str_equals);

mal_obj_t builtin_add(mal_obj_t *x) {
    return *x;
}

mal_obj_t builtin_sub(mal_obj_t *x) {
    return *x;
}

mal_obj_t builtin_mul(mal_obj_t *x) {
    return *x;
}

mal_obj_t builtin_div(mal_obj_t *x) {
    return *x;
}

fun_table_t *create_env_functions() {
    fun_table_t *ftab = fun_table_init(16);
    fun_table_set(ftab, str_from_cstr("+"), builtin_add);
    fun_table_set(ftab, str_from_cstr("-"), builtin_sub);
    fun_table_set(ftab, str_from_cstr("*"), builtin_mul);
    fun_table_set(ftab, str_from_cstr("/"), builtin_div);
    return ftab;
}
