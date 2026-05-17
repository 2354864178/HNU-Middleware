#include "../perf_event.h"
#include "../../common/global_data.h"

#include <cassert>
#include <iostream>

using namespace hnu::Middleware::event;
using namespace hnu::Middleware::common;

#if !defined(USE_GLOBALDATA) || (USE_GLOBALDATA == 0)
// Simple test-only implementation to avoid linking against global_data.cpp
namespace hnu {
namespace Middleware {
namespace common {
std::string GlobalData::GetChannelById(uint64_t id) {
    (void)id;
    return "test_channel";
}
} // namespace common
} // namespace Middleware
} // namespace hnu
#endif

void TestShowTransPerf() {
    assert(TransportEvent::ShowTransPerf(TransPerf::TRANSMIT_BEGIN) == "TRANSMIT_BEGIN");
    assert(TransportEvent::ShowTransPerf(TransPerf::SERIALIZE) == "SERIALIZE");
    assert(TransportEvent::ShowTransPerf(TransPerf::SEND) == "SEND");
    assert(TransportEvent::ShowTransPerf(TransPerf::MESSAGE_ARRIVE) == "MESSAGE_ARRIVE");
    assert(TransportEvent::ShowTransPerf(TransPerf::OBTAIN) == "OBTAIN");
    assert(TransportEvent::ShowTransPerf(TransPerf::DESERIALIZE) == "DESERIALIZE");
    assert(TransportEvent::ShowTransPerf(TransPerf::DISPATCH) == "DISPATCH");
    assert(TransportEvent::ShowTransPerf(TransPerf::NOTIFY) == "NOTIFY");
    assert(TransportEvent::ShowTransPerf(TransPerf::FETCH) == "FETCH");
    assert(TransportEvent::ShowTransPerf(TransPerf::CALLBACK) == "CALLBACK");
}

void TestTransportEventSerialization() {
    uint64_t ch_id = 0;
#if defined(USE_GLOBALDATA) && (USE_GLOBALDATA == 1)
    // When using real GlobalData implementation, register channel to get id
    ch_id = GlobalData::RegisterChannel("test_channel");
#endif

    TransportEvent ev;
    ev.set_eid(static_cast<int>(TransPerf::SEND));
    ev.set_channel_id(ch_id);
    ev.set_msg_seq(42);
    ev.set_stamp(123456789);
    ev.set_adder("peerA");

    std::string s = ev.SerializeToString();
    std::string expected = std::string("1\t") + std::to_string(static_cast<int>(TransPerf::SEND)) + "\t" + "test_channel" + "\t" + std::to_string(42) + "\t" + std::to_string(123456789) + "\t" + "peerA" + "\t";

    assert(s == expected);
}

int main() {
    TestShowTransPerf();
    TestTransportEventSerialization();

    std::cout << "Event tests passed" << std::endl;
    return 0;
}
