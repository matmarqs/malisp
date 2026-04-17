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



#endif
