#pragma once

#include <chrono>
#include <fstream>
#include <memory>
#include <string>
#include <thread>

#include "perf_event.h"
#include "../base/bounded_queue.h"

namespace hnu {
namespace Middleware {
namespace event {

class PerfEventCache {

public:
    using EventBasePtr = std::shared_ptr<EventBase>;
    ~PerfEventCache();
    void AddTransportEvent(const TransPerf event_id, const uint64_t channel_id, const uint64_t msg_seq, const uint64_t stamp = 0, const std::string& adder = "-");

    std::string PerfFile() {
        return perf_file_;
    }
    void Shutdown();

private:
    void Start();
    void Run();

    std::thread io_thread_; // 用于运行 Run() 方法的线程对象，负责从事件队列中获取事件并写入文件
    std::ofstream of_;      // 用于写入性能事件数据的文件输出流对象

    bool enable_ = false;   // 标志位，表示是否开启事件记录功能，AddTransportEvent() 方法会根据这个标志位来决定是否将事件添加到事件队列中
    bool shutdown_ = false; // 标志位，表示是否正在关闭中，Run() 方法会根据这个标志位来决定是否继续从事件队列中获取事件并写入文件

    std::string perf_file_ = "";                   // 用于保存性能事件记录的文件路径和名称，Start() 方法会根据配置文件读取 perf_file 的值并将其保存到这个成员变量中
    base::BoundedQueue<EventBasePtr> event_queue_; // 用于存储性能事件的有界队列，AddTransportEvent() 方法会将事件添加到这个队列中，Run() 方法会从这个队列中获取事件并写入文件

    const int kFlushSize = 512;            // 每次写入文件的事件数量，当事件数量达到这个值时就会调用 of_.flush() 方法将数据写入文件
    const uint64_t kEventQueueSize = 8192; // 事件队列的长度

    DECLARE_SINGLETON(PerfEventCache) // 声明单例模式，确保全局只有一个 PerfEventCache 实例，提供一个静态方法 Instance() 来获取这个实例
};

} // namespace event
} // namespace Middleware
} // namespace hnu
