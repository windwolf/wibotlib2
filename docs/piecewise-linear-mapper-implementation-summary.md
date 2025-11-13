# 分段线性映射器实现总结

## 实现概述

基于现有 `LinearMapper` 的设计模式，成功实现了 `PiecewiseLinearMapper` 分段线性映射器，满足所有设计要求。

## 设计要求完成情况

### ✅ 1. 多通道支持
- 通过模板参数 `CHANNELS` 指定通道数量（编译时确定）
- 支持任意数量的通道：1, 2, 4, 8 等
- 所有通道独立处理，但共享同一套映射配置

### ✅ 2. 多通道共享配置
- 所有通道使用相同的 `Config` 结构体
- 通过 `updateConfig()` 方法统一更新所有通道的映射参数
- 避免了重复存储，节省内存空间

### ✅ 3. 分段映射，参数可配置
- 支持任意数量的分段（通过模板参数 `SEGMENTS` 指定）
- 分段数为 N，则有 N+1 个控制点
- 灵活的配置结构：
  - `inputPoints[]`: 输入控制点数组
  - `outputPoints[]`: 输出控制点数组
  - `clampOutput`: 输出钳位开关
  - `enableExtrapolation`: 外推功能开关

## 核心算法特性

### 分段线性插值
- 使用经典的线性插值公式：`y = y₀ + (y₁ - y₀) × (x - x₀) / (x₁ - x₀)`
- 自动查找输入值所属的分段区间
- 时间复杂度：O(SEGMENTS)

### 边界处理
- **钳位模式**：超出范围时返回边界值
- **外推模式**：使用端点分段的斜率进行线性外推
- 用户可根据应用场景选择合适的边界处理策略

### 配置验证
- 提供 `isConfigValid()` 静态方法验证配置
- 检查输入控制点是否按升序排列
- 防止运行时错误

## 文件结构

```
wibotlib/src/SyncPipeline/mapper/
├── piecewise-linear-mapper.hpp          # 头文件（接口定义）
├── piecewise-linear-mapper.cpp          # 实现文件
├── piecewise-linear-mapper-example.cpp  # 使用示例
└── test-piecewise-linear-mapper.cpp     # 单元测试

wibotlib/docs/
└── piecewise-linear-mapper-README.md    # 详细文档
```

## 设计亮点

### 1. 模板化设计
- 编译时确定通道数和分段数
- 零运行时开销的类型安全
- 支持编译器优化

### 2. 无状态设计
- 继承 LinearMapper 的实时映射理念
- 无内部缓存，避免状态同步问题
- 适合高频实时应用

### 3. 兼容性设计
- 完全符合 SyncPipeline 接口规范
- 与现有 LinearMapper 风格保持一致
- 可与其他管道组件无缝集成

### 4. 健壮性设计
- 输入验证和边界保护
- 配置有效性检查
- 异常输入的安全处理

## 典型应用场景

### 传感器线性化
```cpp
// 温度传感器非线性补偿
PiecewiseLinearMapper<1, 4> tempMapper(adcSource, tempConfig);
```

### 电压分段映射  
```cpp
// 电池电量百分比映射
PiecewiseLinearMapper<1, 2> batteryMapper(voltageSource, batteryConfig);
```

### 多通道同时处理
```cpp
// 4通道ADC同时映射
PiecewiseLinearMapper<4, 3> multiMapper(adcSource, sharedConfig);
```

## 性能特点

- **内存占用**：编译时确定，无动态分配
- **计算效率**：O(SEGMENTS) 查找 + O(1) 插值
- **实时性能**：无状态缓存，支持高频更新
- **类型安全**：模板编译时检查

## 扩展性

### 模板实例化
已提供常用配置的显式实例化：
- 单通道：1-5 分段
- 多通道：2, 4, 8 通道
- 可根据需要添加更多配置

### 算法优化
- 可扩展为二分查找（对于大量分段）
- 支持预计算斜率优化
- 可添加更多边界处理模式

## 测试覆盖

- ✅ 基本线性插值功能
- ✅ 边界值处理
- ✅ 外推功能
- ✅ 多通道独立性
- ✅ 配置验证
- ✅ 异常输入处理

## 总结

成功实现了一个功能完整、性能优良、易于使用的分段线性映射器。该实现不仅满足了所有设计要求，还提供了良好的扩展性和健壮性，可以广泛应用于各种传感器数据处理和信号调理场景。