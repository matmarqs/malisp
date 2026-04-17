#ifndef _READER_H
#define _READER_H

#include "mal_obj.h"
#include "str.h"

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

typedef struct {
    PCRE2_SPTR pos;
    PCRE2_SIZE size;
    PCRE2_SIZE offset;
} mal_reader_token_t;

typedef struct {
    string_t buffer;
    mal_reader_token_t token;
    pcre2_code *re;
    pcre2_match_data *match_data;
} mal_reader_t;

mal_obj_t read_str(mal_reader_t *reader, char *str);
void mal_reader_regex_init(mal_reader_t *reader);
void mal_reader_regex_free(mal_reader_t *reader);

#endif // _READER_H
