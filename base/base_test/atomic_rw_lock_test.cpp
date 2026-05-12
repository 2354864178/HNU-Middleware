#include "../atomic_rw_lock.h"

#include <atomic>
#include <cassert>
#include <chrono>
#include <functional>
#include <iostream>
#include <thread>
#include <vector>

namespace hnu {
namespace Middleware {
namespace base {

bool WaitUntil(const std::function<bool()>& pred, int timeout_ms) {
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
    while (std::chrono::steady_clock::now() < deadline) {
        if (pred()) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return pred();
}

void TestMultipleReadersCanRunTogether() {
    AtomicRWLock lock;
    constexpr int reader_count = 6;

    std::atomic<int> active_readers{0};
    std::atomic<int> max_active_readers{0};
    std::atomic<bool> start{false};
    std::atomic<int> ready{0};

    std::vector<std::thread> readers;
    readers.reserve(reader_count);
    for (int i = 0; i < reader_count; ++i) {
        readers.emplace_back([&]() {
            ready.fetch_add(1, std::memory_order_relaxed);
            while (!start.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }

            ReadLockGuard<AtomicRWLock> read_guard(lock);
            const int cur = active_readers.fetch_add(1, std::memory_order_acq_rel) + 1;

            int old_max = max_active_readers.load(std::memory_order_relaxed);
            while (cur > old_max && !max_active_readers.compare_exchange_weak(old_max, cur, std::memory_order_acq_rel, std::memory_order_relaxed)) {
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            active_readers.fetch_sub(1, std::memory_order_acq_rel);
        });
    }

    while (ready.load(std::memory_order_acquire) < reader_count) {
        std::this_thread::yield();
    }
    start.store(true, std::memory_order_release);

    for (auto& t : readers) {
        t.join();
    }

    assert(max_active_readers.load(std::memory_order_acquire) > 1);
}

void TestWritersAreExclusive() {
    AtomicRWLock lock;
    constexpr int thread_count = 4;
    constexpr int increments_per_thread = 5000;

    int counter = 0;
    std::vector<std::thread> writers;
    writers.reserve(thread_count);

    for (int t = 0; t < thread_count; ++t) {
        writers.emplace_back([&]() {
            for (int i = 0; i < increments_per_thread; ++i) {
                WriteLockGuard<AtomicRWLock> write_guard(lock);
                ++counter;
            }
        });
    }

    for (auto& t : writers) {
        t.join();
    }

    assert(counter == thread_count * increments_per_thread);
}

void TestReadWriteConsistency() {
    AtomicRWLock lock;
    std::atomic<bool> stop{false};

    struct SharedPair {
        int a;
        int b;
    } pair{0, 0};

    std::thread writer([&]() {
        for (int i = 1; i <= 10000; ++i) {
            WriteLockGuard<AtomicRWLock> write_guard(lock);
            pair.a = i;
            pair.b = i;
        }
        stop.store(true, std::memory_order_release);
    });

    constexpr int reader_count = 4;
    std::vector<std::thread> readers;
    readers.reserve(reader_count);
    for (int i = 0; i < reader_count; ++i) {
        readers.emplace_back([&]() {
            while (!stop.load(std::memory_order_acquire)) {
                ReadLockGuard<AtomicRWLock> read_guard(lock);
                assert(pair.a == pair.b);
            }
        });
    }

    writer.join();
    for (auto& t : readers) {
        t.join();
    }

    ReadLockGuard<AtomicRWLock> read_guard(lock);
    assert(pair.a == 10000);
    assert(pair.b == 10000);
}

void TestWriteFirstBlocksLateReaders() {
    AtomicRWLock lock(true);

    std::atomic<bool> hold_reader{true};
    std::atomic<bool> writer_attempting{false};
    std::atomic<bool> writer_acquired{false};
    std::atomic<bool> writer_released{false};
    std::atomic<bool> late_reader_acquired{false};

    std::thread first_reader([&]() {
        ReadLockGuard<AtomicRWLock> read_guard(lock);
        while (hold_reader.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
    });

    std::thread writer([&]() {
        writer_attempting.store(true, std::memory_order_release);
        {
            WriteLockGuard<AtomicRWLock> write_guard(lock);
            writer_acquired.store(true, std::memory_order_release);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        writer_released.store(true, std::memory_order_release);
    });

    assert(WaitUntil([&]() { return writer_attempting.load(std::memory_order_acquire); }, 200));

    std::thread late_reader([&]() {
        ReadLockGuard<AtomicRWLock> read_guard(lock);
        late_reader_acquired.store(true, std::memory_order_release);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    assert(!late_reader_acquired.load(std::memory_order_acquire));

    hold_reader.store(false, std::memory_order_release);
    first_reader.join();

    assert(WaitUntil([&]() { return writer_acquired.load(std::memory_order_acquire); }, 500));
    assert(!late_reader_acquired.load(std::memory_order_acquire));

    writer.join();
    assert(writer_released.load(std::memory_order_acquire));

    late_reader.join();
    assert(late_reader_acquired.load(std::memory_order_acquire));
}

} // namespace base
} // namespace Middleware
} // namespace hnu

int main() {
    hnu::Middleware::base::TestMultipleReadersCanRunTogether();
    hnu::Middleware::base::TestWritersAreExclusive();
    hnu::Middleware::base::TestReadWriteConsistency();
    hnu::Middleware::base::TestWriteFirstBlocksLateReaders();

    std::cout << "AtomicRWLock tests passed" << std::endl;
    return 0;
}
