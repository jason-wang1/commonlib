#pragma once
#include "gtest/gtest.h"
#include "Common/CommonCache.h"

// link: https://mini1.feishu.cn/sheets/shtcnNdVy9j3FwtYGKowEmqUdRe
class RedisProtoData_HashCache : public testing::Test
{
public:
    using Key = long;
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
    virtual void SetUp()
    {
    }

    // 单个Test执行后执行
    virtual void TearDown()
    {
    }
};

// 1. SetDBuf
// 测试路径: 将数据加入到DBuf
// 测试条件1: 设置完后, 临时数据不为空
// 测试条件2: DBuf数量+1
// 测试条件3: RWBuf数量不变
// 测试条件4: 获取缓存值内容一致
TEST_F(RedisProtoData_HashCache, SetDBuf)
{
    Key key = 12345;
    Value value = "proto_12345";

    HashCache<Key, Value> cache;
    int rwbufSize_before = cache.GetRWBufSize();
    int dbufSize_before = cache.GetDBufSize();
    {
        std::unordered_map<Key, Value> mapData1;
        mapData1[key] = value;
        cache.AddDBufCacheData(mapData1);
        ASSERT_EQ(mapData1.empty(), false);
    }

    int dbufSize_after = cache.GetDBufSize();
    ASSERT_EQ(dbufSize_after, dbufSize_before + 1);

    int rwbufSize_after = cache.GetRWBufSize();
    ASSERT_EQ(rwbufSize_after, rwbufSize_before);

    Value get_value;
    bool ret = cache.GetCacheData(key, get_value);
    ASSERT_EQ(ret, true);

    bool equal = (get_value == value);
    if (!equal)
    {
        std::cout << "[   INFO   ] "
                  << "get_value = " << get_value << std::endl;
    }
    ASSERT_EQ(equal, true);
}

// 2. MoveSetDBuf
// 测试路径: 通过Move语义, 将数据加入到DBuf
// 测试条件1: 设置完后, 临时数据为空
// 测试条件2: DBuf数量+1
// 测试条件3: RWBuf数量不变
// 测试条件4: 获取缓存值内容一致
TEST_F(RedisProtoData_HashCache, MoveSetDBuf)
{
    Key key = 12345;
    Value value = "proto_12345";

    HashCache<Key, Value> cache;
    int rwbufSize_before = cache.GetRWBufSize();
    int dbufSize_before = cache.GetDBufSize();
    {
        std::unordered_map<Key, Value> mapData1;
        mapData1[12345] = "proto_12345";
        cache.AddDBufCacheData(std::move(mapData1));
        ASSERT_EQ(mapData1.empty(), true);
    }

    int dbufSize_after = cache.GetDBufSize();
    ASSERT_EQ(dbufSize_after, dbufSize_before + 1);

    int rwbufSize_after = cache.GetRWBufSize();
    ASSERT_EQ(rwbufSize_after, rwbufSize_before);

    Value get_value;
    bool ret = cache.GetCacheData(key, get_value);
    ASSERT_EQ(ret, true);

    bool equal = (get_value == value);
    if (!equal)
    {
        std::cout << "[   INFO   ] "
                  << "get_value = " << get_value << std::endl;
    }
    ASSERT_EQ(equal, true);
}

// 3. SetRWBuf
// 测试路径: 将数据加入到RWBuf
// 测试条件1: 设置完后, 临时数据不为空
// 测试条件2: RWBuf数量+1
// 测试条件3: DBuf数量不变
// 测试条件4: 获取缓存值内容一致
TEST_F(RedisProtoData_HashCache, SetRWBuf)
{
    Key key = 12345;
    Value value = "proto_12345";

    HashCache<Key, Value> cache;
    int rwbufSize_before = cache.GetRWBufSize();
    int dbufSize_before = cache.GetDBufSize();
    {
        std::unordered_map<Key, Value> mapData1;
        mapData1[12345] = "proto_12345";
        cache.AddRWBufCacheData(mapData1);
        ASSERT_EQ(mapData1.empty(), false);
    }
    int rwbufSize_after = cache.GetRWBufSize();
    ASSERT_EQ(rwbufSize_after, rwbufSize_before + 1);

    int dbufSize_after = cache.GetDBufSize();
    ASSERT_EQ(dbufSize_before, dbufSize_after);

    Value get_value;
    bool ret = cache.GetCacheData(key, get_value);
    ASSERT_EQ(ret, true);

    bool equal = (get_value == value);
    if (!equal)
    {
        std::cout << "[   INFO   ] "
                  << "get_value = " << get_value << std::endl;
    }
    ASSERT_EQ(equal, true);
}

