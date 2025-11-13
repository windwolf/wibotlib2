# DigitalSource - 数字输入Pipeline数据源

## 概述

`DigitalSource` 是一个基于Pipeline接口的数字输入处理器，它直接实现了多通道数字输入的消抖和反转处理功能。作为Pipeline数据源组件，它可以与其他Pipeline组件（如滤波器、映射器等）无缝集成。

## 主要特性

### 1. Pipeline接口兼容
- 继承自 `SyncPipeline<bool>`
- 实现标准Pipeline接口：`update()`, `getValue()`, `reset()`
- 支持多通道数字输入（最多32通道）

### 2. 数字输入处理
- 直接实现数字输入处理逻辑
- 支持信号反转配置
- 内建消抖处理机制
- 批量和单通道配置选项

## 类设计

### 模板参数
```cpp
template <uint8_t CHANNELS>
class DigitalSource : public SyncPipeline<bool>
```
- `CHANNELS`: 通道数量，范围1-32

### 配置结构体

#### DigitalSourceConfig
```cpp
struct DigitalSourceConfig {
    uint32_t inverse;           ///< 反转掩码，32位对应最多32个通道
    uint8_t  debounceTimeMs;    ///< 消抖时间（毫秒）
};
```



## 核心方法

### Pipeline接口实现
- `void update()` - 更新Pipeline状态，处理待处理的输入
- `bool getValue(uint8_t channel)` - 获取指定通道的处理后值
- `void reset()` - 重置Pipeline状态

### 数字输入管理
- `void updateRawInputs(uint32_t rawValues)` - 更新原始输入值
- `uint32_t getAllValues()` - 批量获取所有通道值
- `void configureChannel(uint8_t channel, bool inverse, uint8_t debounceTimeMs)` - 配置单通道
- `void configure(const DigitalSourceConfig& config)` - 批量配置

## 使用场景

### 1. 基本数字输入读取
```cpp
// 创建8通道数字输入源
DigitalSource<8> digitalSource(0x03, 50);  // 反转通道0,1，50ms消抖

// 更新GPIO状态
digitalSource.updateRawInputs(gpioReadAll());
digitalSource.update();

// 读取各通道
for (uint8_t ch = 0; ch < 8; ch++) {
    bool state = digitalSource.getValue(ch);
    // 处理通道状态...
}
```

### 2. 多通道批量操作
```cpp
// 批量更新和读取
digitalSource.updateRawInputs(gpioReadAll());
digitalSource.update();

// 批量获取所有通道状态
uint32_t allStates = digitalSource.getAllValues();
for (uint8_t ch = 0; ch < 8; ch++) {
    bool state = (allStates >> ch) & 1U;
    printf("Channel %d: %s\n", ch, state ? "HIGH" : "LOW");
}
```

### 3. Pipeline集成
```cpp
// 作为Pipeline数据源使用
SyncPipeline<bool>* source = &digitalSource;

// Pipeline标准操作
source->update();
bool channelState = source->getValue(0);

// 连接到后续Pipeline组件
// FilterPipeline<bool>* filter = new SomeDigitalFilter(source);
// MapperPipeline<bool, int>* mapper = new BoolToIntMapper(filter);
```

## 设计特点

### 1. 高效的内存使用
- 最小化内存占用
- 使用位掩码高效存储多通道状态
- 静态断言确保通道数限制

### 2. 实时性考虑
- 轻量级的update()操作
- 最小化系统调用
- 高效的位操作和消抖处理

### 3. 独立性设计
- 不依赖外部DigitalInput类
- 直接实现数字输入处理逻辑
- 模板化支持任意通道数
- 显式实例化常用配置

### 4. 错误处理
- 边界检查防止越界访问
- 优雅的错误返回值
- 配置参数验证

## 对比MemorySource

| 特性     | MemorySource       | DigitalSource    |
| -------- | ------------------ | ---------------- |
| 数据类型 | int16_t (模拟信号) | bool (数字信号)  |
| 输入源   | DMA缓冲区          | GPIO寄存器       |
| 处理特性 | ADC转换、校准偏移  | 消抖、信号反转   |
| 额外功能 | 自动校准           | 内建数字输入处理 |
| 通道数   | 模板参数           | 模板参数(≤32)    |

## 注意事项

1. **通道限制**: 最多支持32个通道（受uint32_t位宽限制）
2. **消抖时间**: 所有通道共享相同的消抖时间设置
3. **内存效率**: 使用单个uint32_t存储所有通道状态，内存占用最小化

## 扩展建议

1. **高级滤波**: 可以添加数字滤波算法
2. **边沿检测**: 增加上升沿/下降沿检测功能
3. **事件触发**: 支持状态变化回调机制
4. **多组消抖**: 支持不同通道使用不同消抖时间
5. **状态统计**: 可以基于此类构建数字信号统计分析功能