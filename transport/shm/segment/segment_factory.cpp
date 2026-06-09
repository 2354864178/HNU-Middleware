#include "segment_factory.h"
#include "xsi_segment.h"
#include "posix_segment.h"
#include "common/log.h"
#include "common/global_data.h"

namespace hnu {
namespace Middleware {
namespace transport {

using hnu::Middleware::common::GlobalData;

auto SegmentFactory::CreateSegment(uint64_t channel_id) -> SegmentPtr {
    std::string segment_type(XsiSegment::Type());

    ADEBUG << "segment type: " << segment_type;
    if (segment_type == PosixSegment::Type()) {
        return std::make_shared<PosixSegment>(channel_id);
    }
    return std::make_shared<XsiSegment>(channel_id);
}

} // namespace transport
} // namespace Middleware
} // namespace hnu