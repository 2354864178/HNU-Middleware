#pragma once

#include <type_traits>
#include <iostream>

#include "macros.h"

namespace hnu {
namespace Middleware {
namespace base {

DEFINE_TYPE_TRAIT(HasLess, operator<) // 定义一个类型特征，用于检测一个类型是否支持 operator< 运算符

// 判断 val 是否小于 end，如果 Value 和 End 都支持 operator<，则使用 operator< 进行比较，否则使用 operator!= 进行比较
template <class Value, class End> typename std::enable_if<HasLess<Value>::value && HasLess<End>::value, bool>::type LessThan(const Value& val, const End& end) {
    return val < end;
}

// 如果 Value 或 End 中有一个不支持 operator<，则使用 operator!= 进行比较，判断 val 是否不等于 end
template <class Value, class End> typename std::enable_if<!HasLess<Value>::value || !HasLess<End>::value, bool>::type LessThan(const Value& val, const End& end) {
    return val != end;
}

#define FOR_EACH(i, begin, end) for (auto i = (true ? (begin) : (end)); apollo::cyber::base::LessThan(i, (end)); ++i)

} // namespace base
} // namespace Middleware
} // namespace hnu
