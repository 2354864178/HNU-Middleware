#include "transport/rtps/participant.h"
#include "transport/transport.h"

namespace hnu {
namespace Middleware {
namespace transport {

// Minimal stub implementations to avoid depending on Fast-DDS during unit tests.
Participant::Participant(const std::string& name, int send_port, eprosima::fastdds::rtps::RTPSParticipantListener* listener) : shutdown_(false), name_(name), send_port_(send_port), listener_(listener), fastrtps_participant_(nullptr) {}

Participant::~Participant() {}

void Participant::Shutdown() {
    shutdown_.store(true);
    fastrtps_participant_ = nullptr;
}

eprosima::fastdds::rtps::RTPSParticipant* Participant::fastrtps_participant() {
    return nullptr;
}

RtpsDispatcher* RtpsDispatcher::Instance(bool create_if_needed) {
    static RtpsDispatcher instance;
    return create_if_needed ? &instance : &instance;
}

void RtpsDispatcher::set_participant(const ParticipantPtr& participant) {
    (void)participant;
}

} // namespace transport
} // namespace Middleware
} // namespace hnu
