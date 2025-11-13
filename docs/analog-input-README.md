# AnalogInput - 多通道ADC数据处理库

## 概述

`AnalogInput`是一个功能强大的C++模板类，用于处理多通道ADC（模数转换器）输入数据。它提供了滤波、映射、分桶、校准等高级数据处理功能，特别适用于嵌入式系统中的传感器数据处理。

**注意**: 从最新版本开始，AnalogInput采用统一配置设计，所有通道使用相同的处理参数，简化了配置管理并减少了内存占用。

## 主要特性

### 🔧 核心功能
- **多通道支持**: 支持1-32个ADC通道
- **统一配置**: 所有通道使用相同的处理参数，简化配置
- **多种滤波算法**: 移动平均、低通、中值、卡尔曼滤波
- **数据映射**: 线性、对数、指数映射
- **数据分桶**: 将连续数据离散化到指定区间
- **校准支持**: 偏移和增益校准
- **死区处理**: 避免小幅抖动
- **实时统计**: 最小值、最大值、平均值、RMS

### 📊 滤波器类型
- `FilterType::kNone` - 无滤波
- `FilterType::kMovingAverage` - 移动平均滤波
- `FilterType::kLowPass` - 低通滤波
- `FilterType::kMedian` - 中值滤波
- `FilterType::kKalman` - 卡尔曼滤波

### 🎯 映射模式
- `MappingMode::kLinear` - 线性映射
- `MappingMode::kLogarithmic` - 对数映射
- `MappingMode::kExponential` - 指数映射
- `MappingMode::kCustom` - 自定义映射函数

## 快速开始

### 基本使用

```cpp
#include "analog-input.hpp"

using namespace wibot;

// 创建4通道模拟输入
AnalogInput<4> analogInput;

// 配置
AnalogInput<4>::AnalogInputConfig config = {};
config.activeChannels   = 0x0F;        // 激活前4个通道
config.adcResolution    = 12;           // 12位ADC
config.referenceVoltage = 3.3f;         // 3.3V参考电压
config.sampleRate       = 1000;        // 1kHz采样率

// 设置所有通道的统一配置
config.channelConfig = {
    .filterType        = FilterType::kMovingAverage,
    .filterSize        = 8,
    .mappingMode       = MappingMode::kLinear,
    .inputMin          = 0.0f,
    .inputMax          = 3.3f,
    .outputMin         = 0.0f,
    .outputMax         = 100.0f,
    .calibrationOffset = 0.0f,
    .calibrationGain   = 1.0f,
    .enableDeadband    = false,
    // ... 其他参数
};

analogInput.configure(config);

// 主循环中使用
uint32_t rawValues[4] = {1024, 2048, 3072, 4095}; // ADC原始值
analogInput.update(rawValues);

// 获取处理后的数据
float voltage = analogInput.getVoltage(0);        // 电压值
float filtered = analogInput.getFilteredValue(0); // 滤波值
float mapped = analogInput.getMappedValue(0);     // 映射值
```

### 使用预定义配置

```cpp
// 使用BasicAnalogInput获得默认配置
BasicAnalogInput<4> basicInput;

uint32_t rawValues[4] = {1024, 2048, 3072, 4095};
basicInput.update(rawValues);

float voltage = basicInput.getVoltage(0);
```

## 高级功能

### 1. 滤波器配置

```cpp
// 设置所有通道使用低通滤波器
analogInput.setFilterType(FilterType::kLowPass);
ChannelConfig config;
analogInput.getChannelConfig(&config);
config.filterParameter = 0.1f; // 截止频率
analogInput.setChannelConfig(config);

// 设置所有通道使用中值滤波器
analogInput.setFilterType(FilterType::kMedian, 5); // 5点中值滤波
```

### 2. 校准

```cpp
// 自动校准（使用已知标准值）
analogInput.calibrateChannel(0, 1.0f, 100); // 通道0校准到1.0V，使用100个样本

// 手动设置校准参数（影响所有通道）
analogInput.setCalibration(0, -0.05f, 1.02f); // 偏移-50mV，增益1.02
```

### 3. 数据映射

