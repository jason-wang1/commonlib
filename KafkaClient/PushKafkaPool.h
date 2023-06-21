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

#include "Common/Singleton.h"
#include "TDKafkaProducer.h"

class PushKafkaPool : public Singleton<PushKafkaPool>
{
public:
    PushKafkaPool(token) {}
    ~PushKafkaPool() {}
    PushKafkaPool(PushKafkaPool &) = delete;
    PushKafkaPool &operator=(const PushKafkaPool &) = delete;

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
    using TDKafkaProducerSharedPtr = std::shared_ptr<TDKafkaProducer>;
    using ThreadStatusAtomic = std::atomic<ThreadStatus>;

    /**
     * 线程池中线程存在的基本单位，每个线程都有个自定义的ID，有线程种类标识和状态
     */
    struct ThreadWrapper
    {
        ThreadSharedPtr thread_sptr;
        TDKafkaProducerSharedPtr producer_sptr;
        ThreadStatusAtomic thread_status;

        ThreadWrapper()
        {
            thread_sptr = nullptr;
            producer_sptr = nullptr;
            thread_status.store(ThreadStatus::TS_Init);
        }
    };

    using ThreadWrapperSharedPtr = std::shared_ptr<ThreadWrapper>;
    using MutexUniqueLock = std::unique_lock<std::mutex>;

    // 初始化线程池
    bool Init(int threads_num, const TDKafkaConfig &kafka_conf)
    {
        this->_waiting_threads_num.store(0);

        this->_is_shutdown.store(false);

        if (threads_num <= 0)
        {
            return false;
        }

        for (int idx = 0; idx < threads_num; idx++)
        {
            AddThreadWrapper(kafka_conf);
        }

        _is_available.store(true);

        return true;
    }

    // 放在线程池中执行函数
    void Push(TDKafkaData &&data, int &free_threads_num, int &queue_tasks_num)
    {
        if (this->_is_shutdown.load() || !IsAvailable())
        {
            return;
        }

        {
            MutexUniqueLock lock(this->_tasks_mutex);
            this->_tasks_queue.emplace(data);
            queue_tasks_num = this->_tasks_queue.size();
        }

        free_threads_num = this->_waiting_threads_num.load();
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
                if (thread_wrapper_ptr->thread_sptr->joinable())
                {
                    thread_wrapper_ptr->thread_sptr->join();
                }
            }
        }
    }

private:
    void AddThreadWrapper(const TDKafkaConfig &kafka_conf)
    {
        ThreadWrapperSharedPtr thread_wrapper_ptr = std::make_shared<ThreadWrapper>();
        thread_wrapper_ptr->producer_sptr = std::make_shared<TDKafkaProducer>();
        thread_wrapper_ptr->producer_sptr->Init(kafka_conf);

        auto thread_func = [this, thread_wrapper_ptr]()
        {
            do
            {
                TDKafkaData data;
                {
                    MutexUniqueLock lock(this->_tasks_mutex);

                    thread_wrapper_ptr->thread_status.store(ThreadStatus::TS_Waiting);
                    ++this->_waiting_threads_num;

                    this->_tasks_cv.wait(
                        lock, [this]
                        { return (this->_is_shutdown || !this->_tasks_queue.empty()); });

                    if (this->_is_shutdown && this->_tasks_queue.empty())
                    {
                        break;
                    }

                    --this->_waiting_threads_num;
                    thread_wrapper_ptr->thread_status.store(ThreadStatus::TS_Running);
                    data = std::move(this->_tasks_queue.front());
                    this->_tasks_queue.pop();
                }

                thread_wrapper_ptr->producer_sptr->PushData(data);
            } while (true);
        };

        thread_wrapper_ptr->thread_sptr = std::make_shared<std::thread>(std::move(thread_func));
        this->_worker_threads.emplace_back(std::move(thread_wrapper_ptr));
    }

private:
    std::list<ThreadWrapperSharedPtr> _worker_threads;

    std::queue<TDKafkaData> _tasks_queue;
    std::mutex _tasks_mutex;
    std::condition_variable _tasks_cv;

    std::atomic<int> _waiting_threads_num;

    std::atomic<bool> _is_shutdown;
    std::atomic<bool> _is_available;
};
