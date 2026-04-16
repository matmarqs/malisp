#ifndef DEQUE_H
#define DEQUE_H

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define DEFINE_DEQUE(type, prefix, name)                    \
    struct name {                                           \
        type *items;                                        \
        int start;          /* Index of first element */    \
        int count;          /* Number of elements */        \
        int cap;            /* Capacity */                  \
    };                                                      \
    name *prefix##_create(int initial_cap);                 \
    bool prefix##_is_empty(name *deque);                    \
    void prefix##_push_front(name *deque, type item);       \
    void prefix##_push_back(name *deque, type item);        \
    bool prefix##_pop_front(name *deque, type *out);        \
    bool prefix##_pop_back(name *deque, type *out);         \
    bool prefix##_peek_front(name *deque, type *out);       \
    bool prefix##_peek_back(name *deque, type *out);        \
    void prefix##_free(name *deque);                        \
    int prefix##_len(name *deque);                          \
    type *prefix##_get(name *deque, int index);             \
    bool prefix##_set(name *deque, int index, type value);

#define IMPLEMENT_DEQUE(type, prefix, name)                             \
    static void prefix##_inner_resize(name *deque, int new_cap) {       \
        type *new_items = (type *)malloc(new_cap * sizeof(type));       \
        for (int i = 0; i < deque->count; i++) {                        \
            int src = (deque->start + i) % deque->cap;                  \
            new_items[i] = deque->items[src];                           \
        }                                                               \
        free(deque->items);                                             \
        deque->items = new_items;                                       \
        deque->start = 0;                                               \
        deque->cap = new_cap;                                           \
    }                                                                   \
    name *prefix##_create(int initial_cap) {                            \
        if (initial_cap < 4) initial_cap = 4;                           \
        name *deque = (name *)malloc(sizeof(name));                     \
        deque->items = (type *)malloc(initial_cap * sizeof(type));      \
        deque->start = 0;                                               \
        deque->count = 0;                                               \
        deque->cap = initial_cap;                                       \
        return deque;                                                   \
    }                                                                   \
    bool prefix##_is_empty(name *deque) {                               \
        return deque->count == 0;                                       \
    }                                                                   \
    void prefix##_push_front(name *deque, type item) {                  \
        if (deque->count == deque->cap) {                               \
            prefix##_inner_resize(deque, deque->cap * 2);               \
        }                                                               \
        deque->start = (deque->start - 1 + deque->cap) % deque->cap;    \
        deque->items[deque->start] = item;                              \
        deque->count++;                                                 \
    }                                                                   \
    void prefix##_push_back(name *deque, type item) {                   \
        if (deque->count == deque->cap) {                               \
            prefix##_inner_resize(deque, deque->cap * 2);               \
        }                                                               \
        int idx = (deque->start + deque->count) % deque->cap;           \
        deque->items[idx] = item;                                       \
        deque->count++;                                                 \
    }                                                                   \
    bool prefix##_pop_front(name *deque, type *out) {                   \
        if (deque->count <= 0)                                          \
            return false;                                               \
        if (out) {                                                      \
            *out = deque->items[deque->start];                          \
        }                                                               \
        deque->start = (deque->start + 1) % deque->cap;                 \
        deque->count--;                                                 \
        return true;                                                    \
    }                                                                   \
    bool prefix##_pop_back(name *deque, type *out) {                    \
        if (deque->count <= 0)                                          \
            return false;                                               \
        if (out) {                                                      \
            int idx = (deque->start + deque->count - 1) % deque->cap;   \
            *out = deque->items[idx];                                   \
        }                                                               \
        deque->count--;                                                 \
        return true;                                                    \
    }                                                                   \
    bool prefix##_peek_front(name *deque, type *out) {                  \
        if (out && deque->count > 0) {                                  \
            *out = deque->items[deque->start];                          \
            return true;                                                \
        }                                                               \
        return false;                                                   \
    }                                                                   \
    bool prefix##_peek_back(name *deque, type *out) {                   \
        if (out && deque->count > 0) {                                  \
            int idx = (deque->start + deque->count - 1) % deque->cap;   \
            *out = deque->items[idx];                                   \
            return true;                                                \
        }                                                               \
        return false;                                                   \
    }                                                                   \
    void prefix##_free(name *deque) {                                   \
        free(deque->items);                                             \
        free(deque);                                                    \
    }                                                                   \
    int prefix##_len(name *deque) {                                     \
        return deque->count;                                            \
    }                                                                   \
    type *prefix##_get(name *deque, int index) {                        \
        if (index < 0 || index >= deque->count) return NULL;            \
        int phys = (deque->start + index) % deque->cap;                 \
        return &deque->items[phys];                                     \
    }                                                                   \
    bool prefix##_set(name *deque, int index, type value) {             \
        if (index < 0 || index >= deque->count) return false;           \
        int phys = (deque->start + index) % deque->cap;                 \
        deque->items[phys] = value;                                     \
        return true;                                                    \
    }

#endif // DEQUE_H
