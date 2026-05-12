#pragma once

#include <string>
#include <fstream>
#include <sstream>

#include "../common/macros.h"
#include "logstream.h"

namespace hnu {
namespace Middleware {
namespace logger {

#define debug(format, ...) Logger::Instance()->log(Logger::DEBUG, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define info(format, ...) Logger::Instance()->log(Logger::INFO, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define warn(format, ...) Logger::Instance()->log(Logger::WARN, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define error(format, ...) Logger::Instance()->log(Logger::ERROR, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define fatal(format, ...) Logger::Instance()->log(Logger::FATAL, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define LOG_STREAM(level) Logger::Instance()->logStream(Logger::level, __FILE__, __LINE__)
#define LOG_DEBUG LOG_STREAM(DEBUG)
#define LOG_INFO LOG_STREAM(INFO)
#define LOG_WARN LOG_STREAM(WARN)
#define LOG_ERROR LOG_STREAM(ERROR)
#define LOG_FATAL LOG_STREAM(FATAL)

class Logger {
public:
    enum level { DEBUG = 0, INFO, WARN, ERROR, FATAL, LEVEL_COUNT };

    void open(const std::string& file_name);
    void close();
    void log(level level, const char* file, int line, const char* format, ...);
    void max(int bytes) {
        m_max = bytes;
    }
    void set_level(int level) {
        m_level = level;
    }
    void console(bool enable) {
        m_console = enable;
    }
    template <typename T> Logger& operator<<(const T& msg) {
        m_stream << msg;
        return *this;
    }

    LogStream logStream(level level, const char* file, int line);

private:
    ~Logger();
    void rotate();

private:
    std::string m_file_name;                       // 日志文件名
    std::ofstream m_fout;                          // 日志文件输出流
    std::ostringstream m_stream;                   // 日志内容流
    int m_level;                                   // 当前日志级别
    int m_max;                                     // 日志文件最大长度，单位为字节
    int m_len;                                     // 当前日志内容长度
    bool m_console;                                // 是否同时输出到控制台
    static const char* level_strings[LEVEL_COUNT]; // 日志级别字符串表示
    DECLARE_SINGLETON(Logger);                     // 声明单例类，禁止复制和赋值操作
};

} // namespace logger
} // namespace Middleware
} // namespace hnu