/*
 *  tests/bentley_saxe_tests.cpp
 *
 *  Copyright (C) 2023-2024 Douglas B. Rumbaugh <doug@douglasrumbaugh.com>
 *  Distributed under the Modified BSD License.
 */

#include "psu-util/bentley-saxe.h"
#include "psu-ds/StaticISAM.h"

#include <algorithm>
#include <random>
#include <cstdlib>

#include <check.h>

typedef int64_t key_type;
typedef int64_t val_type;

typedef std::pair<key_type, val_type> record_t;

START_TEST(t_create)
{
    auto bs = psudb::bsm::BentleySaxe<record_t, psudb::ISAMTree<key_type, val_type>>();

    ck_assert_int_eq(bs.record_count(), 0);
}
END_TEST


START_TEST(t_insert)
{
    auto bs = psudb::bsm::BentleySaxe<record_t, psudb::ISAMTree<key_type, val_type>>();
    size_t n=100000;
    for (size_t i=0; i<n; i++) {
        record_t rec = {i, i};
        bs.insert(rec);
    }

    ck_assert_int_eq(bs.record_count(), n);
}
END_TEST


START_TEST(t_insert_then_query)
{
    /* Verify that inserted records are actually retrievable */
    auto bs = psudb::bsm::BentleySaxe<record_t, psudb::ISAMTree<key_type, val_type>>();
    size_t n = 1000;
    for (size_t i=0; i<n; i++) {
        record_t rec = {(key_type) i, (val_type) i};
        bs.insert(rec);
    }

    ck_assert_int_eq(bs.record_count(), n);

    psudb::ISAMTree<key_type, val_type>::RangeQueryParameters parm;
    parm.lower_bound = 0;
    parm.upper_bound = (key_type) n;
    auto res = bs.query(&parm);
    ck_assert_int_eq(res.size(), n);
}
END_TEST


START_TEST(t_insert_single)
{
    auto bs = psudb::bsm::BentleySaxe<record_t, psudb::ISAMTree<key_type, val_type>>();

    record_t rec = {42, 42};
    bs.insert(rec);
    ck_assert_int_eq(bs.record_count(), 1);

    psudb::ISAMTree<key_type, val_type>::RangeQueryParameters parm;
    parm.lower_bound = 0;
    parm.upper_bound = 100;
    auto res = bs.query(&parm);
    ck_assert_int_eq(res.size(), 1);
    ck_assert_int_eq(res[0].first, 42);
}
END_TEST


START_TEST(t_query)
{
    auto bs = psudb::bsm::BentleySaxe<record_t, psudb::ISAMTree<key_type, val_type>>();

    std::vector<key_type> keys;
    for (size_t i=0; i<10000; i++) {
        keys.push_back(i);
    }

    std::mt19937 rng(42);

    std::shuffle(keys.begin(), keys.end(), rng);

    for (size_t i=0; i<keys.size(); i++) {
        record_t rec = {keys[i], i};
        bs.insert(rec);
    }

    std::sort(keys.begin(), keys.end());

    srand(12345);
    for (size_t i=0; i<1000; i++) {
        size_t idx = rand() % keys.size();
        size_t ub = idx + rand() % 1000;

        if(ub >= keys.size()) {
            ub = keys.size() - 1;
        }

        psudb::ISAMTree<key_type, val_type>::RangeQueryParameters parm;
        parm.lower_bound = keys[idx];
        parm.upper_bound = keys[ub];

        auto res = bs.query(&parm);

        ck_assert_int_eq(res.size(), ub - idx);
        for (size_t j=0; j < res.size(); j++) {
            ck_assert_int_ge(res[j].first, keys[idx]);
            ck_assert_int_le(res[j].first, keys[ub]);
        }
    }

}
END_TEST


START_TEST(t_query_empty)
{
    /* Query on an empty structure */
    auto bs = psudb::bsm::BentleySaxe<record_t, psudb::ISAMTree<key_type, val_type>>();

    psudb::ISAMTree<key_type, val_type>::RangeQueryParameters parm;
    parm.lower_bound = 0;
    parm.upper_bound = 100;
    auto res = bs.query(&parm);
    ck_assert_int_eq(res.size(), 0);
}
END_TEST


START_TEST(t_query_no_results)
{
    /* Query range that doesn't overlap any inserted data */
    auto bs = psudb::bsm::BentleySaxe<record_t, psudb::ISAMTree<key_type, val_type>>();

    for (size_t i=0; i<100; i++) {
        record_t rec = {(key_type) i, (val_type) i};
        bs.insert(rec);
    }

    psudb::ISAMTree<key_type, val_type>::RangeQueryParameters parm;
    parm.lower_bound = 500;
    parm.upper_bound = 600;
    auto res = bs.query(&parm);
    ck_assert_int_eq(res.size(), 0);
}
END_TEST


START_TEST(t_query_point)
{
    /* lower == upper should return nothing (strict < in query) */
    auto bs = psudb::bsm::BentleySaxe<record_t, psudb::ISAMTree<key_type, val_type>>();

    for (size_t i=0; i<100; i++) {
        record_t rec = {(key_type) i, (val_type) i};
        bs.insert(rec);
    }

    psudb::ISAMTree<key_type, val_type>::RangeQueryParameters parm;
    parm.lower_bound = 50;
    parm.upper_bound = 50;
    auto res = bs.query(&parm);
    ck_assert_int_eq(res.size(), 0);
}
END_TEST


