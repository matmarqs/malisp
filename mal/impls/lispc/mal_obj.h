#ifndef _MAL_H
#define _MAL_H

#include "types.h"
#include "deque.h"

#include <stdarg.h>

DEFINE_DEQUE(mal_obj_t, mal_list, mal_list_t);

mal_obj_t mal_obj_symbol(char *token, int token_sz);
mal_obj_t mal_obj_num(int64_t num);
mal_obj_t mal_obj_list(int list_size);
mal_obj_t mal_obj_error_format(const char *fmt, ...);
void mal_obj_free(mal_obj_t *mal_object);
void mal_obj_print(mal_obj_t *mal_object);
char *mal_obj_sprint(mal_obj_t *mal_object);
mal_obj_t mal_obj_builtin(fun_t fn_ptr);
mal_obj_t mal_obj_boolean(bool boolean);
mal_obj_t mal_obj_nil();
mal_obj_t mal_obj_empty();
mal_obj_t mal_obj_function(mal_obj_t *params, mal_obj_t *body, mal_env_t *env);

#define MAL_BOOL_ASSERT(feedback, cond, err_fmt, ...)                   \
    if (!(cond)) {                                                      \
        mal_obj_t _err = mal_obj_error_format(err_fmt, ##__VA_ARGS__);  \
        if (feedback) {                                                 \
            *(feedback) = _err;                                         \
        }                                                               \
        return false;                                                   \
    }

#define MAL_OBJ_ASSERT(cond, err_fmt, ...)                              \
    if (!(cond)) {                                                      \
        return mal_obj_error_format(err_fmt, ##__VA_ARGS__);            \
    }

#endif // _MAL_H
