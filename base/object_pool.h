#pragma once

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <new>
#include <utility>

#include "for_each.h"
#include "macros.h"

namespace hnu {
namespace Middleware {
namespace base {

template <typename T> class ObjectPool : public std::enable_shared_from_this<ObjectPool<T>> {
    public:
    using InitFunc = std::function<void(T*)>;
    using ObjectPoolPtr = std::shared_ptr<ObjectPool<T>>;

    // 构造函数，接受对象数量和可变参数列表，用于构造对象池中的对象
    template <typename... Args> explicit ObjectPool(uint32_t num_objects, Args&&... args);

    template <typename... Args> ObjectPool(uint32_t num_objects, InitFunc f, Args&&... args);

    virtual ~ObjectPool(); // 析构函数，释放对象池占用的内存资源

    // 获取对象池中的一个对象，返回一个 shared_ptr，如果对象池已空则返回 nullptr
    std::shared_ptr<T> GetObject();

    private:
    // 定义一个 Node 结构体，包含一个对象和一个指向下一个节点的指针，用于实现对象池的链表结构
    struct Node {
        T object;
        Node* next;
    };
    /*禁用拷贝构造*/
    ObjectPool(ObjectPool&) = delete;
    ObjectPool& operator=(ObjectPool&) = delete;
    void ReleaseObject(T*); // 释放对象，将对象返回到对象池中，接受一个 T* 参数，表示要释放的对象指针

    uint32_t num_objects_ = 0;     // 对象数量
    char* object_arena_ = nullptr; // 对象池占用的内存区域，使用 char* 类型表示，可以存储任意类型的对象
    Node* free_head_ = nullptr;    // 指向对象池中第一个可用对象的指针，使用 Node* 类型表示，指向链表的头部
};

// 构造函数实现，使用可变参数列表构造对象池中的对象，并将它们链接成一个链表，链表的头部指向第一个可用对象
template <typename T> template <typename... Args> ObjectPool<T>::ObjectPool(uint32_t num_objects, Args&&... args) : num_objects_(num_objects) {
    const size_t size = sizeof(Node);
    object_arena_ = static_cast<char*>(std::calloc(num_objects_, size)); // 使用 calloc 分配内存，分配 num_objects_ 个 Node 大小的内存块，并将其转换为 char* 类型
    if (object_arena_ == nullptr) {
        throw std::bad_alloc();
    }

    // 使用 FOR_EACH 宏遍历对象池中的每个对象，使用 placement new 在 object_arena_ 中构造对象，并将它们链接成一个链表，链表的头部指向第一个可用对象
    FOR_EACH(i, 0, num_objects_) {
        T* obj = new (object_arena_ + i * size) T(std::forward<Args>(args)...);
        reinterpret_cast<Node*>(obj)->next = free_head_; // 将当前对象的 next 指针指向链表的头部，即上一个对象
        free_head_ = reinterpret_cast<Node*>(obj);
    }
}

// 构造函数实现，接受一个 InitFunc 类型的函数对象，用于初始化对象池中的对象，其他部分与上一个构造函数相同
template <typename T> template <typename... Args> ObjectPool<T>::ObjectPool(uint32_t num_objects, InitFunc f, Args&&... args) : num_objects_(num_objects) {
    const size_t size = sizeof(Node);
    object_arena_ = static_cast<char*>(std::calloc(num_objects_, size));
    if (object_arena_ == nullptr) {
        throw std::bad_alloc();
    }

    FOR_EACH(i, 0, num_objects_) {
        T* obj = new (object_arena_ + i * size) T(std::forward<Args>(args)...);
        f(obj); // 调用函数对象 f 对当前对象进行初始化
        reinterpret_cast<Node*>(obj)->next = free_head_;
        free_head_ = reinterpret_cast<Node*>(obj);
    }
}

// 析构函数实现，释放对象池占用的内存资源，首先调用每个对象的析构函数，然后使用 free 释放 object_arena_ 占用的内存资源
template <typename T> ObjectPool<T>::~ObjectPool() {
    if (object_arena_ != nullptr) {
        const size_t size = sizeof(Node);
        FOR_EACH(i, 0, num_objects_) {
            reinterpret_cast<Node*>(object_arena_ + i * size)->object.~T();
        }
        std::free(object_arena_);
    }
}

// 释放对象实现，将对象返回到对象池中，首先将对象转换为 Node* 类型，然后将它的 next 指针指向链表的头部，即当前的 free_head_，最后将 free_head_ 更新为当前对象的指针
template <typename T> void ObjectPool<T>::ReleaseObject(T* object) {
    if (cyber_unlikely(object == nullptr)) {
        return;
    }

    reinterpret_cast<Node*>(object)->next = free_head_;
    free_head_ = reinterpret_cast<Node*>(object);
}

// 获取对象实现，从对象池中获取一个对象，首先检查 free_head_ 是否为 nullptr，如果是则表示对象池已空，返回 nullptr；否则创建一个 shared_ptr 对象，指向 free_head_ 所指向的对象，并设置一个自定义的删除器，当 shared_ptr 被销毁时调用 ReleaseObject 将对象返回到对象池中；最后将 free_head_ 更新为下一个可用对象的指针，并返回 shared_ptr 对象
template <typename T> std::shared_ptr<T> ObjectPool<T>::GetObject() {
    if (free_head_ == nullptr) {
        return nullptr;
    }

    auto self = this->shared_from_this();
    auto obj = std::shared_ptr<T>(reinterpret_cast<T*>(free_head_), [self](T* object) { self->ReleaseObject(object); });
    free_head_ = free_head_->next;
    return obj;
}

} // namespace base
} // namespace Middleware
} // namespace hnu
