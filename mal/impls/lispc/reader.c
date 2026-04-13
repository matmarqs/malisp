#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "reader.h"
IMPLEMENT_STACK(mal_t, mal_list, mal_list_t);

static void mal_reader_init(mal_reader_t *reader, char *buffer) {
    reader->buffer = buffer;
    reader->size = strlen(buffer);
    reader->token.pos = NULL;
    reader->token.size = 0;
    reader->offset = 0;
    /* Regex: [\s,]*(~@|[\[\]{}()'`~^@]|"(?:\\.|[^\\"])*"?|;.*|[^\s\[\]{}('"`,;)]*) */
    /* [\s,]*: ignores all whitespaces or commas */
    /* ~@: captures the 2 special characters ~@ */
    /* [\[\]{}()'`~^@]: captures any single special character, one of []{}()'`~^@ */
    /* "(?:\\.|[^\\"])*"?: starts capturing a double quote, and stops at the next double-quote,
       unless it was preceded by a backslash in which case it includes until the next double-quote
       It will aso match unbalanced strings (no ending double-quote) which should be reported as an error */
    /* ;.*: captures any sequence of characters starting with ; (this ill be used for comments) */
    /* [^\s\[\]{}('"`,;)]*: captures a sequence of zero or more non special characters (e.g symbols,
       numbers, "true", "false", and "nil") and is sort of the inverse of the one above that captures
       special characters */
    PCRE2_SPTR pattern = (PCRE2_SPTR)
        "[\\s,]*"
        "("
            "~@"                         "|"
            "[\\[\\]{}()'`~^@]"          "|"
            "\"(?:\\\\.|[^\\\\\"])*\"?"  "|"
            ";.*"                        "|"
            "[^\\s\\[\\]{}('\"`,;)]*"
        ")";
    int error_number;
    PCRE2_SIZE error_offset;
    reader->re = pcre2_compile(pattern, PCRE2_ZERO_TERMINATED, 0, &error_number, &error_offset, NULL);
    reader->match_data = pcre2_match_data_create_from_pattern(reader->re, NULL);
}

static void mal_reader_free(mal_reader_t *reader) {
    pcre2_match_data_free(reader->match_data);
    pcre2_code_free(reader->re);
}

static int mal_reader_next(mal_reader_t *reader) {
    PCRE2_SPTR subject = (PCRE2_SPTR) reader->buffer;
    PCRE2_SIZE subject_size = reader->size;
    pcre2_code *re = reader->re;
    pcre2_match_data *match_data = reader->match_data;
    PCRE2_SIZE offset = reader->offset;
    if (offset < subject_size) {
        int rc = pcre2_match(re, subject, PCRE2_ZERO_TERMINATED, offset, 0, match_data, NULL);
        if (rc < 0) {
            return -1;
        }
        else {
            PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match_data);
            PCRE2_SPTR start = subject + ovector[2];
            PCRE2_SIZE len = ovector[3] - ovector[2];
            reader->offset = (!len) ? offset + 1 : ovector[1];
            reader->token.pos = start;
            reader->token.size = len;
            //printf("DEBUG: Found token: '%.*s'\n", (int)reader->token.size, reader->token.pos);
            return offset;
        }
    }
    return -1;
}

static mal_t create_symbol(char *token, int token_sz) {
    mal_t x = {
        .type = MAL_SYMBOL,
        .data = {
            .symbol = {
                .str = token,
                .str_len = token_sz,
            },
        },
    };
    return x;
}

static mal_t create_num(int64_t num) {
    mal_t x = {
        .type = MAL_NUMBER,
        .data = {
            .number = num,
        },
    };
    return x;
}

static mal_t read_atom(mal_reader_t *reader) {
    char *token = (char *)reader->token.pos;
    int token_size = reader->token.size;
    // match a number here
    errno = 0;
    char *end;
    int64_t num = strtoll(token, &end, 10);
    if (errno != ERANGE && end != token && *end == '\0') {
        return create_num(num);
    }
    // match symbols
    return create_symbol(token, token_size);
}

static mal_t read_form(mal_reader_t *reader);

static mal_t read_list(mal_reader_t *reader) {
    mal_t root = {
        .type = MAL_LIST,
        .data = {
            .list = NULL,
        },
    };
    mal_list_t *list = mal_list_create(5);
    do {
        int offset = mal_reader_next(reader);
        if (offset == -1) {
            // EOF error
            printf("read_list: Error, offset == -1, EOF\n");
            break;
        }
        char *token = (char *)reader->token.pos;
        int token_size = reader->token.size;
        if (strncmp(token, ")", 1) == 0) {
            break;
        }
        mal_t mal_object = read_form(reader);
        mal_list_push(list, mal_object);
    } while (1);
    root.data.list = list;
    return root;
}

static mal_t read_form(mal_reader_t *reader) {
    // peek at the current token of Reader object
    char *token = (char *)reader->token.pos;
    int token_size = reader->token.size;
    if (strncmp(token, "(", 1) == 0) {
        return read_list(reader);
    }
    else {
        return read_atom(reader);
    }
}

mal_t read_str(char *str) {
    mal_reader_t reader;
    mal_reader_init(&reader, str);
    mal_reader_next(&reader);
    mal_t root = read_form(&reader);
    return root;
}
