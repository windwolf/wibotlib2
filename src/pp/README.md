# PP 数据流处理库

## 功能介绍

PP 是面向嵌入式控制循环的同步数据流处理库。应用通过 `In<T>` 和 `Out<T>` 连接节点，
Builder 根据连接关系完成拓扑排序，`PipelineChain::tick()` 再按拓扑顺序依次调用节点。

PP 具有以下特征：

- 不使用动态内存；
- 节点、信号存储和 Pipeline 的容量均由应用静态确定；
- 一个 Input 只能连接一个 Output；
- 一个 Output 可以连接多个 Input，多个 Input 共享同一份信号存储；
- Builder 只在构建阶段保存边，Pipeline 运行阶段只保存排序后的节点指针；
- 未连接且用户不读取的 Output 可以不绑定存储，也可以跳过对应计算。

PP 处理的是每个 `tick()` 内的同步值流。上游节点先写入信号存储，下游节点随后读取该值。
事件队列、跨线程传输和异步背压不属于 PP 的职责。

## 端口和存储模型

`In<T>` 是只读端口，`Out<T>` 是写端口。端口本身不拥有数据，二者通过应用提供的存储传递值。

Output 的使用方式分为三类：

| Output 用途 | 是否绑定存储 | 行为 |
| --- | --- | --- |
| 未连接且用户不读取 | 否 | 节点不写入，并可跳过计算 |
| 未连接但用户需要读取 | 是 | 节点写入，用户在 `tick()` 后读取存储 |
| 连接到一个或多个 Input | 是 | 上游写入，下游从同一存储读取 |

连接到 Input 的 Output 必须先绑定存储。Builder 会拒绝未绑定 Output 的连接。

## 节点实现规范

每个节点继承 `INode`，并实现 `ready()`、`process()` 和 `reset()`。

### `ready()`

`ready()` 只检查：

- 所有必需 Input 是否已绑定；
- 节点配置是否有效。

`ready()` 不检查 Output。Output 是否使用不影响节点能否加入 Pipeline。

```cpp
bool ready() override {
    return inputs.x.bound() && configValid();
}
```

可选 Input 不能作为 `ready()` 的必要条件，节点应在 `process()` 中用 `bound()` 判断。

### `process()`

写入每个 Output 前必须检查 `bound()`。如果所有 Output 都未绑定，应尽早返回，避免无效计算。

```cpp
void process() override {
    if (!outputs.y.bound()) {
        return;
    }

    outputs.y.ref() = calculate(inputs.x.get());
}
```

多个 Output 共享一次计算时，先判断是否至少有一个 Output 被使用：

```cpp
void process() override {
    if (!outputs.value.bound() && !outputs.status.bound()) {
        return;
    }

    auto result = calculate(inputs.x.get());
    if (outputs.value.bound()) {
        outputs.value.ref() = result.value;
    }
    if (outputs.status.bound()) {
        outputs.status.ref() = result.status;
    }
}
```

不要把昂贵计算直接传给一个“可丢弃写入”函数。C++ 会先计算函数参数，无法达到按需计算的目的。

### `reset()`

`reset()` 负责恢复节点内部状态，不修改端口连接和信号绑定。

## Pipeline 构建规范

构建顺序必须是：

1. 创建生命周期足够长的节点、信号存储和 `PipelineChain`；
2. 为所有需要使用的 Output 绑定存储；
3. 将节点加入 Builder；
4. 连接 Output 和 Input；
5. 调用 `build()`；
6. 构建成功后周期调用 `tick()`。

必须遵守以下要求：

- `bind()` 必须在 `connect()` 之前调用；
- 节点和信号存储必须比 `PipelineChain` 活得更久；
- 完成连接后不得重新绑定 Output；
- 必须检查 `build()` 的返回值；
- Builder 会记录任何添加节点或连接失败，发生过错误后 `build()` 必然失败；
- 不允许环路；
- 节点数量不能超过 `MaxNodes`；
- 边数量不能超过 `MaxNodes * 4`；
- 一个 Input 不允许重复连接。

## 完整示例

下面的 Pipeline 将常量源连接到增益节点。`sourceValue` 是节点间信号，
`result` 是用户需要读取的输出。

```cpp
#include "pp/index.hpp"

using namespace wibot;

class GainNode : public INode {
   public:
    struct Inputs {
        In<f32> x;
    } inputs;

    struct Outputs {
        Out<f32> y;
        Out<u32> processCount;
    } outputs;

    explicit GainNode(f32 gain) : _gain(gain) {
    }

    bool ready() override {
        return inputs.x.bound();
    }

    void process() override {
        if (!outputs.y.bound() && !outputs.processCount.bound()) {
            return;
        }

        if (outputs.y.bound()) {
            outputs.y.ref() = inputs.x.get() * _gain;
        }
        if (outputs.processCount.bound()) {
            outputs.processCount.ref() = ++_processCount;
        }
    }

    void reset() override {
        _processCount = 0;
    }

   private:
    f32 _gain;
    u32 _processCount{0};
};

class Example {
   public:
    Example() {
        PipelineChainBuilder<2> builder;

        // source.x 连接到 gain.x，因此必须有节点间存储。
        builder.bind(_source.outputs.x, _sourceValue);

        // 用户需要读取 gain.y，因此为它绑定结果存储。
        builder.bind(_gain.outputs.y, result);

        // processCount 未连接且用户不读取，不绑定存储。
        builder.addNode(_source);
        builder.addNode(_gain);

        bool connected =
            builder.connect(_source, _source.outputs.x, _gain, _gain.inputs.x);
        ASSERT(connected, "PP connect failed");
        ASSERT(builder.build(_pipeline), "PP build failed");
    }

    void tick() {
        _pipeline.tick();
    }

    f32 result{};

   private:
    f32 _sourceValue{};

    ConstantSourceNode<f32> _source{1.5f};
    GainNode _gain{2.0f};
    PipelineChain<2> _pipeline;
};
```

调用一次 `tick()` 后，`result` 为 `3.0f`。`processCount` 没有存储，也不会发生写入。

## 数组 Output

数组 Output 使用 `bindArray()`：

```cpp
KeyEvent events[3]{};
builder.bindArray(scanner.outputs.events, events);
```

也可以只绑定其中一部分：

```cpp
KeyEvent firstEvent{};
builder.bind(scanner.outputs.events[0], firstEvent);
```

节点实现必须逐项检查 `bound()`，不能假设整个 Output 数组都已绑定。
