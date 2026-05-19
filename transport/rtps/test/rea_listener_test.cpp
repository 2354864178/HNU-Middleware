#include <iostream>
#include <cassert>
#include <cstring>
#include <vector>

#include "../rea_listener.h"
#include "../../../common/util.h"

using namespace hnu::Middleware::transport;
using GUID_t = eprosima::fastdds::rtps::GUID_t;

// Use real CacheChange_t from Fast-DDS
#include <fastdds/rtps/common/CacheChange.hpp>

int main() {
    bool called = false;
    uint64_t got_channel = 0;
    std::shared_ptr<std::string> got_msg;
    MessageInfo got_info;

    auto cb = [&](uint64_t channel_id, const std::shared_ptr<std::string>& msg_str, const MessageInfo& msg_info) {
        called = true;
        got_channel = channel_id;
        got_msg = msg_str;
        got_info = msg_info;
    };

    std::string channel_name = "test_chan";
    ReaListener listener(cb, channel_name);

    // prepare real CacheChange_t
    const char* payload = "hello-rtps";
    uint32_t payload_len = static_cast<uint32_t>(strlen(payload));
    eprosima::fastdds::rtps::CacheChange_t change(payload_len);

    // fill writer guid bytes
    GUID_t& gid = change.write_params.sample_identity().writer_guid();
    std::memset(&gid, 0x5A, sizeof(GUID_t));

    // sequence number
    change.sequenceNumber.high = 0x1;
    change.sequenceNumber.low = 0x2;

    // payload copy
    std::memcpy(change.serializedPayload.data, payload, payload_len);
    change.serializedPayload.length = payload_len;

    // call listener
    listener.on_new_cache_change_added(nullptr, &change);

    // prevent CacheChange_t destructor from freeing internal buffer (avoid double-free in test)
    change.serializedPayload.data = nullptr;

    assert(called);
    uint64_t expected_channel = hnu::Middleware::common::Hash(channel_name);
    assert(got_channel == expected_channel);
    assert(got_msg && *got_msg == std::string(payload, change.serializedPayload.length));
    // verify seq num
    uint64_t seq = ((int64_t)change.sequenceNumber.high << 32) | change.sequenceNumber.low;
    assert(got_info.seq_num() == seq);

    free(change.serializedPayload.data);

    std::cout << "rea_listener_test: OK\n";
    return 0;
}
