#include <fastdds/rtps/reader/RTPSReader.hpp>
#include <fastdds/rtps/common/SequenceNumber.hpp>

#include <memory.h>
#include "rea_listener.h"
#include "../../common/util.h"

using GUID_t = eprosima::fastdds::rtps::GUID_t;

namespace hnu {
namespace Middleware {
namespace transport {

ReaListener::ReaListener(const NewMsgCallback& callback, const std::string& channel_name) : callback_(callback), channel_name_(channel_name) {}

ReaListener::~ReaListener() {}

void ReaListener::on_new_cache_change_added(eprosima::fastdds::rtps::RTPSReader* reader, const eprosima::fastdds::rtps::CacheChange_t* const change) {

    std::lock_guard<std::mutex> lock(mutex_);

    // 由于rtps的writer/reader层拿不到 topic_name，因此需要在构造Listener时传入topic_name
    auto channel_id = common::Hash(channel_name_);

    // GUID_t是rtps中实体的标识符,每个实体都是唯一的
    char* ptr = reinterpret_cast<char*>(const_cast<GUID_t*>(&change->write_params.sample_identity().writer_guid()));

    Identity sender_id(false); //
    sender_id.set_data(ptr);
    msg_info_.set_sender_id(sender_id);

    Identity spare_id(false);
    spare_id.set_data(ptr + ID_SIZE);
    msg_info_.set_spare_id(spare_id);

    uint64_t seq_num = ((int64_t)change->sequenceNumber.high << 32) | change->sequenceNumber.low;

    msg_info_.set_seq_num(seq_num);

    // 这里可以修改callback 直接传递二进制数据流的数据和长度
    std::shared_ptr<std::string> msg_str = std::make_shared<std::string>((char*)change->serializedPayload.data, change->serializedPayload.length);

    callback_(channel_id, msg_str, msg_info_);
}

} // namespace transport
} // namespace Middleware
} // namespace hnu