#include "endpoint.h"
#include "../../common/global_data.h"

namespace hnu {
namespace Middleware {
namespace transport {

//  构造函数，使用RoleAttributes初始化Endpoint
Endpoint::Endpoint(const RoleAttributes& attr) : enabled_(false), id_(), attr_(attr) {

    // 如果RoleAttributes中的host_name为空，则使用GlobalData中的HostName
    if (!attr_.host_name.empty()) {
        attr_.host_name = common::GlobalData::Instance()->HostName();
    }

    // 如果RoleAttributes中的process_id为0，则使用GlobalData中的ProcessId
    if (!attr.process_id) {
        attr_.process_id = common::GlobalData::Instance()->ProcessId();
    }

    // 如果RoleAttributes中的id为0，则使用Identity的HashValue作为id
    if (!attr_.id) {
        attr_.id = id_.HashValue();
    }
}

Endpoint::~Endpoint() {}

} // namespace transport
} // namespace Middleware
} // namespace hnu