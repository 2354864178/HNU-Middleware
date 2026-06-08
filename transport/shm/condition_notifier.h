#pragma once

#include "notifier_base.h"
#include "../../common/macros.h"

#include <atomic>

namespace hnu {
namespace Middleware {
namespace transport {

const uint32_t kBufLength = 4096; // 可读信息数组的长度

// 条件变量通知器，使用共享内存实现进程间通信，提供通知和监听功能，支持单例模式
class ConditionNotifier : public NotifierBase {

    struct Indicator {
        std::atomic<uint64_t> next_seq = {0}; // 下一个可用的序列号，表示下一个可写入的位置
        ReadableInfo infos[kBufLength];       // 可读信息数组，存储每个位置的ReadableInfo对象，表示每个位置的状态和信息
        uint64_t seqs[kBufLength] = {0};      // 序列号数组，存储每个位置的序列号，表示每个位置的状态和信息
    };

public:
    virtual ~ConditionNotifier();                             // 虚析构函数，确保派生类的析构函数被正确调用，释放资源
    void Shutdown() override;                                 // 关闭通知器
    bool Notify(const ReadableInfo& info) override;           // 通知可读信息，返回是否成功
    bool Listen(int timeout_ms, ReadableInfo* info) override; // 监听可读信息，等待通知，并将可读信息写入info参数
    static const char* Type() {
        return "condition";
    }

private:
    bool Init();
    bool OpenOrCreate(); // 创建共享内存
    bool OpenOnly();     // 打开共享内存
    bool Remove();       // 移除共享内存
    void Reset();        // 重置共享内存

    key_t key_ = 0;                           // 标识IPC资源
    void* managed_shm_ = nullptr;             // 共享内存的地址，指向映射后的共享内存区域
    size_t shm_size_ = 0;                     // 共享内存的大小，表示共享内存区域的总字节数
    Indicator* indicator_ = nullptr;          // 指向共享内存中Indicator结构体的指针
    uint64_t next_seq_ = 0;                   // 下一个可用的序列号
    std::atomic<bool> is_shutdown_ = {false}; // 是否已经关闭通知器，使用原子变量保证线程安全
    DECLARE_SINGLETON(ConditionNotifier)      // 声明单例类，禁止复制和赋值操作
};

} // namespace transport
} // namespace Middleware
} // namespace hnu
