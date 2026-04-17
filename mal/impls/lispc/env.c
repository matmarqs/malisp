#include "env.h"

// NOT_FOUND = mal_obj_error(error_cstr) (function call) is okay, because it's a macro
IMPLEMENT_HASHMAP(env_table_t, env_table, string_t, mal_obj_t, str_hash, str_equals);

// this is actually a single linked-list (outer environments)
mal_env_t *mal_env_create(mal_env_t *outer) {
    mal_env_t *env = (mal_env_t *) malloc(sizeof(mal_env_t));
    env->outer = outer;
    env->data = env_table_init(8);
    return env;
}

void mal_env_free(mal_env_t *env) {
    env_table_free(env->data);
    free(env);
}

void mal_env_set(mal_env_t *env, string_t str_key, mal_obj_t mal_value) {
    env_table_set(env->data, str_key, mal_value);
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