START_TEST(t_insert_after_query)
{
    /* Interleave inserts and queries */
    auto bs = psudb::bsm::BentleySaxe<record_t, psudb::ISAMTree<key_type, val_type>>();

    /* Insert first batch */
    for (size_t i=0; i<100; i++) {
        record_t rec = {(key_type) i, (val_type) i};
        bs.insert(rec);
    }

    psudb::ISAMTree<key_type, val_type>::RangeQueryParameters parm;
    parm.lower_bound = 0;
    parm.upper_bound = 200;
    auto res1 = bs.query(&parm);
    ck_assert_int_eq(res1.size(), 100);

    /* Insert second batch */
    for (size_t i=100; i<200; i++) {
        record_t rec = {(key_type) i, (val_type) i};
        bs.insert(rec);
    }

    ck_assert_int_eq(bs.record_count(), 200);
    auto res2 = bs.query(&parm);
    ck_assert_int_eq(res2.size(), 200);
}
END_TEST


START_TEST(t_merge_cascade)
{
    /*
     * The Bentley-Saxe method merges levels geometrically. Inserting
     * exactly 2^k records forces a cascade through k levels. Insert
     * a power-of-two count and verify everything is still queryable.
     */
    auto bs = psudb::bsm::BentleySaxe<record_t, psudb::ISAMTree<key_type, val_type>>();
    size_t n = 256; /* 2^8 -- forces 8 merges */

    for (size_t i=0; i<n; i++) {
        record_t rec = {(key_type) i, (val_type) i};
        bs.insert(rec);
    }

    ck_assert_int_eq(bs.record_count(), n);

    psudb::ISAMTree<key_type, val_type>::RangeQueryParameters parm;
    parm.lower_bound = 0;
    parm.upper_bound = (key_type) n;
    auto res = bs.query(&parm);
    ck_assert_int_eq(res.size(), n);

    /* Verify all keys are present by sorting and checking */
    std::sort(res.begin(), res.end());
    for (size_t i=0; i<n; i++) {
        ck_assert_int_eq(res[i].first, (key_type) i);
    }
}
END_TEST


START_TEST(t_merge_cascade_plus_one)
{
    /*
     * 2^k + 1 records: after the cascade from 2^k, we add one more
     * record that sits alone on level 0.
     */
    auto bs = psudb::bsm::BentleySaxe<record_t, psudb::ISAMTree<key_type, val_type>>();
    size_t n = 257;

    for (size_t i=0; i<n; i++) {
        record_t rec = {(key_type) i, (val_type) i};
        bs.insert(rec);
    }

    ck_assert_int_eq(bs.record_count(), n);

    psudb::ISAMTree<key_type, val_type>::RangeQueryParameters parm;
    parm.lower_bound = 0;
    parm.upper_bound = (key_type) n;
    auto res = bs.query(&parm);
    ck_assert_int_eq(res.size(), n);
}
END_TEST


START_TEST(t_negative_keys)
{
    auto bs = psudb::bsm::BentleySaxe<record_t, psudb::ISAMTree<key_type, val_type>>();

    for (int64_t i=-50; i<50; i++) {
        record_t rec = {i, i};
        bs.insert(rec);
    }

    ck_assert_int_eq(bs.record_count(), 100);

    psudb::ISAMTree<key_type, val_type>::RangeQueryParameters parm;
    parm.lower_bound = -25;
    parm.upper_bound = 25;
    auto res = bs.query(&parm);
    ck_assert_int_eq(res.size(), 50);

    std::sort(res.begin(), res.end());
    ck_assert_int_eq(res[0].first, -25);
    ck_assert_int_eq(res[res.size()-1].first, 24);
}
END_TEST


Suite *unit_testing()
{
    Suite *unit = suite_create("Bentley Saxe Framework Unit Tests");

    TCase *create = tcase_create("BentleySaxe::Create Unit Tests");
    tcase_add_test(create, t_create);

    suite_add_tcase(unit, create);


    TCase *insert = tcase_create("BentleySaxe::insert Unit Tests");
    tcase_add_test(insert, t_insert);
    tcase_add_test(insert, t_insert_then_query);
    tcase_add_test(insert, t_insert_single);
    tcase_add_test(insert, t_insert_after_query);

    suite_add_tcase(unit, insert);


    TCase *query = tcase_create("BentleySaxe::query Unit Tests");
    tcase_add_test(query, t_query);
    tcase_add_test(query, t_query_empty);
    tcase_add_test(query, t_query_no_results);
    tcase_add_test(query, t_query_point);

    tcase_set_timeout(query, 1000);
    suite_add_tcase(unit, query);


    TCase *merge = tcase_create("BentleySaxe Merge Cascade Tests");
    tcase_add_test(merge, t_merge_cascade);
    tcase_add_test(merge, t_merge_cascade_plus_one);
    tcase_add_test(merge, t_negative_keys);

    tcase_set_timeout(merge, 1000);
    suite_add_tcase(unit, merge);

    return unit;
}


int run_unit_tests()
{
    int failed = 0;
    Suite *unit = unit_testing();
    SRunner *unit_runner = srunner_create(unit);

    srunner_run_all(unit_runner, CK_NORMAL);
    failed = srunner_ntests_failed(unit_runner);
    srunner_free(unit_runner);

    return failed;
}


int main() 
{
    int unit_failed = run_unit_tests();

    return (unit_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
