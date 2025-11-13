# PiecewiseLinearMapper - 分段线性映射器（更新版本）

## 概述

`PiecewiseLinearMapper` 是一个多通道分段线性映射管道，用于将 `int16_t` 输入值按照分段线性函数映射到 `float` 输出值。所有通道共享同一套分段映射配置，支持实时无状态映射。

## 特性

- **多通道支持**：支持编译时指定的多个输入通道
- **运行时配置**：通过外部指针传入控制点数据，支持动态分段数量
- **共享配置**：所有通道使用相同的分段映射参数
- **分段线性映射**：通过多个控制点定义，相邻控制点间进行线性插值
- **边界处理**：支持输出钳位和外推两种边界处理模式
- **配置验证**：提供配置有效性检查
- **无状态设计**：实时映射，无内部状态缓存

## 模板参数

- `CHANNELS`：通道数量（编译时确定）

## 配置结构

```cpp
struct Config {
    const float* inputPoints;          // 输入控制点数组指针（必须按升序排列）
    const float* outputPoints;         // 输出控制点数组指针
    uint8_t      segmentCount;         // 分段数量（控制点数量为segmentCount+1）
    bool         clampOutput;          // 是否钳位输出到边界值范围
    bool         enableExtrapolation;  // 超出范围时是否允许外推
};
```

## 使用示例

### 基本用法

```cpp
#include "piecewise-linear-mapper.hpp"

// 创建4通道分段线性映射器
MockAdcSource<4> adcSource;

// 准备控制点数据（3段，4个控制点）
static const float inputPoints[] = {0.0f, 1000.0f, 2000.0f, 4095.0f};
static const float outputPoints[] = {0.0f, 1.2f, 2.8f, 3.3f};

// 配置分段映射参数
PiecewiseLinearMapper<4>::Config config;
config.inputPoints = inputPoints;        // 输入控制点数组指针
config.outputPoints = outputPoints;      // 输出控制点数组指针
config.segmentCount = 3;                 // 分段数量
config.clampOutput = true;               // 启用输出钳位
config.enableExtrapolation = false;      // 禁用外推

// 创建映射器
PiecewiseLinearMapper<4> mapper(adcSource, config);

// 使用
mapper.update();
float result = mapper.getValue(0);  // 获取通道0的映射结果
```

### 典型应用场景

#### 1. 非线性传感器线性化

```cpp
// ADC -> 温度映射（热敏电阻非线性补偿）
static const float tempInputPoints[] = {100.0f, 500.0f, 800.0f, 900.0f, 1000.0f};
static const float tempOutputPoints[] = {-10.0f, 20.0f, 50.0f, 70.0f, 80.0f};

PiecewiseLinearMapper<1>::Config tempConfig;
tempConfig.inputPoints = tempInputPoints;
tempConfig.outputPoints = tempOutputPoints;
tempConfig.segmentCount = 4;  // 4段，5个控制点
tempConfig.clampOutput = true;
tempConfig.enableExtrapolation = false;
```

#### 2. 电压分段调节

```cpp
// 电池电压 -> 百分比映射
static const float batteryInputPoints[] = {3000.0f, 3700.0f, 4200.0f};
static const float batteryOutputPoints[] = {0.0f, 50.0f, 100.0f};

PiecewiseLinearMapper<1>::Config batteryConfig;
batteryConfig.inputPoints = batteryInputPoints;
batteryConfig.outputPoints = batteryOutputPoints;
batteryConfig.segmentCount = 2;  // 2段，3个控制点
batteryConfig.clampOutput = true;
batteryConfig.enableExtrapolation = false;
```

## 算法原理

### 分段线性插值

对于输入值 `x`，算法执行以下步骤：

1. **查找分段**：确定 `x` 落在哪个分段 `[x_i, x_{i+1}]` 内
2. **线性插值**：使用公式计算输出值
   ```
   y = y_i + (y_{i+1} - y_i) * (x - x_i) / (x_{i+1} - x_i)
   ```

### 边界处理

- **输入超出下界** (`x < x_0`)：
  - 外推模式：使用第一段斜率外推
  - 钳位模式：返回 `y_0`
  
- **输入超出上界** (`x > x_n`)：
  - 外推模式：使用最后一段斜率外推
  - 钳位模式：返回 `y_n`

## API 参考

### 构造函数

```cpp
PiecewiseLinearMapper(SyncPipeline<int16_t>& upstream, const Config& config)
```

### 主要方法

```cpp
float getValue(uint8_t channel) const;           // 获取指定通道映射值
void update();                                   // 更新管道状态
void reset();                                    // 重置管道
void updateConfig(const Config& config);         // 更新映射配置
static bool isConfigValid(const Config& config); // 验证配置有效性
```

## 配置要求

1. **指针有效性**：`inputPoints` 和 `outputPoints` 不能为空指针
2. **控制点排序**：`inputPoints` 指向的数组必须严格按升序排列
3. **分段数量**：`segmentCount` 必须大于0，至少需要1个分段（2个控制点）
4. **数组大小**：输入和输出数组的大小必须至少为 `segmentCount + 1`
5. **数据类型**：输入为 `int16_t`，输出为 `float`

## 性能特点

- **时间复杂度**：O(segmentCount) - 线性查找分段
- **空间复杂度**：O(1) - 仅存储配置结构和指针
- **实时性**：无状态设计，支持实时映射
- **灵活性**：运行时配置，支持动态调整分段数量
- **内存占用**：控制点数据由用户管理，映射器本身占用最小

## 注意事项

1. 确保输入控制点按升序排列，否则映射结果未定义
2. 使用 `isConfigValid()` 验证配置有效性
3. 考虑输入数据范围，选择合适的边界处理模式
4. 对于高频更新场景，建议使用较少的分段数以优化性能

## 编译配置

项目中已包含常用通道数的模板显式实例化：

```cpp
template class PiecewiseLinearMapper<1>;    // 单通道
template class PiecewiseLinearMapper<2>;    // 双通道
template class PiecewiseLinearMapper<4>;    // 四通道
template class PiecewiseLinearMapper<8>;    // 八通道
template class PiecewiseLinearMapper<16>;   // 十六通道
```

如需其他通道数配置，请在 `.cpp` 文件中添加相应的模板实例化。

## 优势和注意事项

### 优势

1. **运行时灵活性**：可以动态调整分段数量和控制点
2. **内存效率**：控制点数据存储在用户空间，映射器本身占用最小
3. **易于配置**：简单的指针传递，支持静态和动态数据
4. **向前兼容**：保持与现有 SyncPipeline 接口的完全兼容

### 注意事项

1. **数据生命周期**：确保控制点数组在映射器使用期间保持有效
2. **内存对齐**：使用 `static const` 数组可确保数据在编译时分配
3. **线程安全**：如果多线程访问，需要确保控制点数据的同步
4. **配置验证**：始终使用 `isConfigValid()` 验证配置的正确性