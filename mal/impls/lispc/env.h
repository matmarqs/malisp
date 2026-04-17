#ifndef _ENV_H
#define _ENV_H

#include "hashmap.h"
#include "mal_obj.h"
#include "str.h"

DEFINE_HASHMAP(env_table_t, env_table, string_t, mal_obj_t);

typedef struct mal_env_t {
    struct mal_env_t *outer;
    env_table_t *data; // associative data structure, hashmap...
} mal_env_t;

// this is actually a single linked-list (outer environments)
mal_env_t *mal_env_create(mal_env_t *outer);
void mal_env_free(mal_env_t *env);
void mal_env_set(mal_env_t *env, string_t str_key, mal_obj_t mal_val);
bool mal_env_get(mal_env_t *env, string_t str_key, mal_obj_t *mal_val);
void mal_env_register_builtins(mal_env_t *env);

#endif
