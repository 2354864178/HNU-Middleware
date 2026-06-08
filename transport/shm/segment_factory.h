#pragma once

#include "segment.h"

namespace hnu {
namespace Middleware {
namespace transport {

class SegmentFactory {
public:
    static SegmentPtr CreateSegment(uint64_t channel_id);
};

} // namespace transport
} // namespace Middleware
} // namespace hnu
