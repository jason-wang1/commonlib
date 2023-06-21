#pragma once
#include <string>
#include <unordered_map>
#include <list>
#include <mutex>
#include <cassert>
#include "Lock.h"
#include "Time.h"

namespace Common
{
    template <typename Key, typename Value, typename Lock = null_mutex>
    class LFUCache
    {
        // using Key = int;
        // using Value = int;
        // using Lock = null_mutex;

        // 1. 支持key和value任意类型
        // 2. 线程安全
        // 3. 被动触发过期时间(业务实现主动刷新覆盖)
        //      被动的过期的时间表示不会主动检查已经过期的元素, 仅访问时检查是否过期, 如果过期则删除。

        // LFU的淘汰规则是: 优先淘汰低频率数据, 当频率一致时, 淘汰最近最少使用的数据
        // LFU的实现方案及效率对比:
        // 1. 优先队列 priority_queue, 它本身带有一个权重, 它也可以自定义排序规则, 但是其内部实现是一个堆, 故效率为Log(N)。
        // 2. 双向链表 list, 我们对缓存对象增加一个freq字段, 就可以维护频次和时间先后顺序, 但每次freq+1需要向前查询到合适的位置, 故效率为O(N)。
        // 3. 组合结构 哈希+链表 unordered_map<freq, list>, 那么我们更新频次时, 就可以通过一次哈希计算得到对应位置, 其效率为O(1)。
        // 4. 组合结构 二维链表 list<freq+list>, 第一维度为数据的访问频率, 第二维度为数据的访问顺序, 那么它在更新时, 其效率也是O(1), 且会比哈希计算更快。
        //
        // 这里采用二维链表方案, 那么其实也可以当散列的二维数组看待, 假设我们有如下一个表:
        // freq(低-begin) 1 ----- 2 ----- 3 ----- 6 ----- 7 (高-end)
        //              data1 | data4 | data5 | data7 | data8   访(旧)
        //              data2 |       | data6 |       | data9   问
        //              data3                                   顺
        //                                                      序(新)
        // 当要淘汰数据时, 我们只需要从第一维度链表头 freq=1 开始访问, 从旧到新依次删除data1, data2, data3
        // 当 freq=1 被清空时, 删除该节点, 此时表结构如下:
        // freq(低-begin) 2 ----- 3 ----- 6 ----- 7 (高-end)
        //              data4 | data5 | data7 | data8   访(旧)
        //                    | data6 |       | data9   问
        //                                              顺
        //                                              序(新)
        // 如果还要继续清理, 接着按顺序清理 freq=2 的缓存.
        // 当插入新数据 data10 时, 一般初始访问频率 freq=1 故需要重新创建 freq=1 节点, 新的表结构如下:
        // freq(低-begin)  1 ----- 2 ----- 3 ----- 6 ----- 7 (高-end)
        //              data10 | data4 | data5 | data7 | data8   访(旧)
        //                     |       | data6 |       | data9   问
        //                                                       顺
        //                                                       序(新)
        // 接下来访问之前的缓存数据的data7, 那我们需要做什么呢?
        // 1. 我们需要将data7返回给调用方
        // 2. 我们需要将data7的freq值+1
        // 那么我们需要知道以下几个数据:
        // 1. data7存放的位置, 这个可以通过 unordered_map来查找得到list_iter
        // 2. data7此时的频率 freq 是多少, 它在第一维度链表的哪个位置? 这个也需要通过一个 list_iter 来实现
        // 那么我们需要一个什么样的结构就很清晰了:
        // 1. freq_list 频率链表, frqu_node节点存储: freq, node_list
        // 2. data_list 数据链表, data_node节点存储: key, value, freq_list_iter
        // 3. 缓存哈希表, unordered_map<key, data_list_iter>

        class DataNode;
        using DataList = typename std::list<DataNode>;
        using DataListIter = typename DataList::iterator;

        class FreqNode;
        using FreqList = typename std::list<FreqNode>;
        using FreqListIter = typename FreqList::iterator;

        using CacheMap = typename std::unordered_map<Key, DataListIter>;
        using CacheMapIter = typename CacheMap::iterator;

        class DataNode
        {
        public:
            Key key;
            Value value;
            long expire; // ms过期时间戳, 0标识不过期
            FreqListIter freq_iter;

            DataNode(const Key &k, const Value &v, const long &e, const FreqListIter &fi)
                : key(k), value(v), expire(e), freq_iter(fi) {}

            DataNode(const Key &k, Value &&v, const long &e, const FreqListIter &fi)
                : key(k), value(std::move(v)), expire(e), freq_iter(fi) {}
        };

        class FreqNode
        {
        public:
            int freq;
            DataList data_list;

            FreqNode(int f) : freq(f) {}
        };

    public:
        LFUCache(size_t max_size) : m_max_size(max_size) {}

        const bool get(const Key &k, Value &v) noexcept
        {
            long e;
            return get(k, v, e);
        }

