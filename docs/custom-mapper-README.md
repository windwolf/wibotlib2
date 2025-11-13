# CustomMapper 使用指南

## 概述

`CustomMapper`是一个仿造`LinearMapper`实现的自定义映射器，允许用户提供自定义映射函数来实现任意复杂的映射逻辑。它支持多通道实时无状态映射，具有高度的灵活性和可扩展性。

## 文件结构

```
wibotlib/src/SyncPipeline/mapper/
├── custom-mapper.hpp          # 自定义映射器头文件
├── custom-mapper.cpp          # 自定义映射器实现文件
├── linear-mapper.hpp          # 线性映射器（参考）
└── linear-mapper.cpp          # 线性映射器实现（参考）

wibotlib/example/
├── custom-mapper-example.cpp  # 自定义映射器使用示例
└── linear-mapper-example.cpp  # 线性映射器示例（参考）

test_custom_mapper.cpp         # 自定义映射器功能测试
```

## 主要特性

### 1. 泛型设计
- 支持任意输入类型`TIn`和输出类型`TOut`
- 编译时确定的多通道数量（模板参数`CHANNELS`）
- 类型安全的模板化实现

### 2. 自定义映射函数
- 使用`std::function`提供灵活的函数接口
- 支持lambda表达式、函数指针、函数对象等
- 可访问通道索引，支持通道相关的映射逻辑

### 3. 简洁高效设计
- 纯函数式映射，无内部状态
- 轻量级配置结构
- 高性能的直接函数调用

### 4. 实时无状态处理
- 无内部状态，每次调用都是独立的
- 实时处理，无延迟
- 配置可动态更新

## API参考

### 模板参数
```cpp
template <typename TIn, typename TOut, uint8_t CHANNELS>
class CustomMapper;
```
- `TIn`: 输入数据类型
- `TOut`: 输出数据类型  
- `CHANNELS`: 通道数量

### 映射函数类型
```cpp
using MappingFunction = std::function<TOut(TIn input, uint8_t channel)>;
```
- `input`: 当前输入值
- `channel`: 通道索引（0到CHANNELS-1）
- 返回值: 映射后的输出值

### 配置结构
```cpp
struct Config {
    MappingFunction mappingFunc;  // 用户提供的映射函数
};
```

### 主要方法
```cpp
// 构造函数
CustomMapper(SyncPipeline<TIn>& upstream, const Config& config);

// 获取映射后的值
TOut getValue(uint8_t channel) const override;

// 管道控制
void update() override;
void reset() override;

// 配置管理
void updateConfig(const Config& config);
static bool isConfigValid(const Config& config);
```

## 使用方法

### 基本用法

```cpp
#include "wibotlib/src/SyncPipeline/mapper/custom-mapper.hpp"
using namespace wibot;

// 1. 创建数据源
TestDataSource<int16_t, 1> dataSource;

// 2. 定义映射函数
auto squareFunction = [](int16_t input, uint8_t channel) -> float {
    return static_cast<float>(input * input);
};

// 3. 配置映射器
CustomMapper<int16_t, float, 1>::Config config = {
    .mappingFunc = squareFunction
};

// 4. 创建映射器
CustomMapper<int16_t, float, 1> mapper(dataSource, config);

// 5. 使用映射器
dataSource.setValue(0, 5);
mapper.update();
float result = mapper.getValue(0);  // result = 25.0f
```

### 带内部限制的映射

```cpp
// 指数函数映射，在映射函数内部实现输出限制
auto clampedExpMapping = [](float input, uint8_t channel) -> float {
    float result = std::exp(input);
    // 在函数内部进行限制
    if (result > 10.0f) result = 10.0f;
    if (result < 0.0f) result = 0.0f;
    return result;
};

CustomMapper<float, float, 1>::Config config = {
    .mappingFunc = clampedExpMapping
};

CustomMapper<float, float, 1> mapper(dataSource, config);
```

### 多通道映射

