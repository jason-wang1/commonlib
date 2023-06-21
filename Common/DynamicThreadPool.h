// From grpc
#pragma once
#include <list>
#include <memory>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include "Common/Singleton.h"

class DynamicThreadPool final : public Singleton<DynamicThreadPool>
{
public:
    DynamicThreadPool(token) noexcept { shutdown_ = true; }
    ~DynamicThreadPool() {}
    DynamicThreadPool(DynamicThreadPool &) = delete;
    DynamicThreadPool &operator=(const DynamicThreadPool &) = delete;

    bool Init(int reserve_threads, int max_threads)
    {
        std::unique_lock<std::mutex> lock(mu_);
        if (shutdown_)
        {
            shutdown_ = false;
            reserve_threads_ = reserve_threads;
            max_threads_ = max_threads;
            nthreads_ = 0;
            threads_waiting_ = 0;
            for (int i = 0; i < reserve_threads_; i++)
            {
                nthreads_++;
                new DynamicThread(this);
            }
            return true;
        }
        return false;
    }

    void ShutDown()
    {
        std::unique_lock<std::mutex> lock(mu_);
        if (!shutdown_)
        {
            shutdown_ = true;
            cv_.notify_all();
            while (nthreads_ != 0)
            {
                shutdown_cv_.wait(lock);
            }
            ReapThreads(&dead_threads_);
        }
    }

    template <typename Func, typename... Args>
    auto Add(Func &&func, Args &&...args) -> std::future<decltype(func(args...))>
    {
        if (this->shutdown_)
        {
            throw std::runtime_error("push shutdown thread pool.");
        }

        using RetType = decltype(func(args...));
        auto task = std::make_shared<std::packaged_task<RetType()>>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...));

        std::future<RetType> ret = task->get_future();

        std::unique_lock<std::mutex> lock(mu_);

        // Add works to the callbacks list
        callbacks_.emplace([task]()
                           { (*task)(); });

        // Increase pool size or notify as needed
        if (threads_waiting_ == 0 && nthreads_ < max_threads_)
        {
            // Kick off a new thread
            nthreads_++;
            new DynamicThread(this);
        }
        else
        {
            cv_.notify_one();
        }

        // Also use this chance to harvest dead threads
        if (!dead_threads_.empty())
        {
            ReapThreads(&dead_threads_);
        }
        return ret;
    }

    const int GetWaitTaskCount() noexcept
    {
        std::unique_lock<std::mutex> ul(mu_);
        return callbacks_.size();
    }

    const int GetWaitThreadCount() noexcept
    {
        std::unique_lock<std::mutex> ul(mu_);
        return threads_waiting_;
    }

    const int GetTotalThreadCount() noexcept
    {
        std::unique_lock<std::mutex> ul(mu_);
        return nthreads_;
    }

private:
    class DynamicThread
    {
    public:
        explicit DynamicThread(DynamicThreadPool *pool)
            : pool_(pool),
              thd_([](void *th)
                   { static_cast<DynamicThreadPool::DynamicThread *>(th)->ThreadFunc(); },
                   this) {}

        ~DynamicThread()
        {
            if (thd_.joinable())
            {
                thd_.join();
            }
        }

    private:
        DynamicThreadPool *pool_;
        std::thread thd_;
        void ThreadFunc()
        {
            pool_->ThreadFunc();
            // Now that we have killed ourselves, we should reduce the thread count
            std::unique_lock<std::mutex> lock(pool_->mu_);
            pool_->nthreads_--;
            // Move ourselves to dead list
            pool_->dead_threads_.push_back(this);

            if ((pool_->shutdown_) && (pool_->nthreads_ == 0))
            {
                pool_->shutdown_cv_.notify_one();
            }
        }
    };
    std::mutex mu_;
    std::condition_variable cv_;
    std::condition_variable shutdown_cv_;
    bool shutdown_;
    std::queue<std::function<void()>> callbacks_;
    int reserve_threads_;
    int max_threads_;
    int nthreads_;
    int threads_waiting_;
    std::list<DynamicThread *> dead_threads_;

    void ThreadFunc()
    {
        for (;;)
        {
            std::function<void()> cb;
            {
                std::unique_lock<std::mutex> ul(mu_);
                if (!shutdown_ && callbacks_.empty())
                {
                    // If there are too many threads waiting, then quit this thread
                    if (threads_waiting_ >= reserve_threads_)
                    {
                        break;
                    }
                    threads_waiting_++;
                    this->cv_.wait(ul, [this]
                                   { return (this->shutdown_ || !this->callbacks_.empty()); });
                    threads_waiting_--;
                }

                // Drain callbacks before considering shutdown to ensure all work
                // gets completed.
                if (!callbacks_.empty())
                {
                    cb = callbacks_.front();
                    callbacks_.pop();
                }
                else if (shutdown_)
                {
                    break;
                }
            }
            cb();
        }
    }

    static void ReapThreads(std::list<DynamicThread *> *tlist)
    {
        for (auto t = tlist->begin(); t != tlist->end(); t = tlist->erase(t))
        {
            delete *t;
        }
    }
};