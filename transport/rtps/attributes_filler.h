
#pragma once

#include <string>
#include "../config/qos_profile.h"

#include <fastdds/dds/subscriber/qos/ReaderQos.hpp>
#include <fastdds/dds/publisher/qos/WriterQos.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/attributes/ReaderAttributes.hpp>
#include <fastdds/rtps/attributes/HistoryAttributes.hpp>
#include <src/cpp/xmlparser/attributes/TopicAttributes.hpp>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;
using namespace eprosima::fastdds::xmlparser;

namespace hnu {
namespace Middleware {
namespace transport {

// RTPS Writer和Reader的属性结构体
struct RtpsWriterAttributes {
    HistoryAttributes hatt; // 配置writer的history属性
    WriterAttributes watt;  // 配置writer的属性
    WriterQos Wqos;         // 配置writer的qos属性
    TopicAttributes Tatt;   // 配置writer的topic属性
};

struct RtpsReaderAttributes {
    HistoryAttributes hatt; // 配置reader的history属性
    ReaderAttributes ratt;  // 配置reader的属性
    ReaderQos Rqos;         // 配置reader的qos属性
    TopicAttributes Tatt;   // 配置reader的topic属性
};

class AttributesFiller {

public:
    AttributesFiller();
    virtual ~AttributesFiller();

    // 根据channel_name和qos配置来填充WriterAttributes和ReaderAttributes
    static bool FillInWriterAttr(const std::string& channel_name, const QosProfile& qos, RtpsWriterAttributes* writer_attr);
    static bool FillInReaderAttr(const std::string& channel_name, const QosProfile& qos, RtpsReaderAttributes* reader_attr);
};

} // namespace transport
} // namespace Middleware
} // namespace hnu
