#include "attributes_filler.h"
#include "../qos/qos_profile_conf.h"

namespace hnu {
namespace Middleware {
namespace transport {

AttributesFiller::AttributesFiller() {}
AttributesFiller::~AttributesFiller() {}

bool AttributesFiller::FillInWriterAttr(const std::string& channel_name, const QosProfile& qos, RtpsWriterAttributes* writer_attr) {
    /*配置 writer 的topic*/
    writer_attr->Tatt.topicName = channel_name;
    writer_attr->Tatt.topicDataType = "string";
    writer_attr->Tatt.topicKind = NO_KEY;

    /*配置 writer 的history*/
    writer_attr->hatt.payloadMaxSize = qos.msg_size + 255;
    writer_attr->hatt.maximumReservedCaches = 50;

    /*配置 writer 的Attributes*/
    writer_attr->watt.endpoint.reliabilityKind = BEST_EFFORT;

    /* 配置writer 的qos*/
    switch (qos.durability) {
    case QosHistoryPolicy::HISTORY_KEEP_LAST:
        writer_attr->Wqos.m_durability.kind = eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
        ;
        break;
    case QosDurabilityPolicy::DURABILITY_VOLATILE:
        writer_attr->Wqos.m_durability.kind = eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS;
    default:
        break;
    }

    switch (qos.reliability) {
    case QosReliabilityPolicy::RELIABILITY_BEST_EFFORT:
        writer_attr->Wqos.m_reliability.kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
        break;
    case QosReliabilityPolicy::RELIABILITY_RELIABLE:
        writer_attr->Wqos.m_reliability.kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    default:
        break;
    }

    /*配置 writer topic 的history*/

    switch (qos.history) {
    case QosHistoryPolicy::HISTORY_KEEP_LAST:
        writer_attr->Tatt.historyQos.kind = eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS;
        break;
    case QosHistoryPolicy::HISTORY_KEEP_ALL:
        writer_attr->Tatt.historyQos.kind = eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS;
    default:
        break;
    }

    // 在RELIABLE_RELIABILITY_QOS 下需要设置可靠的心跳周期 ,1 fraction = 1/(2^32) seconds
    if (qos.mps != 0) {
        uint64_t mps = qos.mps;
        if (mps > 1024) {
            mps = 1024;
        } else if (mps < 64) {
            mps = 64;
        }
        // 这里为啥要使用 256 我没想明白
        uint64_t fractions = (256ull << 32) / mps;
        uint32_t fraction = fractions & 0xffffffff;
        int32_t seconds = static_cast<int32_t>(fractions >> 32);

        writer_attr->watt.times.heartbeat_period.seconds = seconds;
        writer_attr->watt.times.heartbeat_period.nanosec = fraction;
    }

    if (qos.depth != QosProfileConf::QOS_HISTORY_DEPTH_SYSTEM_DEFAULT) {
        writer_attr->Tatt.historyQos.depth = static_cast<int32_t>(qos.depth);
    }

    if (writer_attr->Tatt.historyQos.depth < 0) {
        return false;
    }

    return true;
}

bool AttributesFiller::FillInReaderAttr(const std::string& channel_name, const QosProfile& qos, RtpsReaderAttributes* reader_attr) {

    reader_attr->Tatt.topicName = channel_name;
    reader_attr->Tatt.topicDataType = "string";
    reader_attr->Tatt.topicKind = NO_KEY;

    reader_attr->hatt.payloadMaxSize = 255;

    reader_attr->ratt.endpoint.reliabilityKind = BEST_EFFORT;
    /* 配置reader 的qos*/
    switch (qos.durability) {
    case QosDurabilityPolicy::DURABILITY_TRANSIENT_LOCAL:
        reader_attr->Rqos.m_durability.kind = eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
        ;
        break;
    case QosDurabilityPolicy::DURABILITY_VOLATILE:
        reader_attr->Rqos.m_durability.kind = eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS;
    default:
        break;
    }

    switch (qos.reliability) {
    case QosReliabilityPolicy::RELIABILITY_BEST_EFFORT:
        reader_attr->Rqos.m_reliability.kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
        break;
    case QosReliabilityPolicy::RELIABILITY_RELIABLE:
        reader_attr->Rqos.m_reliability.kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    default:
        break;
    }

    /*配置 reader 的history*/

    switch (qos.history) {
    case QosHistoryPolicy::HISTORY_KEEP_LAST:
        reader_attr->Tatt.historyQos.kind = eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS;
        break;
    case QosHistoryPolicy::HISTORY_KEEP_ALL:
        reader_attr->Tatt.historyQos.kind = eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS;
    default:
        break;
    }

    if (qos.depth != QosProfileConf::QOS_HISTORY_DEPTH_SYSTEM_DEFAULT) {
        reader_attr->Tatt.historyQos.depth = static_cast<int32_t>(qos.depth);
    }

    if (reader_attr->Tatt.historyQos.depth < 0) {
        return false;
    }

    return true;
}

} // namespace transport
} // namespace Middleware
} // namespace hnu