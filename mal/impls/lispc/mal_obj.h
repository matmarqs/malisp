#ifndef _MAL_H
#define _MAL_H

#include "deque.h"
#include "str.h"

#include <stdint.h>

enum {
    MAL_LIST,
    MAL_NUMBER,
    MAL_SYMBOL,
    MAL_ERROR,
};

typedef struct mal_list_t mal_list_t;

typedef struct mal_obj_t {
    uint8_t type;
    union {
        mal_list_t *list;
        uint64_t number;
        string_t symbol;
        string_t error;
    } data;
} mal_obj_t;

DEFINE_DEQUE(mal_obj_t, mal_list, mal_list_t);

mal_obj_t mal_obj_symbol(char *token, int token_sz);
mal_obj_t mal_obj_num(int64_t num);
mal_obj_t mal_obj_list(void);
mal_obj_t mal_obj_error(char *error_cstr);
void mal_obj_free(mal_obj_t *mal_object);
void mal_obj_print(mal_obj_t *mal_object);

#define ERROR_STR_SYMBOL_NOT_FOUND "Error: symbol not found"
#define ERROR_STR_FIRST_NOT_SYMBOL "Error: first element of list is not a symbol"
#define ERROR_STR_EVAL_NON_LIST "Error: Tried to evaluate a non-list object"
#define ERROR_STR_EVAL_EMPTY_LIST "Error: Tried to evaluate empty list"
#define ERROR_STR_EXPECTED_NUMBER "Error: Expected number"

#endif // _MAL_H
