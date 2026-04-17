#include "reader.h"
#include "mal_obj.h"

#include <stdio.h>
#include <string.h>

void mal_reader_regex_init(mal_reader_t *reader) {
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

static void mal_reader_buffer_init(mal_reader_t *reader, char *buffer) {
    reader->buffer = str_from_cstr(buffer);
    reader->token = (mal_reader_token_t) { .pos = NULL, .size = 0, .offset = 0};
}

void mal_reader_regex_free(mal_reader_t *reader) {
    pcre2_match_data_free(reader->match_data);
    pcre2_code_free(reader->re);
}

static int mal_reader_next(mal_reader_t *reader) {
    PCRE2_SPTR subject = (PCRE2_SPTR) reader->buffer.str;
    PCRE2_SIZE subject_size = reader->buffer.len;
    pcre2_code *re = reader->re;
    pcre2_match_data *match_data = reader->match_data;
    PCRE2_SIZE offset = reader->token.offset;
    if (offset < subject_size) {
        int rc = pcre2_match(re, subject, PCRE2_ZERO_TERMINATED, offset, 0, match_data, NULL);
        if (rc < 0) {
            return -1;
        }
        else {
            PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match_data);
            PCRE2_SPTR start = subject + ovector[2];
            PCRE2_SIZE len = ovector[3] - ovector[2];
            reader->token.offset = (!len) ? offset + 1 : ovector[1];
            reader->token.pos = start;
            reader->token.size = len;
            //printf("DEBUG: Found token: '%.*s'\n", (int)reader->token.size, reader->token.pos);
            return offset;
        }
    }
    return -1;
}

static mal_obj_t read_atom(mal_reader_t *reader) {
    char *token = (char *)reader->token.pos;
    int token_size = reader->token.size;
    // Check if it's a negative number: starts with '-' followed by digits
    int is_negative = 0;
    int start_idx = 0;
    if (token_size > 1 && token[0] == '-') {
        // Check if remaining chars are digits
        int all_digits = 1;
        for (int i = 1; i < token_size; i++) {
            if (token[i] < '0' || token[i] > '9') {
                all_digits = 0;
                break;
            }
        }
        if (all_digits) {
            is_negative = 1;
            start_idx = 1;
        }
    }
    // Check if it's a positive number (all digits)
    int is_positive = 1;
    if (!is_negative) {
        for (int i = 0; i < token_size; i++) {
            if (token[i] < '0' || token[i] > '9') {
                is_positive = 0;
                break;
            }
        }
    }
    if (is_positive || is_negative) {
        int64_t num = 0;
        for (int i = start_idx; i < token_size; i++) {
            num = num * 10 + (token[i] - '0');
        }
        if (is_negative) {
            num = -num;
        }
        return mal_obj_num(num);
    }
    // match symbols
    return mal_obj_symbol(token, token_size);
}

static mal_obj_t read_form(mal_reader_t *reader);

static mal_obj_t read_list(mal_reader_t *reader) {
    mal_obj_t root = mal_obj_list();
    do {
        int offset = mal_reader_next(reader);
        if (offset == -1) {
            // EOF error
            printf("read_list: Error, offset == -1, EOF\n");
            break;
        }
        char *token = (char *)reader->token.pos;
        if (strncmp(token, ")", 1) == 0) {
            break;
        }
        mal_obj_t mal_object = read_form(reader);
        mal_list_push_back(root.data.list, mal_object);
    } while (1);
    return root;
}

static mal_obj_t read_form(mal_reader_t *reader) {
    // peek at the current token of Reader object
    char *token = (char *)reader->token.pos;
    if (!token) {
        return mal_obj_symbol("", 0);
    }
    if (strncmp(token, "(", 1) == 0) {
        return read_list(reader);
    }
    else {
        return read_atom(reader);
    }
}

mal_obj_t read_str(mal_reader_t *reader, char *str) {
    mal_reader_buffer_init(reader, str);
    mal_reader_next(reader);
    return read_form(reader);
}
