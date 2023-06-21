#pragma once
#include <string>
#include <unordered_map>
#include <list>
#include <mutex>
#include "Lock.h"
#include "Time.h"

namespace Common
{
    template <typename Key, typename Value, typename Lock = null_mutex>
    class LRUCache
    {
        // using Key = int;
        // using Value = int;
        // using Lock = null_mutex;

        // 1. 支持key和value任意类型
        // 2. 线程安全
        // 3. 被动触发过期时间(业务实现主动刷新覆盖)
        //      被动的过期的时间表示不会主动检查已经过期的元素, 仅访问时检查是否过期, 如果过期则删除。

        struct DataNode
        {
            Key key;
            Value value;
            long expire; // ms过期时间戳, 0标识不过期

            DataNode(const Key &k, const Value &v, const long &e)
                : key(k), value(v), expire(e) {}

            DataNode(const Key &k, Value &&v, const long &e)
                : key(k), value(std::move(v)), expire(e) {}
        };
        using CacheList = typename std::list<DataNode>;
        using CacheListIter = typename CacheList::iterator;

        using CacheMap = typename std::unordered_map<Key, CacheListIter>;
        using CacheMapIter = typename CacheMap::iterator;

    public:
        LRUCache(std::size_t max_size) : m_max_size(max_size) {}

        const bool get(const Key &k, Value &v) noexcept
        {
            long e;
            return get(k, v, e);
        }

        const bool get(const Key &k, Value &v, long &e) noexcept
        {
            std::lock_guard<Lock> lg(m_lock);

            // 找到数据
            auto cache_iter = m_cache_map.find(k);
            if (cache_iter == m_cache_map.end())
            {
                return false;
            }

            // 判断缓存已过期
            if (check_expired(cache_iter))
            {
                return false;
            }

            // 返回缓存数据
            auto &data_iter = cache_iter->second;
            m_cache_list.splice(m_cache_list.begin(), m_cache_list, data_iter);
            v = data_iter->value;
            e = data_iter->expire;
            return true;
        }

        void set(const Key &k, const Value &v, const long e = 0) noexcept
        {
            std::lock_guard<Lock> lg(m_lock);
            auto it = m_cache_map.find(k);
            m_cache_list.emplace_front(k, v, e);

            if (it != m_cache_map.end())
            {
                m_cache_list.erase(it->second);
                m_cache_map.erase(it);
            }
            m_cache_map[k] = m_cache_list.begin();

            if (m_cache_map.size() > m_max_size)
            {
                const auto &last_item = m_cache_list.back();
                m_cache_map.erase(last_item.key);
                m_cache_list.pop_back();
            }
        }

        void set(const Key &k, Value &&v, const long e = 0) noexcept
        {
            std::lock_guard<Lock> lg(m_lock);
            auto it = m_cache_map.find(k);
            m_cache_list.emplace_front(k, std::move(v), e);

            if (it != m_cache_map.end())
            {
                m_cache_list.erase(it->second);
                m_cache_map.erase(it);
            }
            m_cache_map[k] = m_cache_list.begin();

            if (m_cache_map.size() > m_max_size)
            {
                const auto &last_item = m_cache_list.back();
                m_cache_map.erase(last_item.key);
                m_cache_list.pop_back();
            }
        }

        const bool remove(const Key &k) noexcept
        {
            std::lock_guard<Lock> lg(m_lock);
            auto cache_iter = m_cache_map.find(k);
            if (cache_iter == m_cache_map.end())
            {
                return false;
            }
            remove_iter(cache_iter);
            return true;
        }

        const bool remove_oldest(
            Key *k = nullptr,
            Value *v = nullptr,
            long *e = nullptr) noexcept
        {
            std::lock_guard<Lock> lg(m_lock);
            if (m_cache_list.empty())
            {
                return false;
            }

            const auto &data_node = m_cache_list.back();
            if (k)
            {
                *k = data_node.key;
            }
            if (v)
            {
                *v = data_node.value;
            }
            if (e)
            {
                *e = data_node.expire;
            }
            m_cache_map.erase(data_node.key);
            m_cache_list.pop_back();
            return true;
        }

        const bool exists(const Key &k) noexcept
        {
            std::lock_guard<Lock> lg(m_lock);
            auto cache_iter = m_cache_map.find(k);
            if (cache_iter == m_cache_map.end())
            {
                return false;
            }
            if (check_expired(cache_iter))
            {
                return false;
            }
            return true;
        }

        const std::size_t size() const noexcept
        {
            std::lock_guard<Lock> lg(m_lock);
            return m_cache_map.size();
        }

    private:
        bool check_expired(const CacheMapIter &cache_iter) noexcept
        {
            const auto &data_iter = cache_iter->second;
            const auto &expire = data_iter->expire;
            if (expire != 0 && expire <= Common::get_ms_timestamp())
            {
                remove_iter(cache_iter);
                return true;
            }
            return false;
        }

        // 通过内部迭代器移除缓存数据
        void remove_iter(const CacheMapIter &cache_iter) noexcept
        {
            m_cache_list.erase(cache_iter->second);
            m_cache_map.erase(cache_iter);
        }

    private:
        mutable Lock m_lock;
        std::size_t m_max_size = 0;

        CacheList m_cache_list;
        CacheMap m_cache_map;
    };
}
