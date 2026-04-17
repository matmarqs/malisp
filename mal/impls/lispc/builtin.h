#ifndef _BUILTIN_H
#define _BUILTIN_H

#include "mal_obj.h"

void builtin_add(mal_obj_t *x);
void builtin_sub(mal_obj_t *x);
void builtin_mul(mal_obj_t *x);
void builtin_div(mal_obj_t *x);

#endif // _BUILTIN_H
