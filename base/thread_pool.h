#pragma once

#include <atomic>
#include <functional>
#include <future>
#include <memory>
#include <queue>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

#include "bounded_queue.h"

namespace hnu {
namespace Middleware {
namespace base {

class ThreadPool {
    public:
    explicit ThreadPool(std::size_t thread_num, std::size_t max_task_num = 1000);

    template <typename F, typename... Args> auto Enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;

    ~ThreadPool();

    private:
    std::vector<std::thread> workers_;               // 工作线程池，存储所有工作线程的线程对象
    BoundedQueue<std::function<void()>> task_queue_; // 任务队列，使用有界队列实现，存储待执行的任务，任务类型为 std::function<void()>，表示一个无参数无返回值的可调用对象
    std::atomic_bool stop_;                          // 停止标志，表示线程池是否已经停止，初始值为 false，当线程池被停止时，所有工作线程将退出循环并结束执行
};

// 构造函数，接受线程数量和最大任务数量参数，初始化线程池和任务队列
inline ThreadPool::ThreadPool(std::size_t threads, std::size_t max_task_num) : stop_(false) {
    // 初始化任务队列，指定队列大小和等待策略，使用 BlockWaitStrategy 作为默认的等待策略
    if (!task_queue_.Init(max_task_num, new BlockWaitStrategy())) {
        throw std::runtime_error("Task queue init failed.");
    }

    // 创建指定数量的工作线程，每个线程在一个循环中不断从任务队列中获取任务并执行，直到线程池被停止
    workers_.reserve(threads); // 提前分配线程池的内存，避免在运行时频繁分配和释放内存，提高性能
    for (size_t i = 0; i < threads; ++i) {
        workers_.emplace_back([this] {
            while (!stop_) {
                std::function<void()> task; // 定义一个 std::function 对象，表示一个无参数无返回值的可调用对象，用于存储从任务队列中获取的任务
                // 从任务队列中获取一个任务，如果获取成功则执行该任务，获取失败可能是因为队列为空或者线程池被停止，此时继续循环等待新的任务或者线程池停止
                if (task_queue_.WaitDequeue(&task)) {
                    task();
                }
            }
        });
    }
}

// 模板成员函数，接受一个可调用对象和其参数，创建一个包装任务并将其添加到任务队列中，返回一个 std::future 对象用于获取任务的执行结果
template <typename F, typename... Args> auto ThreadPool::Enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();

    // 将包装任务添加到任务队列中，如果线程池已经停止则返回一个默认构造的 std::future 对象，表示任务无法执行
    if (stop_) {
        return std::future<return_type>();
    }
    task_queue_.Enqueue([task]() { (*task)(); });
    return res;
};

// 析构函数，设置停止标志为 true，打破所有等待线程，并等待所有工作线程结束执行
inline ThreadPool::~ThreadPool() {
    if (stop_.exchange(true)) {
        return;
    }
    task_queue_.BreakAllWait();
    for (std::thread& worker : workers_) {
        worker.join();
    }
}

} // namespace base
} // namespace Middleware
} // namespace hnu
