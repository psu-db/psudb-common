/*
 * tests/bloomfilter_tests.cpp
 *
 * Unit tests for psudb::BloomFilter using check
 */

#include <cstdint>

#include "psu-ds/BloomFilter.h"

#include <check.h>

using namespace psudb;


/* --- Basic insert/lookup --- */

START_TEST(t_insert_and_lookup)
{
    BloomFilter<int64_t> bf(1024, 3);

    int64_t key = 42;
    ck_assert_int_eq(bf.insert(key), 1);
    ck_assert(bf.lookup(key));
}
END_TEST


START_TEST(t_lookup_missing)
{
    BloomFilter<int64_t> bf(1024, 3);

    int64_t key = 42;
    bf.insert(key);

    /* An un-inserted key should (almost certainly) not be found with
     * a large enough filter and few insertions. */
    int64_t missing = 99999;
    ck_assert(!bf.lookup(missing));
}
END_TEST


START_TEST(t_multiple_inserts)
{
    BloomFilter<int64_t> bf(10000, 5);

    for (int64_t i = 0; i < 100; i++) {
        bf.insert(i);
    }

    /* All inserted keys must be found (no false negatives) */
    for (int64_t i = 0; i < 100; i++) {
        ck_assert(bf.lookup(i));
    }
}
END_TEST


START_TEST(t_no_false_negatives)
{
    /* Insert many keys and verify zero false negatives */
    size_t n = 1000;
    BloomFilter<int64_t> bf(n * 20, 7);

    for (size_t i = 0; i < n; i++) {
        int64_t key = (int64_t)(i * 17 + 3);
        bf.insert(key);
    }

    for (size_t i = 0; i < n; i++) {
        int64_t key = (int64_t)(i * 17 + 3);
        ck_assert(bf.lookup(key));
    }
}
END_TEST


/* --- False positive rate --- */

START_TEST(t_false_positive_rate)
{
    /*
     * Insert n keys, then probe n non-inserted keys and measure the
     * false positive rate. With the given parameters this should be
     * well under 5%.
     */
    size_t n = 5000;
    size_t k = 7;
    BloomFilter<int64_t> bf(0.01, n, k);

    for (size_t i = 0; i < n; i++) {
        int64_t key = (int64_t) i;
        bf.insert(key);
    }

    size_t false_positives = 0;
    for (size_t i = n; i < 2 * n; i++) {
        int64_t key = (int64_t) i;
        if (bf.lookup(key)) false_positives++;
    }

    double fpr = (double) false_positives / (double) n;
    /* Allow some slack -- target is 1%, we'll accept up to 5% */
    ck_assert_msg(fpr < 0.05,
        "False positive rate %.2f%% exceeds 5%% threshold", fpr * 100.0);
}
END_TEST


/* --- Clear --- */

START_TEST(t_clear)
{
    BloomFilter<int64_t> bf(1024, 3);

    for (int64_t i = 0; i < 50; i++) {
        bf.insert(i);
    }

    bf.clear();

    /* After clear, previously inserted keys should not be found */
    for (int64_t i = 0; i < 50; i++) {
        ck_assert(!bf.lookup(i));
    }
}
END_TEST


START_TEST(t_clear_then_reuse)
{
    BloomFilter<int64_t> bf(2048, 3);

    int64_t key1 = 100;
    bf.insert(key1);
    ck_assert(bf.lookup(key1));

    bf.clear();
    ck_assert(!bf.lookup(key1));

    /* Insert a different key after clear */
    int64_t key2 = 200;
    bf.insert(key2);
    ck_assert(bf.lookup(key2));
    ck_assert(!bf.lookup(key1));
}
END_TEST


/* --- Constructor variants --- */

START_TEST(t_fpr_constructor)
{
    /* Construct using the FPR-based constructor and verify it works */
    BloomFilter<int64_t> bf(0.01, 1000, 7);

    ck_assert_int_gt(bf.memory_usage(), 0);

    int64_t key = 12345;
    bf.insert(key);
    ck_assert(bf.lookup(key));
}
END_TEST


START_TEST(t_single_hash_function)
{
    /* k=1: single hash function -- still functional */
    BloomFilter<int64_t> bf(1024, 1);

    int64_t key = 77;
    bf.insert(key);
    ck_assert(bf.lookup(key));
}
END_TEST


START_TEST(t_many_hash_functions)
{
    /* k=20: many hash functions */
    BloomFilter<int64_t> bf(100000, 20);

    for (int64_t i = 0; i < 100; i++) {
        bf.insert(i);
    }

    for (int64_t i = 0; i < 100; i++) {
        ck_assert(bf.lookup(i));
    }
}
END_TEST


/* --- Different key types --- */

START_TEST(t_uint32_keys)
{
    BloomFilter<uint32_t> bf(4096, 5);

    uint32_t key = 0xDEADBEEF;
    bf.insert(key);
    ck_assert(bf.lookup(key));

    uint32_t missing = 0xCAFEBABE;
    ck_assert(!bf.lookup(missing));
}
END_TEST


