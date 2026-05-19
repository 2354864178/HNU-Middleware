#include <iostream>
#include <cassert>

#include "../participant.h"

using namespace hnu::Middleware::transport;

int main() {
    Participant p("unit_test_participant", 0);
    // create participant
    auto part = p.fastrtps_participant();
    assert(part != nullptr);

    // shutdown
    p.Shutdown();
    assert(p.is_shutdown());
    // after shutdown, fastrtps_participant should be nullptr
    auto part2 = p.fastrtps_participant();
    assert(part2 == nullptr);

    std::cout << "participant_test: OK\n";
    return 0;
}
