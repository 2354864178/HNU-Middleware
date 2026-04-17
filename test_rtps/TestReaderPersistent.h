#ifndef TEST_TESTREADERPERSISTENT_H_
#define TEST_TESTREADERPERSISTENT_H_

#include <fastdds/dds/subscriber/qos/ReaderQos.hpp>
#include <fastdds/rtps/history/ReaderHistory.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/rtps/reader/RTPSReader.hpp>
#include <fastdds/rtps/reader/ReaderListener.hpp>

// RTPS 读端最小实现：负责创建参与者、注册读端并接收 writer 数据
class TestReaderPersistent {
    public:
    // 创建/销毁读端资源
    TestReaderPersistent();
    ~TestReaderPersistent();

    bool init(); // 初始化读端，包括创建 RTPS 参与者、历史对象和读端
    bool reg();  // 将 reader 注册到指定 topic
    void run();  // 等待一段时间，持续接收数据

    private:
    // 监听器用于统计匹配次数和收到的数据样本
    class MyListener : public eprosima::fastdds::rtps::ReaderListener {
        public:
        // 收到新的缓存样本时触发
        void on_new_cache_change_added(eprosima::fastdds::rtps::RTPSReader* reader, const eprosima::fastdds::rtps::CacheChange_t* const change) override;

        // 发现 writer 并完成匹配时触发
        void on_reader_matched(eprosima::fastdds::rtps::RTPSReader*, const eprosima::fastdds::rtps::MatchingInfo& info) override;

        std::atomic<int> n_received{0}; // 统计收到的数据条数
        std::atomic<int> n_matched{0};  // 统计匹配成功次数
    };

    eprosima::fastdds::rtps::RTPSParticipant* mp_participant; // RTPS参与者对象的指针，负责管理RTPS通信的上下文和资源
    eprosima::fastdds::rtps::RTPSReader* mp_reader;           // RTPS读端对象的指针，负责接收数据并与writer进行通信
    eprosima::fastdds::rtps::ReaderHistory* mp_history;       // RTPS读端历史对象的指针，负责管理接收到的数据样本的缓存和生命周期
    MyListener m_listener;                                    // RTPS读端监听器对象，负责处理读端的事件回调，如接收新数据和匹配事件

    static constexpr const char* kTopicName = "exampleTopic"; // 定义主题名称常量，用于注册读端时指定主题
    static constexpr const char* kTypeName = "raw_int";       // 定义类型名称常量，用于注册读端时指定数据类型
    static constexpr int kDefaultRunSeconds = 10;             // 定义默认运行时间常量，单位为秒，用于在run方法中持续等待接收数据
};

#endif // TEST_TESTREADERPERSISTENT_H_
