#pragma once

#include <memory>
#include <mutex>
#include <functional>
#include "../message/message_info.h"
#include <fastdds/rtps/reader/ReaderListener.hpp>

namespace hnu {
namespace Middleware {
namespace transport {

class ReaListener;
using RealistenerPtr = std::shared_ptr<ReaListener>;

// eprosima::fastdds::rtps::ReaderListener 是一个异步的线程用于监控 Cache中是否有数据
class ReaListener : public eprosima::fastdds::rtps::ReaderListener {
public:
    using NewMsgCallback = std::function<void(uint64_t channel_id, const std::shared_ptr<std::string>& msg_str, const MessageInfo& msg_info)>;

    explicit ReaListener(const NewMsgCallback& callback, const std::string& channel_name);
    virtual ~ReaListener();

    void on_new_cache_change_added(eprosima::fastdds::rtps::RTPSReader* reader, const eprosima::fastdds::rtps::CacheChange_t* const change) override;
    void on_reader_matched(eprosima::fastdds::rtps::RTPSReader* reader, const eprosima::fastdds::rtps::MatchingInfo& info) override {
        if (info.status == eprosima::fastdds::rtps::MATCHED_MATCHING) {
            printf("matched\n");
            n_matched++;
        }
    }

private:
    uint32_t n_matched;
    std::string channel_name_;
    // fast-rtps reader的listener的回调函数，会在onNewCacheChangeAdded 中调用
    NewMsgCallback callback_;
    MessageInfo msg_info_;
    std::mutex mutex_;
};

} // namespace transport
} // namespace Middleware
} // namespace hnu
