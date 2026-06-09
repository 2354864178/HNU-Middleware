#pragma once

#include <atomic>
#include <cstdint>

namespace hnu {
namespace Middleware {
namespace transport {

// Block代表一个内存块，包含消息数据和消息头数据，以及一个读写锁来控制对这个内存块的访问
class Block {
    friend class Segment;

public:
    Block();
    virtual ~Block();

    // 获取消息大小
    uint64_t msg_size() const {
        return msg_size_;
    }

    // 设置消息大小
    void set_msg_size(uint64_t msg_size) {
        msg_size_ = msg_size;
    }

    // 获取消息头大小
    uint64_t msg_info_size() const {
        return msg_info_size_;
    }

    // 设置消息头大小
    void set_msg_info_size(uint64_t msg_info_size) {
        msg_info_size_ = msg_info_size;
    }

    static const int32_t kRWLockFree;      // 读写锁空闲状态
    static const int32_t kWriteExclusive;  // 写锁独占状态
    static const int32_t kMaxTryLockTimes; // 尝试获取锁的最大次数，超过这个次数就放弃获取锁

private:
    bool TryLockForWrite();  // 尝试获取写锁，如果成功返回true，失败返回false
    bool TryLockForRead();   // 尝试获取读锁，如果成功返回true，失败返回false
    void ReleaseWriteLock(); // 释放写锁
    void ReleaseReadLock();  // 释放读锁

    std::atomic<int32_t> lock_num_ = {0}; // 锁的状态，0表示空闲，-1表示写锁独占，正数表示读锁数量
    uint64_t msg_size_;                   // 消息的大小，单位是字节
    uint64_t msg_info_size_;              // 消息头的大小，单位是字节
};

} // namespace transport
} // namespace Middleware
} // namespace hnu
