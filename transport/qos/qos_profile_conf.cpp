#include "qos_profile_conf.h"

namespace hnu {
namespace Middleware {
namespace transport {

QosProfileConf::QosProfileConf() {}

QosProfileConf::~QosProfileConf() {}

// 创建一个QosProfile对象，并根据输入的历史策略、深度、消息速率、可靠性策略和持久化策略来设置相应的属性值
/*
param history: QosHistoryPolicy类型的历史策略，表示消息的历史记录方式
param depth: uint32_t类型的历史深度，表示保留消息的数量，仅在历史策略为HISTORY_KEEP_LAST时有效
param mps: uint32_t类型的消息速率，表示每秒钟允许发送的消息数量，0表示由系统决定
param reliability: QosReliabilityPolicy类型的可靠性策略，表示消息传输的可靠性要求
param durability: QosDurabilityPolicy类型的持久化策略，表示消息的持久化要求
return: 返回一个QosProfile对象，包含了根据输入参数设置的QoS配置，用于指导通信系统在消息传输过程中如何处理消息的
*/
QosProfile QosProfileConf::CreateQosProfile(const QosHistoryPolicy& history, uint32_t depth, uint32_t mps, const QosReliabilityPolicy& reliability, const QosDurabilityPolicy& durability) {
    QosProfile qos_profile;
    qos_profile.history = history;
    qos_profile.reliability = reliability;
    qos_profile.durability = durability;
    qos_profile.mps = mps;
    qos_profile.depth = depth;
    return qos_profile;
}

const uint32_t QosProfileConf::QOS_HISTORY_DEPTH_SYSTEM_DEFAULT = 0;
const uint32_t QosProfileConf::QOS_MPS_SYSTEM_DEFAULT = 0;

const QosProfile QosProfileConf::QOS_PROFILE_DEFAULT = CreateQosProfile(QosHistoryPolicy::HISTORY_KEEP_LAST, 1, QOS_MPS_SYSTEM_DEFAULT, QosReliabilityPolicy::RELIABILITY_RELIABLE, QosDurabilityPolicy::DURABILITY_VOLATILE);

const QosProfile QosProfileConf::QOS_PROFILE_SENSOR_DATA = CreateQosProfile(QosHistoryPolicy::HISTORY_KEEP_LAST, 5, QOS_MPS_SYSTEM_DEFAULT, QosReliabilityPolicy::RELIABILITY_BEST_EFFORT, QosDurabilityPolicy::DURABILITY_VOLATILE);

const QosProfile QosProfileConf::QOS_PROFILE_PARAMETERS = CreateQosProfile(QosHistoryPolicy::HISTORY_KEEP_LAST, 1000, QOS_MPS_SYSTEM_DEFAULT, QosReliabilityPolicy::RELIABILITY_RELIABLE, QosDurabilityPolicy::DURABILITY_VOLATILE);

const QosProfile QosProfileConf::QOS_PROFILE_SERVICES_DEFAULT = CreateQosProfile(QosHistoryPolicy::HISTORY_KEEP_LAST, 10, QOS_MPS_SYSTEM_DEFAULT, QosReliabilityPolicy::RELIABILITY_RELIABLE, QosDurabilityPolicy::DURABILITY_TRANSIENT_LOCAL);

const QosProfile QosProfileConf::QOS_PROFILE_PARAM_EVENT = CreateQosProfile(QosHistoryPolicy::HISTORY_KEEP_LAST, 1000, QOS_MPS_SYSTEM_DEFAULT, QosReliabilityPolicy::RELIABILITY_RELIABLE, QosDurabilityPolicy::DURABILITY_VOLATILE);

const QosProfile QosProfileConf::QOS_PROFILE_SYSTEM_DEFAULT = CreateQosProfile(QosHistoryPolicy::HISTORY_SYSTEM_DEFAULT, QOS_HISTORY_DEPTH_SYSTEM_DEFAULT, QOS_MPS_SYSTEM_DEFAULT, QosReliabilityPolicy::RELIABILITY_RELIABLE, QosDurabilityPolicy::DURABILITY_TRANSIENT_LOCAL);

const QosProfile QosProfileConf::QOS_PROFILE_TF_STATIC = CreateQosProfile(QosHistoryPolicy::HISTORY_KEEP_ALL, 10, QOS_MPS_SYSTEM_DEFAULT, QosReliabilityPolicy::RELIABILITY_RELIABLE, QosDurabilityPolicy::DURABILITY_TRANSIENT_LOCAL);

const QosProfile QosProfileConf::QOS_PROFILE_TOPO_CHANGE = CreateQosProfile(QosHistoryPolicy::HISTORY_KEEP_ALL, 10, QOS_MPS_SYSTEM_DEFAULT, QosReliabilityPolicy::RELIABILITY_RELIABLE, QosDurabilityPolicy::DURABILITY_TRANSIENT_LOCAL);

} // namespace transport
} // namespace Middleware
} // namespace hnu