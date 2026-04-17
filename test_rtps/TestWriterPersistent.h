#ifndef TEST_TESTWRITERPERSISTENT_H_
#define TEST_TESTWRITERPERSISTENT_H_

#include <atomic>
#include <cstdint>

#include <fastdds/dds/publisher/qos/WriterQos.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>
#include <fastdds/rtps/writer/WriterListener.hpp>

// 测试Writer持久化功能的类，包含初始化、注册和运行方法，以及一个内部监听器类用于处理匹配事件
class TestWriterPersistent {
    public:
    TestWriterPersistent();
    ~TestWriterPersistent();

    bool init();                // 初始化Writer，包括创建RTPS参与者、历史对象和写入器
    bool reg();                 // 注册Writer到RTPS参与者，指定主题和数据类型
    void run(uint16_t samples); // 运行Writer，发送指定数量的样本数据到匹配的读者

    private:
    // 内部监听器类，用于处理Writer的匹配事件，统计匹配的读者数量
    class MyListener : public eprosima::fastdds::rtps::WriterListener {
        public:
        void on_writer_matched(eprosima::fastdds::rtps::RTPSWriter*,                        // Writer对象指针，表示发生匹配事件的写入器
                               const eprosima::fastdds::rtps::MatchingInfo& info) override; // 匹配事件回调函数，接收匹配信息参数，更新匹配计数器

        std::atomic<int> n_matched{0}; // 使用原子变量统计匹配的读者数量，确保线程安全
    };

    eprosima::fastdds::rtps::RTPSParticipant* mp_participant; // RTPS参与者对象的指针，负责管理RTPS通信的上下文和资源
    eprosima::fastdds::rtps::RTPSWriter* mp_writer;           // RTPS写入器对象的指针，负责发送数据到匹配的读者
    eprosima::fastdds::rtps::WriterHistory* mp_history;       // RTPS写入器历史对象的指针，负责管理写入器的历史数据缓存
    MyListener m_listener;                                    // RTPS写入器监听器对象，负责处理写入器的事件回调，如匹配事件

    static constexpr const char* kTopicName = "exampleTopic"; // 定义主题名称常量，用于注册写入器时指定主题
    static constexpr const char* kTypeName = "raw_int";       // 定义类型名称常量，用于注册写入器时指定数据类型
};

#endif // TEST_TESTWRITERPERSISTENT_H_
