#pragma once
#include <list>
#include <unordered_set>
#include <unordered_map>
#include <mutex>
#include <condition_variable>

template <typename T, typename Builder, typename Deleter>
class CommonPool
{
    using ret_type = std::unique_ptr<T, Deleter>;

public:
    CommonPool()
    {
        m_init = false;
        m_init_size = 0;
        m_max_idle_size = 0;
        m_max_size = 0;
        m_wait = true;
        add(m_init_size);
    }

    ~CommonPool()
    {
        Destroy();
    }

    bool Init(int init_size,
              int max_idle_size,
              int max_size)
    {
        if (!m_init)
        {
            std::lock_guard<std::mutex> lock_guard(m_lock);
            if (!m_init)
            {
                m_init_size = init_size;
                m_max_idle_size = max_idle_size;
                m_max_size = max_size;
                m_wait = false;
                add(m_init_size);
                m_init = true;
                return true;
            }
        }
        return false; // 重复初始化
    }

    void Destroy()
    {
        m_init = false;

#ifdef Unit_Test
        std::cout << "m_idle_queue.size() = " << m_idle_queue.size() << std::endl;
#endif
        for (auto &i : m_idle_queue)
        {
            Deleter()(i);
        }
        m_idle_queue.clear();

#ifdef Unit_Test
        std::cout << "m_active_set.size() = " << m_active_set.size() << std::endl;
#endif
        for (auto &i : m_active_set)
        {
            Deleter()(i);
        }
        m_active_set.clear();
    }

    ret_type Get()
    {
        auto to_ret_type = [&](T *t)
        {
            return ret_type(t, Deleter());
        };

        bool idle_queue_is_empty = false;
        int active_set_size = 0;
        {
            std::lock_guard<std::mutex> lock_guard(m_lock);
            idle_queue_is_empty = m_idle_queue.empty();
            active_set_size = m_active_set.size();

            if (idle_queue_is_empty && active_set_size < m_max_size)
            {
                T *t = build();
                m_active_set.insert(t);
                return to_ret_type(t);
            }
        }

        if (idle_queue_is_empty && active_set_size >= m_max_size)
        {
            std::unique_lock<std::mutex> lock(m_wait_mutex);
            m_wait = false;
            while (!m_wait)
            {
                // using wait_for prevent timeout
                m_wait_cv.wait(lock);
            }
        }

        std::lock_guard<std::mutex> lock_guard(m_lock);
        idle_queue_is_empty = m_idle_queue.empty();
        if (idle_queue_is_empty)
        {
            T *t = build();
            m_active_set.insert(t);
            return to_ret_type(t);
        }

        T *t = m_idle_queue.front();
        m_idle_queue.pop_front();
        m_active_set.insert(t);
        return to_ret_type(t);
    }

    bool Release(T *t)
    {
        std::lock_guard<std::mutex> lock_guard(m_lock);
        if (m_init)
        {
            if (auto active_it = m_active_set.find(t);
                active_it != m_active_set.end())
            {
                m_active_set.erase(active_it);
            }

            m_idle_queue.push_back(t);

            m_wait = true;
            m_wait_cv.notify_one();
            return true;
        }
        return false;
    }

protected:
    void add(int size)
    {
        for (int i = 0; i < size; ++i)
        {
            T *t = build();
            m_idle_queue.push_back(t);
        }
    }

    inline T *build()
    {
        return Builder::Build();
    }

private:
    std::list<T *> m_idle_queue;          // 闲置资源列表
    std::unordered_set<T *> m_active_set; // 交付使用资源集合

    std::atomic<bool> m_init; // 初始化
    int m_init_size;          // 初始化资源数量
    int m_max_idle_size;      // 最大闲置资源数量
    int m_max_size;           // 最大创建资源数量

    bool m_wait;
    std::mutex m_wait_mutex;
    std::condition_variable m_wait_cv;
    std::mutex m_lock;
};
