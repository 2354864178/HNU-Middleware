#pragma once

#include <string>
#include <fstream>

#include "../common/macros.h"

namespace hnu {
namespace Middleware {
namespace logger {

#define debug(format, ...) Logger::Instance()->log(Logger::DEBUG, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define info(format, ...) Logger::Instance()->log(Logger::INFO, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define warn(format, ...) Logger::Instance()->log(Logger::WARN, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define error(format, ...) Logger::Instance()->log(Logger::ERROR, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define fatal(format, ...) Logger::Instance()->log(Logger::FATAL, __FILE__, __LINE__, format, ##__VA_ARGS__)

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
    void rotate();

private:
    ~Logger();

private:
    std::string m_file_name;                       // 日志文件名
    std::ofstream m_fout;                          // 日志文件输出流
    int m_level;                                   // 当前日志级别
    int m_max;                                     // 日志文件最大长度，单位为字节
    int m_len;                                     // 当前日志内容长度
    static const char* level_strings[LEVEL_COUNT]; // 日志级别字符串表示
    DECLARE_SINGLETON(Logger);                     // 声明单例类，禁止复制和赋值操作
};

} // namespace logger
} // namespace Middleware
} // namespace hnu