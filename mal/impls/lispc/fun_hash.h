#ifndef _FUN_HASH_H
#define _FUN_HASH_H

#include "mal_obj.h"
#include "hashmap.h"
#include "str.h"

typedef mal_obj_t (*fun_t)(mal_obj_t *); // function: mal_obj_t -> mal_obj_t

//              type_name   prefix     key     value  NOT_FOUND_key
DEFINE_HASHMAP(fun_table_t, fun_table, string_t, fun_t, NULL);

fun_table_t *create_env_functions();

#endif // _FUN_HASH_H
