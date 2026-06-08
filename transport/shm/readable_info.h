#pragma once

#include <stdint.h>
#include <string>

namespace hnu {
namespace Middleware {
namespace transport {

class ReadableInfo {

public:
    ReadableInfo();
    ReadableInfo(uint64_t host_id, uint32_t block_index, uint64_t channel_id);
    virtual ~ReadableInfo();

    ReadableInfo& operator=(const ReadableInfo& other);

    bool DeserializeFrom(const std::string& src);
    bool DeserializeFrom(const char* src, std::size_t len);
    bool SerializeTo(std::string* dst) const;

    uint64_t host_id() const {
        return host_id_;
    }
    void set_host_id(uint64_t host_id) {
        host_id_ = host_id;
    }

    uint32_t block_index() const {
        return block_index_;
    }
    void set_block_index(uint32_t block_index) {
        block_index_ = block_index;
    }

    uint64_t channel_id() const {
        return channel_id_;
    }
    void set_channel_id(uint64_t channel_id) {
        channel_id_ = channel_id;
    }

    static const size_t ksize;

private:
    uint64_t host_id_;     // 主机ID，表示这个ReadableInfo对应哪个主机，这个字段可以用来区分不同的进程或者线程
    uint32_t block_index_; // 块索引，表示在共享内存中的位置
    uint64_t channel_id_;  // 通道ID，表示这个ReadableInfo对应哪个通信通道
};

} // namespace transport
} // namespace Middleware
} // namespace hnu
