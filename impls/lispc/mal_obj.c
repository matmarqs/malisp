#include "mal_obj.h"
#include "env.h"
#include "types.h"
IMPLEMENT_DEQUE(mal_obj_t *, mal_list, mal_list_t);

#include <stdio.h>

mal_obj_t *mal_obj_symbol(char *token, int token_sz) {
    mal_obj_t *x = malloc(sizeof(mal_obj_t));
    x->type = MAL_SYMBOL;
    x->refcount = 1;
    x->data.symbol.str = token;
    x->data.symbol.len = token_sz;
    return x;
}

mal_obj_t *mal_obj_num(int64_t num) {
    mal_obj_t *x = malloc(sizeof(mal_obj_t));
    x->type = MAL_NUMBER;
    x->refcount = 1;
    x->data.number = num;
    return x;
}

mal_obj_t *mal_obj_error_format(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    // Determine size
    int size = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    // Allocate and format
    char *error_str = malloc(size + 1);
    va_start(args, fmt);
    vsnprintf(error_str, size + 1, fmt, args);
    va_end(args);

    mal_obj_t *x = malloc(sizeof(mal_obj_t));
    x->type = MAL_ERROR;
    x->refcount = 1;
    x->data.error.str = error_str;
    x->data.error.len = size;
    return x;
}

mal_obj_t *mal_obj_list(int list_size) {
    mal_obj_t *x = malloc(sizeof(mal_obj_t));
    x->type = MAL_LIST;
    x->refcount = 1;
    x->data.list = mal_list_create(list_size);
    return x;
}

mal_obj_t *mal_obj_builtin(fun_t fn_ptr) {
    mal_obj_t *x = malloc(sizeof(mal_obj_t));
    x->type = MAL_BUILTIN;
    x->refcount = 1;
    x->data.builtin_fn = fn_ptr;
    return x;
}

mal_obj_t *mal_obj_boolean(bool boolean) {
    mal_obj_t *x = malloc(sizeof(mal_obj_t));
    x->type = MAL_BOOLEAN;
    x->refcount = 1;
    x->data.boolean = boolean;
    return x;
}

mal_obj_t *mal_obj_nil() {
    mal_obj_t *x = malloc(sizeof(mal_obj_t));
    x->type = MAL_NIL;
    x->refcount = 1;
    // No data needed for nil
    return x;
}

mal_obj_t *mal_obj_function(mal_obj_t *params, mal_obj_t *body, mal_env_t *env) {
    mal_closure_t *f = malloc(sizeof(mal_closure_t));
    mal_obj_retain(params);
    mal_obj_retain(body);
    f->params = params;
    f->body = body;
    mal_env_retain(env);
    f->env = env;
    mal_obj_t *x = malloc(sizeof(mal_obj_t));
    x->type = MAL_FUNCTION;
    x->refcount = 1;
    x->data.function = f;
    return x;
}

mal_obj_t *mal_obj_empty() {
    mal_obj_t *x = malloc(sizeof(mal_obj_t));
    x->type = MAL_EMPTY;
    x->refcount = 1;
    return x;
}

mal_obj_t *mal_obj_string(char *token, int token_sz) {
    mal_obj_t *x = malloc(sizeof(mal_obj_t));
    x->type = MAL_STRING;
    x->refcount = 1;
    x->data.symbol.str = token;
    x->data.symbol.len = token_sz;
    return x;
}

void mal_obj_retain(mal_obj_t *obj) {
    if (obj) obj->refcount++;
}

void mal_obj_release(mal_obj_t *x) {
    if (!x) return;
    if (--x->refcount == 0) {
        switch (x->type) {
        case MAL_LIST:
            for (int i = 0; i < mal_list_len(x->data.list); i++) {
                mal_obj_release(*(mal_list_get(x->data.list, i)));
            }
            mal_list_free(x->data.list);
            break;
        case MAL_SYMBOL:
            free(x->data.symbol.str);
            break;
        case MAL_ERROR:
            free(x->data.error.str);
            break;
        case MAL_STRING:
            free(x->data.string.str);
            break;
        case MAL_FUNCTION:
            mal_obj_release(x->data.function->params);
            mal_obj_release(x->data.function->body);
            mal_env_release(x->data.function->env);
            free(x->data.function);
            break;
        default:
            break;
        }
        free(x);
    }
}

