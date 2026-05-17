#pragma once

#include "../logger/logger.h"
#include "../logger/logstream.h"

using namespace hnu::Middleware::logger;

#define Logger_Init(file_name) Logger::Instance()->open(file_name)
// Use the enum values defined in Logger::level
#define ADEBUG Logger::Instance()->logStream(Logger::DEBUG, __FILE__, __LINE__)
#define AINFO Logger::Instance()->logStream(Logger::INFO, __FILE__, __LINE__)
#define AWARN Logger::Instance()->logStream(Logger::WARN, __FILE__, __LINE__)
#define AERROR Logger::Instance()->logStream(Logger::ERROR, __FILE__, __LINE__)
#define AFATAL Logger::Instance()->logStream(Logger::FATAL, __FILE__, __LINE__)

#define log_debug(format, ...) Logger::Instance()->log(Logger::DEBUG, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define log_info(format, ...) Logger::Instance()->log(Logger::INFO, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define log_warn(format, ...) Logger::Instance()->log(Logger::WARN, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define log_error(format, ...) Logger::Instance()->log(Logger::ERROR, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define log_fatal(format, ...) Logger::Instance()->log(Logger::FATAL, __FILE__, __LINE__, format, ##__VA_ARGS__)

// 定义一些宏来简化日志记录的代码，提供了不同级别的日志记录功能，如调试、信息、警告、错误和致命错误等

// 检查指针是否为nullptr，如果是则记录警告日志并返回
#if !defined(RETURN_IF_NULL)
#define RETURN_IF_NULL(ptr)                                                                                                                                                                                                                                                                                                                                                                                    \
    if (ptr == nullptr) {                                                                                                                                                                                                                                                                                                                                                                                      \
        AWARN << #ptr << " is nullptr.";                                                                                                                                                                                                                                                                                                                                                                       \
        return;                                                                                                                                                                                                                                                                                                                                                                                                \
    }
#endif

// 检查指针是否为nullptr，如果是则记录警告日志并返回指定的值
#if !defined(RETURN_VAL_IF_NULL)
#define RETURN_VAL_IF_NULL(ptr, val)                                                                                                                                                                                                                                                                                                                                                                           \
    if (ptr == nullptr) {                                                                                                                                                                                                                                                                                                                                                                                      \
        AWARN << #ptr << " is nullptr.";                                                                                                                                                                                                                                                                                                                                                                       \
        return val;                                                                                                                                                                                                                                                                                                                                                                                            \
    }
#endif

// 检查条件是否满足，如果满足则记录警告日志并返回
#if !defined(RETURN_IF)
#define RETURN_IF(condition)                                                                                                                                                                                                                                                                                                                                                                                   \
    if (condition) {                                                                                                                                                                                                                                                                                                                                                                                           \
        AWARN << #condition << " is met.";                                                                                                                                                                                                                                                                                                                                                                     \
        return;                                                                                                                                                                                                                                                                                                                                                                                                \
    }
#endif

// 检查条件是否满足，如果满足则记录警告日志并返回指定的值
#if !defined(RETURN_VAL_IF)
#define RETURN_VAL_IF(condition, val)                                                                                                                                                                                                                                                                                                                                                                          \
    if (condition) {                                                                                                                                                                                                                                                                                                                                                                                           \
        AWARN << #condition << " is met.";                                                                                                                                                                                                                                                                                                                                                                     \
        return val;                                                                                                                                                                                                                                                                                                                                                                                            \
    }
#endif
