#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#include <string.h>

typedef struct {
    char *buffer;
    int pos;
} mal_reader_t;

int mal_reader_next(mal_reader_t *reader) {
    reader->pos++;
    return reader->buffer[reader->pos-1];
}

int mal_reader_peek(mal_reader_t *reader) {
    return reader->buffer[reader->pos];
}

void tokenize(char *subject_str) {
    PCRE2_SPTR subject = (PCRE2_SPTR) subject_str;
    PCRE2_SIZE subject_size = strlen(subject_str);

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
    PCRE2_SPTR pattern = (PCRE2_SPTR)"[\\s,]*(~@|[\\[\\]{}()'`~^@]|\\\"(?:\\\\.|[^\\\\\"])*\"?|;.*|[^\\s\\[\\]{}('\"`,;)]*)";
    int error_number;
    PCRE2_SIZE error_offset;

    pcre2_code *re;
    re = pcre2_compile(pattern, PCRE2_ZERO_TERMINATED, 0, &error_number, &error_offset, NULL);
    pcre2_match_data *match_data = pcre2_match_data_create_from_pattern(re, NULL);

    PCRE2_SIZE offset = 0;
    while (offset < subject_size) {
        int rc = pcre2_match(re, subject, PCRE2_ZERO_TERMINATED, offset, 0, match_data, NULL);
        if (rc < 0) {
            break;
        }
        PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match_data);
        PCRE2_SPTR start = subject + ovector[0];
        PCRE2_SIZE len = ovector[1] - ovector[0];

        offset = (!len) ? offset + 1 : ovector[1];
    }

    pcre2_match_data_free(match_data);
    pcre2_code_free(re);
}


//#define PCRE2_CODE_UNIT_WIDTH 8
//#include <pcre2.h>
//#include <stdio.h>
//#include <string.h>
//
//#define TOGREP "Hello World    This is my   phrase   great \n\n right?  yesss"
//
//int main() {
//    pcre2_code *re;
//    PCRE2_SPTR pattern = (PCRE2_SPTR)"[A-Za-z]+"; // Matches words
//    PCRE2_SPTR subject = (PCRE2_SPTR) TOGREP;
//    PCRE2_SIZE subject_size = strlen(TOGREP);
//    int errornumber;
//    PCRE2_SIZE erroroffset;
//
//    // 1. Compile
//    re = pcre2_compile(pattern, PCRE2_ZERO_TERMINATED, 0, &errornumber, &erroroffset, NULL);
//    pcre2_match_data *match_data = pcre2_match_data_create_from_pattern(re, NULL);
//
//    // 2. Match
//    PCRE2_SIZE offset = 0;
//    while (offset < subject_size) {
//      int rc = pcre2_match(re, subject, PCRE2_ZERO_TERMINATED, offset, 0, match_data, NULL);
//      if (rc < 0) break;
//
//      PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match_data);
//      PCRE2_SPTR start = subject + ovector[0];
//      PCRE2_SIZE len = ovector[1] - ovector[0];
//
//      printf("%.*s\n", (int)len, start);
//      offset = (len == 0) ? offset + 1 : ovector[1];
//    }
//
//    // 3. Cleanup
//    pcre2_match_data_free(match_data);
//    pcre2_code_free(re);
//    return 0;
//}

void read_str(char *str) {
    mal_reader_t *reader = tokenize(str);
    read_form(reader);
}
