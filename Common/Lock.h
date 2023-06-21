#pragma once
#include <atomic>
#include <mutex>

namespace Common
{
    // 空锁
    class null_mutex
    {
    public:
        null_mutex() noexcept = default;
        ~null_mutex() = default;

        null_mutex(const null_mutex &) = delete;
        null_mutex &operator=(const null_mutex &) = delete;

        void lock() {}
        void unlock() {}
        bool try_lock() noexcept { return true; }
    };

    // 自旋锁
    class spin_lock
    {
    public:
        spin_lock() {}
        ~spin_lock() { unlock(); }
        spin_lock(const spin_lock &) = delete;
        spin_lock(const spin_lock &&) = delete;
        spin_lock &operator=(const spin_lock &) = delete;

        void lock() noexcept
        {
            for (;;)
            {
                if (!lock_.exchange(true, std::memory_order_acquire))
                {
                    return;
                }

                while (lock_.load(std::memory_order_relaxed))
                {
                    __builtin_ia32_pause();
                }
            }
        }

        bool try_lock()
        {
            return !lock_.load(std::memory_order_relaxed) &&
                   !lock_.exchange(true, std::memory_order_acquire);
        }

        void unlock()
        {
            lock_.store(false, std::memory_order_release);
        }

    private:
        std::atomic<bool> lock_ = false;
    };
}