#pragma once
#include "LRUCache.h"
#include "LFUCache.h"

namespace Common
{
    template <typename Key, typename Value, typename Lock = null_mutex>
    class ARCCache
    {
        // using Key = int;
        // using Value = int;
        // using Lock = null_mutex;

        // ARCCache 是一个线程安全的固定大小自适应替换缓存
        // 参考: https://github.com/songangweb/mcache

    public:
        ARCCache(std::size_t max_size)
            : m_max_size(max_size),
              t1(max_size), b1(max_size),
              t2(max_size), b2(max_size) {}

        const bool get(const Key &k, Value &v) noexcept
        {
            long e;
            return get(k, v, e);
        }

        const bool get(const Key &k, Value &v, long &e) noexcept
        {
            std::lock_guard<Lock> lg(m_lock);

            // If the value is contained in T1 (recent), then promote it to T2 (frequent)
            // 如果这个值出现在t1中(最近使用), 则将其提升到t2中(频繁使用)
            if (t1.get(k, v, e))
            {
                t1.remove(k);
                t2.set(k, v, e);
                return true;
            }
            if (t2.get(k, v, e))
            {
                return true;
            }
            return false;
        }

        // 写入缓存, 如果已经存在则更新信息
        void set(const Key &k, const Value &v, const long e = 0) noexcept
        {
            std::lock_guard<Lock> lg(m_lock);

            // Check if the value is contained in T1 (recent), and potentially promote it to frequent T2
            // 如果这个值出现在t1(最近使用)中, 则将其提升到t2(频繁使用)中
            if (t1.exists(k))
            {
                t1.remove(k);
                t2.set(k, v, e);
                return;
            }

            // Check if the value is already in T2 (frequent) and update it
            // 检查该值是否在t2(频繁使用)中, 并更新它
            if (t2.exists(k))
            {
                // P.s> 注意这里重复写入, 如果t2实现不正确, 则会导致t2内bug
                t2.set(k, v, e);
                return;
            }

            // Check if this value was recently evicted as part of the recently used list
            // 检查这个值是否作为t1(最近使用)的一部分被驱逐
            if (b1.exists(k))
            {
                // T1设置太小，适当增加P
                std::size_t delta = 1;
                const std::size_t b1_size = b1.size();
                const std::size_t b2_size = b2.size();
                if (b1_size != 0 && b2_size > b1_size)
                {
                    delta = b2_size / b1_size;
                }
                p = std::min(m_max_size, p + delta);

                // 可能需要在缓存中腾出空间
                if (t1.size() + t2.size() > m_max_size)
                {
                    replace(false);
                }

                b1.remove(k);    // remove from b1
                t2.set(k, v, e); // Add the key to the frequently used list
                return;
            }

            // Check if this value was recently evicted as part of the frequently used list
            // 检查这个值是否作为t2(频繁使用)的一部分被驱逐
            if (b2.exists(k))
            {
                // T2设置太小, 适当减小P
                std::size_t delta = 1;
                const std::size_t b1_size = b1.size();
                const std::size_t b2_size = b2.size();
                if (b2_size != 0 && b2_size < b1_size)
                {
                    delta = b1_size / b2_size;
                }
                p = std::max(0UL, p - delta);

                // Potentially need to make room in the cache
                // 可能需要在缓存中腾出空间
                if (t1.size() + t2.size() >= m_max_size)
                {
                    replace(true);
                }

                b2.remove(k);    // remove from b2
                t2.set(k, v, e); // Add the key to the frequently used list
                return;
            }

            // 可能需要在缓存中腾出空间
            if (t1.size() + t2.size() > m_max_size)
            {
                replace(false);
            }

            // 保持剔除缓冲区大小
            if (b1.size() > m_max_size - p)
            {
                b1.remove_oldest();
            }
            if (b2.size() > p)
            {
                b2.remove_oldest();
            }

            // Add to the recently seen list
            // 添加到t1(最近使用)列表
            t1.set(k, v, e);
            return;
        }

        const std::size_t size() const noexcept
        {
            std::lock_guard<Lock> lg(m_lock);
            return t1.size() + t2.size();
        }

        const bool remove(const Key &k) noexcept
        {
            std::lock_guard<Lock> lg(m_lock);
            if (t1.remove(k) || b1.remove(k) ||
                t2.remove(k) || b2.remove(k))
            {
                return true;
            }
            return false;
        }

        const bool exists(const Key &k) noexcept
        {
            std::lock_guard<Lock> lg(m_lock);
            return t1.exists(k) || t2.exists(k);
        }

    private:
        // 根据P的当前学习值自适应地从t1或t2中驱逐缓存
        void replace(bool b2_exist_key) noexcept
        {
            const auto t1_size = t1.size();
            if (t1_size > 0 && (t1_size > p || (t1_size == p && b2_exist_key)))
            {
                Key k;
                long e;
                if (t1.remove_oldest(&k, nullptr, &e))
                {
                    b1.set(k, true, e);
                }
            }
            else
            {
                Key k;
                long e;
                if (t2.remove_oldest(&k, nullptr, &e))
                {
                    b2.set(k, true, e);
                }
            }
        }

    private:
        mutable Lock m_lock;
        std::size_t m_max_size = 0;
        std::size_t p = 0;

        Common::LRUCache<Key, Value> t1;
        Common::LRUCache<Key, bool> b1;
        Common::LFUCache<Key, Value> t2;
        Common::LFUCache<Key, bool> b2;
    };
}