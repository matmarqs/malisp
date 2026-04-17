#ifndef _FUN_HASH_H
#define _FUN_HASH_H

#include "mal_obj.h"
#include "hashmap.h"
#include "str.h"

typedef void (*fun_t)(mal_obj_t *); // function: mal_obj_t -> mal_obj_t

//              type_name   prefix     key       value
DEFINE_HASHMAP(fun_table_t, fun_table, string_t, fun_t);

fun_table_t *create_env_functions();

#endif // _FUN_HASH_H
