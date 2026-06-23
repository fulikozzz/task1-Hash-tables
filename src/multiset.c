#include "multiset.h"
#include <stdlib.h>
#include <string.h>

typedef struct _multiset_t
{
	hash_table_t*       table; // хэш-таблица хранит элементы и кол-во въождений
	size_t              total; // общее количество элементов в мультисете (с учётом повторений)
} multiset_t;

multiset_t* multiset_create(size_t capacity)
{
    multiset_t* ms = malloc(sizeof(multiset_t));
    if (ms == NULL) 
        return NULL;

    // Создаем хеш-таблицу для хранения элементов множества
    ms->table = ht_create(capacity);

    if (ms->table == NULL) 
    { 
        free(ms); 
        return NULL; 
    }

    ms->total = 0;
    return ms;
}

void multiset_destroy(multiset_t* ms)
{
    if (ms == NULL) return;

    ht_destroy(ms->table);
    free(ms);
}

bool multiset_add(multiset_t* ms, const char* value)
{
    if (ms == NULL || value == NULL) return false;
	
    // Проверяем, есть ли такой элемент
    size_t* count = ht_find(ms->table, value);
	
    // Если элемент уже есть, увеличиваем его счетчик
    if (count) {
        (*count)++;
        ms->total++;
        return true;
    }
	
    // Если элемента нет, создаем новый счетчик
    size_t* new_count = malloc(sizeof(size_t));
    if (new_count == NULL) return false;
    *new_count = 1;
	
    // Вставляем новый элемент в хэш-таблицу
    if (!ht_insert(ms->table, value, new_count)) 
    { 
        free(new_count); 
        return false; 
    }
    ms->total++;

    return true;
}

bool multiset_remove(multiset_t* ms, const char* value)
{
    if (ms == NULL || value == NULL) return false;

    // Проверяем, есть ли такой элемент
    size_t* count = ht_find(ms->table, value);
    if (!count) return false;

	// Если элеменов много, уменьшаем его счетчик
    if (*count > 1) 
    { 
        (*count)--; 
        ms->total--; 
        return true; 
    }

    // Удаляем элемент из хэш-таблицы, если он единственый
    if (!ht_remove(ms->table, value)) 
        return false;
    
    ms->total--;
    
    return true;
}

size_t multiset_count(const multiset_t* ms, const char* value)
{
    if (ms == NULL || value == NULL) return 0;

    size_t* count = ht_find((hash_table_t*)ms->table, value);

    return count ? *count : 0;
}

size_t multiset_size(const multiset_t* ms)
{
    if (!ms) return 0;
    return ms->total;
}

// Объединение мультимножеств
multiset_t* multiset_union(const multiset_t* first, const multiset_t* second)
{
    if (first == NULL || second == NULL) return NULL;
    multiset_t* result = multiset_create(ht_capacity(first->table) + ht_capacity(second->table));
    
    if (result == NULL) return NULL;

    size_t count = 0;
	// Получаем все ключи из первой хэш-таблицы
    char** keys = ht_keys(first->table, &count);
	// Добавляем все элементы из первой таблицы в результат
    for (size_t i = 0; i < count; i++) {
        size_t count_first = multiset_count(first, keys[i]);
        size_t count_second = multiset_count(second, keys[i]);
		// Добавляем максимальное количество вхождений элемента 
		size_t m = count_first > count_second ? count_first : count_second;
        for (size_t j = 0; j < m; j++) multiset_add(result, keys[i]);
    }
	// Освобождаем память, выделенную для ключей
    free(keys);

    // Получаем все ключи из второй таблицы
    keys = ht_keys(second->table, &count);
    for (size_t i = 0; i < count; i++) {
        if (multiset_count(first, keys[i]) == 0) {
            size_t cb = multiset_count(second, keys[i]);
            for (size_t j = 0; j < cb; j++) multiset_add(result, keys[i]);
        }
    }
    free(keys);

    return result;
}

// Пересечение мультимножеств
multiset_t* multiset_intersection(const multiset_t* first, const multiset_t* second)
{
    if (first == NULL || second == NULL) return NULL;
    multiset_t* result = multiset_create(ht_capacity(first->table));
    
    if (result == NULL) return NULL;

    size_t count = 0;
    // Получаем все ключи из первой хэш-таблицы
    char** keys = ht_keys(first->table, &count);
	// Добавляем элементы, которые есть в обеих таблицах, в результат
    for (size_t i = 0; i < count; i++) {
        size_t ca = multiset_count(first, keys[i]);
        size_t cb = multiset_count(second, keys[i]);
		// Добавляем минимальное количество вхождений элемента 
        size_t m = ca < cb ? ca : cb;
        for (size_t j = 0; j < m; j++) multiset_add(result, keys[i]);
    }
    free(keys);

    return result;
}

// Сумма мультимножеств
multiset_t* multiset_sum(const multiset_t* first, const multiset_t* second)
{
    if (first == NULL || second == NULL) return NULL;
    multiset_t* result = multiset_create(ht_capacity(first->table) + ht_capacity(second->table));

    if (result == NULL) return NULL;

    size_t count = 0;
    // Получаем все ключи из первой хэш-таблицы
    char** keys = ht_keys(first->table, &count);
	// Добавляем элементы, суммируя вхождения
    for (size_t i = 0; i < count; i++) {
        size_t ca = multiset_count(first, keys[i]);
        for (size_t j = 0; j < ca; j++) multiset_add(result, keys[i]);
    }
    free(keys);

    keys = ht_keys(second->table, &count);
    for (size_t i = 0; i < count; i++) {
        size_t cb = multiset_count(second, keys[i]);
        for (size_t j = 0; j < cb; j++) multiset_add(result, keys[i]);
    }
    free(keys);

    return result;
}

void multiset_print_buckets(const multiset_t* ht)
{
    if (ht == NULL) return;
    ht_print_buckets(ht->table);
}
