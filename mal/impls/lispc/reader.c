#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#include <string.h>
#include <stdint.h>

typedef struct {
    PCRE2_SPTR pos;
    PCRE2_SIZE size;
} mal_reader_token_t;

enum {
    MAL_LIST,
    MAL_OP,
    MAL_NUMBER,
};

typedef struct mal_t {
    uint8_t type;
    union {
        struct mal_t *list;
        uint64_t number;
        char op;
    } data;
} mal_t;

typedef struct {
    char *buffer;
    size_t size;
    mal_reader_token_t token;
    pcre2_code *re;
    pcre2_match_data *match_data;
} mal_reader_t;

void mal_reader_init(mal_reader_t *reader, char *buffer) {
    reader->buffer = buffer;
    reader->size = strlen(buffer);
    reader->token.pos = NULL;
    reader->token.size = 0;
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

void mal_reader_free(mal_reader_t *reader) {
    pcre2_match_data_free(reader->match_data);
    pcre2_code_free(reader->re);
}

int mal_reader_next(mal_reader_t *reader) {
    PCRE2_SPTR subject = (PCRE2_SPTR) reader->buffer;
    PCRE2_SIZE subject_size = reader->size;
    pcre2_code *re = reader->re;
    pcre2_match_data *match_data = reader->match_data;
    static PCRE2_SIZE offset = 0;
    if (offset < subject_size) {
        int rc = pcre2_match(re, subject, PCRE2_ZERO_TERMINATED, offset, 0, match_data, NULL);
        if (rc < 0) {
            return -1;
        }
        else {
            PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match_data);
            PCRE2_SPTR start = subject + ovector[0];
            PCRE2_SIZE len = ovector[1] - ovector[0];
            offset = (!len) ? offset + 1 : ovector[1];
            reader->token.pos = start;
            reader->token.size = len;
            return offset;
        }
    }
    return -1;
}

mal_t create_op(char op) {
    mal_t x = {
        .type = MAL_OP,
        .data = {
            .op = op,
        },
    };
    return x;
}

mal_t read_atom(mal_reader_t *reader) {
    char *token = (char *)reader->token.pos;
    int token_size = reader->token.size;
    if (strncmp(token, "+", 1)) {
        return create_op('+');
    }
    else if (strncmp(token, "-", 1)) {
        return create_op('-');
    }
    else if (strncmp(token, "*", 1)) {
        return create_op('*');
    }
    else if (strncmp(token, "/", 1)) {
        return create_op('/');
    }
    // match a number here
}

mal_t read_form(mal_reader_t *reader);

mal_t read_list(mal_reader_t *reader) {
    do {
        mal_t read_form(reader);
    } while (1);
    return NULL;
}

mal_t read_form(mal_reader_t *reader) {
    int offset = mal_reader_next(reader);
    char *token = (char *)reader->token.pos;
    int token_size = reader->token.size;
    if (strncmp(token, "(", 1) == 0) {
        return read_list(reader);
    }
    else {
        return read_atom(reader);
    }
}

void read_str(char *str) {
    mal_reader_t *reader = tokenize(str);
    read_form(reader);
}
