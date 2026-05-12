#pragma once

#include <string>
#include <type_traits>

namespace hnu {
namespace Middleware {
namespace common {

inline std::size_t Hash(const std::string& key) {
    return std::hash<std::string>{}(key);
}

} // namespace common
} // namespace Middleware
} // namespace hnu
