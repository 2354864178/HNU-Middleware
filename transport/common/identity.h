#pragma once

#include <cstdint>
#include <cstring>
#include <string>

namespace hnu {
namespace Middleware {
namespace transport {

constexpr uint8_t ID_SIZE = 8;
// 标识符类，包含一个8字节的标识符和一个哈希值
class Identity {
    public:
    explicit Identity(bool need_generate = true); // 构造函数，默认生成一个新的标识符
    Identity(const Identity& another);            // 拷贝构造函数
    virtual ~Identity();

    Identity& operator=(const Identity& another);   // 赋值运算符重载
    bool operator==(const Identity& another) const; // 相等运算符重载
    bool operator!=(const Identity& another) const; // 不相等运算符重载

    std::string ToString() const; // 将标识符转换为字符串表示
    size_t Length() const;        // 返回标识符的长度
    uint64_t HashValue() const;   // 返回标识符的哈希值
    const char* data() const {
        return data_;
    }

    // 设置标识符的值
    void set_data(const char* data) {
        if (data == nullptr) {
            return;
        }
        std::memcpy(data_, data, sizeof(data_));
        Update();
    }

    private:
    void Update();        // 更新标识符
    char data_[ID_SIZE];  // 8个字节
    uint64_t hash_value_; // 标识符的哈希值
};

} // namespace transport
} // namespace Middleware
} // namespace hnu