// 4. MoveSetRWBuf
// 测试路径: 通过Move语义, 将数据加入到RWBuf
// 测试条件1: 设置完后, 临时数据不为空
// 测试条件2: RWBuf数量+1
// 测试条件3: DBuf数量不变
// 测试条件4: 获取缓存值内容一致
TEST_F(RedisProtoData_HashCache, MoveSetRWBuf)
{
    Key key = 12345;
    Value value = "proto_12345";

    HashCache<Key, Value> cache;
    int rwbufSize_before = cache.GetRWBufSize();
    int dbufSize_before = cache.GetDBufSize();
    {
        std::unordered_map<Key, Value> mapData1;
        mapData1[12345] = "proto_12345";
        cache.AddRWBufCacheData(std::move(mapData1));
        ASSERT_EQ(mapData1.empty(), true);
    }
    int rwbufSize_after = cache.GetRWBufSize();
    ASSERT_EQ(rwbufSize_after, rwbufSize_before + 1);

    int dbufSize_after = cache.GetDBufSize();
    ASSERT_EQ(dbufSize_before, dbufSize_after);

    Value get_value;
    bool ret = cache.GetCacheData(key, get_value);
    ASSERT_EQ(ret, true);

    bool equal = (get_value == value);
    if (!equal)
    {
        std::cout << "[   INFO   ] "
                  << "get_value = " << get_value << std::endl;
    }
    ASSERT_EQ(equal, true);
}

// 5. AddExistDBuf
// 测试路径: 将数据加入到DBuf, 然后将数据加入到DBuf, 两次加入Key一致
// 测试条件1: 第一次加入DBuf数量加1
// 测试条件2: 第二次加入DBuf数量不变
// 测试条件3: 获取缓存值与第二次加入值一致
TEST_F(RedisProtoData_HashCache, AddExistDBuf)
{
    Key key = 12345;
    Value value1 = "proto_12345";
    Value value2 = "proto_123456";

    HashCache<Key, Value> cache;
    int dbufSize_before = cache.GetDBufSize();
    {
        std::unordered_map<Key, Value> mapData1;
        mapData1[key] = value1;
        cache.AddDBufCacheData(mapData1);
    }
    int dbufSize_after1 = cache.GetDBufSize();
    ASSERT_EQ(dbufSize_after1, dbufSize_before + 1);

    {
        std::unordered_map<Key, Value> mapData2;
        mapData2[key] = value2;
        cache.AddDBufCacheData(mapData2);
    }
    int dbufSize_after2 = cache.GetDBufSize();
    ASSERT_EQ(dbufSize_after2, dbufSize_after1);

    Value get_value;
    bool ret = cache.GetCacheData(key, get_value);
    ASSERT_EQ(ret, true);

    bool equal = (get_value == value2);
    if (!equal)
    {
        std::cout << "[   INFO   ] "
                  << "get_value = " << get_value << std::endl;
    }
    ASSERT_EQ(equal, true);
}

