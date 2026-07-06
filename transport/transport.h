#pragma once

#include "transport/rtps/participant.h"
#include "common/macros.h"
#include "transport/config/RoleAttributes.h"
#include "transport/config/transport_config.h"
#include "transport/transmitter/transmitter.h"
#include "transport/receiver/receiver.h"
#include "common/log.h"

#ifndef HNU_TRANSPORT_UNIT_TEST_LIGHT
#include "transport/transmitter/rtps_transmitter.h"
#include "transport/dispatcher/rtps_dispatcher.h"
#include "transport/receiver/rtps_receiver.h"
#include "transport/transmitter/shm_transmitter.h"
#include "transport/receiver/shm_receiver.h"
#endif

namespace hnu {
namespace Middleware {
namespace transport {

using namespace config;

#ifdef HNU_TRANSPORT_UNIT_TEST_LIGHT
template <typename M> class RtpsTransmitter;
template <typename M> class ShmTransmitter;
template <typename M> class RtpsReceiver;
template <typename M> class ShmReceiver;

class RtpsDispatcher {
public:
    static RtpsDispatcher* Instance(bool create_if_needed = true);
    void set_participant(const ParticipantPtr& participant);
};

using RtpsDispatcherPtr = RtpsDispatcher*;
#endif

class Transport {
public:
    virtual ~Transport();

    void Shutdown();

    // 返回一个Transmitter的指针
    template <typename M> auto CreateTransmitter(const RoleAttributes& attr, const OptionalMode& mode = OptionalMode::RTPS) -> typename std::shared_ptr<Transmitter<M>>;

    // 返回一个Receiver的指针
    template <typename M> auto CreateReceiver(const RoleAttributes& attr, const typename Receiver<M>::MessageListener& msg_listener, const OptionalMode& mode = OptionalMode::RTPS) -> typename std::shared_ptr<Receiver<M>>;

    // 返回在构造函数中创建的participant_
    ParticipantPtr participant() const {
        return participant_;
    }

private:
    // 构造函数会调用，来创建一个participant
    void CreateParticipant();
    ParticipantPtr participant_ = nullptr;
    std::atomic<bool> is_shutdown_ = {false};

    RtpsDispatcherPtr rtps_dispatcher_ = nullptr;

    DECLARE_SINGLETON(Transport)
};

template <typename M> auto Transport::CreateTransmitter(const RoleAttributes& attr, const OptionalMode& mode) -> typename std::shared_ptr<Transmitter<M>> {
    if (is_shutdown_.load()) {
        std::cout << "transport has been shut down." << std::endl;
        return nullptr;
    }
    std::shared_ptr<Transmitter<M>> transmitter = nullptr;
    RoleAttributes modified_attr = attr;

    // 默认走fast-rtps
    switch (mode) {
    case OptionalMode::SHM:
        transmitter = std::make_shared<ShmTransmitter<M>>(modified_attr);
        break;
    default:
        transmitter = std::make_shared<RtpsTransmitter<M>>(modified_attr, participant());
        break;
    }

    RETURN_VAL_IF_NULL(transmitter, nullptr);

    if (mode != OptionalMode::HYBRID) {
        ADEBUG << "transmitter Enable";
        transmitter->Enable();
    }
    AINFO << "CreateTransmitter Sucess";
    return transmitter;
}

template <typename M> auto Transport::CreateReceiver(const RoleAttributes& attr, const typename Receiver<M>::MessageListener& msg_listener, const OptionalMode& mode) -> typename std::shared_ptr<Receiver<M>> {
    if (is_shutdown_.load()) {
        AINFO << "transport has been shut down.";
        return nullptr;
    }

    // 新建一个Receiver<M>类型的共享指针
    std::shared_ptr<Receiver<M>> receiver = nullptr;

    RoleAttributes modified_attr = attr;
    ADEBUG << "Receiver Mode: " << mode;
    switch (mode) {
    case OptionalMode::SHM:
        receiver = std::make_shared<ShmReceiver<M>>(modified_attr, msg_listener);
        break;
    default:
        receiver = std::make_shared<RtpsReceiver<M>>(modified_attr, msg_listener);
        break;
    }

    // 保证receiver不为空
    RETURN_VAL_IF_NULL(receiver, nullptr);

    if (mode != OptionalMode::HYBRID) {
        receiver->Enable();
    }
    return receiver;
}

} // namespace transport
} // namespace Middleware
} // namespace hnu
