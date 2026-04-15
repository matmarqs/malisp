#ifndef _STR_H
#define _STR_H

#include <string.h>
#include <stdbool.h>

typedef struct {
    char *str;
    int len;
} string_t;

size_t str_hash(string_t s);
bool str_equals(string_t a, string_t b);
string_t str_from_cstr(char *s);

#endif // _STR_H
