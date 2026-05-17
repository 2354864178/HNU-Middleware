**Transport Common 模块说明**

本文档介绍 `transport/common` 目录下的实现思路、核心类职责、与仓库其余模块的交互关系，以及扩展与测试建议。

**目录与文件**
- `endpoint.h`, `endpoint.cpp`：`Endpoint` 类，封装通信端点的角色属性与标识。
- `identity.h`, `identity.cpp`：`Identity` 类，生成/持有 8 字节标识符并提供哈希值计算。
- `test/`：放置该模块的单元测试（如果存在）。

**总体职责**
- 提供传输层通用的基础类型与工具：唯一标识（Identity）、端点元信息（Endpoint）。
- 将上层配置（如 `RoleAttributes`）与全局环境信息（`GlobalData`）结合，初始化端点属性。

**核心类与实现要点**

1) `Identity`
- 作用：为通信实体（endpoint、node 等）生成并保持一个固定长度（8 字节）的标识符，以及便于比较/打印的哈希值。
- 实现细节：
  - 使用 `libuuid`（uuid_generate）生成原始 UUID，取前 8 字节作为标识数据。
  - `Update()` 使用 `common::Hash`（见 `common/util.h`）将 8 字节内容哈希为 `uint64_t`，作为 `HashValue()` 返回值。
  - 提供拷贝构造、赋值、比较运算符以及 `ToString()`（返回 hash 值字符串）。
- 设计考量：8 字节长度在大多数场景下足够区分实体且节省内存；使用哈希方便日志与索引。

2) `Endpoint`
- 作用：保存一个端点的 `Identity`（默认生成）与 `RoleAttributes`（来自配置），并在构造时补全缺省字段。
- 实现细节（见构造函数）：
  - 若 `RoleAttributes.host_name` 为空，则从 `common::GlobalData::Instance()->HostName()` 填充。
  - 若 `RoleAttributes.process_id` 为 0，则使用 `GlobalData::Instance()->ProcessId()` 填充。
  - 若 `RoleAttributes.id` 为 0，则使用 `Identity::HashValue()` 作为 `id`。
- 设计考量：在端点创建时自动补齐环境相关信息，减轻上层调用代码负担，保证端点元信息一致性。

**与其他模块的交互**
- `RoleAttributes`（定义在 `transport/config/RoleAttributes.h`）由上层（配置解析或创建端点的工厂）传入 `Endpoint`。
- `Endpoint` 在构造时依赖 `common::GlobalData`（`common/global_data.h`）获取主机名与进程 id。
- `Identity` 使用 `common::util` 中的 `Hash` 函数；因此 `common` 模块必须能够在链接时提供相关实现。

**线程安全与生命周期**
- `Identity` 是值类型（POD-ish），拷贝/赋值为深拷贝数据/哈希，线程安全性由调用方决定；在多线程场景下，建议将 `Identity` 实例作为只读共享或通过拷贝传递。
- `Endpoint` 内部包含 `Identity` 与 `RoleAttributes` 副本，构造完成后为只读（当前实现没有提供修改接口），因此可安全在多个线程间共享 `const Endpoint`。

**扩展建议**
- 标识长度与生成策略：如果需要更高唯一性或可读性，考虑把 `Identity` 的长度与生成策略参数化（例如 16 字节或完整 UUID 的文本表示）。
- 可选启用注册中心：在 `Endpoint` 构造或注册阶段调用 `GlobalData::RegisterChannel` / `RegisterNode` 等，把本地端点信息注册到全局表以便跨组件查询。
- 增加序列化：提供 `ToProto()` / `FromProto()` 或 JSON 序列化接口，便于跨进程/网络传输端点信息。

**测试建议**
- `Identity`：测试随机生成是否产生不同的 `HashValue()`（统计学断言），以及拷贝/比较语义正确。
- `Endpoint`：构造时传入部分空字段的 `RoleAttributes`，断言构造后字段由 `GlobalData` 正确填充。可使用 mock `GlobalData`（或在测试中修改 `GlobalData::Instance()` 行为）来控制返回值。

**常见问题与注意事项**
- `Endpoint` 构造里当前对 `RoleAttributes` 的修改是直接在拷贝 `attr_` 上进行（构造函数内修改 `attr_`），确保上层传入的 `RoleAttributes` 副本不会意外被修改（当前实现已拷贝）。
- `GlobalData::Instance()` 的单例实现会延迟创建并调用 `InitHostInfo()`；在某些测试或非标准运行环境（无网络接口）下，主机名/IP 的获取可能失败或返回默认值，应在测试中考虑注入或 mock。

**文件路径参考**
- `transport/common/identity.h` — `Identity` 声明
- `transport/common/identity.cpp` — `Identity` 实现（使用 `uuid` 与 `common::Hash`）
- `transport/common/endpoint.h` — `Endpoint` 声明
- `transport/common/endpoint.cpp` — `Endpoint` 实现（依赖 `common/global_data.h`）

如果你希望，我可以：
- 把 `Identity` 的测试用例添加到 `transport/common/test/`；或
- 为 `Endpoint` 增加序列化支持（JSON 或 protobuf）；或
- 将这份文档合并到仓库根 README 的“模块说明”部分。
