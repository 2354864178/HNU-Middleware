#include "file.h"

namespace hnu {
namespace Middleware {
namespace common {

std::string GetFileName(const std::string& path, const bool remove_extension = false) {
    // 找到字符串中最后一个 `/`的位置
    std::string::size_type start = path.rfind('/');
    if (start == std::string::npos) { // 如果字符串中没有`/` ，则 start = 0
        start = 0;
    } else {
        ++start;
    }

    std::string::size_type end = std::string::npos;
    // 如果 remove_extension 为 true，则找到字符串中最后一个 `.`的位置
    if (remove_extension) {
        end = path.rfind('.');
        // 如果找到 . 且其位置在最后一个 / 之前，说明该 . 是在目录名中而不是文件名中，因此忽略它。
        if (end != std::string::npos && end < start) {
            end = std::string::npos;
        }
    }

    const auto len = (end != std::string::npos) ? end - start : end;

    return path.substr(start, len);
}

} // namespace common
} // namespace Middleware
} // namespace hnu