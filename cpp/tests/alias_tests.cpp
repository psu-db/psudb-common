/*
 * tests/alias_tests.cpp
 *
 * Unit tests for psudb::Alias (Walker's Alias Structure)
 */

#include <cmath>
#include <vector>

#include "psu-ds/Alias.h"

#include <check.h>

using namespace psudb;

/* Helper: create a seeded GSL RNG for reproducible tests. */
static gsl_rng *make_rng(unsigned long seed)
{
    gsl_rng *rng = gsl_rng_alloc(gsl_rng_mt19937);
    gsl_rng_set(rng, seed);
    return rng;
}


/* --- Uniform distribution --- */

START_TEST(t_uniform_two)
{
    /* Two equally weighted items -- each should get ~50% of samples. */
    std::vector<double> weights = {0.5, 0.5};
    Alias alias(weights);

    gsl_rng *rng = make_rng(42);
    size_t counts[2] = {0, 0};
    size_t n = 100000;

    for (size_t i = 0; i < n; i++) {
        size_t idx = alias.get(rng);
        ck_assert_int_lt(idx, 2);
        counts[idx]++;
    }

    /* Each should be within 2% of 50% */
    double ratio0 = (double) counts[0] / n;
    double ratio1 = (double) counts[1] / n;
    ck_assert_msg(fabs(ratio0 - 0.5) < 0.02,
        "Item 0: expected ~50%%, got %.1f%%", ratio0 * 100);
    ck_assert_msg(fabs(ratio1 - 0.5) < 0.02,
        "Item 1: expected ~50%%, got %.1f%%", ratio1 * 100);

    gsl_rng_free(rng);
}
END_TEST


START_TEST(t_uniform_five)
{
    /* Five equally weighted items -- each should get ~20%. */
    std::vector<double> weights = {0.2, 0.2, 0.2, 0.2, 0.2};
    Alias alias(weights);

    gsl_rng *rng = make_rng(123);
    std::vector<size_t> counts(5, 0);
    size_t n = 200000;

    for (size_t i = 0; i < n; i++) {
        size_t idx = alias.get(rng);
        ck_assert_int_lt(idx, 5);
        counts[idx]++;
    }

    for (size_t i = 0; i < 5; i++) {
        double ratio = (double) counts[i] / n;
        ck_assert_msg(fabs(ratio - 0.2) < 0.02,
            "Item %zu: expected ~20%%, got %.1f%%", i, ratio * 100);
    }

    gsl_rng_free(rng);
}
END_TEST


/* --- Skewed distributions --- */

START_TEST(t_heavily_skewed)
{
    /* One item has 90% weight, the other 10%. */
    std::vector<double> weights = {0.9, 0.1};
    Alias alias(weights);

    gsl_rng *rng = make_rng(7);
    size_t counts[2] = {0, 0};
    size_t n = 100000;

    for (size_t i = 0; i < n; i++) {
        size_t idx = alias.get(rng);
        ck_assert_int_lt(idx, 2);
        counts[idx]++;
    }

    double ratio0 = (double) counts[0] / n;
    double ratio1 = (double) counts[1] / n;
    ck_assert_msg(fabs(ratio0 - 0.9) < 0.02,
        "Item 0: expected ~90%%, got %.1f%%", ratio0 * 100);
    ck_assert_msg(fabs(ratio1 - 0.1) < 0.02,
        "Item 1: expected ~10%%, got %.1f%%", ratio1 * 100);

    gsl_rng_free(rng);
}
END_TEST


START_TEST(t_three_way_skew)
{
    /* 50%, 30%, 20% */
    std::vector<double> weights = {0.5, 0.3, 0.2};
    Alias alias(weights);

    gsl_rng *rng = make_rng(99);
    std::vector<size_t> counts(3, 0);
    size_t n = 200000;

    for (size_t i = 0; i < n; i++) {
        size_t idx = alias.get(rng);
        ck_assert_int_lt(idx, 3);
        counts[idx]++;
    }

    double expected[] = {0.5, 0.3, 0.2};
    for (size_t i = 0; i < 3; i++) {
        double ratio = (double) counts[i] / n;
        ck_assert_msg(fabs(ratio - expected[i]) < 0.02,
            "Item %zu: expected ~%.0f%%, got %.1f%%",
            i, expected[i] * 100, ratio * 100);
    }

    gsl_rng_free(rng);
}
END_TEST


/* --- Edge cases --- */

START_TEST(t_single_item)
{
    /* One item with weight 1.0 -- should always return index 0. */
    std::vector<double> weights = {1.0};
    Alias alias(weights);

    gsl_rng *rng = make_rng(1);

    for (size_t i = 0; i < 1000; i++) {
        ck_assert_int_eq(alias.get(rng), 0);
    }

    gsl_rng_free(rng);
}
END_TEST


START_TEST(t_degenerate_one_hot)
{
    /* One item has all the weight, others have zero. */
    std::vector<double> weights = {0.0, 0.0, 1.0, 0.0};
    Alias alias(weights);

    gsl_rng *rng = make_rng(55);

    for (size_t i = 0; i < 10000; i++) {
        ck_assert_int_eq(alias.get(rng), 2);
    }

    gsl_rng_free(rng);
}
END_TEST


