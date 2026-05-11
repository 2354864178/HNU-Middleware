#ifndef CMW_COMMON_MACROS_H_
#define CMW_COMMON_MACROS_H_

#include <iostream>
#include <memory>
#include <mutex>
#include <type_traits>
#include <utility>

#include "../base/macros.h"

DEFINE_TYPE_TRAIT(HasShutdown, Shutdown)

template <typename T> typename std::enable_if<HasShutdown<T>::value>::type CallShutdown(T* instance) {
    instance->Shutdown();
}

template <typename T> typename std::enable_if<!HasShutdown<T>::value>::type CallShutdown(T* instance) {
    (void)instance;
}

// 禁止复制和赋值操作的宏，接受一个类名参数，定义该类的复制构造函数和复制赋值运算符为 delete，禁止对该类进行复制和赋值操作
#define DISALLOW_COPY_AND_ASSIGN(classname)                                                                                                                                                                                                                                                                                                                                                                    \
    classname(const classname&) = delete;                                                                                                                                                                                                                                                                                                                                                                      \
    classname& operator=(const classname&) = delete;

// 定义一个单例类，使用 Meyers 单例模式实现，提供一个静态 Instance 方法用于获取单例实例，支持延迟创建和线程安全，提供一个 CleanUp 方法用于清理单例实例，禁止复制和赋值操作
#define DECLARE_SINGLETON(classname)                                                                                                                                                                                                                                                                                                                                                                           \
    public:                                                                                                                                                                                                                                                                                                                                                                                                    \
    static classname* Instance(bool create_if_needed = true) {                                                                                                                                                                                                                                                                                                                                                 \
        static classname* instance = nullptr;                                                                                                                                                                                                                                                                                                                                                                  \
        if (!instance && create_if_needed) {                                                                                                                                                                                                                                                                                                                                                                   \
            static std::once_flag flag;                                                                                                                                                                                                                                                                                                                                                                        \
            std::call_once(flag, [&] { instance = new (std::nothrow) classname(); });                                                                                                                                                                                                                                                                                                                          \
        }                                                                                                                                                                                                                                                                                                                                                                                                      \
        return instance;                                                                                                                                                                                                                                                                                                                                                                                       \
    }                                                                                                                                                                                                                                                                                                                                                                                                          \
                                                                                                                                                                                                                                                                                                                                                                                                               \
    static void CleanUp() {                                                                                                                                                                                                                                                                                                                                                                                    \
        auto instance = Instance(false);                                                                                                                                                                                                                                                                                                                                                                       \
        if (instance != nullptr) {                                                                                                                                                                                                                                                                                                                                                                             \
            CallShutdown(instance);                                                                                                                                                                                                                                                                                                                                                                            \
        }                                                                                                                                                                                                                                                                                                                                                                                                      \
    }                                                                                                                                                                                                                                                                                                                                                                                                          \
                                                                                                                                                                                                                                                                                                                                                                                                               \
    private:                                                                                                                                                                                                                                                                                                                                                                                                   \
    classname();                                                                                                                                                                                                                                                                                                                                                                                               \
    DISALLOW_COPY_AND_ASSIGN(classname)

#endif