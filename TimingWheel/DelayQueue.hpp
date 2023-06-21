#pragma once
#include <queue>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <any>
#include <optional>

class DelayQueue
{
    using TimePoint = std::chrono::steady_clock::time_point;
    class DelayedItem
    {
    public:
        DelayedItem(std::any data, TimePoint expiry)
            : m_data(std::move(data)), m_expiry(expiry) {}

        std::any data() const { return std::move(m_data); }
        TimePoint expiry() const { return m_expiry; }
        bool expired() const { return m_expiry > std::chrono::steady_clock::now(); }

        const bool operator<(const DelayedItem &other) const
        {
            return m_expiry > other.m_expiry;
        }

    private:
        std::any m_data;
        TimePoint m_expiry;
    };

public:
    void offer(std::any item, std::chrono::milliseconds delay)
    {
        const auto time_point = std::chrono::steady_clock::now() + delay;
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.emplace(std::move(item), time_point);
        m_cv.notify_one();
    }

    void offer(std::any item, const std::chrono::_V2::steady_clock::time_point time_point)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.emplace(std::move(item), time_point);
        m_cv.notify_one();
    }

    std::optional<std::any> take()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty() || m_queue.top().expired())
        {
            if (m_queue.empty())
            {
                m_cv.wait(lock);
            }
            else
            {
                m_cv.wait_until(lock, m_queue.top().expiry());
            }
        }

        auto item = m_queue.top();
        m_queue.pop();
        return std::move(item.data());
    }

    // poll 函数是一个非阻塞函数, 如果没有可取出元素会立即返回std::nullopt
    std::optional<std::any> poll()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_queue.empty() || m_queue.top().expired())
        {
            return std::nullopt;
        }

        auto item = m_queue.top();
        m_queue.pop();
        return std::move(item.data());
    }

    bool empty() const
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    const int size() const
    {
        std::unique_lock<std::mutex> ul(m_mutex);
        return m_queue.size();
    }

private:
    std::priority_queue<DelayedItem> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;
};
