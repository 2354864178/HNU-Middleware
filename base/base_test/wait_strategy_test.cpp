#include "../wait_strategy.h"

#include <atomic>
#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>

namespace hnu {
namespace Middleware {
namespace base {

void TestSleepWaitStrategy() {
    SleepWaitStrategy strategy(2000);
    const auto start = std::chrono::steady_clock::now();
    const bool ok = strategy.EmptyWait();
    const auto elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count();

    assert(ok);
    assert(elapsed_us >= 1500);
}

void TestYieldAndBusySpinWaitStrategy() {
    YieldWaitStrategy yield_strategy;
    BusySpinWaitStrategy busy_strategy;

    assert(yield_strategy.EmptyWait());
    assert(busy_strategy.EmptyWait());
}

void TestBlockWaitStrategyNotify() {
    BlockWaitStrategy strategy;
    std::atomic<bool> awakened{false};

    std::thread waiter([&]() {
        const bool ok = strategy.EmptyWait();
        awakened.store(ok, std::memory_order_release);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    strategy.NotifyOne();
    waiter.join();

    assert(awakened.load(std::memory_order_acquire));
}

void TestTimeoutBlockWaitStrategyTimeoutAndNotify() {
    TimeoutBlockWaitStrategy timeout_strategy(20);
    const auto timeout_start = std::chrono::steady_clock::now();
    const bool timeout_ok = timeout_strategy.EmptyWait();
    const auto timeout_elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - timeout_start).count();

    assert(!timeout_ok);
    assert(timeout_elapsed_ms >= 15);

    TimeoutBlockWaitStrategy notify_strategy(200);
    std::atomic<bool> notify_result{false};
    std::thread waiter([&]() {
        const bool ok = notify_strategy.EmptyWait();
        notify_result.store(ok, std::memory_order_release);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    notify_strategy.NotifyOne();
    waiter.join();

    assert(notify_result.load(std::memory_order_acquire));
}

} // namespace base
} // namespace Middleware
} // namespace hnu

int main() {
    hnu::Middleware::base::TestSleepWaitStrategy();
    hnu::Middleware::base::TestYieldAndBusySpinWaitStrategy();
    hnu::Middleware::base::TestBlockWaitStrategyNotify();
    hnu::Middleware::base::TestTimeoutBlockWaitStrategyTimeoutAndNotify();

    std::cout << "WaitStrategy tests passed" << std::endl;
    return 0;
}