```cpp
// 设置所有通道的映射范围
analogInput.setMappingRange(0.5f, 2.5f, -10.0f, 60.0f); // 0.5-2.5V映射到-10-60°C

// 配置对数映射（需要重新配置）
ChannelConfig config;
analogInput.getChannelConfig(&config);
config.mappingMode = MappingMode::kLogarithmic;
analogInput.setChannelConfig(config);
```

### 4. 分桶配置

```cpp
BinningConfig binConfig = {
    .binCount = 16,           // 16个分桶
    .minValue = 0.0f,
    .maxValue = 100.0f,
    .enableOverflow = true,
    .enableUnderflow = true
};

analogInput.setBinning(binConfig);
uint16_t binIndex = analogInput.getBinIndex(0); // 获取分桶索引
```

### 5. 统计信息

```cpp
float min, max, avg, rms;
analogInput.getStatistics(0, &min, &max, &avg, &rms);

// 重置统计
analogInput.resetStatistics(0);
```

## 应用示例

### 温度传感器处理

```cpp
// 配置所有通道为温度传感器
AnalogInput<4>::AnalogInputConfig config = {};
config.activeChannels   = 0x0F;
config.adcResolution    = 12;
config.referenceVoltage = 3.3f;
config.sampleRate       = 1000;

config.channelConfig = {
    .filterType        = FilterType::kMovingAverage,
    .filterSize        = 10,              // 10点平均降噪
    .mappingMode       = MappingMode::kLinear,
    .inputMin          = 0.5f,            // 对应-10°C
    .inputMax          = 2.5f,            // 对应+60°C
    .outputMin         = -10.0f,
    .outputMax         = 60.0f,
    .calibrationOffset = -2.5f,           // 传感器校准
    .calibrationGain   = 1.02f,
    .enableDeadband    = true,            // 避免小幅抖动
    .deadbandCenter    = 25.0f,           // 室温死区中心
    .deadbandWidth     = 1.0f             // ±0.5°C死区
};

analogInput.configure(config);
```

### 压力传感器处理

```cpp
// 配置所有通道为压力传感器
config.channelConfig = {
    .filterType        = FilterType::kLowPass,
    .filterParameter   = 0.15f,           // 低通滤波截止频率
    .mappingMode       = MappingMode::kLinear,
    .inputMin          = 0.2f,            // 对应0 PSI
    .inputMax          = 3.0f,            // 对应100 PSI
    .outputMin         = 0.0f,
    .outputMax         = 100.0f,
    .calibrationGain   = 0.99f            // 1%校准
};
```

## 设计优势

### 统一配置的好处
- **简化配置**: 只需配置一次，所有通道使用相同参数
- **减少内存**: 不需要为每个通道单独存储配置
- **一致性**: 确保所有通道的处理方式完全一致
- **易于维护**: 配置变更只需修改一处

## 性能考虑

- **内存使用**: 每个滤波器通道根据配置的滤波器大小动态分配内存
- **计算复杂度**: 
  - 移动平均: O(1) 均摊复杂度
  - 中值滤波: O(n log n) 每次更新
  - 低通/卡尔曼: O(1)
- **实时性**: 所有操作都设计为实时处理，适合中断服务程序调用

## 编译要求

- C++11或更高版本
- STL支持（algorithm, cmath, cstring）
- 支持模板特化

## 文件结构

```
hal/
├── analog-input.hpp           # 主头文件
├── analog-input.cpp           # 实现文件
├── analog-input-example.hpp   # 使用示例头文件
├── analog-input-example.cpp   # 使用示例实现
└── README.md                  # 本文档
```

## 线程安全

当前实现不是线程安全的。如果需要在多线程环境中使用，请确保：
1. 每个线程使用独立的AnalogInput实例，或
2. 使用适当的同步机制保护共享实例

## 扩展性

类设计支持以下扩展：
- 自定义滤波算法
- 自定义映射函数
- 额外的统计功能
- 数据记录和回放

## 许可证

遵循项目整体许可证。

---

*参考DigitalInput的设计风格，AnalogInput提供了类似的API接口和配置模式，便于在同一项目中保持一致的编程体验。*