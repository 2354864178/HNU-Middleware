#pragma once

#include <atomic>
#include <cstring>
#include <mutex>

namespace hnu {
namespace Middleware {
namespace transport {

// State代表共享内存的状态，包含消息大小的上限、引用计数、消息序列号和是否需要内存映射等信息
class State {

public:
    explicit State(const uint64_t& ceiling_msg_size); // 构造函数，接受一个 ceiling_msg_size 参数，用于初始化 ceiling_msg_size_ 成员变量
    virtual ~State();

    // 减少引用计数，表示有一个消息不再使用这个状态对象了
    void DecreaseReferenceCounts() {
        // 这里不用fetch_sub是因为需要在引用计数为0时执行一些特殊的逻辑（比如释放资源），而fetch_sub无法保证在引用计数为0时正确地执行这些逻辑
        /*
        初始：reference = 1
        T1 和 T2 同时执行 load()，都得到 cnt==1
        线程 T1 执行 fetch_sub(1) 原子地返回 old=1，写入 new=0
        线程 T2 紧接着执行 fetch_sub(1) 原子地返回 old=0，写入 new=UINT_MAX（下溢）
        结果：最终 reference = UINT_MAX（错误），并且只有 T1 看到 old==1（常用模式
        */
        uint32_t current_reference_count = reference_count_.load();
        do {
            if (current_reference_count == 0) {
                return;
            }
        } while (!reference_count_.compare_exchange_strong(current_reference_count, current_reference_count - 1));
        // 这里采用了循环处理，所以使用compare_exchange_weak也可以，因为即使发生虚假唤醒，循环会继续尝试直到成功减少引用计数
        // std::atomic<int> a{1}; int expected = 1;
        // bool ok = a.compare_exchange_weak(expected, 0);
        // 操作返回 false，但 a 仍然是 1（原子对象没有被修改），expected 可能仍然是 1（或者实现将当前值写回 expected，值不变）。关键：没有发生写入。
    }

    // 增加引用计数
    void IncreaseReferenceCounts() {
        // 多个线程同时 fetch_add(1) 是安全的，最终值等于各次增加之和，所以这里直接使用 fetch_add(1)，不需要额外的同步机制
        reference_count_.fetch_add(1);
    }

    // 增加消息序列号，并返回增加前的值
    uint32_t FetchAddSeq(uint32_t diff) {
        return seq_.fetch_add(diff);
    }

    // 获取当前的消息序列号
    uint32_t seq() {
        return seq_.load();
    }

    // 设置是否需要内存映射
    void set_need_remap(bool need) {
        need_remap_.store(need);
    }

    // 获取是否需要内存映射
    bool need_remap() {
        return need_remap_;
    }

    // 获取消息大小的上限
    uint64_t ceiling_msg_size() {
        return ceiling_msg_size_.load();
    }

    // 返回引用计数
    uint32_t reference_counts() {
        return reference_count_.load();
    }

private:
    std::atomic<bool> need_remap_ = {false};      // 是否需要内存映射，初始值为 false，表示默认不需要内存映射
    std::atomic<uint32_t> seq_ = {0};             // 消息序列号，可以用来区分消息的先后顺序
    std::atomic<uint32_t> reference_count_ = {0}; // 引用计数，表示当前有多少消息正在使用这个状态对象
    std::atomic<uint64_t> ceiling_msg_size_;      // 消息大小的上限，超过这个大小的消息需要进行内存映射
};

} // namespace transport
} // namespace Middleware
} // namespace hnu
