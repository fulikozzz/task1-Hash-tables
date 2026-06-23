#include "set.h"
#include "hash_table.h"
#include <string.h>

// Структура множества
typedef struct _set_t
{
    hash_table_t*       table;
} set_t;

set_t* set_create(size_t capacity)
{
    set_t* set = malloc(sizeof(set_t));
    
    if (set == NULL) 
        return NULL;
    
	// Создаем хеш-таблицу для хранения элементов множества
    set->table = ht_create(capacity);
    
    if (set->table == NULL) 
    { 
        free(set); 
        return NULL; 
    }
    
    return set;
}

void set_destroy(set_t* set)
{
    if (set == NULL) return;
    
    ht_destroy(set->table);
    free(set);
}

bool set_add(set_t* set, const char* value)
{
    if (set == NULL || value == NULL) return false;
	if (set_contains(set, value)) return false; // Проверяем, есть ли уже элемент в множестве
    if (ht_insert(set->table, value, _strdup(value)))
        return true;
    return false;
}

bool set_remove(set_t* set, const char* value)
{
    if (set == NULL || value == NULL) return false;
    return ht_remove(set->table, value);
}

bool set_contains(const set_t* set, const char* value)
{
    if (set == NULL || value == NULL) return false;
    return ht_find(set->table, value) != NULL;
}

size_t set_size(const set_t* set)
{
    if (set == NULL) return 0;
    return ht_size(set->table);
}

// Объединение множеств
set_t* set_union(const set_t* first, const set_t* second)
{
    if (first == NULL || second == NULL) return NULL;
    set_t* result = set_create(set_size(first) + set_size(second));
    
    if (result == NULL) return NULL;

    size_t count = 0;
	// Получаем все ключи из первой таблицы
    char** keys = ht_keys(first->table, &count);
    for (size_t i = 0; i < count; i++) 
        set_add(result, keys[i]);
	// Освобождаем память, выделенную для ключей
    free(keys);

	// Получаем все ключи из второй таблицы
    keys = ht_keys(second->table, &count);
    for (size_t i = 0; i < count; i++) 
        set_add(result, keys[i]);
    free(keys);

    return result;
}

// Пересечение множеств
set_t* set_intersection(const set_t* first, const set_t* second)
{
    if (!first || !second) return NULL;
    set_t* result = set_create(set_size(first));
    if (!result) return NULL;

    size_t count = 0;
	// Получаем все ключи из первой таблицы
    char** keys = ht_keys(first->table, &count);
	// Проверяем, содержатся ли эти ключи во второй таблице
    for (size_t i = 0; i < count; i++) {
        if (set_contains(second, keys[i])) set_add(result, keys[i]);
    }
    free(keys);

    return result;
}

// Разность множеств
set_t* set_difference(const set_t* first, const set_t* second)
{
    if (!first || !second) return NULL;
    set_t* result = set_create(set_size(first));
    if (!result) return NULL;

    size_t count = 0;
	// Получаем все ключи из первой таблицы
    char** keys = ht_keys(first->table, &count);

    for (size_t i = 0; i < count; i++) {
        if (!set_contains(second, keys[i])) set_add(result, keys[i]);
    }
    free(keys);

    return result;
}

void set_print_buckets(const set_t* ht) 
{
	if (ht == NULL) return;
	ht_print_buckets(ht->table);
}