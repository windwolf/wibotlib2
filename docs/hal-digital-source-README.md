# HAL数字输入源 (HalDigitalSource)

基于STM32 HAL库的数字输入源实现，继承自`DigitalSource`，提供实际的GPIO硬件访问功能。

## 概述

`HalDigitalSource`是`DigitalSource`的具体实现，它：
- 继承了所有Pipeline接口和数字输入处理功能
- 提供实际的STM32 GPIO硬件访问
- 支持多达32个GPIO引脚的并行处理
- 保持与HAL库的完全兼容性

## 主要特性

### 🔌 GPIO硬件接入
- 基于STM32 HAL库的GPIO读取
- 支持任意GPIO端口和引脚组合
- 灵活的引脚配置管理

### 📊 继承的数字处理功能
- 数字信号消抖（可配置消抖时间）
- 信号反转（按位配置）
- 多通道并行处理
- Pipeline接口兼容性

### ⚡ 高性能特性
- 一次性读取所有配置的GPIO
- 最小化HAL库调用开销
- 模板化设计避免运行时开销

## 类结构

### 核心类

```cpp
template <uint8_t CHANNELS>
class HalDigitalSource : public DigitalSource<CHANNELS>
```

### 配置结构

```cpp
struct GpioPinConfig {
    GPIO_TypeDef* port;  ///< GPIO端口
    uint16_t      pin;   ///< GPIO引脚
};

struct HalDigitalSourceConfig {
    DigitalSourceConfig digitalConfig;  ///< 基础数字输入配置
    GpioPinConfig       pins[32];       ///< GPIO引脚配置数组
    uint8_t             enabledPins;    ///< 实际启用的引脚数量
};
```

## 使用方法

### 基本使用

```cpp
#include "newHal/hal-digital-source.hpp"

// 1. 配置GPIO引脚
GpioPinConfig pins[] = {
    {GPIOB, GPIO_PIN_1},  // 通道0: PB1
    {GPIOB, GPIO_PIN_0},  // 通道1: PB0
    {GPIOC, GPIO_PIN_6},  // 通道2: PC6
};

// 2. 创建HAL数字输入源
auto digitalInputs = HalDigitalSource<3>(pins, 3, 0, 50);  // 3通道，无反转，50ms消抖

// 3. 主循环中更新
while (true) {
    digitalInputs.update();  // 读取GPIO并处理
    
    // 4. 获取处理后的结果
    bool button1 = digitalInputs.getValue(0);
    bool button2 = digitalInputs.getValue(1);
    uint32_t allInputs = digitalInputs.getValues();
    
    // 使用结果...
    
    delay(10);  // 适当的采样间隔
}
```

### 使用预定义引脚

项目中定义的引脚可以通过便捷函数获取：

```cpp
// 使用项目预定义的引脚
GpioPinConfig pins[] = {
    GpioPins::getDiPwr(),     // DI_PWR引脚
    GpioPins::getDoPwrenN(),  // DO_PWREN_N引脚
    GpioPins::getSpi2Lock(),  // SPI2_LOCK引脚
    GpioPins::getSpi3Lock()   // SPI3_LOCK引脚
};

auto systemInputs = HalDigitalSource<4>(pins, 4);
```

### 高级配置

```cpp
// 使用配置结构
HalDigitalSourceConfig config = {
    {0x06, 30},  // 通道1和2反转，30ms消抖
    {
        {GPIOB, GPIO_PIN_1},  // 通道0
        {GPIOB, GPIO_PIN_0},  // 通道1 (反转)
        {GPIOC, GPIO_PIN_6},  // 通道2 (反转)
        {GPIOA, GPIO_PIN_15}  // 通道3
    },
    4
};

auto advancedInputs = HalDigitalSource<4>(config);
```

### 动态重配置

```cpp
// 运行时重新配置
DigitalSourceConfig newConfig = {0x00, 10};  // 移除反转，10ms消抖
advancedInputs.configure(newConfig);

// 添加新的GPIO引脚
advancedInputs.configureGpioPin(4, GPIOA, GPIO_PIN_0);
```

## API参考

### 构造函数

```cpp
// 简化构造函数
HalDigitalSource(const GpioPinConfig pins[], uint8_t enabledPins,
                 uint32_t inverse = 0, uint8_t debounceTimeMs = 50);

// 完整配置构造函数  
explicit HalDigitalSource(const HalDigitalSourceConfig& config);
```

### 主要方法

