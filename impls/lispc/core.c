#define _POSIX_C_SOURCE 200809L // open_memstream
#include <stdio.h>

#include "core.h"
#include "reader.h"

#include <stdint.h>
#include <stdarg.h>

mal_obj_t *builtin_add(mal_list_t *list)
{
    MAL_OBJ_ASSERT(!mal_list_is_empty(list), "Error: Tried to eval empty list");

    int64_t acc = 0;
    for (int i = 0; i < mal_list_len(list); i++)
    {
        mal_obj_t *elem = *(mal_list_get(list, i));
        MAL_OBJ_ASSERT(elem->type == MAL_NUMBER, "Error: '%s' is not a number", mal_obj_sprint(elem));
        acc += elem->data.number;
    }

    return mal_obj_num(acc);
}

mal_obj_t *builtin_sub(mal_list_t *list) {
    MAL_OBJ_ASSERT(!mal_list_is_empty(list), "Error: Tried to eval empty list");

    mal_obj_t *first = *(mal_list_get(list, 0));
    MAL_OBJ_ASSERT(first->type == MAL_NUMBER,
                   "Error: '%s' is not a number", mal_obj_sprint(first));

    int64_t first_num = first->data.number;
    int64_t acc = mal_list_len(list) == 1 ? -first_num : first_num;

    for (int i = 1; i < mal_list_len(list); i++) {
        mal_obj_t *elem = *(mal_list_get(list, i));
        MAL_OBJ_ASSERT(elem->type == MAL_NUMBER, "Error: '%s' is not a number", mal_obj_sprint(elem));
        acc -= elem->data.number;
    }

    return mal_obj_num(acc);
}

mal_obj_t *builtin_mul(mal_list_t *list) {
    MAL_OBJ_ASSERT(!mal_list_is_empty(list), "Error: Tried to eval empty list");

    int64_t acc = 1;
    for (int i = 0; i < mal_list_len(list); i++) {
        mal_obj_t *elem = *(mal_list_get(list, i));
        MAL_OBJ_ASSERT(elem->type == MAL_NUMBER, "Error: '%s' is not a number", mal_obj_sprint(elem));
        acc *= elem->data.number;
    }

    return mal_obj_num(acc);
}

mal_obj_t *builtin_div(mal_list_t *list) {
    MAL_OBJ_ASSERT(mal_list_len(list) >= 2, "Error: Division expects at least two arguments. "
               "Got %d", mal_list_len(list));

    mal_obj_t *first = *(mal_list_get(list, 0));
    MAL_OBJ_ASSERT(first->type == MAL_NUMBER,
                   "Error: '%s' is not a number", mal_obj_sprint(first));

    int64_t acc = first->data.number;
    for (int i = 1; i < mal_list_len(list); i++) {
        mal_obj_t *elem = *(mal_list_get(list, i));
        MAL_OBJ_ASSERT(elem->type == MAL_NUMBER, "Error: '%s' is not a number", mal_obj_sprint(elem));
        MAL_OBJ_ASSERT(elem->data.number != 0, "Error: Encountered division by zero");
        acc /= elem->data.number;
    }

    return mal_obj_num(acc);
}

mal_obj_t *builtin_prn(mal_list_t *list) {
    for (int i = 0; i < mal_list_len(list); i++) {
        mal_obj_fprint(stdout, *mal_list_get(list, i), true);
        if (i != mal_list_len(list) - 1) {
            putchar(' ');
        }
    }
    putchar('\n');
    return mal_obj_nil();
}

mal_obj_t *builtin_println(mal_list_t *list) {
    for (int i = 0; i < mal_list_len(list); i++) {
        mal_obj_fprint(stdout, *mal_list_get(list, i), false);
        if (i != mal_list_len(list) - 1) {
            putchar(' ');
        }
    }
    putchar('\n');
    return mal_obj_nil();
}

