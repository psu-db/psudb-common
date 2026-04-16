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
#include <algorithm>

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

START_TEST(t_unbuild)
{
    size_t n = 100;
    std::vector<record_t> records;
    for (size_t i=0; i<n; i++) {
        records.push_back({(key_type) i, (val_type) i});
    }

    auto isam = psudb::ISAMTree<key_type, val_type, 64>::build(records);
    ck_assert_int_eq(isam->record_count(), n);

    auto result = isam->unbuild();
    ck_assert_int_eq(result.size(), n);

    /* Records should come back sorted */
    for (size_t i=0; i<n; i++) {
        ck_assert_int_eq(result[i].first, (key_type) i);
        ck_assert_int_eq(result[i].second, (val_type) i);
    }

    /* After unbuild, record_count should be 0 (data was moved out) */
    ck_assert_int_eq(isam->record_count(), 0);
    delete isam;
}
END_TEST


START_TEST(t_unbuild_empty)
{
    std::vector<record_t> records;
    auto isam = psudb::ISAMTree<key_type, val_type, 64>::build(records);

    auto result = isam->unbuild();
    ck_assert_int_eq(result.size(), 0);
    delete isam;
}
END_TEST


START_TEST(t_query_merge)
{
    std::vector<record_t> a = {{1, 10}, {3, 30}};
    std::vector<record_t> b = {{2, 20}, {4, 40}};

    std::vector<record_t> records = {{0, 0}};
    auto isam = psudb::ISAMTree<key_type, val_type, 64>::build(records);

    auto merged = isam->query_merge(a, b);
    ck_assert_int_eq(merged.size(), 4);

    /* query_merge concatenates a and b */
    ck_assert_int_eq(merged[0].first, 1);
    ck_assert_int_eq(merged[1].first, 3);
    ck_assert_int_eq(merged[2].first, 2);
    ck_assert_int_eq(merged[3].first, 4);

    delete isam;
}
END_TEST


START_TEST(t_query_merge_empty)
{
    std::vector<record_t> a;
    std::vector<record_t> b = {{1, 10}};

    std::vector<record_t> records = {{0, 0}};
    auto isam = psudb::ISAMTree<key_type, val_type, 64>::build(records);

    auto merged = isam->query_merge(a, b);
    ck_assert_int_eq(merged.size(), 1);
    ck_assert_int_eq(merged[0].first, 1);

    delete isam;
}
END_TEST


START_TEST(t_query_null_param)
{
    std::vector<record_t> records = {{1, 1}, {2, 2}};
    auto isam = psudb::ISAMTree<key_type, val_type, 64>::build(records);

    auto result = isam->query(nullptr);
    ck_assert_int_eq(result.size(), 0);

    delete isam;
}
END_TEST


START_TEST(t_query_point)
{
    /* lower_bound == upper_bound should return nothing (query uses strict <) */
    size_t n = 100;
    std::vector<record_t> records;
    for (size_t i=0; i<n; i++) {
        records.push_back({(key_type) i, (val_type) i});
    }

    auto isam = psudb::ISAMTree<key_type, val_type, 64>::build(records);

    psudb::ISAMTree<key_type, val_type, 64>::RangeQueryParameters q;
    q.lower_bound = 50;
    q.upper_bound = 50;
    auto result = isam->query(&q);
    ck_assert_int_eq(result.size(), 0);

    delete isam;
}
END_TEST


START_TEST(t_query_single_key)
{
    /* Range that captures exactly one key */
    size_t n = 100;
    std::vector<record_t> records;
    for (size_t i=0; i<n; i++) {
        records.push_back({(key_type) i, (val_type) i});
    }

    auto isam = psudb::ISAMTree<key_type, val_type, 64>::build(records);

    psudb::ISAMTree<key_type, val_type, 64>::RangeQueryParameters q;
    q.lower_bound = 50;
    q.upper_bound = 51;
    auto result = isam->query(&q);
    ck_assert_int_eq(result.size(), 1);
    ck_assert_int_eq(result[0].first, 50);

    delete isam;
}
END_TEST


START_TEST(t_query_out_of_range)
{
    /* Query range falls entirely outside data */
    std::vector<record_t> records;
    for (size_t i=0; i<100; i++) {
        records.push_back({(key_type) i, (val_type) i});
    }

    auto isam = psudb::ISAMTree<key_type, val_type, 64>::build(records);

    psudb::ISAMTree<key_type, val_type, 64>::RangeQueryParameters q;
    q.lower_bound = 200;
    q.upper_bound = 300;
    auto result = isam->query(&q);
    ck_assert_int_eq(result.size(), 0);

    delete isam;
}
END_TEST


