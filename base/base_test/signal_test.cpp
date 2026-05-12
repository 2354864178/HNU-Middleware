#include "../signal.h"

#include <atomic>
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

namespace hnu {
namespace Middleware {
namespace base {

void TestConnectAndEmit() {
    Signal<int> signal;
    std::atomic<int> sum{0};

    auto conn1 = signal.Connect([&](int v) { sum.fetch_add(v, std::memory_order_relaxed); });
    auto conn2 = signal.Connect([&](int v) { sum.fetch_add(v * 2, std::memory_order_relaxed); });

    assert(conn1.IsConnected());
    assert(conn2.IsConnected());

    signal(3);
    assert(sum.load(std::memory_order_acquire) == 9);
}

void TestDisconnectByConnection() {
    Signal<int> signal;
    std::atomic<int> sum{0};

    auto conn1 = signal.Connect([&](int v) { sum.fetch_add(v, std::memory_order_relaxed); });
    auto conn2 = signal.Connect([&](int v) { sum.fetch_add(v * 10, std::memory_order_relaxed); });

    assert(conn2.Disconnect());
    assert(!conn2.IsConnected());

    signal(2);
    assert(sum.load(std::memory_order_acquire) == 2);

    assert(conn1.IsConnected());
}

void TestDisconnectAllSlots() {
    Signal<int> signal;
    std::atomic<int> calls{0};

    signal.Connect([&](int) { calls.fetch_add(1, std::memory_order_relaxed); });
    signal.Connect([&](int) { calls.fetch_add(1, std::memory_order_relaxed); });

    signal.DisconnectAllSlots();
    signal(1);
    assert(calls.load(std::memory_order_acquire) == 0);
}

void TestConcurrentEmit() {
    Signal<int> signal;
    std::atomic<int> calls{0};

    signal.Connect([&](int) { calls.fetch_add(1, std::memory_order_relaxed); });

    constexpr int thread_count = 4;
    constexpr int emit_per_thread = 500;
    std::vector<std::thread> threads;
    threads.reserve(thread_count);

    for (int t = 0; t < thread_count; ++t) {
        threads.emplace_back([&]() {
            for (int i = 0; i < emit_per_thread; ++i) {
                signal(i);
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    assert(calls.load(std::memory_order_acquire) == thread_count * emit_per_thread);
}

} // namespace base
} // namespace Middleware
} // namespace hnu

int main() {
    hnu::Middleware::base::TestConnectAndEmit();
    hnu::Middleware::base::TestDisconnectByConnection();
    hnu::Middleware::base::TestDisconnectAllSlots();
    hnu::Middleware::base::TestConcurrentEmit();

    std::cout << "Signal tests passed" << std::endl;
    return 0;
}
