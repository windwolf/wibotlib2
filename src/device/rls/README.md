# RLS 编码器组件

本目录包含了用于 RLS 绝对式编码器的组件实现。

## 组件概述

### 1. PositionTracker（位置跟踪器）

**功能**：根据编码器原始值计算累积位置，处理折叠（wrap）问题。

**特性**：
- 处理编码器值的环绕（wrap-around）
- 累积位置展开
- 支持任意分辨率的编码器
- 提供角度换算

**使用示例**：
```cpp
#include "position-tracker.hpp"

// 配置
PositionTracker::Config config{
    .resolution = 4096,      // 编码器分辨率 (ticks/rev)
    .inputWrapRange = 4096   // 输入折叠范围
};

// 创建位置跟踪器
PositionTracker tracker(config);

// 初始化（可选）
tracker.reset(0, 0);  // 当前编码器读数=0, 初始位置=0

// 循环中更新
while (true) {
    u32 encoderValue = readEncoder();  // 读取编码器原始值
    tracker.update(encoderValue);
    
    i32 position = tracker.getPosition();      // 获取累积位置 (ticks)
    f32 angle = tracker.getAngular();          // 获取角度 (弧度)
    i32 displacement = tracker.getLastDisplacement(); // 最近一次位移
}
```

### 2. VelocityEstimator（速度估计器）

**功能**：根据位移信息计算速度，使用IIR低通滤波器平滑输出。

**特性**：
- 简洁的单滤波器架构
- 支持固定或可变采样周期
- 由调用者负责位移累计
- 提供角速度换算

**使用示例**：
```cpp
#include "velocity-estimator.hpp"

// 配置
VelocityEstimator::Config config{
    .resolution = 4096,           // 编码器分辨率
    .samplePeriod = 0.001f,       // 采样周期 (s)
    .trackingCycles = 5.0f        // 追踪周期数（滤波器时间常数）
};

// 创建速度估计器
VelocityEstimator estimator(config);

// 初始化
estimator.reset(0.0f);  // 初始速度=0

// 循环中更新 - 方式1：每次都调用（高速场景）
while (true) {
    i32 displacement = getDisplacement();  // 从位置跟踪器获取位移
    estimator.update(displacement);
    
    f32 speed = estimator.getSpeed();           // 获取速度 (ticks/s)
    f32 angularSpeed = estimator.getAngularSpeed(); // 获取角速度 (rad/s)
}

// 循环中更新 - 方式2：累计到足够时才调用（低速场景）
i32 accumulatedDisplacement = 0;
f32 accumulatedTime = 0.0f;
const i32 MIN_DISPLACEMENT = 5;  // 最小累计位移

while (true) {
    i32 displacement = getDisplacement();
    f32 dt = getSamplePeriod();
    
    accumulatedDisplacement += displacement;
    accumulatedTime += dt;
    
    // 当累计位移足够时才更新速度估计
    if (std::abs(accumulatedDisplacement) >= MIN_DISPLACEMENT) {
        estimator.update(accumulatedDisplacement, accumulatedTime);
        accumulatedDisplacement = 0;
        accumulatedTime = 0.0f;
    }
    
    f32 speed = estimator.getSpeed();
}
```

### 3. AbsoluteEncoder（绝对式编码器）

**功能**：组合 PositionTracker 和 VelocityEstimator，提供完整的位置和速度解决方案。

**特性**：
- 位置始终实时更新（不受阈值影响）
- 速度采用位移累计策略（低于阈值时不更新）
- 内部自动管理两个子组件和位移累计
- 统一的配置接口
- 可以访问子组件进行高级控制

**工作原理**：
- **位置**：每次调用 `update()` 都会更新，累积编码器的所有位移
- **速度**：只有当累计位移达到 `minDisplacementThreshold` 时才会更新
  - 低于阈值时：继续累计，保持前一次的速度输出不变
  - 达到阈值时：将累计的位移和时间传给速度估计器，更新输出速度，然后重置累计器

