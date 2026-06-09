#pragma once

#include <memory>
#include "notifier_base.h"

namespace hnu {
namespace Middleware {
namespace transport {

class NotifierFactory {
public:
    static NotifierPtr CreateNotifier();

private:
    static NotifierPtr CreateConditionNotifier();
    static NotifierPtr CreateMulticastNotifier();
};

} // namespace transport
} // namespace Middleware
} // namespace hnu