```cpp
constexpr uint8_t CHANNELS = 3;

// 通道相关的映射函数
auto channelMapping = [](int input, uint8_t channel) -> float {
    switch (channel) {
        case 0: return input * 1.0f;        // CH0: 原值
        case 1: return input * 0.5f;        // CH1: 半值  
        case 2: return std::sqrt(input);    // CH2: 平方根
        default: return 0.0f;
    }
};

CustomMapper<int, float, CHANNELS>::Config config = {
    .mappingFunc = channelMapping
};

CustomMapper<int, float, CHANNELS> mapper(dataSource, config);
```

### 复杂数学函数

```cpp
// 组合数学函数
auto complexMapping = [](float input, uint8_t channel) -> double {
    if (channel == 0) {
        // 正弦波
        return std::sin(input * M_PI / 180.0);  // 角度转弧度
    } else {
        // Sigmoid函数
        return 1.0 / (1.0 + std::exp(-input));
    }
};

CustomMapper<float, double, 2>::Config config = {
    .mappingFunc = complexMapping
};
```

## 实际应用场景

### 1. 传感器数据处理
```cpp
// ADC值转换为物理量
auto adcToVoltage = [](uint16_t adcValue, uint8_t channel) -> float {
    const float vref = 3.3f;
    const uint16_t maxAdc = 4095;  // 12位ADC
    return (adcValue * vref) / maxAdc;
};
```

### 2. 信号调理
```cpp
// 非线性传感器校正
auto thermocouple = [](float voltage, uint8_t channel) -> float {
    // 热电偶非线性补偿（简化版）
    return voltage * 25.0f + voltage * voltage * 0.1f;
};
```

### 3. 数据归一化
```cpp
// 将不同范围的数据归一化到[0,1]
auto normalize = [](int16_t input, uint8_t channel) -> float {
    const int16_t channelMins[] = {-1000, -500, -2000};
    const int16_t channelMaxs[] = {1000, 500, 2000};
    
    float min = channelMins[channel];
    float max = channelMaxs[channel];
    return (input - min) / (max - min);
};
```

### 4. 滤波和平滑
```cpp
// 简单的死区滤波
auto deadband = [](float input, uint8_t channel) -> float {
    const float threshold = 0.1f;
    if (std::abs(input) < threshold) {
        return 0.0f;
    }
    return input > 0 ? input - threshold : input + threshold;
};
```

## 与LinearMapper的对比

| 特性       | LinearMapper    | CustomMapper       |
| ---------- | --------------- | ------------------ |
| 映射类型   | 固定线性映射    | 用户自定义任意函数 |
| 输入类型   | int16_t         | 泛型TIn            |
| 输出类型   | float           | 泛型TOut           |
| 配置复杂度 | 简单（4个参数） | 极简（仅1个函数）  |
| 灵活性     | 有限            | 极高               |
| 性能开销   | 最小            | 轻微（函数调用）   |
| 适用场景   | 简单线性变换    | 复杂非线性映射     |

## 性能考虑

1. **函数调用开销**: `std::function`有轻微的调用开销，但通常可以忽略
2. **内联优化**: lambda表达式通常会被编译器内联优化
3. **无状态设计**: 无内部状态，无额外的内存访问开销
4. **类型转换**: 注意输入输出类型的转换成本

## 最佳实践

### 1. 函数设计
- 保持映射函数简单高效
- 避免在映射函数中进行复杂计算
- 合理使用通道参数进行差异化处理

### 2. 类型选择
- 根据精度需求选择合适的输入输出类型
- 考虑数值范围和计算精度

### 3. 配置验证
- 始终验证配置的有效性
- 为无效配置提供合理的默认值

### 4. 错误处理
- 在映射函数中处理可能的异常输入
- 在映射函数内部实现范围控制，更灵活

## 扩展建议

1. **模板特化**: 为特定类型组合提供优化的特化版本
2. **并行处理**: 为多通道处理添加并行化支持
3. **缓存机制**: 为计算密集的函数添加结果缓存
4. **表查找**: 对于复杂函数可以考虑预计算查找表
5. **SIMD优化**: 利用向量指令加速数学运算

## 注意事项

1. **线程安全**: 当前实现不是线程安全的，多线程环境需要外部同步
2. **函数生命周期**: 确保映射函数在CustomMapper生命周期内有效
3. **异常安全**: 映射函数应该是无异常的或妥善处理异常
4. **数值稳定性**: 注意浮点数计算的精度和稳定性问题