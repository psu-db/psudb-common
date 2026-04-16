/*
 * tests/bitarray_check_tests.cpp
 *
 * Unit tests for psudb::BitArray using check
 */

#include <cstring>

#include "psu-ds/BitArray.h"

#include <check.h>

using namespace psudb;


/* --- Construction --- */

START_TEST(t_construct_small)
{
    BitArray arr(2);
    ck_assert_int_eq(arr.size(), 2);
    ck_assert_int_ge(arr.memory_usage(), 1);

    /* All bits initially unset */
    ck_assert(!arr.is_set(0));
    ck_assert(!arr.is_set(1));
}
END_TEST


START_TEST(t_construct_zero)
{
    BitArray arr(0);
    ck_assert_int_eq(arr.size(), 0);
    ck_assert_int_eq(arr.memory_usage(), 0);
}
END_TEST


START_TEST(t_construct_one)
{
    BitArray arr(1);
    ck_assert_int_eq(arr.size(), 1);
    ck_assert(!arr.is_set(0));
    arr.set(0);
    ck_assert(arr.is_set(0));
}
END_TEST


START_TEST(t_construct_byte_boundary)
{
    /* Exactly 8 bits -- one byte worth */
    BitArray arr(8);
    ck_assert_int_eq(arr.size(), 8);

    for (size_t i = 0; i < 8; i++) {
        ck_assert(!arr.is_set(i));
    }
}
END_TEST


START_TEST(t_construct_large)
{
    BitArray arr(10000);
    ck_assert_int_eq(arr.size(), 10000);

    /* Spot check that all bits start unset */
    for (size_t i = 0; i < 10000; i += 100) {
        ck_assert(!arr.is_set(i));
    }
}
END_TEST


/* --- Set / Unset / is_set --- */

START_TEST(t_set_unset_basic)
{
    BitArray arr(16);

    ck_assert_int_eq(arr.set(0), 1);
    ck_assert(arr.is_set(0));
    ck_assert(!arr.is_set(1));

    ck_assert_int_eq(arr.set(15), 1);
    ck_assert(arr.is_set(15));

    ck_assert_int_eq(arr.unset(0), 1);
    ck_assert(!arr.is_set(0));
    ck_assert(arr.is_set(15));
}
END_TEST


START_TEST(t_set_all_bits)
{
    size_t n = 64;
    BitArray arr(n);

    for (size_t i = 0; i < n; i++) {
        ck_assert_int_eq(arr.set(i), 1);
    }

    for (size_t i = 0; i < n; i++) {
        ck_assert(arr.is_set(i));
    }
}
END_TEST


START_TEST(t_unset_all_bits)
{
    size_t n = 64;
    BitArray arr(n);

    for (size_t i = 0; i < n; i++) arr.set(i);
    for (size_t i = 0; i < n; i++) arr.unset(i);

    for (size_t i = 0; i < n; i++) {
        ck_assert(!arr.is_set(i));
    }
}
END_TEST


START_TEST(t_set_idempotent)
{
    BitArray arr(8);
    arr.set(3);
    arr.set(3);
    ck_assert(arr.is_set(3));

    arr.unset(3);
    ck_assert(!arr.is_set(3));
}
END_TEST


START_TEST(t_unset_already_unset)
{
    BitArray arr(8);
    ck_assert_int_eq(arr.unset(3), 1);
    ck_assert(!arr.is_set(3));
}
END_TEST


START_TEST(t_alternating_bits)
{
    size_t n = 128;
    BitArray arr(n);

    /* Set only even bits */
    for (size_t i = 0; i < n; i += 2) arr.set(i);

    for (size_t i = 0; i < n; i++) {
        if (i % 2 == 0) {
            ck_assert(arr.is_set(i));
        } else {
            ck_assert(!arr.is_set(i));
        }
    }
}
END_TEST


