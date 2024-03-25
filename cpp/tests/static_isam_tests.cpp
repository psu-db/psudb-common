/*
 *  tests/static_isam_tests.cpp
 *
 *  Copyright (C) 2023-2024 Douglas B. Rumbaugh <doug@douglasrumbaugh.com>
 *  Distributed under the Modified BSD License.
 */

#include "psu-ds/StaticISAM.h"
#include "psu-dist/zipf.h"

#include <cstdlib>
#include <cstdio>

/* WARNING: due to #define issues, must include this header LAST in C++ projects */
#include <check.h>

typedef int64_t key_type;
typedef int64_t val_type;
typedef std::pair<key_type, val_type> record_t;

START_TEST(t_build)
{
    size_t n = 10000;
    std::vector<record_t> records;

    for (size_t i=0; i<n; i++) {
        records.push_back({(key_type) rand(), i});
    }

    auto isam = psudb::ISAMTree<key_type, val_type, 64>::build(records);
    ck_assert_int_eq(isam->record_count(), n);
    ck_assert_int_eq(isam->internal_node_count(), 4); // calculated by hand
    
    psudb::ISAMTree<key_type, val_type, 64>::RangeQueryParameters q;
    q.lower_bound = 0;
    q.upper_bound = INT64_MAX;

    ck_assert_int_eq(isam->query(&q).size(), n);
    delete isam;
}
END_TEST


START_TEST(t_build_debug_1)
{
    size_t n = 16;
    std::vector<record_t> records;
    for (size_t i=0; i<n; i++) {
        records.push_back({i,i});
    }

    auto isam = psudb::ISAMTree<key_type, val_type, 4>::build_presorted(records);
    ck_assert_int_eq(isam->record_count(), n);
    ck_assert_int_eq(isam->internal_node_count(), 1); // calculated by hand
    
    psudb::ISAMTree<key_type, val_type, 4>::RangeQueryParameters q;
    q.lower_bound = 0;
    q.upper_bound = INT64_MAX;
    ck_assert_int_eq(isam->query(&q).size(), n);
    delete isam;
}
END_TEST


START_TEST(t_build_debug_2)
{
    size_t n = 20;
    std::vector<record_t> records;
    for (size_t i=0; i<n; i++) {
        records.push_back({i,i});
    }

    auto isam = psudb::ISAMTree<key_type, val_type, 4>::build_presorted(records);
    ck_assert_int_eq(isam->record_count(), n);
    ck_assert_int_eq(isam->internal_node_count(), 3); // calculated by hand
    
    psudb::ISAMTree<key_type, val_type, 4>::RangeQueryParameters q;
    q.lower_bound = 0;
    q.upper_bound = INT64_MAX;
    ck_assert_int_eq(isam->query(&q).size(), n);
    delete isam;
}
END_TEST


START_TEST(t_query_debug_1) 
{
    size_t n = 16;
    std::vector<record_t> records;
    for (size_t i=0; i<n; i++) {
        records.push_back({i,i});
    }

    auto isam = psudb::ISAMTree<key_type, val_type, 4>::build_presorted(records);

    psudb::ISAMTree<key_type, val_type, 4>::RangeQueryParameters q;
    q.lower_bound = 5;
    q.upper_bound = 13;

    auto result = isam->query(&q);

    ck_assert_int_eq(result.size(), q.upper_bound - q.lower_bound);
    for (size_t i=0; i<result.size(); i++) {
        ck_assert_int_eq(result[i].first, q.lower_bound++);
    }
    delete isam;
}
END_TEST

START_TEST(t_query_debug_2)
{
    size_t n = 20;
    std::vector<record_t> records;
    for (size_t i=0; i<n; i++) {
        records.push_back({i,i});
    }

    auto isam = psudb::ISAMTree<key_type, val_type, 4>::build_presorted(records);
    int64_t lower = 5;
    int64_t upper = 13;

    psudb::ISAMTree<key_type, val_type, 4>::RangeQueryParameters q;
    q.lower_bound = 5;
    q.upper_bound = 13;
    auto result = isam->query(&q);

    ck_assert_int_eq(result.size(), q.upper_bound - q.lower_bound);
    for (size_t i=0; i<result.size(); i++) {
        ck_assert_int_eq(result[i].first, lower++);
    }
    delete isam;
}
END_TEST


START_TEST(t_build_empty)
{
    std::vector<record_t> records;

    /*
     * Verify that the structure is indeed empty, and responds to
     * queries accordingly.
     */
    auto isam = psudb::ISAMTree<key_type, val_type, 64>::build(records);
    ck_assert_int_eq(isam->record_count(), 0);
    ck_assert_int_eq(isam->internal_node_count(), 0);

    psudb::ISAMTree<key_type, val_type, 64>::RangeQueryParameters q;
    q.lower_bound = 0;
    q.upper_bound = INT64_MAX;
    ck_assert_int_eq(isam->query(&q).size(), 0);
    delete isam;
}
END_TEST


START_TEST(t_build_oneleaf)
{
    std::vector<record_t> records;
    for (size_t i=0; i<64; i++) {
        records.push_back({i, i});
    }

    auto isam = psudb::ISAMTree<key_type, val_type, 64>::build(records);
    ck_assert_int_eq(isam->record_count(), 64);
    ck_assert_int_eq(isam->internal_node_count(), 1);
    delete isam;
}
END_TEST


