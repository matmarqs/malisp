#ifndef _ENV_H
#define _ENV_H

#include "types.h"

mal_env_t *mal_env_create(mal_env_t *outer);
bool mal_env_bind(mal_env_t *env, mal_list_t *bind_list, mal_list_t *expr_list, mal_obj_t **err_feedback);
void mal_env_free(mal_env_t *env);
void mal_env_set(mal_env_t *env, string_t str_key, mal_obj_t *mal_val);
bool mal_env_get(mal_env_t *env, string_t str_key, mal_obj_t **mal_val);
void mal_env_register_builtins(mal_env_t *env);

#endif
