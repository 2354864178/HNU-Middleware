# `transport/config` 模块说明

`transport/config` 目录负责承载 HNU Middleware 传输层相关的配置数据结构。这里的内容不是具体算法实现，而是描述“一个通信角色需要哪些运行信息”和“这些通信参数该如何选择”。

这一层的目标是把传输相关的可配置项集中起来，便于上层模块统一创建、传递和保存配置，而不是把这些字段散落到各个实现文件里。

## 目录职责

该目录当前主要包含两类配置：

- 角色属性配置：描述某个通信角色在运行时的身份、进程、通道以及 QoS 组合。
- QoS 配置：描述消息历史、可靠性和持久性等传输语义。

它们通常用于：

- 标识一个发布者、订阅者或其他通信实体。
- 决定该实体在网络中的行为方式。
- 作为后续创建 transport、channel 或 endpoint 时的输入参数。

## 文件概览

### [`qos_profile.h`](qos_profile.h)

定义传输层常用的 QoS 枚举和 `QosProfile` 结构体。

### [`RoleAttributes.h`](RoleAttributes.h)

定义角色属性结构体 `RoleAttributes`，把进程信息、通道信息和 QoS 配置组合在一起。

## QoS 配置

`QosProfile` 用来描述消息的传输策略，目前包含三个维度。

### HistoryPolicy

历史缓存策略，表示消息被保留的方式。

- `HISTORY_SYSTEM_DEFAULT`：使用系统默认策略。
- `HISTORY_KEEP_LAST`：只保留最近的一部分历史，通常更适合只关心最新数据的场景。
- `HISTORY_KEEP_ALL`：保留全部历史，适合需要完整消息轨迹的场景。

### ReliabilityPolicy

可靠性策略，表示消息在网络中传输时对丢包的容忍程度。

- `RELIABILITY_SYSTEM_DEFAULT`：使用系统默认策略。
- `RELIABILITY_RELIABLE`：可靠传输，强调尽可能送达。
- `RELIABILITY_BEST_EFFORT`：尽力而为，不保证每条消息都到达。

### DurabilityPolicy

持久性策略，表示消息在发布后是否以及如何对后续订阅者保留。

- `DURABILITY_SYSTEM_DEFAULT`：使用系统默认策略。
- `DURABILITY_TRANSIENT_LOCAL`：在本地临时保留最近状态。
- `DURABILITY_VOLATILE`：不保留历史状态，强调实时性。

### 默认值

`QosProfile` 的默认值如下：

- `history = HISTORY_KEEP_LAST`
- `reliability = RELIABILITY_RELIABLE`
- `durability = DURABILITY_VOLATILE`

这意味着默认配置更偏向“最新数据、可靠传输、不保留持久状态”的常见实时通信场景。

## 角色属性配置

`RoleAttributes` 用于描述一个通信角色的完整运行属性。它把角色身份、通道信息和 QoS 一次性打包，便于上层统一传递。

### 字段说明

- `host_name`：主机名，用于标识当前运行机器。
- `host_ip`：主机 IP 地址，通常用于区分节点或调试输出。
- `process_id`：进程 ID，用于唯一标识进程实例。
- `channel_name`：通道名称，通常是逻辑通信主题或通道标识。
- `channel_id`：通道 ID，注释中说明它是 `channel_name` 的 hash 值，便于快速比较或查找。
- `qos_profile`：该角色对应的 QoS 配置。
- `id`：角色本身的唯一标识，通常用于进一步区分同类实体。

### 设计含义

`RoleAttributes` 体现的是“角色 + 环境 + 传输策略”的组合。

它的用途一般是：

- 作为节点创建通信实体时的输入参数。
- 作为调试信息或日志信息的一部分。
- 作为后续路由、匹配、过滤时的依据。

## 配置使用建议

### 什么时候改 `QosProfile`

- 如果你更关心实时性，通常会保持 `KEEP_LAST` + `BEST_EFFORT` 或 `RELIABLE` 的组合。
- 如果你希望下游订阅者能拿到历史数据，可以考虑 `KEEP_ALL`。
- 如果你希望订阅者后加入时仍能看到最近状态，可以考虑 `TRANSIENT_LOCAL`。

### 什么时候改 `RoleAttributes`

- 当一个通信角色需要区分来源节点时，可以设置 `host_name`、`host_ip` 和 `process_id`。
- 当一个系统包含多个逻辑通道时，可以通过 `channel_name` 和 `channel_id` 区分它们。
- 当不同角色在可靠性、历史保留策略上有不同要求时，可以为每个角色指定不同的 `qos_profile`。

## 示例

下面是一个典型配置示意：

```cpp
using namespace hnu::Middleware::config;

RoleAttributes attr;
attr.host_name = "robot-a";
attr.host_ip = "192.168.1.10";
attr.process_id = 12345;
attr.channel_name = "camera/image";
attr.channel_id = 0x12345678;
attr.qos_profile.history = HISTORY_KEEP_LAST;
attr.qos_profile.reliability = RELIABILITY_RELIABLE;
attr.qos_profile.durability = DURABILITY_VOLATILE;
attr.id = 1;
```

这个示例表达的是：

- 当前角色运行在 `robot-a` 上。
- 使用 `camera/image` 作为逻辑通道。
- 采用可靠传输。
- 只保留最近历史，不做持久化。

## 与其他模块的关系

- `common/global_data.*` 更偏向提供运行时环境信息，例如主机 IP、进程 ID 和进程组。
- `transport/config/*` 则是在传输层语义上把这些信息组织成可配置结构体。

也就是说，`common` 更像“原材料”，`transport/config` 更像“把原材料整理成通信配置”。

## 备注

- 当前目录下的文件都比较轻量，适合直接被头文件包含，不需要额外复杂的初始化逻辑。
- 如果后续配置项继续增加，建议在这里继续补充统一的结构体和枚举，而不是把配置散落到各个实现类里。
