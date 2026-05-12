#pragma once

#include <unistd.h>
#include <atomic>
#include <cstdint>
#include <memory>

namespace hnu {
namespace Middleware {
namespace base {

// 无界队列，支持多生产者多消费者，使用链表实现
template <typename T> class UnboundedQueue {
    public:
    UnboundedQueue() {
        Reset();
    }
    UnboundedQueue& operator=(const UnboundedQueue& other) = delete; // 禁止复制赋值
    UnboundedQueue(const UnboundedQueue& other) = delete;            // 禁止复制构造

    ~UnboundedQueue() {
        Destroy();
    }

    void Clear() {
        Destroy();
        Reset();
    }

    // 入队操作，向队列尾部添加一个元素，线程安全
    void Enqueue(const T& element) {
        auto node = new Node();
        node->data = element;
        Node* old_tail = tail_.load();

        while (true) {
            if (tail_.compare_exchange_strong(old_tail, node)) {
                old_tail->next = node; // 将旧尾节点的 next 指针指向新节点，完成链表的连接
                old_tail->release();   // 释放旧尾节点的引用，表示不再引用旧尾节点
                size_.fetch_add(1);    // 增加队列大小，表示有一个新元素被添加到队列中
                break;
            }
        }
    }

    // 出队操作，从队列头部移除一个元素并返回，线程安全，如果队列为空则返回 false
    bool Dequeue(T* element) {
        Node* old_head = head_.load();
        Node* head_next = nullptr;
        do {
            head_next = old_head->next; // 获取当前头节点的下一个节点，准备将其作为新的头节点
            if (head_next == nullptr) { // 如果下一个节点为 nullptr，说明队列为空，出队失败
                return false;
            }
        } while (!head_.compare_exchange_strong(old_head, head_next));

        *element = head_next->data; // 将新头节点的数据赋值给输出参数，表示出队成功
        size_.fetch_sub(1);         // 减少队列大小，表示有一个元素从队列中移除
        old_head->release();
        return true;
    }

    size_t Size() {
        return size_.load();
    }

    bool Empty() {
        return size_.load() == 0;
    }

    private:
    // 队列的节点结构，包含数据、引用计数和指向下一个节点的指针
    struct Node {
        T data;
        std::atomic<uint32_t> ref_count; // 引用计数，初始值为 2，表示当前节点被 head_ 和 tail_ 两个指针引用，当引用计数减为 0 时说明没有任何指针引用该节点，可以安全地删除它
        Node* next = nullptr;
        Node() {
            ref_count.store(2); // 初始化引用计数为 2，表示当前节点被 head_ 和 tail_ 两个指针引用
        }

        // 释放节点，减少引用计数，如果引用计数减为 0 则删除节点
        void release() {
            ref_count.fetch_sub(1); // 减少引用计数，表示有一个指针不再引用该节点
            if (ref_count.load() == 0) {
                delete this;
            }
        }
    };

    // 重置队列，初始化头节点和尾节点，并将大小设置为 0
    void Reset() {
        auto node = new Node(); // 创建一个新的节点作为队列的头节点，初始时头节点和尾节点都指向这个新节点
        head_.store(node);      // 将头节点的指针存储在 head_ 原子变量中，确保线程安全
        tail_.store(node);      // 将尾节点的指针存储在 tail_ 原子变量中，确保线程安全
        size_.store(0);         // 将队列的大小初始化为 0，存储在 size_ 原子变量中，确保线程安全
    }

    // 销毁队列，释放所有节点的内存
    void Destroy() {
        auto ite = head_.load(); // 从头节点开始遍历队列，获取当前头节点的指针
        Node* tmp = nullptr;
        while (ite != nullptr) {
            tmp = ite->next;
            delete ite;
            ite = tmp;
        }
    }

    std::atomic<Node*> head_;  // 队列的头指针，指向队列的第一个节点，使用原子变量确保线程安全
    std::atomic<Node*> tail_;  // 队列的尾指针，指向队列的最后一个节点，使用原子变量确保线程安全
    std::atomic<size_t> size_; // 队列的大小，表示队列中元素的数量，使用原子变量确保线程安全
};

} // namespace base
} // namespace Middleware
} // namespace hnu