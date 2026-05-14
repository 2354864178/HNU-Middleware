#pragma once

#include "../base/atomic_hash_map.h"
#include "macros.h"

#include <string>

namespace hnu {
namespace Middleware {
namespace common {

using namespace base;
// 全局数据类，包含主机信息和进程信息
class GlobalData {
public:
    ~GlobalData();

    // 返回当前进程的ID
    int ProcessId() const;
    void SetProcessGroup(const std::string& process_group);
    const std::string& HostIp() const;
    const std::string& HostName() const;
    const std::string& ProcessGroup() const;

    static std::string GetChannelById(uint64_t id);

    static uint64_t RegisterChannel(const std::string& channel);
    static uint64_t RegisterNode(const std::string& node_name);

private:
    void InitHostInfo();
    // 运行机器的配置信息
    std::string host_ip_;
    std::string host_name_;

    // 当前进程的信息
    int process_id_;
    std::string process_group_;

    std::string sched_name_ = "HNU-Middleware";

    // 在创建新的channel时会注册进此全局map
    static AtomicHashMap<uint64_t, std::string, 256> channel_id_map_; // 全局 channel_id_map_ 表
    static AtomicHashMap<uint64_t, std::string, 512> node_id_map_;

    // GlobalData为全局单例
    DECLARE_SINGLETON(GlobalData)
};

} // namespace common
} // namespace Middleware
} // namespace hnu
