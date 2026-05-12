#include "../unbounded_queue.h"

#include <atomic>
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

namespace hnu {
namespace Middleware {
namespace base {

void TestEmptyAndSequentialFIFO() {
    UnboundedQueue<int> queue;

    int value = 0;
    assert(!queue.Dequeue(&value));

    queue.Enqueue(10);
    queue.Enqueue(20);
    queue.Enqueue(30);

    assert(queue.Size() == 3);
    assert(queue.Dequeue(&value));
    assert(value == 10);
    assert(queue.Dequeue(&value));
    assert(value == 20);
    assert(queue.Dequeue(&value));
    assert(value == 30);

    assert(queue.Empty());
    assert(!queue.Dequeue(&value));
}

void TestConcurrentProducersSingleConsumer() {
    UnboundedQueue<int> queue;
    constexpr int producer_count = 4;
    constexpr int values_per_producer = 1000;
    constexpr int total_count = producer_count * values_per_producer;

    std::atomic<int> produced{0};
    std::atomic<int> consumed{0};
    std::atomic<long long> sum{0};

    std::vector<std::thread> producers;
    producers.reserve(producer_count);
    for (int p = 0; p < producer_count; ++p) {
        producers.emplace_back([&, p]() {
            for (int i = 1; i <= values_per_producer; ++i) {
                queue.Enqueue(p * values_per_producer + i);
                produced.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    std::thread consumer([&]() {
        int value = 0;
        while (consumed.load(std::memory_order_acquire) < total_count) {
            if (queue.Dequeue(&value)) {
                sum.fetch_add(value, std::memory_order_relaxed);
                consumed.fetch_add(1, std::memory_order_release);
            } else {
                std::this_thread::yield();
            }
        }
    });

    for (auto& t : producers) {
        t.join();
    }
    consumer.join();

    assert(produced.load(std::memory_order_acquire) == total_count);
    assert(consumed.load(std::memory_order_acquire) == total_count);

    long long expected_sum = 0;
    for (int p = 0; p < producer_count; ++p) {
        for (int i = 1; i <= values_per_producer; ++i) {
            expected_sum += (p * values_per_producer + i);
        }
    }
    assert(sum.load(std::memory_order_acquire) == expected_sum);
    assert(queue.Empty());
}

} // namespace base
} // namespace Middleware
} // namespace hnu

int main() {
    hnu::Middleware::base::TestEmptyAndSequentialFIFO();
    hnu::Middleware::base::TestConcurrentProducersSingleConsumer();

    std::cout << "UnboundedQueue tests passed" << std::endl;
    return 0;
}
