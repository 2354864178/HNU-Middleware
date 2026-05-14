#pragma once

#include "../../serialize/serializable.h"
#include "../../serialize/data_stream.h"

namespace hnu {
namespace Middleware {
namespace transport {

using namespace serialize;

enum QosHistoryPolicy {
    HISTORY_SYSTEM_DEFAULT = 0, // 默认值，具体行为由系统决定
    HISTORY_KEEP_LAST = 1,      // 只保留最新的消息，丢弃旧的消息
    HISTORY_KEEP_ALL = 2,       // 保留所有消息，直到被订阅者接收或过期（如果有设置过期时间）。这种策略适用于需要确保所有消息都被处理的场景，但可能会导致内存使用增加。
};

enum QosReliabilityPolicy {
    RELIABILITY_SYSTEM_DEFAULT = 0, // 默认值，具体行为由系统决定
    RELIABILITY_RELIABLE = 1,       // 可靠传输，确保消息被成功接收
    RELIABILITY_BEST_EFFORT = 2,    // 最大努力传输，不保证消息的可靠传递
};

enum QosDurabilityPolicy {
    DURABILITY_SYSTEM_DEFAULT = 0,  // 默认值，具体行为由系统决定
    DURABILITY_TRANSIENT_LOCAL = 1, // 临时本地持久化，发布者在发布消息后会将消息保存在本地，直到所有订阅者接收完毕或过期（如果有设置过期时间）。这种策略适用于需要确保消息在发布后能够被订阅者接收的场景，但不需要跨进程或跨系统的持久化。
    DURABILITY_VOLATILE = 2,        // 易失性，不进行持久化，消息在发布后不会被保存在本地，订阅者只能接收发布时刻之后发布的消息。这种策略适用于对实时性要求较高的场景，但可能会导致消息丢失。
};

/*Qos配置信息结构体，支持序列化*/
class QosProfile : public Serializable {
public:
    QosHistoryPolicy history = HISTORY_KEEP_LAST;
    uint32_t depth = 2; // default = 1
    uint32_t mps = 3;   // [default = 0];    // messages per second
    uint32_t msg_size = 0;
    QosReliabilityPolicy reliability = RELIABILITY_RELIABLE;
    QosDurabilityPolicy durability = DURABILITY_VOLATILE;

    SERIALIZE(history, depth, mps, msg_size, reliability, durability)
};

} // namespace transport
} // namespace Middleware
} // namespace hnu
