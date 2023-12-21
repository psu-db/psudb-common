//
//
// Created by Max Norfolk on 12/21/23.
#include <gtest/gtest.h>
#include <cstdlib>

#include "dynarray.h"

TEST(DynamicArrayTest, RandomInts) {
    srand(0);
    constexpr size_t SIZE = 100;

    std::array<int, SIZE> std_arr{};

    psudb::DynamicArray<int> psu_arr(SIZE);

    for (int i = 0; i < SIZE; ++i) {
        int x = rand();
        std_arr.at(i) = x;
        psu_arr.at(i) = x;
    }

    ASSERT_EQ(std_arr.size(), psu_arr.size());
    ASSERT_EQ(std_arr.front(), psu_arr.front());
    ASSERT_EQ(std_arr.back(), psu_arr.back());

    for (int i = 0; i < SIZE; ++i) {
        ASSERT_EQ(psu_arr[i], std_arr[i]);
    }

    auto p_it = psu_arr.begin();
    auto s_it = std_arr.begin();
    while (true) {
        ASSERT_EQ(*p_it, *s_it);
        p_it++;
        s_it++;
        bool b1 = p_it == psu_arr.end();
        bool b2 = s_it == std_arr.end();
        ASSERT_EQ(b1, b2);
        if (b1 || b2)
            break;
    }
}