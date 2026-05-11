#pragma once
#include <string>

#include "qos_profile.h"

namespace hnu {
namespace Middleware {
namespace config {

struct RoleAttributes {
    std::string host_name; // 主机名称
    std::string host_ip;   // 主机IP地址
    int32_t process_id;    // 进程ID

    std::string channel_name; // 通道名称
    uint64_t channel_id;      // channel_name 的哈希值
    QosProfile qos_profile;   // QoS 配置文件
    uint64_t id;              // 角色ID
};

} // namespace config
} // namespace Middleware
} // namespace hnu
