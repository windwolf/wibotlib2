# PID控制器管道 (PidController)

## 概述

`PidController` 是一个遵循 `SyncPipeline` 接口的PID控制器实现，它可以从上游管道获取测量值并输出控制量。该实现支持串行和并行两种PID形式，并提供了完整的参数配置和状态管理功能。

## 特性

- 🔄 **管道接口兼容**: 实现了 `SyncPipeline<f32, f32*>` 接口，可以与其他管道组件无缝集成
- 📊 **多通道支持**: 支持1-4通道同时控制，每通道独立状态但共享配置参数
- ⚙️ **双模式支持**: 支持串行(Serial)和并行(Parallel)两种PID算法形式
- 🎛️ **完整参数配置**: 支持P、I、D增益，微分滤波，输出限制，积分限制等
- 🔒 **抗饱和保护**: 内置积分抗饱和和输出限制功能
- 📊 **高精度计算**: 使用改进的数值算法提高控制精度
- 🔄 **状态重置**: 支持运行时重置内部状态

## 使用方法

### 基本使用（双通道示例）

```cpp
#include "pid-controller.hpp"

using namespace wibot;

// 创建传感器数据源管道 (假设已实现)
SensorPipeline<f32, f32*> sensorPipeline;

// 创建双通道PID控制器管道
PidController<2> pidController(&sensorPipeline);

// 配置PID参数（所有通道共享）
PidControllerConfig config;
config.mode = PidControllerMode::kParallel;
config.Kp = 2.0f;      // 比例增益
config.Ki = 0.5f;      // 积分增益  
config.Kd = 0.1f;      // 微分增益
config.tau = 0.02f;    // 微分滤波时间常数
config.sampleTime = 0.01f; // 采样时间 (10ms)
config.setPoint = 50.0f;   // 目标设定值（所有通道共享）

// 启用输出限制
config.outputLimitEnable = true;
config.outputLimitMax = 100.0f;
config.outputLimitMin = -100.0f;

// 启用积分限制 (防积分饱和)
config.integratorLimitEnable = true;
config.integratorLimitMax = 50.0f;
config.integratorLimitMin = -50.0f;

pidController.setConfig(config);

// 控制循环
while (true) {
    // 更新PID控制器（同时处理所有通道）
    pidController.update();
    
    // 获取单个通道的控制输出
    f32 controlOutput0 = pidController.getValue(0);  // 通道0
    f32 controlOutput1 = pidController.getValue(1);  // 通道1
    
    // 或者获取所有通道的控制输出
    f32* allOutputs = pidController.getValues();
    
    // 将控制输出应用到执行器
    actuator.setOutput(0, controlOutput0);
    actuator.setOutput(1, controlOutput1);
    
    // 等待下一个采样周期
    delay(10); // 10ms
}
```

### 动态调整参数

```cpp
// 运行时修改设定值
pidController.setSetPoint(75.0f);

// 运行时修改完整配置
PidControllerConfig newConfig = pidController.getConfig();
newConfig.Kp = 3.0f;  // 增加比例增益
pidController.setConfig(newConfig);

// 重置控制器状态 (清除积分、微分历史)
pidController.reset();
```

### 管道链式组合

```cpp
// 创建完整的控制管道链
AdcSourcePipeline adcSource;           // ADC数据源
LowPassFilterPipeline filter(&adcSource);    // 低通滤波器
PidController pidCtrl(&filter);      // PID控制器
DacOutputPipeline dacOutput(&pidCtrl);       // DAC输出

// 配置PID参数
PidControllerConfig config;
config.setPoint = 2.5f;  // 目标电压2.5V
// ... 其他配置
pidCtrl.setConfig(config);

// 更新整个管道 (从数据源到输出)
dacOutput.update();
```

## 配置参数说明

### PidControllerConfig 结构体

| 参数                     | 类型                | 说明                           |
| ------------------------ | ------------------- | ------------------------------ |
| `mode`                   | `PidControllerMode` | 控制器模式 (kSerial/kParallel) |
| `Kp`                     | `f32`               | 比例增益                       |
| `Ki`                     | `f32`               | 积分增益                       |
| `Kd`                     | `f32`               | 微分增益                       |
| `tau`                    | `f32`               | 微分低通滤波时间常数           |
| `outputLimitEnable`      | `bool`              | 输出限制使能                   |
| `outputLimitMax/Min`     | `f32`               | 输出限制最大/最小值            |
| `integratorLimitEnable`  | `bool`              | 积分限制使能                   |
| `integratorLimitMax/Min` | `f32`               | 积分限制最大/最小值            |
| `sampleTime`             | `f32`               | 采样时间 (秒)                  |
| `setPoint`               | `f32`               | 目标设定值                     |

### 控制器模式

- **串行模式 (kSerial)**: 传统PID形式，适用于大多数应用
- **并行模式 (kParallel)**: 独立调节各项，更易于参数整定

## 算法细节

### 数值稳定性改进

1. **积分计算**: 使用梯形积分法提高精度
2. **微分计算**: 基于测量值微分，避免设定值突变干扰
3. **微分滤波**: 内置低通滤波器抑制高频噪声
4. **抗饱和保护**: 独立的积分限制和输出限制

### 数学公式

**串行PID**:
```
u(k) = Kp * [e(k) + (1/Ti) * ∫e(t)dt + Td * de(t)/dt]
```

**并行PID**:
```
u(k) = Kp*e(k) + Ki*∫e(t)dt + Kd*de(t)/dt
```

其中:
- `e(k) = setPoint - measurement`: 控制误差
- 微分项基于测量值: `de(t)/dt ≈ -(measurement(k) - measurement(k-1))/dt`

## 注意事项

1. **采样时间**: 确保 `sampleTime` 与实际调用 `update()` 的频率一致
2. **微分滤波**: `tau` 参数通常设置为采样时间的2-20倍
3. **积分限制**: 建议启用积分限制防止积分饱和，特别是在有输出限制时
4. **参数整定**: 建议从 Kp 开始逐步调节，再加入 Ki 和 Kd
5. **上游依赖**: 确保上游管道在使用前已正确初始化

## 多通道支持

模板类支持不同通道数的配置:

```cpp
// 单通道PID控制器
PidController<1> pidSingle;

// 双通道PID控制器  
PidController<2> pidDual;

// 三通道PID控制器
PidController<3> pidTriple;

// 四通道PID控制器
PidController<4> pidQuad;

// 使用类型别名
PidController1Ch pidController1;  // 单通道
PidController2Ch pidController2;  // 双通道
PidController3Ch pidController3;  // 三通道
PidController4Ch pidController4;  // 四通道
```

### 多通道特性

1. **独立状态**: 每个通道有独立的积分器、微分器和历史状态
2. **共享配置**: 所有通道共享同一套PID参数配置（Kp、Ki、Kd等）
3. **同步更新**: 调用`update()`时同时处理所有通道
4. **灵活访问**: 可以单独获取某个通道的输出，或批量获取所有通道输出

## 示例代码

完整的使用示例请参考 `pid-controller-example.hpp` 文件，包含：
- 基本多通道使用
- 串行vs并行模式
- 动态调整设定值
- 状态重置
- 多通道独立控制示例