#ifndef _MAL_H
#define _MAL_H

#include "stack.h"

#include <stdint.h>

enum {
    MAL_LIST,
    MAL_NUMBER,
    MAL_SYMBOL,
};

typedef struct mal_list_t mal_list_t;

typedef struct mal_obj_t {
    uint8_t type;
    union {
        mal_list_t *list;
        uint64_t number;
        struct { char *str; int str_len; } symbol;
    } data;
} mal_obj_t;

DEFINE_STACK(mal_obj_t, mal_list, mal_list_t);

mal_obj_t mal_obj_symbol(char *token, int token_sz);
mal_obj_t mal_obj_num(int64_t num);
mal_obj_t mal_obj_list(void);
void mal_obj_free(mal_obj_t *mal_object);
void mal_obj_print(mal_obj_t *mal_object);
void mal_obj_println(mal_obj_t *mal_object);

#endif // _MAL_H