// 6. AddNotExistDBuf
// 测试路径: 将数据加入到DBuf, 然后将数据加入到DBuf, 两次加入Key不一致
// 测试条件1: 第一次加入DBuf数量加1
// 测试条件2: 第二次加入DBuf数量加1
// 测试条件3: 获取缓存值一致
TEST_F(RedisProtoData_HashCache, AddNotExistDBuf)
{
    Key key1 = 12345;
    Key key2 = 123456;
    Value value1 = "proto_12345";
    Value value2 = "proto_123456";

    HashCache<Key, Value> cache;
    int dbufSize_before = cache.GetDBufSize();
    {
        std::unordered_map<Key, Value> mapData1;
        mapData1[key1] = value1;
        cache.AddDBufCacheData(mapData1);
    }
    int dbufSize_after1 = cache.GetDBufSize();
    ASSERT_EQ(dbufSize_after1, dbufSize_before + 1);

    {
        std::unordered_map<Key, Value> mapData2;
        mapData2[key2] = value2;
        cache.AddDBufCacheData(mapData2);
    }
    int dbufSize_after2 = cache.GetDBufSize();
    ASSERT_EQ(dbufSize_after2, dbufSize_after1 + 1);

    Value get_value1;
    bool ret1 = cache.GetCacheData(key1, get_value1);
    ASSERT_EQ(ret1, true);

    bool equal1 = (get_value1 == value1);
    if (!equal1)
    {
        std::cout << "[   INFO   ] "
                  << "get_value1 = " << get_value1 << std::endl;
    }
    ASSERT_EQ(equal1, true);

    Value get_value2;
    bool ret2 = cache.GetCacheData(key2, get_value2);
    ASSERT_EQ(ret2, true);

    bool equal2 = (get_value2 == value2);
    if (!equal2)
    {
        std::cout << "[   INFO   ] "
                  << "get_value2 = " << get_value2 << std::endl;
    }
    ASSERT_EQ(equal2, true);
}

// 7. AddExistRWBuf
// 测试路径: 将数据加入到RWBuf, 然后将数据加入到RWBuf, 两次加入Key一致
// 测试条件1: 第一次加入RWBuf数量加1
// 测试条件2: 第二次加入RWBuf数量不变
// 测试条件3: 获取缓存值与第二次加入值一致
TEST_F(RedisProtoData_HashCache, AddExistRWBuf)
{
    Key key = 12345;
    Value value1 = "proto_12345";
    Value value2 = "proto_123456";

    HashCache<Key, Value> cache;
    int rwbufSize_before = cache.GetRWBufSize();
    {
        std::unordered_map<Key, Value> mapData1;
        mapData1[key] = value1;
        cache.AddRWBufCacheData(mapData1);
    }
    int rwbufSize_after1 = cache.GetRWBufSize();
    ASSERT_EQ(rwbufSize_after1, rwbufSize_before + 1);

    {
        std::unordered_map<Key, Value> mapData2;
        mapData2[key] = value2;
        cache.AddRWBufCacheData(mapData2);
    }
    int rwbufSize_after2 = cache.GetRWBufSize();
    ASSERT_EQ(rwbufSize_after2, rwbufSize_after1);

    Value get_value;
    bool ret = cache.GetCacheData(key, get_value);
    ASSERT_EQ(ret, true);

    bool equal = (get_value == value2);
    if (!equal)
    {
        std::cout << "[   INFO   ] "
                  << "get_value = " << get_value << std::endl;
    }
    ASSERT_EQ(equal, true);
}

// 8. AddNotExistRWBuf
// 测试路径: 将数据加入到RWBuf, 然后将数据加入到RWBuf, 两次加入Key不一致
// 测试条件1: 第一次加入RWBuf数量加1
// 测试条件2: 第二次加入RWBuf数量加1
// 测试条件3: 获取缓存值一致
TEST_F(RedisProtoData_HashCache, AddNotExistRWBuf)
{
    Key key1 = 12345;
    Key key2 = 123456;
    Value value1 = "proto_12345";
    Value value2 = "proto_123456";

    HashCache<Key, Value> cache;
    int rwbufSize_before = cache.GetRWBufSize();
    {
        std::unordered_map<Key, Value> mapData1;
        mapData1[key1] = value1;
        cache.AddRWBufCacheData(mapData1);
    }
    int rwbufSize_after1 = cache.GetRWBufSize();
    ASSERT_EQ(rwbufSize_after1, rwbufSize_before + 1);

    {
        std::unordered_map<Key, Value> mapData2;
        mapData2[key2] = value2;
        cache.AddRWBufCacheData(mapData2);
    }
    int rwbufSize_after2 = cache.GetRWBufSize();
    ASSERT_EQ(rwbufSize_after2, rwbufSize_after1 + 1);

    Value get_value1;
    bool ret1 = cache.GetCacheData(key1, get_value1);
    ASSERT_EQ(ret1, true);

    bool equal1 = (get_value1 == value1);
    if (!equal1)
    {
        std::cout << "[   INFO   ] "
                  << "get_value1 = " << get_value1 << std::endl;
    }
    ASSERT_EQ(equal1, true);

    Value get_value2;
    bool ret2 = cache.GetCacheData(key2, get_value2);
    ASSERT_EQ(ret2, true);

    bool equal2 = (get_value2 == value2);
    if (!equal2)
    {
        std::cout << "[   INFO   ] "
                  << "get_value2 = " << get_value2 << std::endl;
    }
    ASSERT_EQ(equal2, true);
}

