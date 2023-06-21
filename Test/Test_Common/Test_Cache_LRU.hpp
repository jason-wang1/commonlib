#pragma once
#include "gtest/gtest.h"
#include "Common/LRUCache.h"

TEST(LRUCacheTest, SimplePut)
{
    Common::LRUCache<int, int> cache_lru(1);
    cache_lru.set(7, 777);
    EXPECT_TRUE(cache_lru.exists(7));

    int val;
    long expire;
    EXPECT_EQ(true, cache_lru.get(7, val, expire));
    EXPECT_EQ(777, val);
    EXPECT_EQ(std::size_t(1), cache_lru.size());
}

TEST(LRUCacheTest, MissingValue)
{
    Common::LRUCache<int, int> cache_lru(1);
    int val;
    long expire;
    EXPECT_EQ(false, cache_lru.get(7, val, expire));
}

TEST(LRUCacheTest, TimeExpire)
{
    Common::LRUCache<int, int> cache_lru(1);
    cache_lru.set(7, 777, Common::get_ms_timestamp());

    int val;
    long expire;
    EXPECT_EQ(false, cache_lru.get(7, val, expire));
}

TEST(LRUCacheTest, KeepsAllValuesWithinCapacity)
{
    // constexpr int NUM_OF_TEST1_RECORDS = 100;
    constexpr int NUM_OF_TEST2_RECORDS = 100;
    constexpr int TEST2_CACHE_CAPACITY = 50;

    Common::LRUCache<int, int> cache_lru(TEST2_CACHE_CAPACITY);

    for (int i = 0; i < NUM_OF_TEST2_RECORDS; ++i)
    {
        cache_lru.set(i, i);
    }

    for (int i = 0; i < NUM_OF_TEST2_RECORDS - TEST2_CACHE_CAPACITY; ++i)
    {
        EXPECT_FALSE(cache_lru.exists(i));
    }

    for (int i = NUM_OF_TEST2_RECORDS - TEST2_CACHE_CAPACITY; i < NUM_OF_TEST2_RECORDS; ++i)
    {
        EXPECT_TRUE(cache_lru.exists(i));

        int val;
        long expire;
        EXPECT_EQ(true, cache_lru.get(i, val, expire));
        EXPECT_EQ(i, val);
    }

    EXPECT_EQ(std::size_t(TEST2_CACHE_CAPACITY), cache_lru.size());
}
