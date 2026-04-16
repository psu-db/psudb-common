/*
 * tests/priority_queue_tests.cpp
 *
 * Unit tests for psudb::PriorityQueue
 *
 * Copyright (C) 2023-2026 Douglas B. Rumbaugh <dbrumbaugh@harrisburgu.edu>
 *
 * All rights reserved. Published under the Revised BSD License.
 */

#include <algorithm>
#include <numeric>
#include <random>
#include <vector>

#include "psu-ds/PriorityQueue.h"

#include <check.h>

using namespace psudb;

/* A record type with tombstone support for testing that code path. */
struct TombstoneRecord {
    int key;
    bool tombstone;

    bool operator<(const TombstoneRecord &other) const { return key < other.key; }
    bool operator>(const TombstoneRecord &other) const { return key > other.key; }
    bool is_tombstone() const { return tombstone; }
};


/* --- MinHeap basics --- */

START_TEST(t_empty_on_construction)
{
    PriorityQueue<int> pq(10);
    ck_assert(pq.empty());
    ck_assert_int_eq(pq.size(), 0);
}
END_TEST


START_TEST(t_single_element)
{
    int val = 42;
    PriorityQueue<int> pq(1);
    pq.push(&val);

    ck_assert(!pq.empty());
    ck_assert_int_eq(pq.size(), 1);
    ck_assert_int_eq(*pq.peek().data, 42);

    pq.pop();
    ck_assert(pq.empty());
}
END_TEST


START_TEST(t_inserted_in_order)
{
    int vals[] = {1, 2, 3, 4, 5};
    PriorityQueue<int> pq(5);

    for (auto &v : vals) pq.push(&v);

    for (int expected = 1; expected <= 5; expected++) {
        ck_assert_int_eq(*pq.peek().data, expected);
        pq.pop();
    }
    ck_assert(pq.empty());
}
END_TEST


START_TEST(t_inserted_in_reverse_order)
{
    int vals[] = {5, 4, 3, 2, 1};
    PriorityQueue<int> pq(5);

    for (auto &v : vals) pq.push(&v);

    for (int expected = 1; expected <= 5; expected++) {
        ck_assert_int_eq(*pq.peek().data, expected);
        pq.pop();
    }
    ck_assert(pq.empty());
}
END_TEST


START_TEST(t_random_order)
{
    constexpr size_t N = 100;
    std::vector<int> vals(N);
    std::iota(vals.begin(), vals.end(), 0);

    std::mt19937 rng(42);
    std::shuffle(vals.begin(), vals.end(), rng);

    PriorityQueue<int> pq(N);
    for (auto &v : vals) pq.push(&v);

    for (size_t i = 0; i < N; i++) {
        ck_assert_int_eq(pq.size(), N - i);
        ck_assert_int_eq(*pq.peek().data, (int)i);
        pq.pop();
    }
    ck_assert(pq.empty());
}
END_TEST


START_TEST(t_duplicates)
{
    int vals[] = {3, 1, 3, 1, 2};
    PriorityQueue<int> pq(5);
    for (auto &v : vals) pq.push(&v);

    int prev = *pq.peek().data;
    pq.pop();
    while (!pq.empty()) {
        int cur = *pq.peek().data;
        ck_assert_int_le(prev, cur);
        prev = cur;
        pq.pop();
    }
}
END_TEST


/* --- MaxHeap --- */

START_TEST(t_maxheap_descending)
{
    int vals[] = {3, 1, 4, 1, 5, 9, 2, 6};
    constexpr size_t N = sizeof(vals) / sizeof(vals[0]);
    PriorityQueue<int, standard_maxheap<int>> pq(N);

    for (auto &v : vals) pq.push(&v);

    int prev = *pq.peek().data;
    pq.pop();
    while (!pq.empty()) {
        int cur = *pq.peek().data;
        ck_assert_int_ge(prev, cur);
        prev = cur;
        pq.pop();
    }
}
END_TEST


/* --- Version tracking --- */

