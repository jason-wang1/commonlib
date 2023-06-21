#pragma once
#include "gtest/gtest.h"
#include "Common/Singleton.h"
#include "Common/CommonPool.h"

// 以String类型为例进行测试
using String = std::string;

class StringBuilder;
class StringDeleter;
class StringPool final
    : public CommonPool<String, StringBuilder, StringDeleter>,
      public Singleton<StringPool>
{
public:
    StringPool(token) {}
    virtual ~StringPool() {}
    StringPool(const StringPool &) = delete;
    StringPool &operator=(const StringPool &) = delete;
};

// 用于创建对象的Builder
class StringBuilder final
{
public:
    // 初始化, 设置创建对象参数.
    static bool Init(const String &defaultValue)
    {
        g_init = true;
        g_default_value = defaultValue;
        return true;
    }

    // 必须实现, 用于创建实际对象, 返回对象的地址即可.
    static String *Build()
    {
        if (g_init)
        {
            return new String(g_default_value);
        }
        return nullptr;
    }

private:
    inline static std::atomic<bool> g_init = false;
    inline static String g_default_value;
};

// 用于清理对象的Deleter
class StringDeleter final
{
public:
    void operator()(String *pValue) const
    {
        if (pValue != nullptr)
        {
            // 如果Release失败的话, 需要自己手动释放
            bool succ = StringPool::GetInstance()->Release(pValue);
            if (!succ)
            {
                std::cout << "[   INFO   ] release failed, delete value = " << *pValue << std::endl;
                delete pValue;
                pValue = nullptr;
            }
        }
    }
};

// String 型数据结构测试
class CommonPool_String : public testing::Test
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

TEST_F(CommonPool_String, InitPool)
{
    StringBuilder::Init("3");
    bool init = StringPool::GetInstance()->Init(0, 3, 3);
    ASSERT_EQ(init, true);

    for (int idx = 0; idx < 3; idx++)
    {
        auto value = StringPool::GetInstance()->Get();
        std::cout << "[   INFO   ] "
                  << "idx = " << idx
                  << ", value = " << *value << std::endl;

        *value = std::to_string(idx);

        std::cout << "[   INFO   ] "
                  << "idx = " << idx
                  << ", set value = " << *value << std::endl;
    }
}