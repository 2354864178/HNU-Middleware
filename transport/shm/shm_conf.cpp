#include <iostream>
#include "shm_conf.h"

namespace hnu {
namespace Middleware {
namespace transport {

// ShmConf 构造函数，默认消息大小上限为16K
ShmConf::ShmConf() {
    Update(MESSAGE_SIZE_16K);
}

// ShmConf 构造函数，根据实际消息大小计算出一个合适的上限，通常是一个预设的值或者是实际消息大小的某个倍数
ShmConf::ShmConf(const uint64_t& real_msg_size) {
    Update(real_msg_size);
}

ShmConf::~ShmConf() {}

const uint64_t ShmConf::EXTRA_SIZE = 1024 * 4; // 区大小为4K，包含State和Block的元信息的大小
const uint64_t ShmConf::STATE_SIZE = 1024;     // State对象的大小为1K，包含State对象的成员变量的大小

const uint64_t ShmConf::BLOCK_SIZE = 1024;        // Block对象的大小为1K，包含Block对象的成员变量的大小
const uint64_t ShmConf::MESSAGE_INFO_SIZE = 1024; // 消息头的大小为1K，包含MessageInfo对象的成员变量的大小

const uint32_t ShmConf::BLOCK_NUM_16K = 512;          // 消息大小在0-16K时块的数量为512
const uint64_t ShmConf::MESSAGE_SIZE_16K = 1024 * 16; // 消息大小在0-16K时的消息上限大小为16K，单位是字节

const uint32_t ShmConf::BLOCK_NUM_128K = 128;           // 消息大小在10K-100K时块的数量为128
const uint64_t ShmConf::MESSAGE_SIZE_128K = 1024 * 128; // 消息大小在10K-100K时的消息上限大小为128K，单位是字节

const uint32_t ShmConf::BLOCK_NUM_1M = 64;             // 消息大小在100K-1M时块的数量为64
const uint64_t ShmConf::MESSAGE_SIZE_1M = 1024 * 1024; // 消息大小在100K-1M时的消息上限大小为1M，单位是字节

const uint32_t ShmConf::BLOCK_NUM_8M = 32;
const uint64_t ShmConf::MESSAGE_SIZE_8M = 1024 * 1024 * 8;

const uint32_t ShmConf::BLOCK_NUM_16M = 16;
const uint64_t ShmConf::MESSAGE_SIZE_16M = 1024 * 1024 * 16;

const uint32_t ShmConf::BLOCK_NUM_MORE = 8;                   // 消息大小在10M以上时块的数量为8
const uint64_t ShmConf::MESSAGE_SIZE_MORE = 1024 * 1024 * 32; // 消息大小在10M以上时的消息上限大小为32M，单位是字节

// 更新共享内存配置，根据新的消息大小重新计算共享内存的大小和块的数量
void ShmConf::Update(const uint64_t& real_msg_size) {
    ceiling_msg_size_ = GetCeilingMessageSize(real_msg_size);                                  // 获取消息大小的上限
    block_buf_size_ = GetBlockBufSize(ceiling_msg_size_);                                      // 获取每个block的buf的大小
    block_num_ = GetBlockNum(ceiling_msg_size_);                                               // 获取块的数量
    managed_shm_size_ = EXTRA_SIZE + STATE_SIZE + (BLOCK_SIZE + block_buf_size_) * block_num_; // 计算管理的共享内存的大小，包含管理区大小、State对象的大小、Block对象的大小和每个block的buf的大小乘以块的数量
}

uint64_t ShmConf::GetCeilingMessageSize(const uint64_t& real_msg_size) {
    uint64_t ceiling_msg_size = MESSAGE_SIZE_16K;
    if (real_msg_size <= MESSAGE_SIZE_16K) {
        ceiling_msg_size = MESSAGE_SIZE_16K;
    } else if (real_msg_size <= MESSAGE_SIZE_128K) {
        ceiling_msg_size = MESSAGE_SIZE_128K;
    } else if (real_msg_size <= MESSAGE_SIZE_1M) {
        ceiling_msg_size = MESSAGE_SIZE_1M;
    } else if (real_msg_size <= MESSAGE_SIZE_8M) {
        ceiling_msg_size = MESSAGE_SIZE_8M;
    } else if (real_msg_size <= MESSAGE_SIZE_16M) {
        ceiling_msg_size = MESSAGE_SIZE_16M;
    } else {
        ceiling_msg_size = MESSAGE_SIZE_MORE;
    }
    return ceiling_msg_size;
}

// 获取每个block的buf的大小，根据消息大小的上限计算出每个block的buf的大小
uint64_t ShmConf::GetBlockBufSize(const uint64_t& ceiling_msg_size) {
    return ceiling_msg_size + MESSAGE_INFO_SIZE;
}

// 获取块的数量，根据消息大小的上限计算出需要多少个块来存储消息
uint32_t ShmConf::GetBlockNum(const uint64_t& ceiling_msg_size) {
    uint32_t num = 0;
    switch (ceiling_msg_size) {
    case MESSAGE_SIZE_16K:
        num = BLOCK_NUM_16K;
        break;
    case MESSAGE_SIZE_128K:
        num = BLOCK_NUM_128K;
        break;
    case MESSAGE_SIZE_1M:
        num = BLOCK_NUM_1M;
        break;
    case MESSAGE_SIZE_8M:
        num = BLOCK_NUM_8M;
        break;
    case MESSAGE_SIZE_16M:
        num = BLOCK_NUM_16M;
        break;
    case MESSAGE_SIZE_MORE:
        num = BLOCK_NUM_MORE;
        break;
    default:
        std::cout << "unknown ceiling_msg_size[" << ceiling_msg_size << "]" << std::endl;
        break;
    }
    return num;
}

} // namespace transport
} // namespace Middleware
} // namespace hnu