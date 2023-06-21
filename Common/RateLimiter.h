#pragma once
#include <cassert>
#include <mutex>
#include <thread>
#include "Common/Lock.h"
#include "Common/Function.h"
#include "Common/AtomicSequence.h"

// 限流器
// RateLimiter r(100);
// r.pass();
// r.try_pass();
class RateLimiter
{
    static constexpr long NS_PER_SECOND = 1000000000;

public:
    RateLimiter(int64_t qps, int64_t cache)
        : m_CacheSize(cache),
          m_SupplyUnitTime(NS_PER_SECOND / qps),
          m_TokenLeft(0)
    {
        assert(qps <= NS_PER_SECOND);
        assert(qps >= 0);
        assert(cache >= 0);

        m_LastAddTokenTime = Common::get_ns_timestamp();
    }

    RateLimiter(const RateLimiter &) = delete;
    RateLimiter(const RateLimiter &&) = delete;
    RateLimiter &operator=(const RateLimiter &) = delete;

    // 对外接口，能返回true说明流量在限定值内
    bool pass()
    {
        mustGetToken();
        return true;
    }

    bool try_pass(const int try_times = 1)
    {
        for (int i = 0; i < try_times; ++i)
        {
            if (tryGetToken())
            {
                return true;
            }
            std::this_thread::yield();
        }
        return false;
    }

private:
    // 更新令牌桶中的令牌
    void supplyTokens()
    {
        auto cur = Common::get_ns_timestamp();
        if (cur - m_LastAddTokenTime < m_SupplyUnitTime)
        {
            return;
        }

        {
            std::lock_guard<Common::spin_lock> lg(m_SpinLock);

            // 等待自旋锁期间可能已经补充过令牌了
            int64_t newTokens = (cur - m_LastAddTokenTime) / m_SupplyUnitTime;
            if (newTokens <= 0)
            {
                return;
            }

            // 更新令牌补充时间, 不能直接=cur, 否则会导致时间丢失
            m_LastAddTokenTime += (newTokens * m_SupplyUnitTime);

            // 限制令牌数量不能超过缓存池大小
            auto freeRoom = m_CacheSize - m_TokenLeft.load();
            if (newTokens > freeRoom || newTokens > m_CacheSize)
            {
                newTokens = freeRoom > m_CacheSize ? m_CacheSize : freeRoom;
            }

            m_TokenLeft.fetch_add(newTokens);
        }
    }

    // 尝试获得令牌
    bool tryGetToken()
    {
        supplyTokens(); // 补充令牌

        // 获得令牌
        auto token = m_TokenLeft.fetch_add(-1);
        if (token <= 0)
        {
            m_TokenLeft.fetch_add(1);
            return false;
        }
        return true;
    }

    // 必定获得令牌, 会进行重试操作
    void mustGetToken()
    {
        constexpr int RETRY_IMMEDIATELY_TIMES = 10; // 不睡眠的最大重试获得令牌的次数
        for (int i = 0; i < RETRY_IMMEDIATELY_TIMES; ++i)
        {
            if (tryGetToken())
            {
                return;
            }
        }

        for (;;)
        {
            if (tryGetToken())
            {
                return;
            }
            std::this_thread::yield();
        }
    }

private:
    const int64_t m_CacheSize;      // 令牌桶大小
    const int64_t m_SupplyUnitTime; // 补充令牌的单位时间
    int64_t m_LastAddTokenTime;     // 上次补充令牌的时间，单位纳秒
    AtomicSequence m_TokenLeft;     // 剩下的token数
    Common::spin_lock m_SpinLock;   // 自旋锁
};
