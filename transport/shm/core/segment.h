#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "block.h"
#include "shm_conf.h"
#include "state.h"

namespace hnu {
namespace Middleware {
namespace transport {

// Segment代表共享内存的一个段，包含State和Block等信息，用于管理共享内存的读写操作
class Segment;
using SegmentPtr = std::shared_ptr<Segment>; // Segment的智能指针类型,方便管理Segment对象的生命周期

// 可写的块内存结构体
struct WritableBlock {
    uint32_t index = 0;     // 块的索引，表示当前块在共享内存中的位置，可以用来区分不同的块
    Block* block = nullptr; // 指向共享内存中Block对象的指针，可以通过这个指针访问Block对象的成员变量和方法
    uint8_t* buf = nullptr; // 指向共享内存中块的buf的指针，可以通过这个指针访问块的buf区域，用于读写消息数据
};

using ReadableBlock = WritableBlock; // 可读的块内存结构体，与WritableBlock相同，可以复用

class Segment {

public:
    explicit Segment(uint64_t channel_id);
    virtual ~Segment() {}

    bool AcquireBlockToWrite(std::size_t msg_size, WritableBlock* writable_block);
    void ReleaseWrittenBlock(const WritableBlock& writable_block);

    bool AcquireBlockToRead(ReadableBlock* readable_block);
    void ReleaseReadBlock(const ReadableBlock& readable_block);

protected:
    // 虚函数，Segment的子类需要实现这些函数来具体操作共享内存，比如创建、打开、删除等
    virtual bool Destroy();          // 销毁共享内存，释放资源
    virtual void Reset() = 0;        // 重置共享内存的状态，清空数据等
    virtual bool Remove() = 0;       // 删除共享内存
    virtual bool OpenOnly() = 0;     // 打开已存在的共享内存，不进行创建
    virtual bool OpenOrCreate() = 0; // 打开共享内存，如果不存在则创建

    bool init_;           // 是否初始化成功
    ShmConf conf_;        // 共享内存的配置
    uint64_t channel_id_; // 共享内存对应的channel_id，可以用来区分不同的共享内存段

    State* state_;                                           // 共享内存管理区的状态对象指针
    Block* blocks_;                                          // 共享内存数据区的块数组
    void* managed_shm_;                                      // 管理的共享内存的起始地址
    std::mutex block_buf_lock_;                              // 保护block_buf_addrs_的互斥锁
    std::unordered_map<uint32_t, uint8_t*> block_buf_addrs_; // block索引到block buf地址的映射关系

private:
    bool Remap();                            // 重新映射共享内存，主要是为了应对消息大小超过上限的情况，此时需要重新创建一个更大的共享内存，并将原来共享内存中的数据迁移到新的共享内存中
    bool Recreate(const uint64_t& msg_size); // 重新创建共享内存，主要是为了应对消息大小超过上限的情况，此时需要重新创建一个更大的共享内存，并将原来共享内存中的数据迁移到新的共享内存中
    uint32_t GetNextWritableBlockIndex();    // 获取下一个可写的块的索引，主要是通过扫描块的状态来找到一个空闲的块，如果没有空闲的块则返回一个特殊值表示没有可用的块
    // 读者走的是“被动接收通知并按索引读取”的消费路径，所以没有专门的“获取下一个可读的块的索引”的方法，读者通过外部通知知道哪个块有数据可读，然后直接尝试读取该块的数据，如果读取失败（可能是因为写者还没有写完或者正在写），读者可以选择重试或者放弃读取
};

} // namespace transport
} // namespace Middleware
} // namespace hnu
