#include "str.h"
#include <stdlib.h>

// Hash function for strings (djb2 algorithm)
size_t str_hash(string_t s) {
    size_t hash = 5381;
    for (int i = 0; i < s.len; i++) {
        unsigned char c = s.str[i];
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

// Equality function for strings
bool str_equals(string_t a, string_t b) {
    if (a.str == b.str) return true;
    if (!a.str || !b.str) return false;
    if (a.len != b.len) return false;
    return memcmp(a.str, b.str, a.len) == 0;
}

// assume null terminated string as input
string_t str_from_cstr(char *s) {
    string_t str = {
        .str = s,
        .len = strlen(s),
    };
    return str;
}

// calls malloc, needs to be freed after
string_t str_copy(string_t s) {
    char *copy = malloc(s.len + 1);
    memcpy(copy, s.str, s.len);
    copy[s.len] = '\0';
    return (string_t) { .str = copy, .len = s.len };
}
