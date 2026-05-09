#ifndef _CORE_H
#define _CORE_H

#include "mal_obj.h"

typedef struct {
    char *symbol;
    fun_t builtin_func;
} core_ns_t;

extern core_ns_t core_ns[];
extern const int core_ns_len;

#endif // _CORE_H
