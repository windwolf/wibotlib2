# 一阶低通滤波器 (LowpassFilter)

## 概述

`LowpassFilter` 是一个基于管道架构的一阶低通滤波器实现，支持多通道实时滤波处理。它采用一阶RC滤波器模型，能够有效去除高频噪声，同时保持信号的主要特征。

## 特性

- **多通道支持**: 编译时确定通道数量，支持1-N个通道同时滤波
- **实时处理**: 基于管道架构，支持实时数据流处理
- **可配置参数**: 支持自定义采样时间、截止频率等参数
- **角度滤波**: 特别支持角度等周期性数据的滤波（折叠值功能）
- **状态管理**: 各通道独立维护滤波状态
- **参数验证**: 内置配置参数有效性检查

## 滤波算法

采用一阶数字低通滤波器：

```
y[n] = α * x[n] + (1-α) * y[n-1]
```

其中：
- `α = 2πfc*T / (1 + 2πfc*T)`
- `fc` 是截止频率（Hz）
- `T` 是采样间隔（秒）

## 使用方法

### 基本用法

```cpp
#include "lowpass-filter.hpp"

// 创建数据源
TestDataSource<1> dataSource;

// 配置滤波器
LowpassFilter<1>::Config config = {
    .sampleTime = 0.01f,   // 10ms采样间隔 (100Hz)
    .cutoffFreq = 5.0f,    // 5Hz截止频率
    .wrapValue = 0.0f,     // 不启用折叠
    .initValue = 0.0f      // 初始值为0
};

// 创建滤波器
LowpassFilter<1> filter(dataSource, config);

// 处理数据
dataSource.setValue(0, inputValue);
filter.update();
float filteredValue = filter.getValue(0);
```

### 角度滤波

对于角度等周期性数据，可以启用折叠值功能：

```cpp
LowpassFilter<1>::Config angleConfig = {
    .sampleTime = 0.01f,
    .cutoffFreq = 2.0f,
    .wrapValue = 2.0f * M_PI,  // 2π弧度折叠（360度）
    .initValue = 0.0f
};

LowpassFilter<1> angleFilter(angleSource, angleConfig);
```

### 多通道滤波

```cpp
// 创建3通道滤波器
LowpassFilter<3> multiFilter(multiSource, config);

// 处理多通道数据
multiSource.setValue(0, ch0_value);
multiSource.setValue(1, ch1_value);
multiSource.setValue(2, ch2_value);

multiFilter.update();

float ch0_filtered = multiFilter.getValue(0);
float ch1_filtered = multiFilter.getValue(1);
float ch2_filtered = multiFilter.getValue(2);
```

## 配置参数

### Config 结构体

| 参数         | 类型  | 描述                                     |
| ------------ | ----- | ---------------------------------------- |
| `sampleTime` | float | 采样间隔（秒），必须 > 0                 |
| `cutoffFreq` | float | 截止频率（Hz），必须 > 0 且 < 采样频率/2 |
| `wrapValue`  | float | 折叠值，用于周期性数据，0表示不启用      |
| `initValue`  | float | 初始输出值                               |

### 参数限制

1. **采样时间**: 必须大于0
2. **截止频率**: 必须大于0且小于奈奎斯特频率（采样频率的一半）
3. **折叠值**: 如果启用，必须大于0

## 主要方法

### 核心接口

- `LowpassFilter(upstream, config)`: 构造函数
- `update()`: 更新滤波器状态
- `getValue(channel)`: 获取指定通道的滤波输出
- `reset()`: 重置滤波器状态

### 配置管理

- `updateConfig(config)`: 更新滤波配置
- `isConfigValid(config)`: 验证配置有效性
- `setInitValue(channel, value)`: 设置指定通道的初始值

## 示例代码

完整的使用示例请参考 `lowpass-filter-example.cpp`，包含：

1. **基本滤波示例**: 演示基本的低通滤波功能
2. **角度滤波示例**: 演示折叠值功能处理角度跳跃
3. **多通道滤波示例**: 演示多通道同时滤波

## 性能特点

- **编译时优化**: 通道数在编译时确定，无运行时开销
- **内存效率**: 每通道仅需存储一个float状态值
- **实时性能**: 算法复杂度O(1)，适合实时应用

## 应用场景

1. **传感器数据滤波**: 去除ADC采样噪声
2. **信号预处理**: 作为数据处理管道的一部分
3. **角度平滑**: IMU姿态角度滤波
4. **控制系统**: 反馈信号平滑处理

## 注意事项

1. 截止频率的选择需要在滤波效果和响应速度之间平衡
2. 角度滤波时要正确设置折叠值（通常是2π）
3. 多通道滤波器所有通道共享相同的滤波配置
4. 滤波器会引入相位延迟，需要考虑对系统的影响