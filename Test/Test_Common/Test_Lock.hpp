#pragma once
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <pthread.h>
#include <sched.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <errno.h>

#include <mutex>
#include "Common/Lock.h"

#ifndef cpu_relax
#define cpu_relax() asm volatile("pause\n" \
                                 :         \
                                 :         \
                                 : "memory")
#endif

/* It's hard to say which spinlock implementation performs best. I guess the
 * performance depends on CPU topology which will affect the cache coherence
 * messages, and maybe other factors.
 *
 * The result of binding threads to the same physical core shows that when
 * threads are on a single physical CPU, contention will not cause severe
 * performance degradation. But there's one exception, cmpxchg. It's performance
 * will degrade no matter the threads resides on the which physical CPU.
 *
 * Here's the result on Dell R910 server (4 CPUs, 10 cores each), with Intel(R)
 * Xeon(R) CPU E7- 4850  @ 2.00GHz
 *
 * - pthread_mutex BEATS ALL when there's more than 2 cores running this
 *   benchmark! Not sure what's the trick used by pthread_mutex.
 *
 * - For spinlock with cmpxchg, the performance degrades very fast with the
 * increase of threads. Seems that it does not have good scalability.
 *
 * - For spinlock with xchg, it has much better scalability than cmpxchg, but it's
 * slower when there's 2 and 4 cores.
 *
 * - For K42, The case for 2 threads is extremely bad, for other number of
 *   threads, the performance is stable and shows good scalability.
 *
 * - MCS spinlock has similar performance with K42, and does not have the
 *   extremely bad 2 thread case.
 *
 * - Ticket spinlock actually performs very badly.
 */

/* Number of total lock/unlock pair.
 * Note we need to ensure the total pair of lock and unlock opeartion are the
 * same no matter how many threads are used. */
#define N_PAIR 16000000

/* Bind threads to specific cores. The goal is to make threads locate on the
 * same physical CPU. Modify bind_core before using this. */
//#define BIND_CORE

static int nthr = 0;

static volatile uint32_t wflag;
/* Wait on a flag to make all threads start almost at the same time. */
void wait_flag(volatile uint32_t *flag, uint32_t expect)
{
    __sync_fetch_and_add((uint32_t *)flag, 1);
    while (*flag != expect)
    {
        cpu_relax();
    }
}

static struct timeval start_time;
static struct timeval end_time;

static void calc_time(struct timeval *start, struct timeval *end)
{
    if (end->tv_usec < start->tv_usec)
    {
        end->tv_sec -= 1;
        end->tv_usec += 1000000;
    }

    assert(end->tv_sec >= start->tv_sec);
    assert(end->tv_usec >= start->tv_usec);
    struct timeval interval = {
        end->tv_sec - start->tv_sec,
        end->tv_usec - start->tv_usec};
    printf("%ld.%06ld\t", (long)interval.tv_sec, (long)interval.tv_usec);
}

// Use an array of counter to see effect on RTM if touches more cache line.
#define NCOUNTER 1
#define CACHE_LINE 64

// Use thread local counter to avoid cache contention between cores.
// For TSX, this avoids TX conflicts so the performance overhead/improvement is
// due to TSX mechanism.
static __thread int8_t counter[CACHE_LINE * NCOUNTER];

// compare_exchange 自旋锁
class spin_lock2
{
public:
    spin_lock2() {}
    ~spin_lock2() { unlock(); }
    spin_lock2(const spin_lock2 &) = delete;
    spin_lock2(const spin_lock2 &&) = delete;
    spin_lock2 &operator=(const spin_lock2 &) = delete;

    void lock()
    {
        int exp = 1;
        while (!sem_.compare_exchange_strong(exp, 0))
        {
            exp = 1;
        }
    }

    void unlock()
    {
        sem_.store(1);
    }

private:
    std::atomic_int sem_ = 1;
};

// bad 自旋锁
class bad_spin_lock
{
public:
    bad_spin_lock() {}
    ~bad_spin_lock() { unlock(); }
    bad_spin_lock(const bad_spin_lock &) = delete;
    bad_spin_lock(const bad_spin_lock &&) = delete;
    bad_spin_lock &operator=(const bad_spin_lock &) = delete;

    void lock()
    {
        while (lock_.exchange(true))
        {
        }
    }

    void unlock()
    {
        lock_.store(false);
    }

private:
    std::atomic<bool> lock_ = false;
};

Common::spin_lock sl;
spin_lock2 sl2;
bad_spin_lock bad_sl;
std::mutex mtx;

