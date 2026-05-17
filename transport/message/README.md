**Transport Message 模块说明**

本文档详细介绍 `transport/message` 下的实现思路、关键类型、序列化格式、线程与并发模型、使用示例以及测试与扩展建议，便于维护与扩展。

目录与文件
- `message_info.h/.cpp`：`MessageInfo`，携带帧级元信息（sender id、seq num、spare id、channel id），提供二进制序列化/反序列化。
- `listener_handler.h`：模板类 `ListenerHandler<MessageT>`，管理消息监听器注册、连接关系与分发逻辑。

设计目标
- 将消息数据与传输元信息分离：`MessageInfo` 保存必要标识（如发送者、序列号、通道），上层业务只需专注消息体 `MessageT`。
- 提供轻量且高效的二进制序列化以便在网络/共享内存中传输小而固定的元信息。
- 支持按发送者（oppo_id）或全局广播的监听器注册，以及线程安全的注册/注销与分发。

核心类型与实现要点

1. MessageInfo
- 字段：
  - `Identity sender_id_`（8 字节标识）
  - `Identity spare_id_`（备用标识）
  - `uint64_t channel_id_`（通道 id，可选）
  - `uint64_t seq_num_`（序列号）
- 常量：`kSize = 2 * ID_SIZE + sizeof(uint64_t)`（二进制表示不包含 `channel_id_`，设计上为固定长度）
- 接口：
  - 构造、拷贝、比较运算符
  - `SerializeTo(std::string*)` / `SerializeTo(char*, len)`：按二进制追加 sender_id、seq_num、spare_id
  - `DeserializeFrom(...)`：从二进制恢复字段，要求长度等于 `kSize`
- 设计考量：二进制布局尽量紧凑且固定长度，便于直接 memcpy 到 buffer，适用于零拷贝场景。

2. ListenerHandler<MessageT>
- 作用：管理对 `MessageT` 的监听器（回调），支持三类注册：全局（任何发送者）、按发送者（oppo_id）、单个 reader 的连接管理。
- 主要成员：
  - `MessageSignal signal_`：全局广播信号
  - `ConnectionMap signal_conns_`：存储基于 self_id 的连接
  - `MessageSignalMap signals_`：按 oppo_id 的信号集合
  - `signals_conns_`：按 oppo_id 保存各 self_id 的连接，便于断开特定 reader
  - `AtomicRWLock rw_lock_`：读写锁保护连接结构
- 注册 API：
  - `Connect(self_id, listener)`：注册到全局 `signal_`
  - `Connect(self_id, oppo_id, listener)`：为特定发送者注册
  - `Disconnect(...)`：按 self 或 self/oppo 移除连接
- 分发：
  - `Run(msg, msg_info)`：先触发 `signal_`（全局监听），再根据 `msg_info.sender_id().HashValue()` 查找 `signals_` 并触发对应信号
  - `RunFromString(...)`：当前未实现（记录错误），建议实现反序列化并调用 `Run`
- 线程模型：
  - 读多写少场景：分发时仅读锁（获取 `signals_`），注册/注销时写锁保护结构修改
  - `Signal`（基于 `base/signal.h`）负责连接生命周期管理（Connection 对象）

序列化与网络交互
- `MessageInfo::SerializeTo` 输出固定长度的二进制数据（sender_id + seq_num + spare_id），上层可先发送该 metadata 再发送消息体，或将 metadata 与消息体合并为单帧传输。
- 设计建议：若通过 UDP 或共享内存传输，请将 `kSize` 放在固定偏移并保证接收端严格按 `DeserializeFrom` 校验长度。

使用示例（伪代码）
```
ListenerHandler<MyMsg> handler;
handler.Connect(self_id, [](const auto& msg, const MessageInfo& info){
    // 处理消息
});

// 在接收层：
MessageInfo info; info.DeserializeFrom(meta_bytes);
auto msg = std::make_shared<MyMsg>(); // 反序列化消息体
handler.Run(msg, info);
```

扩展建议
- 实现 `RunFromString`：使用 `serialize::DataStream` 反序列化 `MessageT`，再调用 `Run`
- 支持按通道过滤：目前基于 sender_id 分发，可扩展检查 `msg_info.channel_id()` 来同时支持频道级别订阅
- 增加统计/限流：在 `Run` 中记录每个 oppo_id 的消息速率，必要时执行背压或丢弃策略

测试建议
- 单元测试：
  - `MessageInfo` 的序列化/反序列化一致性（边界长度、不正确长度的拒绝）
  - `ListenerHandler` 的注册/注销并发性测试（多线程注册/分发）
  - 按 oppo_id 的多读者场景：多个 reader 同时接收同一个 writer 消息
- 集成测试：在本地通过 loopback transport 或内存通道模拟发送者/接收者，验证顺序性与并发正确性

注意事项
- `MessageInfo::DeserializeFrom` 对长度严格要求为 `kSize`，如果未来 `channel_id_` 也序列化，需要更新 `kSize` 并保证兼容性（例如通过版本号或头字段指示可选字段）
- `RunFromString` 当前未实现，请谨慎调用，优先使用 `Run` 接口传入已反序列化的消息对象

文件路径参考
- `transport/message/message_info.h/.cpp`
- `transport/message/listener_handler.h`

如果需要，我可以：
- 添加 `RunFromString` 的实现并提供示例序列化代码；或
- 为 `MessageInfo` 与 `ListenerHandler` 增加单元测试样例并把测试文件放入 `transport/message/test/`。