START_TEST(t_bit_within_same_byte)
{
    /* Verify that setting one bit doesn't corrupt neighbors in the same byte */
    BitArray arr(8);

    arr.set(3);
    for (size_t i = 0; i < 8; i++) {
        if (i == 3) {
            ck_assert(arr.is_set(i));
        } else {
            ck_assert(!arr.is_set(i));
        }
    }

    arr.set(5);
    ck_assert(arr.is_set(3));
    ck_assert(arr.is_set(5));
    ck_assert(!arr.is_set(4));

    arr.unset(3);
    ck_assert(!arr.is_set(3));
    ck_assert(arr.is_set(5));
}
END_TEST


/* --- Non-byte-aligned sizes (regression for byte count bug) --- */

START_TEST(t_nine_bits)
{
    /* 9 bits requires 2 bytes. The old formula (m_bits >> 3) gave 1 byte,
     * which was only saved by the allocator padding to CACHELINE_SIZE. */
    BitArray arr(9);
    ck_assert_int_eq(arr.size(), 9);

    for (size_t i = 0; i < 9; i++) {
        ck_assert_int_eq(arr.set(i), 1);
    }
    for (size_t i = 0; i < 9; i++) {
        ck_assert(arr.is_set(i));
    }

    /* Bit 8 (the 9th bit) is in the second byte -- this was the
     * bit most likely to be corrupted by the old bug. */
    arr.clear();
    arr.set(8);
    ck_assert(arr.is_set(8));
    ck_assert(!arr.is_set(0));
}
END_TEST


START_TEST(t_fifteen_bits)
{
    BitArray arr(15);
    ck_assert_int_eq(arr.size(), 15);

    arr.set(14);
    ck_assert(arr.is_set(14));
    ck_assert(!arr.is_set(13));
}
END_TEST


START_TEST(t_seventeen_bits)
{
    /* 17 bits = 3 bytes needed */
    BitArray arr(17);
    ck_assert_int_eq(arr.size(), 17);

    arr.set(16);
    ck_assert(arr.is_set(16));

    /* Set all and verify */
    for (size_t i = 0; i < 17; i++) arr.set(i);
    for (size_t i = 0; i < 17; i++) ck_assert(arr.is_set(i));
}
END_TEST


START_TEST(t_65_bits)
{
    /* Just past a cacheline boundary (64 bytes = 512 bits), 65 bits = 9 bytes */
    BitArray arr(65);
    ck_assert_int_eq(arr.size(), 65);

    arr.set(64);
    ck_assert(arr.is_set(64));
    ck_assert(!arr.is_set(63));
}
END_TEST


/* --- Out of bounds --- */

START_TEST(t_out_of_bounds_is_set)
{
    BitArray arr(8);
    ck_assert(!arr.is_set(8));
    ck_assert(!arr.is_set(100));
    ck_assert(!arr.is_set((size_t)-1));
}
END_TEST


START_TEST(t_out_of_bounds_set)
{
    BitArray arr(8);
    ck_assert_int_eq(arr.set(8), 0);
    ck_assert_int_eq(arr.set(100), 0);
}
END_TEST


START_TEST(t_out_of_bounds_unset)
{
    BitArray arr(8);
    ck_assert_int_eq(arr.unset(8), 0);
    ck_assert_int_eq(arr.unset(100), 0);
}
END_TEST


/* --- Clear --- */

START_TEST(t_clear)
{
    size_t n = 256;
    BitArray arr(n);

    for (size_t i = 0; i < n; i++) arr.set(i);
    arr.clear();

    for (size_t i = 0; i < n; i++) {
        ck_assert(!arr.is_set(i));
    }
}
END_TEST


START_TEST(t_clear_then_reuse)
{
    BitArray arr(16);
    arr.set(5);
    arr.set(10);
    arr.clear();

    /* Set different bits after clear */
    arr.set(1);
    arr.set(14);

    ck_assert(!arr.is_set(5));
    ck_assert(!arr.is_set(10));
    ck_assert(arr.is_set(1));
    ck_assert(arr.is_set(14));
}
END_TEST


/* --- Copy semantics --- */

START_TEST(t_copy_construct)
{
    BitArray orig(64);
    orig.set(0);
    orig.set(31);
    orig.set(63);

    BitArray copy(orig);

    ck_assert_int_eq(copy.size(), 64);
    ck_assert(copy.is_set(0));
    ck_assert(copy.is_set(31));
    ck_assert(copy.is_set(63));
    ck_assert(!copy.is_set(1));

    /* Modifying copy does not affect original */
    copy.unset(0);
    ck_assert(!copy.is_set(0));
    ck_assert(orig.is_set(0));
}
END_TEST


