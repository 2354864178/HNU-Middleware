#include "../rw_lock_guard.h"

#include <atomic>
#include <cassert>
#include <iostream>

namespace hnu {
namespace Middleware {
namespace base {

class FakeRWLock {
    public:
    void ReadLock() {
        read_lock_count.fetch_add(1, std::memory_order_relaxed);
        active_readers.fetch_add(1, std::memory_order_relaxed);
    }

    void ReadUnlock() {
        read_unlock_count.fetch_add(1, std::memory_order_relaxed);
        active_readers.fetch_sub(1, std::memory_order_relaxed);
    }

    void WriteLock() {
        write_lock_count.fetch_add(1, std::memory_order_relaxed);
        active_writers.fetch_add(1, std::memory_order_relaxed);
    }

    void WriteUnlock() {
        write_unlock_count.fetch_add(1, std::memory_order_relaxed);
        active_writers.fetch_sub(1, std::memory_order_relaxed);
    }

    std::atomic<int> read_lock_count{0};
    std::atomic<int> read_unlock_count{0};
    std::atomic<int> write_lock_count{0};
    std::atomic<int> write_unlock_count{0};
    std::atomic<int> active_readers{0};
    std::atomic<int> active_writers{0};
};

void TestReadLockGuardRAII() {
    FakeRWLock lock;
    {
        ReadLockGuard<FakeRWLock> guard(lock);
        assert(lock.read_lock_count.load(std::memory_order_acquire) == 1);
        assert(lock.active_readers.load(std::memory_order_acquire) == 1);
        assert(lock.read_unlock_count.load(std::memory_order_acquire) == 0);
    }

    assert(lock.read_unlock_count.load(std::memory_order_acquire) == 1);
    assert(lock.active_readers.load(std::memory_order_acquire) == 0);
}

void TestWriteLockGuardRAII() {
    FakeRWLock lock;
    {
        WriteLockGuard<FakeRWLock> guard(lock);
        assert(lock.write_lock_count.load(std::memory_order_acquire) == 1);
        assert(lock.active_writers.load(std::memory_order_acquire) == 1);
        assert(lock.write_unlock_count.load(std::memory_order_acquire) == 0);
    }

    assert(lock.write_unlock_count.load(std::memory_order_acquire) == 1);
    assert(lock.active_writers.load(std::memory_order_acquire) == 0);
}

void TestNestedReadGuards() {
    FakeRWLock lock;
    {
        ReadLockGuard<FakeRWLock> g1(lock);
        {
            ReadLockGuard<FakeRWLock> g2(lock);
            assert(lock.active_readers.load(std::memory_order_acquire) == 2);
        }
        assert(lock.active_readers.load(std::memory_order_acquire) == 1);
    }
    assert(lock.active_readers.load(std::memory_order_acquire) == 0);
    assert(lock.read_lock_count.load(std::memory_order_acquire) == 2);
    assert(lock.read_unlock_count.load(std::memory_order_acquire) == 2);
}

} // namespace base
} // namespace Middleware
} // namespace hnu

int main() {
    hnu::Middleware::base::TestReadLockGuardRAII();
    hnu::Middleware::base::TestWriteLockGuardRAII();
    hnu::Middleware::base::TestNestedReadGuards();

    std::cout << "RWLockGuard tests passed" << std::endl;
    return 0;
}