void *sl_inc_thread(void *id)
{
    int n = N_PAIR / nthr;
    assert(n * nthr == N_PAIR);
    wait_flag(&wflag, nthr);

    if (((long)id == 0))
    {
        gettimeofday(&start_time, NULL);
    }

    /* Start lock unlock test. */
    for (int i = 0; i < n; i++)
    {
        sl.lock();
        for (int j = 0; j < NCOUNTER; j++)
            counter[j * CACHE_LINE]++;
        sl.unlock();
    }

    if (__sync_fetch_and_add((uint32_t *)&wflag, -1) == 1)
    {
        gettimeofday(&end_time, NULL);
    }
    return NULL;
}

void *sl2_inc_thread(void *id)
{
    int n = N_PAIR / nthr;
    assert(n * nthr == N_PAIR);
    wait_flag(&wflag, nthr);

    if (((long)id == 0))
    {
        gettimeofday(&start_time, NULL);
    }

    /* Start lock unlock test. */
    for (int i = 0; i < n; i++)
    {
        sl2.lock();
        for (int j = 0; j < NCOUNTER; j++)
            counter[j * CACHE_LINE]++;
        sl2.unlock();
    }

    if (__sync_fetch_and_add((uint32_t *)&wflag, -1) == 1)
    {
        gettimeofday(&end_time, NULL);
    }
    return NULL;
}

void *bad_sl_inc_thread(void *id)
{
    int n = N_PAIR / nthr;
    assert(n * nthr == N_PAIR);
    wait_flag(&wflag, nthr);

    if (((long)id == 0))
    {
        gettimeofday(&start_time, NULL);
    }

    /* Start lock unlock test. */
    for (int i = 0; i < n; i++)
    {
        bad_sl.lock();
        for (int j = 0; j < NCOUNTER; j++)
            counter[j * CACHE_LINE]++;
        bad_sl.unlock();
    }

    if (__sync_fetch_and_add((uint32_t *)&wflag, -1) == 1)
    {
        gettimeofday(&end_time, NULL);
    }
    return NULL;
}

void *mtx_inc_thread(void *id)
{
    int n = N_PAIR / nthr;
    assert(n * nthr == N_PAIR);
    wait_flag(&wflag, nthr);

    if (((long)id == 0))
    {
        gettimeofday(&start_time, NULL);
    }

    /* Start lock unlock test. */
    for (int i = 0; i < n; i++)
    {
        mtx.lock();
        for (int j = 0; j < NCOUNTER; j++)
            counter[j * CACHE_LINE]++;
        mtx.unlock();
    }

    if (__sync_fetch_and_add((uint32_t *)&wflag, -1) == 1)
    {
        gettimeofday(&end_time, NULL);
    }
    return NULL;
}

void sl_test(const int thread_count)
{
    nthr = thread_count;
    printf("sl     using %d threads, ", thread_count);

    // Start thread
    pthread_t *thr = (pthread_t *)calloc(sizeof(*thr), nthr);
    for (long i = 0; i < nthr; i++)
    {
        if (pthread_create(&thr[i], NULL, sl_inc_thread, (void *)i) != 0)
        {
            perror("thread creating failed");
        }
    }

    // join thread
    for (long i = 0; i < nthr; i++)
        pthread_join(thr[i], NULL);

    calc_time(&start_time, &end_time);

    for (int i = 0; i < NCOUNTER; i++)
    {
        if (counter[i] == N_PAIR)
        {
        }
        else
        {
            printf("counter %d error\n", i);
        }
    }
}

void sl2_test(const int thread_count)
{
    nthr = thread_count;
    printf("sl2    using %d threads, ", thread_count);

    // Start thread
    pthread_t *thr = (pthread_t *)calloc(sizeof(*thr), nthr);
    for (long i = 0; i < nthr; i++)
    {
        if (pthread_create(&thr[i], NULL, sl2_inc_thread, (void *)i) != 0)
        {
            perror("thread creating failed");
        }
    }

    // join thread
    for (long i = 0; i < nthr; i++)
        pthread_join(thr[i], NULL);

    calc_time(&start_time, &end_time);

    for (int i = 0; i < NCOUNTER; i++)
    {
        if (counter[i] == N_PAIR)
        {
        }
        else
        {
            printf("counter %d error\n", i);
        }
    }
}

void bad_sl_test(const int thread_count)
{
    nthr = thread_count;
    printf("bad_sl using %d threads, ", thread_count);

    // Start thread
    pthread_t *thr = (pthread_t *)calloc(sizeof(*thr), nthr);
    for (long i = 0; i < nthr; i++)
    {
        if (pthread_create(&thr[i], NULL, bad_sl_inc_thread, (void *)i) != 0)
        {
            perror("thread creating failed");
        }
    }

    // join thread
    for (long i = 0; i < nthr; i++)
        pthread_join(thr[i], NULL);

    calc_time(&start_time, &end_time);

    for (int i = 0; i < NCOUNTER; i++)
    {
        if (counter[i] == N_PAIR)
        {
        }
        else
        {
            printf("counter %d error\n", i);
        }
    }
}

