#include <string>

#include "notifier_factory.h"
#include "condition_notifier.h"
#include "multicast_notifier.h"
#include "../../common/global_data.h"
#include "../../common/log.h"

namespace hnu {
namespace Middleware {
namespace transport {

using common::GlobalData;

auto NotifierFactory::CreateNotifier() -> NotifierPtr {

    // 默认为共享内存的方式
    std::string notifier_type(ConditionNotifier::Type());
    // 需要根据配置文件传入Notifier type
    if (notifier_type == MulticastNotifier::type()) {
        return CreateMulticastNotifier();
    } else if (notifier_type == ConditionNotifier::Type()) {
        return CreateConditionNotifier();
    }

    AINFO << "unknown notifier, we use default notifier: " << notifier_type;
    return CreateConditionNotifier();
}

auto NotifierFactory::CreateConditionNotifier() -> NotifierPtr {
    return ConditionNotifier::Instance();
}

auto NotifierFactory::CreateMulticastNotifier() -> NotifierPtr {
    return MulticastNotifier::Instance();
}

} // namespace transport
} // namespace Middleware
} // namespace hnu