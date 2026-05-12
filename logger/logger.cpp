#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <stdexcept>
#include <iostream>

#include "logger.h"

using namespace std;

namespace hnu {
namespace Middleware {
namespace logger {

const char* Logger::level_strings[LEVEL_COUNT] = {"DEBUG", "INFO", "WARN", "ERROR", "FATAL"};

Logger* Logger::m_instance = nullptr; // 初始化单例实例指针

Logger::Logger() {}

Logger::~Logger() {
    // 析构函数，关闭日志文件输出流
    if (m_fout.is_open()) {
        m_fout.close();
    }
}

Logger* Logger::instance() {
    // 获取单例实例
    if (m_instance == nullptr) {
        m_instance = new Logger();
    }
    return m_instance;
}

void Logger::open(const std::string& file_name) {
    // 打开指定的日志文件
    m_file_name = file_name;               // 更新日志文件名
    m_fout.open(file_name, std::ios::app); // 以追加模式打开新的日志文件
    if (m_fout.fail()) {
        throw std::logic_error("Failed to open log file: " + file_name);
    }
}

void Logger::close() {
    // 关闭日志文件输出流
    m_fout.close();
}

void Logger::log(level level, const char* file, int line, const char* format, ...) {
    if (m_fout.fail()) {
        throw std::logic_error("Log file is not open.");
    }
    time_t ticks = time(nullptr);
    struct tm* tm_info = localtime(&ticks);
    char time_buffer[32];
    memset(time_buffer, 0, sizeof(time_buffer));
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", tm_info);

    const char* ftm = "%s %s %s:%d";
    int size = snprintf(nullptr, 0, ftm, time_buffer, level_strings[level], file, line);
    if (size > 0) {
        char* buffer = new char[size + 1];
        memset(buffer, 0, size + 1);
        snprintf(buffer, size + 1, ftm, time_buffer, level_strings[level], file, line);
        buffer[size] = '\0';
        m_fout << buffer << ' ';
        // std::cout << buffer << std::endl;
        delete[] buffer;
    }

    va_list arg_ptr;
    va_start(arg_ptr, format);
    size = vsnprintf(nullptr, 0, format, arg_ptr);
    va_end(arg_ptr);
    if (size > 0) {
        char* content = new char[size + 1];
        va_start(arg_ptr, format);
        vsnprintf(content, size + 1, format, arg_ptr);
        va_end(arg_ptr);
        // std::cout << content << std::endl;
        content[size] = '\0';
        m_fout << content << std::endl;
        delete[] content;
    }

    m_fout.flush();

    // std::cout << time_buffer << std::endl;
    // std::cout << file << std::endl;
    // std::cout << line << std::endl;
    // std::cout << format << std::endl;
}

} // namespace logger
} // namespace Middleware
} // namespace hnu