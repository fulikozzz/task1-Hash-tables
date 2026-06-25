#ifndef LF_HASH_TABLE_H
#define LF_HASH_TABLE_H
#include <stdbool.h>
#include <stdlib.h>

typedef struct _lf_hash_table_t lf_hash_table_t;

lf_hash_table_t* lf_ht_create(size_t capacity);
void lf_ht_destroy(lf_hash_table_t* ht);

size_t lf_ht_size(const lf_hash_table_t* ht);
size_t lf_ht_capacity(const lf_hash_table_t* ht);
double lf_ht_load_factor(const lf_hash_table_t* ht);
bool lf_ht_rehash(lf_hash_table_t* ht);

bool lf_ht_insert(lf_hash_table_t* ht, const char* key, void* value);
void* lf_ht_find(lf_hash_table_t* ht, const char* key);
bool lf_ht_remove(lf_hash_table_t* ht, const char* key);

void lf_ht_print_buckets(const lf_hash_table_t* ht);
char** lf_ht_keys(const lf_hash_table_t* ht, size_t* count);

#endif