START_TEST(t_version_stored)
{
    int vals[] = {10, 20};
    PriorityQueue<int> pq(2);
    pq.push(&vals[0], 5);
    pq.push(&vals[1], 3);

    auto rec = pq.peek();
    ck_assert_int_eq(*rec.data, 10);
    ck_assert_int_eq(rec.version, 5);
}
END_TEST


START_TEST(t_ties_broken_by_version)
{
    int val = 7;
    PriorityQueue<int> pq(2);
    pq.push(&val, 10);
    pq.push(&val, 2);

    auto first = pq.peek();
    ck_assert_int_eq(first.version, 2);
    pq.pop();

    auto second = pq.peek();
    ck_assert_int_eq(second.version, 10);
}
END_TEST


/* --- peek_second --- */

START_TEST(t_peek_second)
{
    int vals[] = {5, 1, 3};
    PriorityQueue<int> pq(3);
    for (auto &v : vals) pq.push(&v);

    ck_assert_int_eq(*pq.peek().data, 1);
    ck_assert_int_eq(*pq.peek_second().data, 3);
}
END_TEST


/* --- Tombstone tiebreaking --- */

START_TEST(t_tombstone_tiebreaking)
{
    TombstoneRecord a{5, true};
    TombstoneRecord b{5, false};

    PriorityQueue<TombstoneRecord> pq(2);
    pq.push(&b, 0);
    pq.push(&a, 0);

    auto first = pq.peek();
    ck_assert(first.data->is_tombstone());
}
END_TEST


START_TEST(t_tombstone_before_non_tombstone)
{
    /*
     * Regression: the old comparator returned (a.is_tombstone() && b.is_tombstone()),
     * which meant a tombstone vs non-tombstone returned false in BOTH directions,
     * giving no ordering guarantee. The fix makes tombstones sort strictly before
     * non-tombstones: a.is_tombstone() && !b.is_tombstone().
     */
    TombstoneRecord ts{5, true};
    TombstoneRecord non_ts{5, false};

    /* Push non-tombstone first, then tombstone -- tombstone should bubble up. */
    PriorityQueue<TombstoneRecord> pq(2);
    pq.push(&non_ts, 0);
    pq.push(&ts, 0);

    auto first = pq.peek();
    ck_assert(first.data->is_tombstone());
    pq.pop();

    auto second = pq.peek();
    ck_assert(!second.data->is_tombstone());
    pq.pop();
    ck_assert(pq.empty());
}
END_TEST


START_TEST(t_tombstone_ordering_reversed_push)
{
    /* Same test but push tombstone first -- should still extract tombstone first. */
    TombstoneRecord ts{5, true};
    TombstoneRecord non_ts{5, false};

    PriorityQueue<TombstoneRecord> pq(2);
    pq.push(&ts, 0);
    pq.push(&non_ts, 0);

    auto first = pq.peek();
    ck_assert(first.data->is_tombstone());
    pq.pop();

    auto second = pq.peek();
    ck_assert(!second.data->is_tombstone());
}
END_TEST


START_TEST(t_tombstone_mixed_with_different_keys)
{
    /* Tombstone tiebreaking should only apply when keys and versions are equal.
     * Different keys should sort by key regardless of tombstone status. */
    TombstoneRecord a{3, false};
    TombstoneRecord b{5, true};

    PriorityQueue<TombstoneRecord> pq(2);
    pq.push(&b, 0);
    pq.push(&a, 0);

    /* key 3 < key 5, so a should come first regardless of tombstone */
    ck_assert_int_eq(pq.peek().data->key, 3);
    ck_assert(!pq.peek().data->is_tombstone());
}
END_TEST


/* --- Edge cases --- */

START_TEST(t_two_elements_min_first)
{
    int vals[] = {1, 2};
    PriorityQueue<int> pq(2);
    pq.push(&vals[0]);
    pq.push(&vals[1]);

    ck_assert_int_eq(*pq.peek().data, 1);
    pq.pop();
    ck_assert_int_eq(*pq.peek().data, 2);
    pq.pop();
    ck_assert(pq.empty());
}
END_TEST