mal_obj_t *builtin_pr_str(mal_list_t *list) {
    char *buf;
    size_t len;
    FILE *f = open_memstream(&buf, &len);
    for (int i = 0; i < mal_list_len(list); i++) {
        mal_obj_fprint(f, *mal_list_get(list, i), true);
        if (i != mal_list_len(list) - 1) {
            fputc(' ', f);
        }
    }
    fclose(f);
    return mal_obj_string(buf, len);
}

mal_obj_t *builtin_str(mal_list_t *list) {
    char *buf;
    size_t len;
    FILE *f = open_memstream(&buf, &len);
    for (int i = 0; i < mal_list_len(list); i++)
        mal_obj_fprint(f, *mal_list_get(list, i), false);
    fclose(f);
    return mal_obj_string(buf, len);
}

mal_obj_t *builtin_list(mal_list_t *list) {
    int len = mal_list_len(list);
    mal_obj_t *new_obj = mal_obj_list((3 * len) / 2);
    mal_list_t *new_list = new_obj->data.list;
    for (int i = 0; i < len; i++) {
        mal_obj_t *arg = *mal_list_get(list, i);
        mal_obj_retain(arg);
        mal_list_push_back(new_list, arg);
    }
    return new_obj;
}

mal_obj_t *builtin_list_question_mark(mal_list_t *list) {
    int num_args = mal_list_len(list);
    MAL_OBJ_ASSERT(num_args == 1, "Error: 'list?' only accepts 1 argument. Got %d", num_args);

    mal_obj_t *first = *mal_list_get(list, 0);

    if (first->type == MAL_LIST) {
        return mal_obj_boolean(true);
    }
    else {
        return mal_obj_boolean(false);
    }
}

mal_obj_t *builtin_empty_question_mark(mal_list_t *list) {
    int num_args = mal_list_len(list);
    MAL_OBJ_ASSERT(num_args == 1, "Error: 'empty?' only accepts 1 argument. Got %d", num_args);

    mal_obj_t *first = *mal_list_get(list, 0);

    if (first->type == MAL_LIST) {
        if (mal_list_is_empty(first->data.list)) {
            return mal_obj_boolean(true);
        }
        else {
            return mal_obj_boolean(false);
        }
    }
    else {
        return mal_obj_error_format("Error: '%s' parameter of 'empty?' is not a list",
                                    mal_obj_sprint(first));
    }
}

mal_obj_t *builtin_count(mal_list_t *list) {
    int num_args = mal_list_len(list);
    MAL_OBJ_ASSERT(num_args == 1, "Error: 'count' only accepts 1 argument. Got %d", num_args);

    mal_obj_t *first = *mal_list_get(list, 0);

    if (first->type == MAL_LIST) {
        return mal_obj_num(mal_list_len(first->data.list));
    }
    else {
        return mal_obj_num(0);
    }
}

mal_obj_t *builtin_equal(mal_list_t *list) {
    int num_args = mal_list_len(list);
    MAL_OBJ_ASSERT(num_args == 2, "Error: '=' only accepts 2 argument. Got %d", num_args);

    mal_obj_t *first = *mal_list_get(list, 0);
    mal_obj_t *second = *mal_list_get(list, 1);

    return mal_obj_boolean(mal_obj_equals(first, second));
}

mal_obj_t *builtin_less(mal_list_t *list) {
    int num_args = mal_list_len(list);
    MAL_OBJ_ASSERT(num_args == 2, "Error: '<' only accepts 2 argument. Got %d", num_args);

    mal_obj_t *fst = *mal_list_get(list, 0);
    mal_obj_t *snd = *mal_list_get(list, 1);

    MAL_OBJ_ASSERT(fst->type == (snd->type == MAL_NUMBER), "Error: '<' can only compare numbers");

    return mal_obj_boolean((fst->data.number) < (snd->data.number));
}

mal_obj_t *builtin_less_or_equal(mal_list_t *list) {
    int num_args = mal_list_len(list);
    MAL_OBJ_ASSERT(num_args == 2, "Error: '<=' only accepts 2 argument. Got %d", num_args);

    mal_obj_t *fst = *mal_list_get(list, 0);
    mal_obj_t *snd = *mal_list_get(list, 1);

    MAL_OBJ_ASSERT(fst->type == (snd->type == MAL_NUMBER), "Error: '<=' can only compare numbers");

    return mal_obj_boolean((fst->data.number) <= (snd->data.number));
}

