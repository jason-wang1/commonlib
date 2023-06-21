#include <gtest/gtest.h>
#include "TimingWheel/DelayQueue.hpp"
#include <thread>

TEST(DelayQueueTest, TestOfferTake)
{
    DelayQueue queue;

    // Insert items with different delays.
    queue.offer(1, std::chrono::milliseconds(5));
    queue.offer(2, std::chrono::milliseconds(1));
    queue.offer(3, std::chrono::milliseconds(3));
    queue.offer(4, std::chrono::milliseconds(2));

    // Take items from the queue and verify the order and delay time.
    auto begin_time = std::chrono::steady_clock::now();
    auto result = queue.take();
    ASSERT_EQ(std::any_cast<int>(result.value()), 2);
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin_time).count();
    ASSERT_EQ(duration, 1);

    result = queue.take();
    ASSERT_EQ(std::any_cast<int>(result.value()), 4);
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin_time).count();
    ASSERT_EQ(duration, 2);

    result = queue.take();
    ASSERT_EQ(std::any_cast<int>(result.value()), 3);
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin_time).count();
    ASSERT_EQ(duration, 3);

    result = queue.take();
    ASSERT_EQ(std::any_cast<int>(result.value()), 1);
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin_time).count();
    ASSERT_EQ(duration, 5);
}

TEST(DelayQueueTest, TestOfferWhenThreadTake)
{
    DelayQueue queue;
    std::thread t(
        [&queue]
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            queue.offer(42, std::chrono::milliseconds(10));
        });

    // 证明了take阻塞当前线程, 但不会影响其他线程offer
    auto start = std::chrono::steady_clock::now();
    auto result = queue.take();
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    ASSERT_EQ(42, std::any_cast<int>(result.value()));
    ASSERT_EQ(duration, 20);

    t.join();
}

TEST(DelayQueueTest, OfferAndTakeInt)
{
    DelayQueue q;
    q.offer(1, std::chrono::milliseconds(5));
    ASSERT_FALSE(q.empty());

    auto start = std::chrono::steady_clock::now();
    auto result = q.take();
    auto end = std::chrono::steady_clock::now();
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(std::any_cast<int>(result.value()), 1);
    ASSERT_LE(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(), 5);
}

TEST(DelayQueueTest, PollEmpty)
{
    DelayQueue q;
    auto result = q.poll();
    ASSERT_FALSE(result.has_value());
}

TEST(DelayQueueTest, PollNotExpired)
{
    DelayQueue q;
    q.offer(1, std::chrono::milliseconds(5));
    ASSERT_FALSE(q.empty());

    auto result = q.poll();
    ASSERT_FALSE(result.has_value());
}

TEST(DelayQueueTest, PollExpired)
{
    DelayQueue q;
    q.offer(1, std::chrono::milliseconds(5));
    ASSERT_FALSE(q.empty());

    std::this_thread::sleep_for(std::chrono::milliseconds(6));

    auto result = q.poll();
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(std::any_cast<int>(result.value()), 1);
}

TEST(DelayQueueTest, TestSize)
{
    DelayQueue queue;
    ASSERT_EQ(queue.size(), 0);

    queue.offer(10, std::chrono::milliseconds(10));
    ASSERT_EQ(queue.size(), 1);

    queue.offer(20, std::chrono::milliseconds(20));
    ASSERT_EQ(queue.size(), 2);

    queue.offer(30, std::chrono::milliseconds(5));
    ASSERT_EQ(queue.size(), 3);

    queue.take();
    ASSERT_EQ(queue.size(), 2);

    queue.take();
    ASSERT_EQ(queue.size(), 1);

    queue.take();
    ASSERT_EQ(queue.size(), 0);
}

TEST(DelayQueueTest, TestEmpty)
{
    DelayQueue q;
    ASSERT_TRUE(q.empty());

    q.offer(1, std::chrono::milliseconds(10));
    ASSERT_FALSE(q.empty());

    q.take();
    ASSERT_TRUE(q.empty());
}

TEST(DelayQueueTest, TakeMultiThreadTest)
{
    const int num_threads = 5;
    const int num_items_per_thread = 100;
    const int max_delay_ms = 30;

    DelayQueue queue;

    std::vector<std::thread> threads;
    std::atomic<int> total_items{0};

    // 生产者线程
    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back(
            [&]()
            {
                for (int j = 0; j < num_items_per_thread; ++j)
                {
                    const auto delay = std::chrono::milliseconds(std::rand() % max_delay_ms);
                    queue.offer(j, delay);
                    total_items++;
                }
            });
    }

    // 消费者线程
    threads.emplace_back(
        [&]()
        {
            int count = 0;
            while (count < num_threads * num_items_per_thread)
            {
                const auto item = queue.take();
                if (item.has_value())
                {
                    count++;
                }
            }
        });

    for (auto &thread : threads)
    {
        thread.join();
    }

    EXPECT_EQ(queue.size(), 0);
    EXPECT_EQ(total_items, num_threads * num_items_per_thread);
}

TEST(DelayQueueTest, MultipleProducersAndConsumers)
{
    DelayQueue queue;

    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    const int num_producers = 5;
    const int num_consumers = 3;

    const int num_elements_per_producer = 1000;
    const int max_delay_ms = 30;

    std::atomic<int> produced_count = 0;
    std::atomic<int> consumed_count = 0;
    std::atomic<int> produced_value = 0;
    std::atomic<int> consumed_value = 0;

    // Create producer threads
    for (int i = 0; i < num_producers; ++i)
    {
        producers.emplace_back(
            [&, i]()
            {
                for (int j = 0; j < num_elements_per_producer; ++j)
                {
                    const auto value = i * num_elements_per_producer + j;
                    const auto delay = std::chrono::milliseconds(std::rand() % max_delay_ms);
                    queue.offer(value, delay);
                    produced_count++;
                    produced_value += value;
                    std::this_thread::yield();
                }
            });
    }

    // Create consumer threads
    for (int i = 0; i < num_consumers; ++i)
    {
        consumers.emplace_back(
            [&]()
            {
                while (consumed_count < num_producers * num_elements_per_producer)
                {
                    std::optional<std::any> item = queue.poll();
                    if (item != std::nullopt)
                    {
                        const int value = std::any_cast<int>(item.value());
                        ASSERT_TRUE(value >= 0);
                        ASSERT_TRUE(value < num_producers * num_elements_per_producer);
                        consumed_count++;
                        consumed_value += value;
                    }
                    else
                    {
                        std::this_thread::yield();
                    }
                }
            });
    }

    // Wait for all producers to finish
    for (auto &producer : producers)
    {
        producer.join();
    }

    // Wait for all consumers to finish
    for (auto &consumer : consumers)
    {
        consumer.join();
    }

    // Ensure all items were produced and consumed
    ASSERT_EQ(num_producers * num_elements_per_producer, produced_count.load());
    ASSERT_EQ(num_producers * num_elements_per_producer, consumed_count.load());
    ASSERT_EQ(produced_value, consumed_value);
}
