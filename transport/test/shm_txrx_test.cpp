#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "transport/config/RoleAttributes.h"
#include "transport/common/identity.h"
#include "transport/receiver/shm_receiver.h"
#include "transport/transmitter/shm_transmitter.h"
#include "common/global_data.h"
#include "logger/logger.h"
#include "serialize/serializable.h"

using namespace hnu::Middleware::transport;

namespace {

struct TestMessage : public hnu::Middleware::serialize::Serializable {
    int32_t id = 0;
    std::string text;

    SERIALIZE(id, text)
};

RoleAttributes BuildAttr(const std::string& channel_name) {
    RoleAttributes attr;
    attr.channel_name = channel_name;
    attr.channel_id = hnu::Middleware::common::GlobalData::RegisterChannel(channel_name);
    attr.host_ip = hnu::Middleware::common::GlobalData::Instance()->HostIp();
    attr.host_name = hnu::Middleware::common::GlobalData::Instance()->HostName();
    attr.process_id = hnu::Middleware::common::GlobalData::Instance()->ProcessId();
    return attr;
}

int RunReader() {
    constexpr int kExpect = 10;
    std::atomic<int> received{0};

    RoleAttributes attr = BuildAttr("transport_shm_txrx_test");
    std::cout << "[READER] channel_id=" << attr.channel_id << " host_ip=" << attr.host_ip << std::endl;

    ShmReceiver<TestMessage> receiver(attr, [&](const std::shared_ptr<TestMessage>& msg, const MessageInfo& info, const RoleAttributes&) {
        int count = ++received;
        std::cout << "[READER] recv#" << count << " id=" << msg->id << " text=" << msg->text << " seq=" << info.seq_num() << std::endl;
    });

    receiver.Enable();

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(15);
    while (std::chrono::steady_clock::now() < deadline) {
        if (received.load() >= kExpect) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    receiver.Disable();

    std::cout << "[READER] total received=" << received.load() << std::endl;
    return received.load() >= kExpect ? 0 : 2;
}

int RunWriter() {
    constexpr int kSend = 10;

    RoleAttributes attr = BuildAttr("transport_shm_txrx_test");
    std::cout << "[WRITER] channel_id=" << attr.channel_id << " host_ip=" << attr.host_ip << std::endl;

    ShmTransmitter<TestMessage> transmitter(attr);
    transmitter.Enable();

    MessageInfo msg_info;
    msg_info.set_sender_id(Identity());

    for (int i = 0; i < kSend; ++i) {
        auto msg = std::make_shared<TestMessage>();
        msg->id = i;
        msg->text = "hello_" + std::to_string(i);

        msg_info.set_seq_num(static_cast<uint64_t>(i + 1));
        bool ok = transmitter.Transmit(msg, msg_info);
        std::cout << "[WRITER] send#" << i << " ok=" << (ok ? 1 : 0) << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(120));
    }

    transmitter.Disable();
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    hnu::Middleware::logger::Logger::Instance()->open("shm_txrx_test.log");

    if (argc < 2) {
        std::cout << "Usage: ./shm_txrx_test reader|writer" << std::endl;
        return 1;
    }

    if (std::strcmp(argv[1], "reader") == 0) {
        return RunReader();
    }

    if (std::strcmp(argv[1], "writer") == 0) {
        return RunWriter();
    }

    std::cout << "Unknown role: " << argv[1] << std::endl;
    return 1;
}
