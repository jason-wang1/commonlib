#pragma once
#include "gtest/gtest.h"
#include "Common/LFUCache.h"

TEST(LFUCacheTest, SimplePut)
{
    Common::LFUCache<int, int> cache_lfu(1);
    cache_lfu.set(7, 777);
    EXPECT_TRUE(cache_lfu.exists(7));

    int val;
    long expire;
    EXPECT_EQ(true, cache_lfu.get(7, val, expire));
    EXPECT_EQ(777, val);
    EXPECT_EQ(std::size_t(1), cache_lfu.size());
}

TEST(LFUCacheTest, MissingValue)
{
    Common::LFUCache<int, int> cache_lfu(1);
    int val;
    long expire;
    EXPECT_EQ(false, cache_lfu.get(7, val, expire));
}

TEST(LFUCacheTest, TimeExpire)
{
    Common::LFUCache<int, int> cache_lfu(1);
    cache_lfu.set(7, 777, Common::get_ms_timestamp());

    int val;
    long expire;
    EXPECT_EQ(false, cache_lfu.get(7, val, expire));
}

TEST(LFUCacheTest, KeepsAllValuesWithinCapacity)
{
    // constexpr int NUM_OF_TEST1_RECORDS = 100;
    constexpr int NUM_OF_TEST2_RECORDS = 100;
    constexpr int TEST2_CACHE_CAPACITY = 50;

    Common::LFUCache<int, int> cache_lfu(TEST2_CACHE_CAPACITY);

    for (int i = 0; i < NUM_OF_TEST2_RECORDS; ++i)
    {
        cache_lfu.set(i, i);
    }

    for (int i = 0; i < NUM_OF_TEST2_RECORDS - TEST2_CACHE_CAPACITY; ++i)
    {
        EXPECT_FALSE(cache_lfu.exists(i));
    }

    for (int i = NUM_OF_TEST2_RECORDS - TEST2_CACHE_CAPACITY; i < NUM_OF_TEST2_RECORDS; ++i)
    {
        EXPECT_TRUE(cache_lfu.exists(i));

        int val;
        long expire;
        EXPECT_EQ(true, cache_lfu.get(i, val, expire));
        EXPECT_EQ(i, val);
    }

    EXPECT_EQ(std::size_t(TEST2_CACHE_CAPACITY), cache_lfu.size());
}
