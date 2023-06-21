#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

#include "Singleton.h"

class ThreadPool : public Singleton<ThreadPool>
{
public:
    ThreadPool(token) {}
    ~ThreadPool() {}
    ThreadPool(ThreadPool &) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;

public:
    /**
     * 线程的状态：有等待、运行、停止
     */
    enum ThreadStatus
    {
        TS_Init = 0,
        TS_Waiting = 1,
        TS_Running = 2,
    };

    using ThreadSharedPtr = std::shared_ptr<std::thread>;
    using ThreadStatusAtomic = std::atomic<ThreadStatus>;

    /**
     * 线程池中线程存在的基本单位，每个线程都有个自定义的ID，有线程种类标识和状态
     */
    struct ThreadWrapper
    {
        ThreadSharedPtr tptr;
        ThreadStatusAtomic tstatus;

        ThreadWrapper()
        {
            tptr = nullptr;
            tstatus.store(ThreadStatus::TS_Init);
        }
    };

    using ThreadWrapperSharedPtr = std::shared_ptr<ThreadWrapper>;
    using ThreadPoolUniqueLock = std::unique_lock<std::mutex>;

    // 初始化线程池
    bool Init(int threads_num)
    {
        this->_waiting_threads_num.store(0);

        this->_is_shutdown.store(false);

        if (threads_num <= 0)
        {
            return false;
        }

        this->_threads_num = threads_num;

        for (int idx = 0; idx < this->_threads_num; idx++)
        {
            AddThread();
        }

        _is_available.store(true);

        return true;
    }

    // 获取等待状态的任务数量
    int GetWaitTaskCount()
    {
        ThreadPoolUniqueLock lock(_tasks_mutex);
        return _tasks_queue.size();
    }

    // 获取正在处于等待状态的线程的个数
    int GetWaitThreadCount()
    {
        return this->_waiting_threads_num.load();
    }

    // 获取线程池中当前线程的总个数
    int GetTotalThreadCount()
    {
        return this->_worker_threads.size();
    }

    // 放在线程池中执行函数
    template <typename F, typename... Args>
    void Push(F &&f, Args &&...args)
    {
        if (this->_is_shutdown.load() || !IsAvailable())
        {
            return;
        }

        auto task =
            std::make_shared<std::packaged_task<std::result_of_t<F(Args...)>()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        {
            ThreadPoolUniqueLock lock(this->_tasks_mutex);
            this->_tasks_queue.emplace([task]()
                                       { (*task)(); });
        }

        this->_tasks_cv.notify_one();
    }

    // 当前线程池是否可用
    bool IsAvailable()
    {
        return this->_is_available.load();
    }

    // 关掉线程池，内部还没有执行的任务会继续执行
    void ShutDown()
    {
        if (IsAvailable())
        {

            this->_is_shutdown.store(true);

            this->_tasks_cv.notify_all();

            this->_is_available.store(false);

            for (auto &thread_wrapper_ptr : _worker_threads)
            {
                if (thread_wrapper_ptr->tptr->joinable())
                {
                    thread_wrapper_ptr->tptr->join();
                }
            }
        }
    }

private:
    void AddThread()
    {
        ThreadWrapperSharedPtr thread_wrapper_ptr = std::make_shared<ThreadWrapper>();

        auto thread_func = [this, thread_wrapper_ptr]()
        {
            do
            {
                std::function<void()> task;
                {
                    ThreadPoolUniqueLock lock(this->_tasks_mutex);

                    thread_wrapper_ptr->tstatus.store(ThreadStatus::TS_Waiting);
                    ++this->_waiting_threads_num;

                    this->_tasks_cv.wait(
                        lock, [this, thread_wrapper_ptr]
                        { return (this->_is_shutdown || !this->_tasks_queue.empty()); });

                    if (this->_is_shutdown && this->_tasks_queue.empty())
                    {
                        break;
                    }

                    --this->_waiting_threads_num;
                    thread_wrapper_ptr->tstatus.store(ThreadStatus::TS_Running);
                    task = std::move(this->_tasks_queue.front());
                    this->_tasks_queue.pop();
                }

                task();
            } while (true);
        };

        thread_wrapper_ptr->tptr = std::make_shared<std::thread>(std::move(thread_func));
        this->_worker_threads.emplace_back(std::move(thread_wrapper_ptr));
    }

private:
    int _threads_num;

    std::list<ThreadWrapperSharedPtr> _worker_threads;

    std::queue<std::function<void()>> _tasks_queue;
    std::mutex _tasks_mutex;
    std::condition_variable _tasks_cv;

    std::atomic<int> _waiting_threads_num;

    std::atomic<bool> _is_shutdown;
    std::atomic<bool> _is_available;
};