**使用示例**：
```cpp
#include "absolute-encoder.hpp"

// 配置
AbsoluteEncoder::Config config{
    .resolution = 4096,
    .inputWrapRange = 4096,
    .maxSpeed = 50000.0f,
    .minSpeed = 100.0f,
    .samplePeriod = 0.001f,
    .trackingCycles = 5.0f,
    .minDisplacementThreshold = 5  // 位移阈值：5 ticks
};

// 创建编码器
AbsoluteEncoder encoder(config);

// 初始化
encoder.reset(0, 0, 0.0f);  // 编码器值=0, 位置=0, 速度=0

// 循环中更新
while (true) {
    u32 encoderValue = readEncoder();
    encoder.update(encoderValue);
    
    // 获取信息
    i32 position = encoder.getPosition();         // 位置 (ticks) - 实时更新
    f32 speed = encoder.getSpeed();               // 速度 (ticks/s) - 累计足够时更新
    f32 angle = encoder.getAngular();             // 角度 (rad)
    f32 angularSpeed = encoder.getAngularSpeed(); // 角速度 (rad/s)
}

// 访问子组件（高级用法）
PositionTracker& posTracker = encoder.getPositionTracker();
VelocityEstimator& velEstimator = encoder.getVelocityEstimator();
```

## 组件选择指南

### 使用 AbsoluteEncoder（推荐）
适用于大多数场景，提供完整的位置和速度功能，并内置位移累计策略以提高低速精度。

### 使用 PositionTracker
当你只需要位置信息，不需要速度估计时。

### 使用 VelocityEstimator
当你已经有其他方式获取位移信息，只需要速度估计时。需要外部手动管理位移累计。

## 组合使用示例

```cpp
// 独立使用两个组件（需要手动处理位移累计）
PositionTracker::Config posConfig{4096, 4096};
VelocityEstimator::Config velConfig{4096, 0.001f, 5.0f};

PositionTracker posTracker(posConfig);
VelocityEstimator velEstimator(velConfig);

// 低速场景：累计位移
i32 accumulatedDisplacement = 0;
f32 accumulatedTime = 0.0f;
const i32 MIN_DISPLACEMENT = 5;

while (true) {
    // 步骤1: 更新位置
    u32 encoderValue = readEncoder();
    posTracker.update(encoderValue);
    
    // 步骤2: 累计位移
    i32 displacement = posTracker.getLastDisplacement();
    f32 dt = getSamplePeriod();
    
    accumulatedDisplacement += displacement;
    accumulatedTime += dt;
    
    // 步骤3: 当累计够了才更新速度
    if (std::abs(accumulatedDisplacement) >= MIN_DISPLACEMENT) {
        velEstimator.update(accumulatedDisplacement, accumulatedTime);
        accumulatedDisplacement = 0;
        accumulatedTime = 0.0f;
    }
    
    // 获取结果
    i32 position = posTracker.getPosition();
    f32 speed = velEstimator.getSpeed();
}
```

## 设计原则

1. **单一职责**：每个组件只负责一个功能
2. **可组合性**：组件可以独立使用或组合使用
3. **向后兼容**：AbsoluteEncoder 保持原有接口
4. **性能优化**：
   - 位置始终实时更新
   - 速度采用累计策略，减少不必要的运算
   - 低速时不更新速度估计，保持稳定输出

## 技术细节

### 位置跟踪算法
- 使用无符号数运算处理环绕
- 智能检测边界跨越
- 支持任意折叠范围

### 速度估计算法
- **单一滤波器**：统一使用IIR低通滤波器
- **简洁设计**：外部负责位移累计，内部只负责速度平滑
- **参数配置**：通过 `trackingCycles` 调整滤波器响应速度

### 配置参数说明

**trackingCycles**：追踪周期数，影响滤波器的响应速度和平滑度。
- 值越大：越平滑，但响应越慢
- 推荐值：3-10
- 典型值：5（平衡响应速度和平滑度）
