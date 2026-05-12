#pragma once

#include <memory>

#include "identity.h"
#include "../config/RoleAttributes.h"

namespace hnu {
namespace Middleware {
namespace transport {

using namespace config;
class Endpoint;
using EndpointPtr = std::shared_ptr<Endpoint>;

// Endpoint类，表示通信的端点，包含一个标识符和角色属性
class Endpoint {
    public:
    explicit Endpoint(const RoleAttributes& attr);
    virtual ~Endpoint();

    const Identity& id() const {
        return id_;
    }
    const RoleAttributes& attributes() const {
        return attr_;
    }

    private:
    bool enabled_;
    Identity id_;
    RoleAttributes attr_;
};

} // namespace transport
} // namespace Middleware
} // namespace hnu
