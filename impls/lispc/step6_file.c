#include "mal_obj.h"
#include "reader.h"
#include "env.h"
#include "types.h"

#include <stdio.h>
#include <editline.h>
#include <stdlib.h>
#include <stdbool.h>

char *mal_read() {
    char *input = readline("user> ");
    if (input) {
        add_history(input);
    }
    return input;
}

#define EVAL_RETURN(expr)                       \
    do {                                        \
        mal_obj_t *_r = (expr);                 \
        mal_env_release(env);                   \
        return _r;                              \
    } while(0)

#define MAL_EVAL_ASSERT(cond, err_fmt, ...)                         \
    if (!(cond)) {                                                  \
        EVAL_RETURN(mal_obj_error_format(err_fmt, ##__VA_ARGS__));  \
    }

mal_obj_t *mal_eval(mal_env_t *env, mal_obj_t *node) {
    mal_env_retain(env);

    while (true) {
        if (!node) EVAL_RETURN(mal_obj_empty());
        switch (node->type) {
        case MAL_SYMBOL:
        {
            mal_obj_t *value = NULL;
            MAL_EVAL_ASSERT(mal_env_get(env, node->data.symbol, &value),
                           "Error: Symbol '%s' not found", mal_obj_sprint(node));
            mal_obj_retain(value);
            EVAL_RETURN(value);
        }
        case MAL_LIST:
        {
            mal_list_t *list = node->data.list;
            int listlen = mal_list_len(list);
            if (listlen == 0) {
                EVAL_RETURN(mal_obj_list(0));
            }

            mal_obj_t *first = *mal_list_get(list, 0);
            if (first->type == MAL_SYMBOL) {
                char *symbol = first->data.symbol.str;
                int symblen = first->data.symbol.len;
                if (symblen == 2 && strncmp(symbol, "if", 2) == 0)
                {
                    int listlen = mal_list_len(list);
                    MAL_OBJ_ASSERT(3 <= listlen && listlen <= 4,
                                   "Error: 'if' expects 2 or 3 parameters. Got %d", listlen-1);

                    mal_obj_t *first = mal_eval(env, *mal_list_get(list, 1));
                    bool cond = !((first->type == MAL_BOOLEAN && !first->data.boolean) ||
                                  first->type == MAL_NIL);

                    if (cond) {
                        node = *mal_list_get(list, 2);
                        mal_obj_release(first);
                        continue;
                    }
                    else if (listlen < 4) { // there is no third parameter
                        mal_obj_release(first);
                        EVAL_RETURN(mal_obj_nil());
                    }
                    else {
                        node = *mal_list_get(list, 3);
                        mal_obj_release(first);
                        continue;
                    }
                }
                if (symblen == 4 && strncmp(symbol, "def!", 4) == 0)
                {

                    MAL_OBJ_ASSERT(listlen == 3,
                                   "Error: def! expects 2 arguments. Got %d", mal_list_len(list)-1);

                    mal_obj_t *first = *mal_list_get(list, 1);
                    MAL_OBJ_ASSERT(first->type == MAL_SYMBOL,
                                   "First element '%s' of def! is not a symbol", mal_obj_sprint(first));

                    mal_obj_t *second = *mal_list_get(list, 2);
                    mal_obj_t *result = mal_eval(env, second);

                    if (result->type != MAL_ERROR) {
                        string_t key = first->data.symbol;
                        mal_env_set(env, key, result);
                    }
                    EVAL_RETURN(result);
                }
                if (symblen == 3 && strncmp(symbol, "fn*", 3) == 0)
                {
                    MAL_OBJ_ASSERT(mal_list_len(list) == 3,
                                   "Error: fn* expects 2 arguments. Got %d", mal_list_len(list)-1);
                    mal_obj_t *params = *mal_list_get(list, 1);
                    mal_obj_t *body = *mal_list_get(list, 2);
                    EVAL_RETURN(mal_obj_function(params, body, env));
                }
                if (symblen == 2 && strncmp(symbol, "do", 2) == 0)
                {
                    mal_obj_t *evaluated;
                    for (int i = 1; i < listlen-1; i++) {
                        evaluated = mal_eval(env, *mal_list_get(list, i));
                        mal_obj_release(evaluated);
                    }
                    // TCO
                    node = *mal_list_get(list, listlen-1);
                    continue;
                }
                if (symblen == 4 && strncmp(symbol, "let*", 4) == 0)
                {
                    MAL_EVAL_ASSERT(mal_list_len(list) == 3,
                                    "Error: let* expects 2 arguments. Got %d", mal_list_len(list)-1);

                    mal_obj_t *first = *mal_list_get(list, 1);
                    MAL_EVAL_ASSERT(first->type == MAL_LIST && mal_list_len(first->data.list) % 2 == 0,
                                    "Error: The first argument of let* must be like "
                                    "(key1 expr1 key2 expr2 ...)");

                    mal_list_t *bindings = first->data.list;
                    mal_env_t *new_env = mal_env_create(env);

                    for (int i = 0; i < mal_list_len(bindings); i += 2) {
                        mal_obj_t *symbol = *mal_list_get(bindings, i);

                        if (!(symbol->type == MAL_SYMBOL)) {
                            mal_env_release(new_env);
                            EVAL_RETURN(mal_obj_error_format(
                                            "Error: Cannot bind non-symbol '%s' in let* expression",
                                            mal_obj_sprint(symbol)));
                        }

                        mal_obj_t *value = mal_eval(new_env, *mal_list_get(bindings, i+1));
                        if (value->type == MAL_ERROR) {
                            mal_env_release(new_env);
                            EVAL_RETURN(value);
                        }

                        mal_env_set(new_env, symbol->data.symbol, value);
                        mal_obj_release(value); // because mal_env_set retains it
                    }

                    // TCO: there will be a memory leak here because Reference Counting can't handle cycles
                    // TODO: implement/use a GC
                    mal_env_release(env);
                    env = new_env;
                    node = *mal_list_get(list, 2);
                    continue;
                }
            }
            // apply/invoke case
            mal_obj_t *zeroth = *mal_list_get(list, 0);

            mal_obj_t *func = mal_eval(env, zeroth);
            if (func->type == MAL_ERROR) {
                EVAL_RETURN(func);
            }
            if (!(func->type == MAL_BUILTIN || func->type == MAL_FUNCTION))
            {
                mal_obj_t *err = mal_obj_error_format(
                    "Error: Parameter '%s' cannot be used as a function to evaluate",
                    mal_obj_sprint(func));
                mal_obj_release(func);

                EVAL_RETURN(err);
            }

            mal_list_t *eval_args = mal_list_create(listlen-1);
            for (int i = 1; i < listlen; i++) {
                mal_obj_t **arg_ptr = mal_list_get(list, i);
                mal_obj_t *arg = mal_eval(env, *arg_ptr);
                if (arg->type == MAL_ERROR) {
                    for (int j = 0; j < mal_list_len(eval_args); j++) {
                        mal_obj_release(*(mal_list_get(eval_args, j)));
                    }
                    mal_list_free(eval_args);
                    mal_obj_release(func);
                    EVAL_RETURN(arg);
                }
                mal_list_push_back(eval_args, arg);
            }

            mal_obj_t *final_result = NULL;

            if (func->type == MAL_BUILTIN) {
                final_result = func->data.builtin_fn(eval_args);
            }
            else if (func->type == MAL_FUNCTION) {
                mal_closure_t *f = func->data.function;
                mal_obj_t *err_feedback = NULL;
                mal_env_t *call_env = mal_env_create(f->env);

                // store func in call_env to keep it alive during TCO
                // the function owns f->body, so func must outlive the body
                mal_env_set(call_env, str_from_cstr("__func__"), func);

                if (mal_env_bind(call_env, f->params->data.list, eval_args, &err_feedback)) {
                    // cleanup eval_args and func
                    for (int i = 0; i < mal_list_len(eval_args); i++) {
                        mal_obj_release(*(mal_list_get(eval_args, i)));
                    }
                    mal_list_free(eval_args);
                    mal_obj_release(func);

                    // TCO
                    mal_env_release(env);
                    env = call_env;
                    node = f->body; // body is still alive
                    continue;
                } else {
                    final_result = err_feedback;
                    mal_env_release(call_env);
                }
            }

            // cleanup eval_args and func
            for (int i = 0; i < mal_list_len(eval_args); i++) {
                mal_obj_release(*(mal_list_get(eval_args, i)));
            }
            mal_list_free(eval_args);
            mal_obj_release(func);

            EVAL_RETURN(final_result);
        }
        break;
        default:
            break;
        }
        // default noop case
        mal_obj_retain(node);
        EVAL_RETURN(node);
    }
}

bool mal_rep(mal_reader_t *reader, mal_env_t *env) {
    char *input = mal_read();
    if (!input) {
        puts("");
        return false;
    }
    // skip empty input (only spaces/newlines)
    if (input[0] == '\0') {
        free(input);
        return true;
    }

    //puts(input);

    mal_obj_t *root = read_str(reader, input);
    mal_obj_t *result = mal_eval(env, root);

    free(input);
    mal_obj_println(result, true);
    mal_obj_release(result);
    mal_obj_release(root);

    return true;
}

int main() {
    mal_reader_t reader;
    mal_reader_regex_init(&reader);
    mal_env_t *env = mal_env_create(NULL);
    mal_env_register_builtins(env);

    bool running = true;
    while (running) {
        running = mal_rep(&reader, env);
    }

    mal_env_release(env);
    mal_reader_regex_free(&reader);
    return 0;
}
