#include "../../common/log.h"
#include "readable_info.h"

#include <cstring>

namespace hnu {
namespace Middleware {
namespace transport {

const size_t ReadableInfo::ksize = sizeof(uint64_t) * 2 + sizeof(uint32_t);

ReadableInfo::ReadableInfo() : host_id_(0), block_index_(0), channel_id_(0) {}

ReadableInfo::ReadableInfo(uint64_t host_id, uint32_t block_index, uint64_t channel_id) : host_id_(host_id), block_index_(block_index), channel_id_(channel_id) {}

ReadableInfo::~ReadableInfo() {}

ReadableInfo& ReadableInfo::operator=(const ReadableInfo& other) {
    if (this != &other) {
        this->host_id_ = other.host_id_;
        this->channel_id_ = other.channel_id_;
        this->block_index_ = other.block_index_;
    }
    return *this;
}

// 把ReadableInfo对象序列化成字符串
// 共享内存只在同一台机器上使用，所以不需要考虑字节序问题，直接将成员变量的二进制数据写入字符串
bool ReadableInfo::SerializeTo(std::string* dst) const {
    RETURN_VAL_IF_NULL(dst, false);
    dst->assign(reinterpret_cast<char*>(const_cast<uint64_t*>(&host_id_)), sizeof(host_id_));
    dst->append(reinterpret_cast<char*>(const_cast<uint32_t*>(&block_index_)), sizeof(block_index_));
    dst->append(reinterpret_cast<char*>(const_cast<uint64_t*>(&channel_id_)), sizeof(channel_id_));

    return true;
}

// 反序列化
bool ReadableInfo::DeserializeFrom(const std::string& src) {
    return DeserializeFrom(src.data(), src.size());
}

bool ReadableInfo::DeserializeFrom(const char* src, std::size_t len) {
    RETURN_VAL_IF_NULL(src, false);
    if (len < ksize) {
        // std::cout << "src size[" << len << "] mismatch." << std::endl;
        AERROR << "src size[" << len << "] mismatch.";
        return false;
    }

    char* ptr = const_cast<char*>(src);
    memcpy(reinterpret_cast<char*>(&host_id_), ptr, sizeof(host_id_));
    ptr += sizeof(host_id_);
    memcpy(reinterpret_cast<char*>(&block_index_), ptr, sizeof(block_index_));
    ptr += sizeof(block_index_);
    memcpy(reinterpret_cast<char*>(&channel_id_), ptr, sizeof(channel_id_));

    return true;
}

} // namespace transport
} // namespace Middleware
} // namespace hnu