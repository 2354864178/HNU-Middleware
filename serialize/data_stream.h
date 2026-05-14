#pragma once
#include <cstdint>
#include <vector>
#include <iostream>
#include <string>
#include <type_traits>
#include <stdexcept>
#include <cstring>
#include <list>
#include <map>
#include <set>
#include <algorithm>
using namespace std;

namespace hnu {
namespace Middleware {
namespace serialize {

class Serializable;

// 数据流类，提供序列化和反序列化功能，支持基本数据类型和常用容器类型
class DataStream {
public:
    enum DataType { BOOL = 0, CHAR, INT32, INT64, UINT32, UINT64, FLOAT, DOUBLE, ENUM, STRING, VECTOR, LIST, MAP, SET, CUSTOM };

    enum ByteOrder {
        BigEndian,   // 大端存储：高位字节在前，低位字节在后
        LittleEndian // 小端存储：低位字节在前，高位字节在后
    };

    DataStream();
    DataStream(char* addr, size_t len); // 从已有数据构造 DataStream 对象，便于反序列化

    ~DataStream() {}
    void show() const; // 显示当前数据流内容，主要用于调试

    void write(const char* data, int len); // 写入原始数据到数据流中，底层接口
    void write(bool value);                // 写入布尔值到数据流中
    void write(char value);                // 写入字符到数据流中
    void write(int32_t value);             // 写入32位整数到数据流中
    void write(uint32_t value);            // 写入32位无符号整数到数据流中
    void write(int64_t value);             // 写入64位整数到数据流中
    void write(uint64_t value);            // 写入64位无符号整数到数据流中
    void write(float value);
    void write(double value);
    void write(const char* value);
    void write(const std::string& value); // 写入字符串到数据流中，先写入长度再写入内容
    void write(const Serializable& value);

    // Enum write/read helpers
    template <typename E, typename = typename std::enable_if<std::is_enum<E>::value>::type> void write(E value) {
        using Under = typename std::underlying_type<E>::type;
        write(static_cast<Under>(value));
    }

    template <typename T> void write(const std::vector<T>& value); // 写入 vector 容器到数据流中，先写入长度再写入每个元素

    template <typename T> void write(const std::list<T>& value);

    template <typename K, typename V> void write(const std::map<K, V>& value);

    template <typename T> void write(const std::set<T>& value);

    template <typename T, typename... Args> void write_args(const T& head, const Args&... args);

    void write_args();

    DataStream& operator<<(bool value);
    DataStream& operator<<(char value);
    DataStream& operator<<(int32_t value);
    DataStream& operator<<(int64_t value);
    DataStream& operator<<(float value);
    DataStream& operator<<(double value);
    DataStream& operator<<(const std::string& value);
    DataStream& operator<<(const char* value);
    DataStream& operator<<(const Serializable& value);

    template <typename T> DataStream& operator<<(const std::vector<T>& value);

    template <typename T> DataStream& operator<<(const std::list<T>& value);

    template <typename K, typename V> DataStream& operator<<(const std::map<K, V>& value);

    template <typename T> DataStream& operator<<(const std::set<T>& value);

    bool read(char* data, int len);
    bool read(bool& value);
    bool read(char& value);
    bool read(int32_t& value);
    bool read(int64_t& value);
    bool read(uint32_t& value);
    bool read(uint64_t& value);
    bool read(float& value);
    bool read(double& value);
    bool read(std::string& value);
    bool read(Serializable& value);

    template <typename E, typename = typename std::enable_if<std::is_enum<E>::value>::type> bool read(E& value) {
        using Under = typename std::underlying_type<E>::type;
        Under tmp;
        if (!read(tmp))
            return false;
        value = static_cast<E>(tmp);
        return true;
    }

    template <typename T> bool read(std::vector<T>& value);

    template <typename T> bool read(std::list<T>& value);

    template <typename K, typename V> bool read(std::map<K, V>& value);

    template <typename T> bool read(std::set<T>& value);

    template <typename T, typename... Args> bool read_args(T& head, Args&... args);

    bool read_args();

    DataStream& operator>>(bool& value);
    DataStream& operator>>(char& value);
    DataStream& operator>>(int32_t& value);
    DataStream& operator>>(int64_t& value);
    DataStream& operator>>(float& value);
    DataStream& operator>>(double& value);
    DataStream& operator>>(string& value);

