#include "data_stream.h"

namespace hnu {
namespace Middleware {
namespace serialize {
// 构造函数：默认构造一个空的数据流对象，初始化位置指针和字节序
DataStream::DataStream() : m_pos(0) {
    m_byteorder = byteorder();
}

DataStream::DataStream(char* addr, size_t len) : m_pos(0) {
    m_byteorder = byteorder();      // 判断当前机器的字节序，设置 m_byteorder 成员变量
    m_buf.assign(addr, addr + len); // 将输入的 char* 数据复制到 m_buf 向量中，构造一个 DataStream 对象用于反序列化
}

// 返回 m_buf 向量内部数据的指针，供外部访问序列化后的数据
const char* DataStream::data() const {
    return m_buf.data();
}

// 返回 m_buf 向量的大小，即序列化后的数据量
size_t DataStream::size() const {
    return m_buf.size();
}

// 清空 m_buf 向量，重置数据流对象到初始状态
void DataStream::clear() {
    m_buf.clear();
}

// 重置位置指针 m_pos 到 0，准备从头开始读取数据
void DataStream::reset() {
    m_pos = 0;
}

/*判断机器是大端存储还是小端存储*/
DataStream::ByteOrder DataStream::byteorder() {
    int n = 0x12345678;
    char str[4];
    memcpy(str, &n, sizeof(int));
    if (str[0] == 0x12) {
        return ByteOrder::BigEndian;
    } else {
        return ByteOrder::LittleEndian;
    }
}

// 显示当前数据流内容，主要用于调试，按照 DataType 枚举定义的格式解析 m_buf 中的数据并打印出来
void DataStream::show() const {
    int size = m_buf.size();
    std::cout << "data size = " << size << std::endl;
    int i = 0;
    while (i < size) {
        switch ((DataType)m_buf[i]) {
        case DataType::BOOL:
            if ((int)m_buf[++i] == 0) {
                std::cout << "false";
            } else {
                std::cout << "true";
            }
            ++i;
            break;
        case DataType::CHAR:
            std::cout << m_buf[++i];
            ++i;
            break;
        case DataType::INT32:
            std::cout << *((int32_t*)(&m_buf[++i]));
            i += 4;
            break;
        case DataType::INT64:
            std::cout << *((int64_t*)(&m_buf[++i]));
            i += 8;
            break;
        case DataType::DOUBLE:
            std::cout << *((double*)(&m_buf[++i]));
            i += 8;
            break;
        case DataType::FLOAT:
            std::cout << *((float*)(&m_buf[++i]));
            i += 4;
            break;
        case DataType::STRING:
            if ((DataType)m_buf[++i] == DataType::INT32) {
                int len = *((int*)(&m_buf[++i]));
                i += 4;
                std::cout << std::string(&m_buf[i], len);
                i += len;
            } else {
                throw std::logic_error("parse string error");
            }
            break;

        default:
            break;
        }
    }
}
void DataStream::reserve(int len) {
    int size = m_buf.size();    // 当前数据流的大小
    int cap = m_buf.capacity(); // 当前数据流的容量，即在不重新分配内存的情况下可以容纳的最大数据量

    if (size + len > cap) {
        while (size + len > cap) {
            if (cap == 0) {
                cap = 1;
            } else {
                cap *= 2; // 如果当前容量不足以容纳新的数据，则将容量翻倍，直到足够大为止。这是一种常见的动态数组扩容策略，可以在平均情况下提供较好的性能。
            }
        }
        m_buf.reserve(cap); // std::vector 的 reserve 方法，增加 m_buf 的容量到新的 cap 值，以确保有足够的空间来写入新的数据
    }
}

void DataStream::write(const char* data, int len) {
    reserve(len);                         // 确保 m_buf 有足够的容量来写入新的数据
    int size = m_buf.size();              // 获取当前数据流的大小，即已经写入的数据量
    m_buf.resize(size + len);             // 将 m_buf 的大小增加 len，以便为新的数据腾出空间
    std::memcpy(&m_buf[size], data, len); // 将新的数据从 data 指针指向的内存位置复制到 m_buf 中，从 size 位置开始，长度为 len
}

void DataStream::write(bool value) {
    char type = DataType::BOOL;
    write((char*)&type, sizeof(char));  // 写入数据类型标识符，告诉读取方接下来是什么类型的数据
    write((char*)&value, sizeof(char)); // 将布尔值转换为 char 类型（0 或 1）并写入数据流中，占用一个字节
}

void DataStream::write(char value) {
    char type = DataType::CHAR;
    write((char*)&type, sizeof(char));
    write((char*)&value, sizeof(char));
}

void DataStream::write(int32_t value) {
    char type = DataType::INT32;
    write((char*)&type, sizeof(char));
    if (m_byteorder == ByteOrder::BigEndian) {
        char* first = (char*)&value;
        char* last = first + sizeof(int32_t);
        std::reverse(first, last); // 如果当前机器是大端存储，则需要将整数的字节顺序反转，以确保在不同字节序的机器之间进行正确的序列化和反序列化
    }
    write((char*)&value, sizeof(int32_t));
}

void DataStream::write(int64_t value) {
    char type = DataType::INT64;
    write((char*)&type, sizeof(char));
    if (m_byteorder == ByteOrder::BigEndian) {
        char* first = (char*)&value;
        char* last = first + sizeof(int64_t);
        std::reverse(first, last);
    }
    write((char*)&value, sizeof(int64_t));
}

void DataStream::write(float value) {
    char type = DataType::FLOAT;
    write((char*)&type, sizeof(char));
    if (m_byteorder == ByteOrder::BigEndian) {
        char* first = (char*)&value;
        char* last = first + sizeof(float);
        std::reverse(first, last);
    }
    write((char*)&value, sizeof(float));
}

void DataStream::write(double value) {
    char type = DataType::DOUBLE;
    write((char*)&type, sizeof(char));
    if (m_byteorder == ByteOrder::BigEndian) {
        char* first = (char*)&value;
        char* last = first + sizeof(double);
        std::reverse(first, last);
    }
    write((char*)&value, sizeof(double));
}

void DataStream::write(const char* value) {
    char type = DataType::STRING;
    write((char*)&type, sizeof(char));
    int len = strlen(value);
    write(len);
    write(value, len);
}

void DataStream::write(const std::string& value) {
    char type = DataType::STRING;
    write((char*)&type, sizeof(char));
    int len = value.size();
    write(len);
    write(value.data(), len);
}

bool DataStream::read(bool& value) {
    if (m_buf[m_pos] != DataType::BOOL) {
        return false;
    }
    ++m_pos;
    value = m_buf[m_pos];
    ++m_pos;
    return true;
}

bool DataStream::read(char& value) {
    if (m_buf[m_pos] != DataType::CHAR) {
        return false;
    }
    ++m_pos;
    value = m_buf[m_pos];
    ++m_pos;
    return true;
}

bool DataStream::read(int32_t& value) {
    if (m_buf[m_pos] != DataType::INT32) {
        return false;
    }
    ++m_pos;
    value = *((int32_t*)(&m_buf[m_pos]));
    if (m_byteorder == ByteOrder::BigEndian) {
        char* first = (char*)&value;
        char* last = first + sizeof(int32_t);
        std::reverse(first, last);
    }
    m_pos += 4;
    return true;
}

bool DataStream::read(int64_t& value) {
    if (m_buf[m_pos] != DataType::INT64) {
        return false;
    }
    ++m_pos;
    value = *((int64_t*)(&m_buf[m_pos]));
    if (m_byteorder == ByteOrder::BigEndian) {
        char* first = (char*)&value;
        char* last = first + sizeof(int64_t);
        std::reverse(first, last);
    }
    m_pos += 8;
    return true;
}

bool DataStream::read(float& value) {
    if (m_buf[m_pos] != DataType::FLOAT) {
        return false;
    }
    ++m_pos;
    value = *((float*)(&m_buf[m_pos]));
    if (m_byteorder == ByteOrder::BigEndian) {
        char* first = (char*)&value;
        char* last = first + sizeof(float);
        std::reverse(first, last);
    }
    m_pos += 4;
    return true;
}

bool DataStream::read(double& value) {
    if (m_buf[m_pos] != DataType::DOUBLE) {
        return false;
    }
    ++m_pos;
    value = *((double*)(&m_buf[m_pos]));
    if (m_byteorder == ByteOrder::BigEndian) {
        char* first = (char*)&value;
        char* last = first + sizeof(double);
        std::reverse(first, last);
    }
    m_pos += 8;
    return true;
}

bool DataStream::read(string& value) {
    if (m_buf[m_pos] != DataType::STRING) {
        return false;
    }
    ++m_pos;
    int len;
    read(len);
    if (len < 0) {
        return false;
    }
    value.assign((char*)&(m_buf[m_pos]), len);
    m_pos += len;
    return true;
}

DataStream& DataStream::operator<<(bool value) {
    write(value);
    return *this;
}

DataStream& DataStream::operator<<(char value) {
    write(value);
    return *this;
}

DataStream& DataStream::operator<<(int32_t value) {
    write(value);
    return *this;
}

DataStream& DataStream::operator<<(int64_t value) {
    write(value);
    return *this;
}

DataStream& DataStream::operator<<(float value) {
    write(value);
    return *this;
}

DataStream& DataStream::operator<<(double value) {
    write(value);
    return *this;
}

DataStream& DataStream::operator<<(const char* value) {
    write(value);
    return *this;
}

DataStream& DataStream::operator<<(const std::string& value) {
    write(value);
    return *this;
}

DataStream& DataStream::operator>>(bool& value) {
    read(value);
    return *this;
}

DataStream& DataStream::operator>>(char& value) {
    read(value);
    return *this;
}

DataStream& DataStream::operator>>(int32_t& value) {
    read(value);
    return *this;
}

DataStream& DataStream::operator>>(int64_t& value) {
    read(value);
    return *this;
}

DataStream& DataStream::operator>>(float& value) {
    read(value);
    return *this;
}

DataStream& DataStream::operator>>(double& value) {
    read(value);
    return *this;
}

DataStream& DataStream::operator>>(string& value) {
    read(value);
    return *this;
}

} // namespace serialize
} // namespace Middleware
} // namespace hnu
