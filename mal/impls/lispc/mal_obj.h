#ifndef _MAL_H
#define _MAL_H

#include "types.h"
#include "deque.h"

#include <stdarg.h>

DEFINE_DEQUE(mal_obj_t, mal_list, mal_list_t);

mal_obj_t mal_obj_symbol(char *token, int token_sz);
mal_obj_t mal_obj_num(int64_t num);
mal_obj_t mal_obj_list(void);
mal_obj_t mal_obj_error_format(const char *fmt, ...);
void mal_obj_free(mal_obj_t *mal_object);
void mal_obj_print(mal_obj_t *mal_object);
char *mal_obj_sprint(mal_obj_t *mal_object);
mal_obj_t mal_obj_builtin(fun_t fn_ptr);
mal_obj_t mal_obj_boolean(bool boolean);
mal_obj_t mal_obj_nil();

#define MAL_ASSERT(target, cond, err_fmt, ...)                          \
    if (!(cond)) {                                                      \
        mal_obj_t _err = mal_obj_error_format(err_fmt, ##__VA_ARGS__);  \
        if (target) {                                                   \
            mal_obj_free(target);                                       \
            *(target) = _err;                                           \
        }                                                               \
        return false;                                                   \
    }

#endif // _MAL_H
