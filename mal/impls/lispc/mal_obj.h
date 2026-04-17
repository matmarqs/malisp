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
    MAL_BUILTIN,
};

typedef struct mal_list_t mal_list_t;

typedef struct mal_obj_t mal_obj_t;

typedef void (*fun_t)(mal_obj_t *); // function return void: modify the (mal_obj_t *) in place

struct mal_obj_t {
    uint8_t type;
    bool persistent;
    union {
        mal_list_t *list;
        int64_t number;
        string_t symbol;
        string_t error;
        fun_t builtin_fn;
    } data;
};

DEFINE_DEQUE(mal_obj_t, mal_list, mal_list_t);

mal_obj_t mal_obj_symbol(char *token, int token_sz);
mal_obj_t mal_obj_num(int64_t num);
mal_obj_t mal_obj_list(void);
mal_obj_t mal_obj_error(char *error_cstr);
void mal_obj_free(mal_obj_t *mal_object);
void mal_obj_print(mal_obj_t *mal_object);
mal_obj_t mal_obj_builtin(fun_t fn_ptr);

#define ERROR_STR_SYMBOL_NOT_FOUND "Error: symbol not found"
#define ERROR_STR_FIRST_NOT_SYMBOL "Error: first element of list is not a symbol"
#define ERROR_STR_EVAL_NON_LIST "Error: Tried to evaluate a non-list object"
#define ERROR_STR_EVAL_EMPTY_LIST "Error: Tried to evaluate empty list"
#define ERROR_STR_EXPECTED_NUMBER "Error: Expected number"
#define ERROR_STR_NEED_AT_LEAST_2_ARGS "Error: Need at least 2 arguments"
#define ERROR_STR_DIVISION_BY_ZERO "Error: Encountered division by zero"
#define ERROR_STR_NOT_FOUND_IN_ENV "Error: Symbol not found in environment"
#define ERROR_STR_WRONG_NUM_OF_ARGS "Error: Number wrong of arguments"

#endif // _MAL_H
