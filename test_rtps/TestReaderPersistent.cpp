// 测试 Reader 端持久化/最小 RTPS 接收流程。
#include "TestReaderPersistent.h"

#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <thread>

#include <fastdds/rtps/RTPSDomain.hpp>
#include <fastdds/rtps/attributes/HistoryAttributes.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/attributes/ReaderAttributes.hpp>
#include <fastdds/rtps/builtin/data/TopicDescription.hpp>
#include <fastdds/rtps/history/ReaderHistory.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/rtps/reader/RTPSReader.hpp>

using eprosima::fastdds::dds::ReaderQos;
using namespace eprosima::fastdds::rtps;

// 构造函数：初始化资源句柄为空，避免悬空指针
TestReaderPersistent::TestReaderPersistent()
    : mp_participant(nullptr), mp_reader(nullptr), mp_history(nullptr) {}

// 析构函数：先从 RTPS 域中移除 participant，再释放 history
TestReaderPersistent::~TestReaderPersistent() {
  if (mp_participant != nullptr) {
    RTPSDomain::removeRTPSParticipant(mp_participant);
    mp_participant = nullptr;
  }
  delete mp_history;
  mp_history = nullptr;
}

// 创建 participant、history 和 reader
bool TestReaderPersistent::init() {
  // 参与者属性：使用 SIMPLE 发现模式，便于与 writer 端配对
  RTPSParticipantAttributes participant_attr;
  participant_attr.builtin.discovery_config.discoveryProtocol = DiscoveryProtocol::SIMPLE;
  participant_attr.builtin.use_WriterLivelinessProtocol = true; // 启用 Writer 生命协议，增强发现稳定性

  // 指定单播 locator，便于稳定发现对端。
  Locator_t loc(22223); // 端口号，确保与 writer 端一致
  participant_attr.defaultUnicastLocatorList.push_back(loc);  // 添加单播 locator 到参与者属性

  // 创建 RTPS participant。
  mp_participant = RTPSDomain::createParticipant(0, participant_attr);
  if (mp_participant == nullptr) {
    return false;
  }

  // 创建 ReaderHistory，用于缓存接收到的样本
  HistoryAttributes history_attr;     // 设置历史属性，如最大负载大小和预留缓存数量
  history_attr.payloadMaxSize = 256;  // 设置最大负载大小，确保能容纳发送的样本数据
  mp_history = new ReaderHistory(history_attr); // 创建 ReaderHistory 对象，管理接收到的数据样本

  // 创建 RTPSReader，并绑定监听器接收回调
  ReaderAttributes reader_attr;
  mp_reader = RTPSDomain::createRTPSReader(mp_participant, reader_attr, mp_history, &m_listener);
  return mp_reader != nullptr;
}

// 将 reader 注册到固定 topic/type，供 discovery 匹配使用
bool TestReaderPersistent::reg() {
  TopicDescription topic_desc;
  topic_desc.topic_name = kTopicName;
  topic_desc.type_name = kTypeName;
  ReaderQos reader_qos;
  return mp_participant->register_reader(mp_reader, topic_desc, reader_qos);
}

// 持续等待一段时间，观察 writer 是否能成功推送数据
void TestReaderPersistent::run() {
  std::cout << "[READER] waiting for data for " << kDefaultRunSeconds << " seconds" << std::endl;
  auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(kDefaultRunSeconds);
  while (std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }

  std::cout << "[READER] matched=" << m_listener.n_matched.load()
            << " received=" << m_listener.n_received.load() << std::endl;
}

// 收到新缓存样本时触发：先计数，再把 payload 按 int32 解析出来并打印
void TestReaderPersistent::MyListener::on_new_cache_change_added(
    RTPSReader* reader,
    const CacheChange_t* const change) {
  ++n_received;

  int32_t value = 0;
  if (change->serializedPayload.length >= sizeof(value)) {
    std::memcpy(&value, change->serializedPayload.data, sizeof(value));
    std::cout << "[READER] value=" << value
              << " payload_len=" << change->serializedPayload.length << std::endl;
  } else {
    std::cout << "[READER] payload_len=" << change->serializedPayload.length
              << " (too short for int32)" << std::endl;
  }

  reader->get_history()->remove_change(const_cast<CacheChange_t*>(change));
}

// writer 与 reader 匹配时触发，统计匹配次数并打印日志
void TestReaderPersistent::MyListener::on_reader_matched(
    RTPSReader*,
    const MatchingInfo& info) {
  if (info.status == MATCHED_MATCHING) {
    ++n_matched;
    std::cout << "[READER] matched" << std::endl;
  }
}