START_TEST(t_two_elements_max_first)
{
    int vals[] = {2, 1};
    PriorityQueue<int> pq(2);
    pq.push(&vals[0]);
    pq.push(&vals[1]);

    ck_assert_int_eq(*pq.peek().data, 1);
    pq.pop();
    ck_assert_int_eq(*pq.peek().data, 2);
    pq.pop();
    ck_assert(pq.empty());
}
END_TEST


START_TEST(t_fill_to_exact_capacity)
{
    constexpr size_t N = 7;
    int vals[N] = {4, 7, 2, 9, 1, 6, 3};
    PriorityQueue<int> pq(N);

    for (size_t i = 0; i < N; i++) pq.push(&vals[i]);

    ck_assert_int_eq(pq.size(), N);

    int prev = *pq.peek().data;
    pq.pop();
    while (!pq.empty()) {
        int cur = *pq.peek().data;
        ck_assert_int_le(prev, cur);
        prev = cur;
        pq.pop();
    }
}
END_TEST


START_TEST(t_reuse_after_drain)
{
    int vals[] = {5, 3, 8, 1};
    PriorityQueue<int> pq(4);

    /* First fill and drain */
    for (auto &v : vals) pq.push(&v);
    while (!pq.empty()) pq.pop();
    ck_assert(pq.empty());
    ck_assert_int_eq(pq.size(), 0);

    /* Second fill and drain -- verify ordering still correct */
    int vals2[] = {9, 2, 7, 4};
    for (auto &v : vals2) pq.push(&v);
    ck_assert_int_eq(pq.size(), 4);

    ck_assert_int_eq(*pq.peek().data, 2);
    pq.pop();
    ck_assert_int_eq(*pq.peek().data, 4);
    pq.pop();
    ck_assert_int_eq(*pq.peek().data, 7);
    pq.pop();
    ck_assert_int_eq(*pq.peek().data, 9);
    pq.pop();
    ck_assert(pq.empty());
}
END_TEST


START_TEST(t_all_identical_values)
{
    int vals[] = {5, 5, 5, 5, 5};
    PriorityQueue<int> pq(5);
    for (auto &v : vals) pq.push(&v);

    while (!pq.empty()) {
        ck_assert_int_eq(*pq.peek().data, 5);
        pq.pop();
    }
}
END_TEST


START_TEST(t_negative_values)
{
    int vals[] = {-3, -1, -4, -1, -5, 0, 2, -2};
    constexpr size_t N = sizeof(vals) / sizeof(vals[0]);
    PriorityQueue<int> pq(N);
    for (auto &v : vals) pq.push(&v);

    int prev = *pq.peek().data;
    pq.pop();
    while (!pq.empty()) {
        int cur = *pq.peek().data;
        ck_assert_int_le(prev, cur);
        prev = cur;
        pq.pop();
    }
}
END_TEST


START_TEST(t_large_capacity_few_elements)
{
    int vals[] = {3, 1, 2};
    PriorityQueue<int> pq(1000);

    for (auto &v : vals) pq.push(&v);
    ck_assert_int_eq(pq.size(), 3);

    ck_assert_int_eq(*pq.peek().data, 1);
    pq.pop();
    ck_assert_int_eq(*pq.peek().data, 2);
    pq.pop();
    ck_assert_int_eq(*pq.peek().data, 3);
    pq.pop();
    ck_assert(pq.empty());
}
END_TEST


START_TEST(t_peek_second_two_elements)
{
    int vals[] = {10, 20};
    PriorityQueue<int> pq(2);
    pq.push(&vals[0]);
    pq.push(&vals[1]);

    ck_assert_int_eq(*pq.peek().data, 10);
    ck_assert_int_eq(*pq.peek_second().data, 20);
}
END_TEST