void mtx_test(const int thread_count)
{
    nthr = thread_count;
    printf("mtx    using %d threads, ", thread_count);

    // Start thread
    pthread_t *thr = (pthread_t *)calloc(sizeof(*thr), nthr);
    for (long i = 0; i < nthr; i++)
    {
        if (pthread_create(&thr[i], NULL, mtx_inc_thread, (void *)i) != 0)
        {
            perror("thread creating failed");
        }
    }

    // join thread
    for (long i = 0; i < nthr; i++)
        pthread_join(thr[i], NULL);

    calc_time(&start_time, &end_time);

    for (int i = 0; i < NCOUNTER; i++)
    {
        if (counter[i] == N_PAIR)
        {
        }
        else
        {
            printf("counter %d error\n", i);
        }
    }
}

TEST(SpinLockTest, TestThread1)
{
    sl_test(1);
    sl2_test(1);
    bad_sl_test(1);
    mtx_test(1);
}

TEST(SpinLockTest, TestThread2)
{
    sl_test(2);
    sl2_test(2);
    bad_sl_test(2);
    mtx_test(2);
}

TEST(SpinLockTest, TestThread5)
{
    sl_test(5);
    sl2_test(5);
    bad_sl_test(5);
    mtx_test(5);
}

TEST(SpinLockTest, TestThread10)
{
    sl_test(10);
    sl2_test(10);
    bad_sl_test(10);
    mtx_test(10);
}

TEST(SpinLockTest, TestThread20)
{
    sl_test(20);
    sl2_test(20);
    bad_sl_test(20);
    mtx_test(20);
}

TEST(SpinLockTest, TestThread50)
{
    sl_test(50);
    mtx_test(50);
}

TEST(SpinLockTest, TestThread100)
{
    sl_test(100);
    mtx_test(100);
}

/*
Test:
[----------] 7 tests from SpinLockTest
[ RUN      ] SpinLockTest.TestThread1
sl     using 1 threads, 0.137425        counter 0 error
sl2    using 1 threads, 0.186268        counter 0 error
bad_sl using 1 threads, 0.185816        counter 0 error
mtx    using 1 threads, 0.289596        counter 0 error
[       OK ] SpinLockTest.TestThread1 (799 ms)
[ RUN      ] SpinLockTest.TestThread2
sl     using 2 threads, 0.576911        counter 0 error
sl2    using 2 threads, 0.790349        counter 0 error
bad_sl using 2 threads, 0.824030        counter 0 error
mtx    using 2 threads, 1.278605        counter 0 error
[       OK ] SpinLockTest.TestThread2 (3470 ms)
[ RUN      ] SpinLockTest.TestThread5
sl     using 5 threads, 0.977430        counter 0 error
sl2    using 5 threads, 2.665210        counter 0 error
bad_sl using 5 threads, 2.796452        counter 0 error
mtx    using 5 threads, 1.232018        counter 0 error
[       OK ] SpinLockTest.TestThread5 (7671 ms)
[ RUN      ] SpinLockTest.TestThread10
sl     using 10 threads, 1.555525       counter 0 error
sl2    using 10 threads, 5.256823       counter 0 error
bad_sl using 10 threads, 5.417147       counter 0 error
mtx    using 10 threads, 1.135420       counter 0 error
[       OK ] SpinLockTest.TestThread10 (13365 ms)
[ RUN      ] SpinLockTest.TestThread20
sl     using 20 threads, 2.121470       counter 0 error
sl2    using 20 threads, 8.133860       counter 0 error
bad_sl using 20 threads, 9.763411       counter 0 error
mtx    using 20 threads, 1.189618       counter 0 error
[       OK ] SpinLockTest.TestThread20 (21210 ms)
[ RUN      ] SpinLockTest.TestThread50
sl     using 50 threads, 4.258198       counter 0 error
mtx    using 50 threads, 1.193728       counter 0 error
[       OK ] SpinLockTest.TestThread50 (5514 ms)
[ RUN      ] SpinLockTest.TestThread100
sl     using 100 threads, 7.925868      counter 0 error
mtx    using 100 threads, 1.201947      counter 0 error
[       OK ] SpinLockTest.TestThread100 (9377 ms)
[----------] 7 tests from SpinLockTest (61409 ms total)
*/