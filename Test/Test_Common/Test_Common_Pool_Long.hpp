#pragma once
#include "gtest/gtest.h"
#include "Common/Singleton.h"
#include "Common/CommonPool.h"

// 以Long类型为例进行测试
using Long = long;

class LongBuilder;
class LongDeleter;
class LongPool final
    : public CommonPool<Long, LongBuilder, LongDeleter>,
      public Singleton<LongPool>
{
public:
    LongPool(token) {}
    virtual ~LongPool() {}
    LongPool(const LongPool &) = delete;
    LongPool &operator=(const LongPool &) = delete;
};

// 用于创建对象的Builder
class LongBuilder final
{
public:
    // 初始化, 设置创建对象参数.
    static bool Init(const Long defaultValue)
    {
        g_init = true;
        g_default_value = defaultValue;
        return true;
    }

    // 必须实现, 用于创建实际对象, 返回对象的地址即可.
    static Long *Build()
    {
        if (g_init)
        {
            return new Long(g_default_value);
        }
        return nullptr;
    }

private:
    inline static std::atomic<bool> g_init = false;
    inline static Long g_default_value = 0;
};

// 用于清理对象的Deleter
class LongDeleter final
{
public:
    void operator()(Long *pValue) const
    {
        if (pValue != nullptr)
        {
            // 如果Release失败的话, 需要自己手动释放
            bool succ = LongPool::GetInstance()->Release(pValue);
            if (!succ)
            {
                std::cout << "[   INFO   ] release failed, delete value = " << *pValue << std::endl;
                delete pValue;
                pValue = nullptr;
            }
        }
    }
};

// Long 型数据结构测试
class CommonPool_Long : public testing::Test
{
public:
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

TEST_F(CommonPool_Long, InitPool)
{
    LongBuilder::Init(3);
    bool init = LongPool::GetInstance()->Init(0, 3, 3);
    ASSERT_EQ(init, true);

    for (int idx = 0; idx < 3; idx++)
    {
        auto value = LongPool::GetInstance()->Get();
        std::cout << "[   INFO   ] "
                  << "idx = " << idx
                  << ", value = " << *value << std::endl;

        *value = idx;

        std::cout << "[   INFO   ] "
                  << "idx = " << idx
                  << ", set value = " << *value << std::endl;
    }
}