START_TEST(t_peek_second_right_child_smaller)
{
    /* Push 3 values so root has both children; right child should be smaller. */
    int vals[] = {1, 3, 2};
    PriorityQueue<int> pq(3);
    for (auto &v : vals) pq.push(&v);

    ck_assert_int_eq(*pq.peek().data, 1);
    ck_assert_int_eq(*pq.peek_second().data, 2);
}
END_TEST


START_TEST(t_version_three_way_tiebreak)
{
    /* Three records with the same pointer, different versions. */
    int val = 42;
    PriorityQueue<int> pq(3);
    pq.push(&val, 10);
    pq.push(&val, 5);
    pq.push(&val, 1);

    ck_assert_int_eq(pq.peek().version, 1);
    pq.pop();
    ck_assert_int_eq(pq.peek().version, 5);
    pq.pop();
    ck_assert_int_eq(pq.peek().version, 10);
    pq.pop();
    ck_assert(pq.empty());
}
END_TEST


START_TEST(t_equal_key_version_no_tombstone)
{
    /* For a type without is_tombstone, equal key+version falls through to false. */
    int a = 5;
    int b = 5;
    PriorityQueue<int> pq(2);
    pq.push(&a, 0);
    pq.push(&b, 0);

    /* Both should be extractable regardless of ordering. */
    ck_assert_int_eq(*pq.peek().data, 5);
    pq.pop();
    ck_assert_int_eq(*pq.peek().data, 5);
    pq.pop();
    ck_assert(pq.empty());
}
END_TEST


START_TEST(t_tombstone_both_tombstones)
{
    /* Two tombstones with equal key and version. */
    TombstoneRecord a{5, true};
    TombstoneRecord b{5, true};

    PriorityQueue<TombstoneRecord> pq(2);
    pq.push(&a, 0);
    pq.push(&b, 0);

    ck_assert(pq.peek().data->is_tombstone());
    pq.pop();
    ck_assert(pq.peek().data->is_tombstone());
    pq.pop();
    ck_assert(pq.empty());
}
END_TEST


START_TEST(t_tombstone_neither_tombstone)
{
    /* Two non-tombstones with equal key and version -- hits the !ts && !ts case. */
    TombstoneRecord a{5, false};
    TombstoneRecord b{5, false};

    PriorityQueue<TombstoneRecord> pq(2);
    pq.push(&a, 0);
    pq.push(&b, 0);

    ck_assert(!pq.peek().data->is_tombstone());
    pq.pop();
    ck_assert(!pq.peek().data->is_tombstone());
    pq.pop();
    ck_assert(pq.empty());
}
END_TEST


START_TEST(t_pop_maintains_heap_deep_sift)
{
    /*
     * Build a heap large enough that popping the root requires sifting
     * down through multiple levels (height >= 3).
     */
    constexpr size_t N = 15;
    int vals[N];
    for (size_t i = 0; i < N; i++) vals[i] = (int)i;

    /* Insert in reverse so every push sifts all the way up. */
    PriorityQueue<int> pq(N);
    for (ssize_t i = N - 1; i >= 0; i--) pq.push(&vals[i]);

    for (size_t i = 0; i < N; i++) {
        ck_assert_int_eq(*pq.peek().data, (int)i);
        pq.pop();
    }
}
END_TEST


/* --- Stress tests --- */

START_TEST(t_fill_and_drain)
{
    constexpr size_t N = 1000;
    std::vector<int> vals(N);
    std::iota(vals.begin(), vals.end(), 0);

    std::mt19937 rng(123);
    std::shuffle(vals.begin(), vals.end(), rng);

    PriorityQueue<int> pq(N);
    for (auto &v : vals) pq.push(&v);

    int prev = *pq.peek().data;
    pq.pop();
    while (!pq.empty()) {
        int cur = *pq.peek().data;
        ck_assert_int_le(prev, cur);
        prev = cur;
        pq.pop();
    }
}
END_TEST


