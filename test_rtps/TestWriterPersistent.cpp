#include "TestWriterPersistent.h" // 测试Writer持久化功能

#include <chrono>
#include <cstring>
#include <iostream>
#include <thread>

#include <fastdds/rtps/RTPSDomain.hpp>
#include <fastdds/rtps/attributes/HistoryAttributes.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/builtin/data/TopicDescription.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>

using eprosima::fastdds::dds::WriterQos;
using namespace eprosima::fastdds::rtps;

// 构造函数，初始化成员变量
TestWriterPersistent::TestWriterPersistent() : mp_participant(nullptr), mp_writer(nullptr), mp_history(nullptr) {}

// 析构函数，清理资源
TestWriterPersistent::~TestWriterPersistent() {
    if (mp_participant != nullptr) {
        RTPSDomain::removeRTPSParticipant(mp_participant); // 删除RTPS参与者对象，释放相关资源
        mp_participant = nullptr;
    }
    delete mp_history;    // 删除Writer历史对象
    mp_history = nullptr; // 删除Writer历史对象后置空
}

// 初始化Writer，包括创建RTPS参与者、历史对象和写入器
bool TestWriterPersistent::init() {
    RTPSParticipantAttributes participant_attr;                                              // 设置RTPS参与者属性
    participant_attr.builtin.discovery_config.discoveryProtocol = DiscoveryProtocol::SIMPLE; // 设置简单发现协议
    participant_attr.builtin.use_WriterLivelinessProtocol = true;                            // 启用写入器生命协议

    mp_participant = RTPSDomain::createParticipant(0, participant_attr); // 创建RTPS参与者
    if (mp_participant == nullptr) {
        return false;
    }

    HistoryAttributes history_attr;               // 设置Writer历史属性
    history_attr.payloadMaxSize = 256;            // 设置最大负载大小
    history_attr.maximumReservedCaches = 32;      // 设置最大预留缓存数量
    mp_history = new WriterHistory(history_attr); // 创建Writer历史对象

    WriterAttributes writer_attr;                       // 设置Writer属性
    writer_attr.endpoint.reliabilityKind = BEST_EFFORT; // 设置可靠性为最佳努力

    mp_writer = RTPSDomain::createRTPSWriter(mp_participant, writer_attr, mp_history,
                                             &m_listener); // 创建RTPS写入器并关联监听器
    return mp_writer != nullptr;
}

// 注册Writer到RTPS参与者，指定主题和数据类型
bool TestWriterPersistent::reg() {
    TopicDescription topic_desc;        // 创建主题描述对象
    topic_desc.topic_name = kTopicName; // 设置主题名称
    topic_desc.type_name = kTypeName;   // 设置类型名称
    WriterQos writer_qos;               // 创建Writer QoS对象，使用默认设置
    return mp_participant->register_writer(mp_writer, topic_desc,
                                           writer_qos); // 注册写入器到参与者，返回注册结果
}

// 运行Writer，发送指定数量的样本数据到匹配的读者
// samples参数指定要发送的样本数量
void TestWriterPersistent::run(uint16_t samples) {
    std::cout << "[WRITER] waiting for matched readers" << std::endl;
    while (m_listener.n_matched.load() == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200)); // 等待匹配的读者连接，使用原子变量确保线程安全
    }

    for (uint16_t i = 0; i < samples; ++i) {
        CacheChange_t* ch = mp_history->create_change(sizeof(int32_t), ALIVE); // 创建一个新的缓存更改对象，指定负载大小和状态
        if (ch == nullptr) {
            std::cout << "[WRITER] create_change failed, retry later" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        const int32_t value = static_cast<int32_t>(i); // 生成要发送的样本数据，这里使用简单的整数值
        ch->serializedPayload.length = sizeof(value);  // 设置序列化负载的长度
        std::memcpy(ch->serializedPayload.data, &value,
                    sizeof(value)); // 将样本数据复制到缓存更改的序列化负载中
        mp_history->add_change(ch); // 将更改添加到Writer历史中，准备发送给匹配的读者

        std::cout << "[WRITER] sent value=" << value << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
}

// Writer匹配事件回调函数，接收匹配信息参数，更新匹配计数器
void TestWriterPersistent::MyListener::on_writer_matched(RTPSWriter*, const MatchingInfo& info) {
    if (info.status == MATCHED_MATCHING) {
        ++n_matched;
        std::cout << "[WRITER] matched" << std::endl;
    }
}
