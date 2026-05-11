#pragma once

#include <string>

namespace hnu {
namespace Middleware {
namespace common {

// 给定一个文件路径，获取文件名
std::string GetFileName(const std::string& path, const bool remove_extension = false);

} // namespace common
} // namespace Middleware
} // namespace hnu
