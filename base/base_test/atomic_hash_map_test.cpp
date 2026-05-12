#include "../atomic_hash_map.h"

#include <atomic>
#include <cassert>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace hnu {
namespace Middleware {
namespace base {

void TestBasicInsertAndGet() {
    AtomicHashMap<int, std::string, 8> map;

    map.Set(1, std::string("one"));
    map.Set(2, std::string("two"));

    std::string value;
    assert(map.HAS(1));
    assert(map.HAS(2));
    assert(map.Get(1, &value));
    assert(value == "one");
    assert(map.Get(2, &value));
    assert(value == "two");
}

void TestOverwriteExistingKey() {
    AtomicHashMap<int, std::string, 8> map;

    map.Set(3, std::string("old"));
    map.Set(3, std::string("new"));

    std::string value;
    assert(map.Get(3, &value));
    assert(value == "new");
}

void TestCollisionBucket() {
    AtomicHashMap<int, int, 8> map;

    map.Set(1, 11);
    map.Set(9, 99);

    int value = 0;
    assert(map.Get(1, &value));
    assert(value == 11);
    assert(map.Get(9, &value));
    assert(value == 99);
}

void TestConcurrentInsert() {
    constexpr int thread_count = 4;
    constexpr int per_thread = 256;

    AtomicHashMap<int, int, 256> map;
    std::atomic<int> ready{0};
    std::atomic<bool> start{false};

    std::vector<std::thread> workers;
    workers.reserve(thread_count);
    for (int t = 0; t < thread_count; ++t) {
        workers.emplace_back([&, t]() {
            ready.fetch_add(1, std::memory_order_relaxed);
            while (!start.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }

            for (int i = 0; i < per_thread; ++i) {
                int key = t * per_thread + i;
                map.Set(key, key * 10);
            }
        });
    }
    while (ready.load(std::memory_order_acquire) < thread_count) {
        std::this_thread::yield();
    }
    start.store(true, std::memory_order_release);

    for (auto& worker : workers) {
        worker.join();
    }

    for (int key = 0; key < thread_count * per_thread; ++key) {
        int value = 0;
        assert(map.Get(key, &value));
        assert(value == key * 10);
    }
}

} // namespace base
} // namespace Middleware
} // namespace hnu

int main() {
    hnu::Middleware::base::TestBasicInsertAndGet();
    hnu::Middleware::base::TestOverwriteExistingKey();
    hnu::Middleware::base::TestCollisionBucket();
    hnu::Middleware::base::TestConcurrentInsert();

    std::cout << "AtomicHashMap tests passed" << std::endl;
    return 0;
}
