#ifndef CMW_BASE_ATOMIC_HASH_MAP_H_
#define CMW_BASE_ATOMIC_HASH_MAP_H_

#include <atomic>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace hnu {
namespace Middleware {
namespace base {

// // SFINAE 技术，启用该模板仅当 K 是整数类型且 TableSize 是 2 的幂次方时
template <typename K, typename V, std::size_t TableSize = 128,
          typename std::enable_if<std::is_integral<K>::value &&         // 只允许整数类型作为键
                                      (TableSize&(TableSize - 1)) == 0, // 哈希表大小必须是 2 的幂次方
                                  int>::type = 0>
class AtomicHashMap {
    public:
    AtomicHashMap() : capacity_(TableSize), mode_num_(capacity_ - 1) {}
    AtomicHashMap(const AtomicHashMap& other) = delete;            // 禁止复制构造和赋值操作
    AtomicHashMap& operator=(const AtomicHashMap& other) = delete; // 禁止复制构造和赋值操作
};

} // namespace base
} // namespace Middleware
} // namespace hnu

#endif