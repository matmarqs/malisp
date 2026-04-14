#ifndef _READER_H
#define _READER_H

#include "mal_obj.h"

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

typedef struct {
    PCRE2_SPTR pos;
    PCRE2_SIZE size;
} mal_reader_token_t;

typedef struct {
    char *buffer;
    size_t size;
    mal_reader_token_t token;
    pcre2_code *re;
    pcre2_match_data *match_data;
    PCRE2_SIZE offset;
} mal_reader_t;

mal_obj_t read_str(char *str);

#endif // _READER_H
