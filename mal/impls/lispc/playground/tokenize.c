#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#include <stdio.h>
#include <string.h>

#define TOGREP "Hello World    This is my   phrase   great \n\n right?  yesss"

int main() {
    pcre2_code *re;
    PCRE2_SPTR pattern = (PCRE2_SPTR)"[A-Za-z]+"; // Matches words
    PCRE2_SPTR subject = (PCRE2_SPTR) TOGREP;
    PCRE2_SIZE subject_size = strlen(TOGREP);
    int errornumber;
    PCRE2_SIZE erroroffset;

    // 1. Compile
    re = pcre2_compile(pattern, PCRE2_ZERO_TERMINATED, 0, &errornumber, &erroroffset, NULL);
    pcre2_match_data *match_data = pcre2_match_data_create_from_pattern(re, NULL);

    // 2. Match
    PCRE2_SIZE offset = 0;
    while (offset < subject_size) {
      int rc = pcre2_match(re, subject, PCRE2_ZERO_TERMINATED, offset, 0, match_data, NULL);
      if (rc < 0) break;

      PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match_data);
      PCRE2_SPTR start = subject + ovector[0];
      PCRE2_SIZE len = ovector[1] - ovector[0];

      printf("%.*s\n", (int)len, start);
      offset = (len == 0) ? offset + 1 : ovector[1];
    }

    // 3. Cleanup
    pcre2_match_data_free(match_data);
    pcre2_code_free(re);
    return 0;
}
