#pragma once

#include "../base/atomic_hash_map.h"
#include "macros.h"

#include <string>

namespace hnu {
namespace Middleware {
namespace common {

class GlobalData {
    public:
    ~GlobalData();

    // 返回当前进程的ID
    int ProcessId() const;
    void SetProcessGroup(const std::string& process_group);
    const std::string& HostIp() const;
    const std::string& HostName() const;
    const std::string& ProcessGroup() const;

    private:
    void InitHostInfo();
    // 运行机器的配置信息
    std::string host_ip_;
    std::string host_name_;

    // 当前进程的信息
    int process_id_;
    std::string process_group_;

    std::string sched_name_ = "HNU_CMW_DEFAULT";

    // GlobalData为全局单例
    DECLARE_SINGLETON(GlobalData)
};

} // namespace common
} // namespace Middleware
} // namespace hnu
