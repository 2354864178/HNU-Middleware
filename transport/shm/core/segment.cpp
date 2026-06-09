#include <iostream>

#include "segment.h"
#include "common/log.h"

namespace hnu {
namespace Middleware {
namespace transport {

// Segment的构造函数，接受一个channel_id参数，用于初始化channel_id_成员变量，同时将其他成员变量初始化为默认值
Segment::Segment(uint64_t channel_id) : init_(false), conf_(), channel_id_(channel_id), state_(nullptr), blocks_(nullptr), managed_shm_(nullptr), block_buf_lock_(), block_buf_addrs_() {}

// 用于获取一个可写的块，接受消息大小和一个WritableBlock指针作为参数，如果成功获取到一个可写的块，则将WritableBlock指针指向该块，并返回true；如果获取失败，则返回false
bool Segment::AcquireBlockToWrite(std::size_t msg_size, WritableBlock* writable_block) {
    RETURN_VAL_IF_NULL(writable_block, false);
    // 如果共享内存没有初始化成功，并且尝试打开或创建共享内存失败，则输出错误日志并返回false，表示无法获取可写的块
    if (!init_ && !OpenOrCreate()) {
        AERROR << "create shm failed, can't write now.";
        return false;
    }

    bool result = true;
    if (state_->need_remap()) {
        result = Remap();
    }

    // 如果msg_size 超过默认的 size，则销毁之前创建的共享内存，重新创建一块更大的内存
    if (msg_size > conf_.ceiling_msg_size()) {
        AERROR << "msg_size: " << msg_size << " larger than current shm_buffer_size: " << conf_.ceiling_msg_size() << " , need recreate.";
        result = Recreate(msg_size);
    }

    // 如果需要重新映射或者重新创建共享内存失败，则输出错误日志并返回false，表示无法获取可写的块
    if (!result) {
        AERROR << "segment update failed.";
        return false;
    }

    uint32_t index = GetNextWritableBlockIndex();
    // 将writable_block 指向 blocks_[index]
    writable_block->index = index;
    writable_block->block = &blocks_[index];
    writable_block->buf = block_buf_addrs_[index];
    return true;
}

// 释放Block的写锁
void Segment::ReleaseWrittenBlock(const WritableBlock& writable_block) {
    auto index = writable_block.index;
    if (index >= conf_.block_num()) {
        return;
    }
    // 释放写锁
    blocks_[index].ReleaseWriteLock();
}

// 用于获取一个可读的块，接受一个ReadableBlock指针作为参数，如果成功获取到一个可读的块，则将ReadableBlock指针指向该块，并返回true；如果获取失败，则返回false
bool Segment::AcquireBlockToRead(ReadableBlock* readable_block) {
    RETURN_VAL_IF_NULL(readable_block, false);
    // 如果共享内存没有初始化成功，并且尝试打开已存在的共享内存失败，则输出错误日志并返回false，表示无法获取可读的块
    if (!init_ && !OpenOnly()) {
        AERROR << "failed to open shared memory, can't read now.";
        return false;
    }

    auto index = readable_block->index;
    if (index >= conf_.block_num()) {
        AERROR << "invalid block_index[" << index << "].";
        return false;
    }

    bool result = true;
    if (state_->need_remap()) {
        result = Remap();
    }

    if (!result) {
        AERROR << "segment update failed.";
        return false;
    }

    if (!blocks_[index].TryLockForRead()) {
        return false;
    }
    readable_block->block = blocks_ + index;
    readable_block->buf = block_buf_addrs_[index];
    return true;
}

// 释放Block的读锁
void Segment::ReleaseReadBlock(const ReadableBlock& readable_block) {
    auto index = readable_block.index;
    if (index >= conf_.block_num()) {
        return;
    }
    blocks_[index].ReleaseReadLock();
}

// 销毁共享内存，释放资源
bool Segment::Destroy() {
    // 如果共享内存没有初始化成功，则直接返回true，表示销毁成功，因为没有资源需要释放
    if (!init_) {
        return true;
    }
    init_ = false;

    if (state_ == nullptr) {
        AERROR << "state_ is null in Destroy().";
        return false;
    }

    // 减少引用计数，表示有一个消息不再使用这个状态对象了
    state_->DecreaseReferenceCounts();
    uint32_t reference_counts = state_->reference_counts();
    if (reference_counts == 0) {
        return Remove();
    }

    ADEBUG << "destroy.";
    return true;
}

// 在本进程重新映射已经存在的共享内存段，不创建新段，仅重置本地映射并调用 OpenOnly() 重新打开
bool Segment::Remap() {
    init_ = false;
    ADEBUG << "before reset.";
    Reset(); // 重置共享内存的状态，清空数据等
    ADEBUG << "after reset.";
    return OpenOnly(); // 打开已存在的共享内存，不进行创建
}

// 用于创建更大或不同配置的共享内存段，首先销毁当前的共享内存段，然后根据新的消息大小更新共享内存配置，并尝试打开或创建新的共享内存段
bool Segment::Recreate(const uint64_t& msg_size) {
    init_ = false;
    state_->set_need_remap(true);
    Reset();                // 重置共享内存的状态，清空数据等
    Remove();               // 删除当前的共享内存段
    conf_.Update(msg_size); // 更新共享内存配置，根据新的消息大小重新计算共享内存的大小和块的数量
    return OpenOrCreate();  // 打开共享内存，如果不存在则创建
}

// 获取下一个可写的块的索引，主要是通过扫描块的状态来找到一个空闲的块，如果没有空闲的块则返回一个特殊值表示没有可用的块
uint32_t Segment::GetNextWritableBlockIndex() {
    const auto block_num = conf_.block_num();
    while (1) {
        // 返回seq，并将seq加一
        uint32_t try_idx = state_->FetchAddSeq(1) % block_num;
        // ADEBUG << "try_idx: " << try_idx;
        // 为blocks_[try_idx] 这块内存加上写锁
        if (blocks_[try_idx].TryLockForWrite()) {
            return try_idx;
        }
    }
    return 0;
}

} // namespace transport
} // namespace Middleware
} // namespace hnu