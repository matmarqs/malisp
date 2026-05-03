#ifndef _BUILTIN_H
#define _BUILTIN_H

#include "mal_obj.h"

mal_obj_t builtin_add(mal_list_t *list);
mal_obj_t builtin_sub(mal_list_t *list);
mal_obj_t builtin_mul(mal_list_t *list);
mal_obj_t builtin_div(mal_list_t *list);

#endif // _BUILTIN_H
