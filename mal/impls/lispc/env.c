#include "env.h"
#include "builtin.h"
#include "hashmap.h"
#include "mal_obj.h"
#include "str.h"

IMPLEMENT_HASHMAP(env_table_t, env_table, string_t, mal_obj_t, str_hash, str_equals);

// this is actually a single linked-list (outer environments)
mal_env_t *mal_env_create(mal_env_t *outer) {
    mal_env_t *env = (mal_env_t *) malloc(sizeof(mal_env_t));
    env->outer = outer;
    env->data = env_table_init(16);
    return env;
}

// returns true if the binding was successful, otherwise returns false
// if err != NULL, a MAL_ERROR, with the error string allocated, will be put in it
bool mal_env_bind(mal_env_t *env, mal_obj_t *binds, mal_obj_t *exprs, mal_obj_t *err_feedback) {
    if (binds && exprs) {
        MAL_ASSERT(err_feedback, binds->type == MAL_LIST, "Error: Tried to bind a non-list");
        MAL_ASSERT(err_feedback, exprs->type == MAL_LIST, "Error: Binding needs a list of expressions");
        int binds_len = mal_list_len(binds->data.list);
        int exprs_len = mal_list_len(exprs->data.list);
        MAL_ASSERT(err_feedback,
                   binds_len == exprs_len,
                   "Error: List of bindings and expressions need to have the same length. "
                   "Expected %d. Got %d", binds_len, exprs_len);
        for (int i = 0; i < binds_len; i++) {
            mal_obj_t *bind = mal_list_get(binds->data.list, i);
            mal_obj_t *expr = mal_list_get(exprs->data.list, i);
            // here we use env_table_get instead of mal_env_get
            // the idea is that we can overwrite outer variables (such as global)
            // but we do not want something like (fn* (a a) (+ a a))
            mal_obj_t temp;
            MAL_ASSERT(err_feedback, bind->type == MAL_SYMBOL &&
                       !env_table_get(env->data, bind->data.symbol, NULL) &&
                       (!mal_env_get(env->outer, bind->data.symbol, &temp) ||
                        temp.type != MAL_BUILTIN), // cannot bind a BUILTIN
                       "Error: Tried to bind invalid symbol '%s'", mal_obj_sprint(bind));
            mal_env_set(env, bind->data.symbol, *expr);
        }
    }
    return true;
}

void mal_env_free(mal_env_t *env) {
    env_table_t *table = env->data;
    // free the copied string keys
    for (int i = 0; i < table->capacity; i++) {
        if (table->slots[i].state == HM_STATE_OCCUPIED) {
            free(table->slots[i].key.str);
            mal_obj_free(&table->slots[i].val);
        }
    }
    env_table_free(table);
    free(env);
}

void mal_env_set(mal_env_t *env, string_t str_key, mal_obj_t mal_val) {
    // every string key is actually copied (malloc'ed) because the char * pointers
    // come from the input buffer read by readline, and they are freed after each mal_eval()
    // so we need to copy the strings here in other for them to persist in the environment
    if (env_table_get(env->data, str_key, NULL)) { // exits already, only update
        env_table_set(env->data, str_key, mal_val);
    }
    else { // does not exists, store a copy
        env_table_set(env->data, str_copy(str_key), mal_val);
    }
}

bool mal_env_get(mal_env_t *env, string_t str_key, mal_obj_t *mal_val) {
    while (env) {
        if (env_table_get(env->data, str_key, mal_val)) {
            return true;
        }
        env = env->outer;
    }
    return false;
}

void mal_env_register_builtins(mal_env_t *env) {
    mal_env_set(env, str_from_cstr("+"), mal_obj_builtin(builtin_add));
    mal_env_set(env, str_from_cstr("-"), mal_obj_builtin(builtin_sub));
    mal_env_set(env, str_from_cstr("*"), mal_obj_builtin(builtin_mul));
    mal_env_set(env, str_from_cstr("/"), mal_obj_builtin(builtin_div));
}
