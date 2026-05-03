#pragma once

#include <unistd.h>
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <utility>

#include "wait_strategy.h"
#include "macros.h"

namespace hnu {
namespace Middleware {
namespace base {

// 有界队列，使用循环数组实现，支持多生产者多消费者场景
template <typename T> class BoundedQueue {
    public:
    using value_type = T;       // 定义 value_type 类型别名，表示队列中元素的类型
    using size_type = uint64_t; // 定义 size_type 类型别名，表示队列大小的类型

    public:
    BoundedQueue() {}
    BoundedQueue& operator=(const BoundedQueue& other) = delete;
    BoundedQueue(const BoundedQueue& other) = delete;
    ~BoundedQueue();
    bool Init(uint64_t size);                         // 初始化队列，指定队列大小，默认使用睡眠等待策略
    bool Init(uint64_t size, WaitStrategy* strategy); // 初始化队列，指定队列大小和等待策略
    bool Enqueue(const T& element);                   // 入队操作，向队列尾部添加一个元素，线程安全
    bool Enqueue(T&& element);                        // 入队操作，支持移动语义，向队列尾部添加一个元素，线程安全
    bool WaitEnqueue(const T& element);               // 基于等待策略的入队操作，如果队列满了则执行等待策略，直到入队成功或超时
    bool WaitEnqueue(T&& element);                    // 基于等待策略的入队操作，如果队列满了则执行等待策略，直到入队成功或超时
    bool Dequeue(T* element);                         // 出队操作，从队列头部移除一个元素并返回，线程安全，如果队列为空则返回 false
    bool WaitDequeue(T* element);                     // 基于等待策略的出队操作，如果队列空了则执行等待策略，直到出队成功或超时
    uint64_t Size();
    bool Empty(); // 判断队列是否为空
    void SetWaitStrategy(WaitStrategy* WaitStrategy);
    void BreakAllWait();
    uint64_t Head() {
        return head_.load();
    }
    uint64_t Tail() {
        return tail_.load();
    }
    uint64_t Commit() {
        return commit_.load();
    }

    private:
    uint64_t GetIndex(uint64_t num);

    alignas(CACHELINE_SIZE) std::atomic<uint64_t> head_ = {0};
    alignas(CACHELINE_SIZE) std::atomic<uint64_t> tail_ = {1};
    alignas(CACHELINE_SIZE) std::atomic<uint64_t> commit_ = {1};
    // alignas(CACHELINE_SIZE) std::atomic<uint64_t> size_ = {0};
    uint64_t pool_size_ = 0;
    T* pool_ = nullptr;
    std::unique_ptr<WaitStrategy> wait_strategy_ = nullptr;
    volatile bool break_all_wait_ = false;
};

// 析构函数，释放队列资源，包括等待策略对象和循环数组
template <typename T> BoundedQueue<T>::~BoundedQueue() {
    if (wait_strategy_) {
        BreakAllWait();
    }
    if (pool_) {
        for (uint64_t i = 0; i < pool_size_; ++i) {
            pool_[i].~T();
        }
        std::free(pool_);
    }
}

// 指定队列大小，默认使用睡眠等待策略
template <typename T> inline bool BoundedQueue<T>::Init(uint64_t size) {
    return Init(size, new SleepWaitStrategy());
}

// 指定队列大小和等待策略
template <typename T> bool BoundedQueue<T>::Init(uint64_t size, WaitStrategy* strategy) {
    // Head and tail each occupy a space
    pool_size_ = size + 2;
    pool_ = reinterpret_cast<T*>(std::calloc(pool_size_, sizeof(T)));
    if (pool_ == nullptr) {
        return false;
    }
    for (uint64_t i = 0; i < pool_size_; ++i) {
        new (&(pool_[i])) T();
    }
    wait_strategy_.reset(strategy);
    return true;
}

// 入队操作，使用 CAS 操作更新 tail 和 commit 指针，确保线程安全
template <typename T> bool BoundedQueue<T>::Enqueue(const T& element) {
    uint64_t new_tail = 0;
    uint64_t old_commit = 0;
    uint64_t old_tail = tail_.load(std::memory_order_acquire);
    do {
        new_tail = old_tail + 1;
        if (GetIndex(new_tail) == GetIndex(head_.load(std::memory_order_acquire))) {
            return false;
        }
    } while (!tail_.compare_exchange_weak(old_tail, new_tail, std::memory_order_acq_rel, std::memory_order_relaxed));
    pool_[GetIndex(old_tail)] = element;
    do {
        old_commit = old_tail;
    } while (cyber_unlikely(!commit_.compare_exchange_weak(old_commit, new_tail, std::memory_order_acq_rel, std::memory_order_relaxed)));
    wait_strategy_->NotifyOne();
    return true;
}

// 入队操作，使用 CAS 操作更新 tail 和 commit 指针，确保线程安全
template <typename T> bool BoundedQueue<T>::Enqueue(T&& element) {
    uint64_t new_tail = 0;
    uint64_t old_commit = 0;
    uint64_t old_tail = tail_.load(std::memory_order_acquire);
    do {
        new_tail = old_tail + 1;
        if (GetIndex(new_tail) == GetIndex(head_.load(std::memory_order_acquire))) {
            return false;
        }
    } while (!tail_.compare_exchange_weak(old_tail, new_tail, std::memory_order_acq_rel, std::memory_order_relaxed));
    pool_[GetIndex(old_tail)] = std::move(element);
    do {
        old_commit = old_tail;
    } while (cyber_unlikely(!commit_.compare_exchange_weak(old_commit, new_tail, std::memory_order_acq_rel, std::memory_order_relaxed)));
    wait_strategy_->NotifyOne();
    return true;
}

// 出队操作，使用 CAS 操作更新 head 指针，确保线程安全
template <typename T> bool BoundedQueue<T>::Dequeue(T* element) {
    uint64_t new_head = 0;
    uint64_t old_head = head_.load(std::memory_order_acquire);
    do {
        new_head = old_head + 1;
        if (new_head == commit_.load(std::memory_order_acquire)) {
            return false;
        }
        *element = pool_[GetIndex(new_head)];
    } while (!head_.compare_exchange_weak(old_head, new_head, std::memory_order_acq_rel, std::memory_order_relaxed));
    return true;
}

// 基于等待策略的入队操作，如果队列满了则执行等待策略，直到入队成功或超时
template <typename T> bool BoundedQueue<T>::WaitEnqueue(const T& element) {
    while (!break_all_wait_) {
        if (Enqueue(element)) {
            return true;
        }
        if (wait_strategy_->EmptyWait()) {
            continue;
        }
        // wait timeout
        break;
    }

    return false;
}

// 基于等待策略的出队操作，如果队列空了则执行等待策略，直到出队成功或超时
template <typename T> bool BoundedQueue<T>::WaitEnqueue(T&& element) {
    while (!break_all_wait_) {
        if (Enqueue(std::move(element))) {
            return true;
        }
        if (wait_strategy_->EmptyWait()) {
            continue;
        }
        // wait timeout
        break;
    }

    return false;
}

// 基于等待策略的出队操作，如果队列空了则执行等待策略，直到出队成功或超时
template <typename T> bool BoundedQueue<T>::WaitDequeue(T* element) {
    while (!break_all_wait_) {
        /*如果对了里有数据，则直接return true，否则返回false*/
        if (Dequeue(element)) {
            return true;
        }
        /*执行等待策略*/
        if (wait_strategy_->EmptyWait()) {
            continue;
        }
        // wait timeout
        break;
    }

    return false;
}

// 获取队列大小，计算方法是 tail - head - 1，因为 tail 和 head 之间至少要有一个空位来区分队列满和队列空的情况
template <typename T> inline uint64_t BoundedQueue<T>::Size() {
    return tail_ - head_ - 1;
}

// 判断队列是否为空，队列为空的条件是 head 和 tail 之间没有元素，即 tail - head == 1
template <typename T> inline bool BoundedQueue<T>::Empty() {
    return Size() == 0;
}

// 计算索引，使用减法和乘法代替取模运算，提高性能
template <typename T> inline uint64_t BoundedQueue<T>::GetIndex(uint64_t num) {
    return num - (num / pool_size_) * pool_size_; // faster than %
}

// 设置等待策略，接受一个 WaitStrategy 指针参数，使用 unique_ptr 管理等待策略对象的生命周期
template <typename T> inline void BoundedQueue<T>::SetWaitStrategy(WaitStrategy* strategy) {
    wait_strategy_.reset(strategy);
}

// 打破所有等待线程，设置 break_all_wait_ 标志为 true，并调用等待策略的 BreakAllWait 方法唤醒所有等待线程
template <typename T> inline void BoundedQueue<T>::BreakAllWait() {
    break_all_wait_ = true;
    wait_strategy_->BreakAllWait();
}

} // namespace base
} // namespace Middleware
} // namespace hnu