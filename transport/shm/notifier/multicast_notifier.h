#pragma once

#include <netinet/in.h>
#include <atomic>

#include "notifier_base.h"
#include "common/macros.h"

namespace hnu {
namespace Middleware {
namespace transport {

// 多播通知器，使用UDP组播实现进程间通信，提供通知和监听功能，支持单例模式，当前并未使用
class MulticastNotifier : public NotifierBase {
public:
    virtual ~MulticastNotifier();

    void Shutdown() override;
    bool Notify(const ReadableInfo& info) override;
    bool Listen(int timeout_ms, ReadableInfo* info) override;

    static const char* type() {
        return "multicast";
    }

private:
    bool Init();

    int notify_fd_ = -1;
    struct sockaddr_in notify_addr_;

    int listen_fd_ = -1;
    struct sockaddr_in listen_addr_;

    std::atomic<bool> is_shutdown_ = {false};

    DECLARE_SINGLETON(MulticastNotifier)
};

} // namespace transport
} // namespace Middleware
} // namespace hnu
