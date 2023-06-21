#pragma once
#include "gtest/gtest.h"
#include "Common/ARCCache.h"

TEST(ARCCacheTest, SimplePut)
{
    Common::ARCCache<int, int> cache_arc(1);
    cache_arc.set(7, 777);
    EXPECT_TRUE(cache_arc.exists(7));

    int val;
    long expire;
    EXPECT_EQ(true, cache_arc.get(7, val, expire));
    EXPECT_EQ(777, val);
    EXPECT_EQ(std::size_t(1), cache_arc.size());
}

TEST(ARCCacheTest, MissingValue)
{
    Common::ARCCache<int, int> cache_arc(1);
    int val;
    long expire;
    EXPECT_EQ(false, cache_arc.get(7, val, expire));
}

TEST(ARCCacheTest, TimeExpire)
{
    Common::ARCCache<int, int> cache_arc(1);
    cache_arc.set(7, 777, Common::get_ms_timestamp());

    int val;
    long expire;
    EXPECT_EQ(false, cache_arc.get(7, val, expire));
}

TEST(ARCCacheTest, KeepsAllValuesWithinCapacity)
{
    // constexpr int NUM_OF_TEST1_RECORDS = 100;
    constexpr int NUM_OF_TEST2_RECORDS = 100;
    constexpr int TEST2_CACHE_CAPACITY = 50;

    Common::ARCCache<int, int> cache_arc(TEST2_CACHE_CAPACITY);

    for (int i = 0; i < NUM_OF_TEST2_RECORDS; ++i)
    {
        cache_arc.set(i, i);
    }

    for (int i = 0; i < NUM_OF_TEST2_RECORDS - TEST2_CACHE_CAPACITY; ++i)
    {
        EXPECT_FALSE(cache_arc.exists(i));
    }

    for (int i = NUM_OF_TEST2_RECORDS - TEST2_CACHE_CAPACITY; i < NUM_OF_TEST2_RECORDS; ++i)
    {
        EXPECT_TRUE(cache_arc.exists(i));

        int val;
        long expire;
        EXPECT_EQ(true, cache_arc.get(i, val, expire));
        EXPECT_EQ(i, val);
    }

    EXPECT_EQ(std::size_t(TEST2_CACHE_CAPACITY), cache_arc.size());
}
