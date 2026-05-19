#include <iostream>
#include <vector>
#include <cassert>

#include "../message_info.h"
#include "../../common/identity.h"

using namespace hnu::Middleware::transport;

int main() {
    // 基本构造
    MessageInfo a;
    assert(a.seq_num() == 0);

    // 填充字段
    Identity s(true);
    Identity spare(true);
    a.set_sender_id(s);
    a.set_spare_id(spare);
    a.set_seq_num(12345);

    // 序列化到 string
    std::string buf;
    bool ok = a.SerializeTo(&buf);
    assert(ok);
    assert(buf.size() == MessageInfo::kSize);

    // 序列化到原始缓冲
    std::vector<char> raw(MessageInfo::kSize);
    ok = a.SerializeTo(raw.data(), raw.size());
    assert(ok);

    // 反序列化
    MessageInfo b;
    ok = b.DeserializeFrom(buf);
    assert(ok);
    assert(a == b);

    // 错误长度拒绝
    std::string bad(buf);
    bad.push_back('x');
    ok = b.DeserializeFrom(bad.data(), bad.size());
    assert(!ok);

    // 空指针序列化失败
    ok = a.SerializeTo(nullptr);
    assert(!ok);

    // 缓冲区长度不足
    ok = a.SerializeTo(raw.data(), raw.size() - 1);
    assert(!ok);

    std::cout << "message_info_test: OK\n";
    return 0;
}
