#pragma once
#include <memory>
#include <mutex>

template <typename T>
class Singleton
{
public:
    using Ptr = std::shared_ptr<T>;
    static Ptr &GetInstance() noexcept(std::is_nothrow_constructible<T>::value)
    {
        if (m_instance_ptr == nullptr)
        {
            std::lock_guard<std::mutex> lg(m_mutex);
            if (m_instance_ptr == nullptr)
            {
                m_instance_ptr.reset(new T(token()));
            }
        }
        return m_instance_ptr;
    }

    virtual ~Singleton() = default;
    Singleton(const Singleton &) = delete;
    Singleton &operator=(const Singleton &) = delete;

protected:
    struct token
    { // helper class
    };
    Singleton() noexcept = default;

private:
    static std::shared_ptr<T> m_instance_ptr;
    static std::mutex m_mutex;
};

template <typename T>
std::shared_ptr<T> Singleton<T>::m_instance_ptr = nullptr;

template <typename T>
std::mutex Singleton<T>::m_mutex;