START_TEST(t_interleaved_push_pop)
{
    std::vector<int> vals = {10, 4, 7, 1, 8, 3, 9, 2, 6, 5};
    PriorityQueue<int> pq(vals.size());

    /* Push first 5 */
    for (size_t i = 0; i < 5; i++) pq.push(&vals[i]);

    /* Pop 2 */
    ck_assert_int_eq(*pq.peek().data, 1);
    pq.pop();
    ck_assert_int_eq(*pq.peek().data, 4);
    pq.pop();

    /* Push remaining 5 */
    for (size_t i = 5; i < vals.size(); i++) pq.push(&vals[i]);

    ck_assert_int_eq(pq.size(), 8);

    /* Drain and verify sorted */
    int prev = *pq.peek().data;
    pq.pop();
    while (!pq.empty()) {
        int cur = *pq.peek().data;
        ck_assert_int_le(prev, cur);
        prev = cur;
        pq.pop();
    }
}
END_TEST


Suite *unit_testing()
{
    Suite *unit = suite_create("PriorityQueue Unit Testing");

    TCase *basics = tcase_create("MinHeap Basics");
    tcase_add_test(basics, t_empty_on_construction);
    tcase_add_test(basics, t_single_element);
    tcase_add_test(basics, t_inserted_in_order);
    tcase_add_test(basics, t_inserted_in_reverse_order);
    tcase_add_test(basics, t_random_order);
    tcase_add_test(basics, t_duplicates);
    suite_add_tcase(unit, basics);

    TCase *maxheap = tcase_create("MaxHeap");
    tcase_add_test(maxheap, t_maxheap_descending);
    suite_add_tcase(unit, maxheap);

    TCase *version = tcase_create("Version Tracking");
    tcase_add_test(version, t_version_stored);
    tcase_add_test(version, t_ties_broken_by_version);
    suite_add_tcase(unit, version);

    TCase *peek = tcase_create("peek_second");
    tcase_add_test(peek, t_peek_second);
    suite_add_tcase(unit, peek);

    TCase *tombstone = tcase_create("Tombstone Tiebreaking");
    tcase_add_test(tombstone, t_tombstone_tiebreaking);
    tcase_add_test(tombstone, t_tombstone_both_tombstones);
    tcase_add_test(tombstone, t_tombstone_neither_tombstone);
    tcase_add_test(tombstone, t_tombstone_before_non_tombstone);
    tcase_add_test(tombstone, t_tombstone_ordering_reversed_push);
    tcase_add_test(tombstone, t_tombstone_mixed_with_different_keys);
    suite_add_tcase(unit, tombstone);

    TCase *edge = tcase_create("Edge Cases");
    tcase_add_test(edge, t_two_elements_min_first);
    tcase_add_test(edge, t_two_elements_max_first);
    tcase_add_test(edge, t_fill_to_exact_capacity);
    tcase_add_test(edge, t_reuse_after_drain);
    tcase_add_test(edge, t_all_identical_values);
    tcase_add_test(edge, t_negative_values);
    tcase_add_test(edge, t_large_capacity_few_elements);
    tcase_add_test(edge, t_peek_second_two_elements);
    tcase_add_test(edge, t_peek_second_right_child_smaller);
    tcase_add_test(edge, t_version_three_way_tiebreak);
    tcase_add_test(edge, t_equal_key_version_no_tombstone);
    tcase_add_test(edge, t_pop_maintains_heap_deep_sift);
    suite_add_tcase(unit, edge);

    TCase *stress = tcase_create("Stress Tests");
    tcase_add_test(stress, t_fill_and_drain);
    tcase_add_test(stress, t_interleaved_push_pop);
    suite_add_tcase(unit, stress);

    return unit;
}


int main()
{
    int failed = 0;
    Suite *unit = unit_testing();
    SRunner *unit_runner = srunner_create(unit);

    srunner_run_all(unit_runner, CK_NORMAL);
    failed = srunner_ntests_failed(unit_runner);
    srunner_free(unit_runner);

    return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
