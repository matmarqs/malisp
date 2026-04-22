#ifndef _ENV_H
#define _ENV_H

#include "types.h"

// this is actually a single linked-list (outer environments)
mal_env_t *mal_env_create(mal_env_t *outer);
bool mal_env_bind(mal_env_t *env, mal_obj_t *binds, mal_obj_t *exprs, mal_obj_t *err_feedback);
void mal_env_free(mal_env_t *env);
void mal_env_set(mal_env_t *env, string_t str_key, mal_obj_t mal_val);
bool mal_env_get(mal_env_t *env, string_t str_key, mal_obj_t *mal_val);
void mal_env_register_builtins(mal_env_t *env);

#define MAL_ASSERT_FREE_ENV(target, env, cond, err_fmt, ...)            \
    if (!(cond)) {                                                      \
        mal_obj_t _err = mal_obj_error_format(err_fmt, ##__VA_ARGS__);  \
        if (target) {                                                   \
            mal_obj_free(target);                                       \
            *(target) = _err;                                           \
        }                                                               \
        mal_env_free(env);                                              \
        return false;                                                   \
    }

#endif
