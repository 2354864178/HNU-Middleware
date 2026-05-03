#pragma once

#include <unistd.h>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <thread>

namespace hnu {
namespace Middleware {
namespace base {

// 读锁保护类，构造函数接受一个 RWLock 引用参数，在构造函数中调用 RWLock 的 ReadLock 方法获取读锁，在析构函数中调用 RWLock 的 ReadUnlock 方法释放读锁，禁止复制和赋值操作
template <typename RWLock> class ReadLockGuard {
    public:
    explicit ReadLockGuard(RWLock& lock) : rw_lock_(lock) {
        rw_lock_.ReadLock();
    }

    ~ReadLockGuard() {
        rw_lock_.ReadUnlock();
    }

    private:
    ReadLockGuard(const ReadLockGuard& other) = delete;
    ReadLockGuard& operator=(const ReadLockGuard& other) = delete;
    RWLock& rw_lock_; // 读锁引用
};

template <typename RWLock> class WriteLockGuard {
    public:
    explicit WriteLockGuard(RWLock& lock) : rw_lock_(lock) {
        rw_lock_.WriteLock();
    }

    ~WriteLockGuard() {
        rw_lock_.WriteUnlock();
    }

    private:
    WriteLockGuard(const WriteLockGuard& other) = delete;
    WriteLockGuard& operator=(const WriteLockGuard& other) = delete;
    RWLock& rw_lock_; // 写锁引用
};

} // namespace base
} // namespace Middleware
} // namespace hnu
