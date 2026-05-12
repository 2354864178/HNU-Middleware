#pragma once

#include <sstream>

namespace hnu {
namespace Middleware {
namespace logger {

class Logger;

class LogStream {
public:
    LogStream(int level, const char* file, int line);
    ~LogStream();

    template <typename T> LogStream& operator<<(const T& msg) {
        m_stream << msg;
        return *this;
    }

    LogStream(const LogStream& other) : m_level(other.m_level), m_file(other.m_file), m_line(other.m_line), m_stream() {
        m_stream << other.m_stream.str();
    }

private:
    int m_level;
    const char* m_file;
    int m_line;
    std::ostringstream m_stream;
};

} // namespace logger
} // namespace Middleware
} // namespace hnu
