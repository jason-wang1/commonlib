#pragma once
#include <memory>
#include <functional>
#include <chrono>
#include <list>
#include <mutex>

namespace TimingWheel
{
    struct Timer;
    struct Bucket;
    using SharedTimer = std::shared_ptr<Timer>;
    using SharedBucket = std::shared_ptr<Bucket>;
    using TimerList = std::list<SharedTimer>;
    using Callback = std::function<void()>;
    struct Bucket : public std::enable_shared_from_this<Bucket>
    {
        bool Add(const std::shared_ptr<Timer> &spTimer)
        {
            if (spTimer == nullptr)
            {
                return false;
            }

            std::unique_lock<std::mutex> lock(m_mutex);
            auto iter = m_timerList.insert(m_timerList.end(), spTimer);
            spTimer->m_wpBucket = shared_from_this();
            spTimer->m_selfIter = iter;
            return true;
        }

        bool Remove(const std::shared_ptr<Timer> &spTimer)
        {
            if (spTimer == nullptr)
            {
                return false;
            }

            std::unique_lock<std::mutex> lock(m_mutex);
            auto spBucket = spTimer->m_wpBucket.lock();
            if (spBucket != shared_from_this())
            {
                return false;
            }

            m_timerList.erase(spTimer->m_selfIter);
            spTimer->m_wpBucket.reset();
            return true;
        }

        auto Expire() const
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_expire;
        }

        bool SetExpire(const std::chrono::steady_clock::time_point expire)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            if (m_expire == expire)
            {
                return false;
            }
            m_expire = expire;
            return true;
        }

        bool Flush(std::function<void(std::shared_ptr<TimingWheel::Timer>)> func)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            auto iter = m_timerList.begin();
            while (iter != m_timerList.end())
            {
                auto &spTimer = *iter;
                iter = m_timerList.erase(iter);
                func(spTimer);
            }
        }

    private:
        mutable std::mutex m_mutex;
        std::chrono::steady_clock::time_point m_expire;
        TimerList m_timerList;
    };

    struct Timer : public std::enable_shared_from_this<Timer>
    {
        Timer(Callback task, std::chrono::steady_clock::time_point expired)
            : m_task(task), m_expired(expired) {}

        Callback m_task;
        std::chrono::steady_clock::time_point m_expired;

        std::weak_ptr<Bucket> m_wpBucket;
        TimerList::iterator m_selfIter;

        // 停止当前任务, 从Bucket中移除即可
        bool Stop()
        {
            auto spBucket = m_wpBucket.lock();
            if (spBucket != nullptr)
            {
                auto spThis = shared_from_this();
                spBucket->Remove(spThis);
            }
        }
    };
}