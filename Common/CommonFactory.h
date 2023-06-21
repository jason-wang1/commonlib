#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>

template <typename T>
class CommonBuilder_Interface
{
public:
    virtual std::shared_ptr<T> build() const = 0;
};

template <typename T>
class CommonFactory
{
private:
    using InstPtr = std::shared_ptr<CommonFactory<T>>;
    using BuilderPtr = std::shared_ptr<CommonBuilder_Interface<T>>;

public:
    struct Register
    {
        Register(const std::string &name, const BuilderPtr &builder) noexcept
        {
            GetInstance()->AddBuilder(name, builder);
        }
    };

public:
    // 自行实现单例 - Singleton继承, 编译太难了......
    static InstPtr &GetInstance() noexcept
    {
        if (m_instance_ptr == nullptr)
        {
            std::lock_guard<std::mutex> lg(m_mutex);
            if (m_instance_ptr == nullptr)
            {
                m_instance_ptr = std::make_shared<CommonFactory<T>>();
            }
        }
        return m_instance_ptr;
    }

public:
    bool AddBuilder(const std::string &name, const BuilderPtr &builder) noexcept
    {
        std::lock_guard<std::mutex> lg(m_mutex); // 注册的时候加个锁吧
        if (m_mapBuilderPtr.find(name) == m_mapBuilderPtr.end())
        {
            m_mapBuilderPtr[name] = builder;
            return true;
        }
        return false;
    }

    std::shared_ptr<T> Create(const std::string &name) const noexcept
    {
        auto it_builder = m_mapBuilderPtr.find(name);
        if (it_builder != m_mapBuilderPtr.end())
        {
            return it_builder->second->build();
        }
        return nullptr;
    }

private:
    static InstPtr m_instance_ptr;
    static std::mutex m_mutex;
    std::unordered_map<std::string, BuilderPtr> m_mapBuilderPtr;
};

template <typename T>
std::shared_ptr<CommonFactory<T>> CommonFactory<T>::m_instance_ptr = nullptr;

template <typename T>
std::mutex CommonFactory<T>::m_mutex;