// 9. DBuf_Before_RWBuf
// 测试路径: 先将数据加入到DBuf, 再将数据加入到RWBuf, 两次数据Key一致
// 测试条件1: 第一次加入后, DBuf内数据量+1, RWBuf内数据量不变
// 测试条件2: 第二次加入后, DBuf内数据量不变, RWBuf内数据量+1
// 测试条件3: 获取缓存值与第一次DBuf加入的值一致(DBuf优先)
TEST_F(RedisProtoData_HashCache, DBuf_Before_RWBuf)
{
    Key key = 12345;
    Value value1 = "proto_12345";
    Value value2 = "proto_123456";

    HashCache<Key, Value> cache;
    int rwbufSize_before = cache.GetRWBufSize();
    int dbufSize_before = cache.GetDBufSize();
    {
        std::unordered_map<Key, Value> mapData1;
        mapData1[key] = value1;
        cache.AddDBufCacheData(mapData1);
    }

    int rwbufSize_after1 = cache.GetRWBufSize();
    ASSERT_EQ(rwbufSize_after1, rwbufSize_before);

    int dbufSize_after1 = cache.GetDBufSize();
    ASSERT_EQ(dbufSize_after1, dbufSize_before + 1);

    {
        std::unordered_map<Key, Value> mapData2;
        mapData2[key] = value2;
        cache.AddRWBufCacheData(mapData2);
    }
    int rwbufSize_after2 = cache.GetRWBufSize();
    ASSERT_EQ(rwbufSize_after2, rwbufSize_after1 + 1);

    int dbufSize_after2 = cache.GetDBufSize();
    ASSERT_EQ(dbufSize_after2, dbufSize_after1);

    Value get_value;
    bool ret = cache.GetCacheData(key, get_value);
    ASSERT_EQ(ret, true);

    bool equal = (get_value == value1);
    if (!equal)
    {
        std::cout << "[   INFO   ] "
                  << "get_value = " << get_value << std::endl;
    }
    ASSERT_EQ(equal, true);
}

// 10. RWBuf_Before_DBuf
// 测试路径: 先将数据加入到RWBuf, 再将数据加入到DBuf, 两次数据Key一致
// 测试条件1: 第一次加入后, DBuf内数据量不变, RWBuf内数据量+1
// 测试条件2: 第二次加入后, DBuf内数据量+1, RWBuf内数据量不变
// 测试条件3: 获取缓存值与第一次DBuf加入的值一致(DBuf优先)
TEST_F(RedisProtoData_HashCache, RWBuf_Before_DBuf)
{
    Key key = 12345;
    Value value1 = "proto_12345";
    Value value2 = "proto_123456";

    HashCache<Key, Value> cache;
    int rwbufSize_before = cache.GetRWBufSize();
    int dbufSize_before = cache.GetDBufSize();
    {
        std::unordered_map<Key, Value> mapData1;
        mapData1[key] = value1;
        cache.AddRWBufCacheData(mapData1);
    }

    int rwbufSize_after1 = cache.GetRWBufSize();
    ASSERT_EQ(rwbufSize_after1, rwbufSize_before + 1);

    int dbufSize_after1 = cache.GetDBufSize();
    ASSERT_EQ(dbufSize_after1, dbufSize_before);

    {
        std::unordered_map<Key, Value> mapData2;
        mapData2[key] = value2;
        cache.AddDBufCacheData(mapData2);
    }
    int rwbufSize_after2 = cache.GetRWBufSize();
    ASSERT_EQ(rwbufSize_after2, rwbufSize_after1);

    int dbufSize_after2 = cache.GetDBufSize();
    ASSERT_EQ(dbufSize_after2, dbufSize_after1 + 1);

    Value get_value;
    bool ret = cache.GetCacheData(key, get_value);
    ASSERT_EQ(ret, true);

    bool equal = (get_value == value2);
    if (!equal)
    {
        std::cout << "[   INFO   ] "
                  << "get_value = " << get_value << std::endl;
    }
    ASSERT_EQ(equal, true);
}