void mal_obj_fprint(FILE *f, mal_obj_t *obj, bool print_readably) {
    if (!obj) {
        fprintf(f, "<null>");
        return;
    }
    switch (obj->type) {
    case MAL_SYMBOL:
        fprintf(f, "%.*s", obj->data.symbol.len, obj->data.symbol.str);
        break;
    case MAL_STRING: {
        string_t s = obj->data.string;
        if (!print_readably) {
            fprintf(f, "%.*s", s.len, s.str);
        } else {
            fputc('"', f);
            for (int i = 0; i < s.len; i++) {
                char c = s.str[i];
                if (c == '\n')      fputs("\\n", f);
                else if (c == '"')  fputs("\\\"", f);
                else if (c == '\\') fputs("\\\\", f);
                else                fputc(c, f);
            }
            fputc('"', f);
        }
        break;
    }
    case MAL_NUMBER:
        fprintf(f, "%lld", (long long)obj->data.number);
        break;
    case MAL_LIST:
        fputc('(', f);
        for (int i = 0; i < mal_list_len(obj->data.list); i++) {
            mal_obj_fprint(f, *(mal_list_get(obj->data.list, i)), print_readably);
            if (i != mal_list_len(obj->data.list) - 1) {
                fputc(' ', f);
            }
        }
        fputc(')', f);
        break;
    case MAL_ERROR:
        fprintf(f, "%.*s", obj->data.error.len, obj->data.error.str);
        break;
    case MAL_BUILTIN:
        fprintf(f, "<builtin>");
        break;
    case MAL_FUNCTION:
        fprintf(f, "#<function>");
        break;
    case MAL_BOOLEAN:
        fprintf(f, obj->data.boolean ? "true" : "false");
        break;
    case MAL_NIL:
        fprintf(f, "nil");
        break;
    case MAL_EMPTY:
        break;
    default:
        fprintf(f, "<unknown type %d>", obj->type);
        break;
    }
}

void mal_obj_print(mal_obj_t *obj, bool print_readably) {
    mal_obj_fprint(stdout, obj, print_readably);
}

void mal_obj_println(mal_obj_t *obj, bool print_readably) {
    mal_obj_fprint(stdout, obj, print_readably);
    putchar('\n');
}

char *mal_obj_sprint(mal_obj_t *obj) {
    static char _buffer[1024];
    if (!obj) {
        snprintf(_buffer, sizeof(_buffer), "<null>");
        return _buffer;
    }
    switch (obj->type) {
    case MAL_SYMBOL:
        snprintf(_buffer, sizeof(_buffer), "%.*s",
                 obj->data.symbol.len, obj->data.symbol.str);
        break;
    case MAL_STRING:
        snprintf(_buffer, sizeof(_buffer), "%.*s",
                 obj->data.string.len, obj->data.string.str);
        break;
    case MAL_NUMBER:
        snprintf(_buffer, sizeof(_buffer), "%lld", (long long)obj->data.number);
        break;
    case MAL_LIST:
        snprintf(_buffer, sizeof(_buffer), "<list>");
        break;
    case MAL_ERROR:
        snprintf(_buffer, sizeof(_buffer), "%.*s",
                 obj->data.error.len, obj->data.error.str);
        break;
    case MAL_BUILTIN:
        snprintf(_buffer, sizeof(_buffer), "<builtin>");
        break;
    case MAL_FUNCTION:
        snprintf(_buffer, sizeof(_buffer), "#<function>");
        break;
    case MAL_BOOLEAN:
        snprintf(_buffer, sizeof(_buffer), obj->data.boolean ? "true" : "false");
        break;
    case MAL_NIL:
        snprintf(_buffer, sizeof(_buffer), "nil");
        break;
    default:
        snprintf(_buffer, sizeof(_buffer), "<unknown>");
        break;
    }
    return _buffer;
}

bool mal_list_equals(mal_list_t *fst_list, mal_list_t *snd_list) {
    int fst_len = mal_list_len(fst_list);
    int snd_len = mal_list_len(snd_list);
    if (fst_len != snd_len)
        return false;
    for (int i = 0; i < fst_len; i++) {
        mal_obj_t *fst_i = *mal_list_get(fst_list, i);
        mal_obj_t *snd_i = *mal_list_get(snd_list, i);
        if (!mal_obj_equals(fst_i, snd_i))
            return false;
    }
    return true;
}

bool mal_obj_equals(mal_obj_t *fst, mal_obj_t *snd)
{
    if (fst->type != snd->type) {
        return false;
    }
    switch (fst->type) {
    case MAL_LIST:
        return mal_list_equals(fst->data.list, snd->data.list);
    case MAL_NUMBER:
        return fst->data.number == snd->data.number;
    case MAL_SYMBOL:
        return str_equals(fst->data.symbol, fst->data.symbol);
    case MAL_ERROR:
        return str_equals(fst->data.error, fst->data.error);
    case MAL_BUILTIN:
        return fst->data.builtin_fn == snd->data.builtin_fn;
    case MAL_NIL:
        return true;
    case MAL_BOOLEAN:
        return fst->data.boolean == snd->data.boolean;
    case MAL_FUNCTION:
        // return true if they have exactly the same pointers
        // we could compare the objects themselves, but i don't really see the point
        // the comparison would be expensive
        return fst->data.function->env == snd->data.function->env &&
            fst->data.function->params == snd->data.function->params &&
            fst->data.function->body == snd->data.function->body;
    case MAL_EMPTY:
        return true;
    default:
        break;
    }
    return false;
}
