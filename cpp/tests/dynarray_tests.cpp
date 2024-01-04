//
//
// Created by Max Norfolk on 12/21/23.
#include <gtest/gtest.h>
#include <cstdlib>

#include "psu-ds/dynarray.h"

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

TEST(DynamicArrayTest, MoveTest) {
    psudb::DynamicArray<int> x{10, 20};
    ASSERT_EQ(x.size(), 2);
    ASSERT_EQ(x[0], 10);
    ASSERT_EQ(x[1], 20);

    // move assignment operator
    psudb::DynamicArray<int> y;
    ASSERT_EQ(y.size(), 0);
    y = std::move(x);
    ASSERT_EQ(y.size(), 2);
    ASSERT_EQ(y[0], 10);
    ASSERT_EQ(y[1], 20);

    // move constructor
    psudb::DynamicArray<int> z = std::move(y);
    ASSERT_EQ(z.size(), 2);
    ASSERT_EQ(z[0], 10);
    ASSERT_EQ(z[1], 20);
}

TEST(DynamicArrayTest, CopyTests) {
    psudb::DynamicArray<double> x = {0.3, 10};
    psudb::DynamicArray<double> y = {60, 10, 9.3};
    {
        auto z = y;
        ASSERT_EQ(z.size(), y.size());

        for (int i = 0; i < z.size(); ++i) {
            ASSERT_EQ(z[0], y[0]);
        }

        z[0] = z[0] * 2;
        ASSERT_NE(z[0], y[0]);
    } // deconstructor of z

    ASSERT_NE(x.size(), y.size());
    x = y;
    ASSERT_EQ(x.size(), y.size());
    for (int i = 0; i < x.size(); ++i) {
        ASSERT_EQ(x[0], y[0]);
    }
    x[0] = x[0] * 2;
    ASSERT_NE(x[0], y[0]);
}