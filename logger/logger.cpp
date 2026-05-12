#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <stdexcept>
#include <iostream>

#include "logger.h"
#include "logstream.h"

using namespace std;

namespace hnu {
namespace Middleware {
namespace logger {

const char* Logger::level_strings[LEVEL_COUNT] = {"DEBUG", "INFO", "WARN", "ERROR", "FATAL"};

LogStream::LogStream(int level, const char* file, int line) : m_level(level), m_file(file), m_line(line) {}

LogStream::~LogStream() {
    Logger::Instance()->log(static_cast<Logger::level>(m_level), m_file, m_line, "%s", m_stream.str().c_str());
}

Logger::Logger() : m_level(DEBUG), m_max(0), m_len(0), m_console(true) {}

Logger::~Logger() {
    close();
}

void Logger::open(const std::string& file_name) {
    // 打开指定的日志文件
    m_file_name = file_name;               // 更新日志文件名
    m_fout.open(file_name, std::ios::app); // 以追加模式打开新的日志文件
    if (m_fout.fail()) {
        throw std::logic_error("Failed to open log file: " + file_name);
    }
    m_fout.seekp(0, std::ios::end); // 移动文件写指针到文件的末尾
    m_len = m_fout.tellp();         // 返回当前写指针在文件中的位置，即从文件开始到写指针当前位置的字节数
}

void Logger::close() {
    // 关闭日志文件输出流
    m_fout.close();
}

void Logger::log(level level, const char* file, int line, const char* format, ...) {
    if (level < m_level) {
        return; // 如果日志级别低于当前设置的级别，则不记录日志
    }

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
        m_fout << buffer;
        // std::cout << buffer << std::endl;
        m_len += size;
        delete[] buffer;
    }

    m_fout << ' ';

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
        m_fout << content;
        m_len += size;
        delete[] content;
    }
    m_fout << '\n';
    m_fout.flush();

    if (m_len >= m_max && m_max > 0) {
        std::cout << "Rotating log file..." << std::endl;
        rotate();
    }
    // std::cout << time_buffer << std::endl;
    // std::cout << file << std::endl;
    // std::cout << line << std::endl;
    // std::cout << format << std::endl;
}

void Logger::rotate() {
    close();
    time_t ticks = time(NULL);
    struct tm* ptm = localtime(&ticks);
    char timestamp[32];
    memset(timestamp, 0, sizeof(timestamp));
    strftime(timestamp, sizeof(timestamp), ".%Y-%m-%d_%H-%M-%S", ptm);
    string filename = m_file_name + timestamp;
    if (rename(m_file_name.c_str(), filename.c_str()) != 0) {
        throw std::logic_error("rename log file failed: " + string(strerror(errno)));
    }
    open(m_file_name);
}

LogStream Logger::logStream(level level, const char* file, int line) {
    return LogStream(level, file, line);
}

} // namespace logger
} // namespace Middleware
} // namespace hnu