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

START_TEST(t_query)
{
    auto bs = psudb::bsm::BentleySaxe<record_t, psudb::ISAMTree<key_type, val_type>>();

    std::vector<key_type> keys;
    for (size_t i=0; i<10000; i++) {
        keys.push_back(i);
    }

    std::random_device rd;
    std::mt19937 rng(rd());

    std::shuffle(keys.begin(), keys.end(), rng);

    for (size_t i=0; i<keys.size(); i++) {
        record_t rec = {keys[i], i};
        bs.insert(rec);
    }

    std::sort(keys.begin(), keys.end());

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

START_TEST(t_create_mdsp)
{
    auto bs = psudb::bsm::BentleySaxe<record_t, psudb::ISAMTree<key_type, val_type>, true>();

    ck_assert_int_eq(bs.record_count(), 0);
}
END_TEST


START_TEST(t_insert_mdsp)
{
    auto bs = psudb::bsm::BentleySaxe<record_t, psudb::ISAMTree<key_type, val_type>, true>();
    size_t n=100000;
    for (size_t i=0; i<n; i++) {
        record_t rec = {i, i};
        bs.insert(rec);
    }

    ck_assert_int_eq(bs.record_count(), n);
}
END_TEST

START_TEST(t_query_mdsp)
{
    auto bs = psudb::bsm::BentleySaxe<record_t, psudb::ISAMTree<key_type, val_type>, true>();

    std::vector<key_type> keys;
    for (size_t i=0; i<10000; i++) {
        keys.push_back(i);
    }

    std::random_device rd;
    std::mt19937 rng(rd());

    std::shuffle(keys.begin(), keys.end(), rng);

    for (size_t i=0; i<keys.size(); i++) {
        record_t rec = {keys[i], i};
        bs.insert(rec);
    }

    std::sort(keys.begin(), keys.end());

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



Suite *unit_testing()
{
    Suite *unit = suite_create("Bentley Saxe Framework Unit Tests");

    TCase *create = tcase_create("BentleySaxe::Create Unit Tests");
    tcase_add_test(create, t_create);
    tcase_add_test(create, t_create_mdsp);

    suite_add_tcase(unit, create);


    TCase *insert = tcase_create("BentleySaxe::insert Unit Tests");
    tcase_add_test(insert, t_insert);
    tcase_add_test(insert, t_insert_mdsp);

    suite_add_tcase(unit, insert);


    TCase *query = tcase_create("BentleySaxe::query Unit Tests");
    tcase_add_test(query, t_query);
    tcase_add_test(query, t_query_mdsp);
    tcase_set_timeout(query, 1000);

    suite_add_tcase(unit, query);

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
