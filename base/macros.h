#pragma once

#include <cstdlib>
#include <new>

// UNUSED宏用于标记未使用的变量或参数，避免编译器发出警告
#ifndef CACHELINE_SIZE
#define CACHELINE_SIZE 64
#endif

#if __GNUC__ >= 3
#define cyber_likely(x) (__builtin_expect((x), 1))   // GCC提供的一个内置函数，用于告诉编译器某个条件表达式的结果更可能是true还是false，从而优化代码的执行路径。
#define cyber_unlikely(x) (__builtin_expect((x), 0)) // GCC提供的一个内置函数，用于告诉编译器某个条件表达式的结果更可能是true还是false，从而优化代码的执行路径。
#else
#define cyber_likely(x) (x)
#define cyber_unlikely(x) (x)
#endif

// // 创建一个名为name的类用于判断 T 中是否含有 func 函数
#define DEFINE_TYPE_TRAIT(name, func)                                                                                                                                                                                                                                                                                                                                                                          \
    template <typename T> struct name {                                                                                                                                                                                                                                                                                                                                                                        \
        template <typename Class> static constexpr bool Test(decltype(&Class::func)*) {                                                                                                                                                                                                                                                                                                                        \
            return true;                                                                                                                                                                                                                                                                                                                                                                                       \
        }                                                                                                                                                                                                                                                                                                                                                                                                      \
        template <typename> static constexpr bool Test(...) {                                                                                                                                                                                                                                                                                                                                                  \
            return false;                                                                                                                                                                                                                                                                                                                                                                                      \
        }                                                                                                                                                                                                                                                                                                                                                                                                      \
                                                                                                                                                                                                                                                                                                                                                                                                               \
        static constexpr bool value = Test<T>(nullptr);                                                                                                                                                                                                                                                                                                                                                        \
    };                                                                                                                                                                                                                                                                                                                                                                                                         \
                                                                                                                                                                                                                                                                                                                                                                                                               \
    template <typename T> constexpr bool name<T>::value;
