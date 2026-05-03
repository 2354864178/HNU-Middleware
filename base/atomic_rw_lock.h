#pragma once

#include "rw_lock_guard.h"

namespace hnu {
namespace Middleware {
namespace base {

class AtomicRWLock {
    friend class ReadLockGuard<AtomicRWLock>;
    friend class WriteLockGuard<AtomicRWLock>;

    public:
    static const int32_t RW_LOCK_FREE = 0;     // 锁的空闲状态，表示没有线程持有读锁或写锁
    static const int32_t WRITE_EXCLUSIVE = -1; // 写锁的状态，表示有一个线程持有写锁，其他线程不能获取读锁或写锁
    static const uint32_t MAX_RETRY_TIMES = 5; // 获取锁失败的最大重试次数，超过这个次数后让出 CPU 时间片，避免长时间占用 CPU 资源
    AtomicRWLock() {}
    explicit AtomicRWLock(bool write_first) : write_first_(write_first) {}

    private:
    void ReadLock();  // 获取读锁，允许多个线程同时持有读锁，但不允许写锁存在
    void WriteLock(); // 获取写锁，要求当前没有其他线程持有读锁或写锁，写锁具有排他性

    void ReadUnlock();  // 释放读锁，减少持有读锁的线程数量，如果没有线程持有读锁，则允许写锁获取
    void WriteUnlock(); // 释放写锁，允许其他线程获取读锁或写锁

    AtomicRWLock(const AtomicRWLock&) = delete;
    AtomicRWLock& operator=(const AtomicRWLock&) = delete;
    std::atomic<uint32_t> write_lock_wait_num_ = {0}; // 等待写锁的线程数量，初始值为 0，表示没有线程等待写锁，当有线程等待写锁时，读锁获取需要等待写锁释放，确保写锁优先级高于读锁
    std::atomic<int32_t> lock_num_ = {0};             // 当前锁的状态，初始值为0，表示没有线程持有读锁或写锁，正数表示持有读锁的线程数量，负数表示持有写锁的线程数量（通常为 -1）
    bool write_first_ = true;
};

inline void AtomicRWLock::ReadLock() {
    uint32_t retry_times = 0;
    int32_t lock_num = lock_num_.load();

    // 封装「是否需要等待」的条件判断，消除分支重复
    auto need_wait = [this, &lock_num]() {
        const bool has_write_lock = lock_num < RW_LOCK_FREE;
        const bool has_write_waiter = write_first_ && (write_lock_wait_num_.load() > 0);
        return has_write_lock || has_write_waiter;
    };

    // 统一的自旋+CAS逻辑
    do {
        // 自旋等待直到条件满足
        while (need_wait()) {
            if (++retry_times == MAX_RETRY_TIMES) {
                std::this_thread::yield();
                retry_times = 0;
            }
            lock_num = lock_num_.load();
        }
    } while (!lock_num_.compare_exchange_weak(lock_num, lock_num + 1, std::memory_order_acq_rel, std::memory_order_relaxed));
}

// // 获取读锁，允许多个线程同时持有读锁，但不允许写锁存在
// inline void AtomicRWLock::ReadLock() {
//     uint32_t retry_times = 0;            // 读锁获取失败的重试次数，超过一定次数后让出 CPU 时间片，避免长时间占用 CPU 资源
//     int32_t lock_num = lock_num_.load(); // 当前锁的状态，初始值为 RW_LOCK_FREE，表示没有线程持有读锁或写锁，正数表示持有读锁的线程数量，负数表示持有写锁的线程数量（通常为 -1）

//     // 如果写锁优先，读锁获取时需要等待写锁释放，确保写锁优先级高于读锁
//     if (write_first_) {
//         // 尝试将锁的状态从 lock_num 增加 1，表示当前线程获取了一个读锁
//         // 如果 CAS 操作成功，说明获取读锁成功，返回；如果 CAS 操作失败，说明锁的状态发生了变化，可能是有线程持有写锁或者有线程等待写锁，此时需要重新检查锁的状态，直到满足获取读锁的条件，即没有线程持有写锁且没有线程等待写锁
//         do {
//             // 如果当前锁的状态小于 RW_LOCK_FREE，表示有线程持有写锁，或者有线程等待写锁，此时读锁获取需要等待，直到没有线程持有写锁且没有线程等待写锁
//             while (lock_num < RW_LOCK_FREE || write_lock_wait_num_.load() > 0) {
//                 // 如果重试次数达到最大值，说明读锁获取失败过多次，此时让出 CPU 时间片，允许其他线程执行，避免长时间占用 CPU 资源，然后重置重试次数
//                 if (++retry_times == MAX_RETRY_TIMES) {
//                     std::this_thread::yield();
//                     retry_times = 0;
//                 }
//                 lock_num = lock_num_.load(); // 重新加载锁的状态，检查是否满足获取读锁的条件，即没有线程持有写锁且没有线程等待写锁
//             }
//         } while (!lock_num_.compare_exchange_weak(lock_num, lock_num + 1, std::memory_order_acq_rel, std::memory_order_relaxed));
//     } else {
//         do {
//             while (lock_num < RW_LOCK_FREE) {
//                 if (++retry_times == MAX_RETRY_TIMES) {
//                     std::this_thread::yield();
//                     retry_times = 0;
//                 }
//                 lock_num = lock_num_.load();
//             }
//         } while (!lock_num_.compare_exchange_weak(lock_num, lock_num + 1, std::memory_order_acq_rel, std::memory_order_relaxed));
//     }
// }

// 获取写锁，要求当前没有其他线程持有读锁或写锁，写锁具有排他性
inline void AtomicRWLock::WriteLock() {
    int32_t rw_lock_free = RW_LOCK_FREE;
    uint32_t retry_times = 0;
    write_lock_wait_num_.fetch_add(1);
    while (!lock_num_.compare_exchange_weak(rw_lock_free, WRITE_EXCLUSIVE, std::memory_order_acq_rel, std::memory_order_relaxed)) {
        // rw_lock_free will change after CAS fail, so init agin
        rw_lock_free = RW_LOCK_FREE;
        if (++retry_times == MAX_RETRY_TIMES) {
            // saving cpu
            std::this_thread::yield();
            retry_times = 0;
        }
    }
    write_lock_wait_num_.fetch_sub(1);
}

inline void AtomicRWLock::ReadUnlock() {
    lock_num_.fetch_sub(1);
}

inline void AtomicRWLock::WriteUnlock() {
    lock_num_.fetch_add(1);
}

} // namespace base
} // namespace Middleware
} // namespace hnu
