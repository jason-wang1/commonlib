#pragma once
#include "TimingWheelInterface.h"
#include "Timer.h"
#include "DelayQueue.hpp"
#include <chrono>
#include <thread>
#include <memory>
#include <list>
#include <atomic>
#include <mutex>
#include <thread>

namespace TimingWheel
{
    class WheelItem
    {
    public:
        WheelItem(const long tick, const long wheel_size)
        {
            m_tick = tick;
            m_wheel_size = wheel_size;

            m_curTime = std::chrono::steady_clock::now();
            m_interval = m_tick * m_wheel_size;
            m_vecSPBucket.resize(m_wheel_size);
            for (int idx = 0; idx < m_wheel_size; idx++)
            {
                m_vecSPBucket[idx] = std::make_shared<Bucket>();
            }
            m_spQueue = std::make_shared<DelayQueue>();
            m_spOverflowWheel = nullptr;
        }

    public:
        bool addTask(Callback task, const long delay)
        {
            addTask(task, std::chrono::steady_clock::now() + std::chrono::milliseconds(delay));
        }

        bool addTask(Callback task, const std::chrono::steady_clock::time_point expired)
        {
            addTask(std::make_shared<Timer>(task, expired));
        }

        bool addTask(const SharedTimer &spTimer)
        {
            // P.s> m_curTime是时间轮运转的时间, 并不是当前的真实时间
            auto expired = spTimer->m_expired;
            if (expired < m_curTime + std::chrono::milliseconds(m_tick))
            {
                return false; // 已超时, 应当直接执行
            }
            else if (expired < m_curTime + std::chrono::milliseconds(m_interval))
            {
                const long delay = std::chrono::duration_cast<std::chrono::milliseconds>(expired - m_curTime).count();
                const auto virtualID = delay / m_tick;
                auto &spBucket = m_vecSPBucket[virtualID % m_wheel_size];
                spBucket->Add(spTimer);

                const auto bucket_expired = m_curTime + std::chrono::milliseconds(virtualID * m_tick);
                if (spBucket->SetExpire(bucket_expired))
                {
                    m_spQueue->offer(spBucket, spBucket->Expire());
                }
                return true;
            }
            else
            {
                // 超出interval, 需交付给上级时间轮
                if (m_spOverflowWheel == nullptr)
                {
                    m_spOverflowWheel = std::make_shared<WheelItem>(m_interval, m_wheel_size);
                }
                return m_spOverflowWheel->addTask(spTimer);
            }
        }

        void advance(const std::chrono::steady_clock::time_point expired)
        {
            if (expired >= m_curTime + std::chrono::milliseconds(m_tick))
            {
                auto interval = std::chrono::duration_cast<std::chrono::milliseconds>(expired - m_curTime).count();
                m_curTime = expired - std::chrono::milliseconds(interval % m_tick);

                if (m_spOverflowWheel != nullptr)
                {
                    m_spOverflowWheel->advance(m_curTime);
                }
            }
        }

    protected:
        void AddOrRun(const SharedTimer &timer)
        {
            if (!addTask(timer))
            {
                timer->m_task();
            }
        }

        void Start()
        {
            while (m_init)
            {
                auto optBucket = m_spQueue->take();
                if (optBucket.has_value())
                {
                    auto spBucket = std::any_cast<SharedBucket>(optBucket.value());
                    if (spBucket != nullptr)
                    {
                        this->advance(spBucket->Expire());
                        spBucket->Flush([&](const std::shared_ptr<TimingWheel::Timer> &timer)
                                        { this->AddOrRun(timer); });
                    }
                }
            }
        }

    private:
        std::atomic<bool> m_init = false;
        std::mutex m_Mutex;
        std::thread m_Thread;

        long m_tick;
        long m_wheel_size;

        long m_interval;
        std::chrono::steady_clock::time_point m_startMs;        // 时间轮起始时间
        std::chrono::steady_clock::time_point m_curTime;        // 时间轮当前时间
        std::vector<SharedBucket> m_vecSPBucket;                // Hash任务桶
        std::shared_ptr<DelayQueue> m_spQueue = nullptr;        // 延迟队列
        std::shared_ptr<WheelItem> m_spOverflowWheel = nullptr; // 上级时间轮
    };
}
