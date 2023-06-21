#pragma once
#include <deque>
#include <vector>
#include <cassert>
#include <mutex>
#include <condition_variable>

/////////////////////////////////////////////////
/**
 * @file SafeQueue.h
 * @brief 线程安全队列
 */

/////////////////////////////////////////////////

template <typename T, typename D = std::deque<T>>
class SafeQueue
{
public:
    SafeQueue() : _size(0){};

public:
    typedef D queue_type;

    /**
     * @brief 从头部获取多个数据, 没有数据则等待.
     *
     * @param vecT
     * @param number
     * @param millsecond(wait = true时才生效) 阻塞等待时间(ms)
     *                    0 表示不阻塞
     * 					 -1 永久等待
     * @param wait, 是否wait
     * @return bool: true, 获取了数据, false, 无数据
     */
    bool pop_front(std::vector<T> &vecT, size_t number = 10, size_t millsecond = 0, bool wait = true);

    /**
     * @brief 通知等待在队列上面的线程都醒过来
     */
    void notifyT();

    /**
     * @brief 放数据到队列后端.
     *
     * @param t
     * @param count
     * @param notify, 是否notify
     */
    void push_back(const T &t, size_t count = 10, bool notify = true);

    /**
     * @brief 放数据到队列后端.
     *
     * @param t
     * @param count
     * @param notify, 是否notify
     */
    void push_back(const std::vector<T> &vec_t, size_t count = 10, bool notify = true);

    /**
     * @brief  队列大小.
     *
     * @return size_t 队列大小
     */
    size_t size() const;

    /**
     * @brief  清空队列
     */
    void clear();

    /**
     * @brief  是否数据为空.
     *
     * @return bool 为空返回true，否则返回false
     */
    bool empty() const;

protected:
    SafeQueue(const SafeQueue &) = delete;
    SafeQueue(SafeQueue &&) = delete;
    SafeQueue &operator=(const SafeQueue &) = delete;
    SafeQueue &operator=(SafeQueue &&) = delete;

protected:
    //队列
    queue_type _queue;

    //队列长度
    size_t _size;

    //条件变量
    std::condition_variable _cond;

    //锁
    mutable std::mutex _mutex;
};

template <typename T, typename D>
bool SafeQueue<T, D>::pop_front(std::vector<T> &vecT, size_t number, size_t millsecond, bool wait)
{
    if (wait)
    {
        std::unique_lock<std::mutex> lock(_mutex);

        if (_size < number)
        {
            if (millsecond == 0)
            {
                return false;
            }
            if (millsecond == (size_t)-1)
            {
                //永久等待
                _cond.wait(lock);
            }
            else
            {
                //阻塞等待时间(ms): millsecond
                _cond.wait_for(lock, std::chrono::milliseconds(millsecond));
            }
        }

        if (_queue.empty())
        {
            return false;
        }

        int count = std::min(_size, number);
        while (--count >= 0)
        {
            vecT.push_back(_queue.front());
            _queue.pop_front();
            assert(_size > 0);
            --_size;
        }

        return true;
    }
    else
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_queue.empty())
        {
            return false;
        }

        int count = std::min(_size, number);
        while (--count >= 0)
        {
            vecT.push_back(_queue.front());
            _queue.pop_front();
            assert(_size > 0);
            --_size;
        }

        return true;
    }
}

template <typename T, typename D>
void SafeQueue<T, D>::notifyT()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _cond.notify_all();
}

template <typename T, typename D>
void SafeQueue<T, D>::push_back(const T &t, size_t count, bool notify)
{
    if (notify)
    {
        std::unique_lock<std::mutex> lock(_mutex);

        _queue.push_back(t);
        ++_size;

        if (_size >= count)
        {
            _cond.notify_one();
        }
    }
    else
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue.push_back(t);
        ++_size;
    }
}

template <typename T, typename D>
void SafeQueue<T, D>::push_back(const std::vector<T> &vec_t, size_t count, bool notify)
{
    if (notify)
    {
        std::unique_lock<std::mutex> lock(_mutex);

        for (auto &t : vec_t)
        {
            _queue.push_back(t);
            ++_size;
        }

        if (_size >= count)
        {
            _cond.notify_one();
        }
    }
    else
    {
        std::lock_guard<std::mutex> lock(_mutex);
        for (auto &t : vec_t)
        {
            _queue.push_back(t);
        }
        ++_size;
    }
}

template <typename T, typename D>
size_t SafeQueue<T, D>::size() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _size;
}

template <typename T, typename D>
void SafeQueue<T, D>::clear()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _queue.clear();
    _size = 0;
}

template <typename T, typename D>
bool SafeQueue<T, D>::empty() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _queue.empty();
}
