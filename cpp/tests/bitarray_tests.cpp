// Some tests for the bit array
//
// Created by Max Norfolk on 1/4/24.

#include <gtest/gtest.h>

#include "psu-ds/BitArray.h"


TEST(BitArrayTest, SmallBitArray) {
    psudb::BitArray arr{2};
    ASSERT_EQ(arr.size(),2);
    ASSERT_EQ(arr.memory_usage(), psudb::CACHELINE_SIZE); // 1 byte for 2 bits

    ASSERT_FALSE(arr.is_set(0));
    ASSERT_FALSE(arr.is_set(1));
    arr.set(0);
    ASSERT_TRUE(arr.is_set(0));
    ASSERT_FALSE(arr.is_set(1));
    arr.set(1);
    ASSERT_TRUE(arr.is_set(0));
    ASSERT_TRUE(arr.is_set(1));
    arr.unset(0);
    ASSERT_FALSE(arr.is_set(0));
    ASSERT_TRUE(arr.is_set(1));
}