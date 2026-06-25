#include "lock_free_hash_table.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdatomic.h>

#define FNV1A64_OFFSET_BASIS  14695981039346656037
#define FNV1A64_PRIME         1099511628211

#define MAX_LOAD_FACTOR 0.75

// Структура узла
typedef struct _node
{
    char*                   key;        // ключ (строка)
    _Atomic(void*)          value;      // void* для хранения значения любого типа
    struct _node*           next;       // указатель на следующий узел в цепочке
} node;

// Структура хэш-таблицы
struct _lf_hash_table_t
{
    _Atomic(node*)*         buckets;    // массив указателей на узлы
    _Atomic(size_t)         size;       // текущее количество элементов в таблице
    size_t                  capacity;   // текущая вместимость таблицы
};

lf_hash_table_t* lf_ht_create(size_t capacity)
{
    if (capacity == 0)
        capacity = 1;

    lf_hash_table_t* ht = malloc(sizeof(lf_hash_table_t));

    if (ht == NULL)
        return NULL;

    ht->buckets = calloc(capacity, sizeof(node*));

    if (ht->buckets == NULL)
    {
        free(ht);
        return NULL;
    }

    atomic_init(&ht->size, 0);
    ht->capacity = capacity;

    return ht;
}

void lf_ht_destroy(lf_hash_table_t* ht)
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
static size_t lf_fnv1a_hash(const char* key, size_t capacity)
{
    size_t index = FNV1A64_OFFSET_BASIS;
    while (*key) {
        index = index ^ *key;
        key++;
        index = index * FNV1A64_PRIME;
    }
    return index % capacity;
}

size_t lf_ht_size(const lf_hash_table_t* ht)
{
    return atomic_load(&ht->size);
}

size_t lf_ht_capacity(const lf_hash_table_t* ht)
{
    return ht->capacity;
}

double lf_ht_load_factor(const lf_hash_table_t* ht)
{
    return (double)atomic_load(&ht->size) / ht->capacity;
}
 
bool lf_ht_rehash(lf_hash_table_t* ht)
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

            size_t new_index = lf_fnv1a_hash(current->key, new_capacity);

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

bool lf_ht_insert(lf_hash_table_t* ht, const char* key, void* value)
{
    if (ht == NULL || key == NULL) return false;

    // проверяем наличие ключа в таблице
    size_t index = lf_fnv1a_hash(key, ht->capacity);
    node* current = atomic_load(&ht->buckets[index]);
    while (current != NULL) {
        if (!strcmp(current->key, key)) {
            atomic_store(&current->value, value);  // обновляем без рехеша
            return true;
        }
        current = current->next;
    }

    // если ключ новый
    if (lf_ht_load_factor(ht) > MAX_LOAD_FACTOR) {
        if (!lf_ht_rehash(ht)) return false;
        index = lf_fnv1a_hash(key, ht->capacity);  // пересчитываем после рехеша
    }

    node* to_insert = malloc(sizeof(node));
    if (to_insert == NULL) return false;

    to_insert->key = _strdup(key);
    if (to_insert->key == NULL) {
        free(to_insert);
        return false;
    }

    atomic_init(&to_insert->value, value);
    
    // Указатель для хранения старого значения
    node* old_head;

    do {
		old_head = atomic_load(&ht->buckets[index]);
		to_insert->next = old_head;
    } while (!atomic_compare_exchange_weak(
        &ht->buckets[index], &old_head, to_insert));
    
    atomic_fetch_add(&ht->size, 1);

    return true;
}

void* lf_ht_find(lf_hash_table_t* ht, const char* key)
{
    if (ht == NULL || key == NULL) return NULL;

    size_t index = lf_fnv1a_hash(key, ht->capacity);
    // Проходим по цепочке в соответствующем бакете
    node* current = atomic_load(&ht->buckets[index]);
    while (current != NULL) {
        node* next = current->next;
        if (!strcmp(current->key, key))
            return atomic_load(&current->value);
        current = next;
    }
    return NULL;
}
 
bool lf_ht_remove(lf_hash_table_t* ht, const char* key)
{
    if (ht == NULL || key == NULL) return false;

    size_t index = lf_fnv1a_hash(key, ht->capacity);

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
void lf_ht_print_buckets(const lf_hash_table_t* ht)
{
    printf("  capacity=%zu  size=%zu  load=%.2f\n", ht->capacity, ht->size, lf_ht_load_factor(ht));
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
char** lf_ht_keys(const lf_hash_table_t* ht, size_t* count)
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