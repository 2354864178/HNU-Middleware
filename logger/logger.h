#pragma once

#include <string>
#include <fstream>

namespace hnu {
namespace Middleware {
namespace logger {

#define debug(format, ...) Logger::instance()->log(Logger::DEBUG, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define info(format, ...) Logger::instance()->log(Logger::INFO, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define warn(format, ...) Logger::instance()->log(Logger::WARN, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define error(format, ...) Logger::instance()->log(Logger::ERROR, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define fatal(format, ...) Logger::instance()->log(Logger::FATAL, __FILE__, __LINE__, format, ##__VA_ARGS__)

class Logger {
public:
    enum level { DEBUG = 0, INFO, WARN, ERROR, FATAL, LEVEL_COUNT };

    static Logger* instance();
    void open(const std::string& file_name);
    void close();
    void log(level level, const char* file, int line, const char* format, ...);

private:
    Logger();
    ~Logger();

private:
    std::string m_file_name;                       // 日志文件名
    std::ofstream m_fout;                          // 日志文件输出流
    static const char* level_strings[LEVEL_COUNT]; // 日志级别字符串表示
    static Logger* m_instance;                     // 单例实例
};

} // namespace logger
} // namespace Middleware
} // namespace hnu