START_TEST(t_small_struct_key)
{
    struct SmallKey {
        int32_t a;
        int32_t b;
    };

    BloomFilter<SmallKey> bf(4096, 5);

    SmallKey k1 = {1, 2};
    SmallKey k2 = {3, 4};
    SmallKey k3 = {1, 2};

    bf.insert(k1);
    ck_assert(bf.lookup(k1));
    ck_assert(bf.lookup(k3)); /* same content as k1 */
    ck_assert(!bf.lookup(k2));
}
END_TEST


/* --- Copy/Move semantics (regression for double-free bug) --- */

START_TEST(t_copy_construct)
{
    BloomFilter<int64_t> orig(4096, 5);

    for (int64_t i = 0; i < 100; i++) {
        orig.insert(i);
    }

    BloomFilter<int64_t> copy(orig);

    /* Copy should find all the same keys */
    for (int64_t i = 0; i < 100; i++) {
        ck_assert(copy.lookup(i));
    }

    /* Modifying copy should not affect original */
    copy.clear();
    for (int64_t i = 0; i < 100; i++) {
        ck_assert(orig.lookup(i));
        ck_assert(!copy.lookup(i));
    }
}
END_TEST


START_TEST(t_copy_assign)
{
    BloomFilter<int64_t> a(4096, 5);
    BloomFilter<int64_t> b(2048, 3);

    int64_t key = 42;
    a.insert(key);

    b = a;

    ck_assert(b.lookup(key));

    /* Independent after copy */
    b.clear();
    ck_assert(a.lookup(key));
}
END_TEST


START_TEST(t_move_construct)
{
    BloomFilter<int64_t> orig(4096, 5);

    int64_t key = 123;
    orig.insert(key);

    BloomFilter<int64_t> moved(std::move(orig));
    ck_assert(moved.lookup(key));
}
END_TEST


START_TEST(t_move_assign)
{
    BloomFilter<int64_t> a(4096, 5);
    BloomFilter<int64_t> b(2048, 3);

    int64_t key = 456;
    a.insert(key);

    b = std::move(a);
    ck_assert(b.lookup(key));
}
END_TEST


/* --- Edge cases --- */

START_TEST(t_insert_same_key_twice)
{
    BloomFilter<int64_t> bf(1024, 3);

    int64_t key = 42;
    bf.insert(key);
    bf.insert(key);

    ck_assert(bf.lookup(key));
}
END_TEST


START_TEST(t_negative_keys)
{
    BloomFilter<int64_t> bf(4096, 5);

    int64_t keys[] = {-1, -100, -999999, 0};
    for (auto k : keys) {
        bf.insert(k);
    }

    for (auto k : keys) {
        ck_assert(bf.lookup(k));
    }
}
END_TEST


START_TEST(t_zero_key)
{
    BloomFilter<int64_t> bf(1024, 3);

    int64_t key = 0;
    bf.insert(key);
    ck_assert(bf.lookup(key));
}
END_TEST


START_TEST(t_memory_usage)
{
    BloomFilter<int64_t> bf(1024, 3);
    ck_assert_int_gt(bf.memory_usage(), 0);

    /* Larger filter should use more memory */
    BloomFilter<int64_t> bf_large(100000, 3);
    ck_assert_int_gt(bf_large.memory_usage(), bf.memory_usage());
}
END_TEST


Suite *unit_testing()
{
    Suite *unit = suite_create("BloomFilter Unit Testing");

    TCase *basic = tcase_create("Basic Insert/Lookup");
    tcase_add_test(basic, t_insert_and_lookup);
    tcase_add_test(basic, t_lookup_missing);
    tcase_add_test(basic, t_multiple_inserts);
    tcase_add_test(basic, t_no_false_negatives);
    suite_add_tcase(unit, basic);

    TCase *fpr = tcase_create("False Positive Rate");
    tcase_add_test(fpr, t_false_positive_rate);
    suite_add_tcase(unit, fpr);

    TCase *clr = tcase_create("Clear");
    tcase_add_test(clr, t_clear);
    tcase_add_test(clr, t_clear_then_reuse);
    suite_add_tcase(unit, clr);

    TCase *ctors = tcase_create("Constructor Variants");
    tcase_add_test(ctors, t_fpr_constructor);
    tcase_add_test(ctors, t_single_hash_function);
    tcase_add_test(ctors, t_many_hash_functions);
    suite_add_tcase(unit, ctors);

    TCase *types = tcase_create("Different Key Types");
    tcase_add_test(types, t_uint32_keys);
    tcase_add_test(types, t_small_struct_key);
    suite_add_tcase(unit, types);

    TCase *copymove = tcase_create("Copy/Move Semantics");
    tcase_add_test(copymove, t_copy_construct);
    tcase_add_test(copymove, t_copy_assign);
    tcase_add_test(copymove, t_move_construct);
    tcase_add_test(copymove, t_move_assign);
    suite_add_tcase(unit, copymove);

    TCase *edge = tcase_create("Edge Cases");
    tcase_add_test(edge, t_insert_same_key_twice);
    tcase_add_test(edge, t_negative_keys);
    tcase_add_test(edge, t_zero_key);
    tcase_add_test(edge, t_memory_usage);
    suite_add_tcase(unit, edge);

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
