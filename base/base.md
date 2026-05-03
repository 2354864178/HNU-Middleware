# `base` 模块说明

`base` 目录提供了 HNU Middleware 的通用基础组件，主要覆盖并发控制、队列、对象池、信号槽和模板工具等能力。这些组件通常是上层模块的底层依赖，设计目标是尽量减少重复实现，并提供相对直接的接口。

## 组件概览

### 并发与任务调度

- [`atomic_rw_lock.h`](atomic_rw_lock.h): 原子实现的读写锁，支持读锁共享、写锁独占，并可配置写优先策略。
- [`rw_lock_guard.h`](rw_lock_guard.h): 读锁/写锁的 RAII 守卫类，构造时加锁，析构时解锁。
- [`bounded_queue.h`](bounded_queue.h): 有界队列，基于循环数组实现，支持多生产者多消费者场景，并可挂接等待策略。
- [`unbounded_queue.h`](unbounded_queue.h): 无界队列，基于链表实现，使用原子操作维护头尾节点。
- [`wait_strategy.h`](wait_strategy.h): 等待策略接口与实现，包括阻塞、睡眠、让出时间片、忙等、超时阻塞等策略。
- [`thread_pool.h`](thread_pool.h): 线程池，内部使用有界任务队列和工作线程循环执行任务。

### 对象与事件

- [`object_pool.h`](object_pool.h): 对象池，预分配对象并通过 `shared_ptr` + 自定义释放器复用对象。
- [`signal.h`](signal.h): 信号槽机制，支持一个信号连接多个槽函数，并在线程安全条件下触发回调。

### 模板与工具

- [`for_each.h`](for_each.h): 提供 `FOR_EACH` 宏和类型特征辅助，用于写通用循环模板。
- [`atmoic_hash_map.h`](atmoic_hash_map.h): 原子哈希表实现，键要求为整数类型，表大小要求是 2 的幂次方。
- [`macros.h`](macros.h): 项目内常用宏定义集合，供其他基础组件复用。

## 设计特点

1. 组件以头文件为主，便于模板代码直接展开和内联。
2. 并发部分大量使用 `std::atomic`、CAS 和等待策略，而不是直接依赖互斥锁。
3. 多个组件都采用 RAII 或智能指针管理资源，减少手动释放的出错概率。
4. 基础工具层尽量保持通用，供上层模块组合使用。

## 典型使用方式

### 线程池

线程池适合提交短任务或异步任务，任务会被封装为 `std::function<void()>` 放入 `BoundedQueue`，由工作线程循环取出并执行。

### 读写锁

`AtomicRWLock` 与 `ReadLockGuard`、`WriteLockGuard` 配合使用时，可以让临界区自动加解锁，减少手写 `lock/unlock` 的错误。

### 对象池

`ObjectPool<T>` 适合高频创建和释放的小对象场景，可以提前分配内存并复用对象，减少动态分配压力。

### 信号槽

`Signal<Args...>` 可用于观察者模式：连接多个回调，在事件发生时统一通知所有槽函数。

## 备注

- `atmoic_hash_map.h` 文件名中 `atmoic` 可能是拼写错误，但这里保持与现有代码一致。
- `WaitStrategy` 的不同实现适合不同场景：阻塞适合省 CPU，忙等适合极低延迟场景，超时阻塞适合可控等待。

