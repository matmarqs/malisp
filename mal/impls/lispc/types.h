#ifndef _TYPES_H
#define _TYPES_H

#include "str.h"
#include "hashmap.h"

#include <stdint.h>

enum {
    MAL_LIST,
    MAL_NUMBER,
    MAL_SYMBOL,
    MAL_ERROR,
    MAL_BUILTIN,
    MAL_NIL,
    MAL_BOOLEAN,
    MAL_FUNCTION,
};

typedef struct mal_list_t mal_list_t;

typedef struct mal_obj_t mal_obj_t;

typedef bool (*fun_t)(mal_obj_t *); // return bool (if success): modify the (mal_obj_t *) in place

typedef struct mal_env_t mal_env_t; // forward declaration

typedef struct mal_closure_t {
    mal_env_t *env;
    mal_obj_t *params;
    mal_obj_t *body;
} mal_closure_t;

struct mal_obj_t {
    uint8_t type;
    union {
        mal_list_t *list;
        int64_t number;
        string_t symbol;
        string_t error;
        fun_t builtin_fn;
        bool boolean;
        bool nil;
        mal_closure_t *function;
    } data;
};

DEFINE_HASHMAP(env_table_t, env_table, string_t, mal_obj_t);

typedef struct mal_env_t {
    struct mal_env_t *outer;
    env_table_t *data; // associative data structure, hashmap...
} mal_env_t;

#endif // _TYPES_H
