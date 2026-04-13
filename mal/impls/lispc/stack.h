#ifndef STACK_H
#define STACK_H

#include <stdlib.h>
#include <stdbool.h>

#define DEFINE_STACK(type, prefix, name)               \
    struct name {                                      \
        type *items;                                   \
        int top;                                       \
        int capacity;                                  \
    };                                                 \
    static void prefix##_inner_resize(name *stack);    \
    name *prefix##_create(int initial_capacity);       \
    bool prefix##_is_empty(name *stack);               \
    bool prefix##_is_full(name *stack);                \
    void prefix##_push(name *stack, type item);        \
    type prefix##_pop(name *stack);                    \
    type prefix##_peek(name *stack);                   \
    void prefix##_free(name *stack);                   \

#define IMPLEMENT_STACK(type, prefix, name)                             \
    static void prefix##_inner_resize(name *stack) {                    \
        int new_capacity = stack->capacity * 2;                         \
        type *new_items = (type *)realloc(stack->items, new_capacity * sizeof(type)); \
        stack->items = new_items;                                       \
        stack->capacity = new_capacity;                                 \
    }                                                                   \
                                                                        \
    name *prefix##_create(int initial_capacity) {                       \
        name *stack = (name *)malloc(sizeof(name));                     \
        stack->items = (type *)malloc(initial_capacity * sizeof(type)); \
        stack->top = -1;                                                \
        stack->capacity = initial_capacity;                             \
        return stack;                                                   \
    }                                                                   \
                                                                        \
    bool prefix##_is_empty(name *stack) {                               \
        return stack->top == -1;                                        \
    }                                                                   \
                                                                        \
    bool prefix##_is_full(name *stack) {                                \
        return stack->top == stack->capacity - 1;                       \
    }                                                                   \
                                                                        \
    void prefix##_push(name *stack, type item) {                        \
        if (prefix##_is_full(stack)) {                                  \
            prefix##_inner_resize(stack);                               \
        }                                                               \
        stack->items[++stack->top] = item;                              \
    }                                                                   \
                                                                        \
    type prefix##_pop(name *stack) {                                    \
        return stack->items[stack->top--];                              \
    }                                                                   \
                                                                        \
    type prefix##_peek(name *stack) {                                   \
        return stack->items[stack->top];                                \
    }                                                                   \
                                                                        \
    void prefix##_free(name *stack) {                                   \
        free(stack->items);                                             \
        free(stack);                                                    \
    }

#endif // STACK_H
