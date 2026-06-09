#pragma once

#include "transport/shm/core/readable_info.h"

namespace hnu {
namespace Middleware {
namespace transport {

class NotifierBase;
using NotifierPtr = NotifierBase*;
class NotifierBase {

public:
    virtual ~NotifierBase() = default;

    virtual void Shutdown() = 0;
    virtual bool Notify(const ReadableInfo& info) = 0;
    virtual bool Listen(int timeout_ms, ReadableInfo* info) = 0;
};

} // namespace transport
} // namespace Middleware
} // namespace hnu
