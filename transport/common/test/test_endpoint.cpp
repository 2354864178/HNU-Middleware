#include <iostream>

#include "transport/common/identity.h"
#include "transport/common/endpoint.h"
#include "common/global_data.h"
#include "transport/config/RoleAttributes.h"
#include "serialize/serializable.h"

using namespace hnu::Middleware::transport;
using namespace hnu::Middleware::common;

int main() {
    // 初始化全局单例（会在构造中设置host/process信息）
    GlobalData::Instance();

    RoleAttributes attr;
    attr.host_name = "";
    attr.process_id = 0;
    attr.id = 0;

    Endpoint ep(attr);

    std::cout << "Identity hash: " << ep.id().ToString() << std::endl;
    std::cout << "Role host_name: " << ep.attributes().host_name << std::endl;
    std::cout << "Role process_id: " << ep.attributes().process_id << std::endl;
    std::cout << "Role id: " << ep.attributes().id << std::endl;

    return 0;
}
