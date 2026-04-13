#ifndef _READER_H
#define _READER_H

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#include "stack.h"

typedef struct {
    PCRE2_SPTR pos;
    PCRE2_SIZE size;
} mal_reader_token_t;

enum {
    MAL_LIST,
    MAL_OP,
    MAL_NUMBER,
};

typedef struct mal_list_t mal_list_t;

typedef struct mal_t {
    uint8_t type;
    union {
        mal_list_t *list;
        uint64_t number;
        char op;
    } data;
} mal_t;

DEFINE_STACK(mal_t, mal_list, mal_list_t);

typedef struct {
    char *buffer;
    size_t size;
    mal_reader_token_t token;
    pcre2_code *re;
    pcre2_match_data *match_data;
} mal_reader_t;

mal_t read_str(char *str);

#endif // _READER_H
