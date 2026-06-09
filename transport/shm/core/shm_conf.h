#pragma once

#include <cstdint>
#include <string>

namespace hnu {
namespace Middleware {
namespace transport {

// 共享内存配置类，根据消息大小来计算共享内存的大小和块的数量
class ShmConf {

public:
    ShmConf();
    explicit ShmConf(const uint64_t& real_msg_size);
    virtual ~ShmConf();

    void Update(const uint64_t& real_msg_size); // 更新共享内存配置，根据新的消息大小重新计算共享内存的大小和块的数量

    // 获取消息大小的上限
    const uint64_t& ceiling_msg_size() {
        return ceiling_msg_size_;
    }

    // 获取每个block的buf的大小
    const uint64_t& block_buf_size() {
        return block_buf_size_;
    }

    // 获取块的数量
    const uint64_t& block_num() {
        return block_num_;
    }

    // 获取管理的共享内存的大小
    const uint64_t& managed_shm_size() {
        return managed_shm_size_;
    }

private:
    uint64_t GetCeilingMessageSize(const uint64_t& real_msg_size); // 获取消息大小的上限，根据实际消息大小计算出一个合适的上限，通常是一个预设的值或者是实际消息大小的某个倍数
    uint64_t GetBlockBufSize(const uint64_t& ceiling_msg_size);    // 获取每个block的buf的大小，根据消息大小的上限计算出每个block的buf的大小，通常是一个预设的值或者是消息大小上限的某个倍数
    uint32_t GetBlockNum(const uint64_t& ceiling_msg_size);        // 获取块的数量，根据消息大小的上限计算出需要多少个块来存储消息，通常是一个预设的值或者是根据消息大小上限和每个block的buf的大小计算出来的值

    uint64_t ceiling_msg_size_; // 消息上限大小
    uint64_t block_buf_size_;   // 每个block的buf的大小
    uint64_t block_num_;        // 块的数量
    uint64_t managed_shm_size_; // 管理的共享内存的大小

    static const uint64_t EXTRA_SIZE; // 管理区大小，单位是字节，包含State和Block的元信息的大小
    static const uint64_t STATE_SIZE; // State对象的大小，单位是字节，包含State对象的成员变量的大小
    static const uint64_t BLOCK_SIZE; // Block对象的大小，单位是字节，包含Block对象的成员变量的大小

    static const uint64_t MESSAGE_INFO_SIZE; // 消息头的大小，单位是字节，包含MessageInfo对象的成员变量的大小

    static const uint32_t BLOCK_NUM_16K;    // 消息大小在0-16K时块的数量
    static const uint64_t MESSAGE_SIZE_16K; // 消息大小在0-16K时的消息上限大小，单位是字节

    static const uint32_t BLOCK_NUM_128K;    // 消息大小在10K-100K时块的数量
    static const uint64_t MESSAGE_SIZE_128K; // 消息大小在10K-100K时的消息上限大小，单位是字节

    static const uint32_t BLOCK_NUM_1M;    // 消息大小在100K-1M时块的数量
    static const uint64_t MESSAGE_SIZE_1M; // 消息大小在100K-1M时的消息上限大小，单位是字节

    static const uint32_t BLOCK_NUM_8M;    // 消息大小在1M-6M时块的数量
    static const uint64_t MESSAGE_SIZE_8M; // 消息大小在1M-6M时的消息上限大小，单位是字节

    static const uint32_t BLOCK_NUM_16M;    // 消息大小在6M-10M时块的数量
    static const uint64_t MESSAGE_SIZE_16M; // 消息大小在6M-10M时的消息上限大小，单位是字节

    static const uint32_t BLOCK_NUM_MORE;    // 消息大小在10M以上时块的数量
    static const uint64_t MESSAGE_SIZE_MORE; // 消息大小在10M以上时的消息上限大小，单位是字节
};

} // namespace transport
} // namespace Middleware
} // namespace hnu
