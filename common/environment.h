#pragma once

#include <string>
#include <iostream>
#include "log.h"

namespace hnu {
namespace Middleware {
namespace common {

// 获取环境变量的值，如果环境变量未设置则返回默认值，并记录警告日志
inline std::string GetEnv(const std::string& var_name, const std::string& default_value = "") {
    const char* var = std::getenv(var_name.c_str());
    if (var == nullptr) {
        AWARN << "Environment variable [" << var_name << "] not set, fallback to " << default_value;
        return default_value;
    }
    return std::string(var);
}

inline const std::string WorkRoot() {
    std::string work_root = GetEnv("CMW_PATH");
    if (work_root.empty()) {
        work_root = "/cmw";
    }
    return work_root;
}

} // namespace common
} // namespace Middleware
} // namespace hnu
