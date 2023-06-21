#pragma once
#include "gtest/gtest.h"
#include "Common/Singleton.h"
#include "Common/CommonPool.h"

// 以String类型为例进行测试
struct Struct
{
public:
    Struct() { std::cout << "[   INFO   ] create struct" << std::endl; }
    ~Struct() { std::cout << "[   INFO   ] delete struct" << std::endl; }
};

class StructBuilder;
class StructDeleter;
class StructPool final
    : public CommonPool<Struct, StructBuilder, StructDeleter>,
      public Singleton<StructPool>
{
public:
    StructPool(token) {}
    virtual ~StructPool() {}
    StructPool(const StructPool &) = delete;
    StructPool &operator=(const StructPool &) = delete;
};

// 用于创建对象的Builder
class StructBuilder final
{
    // P.s> 部分特殊对象就可以不实现Init函数.
public:
    // 必须实现, 用于创建实际对象, 返回对象的地址即可.
    static Struct *Build()
    {
        return new Struct();
    }
};

// 用于清理对象的Deleter
class StructDeleter final
{
public:
    void operator()(Struct *pValue) const
    {
        if (pValue != nullptr)
        {
            // 如果Release失败的话, 需要自己手动释放
            bool succ = StructPool::GetInstance()->Release(pValue);
            if (!succ)
            {
                delete pValue;
                pValue = nullptr;
            }
        }
    }
};

// Struct 型数据结构测试
class CommonPool_Struct : public testing::Test
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

TEST_F(CommonPool_Struct, InitPool)
{
    bool init = StructPool::GetInstance()->Init(0, 3, 3);
    ASSERT_EQ(init, true);

    for (int idx = 0; idx < 3; idx++)
    {
        auto value = StructPool::GetInstance()->Get();
    }
}