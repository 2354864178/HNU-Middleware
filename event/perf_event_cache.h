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

    std::thread io_thread_;
    std::ofstream of_;

    bool enable_ = false;
    bool shutdown_ = false;

    std::string perf_file_ = "";
    base::BoundedQueue<EventBasePtr> event_queue_;

    const int kFlushSize = 512;
    const uint64_t kEventQueueSize = 8192; // 事件队列的长度

    DECLARE_SINGLETON(PerfEventCache)
};

} // namespace event
} // namespace Middleware
} // namespace hnu
