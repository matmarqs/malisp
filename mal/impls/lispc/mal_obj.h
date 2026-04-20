#ifndef _MAL_H
#define _MAL_H

#include "deque.h"
#include "str.h"

#include <stdint.h>
#include <stdarg.h>

enum {
    MAL_LIST,
    MAL_NUMBER,
    MAL_SYMBOL,
    MAL_ERROR,
    MAL_BUILTIN,
};

enum {
    MAL_BUILTIN_FN,
    MAL_BUILTIN_BOOL,
    MAL_BUILTIN_NIL,
    MAL_BUILTIN_NUMBER,
};

typedef struct mal_list_t mal_list_t;

typedef struct mal_obj_t mal_obj_t;

typedef void (*fun_t)(mal_obj_t *); // function return void: modify the (mal_obj_t *) in place

struct mal_obj_t {
    uint8_t type;
    union {
        mal_list_t *list;
        int64_t number;
        string_t symbol;
        string_t error;
        struct {
            uint8_t type;
            union {
                fun_t builtin_fn;
                bool builtin_bool;
                bool builtin_nil;
                int64_t builtin_num;
            } data;
        } builtin;
    } data;
};

DEFINE_DEQUE(mal_obj_t, mal_list, mal_list_t);

mal_obj_t mal_obj_symbol(char *token, int token_sz);
mal_obj_t mal_obj_num(int64_t num);
mal_obj_t mal_obj_list(void);
mal_obj_t mal_obj_error_format(const char *fmt, ...);
void mal_obj_free(mal_obj_t *mal_object);
void mal_obj_print(mal_obj_t *mal_object);
char *mal_obj_sprint(mal_obj_t *mal_object);
mal_obj_t mal_obj_builtin_fn(fun_t fn_ptr);
mal_obj_t mal_obj_builtin_bool(bool boolean);
mal_obj_t mal_obj_builtin_nil();
mal_obj_t mal_obj_builtin_num(int64_t num);

#define MAL_ASSERT(target, cond, err_fmt, ...)                          \
    if (!(cond)) {                                                      \
        mal_obj_t _err = mal_obj_error_format(err_fmt, ##__VA_ARGS__);  \
        mal_obj_free(target);                                           \
        *(target) = _err;                                               \
        return;                                                         \
    }

#endif // _MAL_H
