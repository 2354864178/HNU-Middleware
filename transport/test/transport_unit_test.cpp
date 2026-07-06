#include <iostream>

#include "transport/transport.h"

using namespace hnu::Middleware::transport;

int main() {
    auto transport = Transport::Instance();
    if (transport == nullptr) {
        std::cerr << "Transport::Instance() returned nullptr" << std::endl;
        return 1;
    }

    auto participant = transport->participant();
    if (participant == nullptr) {
        std::cerr << "participant is nullptr after Transport construction" << std::endl;
        return 2;
    }

    std::cout << "Transport created participant successfully" << std::endl;

    transport->Shutdown();
    if (transport->participant() != nullptr) {
        std::cerr << "participant should be nullptr after Shutdown" << std::endl;
        return 3;
    }

    std::cout << "Transport shutdown successfully" << std::endl;
    return 0;
}
