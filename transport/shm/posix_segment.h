#pragma once

#include <string>
#include "segment.h"

namespace hnu {
namespace Middleware {
namespace transport {

class PosixSegment : public Segment {
public:
    explicit PosixSegment(uint64_t channel_id);
    virtual ~PosixSegment();

    static const char* Type() {
        return "posix";
    }

private:
    void Reset() override;
    bool Remove() override;
    bool OpenOnly() override;
    bool OpenOrCreate() override;

    std::string shm_name_;
};

} // namespace transport
} // namespace Middleware
} // namespace hnu
