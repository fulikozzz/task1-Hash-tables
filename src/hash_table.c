#include "hash_table.h"
#include <string.h>
#include <stdio.h>

#define FNV1A64_OFFSET_BASIS  14695981039346656037
#define FNV1A64_PRIME         1099511628211

#define MAX_LOAD_FACTOR 0.75

// Структура узла
typedef struct _node
{
    char*           key;        // ключ (строка)
    void*           value;      // void* для хранения значения любого типа
    struct _node*   next;       // указатель на следующий узел в цепочке
} node;

// Структура хэш-таблицы
struct _hash_table_t
{
    node**          buckets;    // массив указателей на узлы
	size_t          size;       // текущее количество элементов в таблице
	size_t          capacity;   // текущая вместимость таблицы
};

hash_table_t* ht_create(size_t capacity) 
{
    if (capacity == 0) 
        capacity = 1; 

    hash_table_t* ht = malloc(sizeof(hash_table_t));
    
    if (ht == NULL) 
        return NULL;

    ht->buckets = calloc(capacity, sizeof(node*));

    if (ht->buckets == NULL)
    {
        free(ht);
        return NULL;
    }

    ht->size = 0;
    ht->capacity = capacity;

    return ht;
}

void ht_destroy(hash_table_t* ht)
{
	if (ht == NULL) return;
	// Последовательно освобождаем память для каждого узла в каждой корзине
    for (size_t i = 0; i < ht->capacity; i++)
    {
        node* current = ht->buckets[i];
        while (current != NULL) {
            node* next = current->next;
            free(current->value);
            free(current->key);
            free(current);
            current = next;
        }
    }
    free(ht->buckets);
    free(ht);
}

// Функция хэширования по алгоритму FNV-1a 64-бит
static size_t fnv1a_hash(const char* key, size_t capacity)
{
    size_t index = FNV1A64_OFFSET_BASIS; 
    while (*key) {
        index = index ^ *key;
        key++;
        index = index * FNV1A64_PRIME;
    }
    return index % capacity;
}

size_t ht_size(const hash_table_t* ht)
{
    return ht->size;
}

size_t ht_capacity(const hash_table_t* ht)
{
    return ht->capacity;
}

double ht_load_factor(const hash_table_t* ht)
{
    return (double)ht->size / ht->capacity;
}

bool ht_rehash(hash_table_t* ht)
{
	size_t new_capacity = ht->capacity * 2; // Увеличиваем вместимость в 2 раза

	node** new_buckets = calloc(new_capacity, sizeof(node*)); // Выделяем память для нового массива
    
    if (new_buckets == NULL)
        return false;

	// Переносим все элементы из старой таблицы в новую
    for (size_t i = 0; i < ht->capacity; i++)
    {
        node* current = ht->buckets[i];

        while (current != NULL)
        {
            node* next = current->next;

            size_t new_index = fnv1a_hash(current->key, new_capacity);

            current->next = new_buckets[new_index];
            new_buckets[new_index] = current;

            current = next;
        }
    }

    free(ht->buckets);

    ht->buckets = new_buckets;
    ht->capacity = new_capacity;

    return true;
}

bool ht_insert(hash_table_t* ht, const char* key, void* value)
{
    if (ht == NULL || key == NULL) return false;

	// Выполняем рехеширование при необходимости
	if (ht_load_factor(ht) > MAX_LOAD_FACTOR)
	{
		if (!ht_rehash(ht))
			return false;
	}

	// Вычисляем индекс бакета для данного ключа
    size_t index = fnv1a_hash(key, ht->capacity);
    node* to_insert = malloc(sizeof(node));
 
    if (to_insert == NULL) return false;
    
    to_insert->key = _strdup(key);
	to_insert->value = value;
    
    if (to_insert->key == NULL || to_insert->value == NULL)
    {
        free(to_insert);
        return false;
    }
    
    to_insert->next = ht->buckets[index];
    ht->buckets[index] = to_insert;
    ht->size++;
    
    return true;
}

void* ht_find(hash_table_t* ht, const char* key)
{
    if (ht == NULL || key == NULL) return NULL;

    size_t index = fnv1a_hash(key, ht->capacity);
	// Проходим по цепочке в соответствующем бакете
    node* current = ht->buckets[index];
        while (current != NULL) {
            node* next = current->next;
            if (!strcmp(current->key, key))
                return current->value;
        current = next;
        }
    return NULL;
}

bool ht_remove(hash_table_t* ht, const char* key)
{
    if (ht == NULL || key == NULL) return false;

    size_t index = fnv1a_hash(key, ht->capacity);

	node* current = ht->buckets[index];
	node* previous = NULL;      // Хранит предыдущий узел для корректного удаления из цепочки
    
    while (current != NULL) {
		node* next = current->next;         // Сохраняем указатель на следующий узел перед возможным удалением
		if (!strcmp(current->key, key)) {   // Найден ключ для удаления
			if (previous == NULL)           // Если удаляемый узел является первым в цепочке
                ht->buckets[index] = current->next;
            else
                previous->next = current->next;
            
            free(current->key);
            free(current->value);
            free(current);
            
            ht->size--;
            
            return true;
        }
        else
        {
            previous = current;
            current = next;
        }
    }
    return false;
}

// Функция для печати состояния бакетов хэш-таблицы
void ht_print_buckets(const hash_table_t* ht)
{
    printf("  capacity=%zu  size=%zu  load=%.2f\n", ht->capacity, ht->size, ht_load_factor(ht));
	// Проходим по каждому бакету и выводим его содержимое
    for (size_t i = 0; i < ht->capacity; i++) {
		if (ht->buckets[i] == NULL) continue; 
        printf("  [%zu]: ", i);
		// Проходим по цепочке узлов в бакете и выводим ключи
        node* current = ht->buckets[i];
        while (current != NULL) {
            printf("\"%s\"", current->key);
            if (current->next) printf(" -> ");
            current = current->next;
        }
        printf("\n");
    }
}

// Функция для получения всех ключей из хэш-таблицы в виде массива строк
char** ht_keys(const hash_table_t* ht, size_t* count)
{
    if (ht == NULL) {
        if (count) *count = 0;
        return NULL;
    }

    char** keys = malloc(sizeof(char*) * ht->size);
    if (keys == NULL) {
        if (count) *count = 0;
        return NULL;
    }

    size_t index = 0;
	// Проходим по каждому бакету и цепочке
    for (size_t i = 0; i < ht->capacity; i++) {
        node* current = ht->buckets[i];
        while (current) {
            keys[index++] = current->key;
            current = current->next;
        }
    }

    if (count) *count = index;
    return keys;
}