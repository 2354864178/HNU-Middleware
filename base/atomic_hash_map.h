#pragma once

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
    AtomicHashMap(const AtomicHashMap& other) = delete;            // 禁止复制构造
    AtomicHashMap& operator=(const AtomicHashMap& other) = delete; // 禁止复制赋值

    // 检查键是否存在
    bool HAS(K key) const {
        uint64_t index = key & mode_num_; // 计算哈希值
        return table_[index].Has(key);
    }

    // 获取键对应的值，如果键存在则返回 true，并将值通过指针参数返回
    bool Get(K key, V** value) {
        uint64_t index = key & mode_num_;
        return table_[index].Get(key, value);
    }

    // 获取值的重载版本，直接返回值而不是指针
    bool Get(K key, V* value) {
        uint64_t index = key & mode_num_;
        V* val = nullptr;
        bool res = table_[index].Get(key, &val);
        if (res) {
            *value = *val;
        }
        return res;
    }

    // 插入键，值默认为默认构造的 V 对象
    void Set(K key) {
        uint64_t index = key & mode_num_;
        table_[index].Insert(key);
    }

    // 插入键值对
    void Set(K key, const V& value) {
        uint64_t index = key & mode_num_;
        table_[index].Insert(key, value);
    }

    // 插入键值对，支持移动语义
    void Set(K key, V&& value) {
        uint64_t index = key & mode_num_;
        table_[index].Insert(key, std::forward<V>(value));
    }

    // 每个 Entry 代表哈希表中的一个节点，包含一个键和一个指向值的原子指针，以及一个指向下一个 Entry 的原子指针（用于链表）
    struct Entry {
        Entry() {}
        explicit Entry(K key) : key(key) {
            value_ptr.store(new V(), std::memory_order_release);
        }
        Entry(K key, const V& value) : key(key) {
            value_ptr.store(new V(value), std::memory_order_release);
        }
        Entry(K key, V&& value) : key(key) {
            value_ptr.store(new V(std::forward<V>(value)), std::memory_order_release);
        }
        ~Entry() {
            delete value_ptr.load(std::memory_order_acquire);
        }

        K key = 0;
        std::atomic<V*> value_ptr = {nullptr}; // 存储值的原子指针，初始为 nullptr
        std::atomic<Entry*> next = {nullptr};  // 指向链表中下一个 Entry 的原子指针，初始为 nullptr
    };

    // 桶类，包含一个指向链表头的指针，以及一些操作链表的方法
    class Bucket {
        public:
        Bucket() : head_(new Entry()) {} // 构造函数，初始化链表头为一个哨兵节点
        ~Bucket() {
            Entry* ite = head_;
            while (ite) {
                auto tmp = ite->next.load(std::memory_order_acquire); // 获取下一个节点
                delete ite;                                           // 删除当前节点
                ite = tmp;                                            // 移动到下一个节点
            }
        }

        // 检查链表中是否存在指定键
        bool Has(K key) {
            Entry* m_target = head_->next.load(std::memory_order_acquire);
            while (Entry* target = m_target) {
                if (target->key < key) {
                    m_target = target->next.load(std::memory_order_acquire);
                    continue;
                } else {
                    return target->key == key;
                }
            }
            return false;
        }

        // 查找指定键在链表中的位置，返回是否找到，并通过指针参数返回前一个节点和目标节点
        bool Find(K key, Entry** prev_ptr, Entry** target_ptr) {
            Entry* prev = head_;
            Entry* m_target = head_->next.load(std::memory_order_acquire);
            while (Entry* target = m_target) {
                if (target->key == key) {
                    *prev_ptr = prev;
                    *target_ptr = target;
                    return true;
                } else if (target->key > key) {
                    *prev_ptr = prev;
                    *target_ptr = target;
                    return false;
                } else {
                    prev = target;
                    m_target = target->next.load(std::memory_order_acquire);
                }
            }
            *prev_ptr = prev;
            *target_ptr = nullptr;
            return false;
        }

        // 插入键值对，如果键已存在则更新值，否则插入新节点
        void Insert(K key, const V& value) {
            Entry* prev = nullptr;
            Entry* target = nullptr;
            Entry* new_entry = nullptr;
            V* new_value = nullptr;
            while (true) {
                if (Find(key, &prev, &target)) {
                    if (!new_value) {
                        new_value = new V(value);
                    }
                    auto old_val_ptr = target->value_ptr.load(std::memory_order_acquire);
                    if (target->value_ptr.compare_exchange_strong(old_val_ptr, new_value, std::memory_order_acq_rel, std::memory_order_relaxed)) {
                        delete old_val_ptr;
                        if (new_entry) {
                            delete new_entry;
                            new_entry = nullptr;
                        }
                        return;
                    }
                    continue;
                } else {
                    if (!new_entry) {
                        new_entry = new Entry(key, value);
                    }
                    new_entry->next.store(target, std::memory_order_release);
                    if (prev->next.compare_exchange_strong(target, new_entry, std::memory_order_acq_rel, std::memory_order_relaxed)) {
                        if (new_value) {
                            delete new_value;
                            new_value = nullptr;
                        }
                        return;
                    }
                }
            }
        }

        void Insert(K key, V&& value) {
            Entry* prev = nullptr;
            Entry* target = nullptr;
            Entry* new_entry = nullptr;
            V* new_value = nullptr;
            while (true) {
                if (Find(key, &prev, &target)) {
                    if (!new_value) {
                        new_value = new V(std::forward<V>(value));
                    }
                    auto old_val_ptr = target->value_ptr.load(std::memory_order_acquire);
                    if (target->value_ptr.compare_exchange_strong(old_val_ptr, new_value, std::memory_order_acq_rel, std::memory_order_relaxed)) {
                        delete old_val_ptr;
                        if (new_entry) {
                            delete new_entry;
                            new_entry = nullptr;
                        }
                        return;
                    }
                    continue;
                } else {
                    if (!new_entry) {
                        new_entry = new Entry(key, value);
                    }
                    new_entry->next.store(target, std::memory_order_release);
                    if (prev->next.compare_exchange_strong(target, new_entry, std::memory_order_acq_rel, std::memory_order_relaxed)) {
                        if (new_value) {
                            delete new_value;
                            new_value = nullptr;
                        }
                        return;
                    }
                }
            }
        }

        void Insert(K key) {
            Entry* prev = nullptr;
            Entry* target = nullptr;
            Entry* new_entry = nullptr;
            V* new_value = nullptr;
            while (true) {
                if (Find(key, &prev, &target)) {
                    if (!new_value) {
                        new_value = new V();
                    }
                    auto old_val_ptr = target->value_ptr.load(std::memory_order_acquire);
                    if (target->value_ptr.compare_exchange_strong(old_val_ptr, new_value, std::memory_order_acq_rel, std::memory_order_relaxed)) {
                        delete old_val_ptr;
                        if (new_entry) {
                            delete new_entry;
                            new_entry = nullptr;
                        }
                        return;
                    }
                    continue;
                } else {
                    if (!new_entry) {
                        new_entry = new Entry(key);
                    }
                    new_entry->next.store(target, std::memory_order_release);
                    if (prev->next.compare_exchange_strong(target, new_entry, std::memory_order_acq_rel, std::memory_order_relaxed)) {
                        if (new_value) {
                            delete new_value;
                            new_value = nullptr;
                        }
                        return;
                    }
                }
            }
        }

        // 获取键对应的值，如果键存在则返回 true，并将值通过指针参数返回
        bool Get(K key, V** value) {
            Entry* prev = nullptr;
            Entry* target = nullptr;
            if (Find(key, &prev, &target)) {
                *value = target->value_ptr.load(std::memory_order_acquire);
                return true;
            }
            return false;
        }

        Entry* head_; // 桶的链表头，指向一个哨兵节点，哨兵节点不存储实际数据，只是为了简化链表操作
    };

    private:
    Bucket table_[TableSize]; // 哈希表数组，大小为 TableSize
    uint64_t capacity_;       // 哈希表的容量，等于 TableSize
    uint64_t mode_num_;       // 哈希表的掩码，用于计算哈希值，等于 TableSize - 1
};

} // namespace base
} // namespace Middleware
} // namespace hnu
