#include "mal_obj.h"
#include "reader.h"
#include "env.h"
#include "types.h"

#include <stdio.h>
#include <editline.h>
#include <stdlib.h>
#include <stdbool.h>

mal_obj_t *mal_eval(mal_env_t *env, mal_obj_t *node);

char *mal_read() {
    char *input = readline("user> ");
    if (input) {
        add_history(input);
    }
    return input;
}

void mal_print(mal_obj_t *root) {
    if (root && root->type != MAL_EMPTY) {
        mal_obj_print(root);
        putchar('\n');
    }
}

mal_obj_t *mal_eval_noop(mal_obj_t *node) {
    mal_obj_retain(node);
    return node;
}

// assumes that node->type == MAL_SYMBOL
mal_obj_t *mal_eval_symbol(mal_env_t *env, mal_obj_t *node) {
    mal_obj_t *value = NULL;
    MAL_OBJ_ASSERT(mal_env_get(env, node->data.symbol, &value),
                   "Error: Symbol '%s' not found", mal_obj_sprint(node));
    return value;
}

mal_obj_t *mal_handle_apply(mal_env_t *env, mal_obj_t *list_obj) {
    mal_list_t *list = list_obj->data.list;
    int list_len = mal_list_len(list);
    mal_obj_t **zeroth_ptr = mal_list_get(list, 0);
    mal_obj_t *zeroth = zeroth_ptr ? *zeroth_ptr : NULL;

    mal_obj_t *func = mal_eval(env, zeroth);
    if (func->type == MAL_ERROR) return func;
    MAL_OBJ_ASSERT(func->type == MAL_BUILTIN || func->type == MAL_FUNCTION,
                   "Error: Parameter '%s' cannot be used as a function to evaluate", mal_obj_sprint(func));

    mal_list_t *eval_args = mal_list_create(16);
    for (int i = 1; i < list_len; i++) {
        mal_obj_t **arg_ptr = mal_list_get(list, i);
        mal_obj_t *arg = mal_eval(env, *arg_ptr);
        if (arg->type == MAL_ERROR) {
            for (int j = 0; j < mal_list_len(eval_args); j++) {
                mal_obj_release(*(mal_list_get(eval_args, j)));
            }
            mal_list_free(eval_args);
            return arg;
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

        if (mal_env_bind(call_env, f->params->data.list, eval_args, &err_feedback)) {
            final_result = mal_eval(call_env, f->body);
        } else {
            final_result = err_feedback;
        }
        mal_env_free(call_env);
    }

    for (int i = 0; i < mal_list_len(eval_args); i++) {
        mal_obj_release(*(mal_list_get(eval_args, i)));
    }
    mal_list_free(eval_args);
    mal_obj_release(func);

    return final_result;
}

// assumes 0th element of MAL_LIST is the symbol "def!"
// we expect (def! symbol_name (expr arg1 arg2))
mal_obj_t *mal_handle_def(mal_env_t *env, mal_obj_t *list_obj) {
    mal_list_t *list = list_obj->data.list;
    MAL_OBJ_ASSERT(mal_list_len(list) == 3,
                   "Error: def! expects 2 arguments. Got %d", mal_list_len(list)-1);

    mal_obj_t **first_ptr = mal_list_get(list, 1);
    mal_obj_t *first = *first_ptr;
    MAL_OBJ_ASSERT(first->type == MAL_SYMBOL,
                   "First element '%s' of def! is not a symbol", mal_obj_sprint(first));

    mal_obj_t **second_ptr = mal_list_get(list, 2);
    mal_obj_t *second = *second_ptr;
    mal_obj_t *result = mal_eval(env, second);

    if (result->type != MAL_ERROR) {
        string_t key = first->data.symbol;
        mal_env_set(env, key, result);
    }
    return result;
}

mal_obj_t *mal_handle_let(mal_env_t *env, mal_obj_t *list_obj) {
    mal_list_t *list = list_obj->data.list;
    MAL_OBJ_ASSERT(mal_list_len(list) == 3,
                   "Error: let* expects 2 arguments. Got %d", mal_list_len(list));

    mal_obj_t **first_ptr = mal_list_get(list, 1);
    mal_obj_t *first = *first_ptr;
    MAL_OBJ_ASSERT(first->type == MAL_LIST && mal_list_len(first->data.list) % 2 == 0,
                   "Error: The first argument of let* must be like "
                   "(key1 expr1 key2 expr2 ...)");

    mal_list_t *bindings = first->data.list;
    mal_env_t *new_env = mal_env_create(env);

    for (int i = 0; i < mal_list_len(bindings); i += 2) {
        mal_obj_t **symbol_ptr = mal_list_get(bindings, i);
        mal_obj_t *symbol = *symbol_ptr;

        if (!(symbol->type == MAL_SYMBOL)) {
            mal_env_free(new_env);
            return mal_obj_error_format("Error: Cannot bind non-symbol '%s' in let* expression",
                                        mal_obj_sprint(symbol));
        }

        mal_obj_t **value_ptr = mal_list_get(bindings, i+1);
        mal_obj_t *value = mal_eval(new_env, *value_ptr);
        if (value->type == MAL_ERROR) {
            mal_env_free(new_env);
            return value;
        }

        mal_env_set(new_env, symbol->data.symbol, value);
        mal_obj_release(value); // because mal_env_set retains it
    }

    mal_obj_t **result_ptr = mal_list_get(list, 2);
    mal_obj_t *result = mal_eval(new_env, *result_ptr);
    mal_env_free(new_env);
    return result;
}

mal_obj_t *mal_handle_fn(mal_env_t *env, mal_obj_t *list_obj) {
    mal_list_t *list = list_obj->data.list;
    MAL_OBJ_ASSERT(mal_list_len(list) == 3,
                   "Error: fn* expects 2 arguments. Got %d", mal_list_len(list));

    mal_obj_t **binds_ptr = mal_list_get(list, 1);
    mal_obj_t **body_ptr = mal_list_get(list, 2);
    mal_obj_t *binds = *binds_ptr;
    mal_obj_t *body = *body_ptr;

    // transfer ownership to the MAL_FUNCTION, popping 'binds' and 'body' from the original AST
    mal_list_pop_back(list, NULL);
    mal_list_pop_back(list, NULL);

    return mal_obj_function(binds, body, env);
}

// assumes node->type == MAL_LIST
mal_obj_t *mal_eval_list(mal_env_t *env, mal_obj_t *node) {
    mal_list_t *list = node->data.list;
    if (mal_list_is_empty(list))
        return mal_obj_list(0);

    mal_obj_t **first_ptr = mal_list_get(list, 0);
    mal_obj_t *first = *first_ptr;
    if (first->type == MAL_SYMBOL) {
        char *symbol = first->data.symbol.str;
        if (strncmp(symbol, "def!", 4) == 0) {
            return mal_handle_def(env, node);
        }
        if (strncmp(symbol, "let*", 4) == 0) {
            return mal_handle_let(env, node);
        }
        if (strncmp(symbol, "fn*", 3) == 0) {
            return mal_handle_fn(env, node);
        }
    }

    /* standard apply case here */
    return mal_handle_apply(env, node);
}

mal_obj_t *mal_eval(mal_env_t *env, mal_obj_t *node) {
    if (!node) return mal_obj_empty();
    switch (node->type) {
    case MAL_SYMBOL:
        return mal_eval_symbol(env, node);
    case MAL_LIST:
        return mal_eval_list(env, node);
    default:
        break;
    }
    return mal_eval_noop(node);
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

    mal_obj_t *root = read_str(reader, input);
    mal_obj_t *result = mal_eval(env, root);

    free(input);
    mal_print(result);
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

    mal_env_free(env);
    mal_reader_regex_free(&reader);
    return 0;
}
