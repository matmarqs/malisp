#include "env.h"
#include "builtin.h"
#include "hashmap.h"
#include "mal_obj.h"
#include "str.h"

IMPLEMENT_HASHMAP(env_table_t, env_table, string_t, mal_obj_t *, str_hash, str_equals);

// this is actually a single linked-list (outer environments)
mal_env_t *mal_env_create(mal_env_t *outer) {
    mal_env_t *env = (mal_env_t *) malloc(sizeof(mal_env_t));
    env->outer = outer;
    env->data = env_table_init(16);
    return env;
}

// returns true if the binding was successful, otherwise returns false
// if err != NULL, a MAL_ERROR, with the error string allocated, will be put in it
bool mal_env_bind(mal_env_t *env, mal_list_t *bind_list, mal_list_t *expr_list, mal_obj_t **err_feedback) {
    if (bind_list && expr_list) {
        int binds_len = mal_list_len(bind_list);
        int exprs_len = mal_list_len(expr_list);
        MAL_BOOL_ASSERT(err_feedback,
                        binds_len == exprs_len,
                        "Error: List of bindings and expressions need to have the same length. "
                        "Expected %d. Got %d", binds_len, exprs_len);
        for (int i = 0; i < binds_len; i++) {
            mal_obj_t **bind_ptr = mal_list_get(bind_list, i);
            mal_obj_t **expr_ptr = mal_list_get(expr_list, i);
            mal_obj_t *bind = *bind_ptr;
            mal_obj_t *expr = *expr_ptr;
            MAL_BOOL_ASSERT(err_feedback, bind->type == MAL_SYMBOL,
                            "Error: Cannot bind non-symbol '%s'", mal_obj_sprint(bind));
            mal_env_set(env, bind->data.symbol, expr);
        }
    }
    return true;
}

void mal_env_free(mal_env_t *env) {
    env_table_t *table = env->data;
    // free the copied string keys and release values
    for (int i = 0; i < table->capacity; i++) {
        if (table->slots[i].state == HM_STATE_OCCUPIED) {
            free(table->slots[i].key.str);
            mal_obj_release(table->slots[i].val);
        }
    }
    env_table_free(table);
    free(env);
}

void mal_env_set(mal_env_t *env, string_t str_key, mal_obj_t *val) {
    // every string key is actually copied (malloc'ed) because the char * pointers
    // come from the input buffer read by readline, and they are freed after each mal_eval()
    // so we need to copy the strings here in order for them to persist in the environment
    mal_obj_t *existing_ptr = NULL;
    if (env_table_get(env->data, str_key, &existing_ptr)) {
        mal_obj_release(existing_ptr);
        env_table_remove(env->data, str_key);
    }
    mal_obj_retain(val);
    env_table_set(env->data, str_copy(str_key), val);
}

bool mal_env_get(mal_env_t *env, string_t str_key, mal_obj_t **out) {
    while (env) {
        if (env_table_get(env->data, str_key, out)) {
            mal_obj_retain(*out); // caller gets a retained reference
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