START_TEST(t_copy_assign)
{
    BitArray a(32);
    a.set(10);

    BitArray b(16);
    b.set(5);

    b = a;

    ck_assert_int_eq(b.size(), 32);
    ck_assert(b.is_set(10));
    ck_assert(!b.is_set(5));

    /* Independent */
    b.unset(10);
    ck_assert(a.is_set(10));
}
END_TEST


/* --- Move semantics --- */

START_TEST(t_move_construct)
{
    BitArray orig(64);
    orig.set(7);
    orig.set(42);

    BitArray moved(std::move(orig));

    ck_assert_int_eq(moved.size(), 64);
    ck_assert(moved.is_set(7));
    ck_assert(moved.is_set(42));
}
END_TEST


START_TEST(t_move_assign)
{
    BitArray a(32);
    a.set(10);

    BitArray b(16);
    b = std::move(a);

    ck_assert_int_eq(b.size(), 32);
    ck_assert(b.is_set(10));
}
END_TEST


/* --- Swap --- */

START_TEST(t_swap)
{
    BitArray a(32);
    a.set(5);

    BitArray b(64);
    b.set(50);

    a.swap(b);

    ck_assert_int_eq(a.size(), 64);
    ck_assert(a.is_set(50));
    ck_assert(!a.is_set(5));

    ck_assert_int_eq(b.size(), 32);
    ck_assert(b.is_set(5));
    ck_assert(!b.is_set(50));
}
END_TEST


Suite *unit_testing()
{
    Suite *unit = suite_create("BitArray Unit Testing");

    TCase *construct = tcase_create("Construction");
    tcase_add_test(construct, t_construct_small);
    tcase_add_test(construct, t_construct_zero);
    tcase_add_test(construct, t_construct_one);
    tcase_add_test(construct, t_construct_byte_boundary);
    tcase_add_test(construct, t_construct_large);
    suite_add_tcase(unit, construct);

    TCase *setunset = tcase_create("Set/Unset/is_set");
    tcase_add_test(setunset, t_set_unset_basic);
    tcase_add_test(setunset, t_set_all_bits);
    tcase_add_test(setunset, t_unset_all_bits);
    tcase_add_test(setunset, t_set_idempotent);
    tcase_add_test(setunset, t_unset_already_unset);
    tcase_add_test(setunset, t_alternating_bits);
    tcase_add_test(setunset, t_bit_within_same_byte);
    suite_add_tcase(unit, setunset);

    TCase *nonaligned = tcase_create("Non-byte-aligned Sizes");
    tcase_add_test(nonaligned, t_nine_bits);
    tcase_add_test(nonaligned, t_fifteen_bits);
    tcase_add_test(nonaligned, t_seventeen_bits);
    tcase_add_test(nonaligned, t_65_bits);
    suite_add_tcase(unit, nonaligned);

    TCase *bounds = tcase_create("Out of Bounds");
    tcase_add_test(bounds, t_out_of_bounds_is_set);
    tcase_add_test(bounds, t_out_of_bounds_set);
    tcase_add_test(bounds, t_out_of_bounds_unset);
    suite_add_tcase(unit, bounds);

    TCase *clr = tcase_create("Clear");
    tcase_add_test(clr, t_clear);
    tcase_add_test(clr, t_clear_then_reuse);
    suite_add_tcase(unit, clr);

    TCase *copy = tcase_create("Copy Semantics");
    tcase_add_test(copy, t_copy_construct);
    tcase_add_test(copy, t_copy_assign);
    suite_add_tcase(unit, copy);

    TCase *move = tcase_create("Move Semantics");
    tcase_add_test(move, t_move_construct);
    tcase_add_test(move, t_move_assign);
    suite_add_tcase(unit, move);

    TCase *swp = tcase_create("Swap");
    tcase_add_test(swp, t_swap);
    suite_add_tcase(unit, swp);

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