    template <typename T> DataStream& operator>>(std::vector<T>& value);

    template <typename T> DataStream& operator>>(std::list<T>& value);

    template <typename K, typename V> DataStream& operator>>(std::map<K, V>& value);

    template <typename T> DataStream& operator>>(std::set<T>& value);

public:
    const char* data() const;
    size_t size() const;
    void clear();
    void reset();

private:
    void reserve(int len);
    ByteOrder byteorder();

private:
    ByteOrder m_byteorder;
    std::vector<char> m_buf;
    int m_pos;
};

template <typename T> void DataStream::write(const std::vector<T>& value) {
    char type = DataType::VECTOR;
    write((char*)&type, sizeof(char));
    int len = value.size();
    write(len);
    for (int i = 0; i < len; i++) {
        write(value[i]);
    }
}

template <typename T> void DataStream::write(const std::list<T>& value) {
    char type = DataType::LIST;
    write((char*)&type, sizeof(char));
    int len = value.size();
    write(len);
    for (auto it = value.begin(); it != value.end(); it++) {
        write((*it));
    }
}

template <typename K, typename V> void DataStream::write(const std::map<K, V>& value) {
    char type = DataType::MAP;
    write((char*)&type, sizeof(char));
    int len = value.size();
    write(len);
    for (auto it = value.begin(); it != value.end(); it++) {
        write(it->first);
        write(it->second);
    }
}

template <typename T> void DataStream::write(const std::set<T>& value) {
    char type = DataType::SET;
    write((char*)&type, sizeof(char));
    int len = value.size();
    write(len);
    for (auto it = value.begin(); it != value.end(); it++) {
        write(*it);
    }
}

template <typename T, typename... Args> void DataStream::write_args(const T& head, const Args&... args) {
    write(head);
    write_args(args...);
}

template <typename T> DataStream& DataStream::operator<<(const std::vector<T>& value) {
    write(value);
    return *this;
}

template <typename K, typename V> DataStream& DataStream::operator<<(const std::map<K, V>& value) {
    write(value);
    return *this;
}

template <typename T> DataStream& DataStream::operator<<(const std::set<T>& value) {
    write(value);
    return *this;
}

template <typename T> bool DataStream::read(std::vector<T>& value) {
    value.clear();
    if (m_buf[m_pos] != DataType::VECTOR) {
        return false;
    }
    ++m_pos;
    int len;
    read(len);
    for (int i = 0; i < len; i++) {
        T v;
        read(v);
        value.push_back(v);
    }
    return true;
}

template <typename T> bool DataStream::read(std::list<T>& value) {
    value.clear();
    if (m_buf[m_pos] != DataType::LIST) {
        return false;
    }
    ++m_pos;
    int len;
    read(len);
    for (int i = 0; i < len; i++) {
        T v;
        read(v);
        value.push_back(v);
    }
    return true;
}

template <typename K, typename V> bool DataStream::read(std::map<K, V>& value) {
    value.clear();
    if (m_buf[m_pos] != DataType::MAP) {
        return false;
    }
    ++m_pos;
    int len;
    read(len);
    for (int i = 0; i < len; i++) {
        K k;
        read(k);

        V v;
        read(v);
        value[k] = v;
    }
    return true;
}

template <typename T> bool DataStream::read(std::set<T>& value) {
    value.clear();
    if (m_buf[m_pos] != DataType::SET) {
        return false;
    }
    ++m_pos;
    int len;
    read(len);
    for (int i = 0; i < len; i++) {
        T v;
        read(v);
        value.insert(v);
    }
    return true;
}

template <typename T, typename... Args> bool DataStream::read_args(T& head, Args&... args) {
    read(head);
    return read_args(args...);
}

template <typename T> DataStream& DataStream::operator>>(std::vector<T>& value) {
    read(value);
    return *this;
}

template <typename T> DataStream& DataStream::operator>>(std::list<T>& value) {
    read(value);
    return *this;
}

template <typename K, typename V> DataStream& DataStream::operator>>(std::map<K, V>& value) {
    read(value);
    return *this;
}

template <typename T> DataStream& DataStream::operator>>(std::set<T>& value) {
    read(value);
    return *this;
}

} // namespace serialize
} // namespace Middleware
} // namespace hnu