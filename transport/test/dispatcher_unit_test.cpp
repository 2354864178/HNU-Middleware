#include <iostream>
#include <memory>
#include <string>

#include "transport/dispatcher/dispatcher.h"
#include "transport/message/listener_handler.h"
#include "transport/config/RoleAttributes.h"
#include "transport/message/message_info.h"
#include "transport/common/identity.h"

using namespace hnu::Middleware::transport;

class TestDispatcher : public Dispatcher {
public:
    void TriggerString(uint64_t channel_id, const std::shared_ptr<std::string>& msg, const MessageInfo& info) {
        ListenerHandlerBasePtr* handler_base = nullptr;
        if (msg_listeners_.Get(channel_id, &handler_base)) {
            auto handler = std::dynamic_pointer_cast<ListenerHandler<std::string>>(*handler_base);
            if (handler) {
                handler->Run(msg, info);
            } else {
                std::cout << "handler cast failed" << std::endl;
            }
        } else {
            std::cout << "no handler for channel" << std::endl;
        }
    }
};

int main() {
    TestDispatcher d;

    RoleAttributes attr;
    attr.channel_name = "test_channel";
    attr.channel_id = 0xabc123;
    attr.id = 1001; // self id for this receiver

    // create a sender identity and set opposite_attr.id to its hash
    Identity sender(true);
    RoleAttributes opposite_attr;
    opposite_attr.id = sender.HashValue();

    // generic listener
    d.AddListener<std::string>(attr, [](const std::shared_ptr<std::string>& msg, const MessageInfo& info) { std::cout << "Generic listener got msg: " << *msg << " seq=" << info.seq_num() << std::endl; });

    // directed listener (only invoked when sender matches opposite_attr.id)
    d.AddListener<std::string>(attr, opposite_attr, [](const std::shared_ptr<std::string>& msg, const MessageInfo& info) { std::cout << "Directed listener got msg from sender hash " << info.sender_id().HashValue() << " msg=" << *msg << std::endl; });

    MessageInfo msg_info(sender, 42);

    auto payload = std::make_shared<std::string>("hello dispatcher");

    std::cout << "Triggering message..." << std::endl;
    d.TriggerString(attr.channel_id, payload, msg_info);

    return 0;
}