```cpp
// Pipeline接口 (继承)
void update() override;                    // 更新所有通道
bool getValue(uint8_t channel) const;      // 获取指定通道值
uint32_t getValues() const;                // 获取所有通道值
void reset();                              // 重置状态

// GPIO硬件接口
bool readGpioChannel(uint8_t channel) const;      // 直接读取GPIO
uint32_t readAllGpioChannels() const;             // 读取所有GPIO

// 配置接口
void configureHal(const HalDigitalSourceConfig& config);           // 重新配置
void configureGpioPin(uint8_t channel, GPIO_TypeDef* port, uint16_t pin);  // 配置单个引脚
```

## 工作原理

### 数据流程

```
GPIO硬件 -> HAL_GPIO_ReadPin() -> readAllGpioChannels() -> updateRawValues() -> 
基类处理(消抖+反转) -> getValue() -> 应用程序
```

### 更新过程

1. **GPIO读取**：`update()`调用`readAllGpioChannels()`
2. **原始数据传递**：通过`updateRawValues()`传递给基类
3. **数字信号处理**：基类执行消抖和反转逻辑
4. **结果输出**：通过`getValue()`等方法获取处理结果

## 性能特性

### 优化策略

- **批量GPIO读取**：一次性读取所有配置的引脚
- **模板特化**：编译时确定通道数量
- **最小HAL调用**：每个引脚每次更新只调用一次HAL函数

### 性能指标

- **GPIO读取**：~1-2μs per channel (取决于HAL实现)
- **总处理时间**：~10-20μs for 8 channels (包含消抖逻辑)
- **内存占用**：~80 bytes for 8-channel instance

## 应用场景

### 工业控制

```cpp
// 工业设备状态监控
GpioPinConfig systemPins[] = {
    GpioPins::getDiPwr(),     // 电源状态
    GpioPins::getSpi2Lock(),  // 通讯锁定
    GpioPins::getSpi3Lock(),  // 通讯锁定
    makeGpioPin(GPIOA, GPIO_PIN_8),  // 紧急停止
};

auto systemMonitor = HalDigitalSource<4>(systemPins, 4, 0x08, 50);  // 紧急停止反转
```

### 用户界面

```cpp
// 按钮输入处理
GpioPinConfig buttonPins[] = {
    makeGpioPin(GPIOC, GPIO_PIN_0),  // 按钮1
    makeGpioPin(GPIOC, GPIO_PIN_1),  // 按钮2  
    makeGpioPin(GPIOC, GPIO_PIN_2),  // 按钮3
    makeGpioPin(GPIOC, GPIO_PIN_3),  // 按钮4
};

auto userButtons = HalDigitalSource<4>(buttonPins, 4, 0x0F, 30);  // 全部反转，30ms消抖
```

### 传感器接入

```cpp
// 数字传感器状态
GpioPinConfig sensorPins[] = {
    makeGpioPin(GPIOD, GPIO_PIN_0),  // 温度报警
    makeGpioPin(GPIOD, GPIO_PIN_1),  // 压力报警
    makeGpioPin(GPIOD, GPIO_PIN_2),  // 液位检测
};

auto sensorInputs = HalDigitalSource<3>(sensorPins, 3, 0, 100);  // 100ms消抖
```

## 注意事项

### GPIO初始化

确保在使用前已正确初始化GPIO：

```cpp
// 在使用HalDigitalSource前调用
MX_GPIO_Init();  // STM32CubeMX生成的初始化函数
```

### 线程安全

HalDigitalSource不是线程安全的，在多线程环境中需要适当的同步机制。

### 错误处理

- 无效通道访问返回`false`
- 未配置的引脚读取返回`false`
- 配置验证在构造时进行

## 示例代码

详细示例请参考：
- `hal-digital-source-example.hpp/cpp` - 完整使用示例
- `hal-digital-source-test.cpp` - 测试代码

运行示例：

```cpp
#include "newHal/hal-digital-source-example.hpp"

// 在main函数中
HalDigitalSourceExample::runAllExamples();
```

## 与DigitalSource的关系

```
HalDigitalSource<N>
    ├── 继承自 DigitalSource<N>
    │   ├── 继承自 SyncPipeline<bool, uint32_t>
    │   └── 提供消抖、反转等数字信号处理
    └── 添加HAL GPIO硬件接口
        ├── GPIO配置管理
        ├── 硬件状态读取  
        └── 与STM32 HAL库集成
```

HalDigitalSource是DigitalSource的具体硬件实现，保持了所有抽象接口的兼容性，同时提供了实际的GPIO访问能力。