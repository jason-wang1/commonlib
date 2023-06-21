#pragma once
#include "gtest/gtest.h"
#include "Common/CommonCache.h"

class RedisProtoData_SingleCache : public testing::Test
{
public:
    using Value = std::string;

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
TEST_F(RedisProtoData_SingleCache, SetCacheData)
{
    Value value = "proto_12345";

    SingleCache<Value> cache;
    cache.SetCacheData(value);
    ASSERT_EQ(value.empty(), false);

    Value get_value;
    bool ret = cache.GetCacheData(get_value);
    ASSERT_EQ(ret, true);

    bool equal = (get_value == value);
    if (!equal)
    {
        std::cout << "[   INFO   ] "
                  << "get_value = " << get_value << std::endl;
    }
    ASSERT_EQ(equal, true);
}

// 1. MoveSetCacheData
// 测试路径: 将数据加入到缓存
// 测试条件1: value值为空
// 测试条件2: 获取缓存值内容一致
TEST_F(RedisProtoData_SingleCache, MoveSetCacheData)
{
    Value value = "proto_12345";

    SingleCache<Value> cache;
    {
        Value tmpValue = value;
        cache.SetCacheData(std::move(tmpValue));
        ASSERT_EQ(tmpValue.empty(), true);
    }

    Value get_value;
    bool ret = cache.GetCacheData(get_value);
    ASSERT_EQ(ret, true);

    bool equal = (get_value == value);
    if (!equal)
    {
        std::cout << "[   INFO   ] "
                  << "get_value = " << get_value
                  << ", value = " << value
                  << std::endl;
    }
    ASSERT_EQ(equal, true);
}