START_TEST(t_two_nonzero_among_zeros)
{
    /* Only items 1 and 3 have weight; should only sample those. */
    std::vector<double> weights = {0.0, 0.6, 0.0, 0.4};
    Alias alias(weights);

    gsl_rng *rng = make_rng(88);
    size_t counts[4] = {0};
    size_t n = 100000;

    for (size_t i = 0; i < n; i++) {
        size_t idx = alias.get(rng);
        ck_assert_int_lt(idx, 4);
        counts[idx]++;
    }

    ck_assert_int_eq(counts[0], 0);
    ck_assert_int_eq(counts[2], 0);

    double ratio1 = (double) counts[1] / n;
    double ratio3 = (double) counts[3] / n;
    ck_assert_msg(fabs(ratio1 - 0.6) < 0.02,
        "Item 1: expected ~60%%, got %.1f%%", ratio1 * 100);
    ck_assert_msg(fabs(ratio3 - 0.4) < 0.02,
        "Item 3: expected ~40%%, got %.1f%%", ratio3 * 100);

    gsl_rng_free(rng);
}
END_TEST


START_TEST(t_many_items)
{
    /* 100 items with uniform weight. */
    size_t k = 100;
    std::vector<double> weights(k, 1.0 / k);
    Alias alias(weights);

    gsl_rng *rng = make_rng(314);
    std::vector<size_t> counts(k, 0);
    size_t n = 1000000;

    for (size_t i = 0; i < n; i++) {
        size_t idx = alias.get(rng);
        ck_assert_int_lt(idx, k);
        counts[idx]++;
    }

    /* Each should be within 1% of 1% (i.e., ratio in [0.005, 0.015]). */
    double expected = 1.0 / k;
    for (size_t i = 0; i < k; i++) {
        double ratio = (double) counts[i] / n;
        ck_assert_msg(fabs(ratio - expected) < 0.005,
            "Item %zu: expected ~%.2f%%, got %.2f%%",
            i, expected * 100, ratio * 100);
    }

    gsl_rng_free(rng);
}
END_TEST


/* --- Reproducibility --- */

START_TEST(t_deterministic_with_same_seed)
{
    std::vector<double> weights = {0.3, 0.5, 0.2};
    Alias alias(weights);

    gsl_rng *rng1 = make_rng(42);
    gsl_rng *rng2 = make_rng(42);

    for (size_t i = 0; i < 10000; i++) {
        ck_assert_int_eq(alias.get(rng1), alias.get(rng2));
    }

    gsl_rng_free(rng1);
    gsl_rng_free(rng2);
}
END_TEST


START_TEST(t_different_seeds_differ)
{
    std::vector<double> weights = {0.3, 0.5, 0.2};
    Alias alias(weights);

    gsl_rng *rng1 = make_rng(1);
    gsl_rng *rng2 = make_rng(999);

    /* With different seeds, at least some samples should differ. */
    size_t differ = 0;
    size_t n = 1000;
    for (size_t i = 0; i < n; i++) {
        if (alias.get(rng1) != alias.get(rng2)) differ++;
    }

    ck_assert_int_gt(differ, 0);

    gsl_rng_free(rng1);
    gsl_rng_free(rng2);
}
END_TEST


/* --- Index bounds --- */

START_TEST(t_indices_in_range)
{
    /* Verify that get() never returns an out-of-range index. */
    for (size_t k = 1; k <= 20; k++) {
        std::vector<double> weights(k, 1.0 / k);
        Alias alias(weights);

        gsl_rng *rng = make_rng(k);
        for (size_t i = 0; i < 10000; i++) {
            size_t idx = alias.get(rng);
            ck_assert_int_lt(idx, k);
        }
        gsl_rng_free(rng);
    }
}
END_TEST


Suite *unit_testing()
{
    Suite *unit = suite_create("Alias Unit Testing");

    TCase *uniform = tcase_create("Uniform Distributions");
    tcase_add_test(uniform, t_uniform_two);
    tcase_add_test(uniform, t_uniform_five);
    suite_add_tcase(unit, uniform);

    TCase *skewed = tcase_create("Skewed Distributions");
    tcase_add_test(skewed, t_heavily_skewed);
    tcase_add_test(skewed, t_three_way_skew);
    suite_add_tcase(unit, skewed);

    TCase *edge = tcase_create("Edge Cases");
    tcase_add_test(edge, t_single_item);
    tcase_add_test(edge, t_degenerate_one_hot);
    tcase_add_test(edge, t_two_nonzero_among_zeros);
    tcase_add_test(edge, t_many_items);
    suite_add_tcase(unit, edge);

    TCase *repro = tcase_create("Reproducibility");
    tcase_add_test(repro, t_deterministic_with_same_seed);
    tcase_add_test(repro, t_different_seeds_differ);
    suite_add_tcase(unit, repro);

    TCase *bounds = tcase_create("Index Bounds");
    tcase_add_test(bounds, t_indices_in_range);
    suite_add_tcase(unit, bounds);

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
