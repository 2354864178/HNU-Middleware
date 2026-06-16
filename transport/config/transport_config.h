#pragma once

namespace hnu {
namespace Middleware {
namespace config {

// 传输层使用的通讯方式
enum OptionalMode {
    HYBRID = 0,
    INTRA = 1,
    SHM = 2,
    RTPS = 3,
};

} // namespace config
} // namespace Middleware
} // namespace hnu
