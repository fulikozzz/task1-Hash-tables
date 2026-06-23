#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash_table.h"
#include "set.h"
#include "multiset.h"

#define TEST(name)   printf("\n=== %s ===\n", name)
#define ОК(msg)    printf("  [OK] %s\n", msg)
#define TOTAL() \
    printf("\nИтог: %d/%d тестов пройдено\n", passed, total)

static int passed = 0;
static int total  = 0;

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

    hash_table_t* ht = ht_create(8);

    CHECK(ht != NULL,               "создание таблицы");
    CHECK(ht_size(ht) == 0,         "размер пустой таблицы = 0");

    ht_insert(ht, "apple", _strdup("apple"));
    ht_insert(ht, "banana", _strdup("banana"));
    ht_insert(ht, "cherry", _strdup("cherry"));

    CHECK(ht_size(ht) == 3,         "размер после 3 вставок = 3");
    CHECK(ht_find(ht, "apple") != NULL,    "поиск существующего элемента");
    CHECK(ht_find(ht, "mango") == NULL,    "поиск несуществующего элемента");

    ht_remove(ht, "apple");
    CHECK(ht_size(ht) == 2,         "размер после удаления = 2");
    CHECK(ht_find(ht, "apple") == NULL,   "удалённый элемент не находится");

    ht_destroy(ht);
}

void collision_test()
{
    TEST("Коллизии");

    hash_table_t* ht = ht_create(3);

    const char* arr[] = {
        "apple", "banana", "cherry", "date",
        "car", "fly", "grape", "miss"
    };
    int n = 8;

    for (int i = 0; i < n; i++)
        ht_insert(ht, arr[i], _strdup(arr[i]));

    ht_insert(ht, "banana", _strdup("banana"));
    printf("  Состояние бакетов после вставки %d элементов:\n", n);
    ht_print_buckets(ht);

    int founded = 0;
    for (int i = 0; i < n; i++)
        if (ht_find(ht, arr[i])) founded++;

    CHECK(founded == n, "все элементы находятся при наличии коллизий");
    CHECK(ht_find(ht, "mango") == NULL, "несуществующий элемент не находится");

    ht_destroy(ht);
}

void rehash_test()
{
    TEST("Рехеширование");

    hash_table_t* ht = ht_create(4);

    printf("  Начальная capacity = 4\n");

    ht_insert(ht, "1", _strdup("1"));
    printf("  После 1 вставки: load=%.2f  capacity=%zu\n", ht_load_factor(ht), ht_capacity(ht));

    ht_insert(ht, "2", _strdup("2"));
    ht_insert(ht, "3", _strdup("3"));

    printf("  После 3 вставок: load=%.2f size=%zu capacity=%zu\n", ht_load_factor(ht), ht_size(ht),ht_capacity(ht));

    ht_insert(ht, "4", _strdup("4"));
    ht_insert(ht, "5", _strdup("5"));

    printf("  После 5 вставок:  load=%.2f size=%zu capacity=%zu\n", ht_load_factor(ht), ht_size(ht), ht_capacity(ht));

    printf("  Состояние после рехеша:\n");
    ht_print_buckets(ht);

    CHECK(ht_load_factor(ht) <= 0.75, "Загрузка не превышает 0.75 после рехеша");
    CHECK(ht_find(ht, "1") != NULL,        "элемент 1 найден после рехеша");
    CHECK(ht_find(ht, "5") != NULL,        "элемент 5 найден после рехеша");

    ht_destroy(ht);
}

void set_test()
{
    TEST("Set: базовые операции");

    set_t* a = set_create(8);
    CHECK(a != NULL, "создание set a");

    set_add(a, "apple");
    set_add(a, "banana");
    set_add(a, "cherry");

    CHECK(set_size(a) == 3, "После 3 вставок = 3");
    CHECK(set_contains(a, "apple"), "set содержит apple");
    CHECK(!set_contains(a, "mango"), "set не содержит mango");

    set_remove(a, "apple");
    CHECK(!set_contains(a, "apple"), "apple удалён из set");
    CHECK(set_size(a) == 2, "размер после удаления = 2");

    set_t* b = set_create(4);
    set_add(b, "banana");
    set_add(b, "date");

    set_t* u = set_union(a, b);
    CHECK(u != NULL, "Объединение создано");
    CHECK(set_size(u) == 3, "размер = 3 (banana,cherry,date)");

    printf("  Состояние бакетов после объединения: \n"); 
    set_print_buckets(u);

    set_t* inter = set_intersection(a, b);
    CHECK(inter != NULL, "Пересечение создано");
    CHECK(set_size(inter) == 1 && set_contains(inter, "banana"), "пересечение содержит banana 1 раз");
    printf("  Состояние бакетов после пересечения: \n");
    set_print_buckets(u);

    set_t* diff = set_difference(a, b);
    CHECK(diff != NULL, "Разность создана");
    CHECK(set_size(diff) == 1 && set_contains(diff, "cherry"), "разность содержит cherry");

    set_destroy(a);
    set_destroy(b);
    set_destroy(u);
    set_destroy(inter);
    set_destroy(diff);
}

void multiset_test()
{
    TEST("Multiset: базовые операции");

    multiset_t* a = multiset_create(8);
    CHECK(a != NULL, "создание multiset a");

    multiset_add(a, "apple");
    multiset_add(a, "apple");
    multiset_add(a, "banana");

    CHECK(multiset_count(a, "apple") == 2, "количество apple = 2");
    CHECK(multiset_size(a) == 3, "размер == 3");

    multiset_remove(a, "apple");
    CHECK(multiset_count(a, "apple") == 1, "после удаления размер = 1");

    multiset_remove(a, "apple");
    CHECK(multiset_count(a, "apple") == 0, "удалено apple");

    multiset_t* b = multiset_create(8);
    multiset_add(b, "apple");
    multiset_add(b, "apple");
    multiset_add(b, "cherry");
    multiset_add(b, "cherry");

    multiset_t* sum = multiset_sum(a, b);
    CHECK(sum != NULL, "Сумма создана");
    CHECK(multiset_count(sum, "apple") == 2, "количество apple = 2");
    CHECK(multiset_count(sum, "cherry") == 2, "количество cherry = 2");

    multiset_t* inter = multiset_intersection(a, b);
    CHECK(inter != NULL, "Пересечение создана");
    CHECK(multiset_count(inter, "apple") == 0, "apple содержится 0 раз");

    multiset_t* uni = multiset_union(a, b);
    CHECK(uni != NULL, "Объединение создано");
    CHECK(multiset_count(uni, "apple") == 2, "apple содержится 2 раза");

    multiset_destroy(a);
    multiset_destroy(b);
    multiset_destroy(sum);
    multiset_destroy(inter);
    multiset_destroy(uni);
}

int main()
{
	// тест хэш-таблицы
    test();
    collision_test();
    rehash_test();
    // тест структур
    set_test();
    multiset_test();

    TOTAL();

    return (passed == total) ? 0 : 1;
}