START_TEST(t_query_below_range)
{
    /* Query range falls entirely below data */
    std::vector<record_t> records;
    for (size_t i=100; i<200; i++) {
        records.push_back({(key_type) i, (val_type) i});
    }

    auto isam = psudb::ISAMTree<key_type, val_type, 64>::build(records);

    psudb::ISAMTree<key_type, val_type, 64>::RangeQueryParameters q;
    q.lower_bound = 0;
    q.upper_bound = 50;
    auto result = isam->query(&q);
    ck_assert_int_eq(result.size(), 0);

    delete isam;
}
END_TEST


START_TEST(t_query_min_max_boundary)
{
    /* Query exactly at the min and max keys */
    std::vector<record_t> records;
    for (size_t i=0; i<100; i++) {
        records.push_back({(key_type) i, (val_type) i});
    }

    auto isam = psudb::ISAMTree<key_type, val_type, 64>::build(records);

    /* Lower bound at min key (0), upper bound at max key (99) -- exclusive */
    psudb::ISAMTree<key_type, val_type, 64>::RangeQueryParameters q;
    q.lower_bound = 0;
    q.upper_bound = 99;
    auto result = isam->query(&q);
    ck_assert_int_eq(result.size(), 99);
    ck_assert_int_eq(result[0].first, 0);
    ck_assert_int_eq(result[result.size()-1].first, 98);

    /* Upper bound past max key -- should include all */
    q.lower_bound = 0;
    q.upper_bound = 100;
    result = isam->query(&q);
    ck_assert_int_eq(result.size(), 100);

    delete isam;
}
END_TEST


START_TEST(t_query_empty_tree)
{
    std::vector<record_t> records;
    auto isam = psudb::ISAMTree<key_type, val_type, 64>::build(records);

    psudb::ISAMTree<key_type, val_type, 64>::RangeQueryParameters q;
    q.lower_bound = 0;
    q.upper_bound = 100;
    auto result = isam->query(&q);
    ck_assert_int_eq(result.size(), 0);

    delete isam;
}
END_TEST


START_TEST(t_query_reproducible)
{
    /* Like t_query but with a fixed seed for reproducibility. */
    srand(12345);
    size_t n = 100000;
    std::vector<record_t> records;
    std::vector<key_type> records_copy;
    for (size_t i=0; i<n; i++) {
        key_type k = (int64_t) rand();
        records.push_back({k, (int64_t) i});
        records_copy.push_back(k);
    }

    auto isam = psudb::ISAMTree<key_type, val_type, 64>::build(records);
    std::sort(records_copy.begin(), records_copy.end());

    for (size_t i=0; i<20; i++) {
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


START_TEST(t_build_negative_keys)
{
    std::vector<record_t> records;
    for (int64_t i=-50; i<50; i++) {
        records.push_back({i, i});
    }

    auto isam = psudb::ISAMTree<key_type, val_type, 64>::build(records);
    ck_assert_int_eq(isam->record_count(), 100);

    psudb::ISAMTree<key_type, val_type, 64>::RangeQueryParameters q;
    q.lower_bound = -25;
    q.upper_bound = 25;
    auto result = isam->query(&q);
    ck_assert_int_eq(result.size(), 50);
    ck_assert_int_eq(result[0].first, -25);
    ck_assert_int_eq(result[result.size()-1].first, 24);

    delete isam;
}
END_TEST


START_TEST(t_build_fanout_boundary)
{
    /* Exactly FANOUT+1 records forces a second leaf and internal node structure change */
    size_t n = 65;
    std::vector<record_t> records;
    for (size_t i=0; i<n; i++) {
        records.push_back({(key_type) i, (val_type) i});
    }

    auto isam = psudb::ISAMTree<key_type, val_type, 64>::build(records);
    ck_assert_int_eq(isam->record_count(), n);

    psudb::ISAMTree<key_type, val_type, 64>::RangeQueryParameters q;
    q.lower_bound = 0;
    q.upper_bound = (key_type) n;
    ck_assert_int_eq(isam->query(&q).size(), n);
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
    tcase_add_test(build, t_build_negative_keys);
    tcase_add_test(build, t_build_fanout_boundary);

    suite_add_tcase(unit, build);

    TCase *unbuild_tc = tcase_create("Static ISAM unbuild Testing");
    tcase_add_test(unbuild_tc, t_unbuild);
    tcase_add_test(unbuild_tc, t_unbuild_empty);

    suite_add_tcase(unit, unbuild_tc);

    TCase *merge = tcase_create("Static ISAM query_merge Testing");
    tcase_add_test(merge, t_query_merge);
    tcase_add_test(merge, t_query_merge_empty);

    suite_add_tcase(unit, merge);

    TCase *query = tcase_create("Static ISAM Query Testing");
    tcase_add_test(query, t_query);
    tcase_add_test(query, t_query_reproducible);
    tcase_add_test(query, t_query_null_param);
    tcase_add_test(query, t_query_point);
    tcase_add_test(query, t_query_single_key);
    tcase_add_test(query, t_query_out_of_range);
    tcase_add_test(query, t_query_below_range);
    tcase_add_test(query, t_query_min_max_boundary);
    tcase_add_test(query, t_query_empty_tree);

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
