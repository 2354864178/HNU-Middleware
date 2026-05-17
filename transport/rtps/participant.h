#pragma once

#include <string>
#include <atomic>
#include <memory>
#include <mutex>
#include <fastdds/rtps/participant/RTPSParticipantListener.hpp>

namespace hnu {
namespace Middleware {
namespace transport {

class Participant;
using ParticipantPtr = std::shared_ptr<Participant>;

class Participant {
public:
    Participant(const std::string& name, int send_port, eprosima::fastdds::rtps::RTPSParticipantListener* listener = nullptr);
    virtual ~Participant();

    void Shutdown();
    bool is_shutdown() const {
        return shutdown_.load();
    }

    // 返回rtps的Participant
    eprosima::fastdds::rtps::RTPSParticipant* fastrtps_participant();

private:
    // 创建一个 rtps的Participant
    void CreateFastRtpsParticipant(const std::string& name, int send_port, eprosima::fastdds::rtps::RTPSParticipantListener* listener);

    std::atomic<bool> shutdown_;
    std::string name_; // RTPSParticipant name
    int send_port_;
    eprosima::fastdds::rtps::RTPSParticipant* fastrtps_participant_; // RTPSParticipant
    eprosima::fastdds::rtps::RTPSParticipantListener* listener_;     // RTPSParticipantListener
    std::mutex mutex_;
};

} // namespace transport
} // namespace Middleware
} // namespace hnu