mal_obj_t *builtin_greater(mal_list_t *list) {
    int num_args = mal_list_len(list);
    MAL_OBJ_ASSERT(num_args == 2, "Error: '>' only accepts 2 argument. Got %d", num_args);

    mal_obj_t *fst = *mal_list_get(list, 0);
    mal_obj_t *snd = *mal_list_get(list, 1);

    MAL_OBJ_ASSERT(fst->type == (snd->type == MAL_NUMBER), "Error: '>' can only compare numbers");

    return mal_obj_boolean((fst->data.number) > (snd->data.number));
}

mal_obj_t *builtin_greater_or_equal(mal_list_t *list) {
    int num_args = mal_list_len(list);
    MAL_OBJ_ASSERT(num_args == 2, "Error: '>=' only accepts 2 argument. Got %d", num_args);

    mal_obj_t *fst = *mal_list_get(list, 0);
    mal_obj_t *snd = *mal_list_get(list, 1);

    MAL_OBJ_ASSERT(fst->type == (snd->type == MAL_NUMBER), "Error: '>=' can only compare numbers");

    return mal_obj_boolean((fst->data.number) >= (snd->data.number));
}

mal_obj_t *builtin_read_string(mal_list_t *list) {
    MAL_OBJ_ASSERT(mal_list_len(list) == 1,
                   "Error: 'read-string' expects 1 argument. Got %d", mal_list_len(list));

    mal_obj_t *arg = *mal_list_get(list, 0);
    MAL_OBJ_ASSERT(arg->type == MAL_STRING,
                   "Error: 'read-string' expects a string argument");

    mal_reader_t reader;
    mal_reader_regex_init(&reader);
    mal_obj_t *result = read_str(&reader, arg->data.string.str);
    mal_reader_regex_free(&reader);
    return result;
}

mal_obj_t *builtin_slurp(mal_list_t *list) {
    MAL_OBJ_ASSERT(mal_list_len(list) == 1,
                   "Error: 'slurp' expects 1 argument. Got %d", mal_list_len(list));

    mal_obj_t *arg = *mal_list_get(list, 0);
    MAL_OBJ_ASSERT(arg->type == MAL_STRING,
                   "Error: 'slurp' expects a string argument");

    FILE *f = fopen(arg->data.string.str, "rb");
    if (!f) {
        return mal_obj_error_format("Error: Could not open file '%s'", arg->data.string.str);
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    if (size < 0) {
        fclose(f);
        return mal_obj_error_format("Error: Could not determine size of file '%s'", arg->data.string.str);
    }
    rewind(f);

    char *buf = malloc(size + 1);
    if (!buf) {
        fclose(f);
        return mal_obj_error_format("Error: Out of memory reading file '%s'", arg->data.string.str);
    }

    size_t bytes_read = fread(buf, 1, size, f);
    if (bytes_read != (size_t) size) {
        fclose(f);
        free(buf);
        return mal_obj_error_format("Error: Error reading file '%s'", arg->data.string.str);
    }
    buf[size] = '\0';
    fclose(f);

    return mal_obj_string(buf, size);
}

core_ns_t core_ns[] = {
    { "+", builtin_add },
    { "-", builtin_sub },
    { "*", builtin_mul },
    { "/", builtin_div },
    { "prn", builtin_prn },
    { "pr-str", builtin_pr_str },
    { "str", builtin_str },
    { "println", builtin_println },
    { "list", builtin_list },
    { "list?", builtin_list_question_mark },
    { "empty?", builtin_empty_question_mark },
    { "count", builtin_count },
    { "=", builtin_equal },
    { "<", builtin_less },
    { "<=", builtin_less_or_equal },
    { ">", builtin_greater },
    { ">=", builtin_greater_or_equal },
    { "read-string", builtin_read_string },
    { "slurp", builtin_slurp },
};

const int core_ns_len = sizeof(core_ns) / sizeof(core_ns[0]);
