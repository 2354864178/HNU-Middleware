#pragma once
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <mutex>
#include <thread>

namespace hnu {
namespace Middleware {
namespace base {

// 等待策略接口，定义了等待策略的基本操作，包括通知一个等待线程、打破所有等待线程、空等待等
class WaitStrategy {
    public:
    virtual void NotifyOne() {}    // 通知一个等待线程，默认实现为空
    virtual void BreakAllWait() {} // 打破所有等待线程，默认实现为空
    virtual bool EmptyWait() = 0;  // 空等待，纯虚函数，必须由派生类实现
    virtual ~WaitStrategy() {}
};

// 阻塞等待策略
class BlockWaitStrategy : public WaitStrategy {
    public:
    BlockWaitStrategy() {}

    // 通知一个等待线程，唤醒一个等待线程继续执行
    void NotifyOne() override {
        cv_.notify_one(); // 唤醒一个等待线程，通知一个线程可以继续执行
    }

    // 空等待，线程阻塞直到被通知唤醒，返回 true 表示等待成功
    bool EmptyWait() override {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock);
        return true;
    }

    // 打破所有等待线程，唤醒所有等待线程继续执行
    void BreakAllWait() override {
        cv_.notify_all();
    }

    private:
    std::mutex mutex_;           // 互斥锁，用于保护条件变量的访问，确保线程安全
    std::condition_variable cv_; // 条件变量，用于实现线程的阻塞和唤醒机制
};

// 睡眠等待策略
class SleepWaitStrategy : public WaitStrategy {
    public:
    SleepWaitStrategy() {}

    // 构造函数，接受一个睡眠时间参数，单位为微秒，默认值为 10000 微秒（10 毫秒）
    explicit SleepWaitStrategy(uint64_t sleep_time_us) : sleep_time_us_(sleep_time_us) {}

    // 空等待，线程睡眠指定的时间后返回 true，表示等待成功
    bool EmptyWait() override {
        std::this_thread::sleep_for(std::chrono::microseconds(sleep_time_us_));
        return true;
    }

    // 设置睡眠时间，接受一个睡眠时间参数，单位为微秒
    void SetSleepTimeMicroSeconds(uint64_t sleep_time_us) {
        sleep_time_us_ = sleep_time_us;
    }

    private:
    uint64_t sleep_time_us_ = 10000;
};

// 调度等待策略
class YieldWaitStrategy : public WaitStrategy {
    public:
    YieldWaitStrategy() {}

    // 空等待，线程让出当前的 CPU 时间片，允许其他线程执行，返回 true 表示等待成功
    bool EmptyWait() override {
        std::this_thread::yield();
        return true;
    }
};

// 忙等待策略，线程持续占用 CPU 资源进行等待，返回 true 表示等待成功
class BusySpinWaitStrategy : public WaitStrategy {
    public:
    BusySpinWaitStrategy() {}

    // 空等待，线程持续占用 CPU 资源进行等待，返回 true 表示等待成功
    bool EmptyWait() override {
        return true;
    }
};

// 超时阻塞等待策略
class TimeoutBlockWaitStrategy : public WaitStrategy {
    public:
    TimeoutBlockWaitStrategy() {}
    // 构造函数，接受一个超时时间参数，单位为毫秒，默认值为 1000 毫秒（1 秒）
    explicit TimeoutBlockWaitStrategy(uint64_t timeout) : time_out_(std::chrono::milliseconds(timeout)) {}

    // 通知一个等待线程，唤醒一个等待线程继续执行
    void NotifyOne() override {
        cv_.notify_one();
    }

    // 空等待，线程阻塞直到被通知唤醒或超时，返回 false 表示超时，返回 true 表示等待成功
    bool EmptyWait() override {
        std::unique_lock<std::mutex> lock(mutex_);
        if (cv_.wait_for(lock, time_out_) == std::cv_status::timeout) {
            return false;
        }
        return true;
    }

    // 打破所有等待线程，唤醒所有等待线程继续执行
    void BreakAllWait() override {
        cv_.notify_all();
    }

    // 设置超时时间，接受一个超时时间参数，单位为毫秒
    void SetTimeout(uint64_t timeout) {
        time_out_ = std::chrono::milliseconds(timeout);
    }

    private:
    std::mutex mutex_;
    std::condition_variable cv_;
    std::chrono::milliseconds time_out_;
};

} // namespace base
} // namespace Middleware
} // namespace hnu
