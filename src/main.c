#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../lock_free_hash_table.h"

#define TEST(name)   printf("\n=== %s ===\n", name)
#define ОК(msg)    printf("  [OK] %s\n", msg)
#define TOTAL() \
    printf("\nИтог: %d/%d тестов пройдено\n", passed, total)

static int passed = 0;
static int total = 0;

#define CHECK(ifelse, msg)   \
    do {                                \
        total++;                        \
        if (ifelse) {                  \
            passed++;                   \
            printf("  [OK] %s\n", msg); \
        } else {                        \
            printf("  [!!] ПРОВАЛ: %s\n", msg); \
        }                               \
    } while(0)

void test()
{
    TEST("Базовые операции");

    lf_hash_table_t* ht = lf_ht_create(8);

    CHECK(ht != NULL, "создание таблицы");
    CHECK(lf_ht_size(ht) == 0, "размер пустой таблицы = 0");

    lf_ht_insert(ht, "apple", _strdup("apple"));
    lf_ht_insert(ht, "banana", _strdup("banana"));
    lf_ht_insert(ht, "cherry", _strdup("cherry"));

    CHECK(lf_ht_size(ht) == 3, "размер после 3 вставок = 3");
    CHECK(lf_ht_find(ht, "apple") != NULL, "поиск существующего элемента");
    CHECK(lf_ht_find(ht, "mango") == NULL, "поиск несуществующего элемента");

    lf_ht_remove(ht, "apple");
    CHECK(lf_ht_size(ht) == 2, "размер после удаления = 2");
    CHECK(lf_ht_find(ht, "apple") == NULL, "удалённый элемент не находится");

    lf_ht_destroy(ht);
}

void collision_test()
{
    TEST("Коллизии");

    lf_hash_table_t* ht = lf_ht_create(10);

    const char* arr[] = {
        "apple", "apple", "banana", "cherry", "date",
        "car", "fly", "grape", "miss"
    };
    int n = 8;

    for (int i = 0; i < n; i++)
        lf_ht_insert(ht, arr[i], _strdup(arr[i]));

    lf_ht_insert(ht, "banana", _strdup("banana"));
    printf("  Состояние бакетов после вставки %d элементов:\n", n);
    lf_ht_print_buckets(ht);

    int founded = 0;
    for (int i = 0; i < n; i++)
        if (lf_ht_find(ht, arr[i])) founded++;

    CHECK(founded == n, "все элементы находятся при наличии коллизий");
    CHECK(lf_ht_find(ht, "mango") == NULL, "несуществующий элемент не находится");

    lf_ht_destroy(ht);
}

int main()
{
    test();
    collision_test();

    TOTAL();

    return (passed == total) ? 0 : 1;
}