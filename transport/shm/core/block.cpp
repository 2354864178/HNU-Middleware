#include <iostream>

#include "block.h"
#include "common/log.h"

namespace hnu {
namespace Middleware {
namespace transport {

const int32_t Block::kRWLockFree = 0;      // 代表没有线程持有锁，既没有线程在读，也没有线程在写
const int32_t Block::kWriteExclusive = -1; // 代表正在写
const int32_t Block::kMaxTryLockTimes = 5; // 代表尝试获取锁的最大次数，超过这个次数就放弃获取锁，避免死循环

Block::Block() : msg_size_(0), msg_info_size_(0) {}

Block::~Block() {}

// 对block加上写锁
bool Block::TryLockForWrite() {
    int32_t rw_lock_free = kRWLockFree;
    // “写者排他、读者计数” ，写的优先级比读高，写的时候不能有其他线程在读或者在写
    if (!lock_num_.compare_exchange_weak(rw_lock_free, kWriteExclusive, std::memory_order_acq_rel, std::memory_order_relaxed)) {
        ADEBUG << "lock_num_: " << lock_num_.load(); // 输出当前锁的状态，帮助调试
        return false;
    }
    return true;
}

// 加block加上读锁
bool Block::TryLockForRead() {
    int32_t lock_num = lock_num_.load();
    /*
    约定：kRWLockFree = 0（空闲），kWriteExclusive = -1（写者独占）。
    检查 lock_num_ < kRWLockFree（即 < 0）就是在问“当前是否有写者占用？”。
    如果为 true，说明正在写（-1），读者应失败或等待；若为 false（≥0），说明没有写者
    */
    if (lock_num < kRWLockFree) {
        // 如果有其他线程正在写，则直接return，写的优先级比读高
        AINFO << "block is being written";
        return false;
    }
    int32_t try_times = 0;
    // 如果此时没有线程持有锁，则将lock_num_的值加1
    while (!lock_num_.compare_exchange_weak(lock_num, lock_num + 1, std::memory_order_acq_rel, std::memory_order_relaxed)) {
        // 进入循环内部说明 lock_num_！= lock_num ，有另外一个线程拿到了锁
        ++try_times;
        // 如果循环五次还没拿到读锁，则return
        if (try_times == kMaxTryLockTimes) {
            AINFO << "fail to add read lock num, curr num: " << lock_num;
            return false;
        }

        // 再加载一次lock_num_
        lock_num = lock_num_.load();

        if (lock_num < kRWLockFree) {
            // 如果此时另外一个线程在写，直接跳出循环，不允许读
            AINFO << "block is being written";
            return false;
        }
    }

    return true;
}

// 释放读锁,将lock_num_-1
void Block::ReleaseReadLock() {
    lock_num_.fetch_sub(1);
}

// 释放写锁,将lock_num_+1
void Block::ReleaseWriteLock() {
    lock_num_.fetch_add(1);
}

} // namespace transport
} // namespace Middleware
} // namespace hnu