#include "dispatcher.h"

namespace hnu {
namespace Middleware {
namespace transport {

Dispatcher::Dispatcher() : is_shutdown_(false) {}

Dispatcher::~Dispatcher() {
    Shutdown();
}

void Dispatcher::Shutdown() {
    is_shutdown_.store(true);
    std::cout << "Shutdown" << std::endl;
}

bool Dispatcher::HasChannel(uint64_t channel_id) {
    return msg_listeners_.Has(channel_id);
}

} // namespace transport
} // namespace Middleware
} // namespace hnu
