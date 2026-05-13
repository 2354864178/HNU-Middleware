#pragma once
#include <string>

#include "qos_profile.h"
#include "../../serialize/serializable.h"
#include "../../serialize/data_stream.h"

namespace hnu {
namespace Middleware {
namespace config {
using namespace serialize;
struct RoleAttributes : public serialize::Serializable {
    std::string host_name; // 主机名
    std::string host_ip;   // 主机IP
    int32_t process_id;    // 进程ID

    std::string channel_name; // 通信通道名称
    uint64_t channel_id;      // 哈希值，基于channel_name计算得到

    QosProfile qos_profile; // Qos配置策略
    uint64_t id;            //

    std::string node_name; // 节点名称
    uint64_t node_id;      // 哈希值，基于node_name计算得到

    std::string message_type; // 消息类型
    SERIALIZE(host_name, host_ip, process_id, channel_name, qos_profile, id, node_name, node_id, message_type)
};

} // namespace config
} // namespace Middleware
} // namespace hnu
