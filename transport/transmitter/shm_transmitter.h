#pragma once

#include <cstring>
#include <iostream>
#include <memory>

#include "transmitter.h"
#include "transport/shm/notifier/notifier_factory.h"
#include "transport/shm/core/readable_info.h"
#include "transport/shm/segment/segment_factory.h"
#include "common/util.h"
#include "common/log.h"
#include "common/global_data.h"
#include "serialize/data_stream.h"

namespace hnu {
namespace Middleware {
namespace transport {

template <typename M> class ShmTransmitter : public Transmitter<M> {

public:
    using MessagePtr = std::shared_ptr<M>;

    explicit ShmTransmitter(const RoleAttributes& attr);
    virtual ~ShmTransmitter();

    void Enable() override;
    void Disable() override;

    bool Transmit(const MessagePtr& msg, const MessageInfo& info) override;

private:
    bool Transmit(const M& msg, const MessageInfo& msg_info);

    SegmentPtr segment_;   // 管理共享内存的Segment对象指针，用于操作共享内存的读写
    uint64_t channel_id_;  // 共享内存对应的channel_id，可以用来区分不同的共享内存段
    uint64_t host_id_;     // 发送者主机的ID，可以用来区分不同的发送者
    NotifierPtr notifier_; // 通知器对象指针，用于通知接收数据的进程处理数据，发送ReadableInfo
};

template <typename M> ShmTransmitter<M>::ShmTransmitter(const RoleAttributes& attr) : Transmitter<M>(attr), segment_(nullptr), channel_id_(attr.channel_id), notifier_(nullptr) {
    host_id_ = common::Hash(attr.host_ip);
}

template <typename M> ShmTransmitter<M>::~ShmTransmitter() {
    Disable();
}

template <typename M> void ShmTransmitter<M>::Enable() {
    if (this->enabled_) {
        return;
    }
    segment_ = SegmentFactory::CreateSegment(channel_id_); // 创建一个Segment对象来管理共享内存，传入channel_id_以区分不同的共享内存段
    notifier_ = NotifierFactory::CreateNotifier();         // 创建一个通知器对象
    this->enabled_ = true;
}

template <typename M> void ShmTransmitter<M>::Disable() {
    if (this->enabled_) {
        segment_ = nullptr;
        notifier_ = nullptr;
        this->enabled_ = false;
    }
}

template <typename M> bool ShmTransmitter<M>::Transmit(const MessagePtr& msg, const MessageInfo& msg_info) {
    return Transmit(*msg, msg_info);
}

// 发送消息的具体实现，主要是将消息序列化到共享内存中，并通过通知器通知接收数据的进程处理数据
template <typename M> bool ShmTransmitter<M>::Transmit(const M& msg, const MessageInfo& msg_info) {
    if (!this->enabled_) {
        ADEBUG << "not enable.";
        return false;
    }

    WritableBlock wb;         // 可写的块内存结构体，用于存储要写入共享内存的数据和相关信息
    serialize::DataStream ds; // 创建一个数据流对象，用于序列化消息数据
    ds << msg;                // 将消息对象序列化到数据流中

    std::size_t msg_size = ds.ByteSize(); // 获取序列化后的消息数据的大小，单位是字节

    // 拿到一块block去写，并对拿到的这块block加上写锁
    if (!segment_->AcquireBlockToWrite(msg_size, &wb)) {
        AERROR << "acquire block failed.";
        return false;
    }

    // ADEBUG << "block index: " << wb.index;
    // 拷贝序列化后的数据到wb.buf处
    std::memcpy(wb.buf, ds.data(), msg_size);

    wb.block->set_msg_size(msg_size);

    char* msg_info_addr = reinterpret_cast<char*>(wb.buf) + msg_size;

    std::memcpy(msg_info_addr, msg_info.sender_id().data(), ID_SIZE);               // 拷贝sender_id
    std::memcpy(msg_info_addr + ID_SIZE, msg_info.spare_id().data(), ID_SIZE);      // 拷贝spare_id_
    *reinterpret_cast<uint64_t*>(msg_info_addr + ID_SIZE * 2) = msg_info.seq_num(); // 拷贝 seq

    wb.block->set_msg_info_size(ID_SIZE * 2 + sizeof(uint64_t)); // 消息头的大小，单位是字节
    segment_->ReleaseWrittenBlock(wb);                           // 释放写锁后，读者才能读到这个块的数据

    ReadableInfo readable_info(host_id_, wb.index, channel_id_); // 新建一个ReadableInfo

    ADEBUG << "Writing sharedmem message: " << common::GlobalData::GetChannelById(channel_id_) << " to block: " << wb.index;
    // 通知接收数据的进程处理数据,发送ReadableInfo
    return notifier_->Notify(readable_info);
}

} // namespace transport
} // namespace Middleware
} // namespace hnu
