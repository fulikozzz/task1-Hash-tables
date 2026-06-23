#ifndef SET_H
#define SET_H

#include <stdbool.h>
#include "hash_table.h"

typedef struct _set_t set_t;

set_t* set_create(size_t capacity);
void set_destroy(set_t* set);

bool set_add(set_t* set, const char* value);
bool set_remove(set_t* set, const char* value);
bool set_contains(const set_t* set, const char* value);

size_t set_size(const set_t* set);

set_t* set_union(const set_t* a, const set_t* b);
set_t* set_intersection(const set_t* a, const set_t* b);
set_t* set_difference(const set_t* a, const set_t* b);

void set_print_buckets(const set_t* ht);

#endif