        const bool get(const Key &k, Value &v, long &e) noexcept
        {
            constexpr int incr_freq = 1;
            std::lock_guard<Lock> lg(m_lock);

            auto cache_iter = m_cache_map.find(k);
            if (cache_iter == m_cache_map.end())
            {
                return false;
            }

            // 判断缓存是否过期
            if (check_expired(cache_iter))
            {
                return false;
            }

            auto &data_iter = cache_iter->second;
            v = data_iter->value;
            e = data_iter->expire;

            // 确定新数据频率
            auto &cur_data_list = data_iter->freq_iter->data_list;
            auto new_freq_iter = data_iter->freq_iter;
            const int new_freq = data_iter->freq_iter->freq + incr_freq;

            // 找到下一个频率结点
            new_freq_iter++; // 指向下一结点
            if (new_freq_iter->freq != new_freq)
            {
                // 频率不匹配时, 需要创建新的freq_node结点
                new_freq_iter = m_freq_list.emplace(new_freq_iter, new_freq);
            }

            // 移动list结点
            data_iter->freq_iter = new_freq_iter;
            auto &new_data_list = new_freq_iter->data_list;
            new_data_list.splice(new_data_list.begin(), cur_data_list, data_iter);

            return true;
        }

        void set(const Key &k, const Value &v, const long e = 0) noexcept
        {
            constexpr int init_freq = 1; // all new value initialized with the frequency 1
            std::lock_guard<Lock> lg(m_lock);

            // 判断结点是否有超过, 最大缓存, 如果有则清理
            while (m_cache_map.size() >= m_max_size)
            {
                assert(!m_freq_list.empty());

                auto low_freq_iter = m_freq_list.begin();
                if (low_freq_iter != m_freq_list.end())
                {
                    auto &low_freq_data_list = low_freq_iter->data_list;
                    if (!low_freq_data_list.empty())
                    {
                        m_cache_map.erase(low_freq_data_list.begin()->key);
                        low_freq_data_list.pop_front();
                    }

                    if (low_freq_data_list.empty())
                    {
                        m_freq_list.erase(low_freq_iter);
                    }
                }
            }

            auto new_freq_iter = m_freq_list.begin();
            if (new_freq_iter->freq != init_freq)
            {
                // 频率不匹配, 创建新的 freq_node
                new_freq_iter = m_freq_list.emplace(new_freq_iter, init_freq);
            }

            // 新增结点
            auto &new_data_list = new_freq_iter->data_list;
            auto new_data_iter = new_data_list.emplace(new_data_list.end(), k, v, e, new_freq_iter);
            m_cache_map[k] = new_data_iter;
        }

        void set(const Key &k, Value &&v, const long e = 0) noexcept
        {
            constexpr int init_freq = 1; // all new value initialized with the frequency 1
            std::lock_guard<Lock> lg(m_lock);

            // 判断结点是否有超过, 最大缓存, 如果有则清理
            while (m_cache_map.size() >= m_max_size)
            {
                assert(!m_freq_list.empty());

                auto low_freq_iter = m_freq_list.begin();
                if (low_freq_iter != m_freq_list.end())
                {
                    auto &low_freq_data_list = low_freq_iter->data_list;
                    if (!low_freq_data_list.empty())
                    {
                        m_cache_map.erase(low_freq_data_list.begin()->key);
                        low_freq_data_list.pop_front();
                    }

                    if (low_freq_data_list.empty())
                    {
                        m_freq_list.erase(low_freq_iter);
                    }
                }
            }

            auto new_freq_iter = m_freq_list.begin();
            if (new_freq_iter->freq != init_freq)
            {
                // 频率不匹配, 创建新的 freq_node
                new_freq_iter = m_freq_list.emplace(new_freq_iter, init_freq);
            }

            // 新增结点
            auto &new_data_list = new_freq_iter->data_list;
            auto new_data_iter = new_data_list.emplace(new_data_list.end(), k, std::move(v), e, new_freq_iter);
            m_cache_map[k] = new_data_iter;
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

            bool rm = false;
            while (!rm && !m_freq_list.empty())
            {
                auto low_freq_iter = m_freq_list.begin();
                if (low_freq_iter != m_freq_list.end())
                {
                    auto &low_freq_data_list = low_freq_iter->data_list;
                    if (const auto data_iter = low_freq_data_list.begin();
                        data_iter != low_freq_data_list.end())
                    {
                        if (k)
                        {
                            *k = data_iter->key;
                        }
                        if (v)
                        {
                            *v = data_iter->value;
                        }
                        if (e)
                        {
                            *e = data_iter->expire;
                        }

                        m_cache_map.erase(data_iter->key);
                        low_freq_data_list.pop_front();
                        rm = true;
                    }

                    if (low_freq_data_list.empty())
                    {
                        m_freq_list.erase(low_freq_iter);
                    }
                }
            }
            return rm;
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
        bool check_expired(const CacheMapIter cache_iter) noexcept
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

        void remove_iter(const CacheMapIter cache_iter) noexcept
        {
            auto data_iter = cache_iter->second;
            auto freq_iter = data_iter->freq_iter;
            freq_iter->data_list.erase(data_iter);
            if (freq_iter->data_list.empty())
            {
                m_freq_list.erase(freq_iter);
            }
            m_cache_map.erase(cache_iter);
        }

    private:
        mutable Lock m_lock;
        std::size_t m_max_size = 0;

        FreqList m_freq_list;
        CacheMap m_cache_map;
    };
}