START_TEST(t_build_onerec) 
{
    std::vector<record_t> records = {{5, 5}};
    auto isam = psudb::ISAMTree<key_type, val_type, 64>::build(records);

    ck_assert_int_eq(isam->record_count(), 1);
    ck_assert_int_eq(isam->internal_node_count(), 1);

    /* verify that querying works */
    psudb::ISAMTree<key_type, val_type, 64>::RangeQueryParameters q;
    q.lower_bound = 0;
    q.upper_bound = 4;
    auto r1 = isam->query(&q);
    ck_assert_int_eq(r1.size(), 0);

    q.lower_bound = 0;
    q.upper_bound = 10;
    auto r2 = isam->query(&q);
    ck_assert_int_eq(r2.size(), 1);
    ck_assert_int_eq(r2[0].first, 5);
    ck_assert_int_eq(r2[0].second, 5);
    delete isam;
}
END_TEST


START_TEST(t_build_tworec) 
{
    std::vector<record_t> records = {{5, 5}, {8, 8}};
    auto isam = psudb::ISAMTree<key_type, val_type, 64>::build(records);

    ck_assert_int_eq(isam->record_count(), 2);
    ck_assert_int_eq(isam->internal_node_count(), 1);
    psudb::ISAMTree<key_type, val_type, 64>::RangeQueryParameters q;
    q.lower_bound = 0;
    q.upper_bound = 4;

    /* verify that querying works */
    auto r1 = isam->query(&q);
    ck_assert_int_eq(r1.size(), 0);

    q.lower_bound = 0;
    q.upper_bound = 10;
    auto r2 = isam->query(&q);

    ck_assert_int_eq(r2.size(), 2);
    ck_assert_int_eq(r2[0].first, 5);
    ck_assert_int_eq(r2[0].second, 5);
    ck_assert_int_eq(r2[1].first, 8);
    ck_assert_int_eq(r2[1].second, 8);
    delete isam;
}
END_TEST


START_TEST(t_build_presorted) 
{
    size_t n = 10000;
    std::vector<record_t> records;

    for (size_t i=0; i<n; i++) {
        records.push_back({(key_type) rand(), i});
    }

    std::sort(records.begin(), records.end());

    auto isam = psudb::ISAMTree<key_type, val_type, 64>::build_presorted(records);
    ck_assert_int_eq(isam->record_count(), n);
    ck_assert_int_eq(isam->internal_node_count(), 4); // calculated by hand
    
    psudb::ISAMTree<key_type, val_type, 64>::RangeQueryParameters q;
    q.lower_bound = 0;
    q.upper_bound = INT64_MAX;
    ck_assert_int_eq(isam->query(&q).size(), n);
    delete isam;
}
END_TEST


START_TEST(t_query)
{
    size_t n = 1000000;
    std::vector<record_t> records;
    std::vector<key_type> records_copy;
    for (size_t i=0; i<n; i++) {
        records.push_back({(int64_t) rand(), (int64_t) rand()});
        records_copy.push_back(records[i].first);
    }

    auto isam = psudb::ISAMTree<key_type, val_type, 64>::build(records);

    std::sort(records_copy.begin(), records_copy.end());

    for (size_t i=0; i<10; i++) {
        int64_t test_key1 = rand();
        int64_t test_key2 = rand();

        auto lower = std::min(test_key1, test_key2);
        auto upper = std::max(test_key1, test_key2);
        psudb::ISAMTree<key_type, val_type, 64>::RangeQueryParameters q;
        q.lower_bound = lower;
        q.upper_bound = upper;

        auto results = isam->query(&q);
        auto test_itr = std::lower_bound(records_copy.begin(), records_copy.end(), lower);
        auto test_upper = std::upper_bound(records_copy.begin(), records_copy.end(), upper);

        ck_assert_int_eq(test_upper - test_itr, results.size());

        for (size_t j=0; j<results.size(); j++) {
            ck_assert_int_eq(results[j].first, *test_itr);
            test_itr++;
        }
    }
    delete isam;
}
END_TEST

Suite *unit_testing()
{
    Suite *unit = suite_create("Static ISAM Tree Unit Testing");

    TCase *debug = tcase_create("Static ISAM Debugging Cases");
    tcase_add_test(debug, t_build_debug_1);
    tcase_add_test(debug, t_build_debug_2);
    tcase_add_test(debug, t_query_debug_1);
    tcase_add_test(debug, t_query_debug_2);

    suite_add_tcase(unit, debug);


    TCase *build = tcase_create("Static ISAM Construction Testing");
    tcase_add_test(build, t_build);
    tcase_add_test(build, t_build_presorted);
    tcase_add_test(build, t_build_empty);
    tcase_add_test(build, t_build_oneleaf);
    tcase_add_test(build, t_build_onerec);
    tcase_add_test(build, t_build_tworec);

    suite_add_tcase(unit, build);

    TCase *query = tcase_create("Static ISAM Query Testing");
    tcase_add_test(query, t_query);

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
