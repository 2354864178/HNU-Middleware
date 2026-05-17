**Transport QoS 设计与实现说明**

本文档总结并说明仓库中 `transport/qos` 模块的实现思路、设计决策、与 Fast‑DDS/RTPS 映射关系，以及后续扩展和测试建议。目的是让维护者快速理解当前实现并能安全扩展或集成第三方中间件。

**概览**:
- **职责**：定义一套轻量级的 QoS 抽象（`QosProfile` 及枚举类型），提供常用配置（`QosProfileConf`），并在传输层将抽象映射到底层中间件（如 Fast‑DDS）的具体属性。
- **位置**：代码位于 `transport/qos`，主要文件 `qos_profile_conf.h/.cpp`。

**设计目标**:
- 清晰、可扩展的 QoS 抽象，屏蔽底层中间件差异。
- 提供合理的默认配置以覆盖常见使用场景（sensor data、parameters、services 等）。
- 便于测试与本地覆盖（可在不链接 Fast‑DDS 的情况下运行单元测试）。

**核心数据结构**:
- `QosProfile`：承载 QoS 配置的结构（history、depth、mps、reliability、durability 等字段）。
- 枚举：
  - `QosHistoryPolicy`（例如 `HISTORY_KEEP_LAST`、`HISTORY_KEEP_ALL`、`HISTORY_SYSTEM_DEFAULT`）
  - `QosReliabilityPolicy`（`RELIABILITY_RELIABLE`、`RELIABILITY_BEST_EFFORT`）
  - `QosDurabilityPolicy`（`DURABILITY_VOLATILE`、`DURABILITY_TRANSIENT_LOCAL` 等）

这些类型由 `transport/config/qos_profile.h`（或上层 config）定义并被 `QosProfileConf` 使用。

**QosProfileConf 实现要点**:
- `CreateQosProfile(...)`：工厂函数，接收基本策略与参数并返回填充好的 `QosProfile`。
- 预定义常量：若干常用 `QosProfile` 常量（如 `QOS_PROFILE_DEFAULT`、`QOS_PROFILE_SENSOR_DATA` 等）放在 `qos_profile_conf.cpp` 中，便于统一管理与引用。
- 系统默认值（`QOS_HISTORY_DEPTH_SYSTEM_DEFAULT`、`QOS_MPS_SYSTEM_DEFAULT`）为 0，表示交由底层中间件决定具体语义。

实现理由：将所有默认配置集中在一处，便于跨模块统一策略和调整。

**与 Fast‑DDS / RTPS 的映射**:
- 在传输层（例如 `transport/rtps/attributes_filler.h/.cpp`）创建 `TopicAttributes`、`WriterAttributes`、`ReaderAttributes`、`WriterQos`、`ReaderQos` 等并根据 `QosProfile` 填充。
- 映射原则：
  - history/depth -> `HistoryAttributes` / `HistoryQos`（keep_last -> depth；keep_all -> keep_all）
  - reliability -> `WriterQos.reliability()` / `ReaderQos.reliability()`
  - durability -> `WriterQos.durability()` / `ReaderQos.durability()`
  - mps (messages per second) -> 可用于速率限制或作为 hint，若底层支持则映射到相应速率设置；否则仅作为上层控制参数使用。

注意事项：不同中间件对同一 QoS 字段的语义和默认值可能不同。`QosProfileConf` 只负责抽象与默认值；`AttributesFiller` 或类似模块负责把抽象语义正确地映射到特定中间件属性并处理中间件不支持的情况。

**运行时行为与线程安全**:
- `QosProfile` 本身是 POD 风格的配置对象，建议在启动/创建 Topic/Writer/Reader 前构建并在只读场景下共享。
- 如果需要在运行时修改 QoS（动态 QoS），应在传输层序列化变更并按中间件支持的方式应用（例如通过重新创建实体或使用中间件提供的动态 QoS 接口）。当前实现假定 QoS 在实体创建时确定。

**集成点**:
- `AttributesFiller::FillInWriterAttr` / `FillInReaderAttr`：读取 `QosProfile` 并填充 RTPS/FAST‑DDS 的相应结构。
- 主题/节点初始化：上层模块（如 `transport` 的 factory）在创建 writer/reader 前获取 `QosProfile` 并调用填充函数。

**如何新增 QoS 配置**:
1. 在 `qos_profile_conf.h/.cpp` 添加常量（通过 `CreateQosProfile` 生成）。
2. 在需要使用的模块引用相应常量（不要直接硬编码 magic 值）。
3. 如需映射到底层新字段，更新 `AttributesFiller` 中的映射逻辑并添加单元测试。

**测试建议**:
- 单元测试：验证 `CreateQosProfile` 返回的 `QosProfile` 字段与预期常量一致。
- 集成测试：在启用了 Fast‑DDS 的 CI 环境中，启动一个简单的 publisher/subscriber，验证在不同 QoS 下消息交付行为（可靠/最佳努力、持久性/瞬时、history depth 行为）。
- Mock 测试：在不依赖 Fast‑DDS 的情况下，使用 `AttributesFiller` 的 mock 替代实现来断言映射规则是否正确。

**常见问题与注意点**:
- `mps` 语义：库中将其作为每秒消息速率的提示；若下层不支持速率限制，需由上层在发送侧做限速处理。
- 默认值策略：`SYSTEM_DEFAULT` 表示让下层使用自己的默认行为，避免在抽象层强行覆盖会造成与中间件行为不一致的问题。

**迁移建议（若替换底层中间件）**:
- 保持 `QosProfile` 抽象不变，仅修改 `AttributesFiller` 中的映射逻辑。
- 编写映射表文档：列出抽象字段到新中间件字段的对应关系与不可映射项（并提供替代实现或 fallback）。

---
维护者备注：把 `QosProfileConf` 与 `AttributesFiller` 的职责严格区分：前者负责“值与默认策略”，后者负责“语义到中间件 API 的映射”。这样可在未来更换中间件时最小化修改范围。
