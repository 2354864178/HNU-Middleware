#include <iostream>
#include <cassert>
#include <cstdint>

#include "../attributes_filler.h"
#include "../../config/qos_profile.h"
#include "../../qos/qos_profile_conf.h"

using namespace hnu::Middleware::transport;

int main() {
    AttributesFiller filler;

    QosProfile qos; // defaults
    std::string channel = "test_channel";

    RtpsWriterAttributes wattr;
    bool ok = filler.FillInWriterAttr(channel, qos, &wattr);
    assert(ok);
    assert(wattr.Tatt.topicName == channel);
    assert(static_cast<uint32_t>(wattr.hatt.payloadMaxSize) == qos.msg_size + 255);

    RtpsReaderAttributes rattr;
    ok = filler.FillInReaderAttr(channel, qos, &rattr);
    assert(ok);
    assert(rattr.Tatt.topicName == channel);

    // depth overflow -> negative depth after cast -> should return false
    qos.depth = UINT32_MAX; // large value to provoke negative int32_t
    ok = filler.FillInWriterAttr(channel, qos, &wattr);
    assert(!ok);

    ok = filler.FillInReaderAttr(channel, qos, &rattr);
    assert(!ok);

    std::cout << "attributes_filler_test: OK\n";
    return 0;
}
