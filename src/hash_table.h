#ifndef HASH_TABLE_H
#define HASH_TABLE_H
#include <stdbool.h>
#include <stdlib.h>

typedef struct _hash_table_t hash_table_t;

hash_table_t* ht_create(size_t capacity);
void ht_destroy(hash_table_t* ht);

size_t ht_size(const hash_table_t* ht);
size_t ht_capacity(const hash_table_t* ht);
double ht_load_factor(const hash_table_t* ht);
bool ht_rehash(hash_table_t* ht);

bool ht_insert(hash_table_t* ht, const char* key, void* value);
void* ht_find(hash_table_t* ht, const char* key);
bool ht_remove(hash_table_t* ht, const char* key);

void ht_print_buckets(const hash_table_t* ht);
char** ht_keys(const hash_table_t* ht, size_t* count);

#endif