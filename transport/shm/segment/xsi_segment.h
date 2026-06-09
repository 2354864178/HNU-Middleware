#pragma once

#include "transport/shm/core/segment.h"

namespace hnu {
namespace Middleware {
namespace transport {

class XsiSegment : public Segment {

public:
    explicit XsiSegment(uint64_t channel_id);
    virtual ~XsiSegment();

    // 返回共享内存段的类型标识，用于区分不同类型的共享内存段，比如"sysv"、"xsi"等
    static const char* Type() {
        return "xsi";
    }

private:
    void Reset() override;
    bool Remove() override;
    bool OpenOnly() override;
    bool OpenOrCreate() override;

    key_t key_; // 共享内存的key值，用于标识和访问共享内存段
};

} // namespace transport
} // namespace Middleware
} // namespace hnu
