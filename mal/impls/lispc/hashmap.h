#ifndef _HASHMAP_H
#define _HASHMAP_H

#include <stdbool.h>
#include <stdlib.h>

typedef enum { HM_STATE_FREE, HM_STATE_OCCUPIED, HM_STATE_DELETED } _hashmap_slot_state_t;

#define DEFINE_HASHMAP(name, prefix, key_type, val_type)            \
    typedef struct {                                                \
        key_type key;                                               \
        val_type val;                                               \
        _hashmap_slot_state_t state;                                \
    } name##_Slot;                                                  \
    typedef struct {                                                \
        name##_Slot *slots;                                         \
        int size;                                                   \
        int capacity;                                               \
        int threshold;                                              \
    } name;                                                         \
    name *prefix##_init(int initial_capacity);                      \
    void prefix##_free(name *map);                                  \
    bool prefix##_get(name *map, key_type key, val_type *out);      \
    void prefix##_set(name *map, key_type key, val_type val);       \
    void prefix##_remove(name *map, key_type key);

#define IMPLEMENT_HASHMAP(name, prefix, key_type, val_type, hash_func, key_equals) \
    static int prefix##_next_power_of_two(int n) {                      \
        if (n <= 1) return 1;                                           \
        int power = 1;                                                  \
        while (power < n) power *= 2;                                   \
        return power;                                                   \
    }                                                                   \
    static void prefix##_inner_rehash(name *map) {                      \
        int new_capacity = map->capacity * 2;                           \
        name##_Slot *new_slots = calloc(new_capacity, sizeof(name##_Slot)); \
        if (!new_slots) return;                                         \
        int old_capacity = map->capacity;                               \
        name##_Slot *old_slots = map->slots;                            \
        map->slots = new_slots;                                         \
        map->capacity = new_capacity;                                   \
        map->threshold = (int)(new_capacity * 0.7);                     \
        map->size = 0;                                                  \
        for (int i = 0; i < old_capacity; i++) {                        \
            if (old_slots[i].state == HM_STATE_OCCUPIED) {              \
                prefix##_set(map, old_slots[i].key, old_slots[i].val);  \
            }                                                           \
        }                                                               \
        free(old_slots);                                                \
    }                                                                   \
    name *prefix##_init(int initial_capacity) {                         \
        name *map = malloc(sizeof(name));                               \
        if (!map) return NULL;                                          \
        map->capacity = prefix##_next_power_of_two(initial_capacity);   \
        if (map->capacity < 16) map->capacity = 16;                     \
        map->slots = calloc(map->capacity, sizeof(name##_Slot));        \
        if (!map->slots) {                                              \
            free(map);                                                  \
            return NULL;                                                \
        }                                                               \
        map->size = 0;                                                  \
        map->threshold = (int)(map->capacity * 0.7);                    \
        return map;                                                     \
    }                                                                   \
    void prefix##_free(name *map) {                                     \
        free(map->slots);                                               \
        free(map);                                                      \
    }                                                                   \
    bool prefix##_get(name *map, key_type key, val_type *out) {         \
        if (map->size == 0) return false;                               \
        size_t hash_val = hash_func(key);                               \
        int idx = (int)(hash_val & (map->capacity - 1));                \
        for (int i = 0; i < map->capacity; i++) {                       \
            int j = (idx + i) & (map->capacity - 1);                    \
            if (map->slots[j].state == HM_STATE_FREE) break;            \
            if (map->slots[j].state == HM_STATE_OCCUPIED &&             \
                key_equals(map->slots[j].key, key)) {                   \
                if (out) *out = map->slots[j].val;                      \
                return true;                                            \
            }                                                           \
        }                                                               \
        return false;                                                   \
    }                                                                   \
    void prefix##_set(name *map, key_type key, val_type val) {          \
        if (map->size >= map->threshold) {                              \
            prefix##_inner_rehash(map);                                 \
        }                                                               \
        size_t hash_val = hash_func(key);                               \
        int idx = (int)(hash_val & (map->capacity - 1));                \
        int first_deleted = -1;                                         \
        for (int i = 0; i < map->capacity; i++) {                       \
            int j = (idx + i) & (map->capacity - 1);                    \
            if (map->slots[j].state == HM_STATE_OCCUPIED) {             \
                if (key_equals(map->slots[j].key, key)) {               \
                    map->slots[j].val = val;                            \
                    return;                                             \
                }                                                       \
            } else if (map->slots[j].state == HM_STATE_FREE) {          \
                if (first_deleted == -1) {                              \
                    map->slots[j].key = key;                            \
                    map->slots[j].val = val;                            \
                    map->slots[j].state = HM_STATE_OCCUPIED;            \
                    map->size++;                                        \
                } else {                                                \
                    map->slots[first_deleted].key = key;                \
                    map->slots[first_deleted].val = val;                \
                    map->slots[first_deleted].state = HM_STATE_OCCUPIED; \
                    map->size++;                                        \
                }                                                       \
                return;                                                 \
            } else if (first_deleted == -1) {                           \
                first_deleted = j;                                      \
            }                                                           \
        }                                                               \
        if (first_deleted != -1) {                                      \
            map->slots[first_deleted].key = key;                        \
            map->slots[first_deleted].val = val;                        \
            map->slots[first_deleted].state = HM_STATE_OCCUPIED;        \
            map->size++;                                                \
        }                                                               \
    }                                                                   \
    void prefix##_remove(name *map, key_type key) {                     \
        size_t hash_val = hash_func(key);                               \
        int idx = (int)(hash_val & (map->capacity - 1));                \
        for (int i = 0; i < map->capacity; i++) {                       \
            int j = (idx + i) & (map->capacity - 1);                    \
            if (map->slots[j].state == HM_STATE_FREE) return;           \
            if (map->slots[j].state == HM_STATE_OCCUPIED &&             \
                key_equals(map->slots[j].key, key)) {                   \
                map->slots[j].state = HM_STATE_DELETED;                 \
                map->size--;                                            \
                return;                                                 \
            }                                                           \
        }                                                               \
    }

#endif // _HASHMAP_H
