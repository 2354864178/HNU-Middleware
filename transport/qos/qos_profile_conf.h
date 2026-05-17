#pragma once

#include <stdint.h>
#include "../config/qos_profile.h"

namespace hnu {
namespace Middleware {
namespace transport {

// 用于创建和管理不同类型的QosProfile配置，提供了一些预定义的QosProfile对象，适用于不同的通信场景，如传感器数据、参数更新、服务通信等
class QosProfileConf {
public:
    QosProfileConf();
    virtual ~QosProfileConf();
    static QosProfile CreateQosProfile(const QosHistoryPolicy& history, uint32_t depth, uint32_t mps, const QosReliabilityPolicy& reliability, const QosDurabilityPolicy& durability);

    static const uint32_t QOS_HISTORY_DEPTH_SYSTEM_DEFAULT; // 历史深度的系统默认值，表示由系统决定历史深度的行为
    static const uint32_t QOS_MPS_SYSTEM_DEFAULT;           // 消息速率的系统默认值，表示由系统决定消息速率的行为
    static const QosProfile QOS_PROFILE_DEFAULT;            // 默认的QosProfile配置，适用于一般的通信场景，历史策略为只保留最新的消息，深度为1，可靠传输，易失性持久化
    static const QosProfile QOS_PROFILE_SENSOR_DATA;        // 适用于传感器数据的QosProfile配置，历史策略为只保留最新的消息，深度为5，最大努力传输，易失性持久化
    static const QosProfile QOS_PROFILE_PARAMETERS;         // 适用于参数更新的QosProfile配置，历史策略为只保留最新的消息，深度为1000，可靠传输，易失性持久化
    static const QosProfile QOS_PROFILE_SERVICES_DEFAULT;   // 适用于服务通信的QosProfile配置，历史策略为只保留最新的消息，深度为10，可靠传输，临时本地持久化
    static const QosProfile QOS_PROFILE_PARAM_EVENT;        // 适用于参数事件的QosProfile配置，历史策略为只保留最新的消息，深度为1000，可靠传输，易失性持久化
    static const QosProfile QOS_PROFILE_SYSTEM_DEFAULT;     // 系统默认的QosProfile配置，历史策略和深度由系统决定，可靠传输，临时本地持久化
    static const QosProfile QOS_PROFILE_TF_STATIC;          // 适用于静态TF数据的QosProfile配置，历史策略为保留所有消息，深度为10，可靠传输，临时本地持久化
    static const QosProfile QOS_PROFILE_TOPO_CHANGE;        // 适用于拓扑变化事件的QosProfile配置，历史策略为保留所有消息，深度为10，可靠传输，临时本地持久化
};

} // namespace transport
} // namespace Middleware
} // namespace hnu
