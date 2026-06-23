#ifndef MULTISET_H
#define MULTISET_H

#include <stddef.h>
#include <stdbool.h>
#include "hash_table.h"

typedef struct _multiset_t multiset_t;

multiset_t* multiset_create(size_t capacity);
void multiset_destroy(multiset_t* ms);

bool multiset_add(multiset_t* ms, const char* value);
bool multiset_remove(multiset_t* ms, const char* value); 
size_t multiset_count(const multiset_t* ms, const char* value);
size_t multiset_size(const multiset_t* ms);

multiset_t* multiset_union(const multiset_t* a, const multiset_t* b); 
multiset_t* multiset_intersection(const multiset_t* a, const multiset_t* b); 
multiset_t* multiset_sum(const multiset_t* a, const multiset_t* b); 

void multiset_print_buckets(const multiset_t* ht);

#endif
