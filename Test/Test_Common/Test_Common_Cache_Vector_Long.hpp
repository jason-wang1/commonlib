#pragma once
#include "gtest/gtest.h"
#include "Common/CommonCache.h"

class RedisProtoData_VectorCache_Long : public testing::Test
{
public:
    using Value = long;

    // 整体test执行前执行
    static void SetUpTestSuite()
    {
    }

    // 整体test执行完毕后执行
    static void TearDownTestSuite()
    {
    }

    // 单个Test执行前执行
    virtual void SetUp() override
    {
    }

    // 单个Test执行后执行
    virtual void TearDown() override
    {
    }
};

// 1. SetCacheData
// 测试路径: 将数据加入到缓存
// 测试条件1: value值不为空
// 测试条件2: 获取缓存值内容一致
TEST_F(RedisProtoData_VectorCache_Long, SetCacheData)
{
    int idx = 0;
    Value value = 12345;

    VectorCache<Value> cache;
    {
        std::vector<Value> tmpCache;
        tmpCache.push_back(value);
        cache.SetCacheData(tmpCache);
        ASSERT_EQ(tmpCache.empty(), false);
    }

    Value get_value = 0;
    bool ret = cache.GetCacheData(idx, get_value);
    ASSERT_EQ(ret, true);

    bool equal = (get_value == value);
    if (!equal)
    {
        std::cout << "[   INFO   ] "
                  << "get_value = " << get_value << std::endl;
    }
    ASSERT_EQ(equal, true);
}

// 2. MoveSetCacheData
// 测试路径: 将数据加入到缓存
// 测试条件1: value值为空
// 测试条件2: 获取缓存值内容一致
TEST_F(RedisProtoData_VectorCache_Long, MoveSetCacheData)
{
    int idx = 0;
    Value value = 12345;

    VectorCache<Value> cache;
    {
        std::vector<Value> tmpCache;
        tmpCache.push_back(value);
        cache.SetCacheData(std::move(tmpCache));
        ASSERT_EQ(tmpCache.empty(), true);
    }

    Value get_value = 0;
    bool ret = cache.GetCacheData(idx, get_value);
    ASSERT_EQ(ret, true);

    bool equal = (get_value == value);
    if (!equal)
    {
        std::cout << "[   INFO   ] "
                  << "get_value = " << get_value << std::endl;
    }
    ASSERT_EQ(equal, true);
}

// 3. AppendCacheData
// 测试路径: 将数据加入到缓存
// 测试条件1: value值为空
// 测试条件2: 获取缓存值内容一致
TEST_F(RedisProtoData_VectorCache_Long, AppendCacheData)
{
    int idx1 = 1;
    Value value0 = 12345;
    Value value1 = 123456;

    VectorCache<Value> cache;
    int cacheSize_before = cache.GetCacheSize();
    {
        std::vector<Value> tmpCache;
        tmpCache.push_back(value0);
        cache.AppendCacheData(tmpCache);
        ASSERT_EQ(tmpCache.empty(), false);
    }

    {
        std::vector<Value> tmpCache;
        tmpCache.push_back(value1);
        cache.AppendCacheData(std::move(tmpCache));
        ASSERT_EQ(tmpCache.empty(), true);
    }

    int cacheSzie_after = cache.GetCacheSize();
    ASSERT_EQ(cacheSzie_after, cacheSize_before + 2);

    Value get_value = 0;
    bool ret = cache.GetCacheData(idx1, get_value);
    ASSERT_EQ(ret, true);

    bool equal = (get_value == value1);
    if (!equal)
    {
        std::cout << "[   INFO   ] "
                  << "get_value = " << get_value << std::endl;
    }
    ASSERT_EQ(equal, true);
}
