# ConstantSource 使用指南

## 概述

`ConstantSource` 是一个生成常量值的多通道数据源，继承自 `SyncPipeline<T>` 接口。它允许为每个通道设置不同的常量值，这些值可以在运行时动态修改。支持多种数据类型的模板化。

## 特性

- **类型模板化**: 通过模板参数 `T` 支持不同的数据类型（i16, u16, i32, f32等）
- **多通道支持**: 通过模板参数 `CHANNELS` 指定通道数量
- **可配置常量值**: 每个通道可以设置独立的常量值
- **运行时修改**: 可以在程序运行过程中动态修改常量值
- **类型安全**: 使用模板确保类型安全和与其他管道组件的兼容性
- **边界检查**: 自动处理无效的通道索引

## 基本用法

### 1. 创建 ConstantSource 实例

```cpp
#include "constant-source.hpp"

// 创建4通道的i16类型常量源，所有通道初始值为默认构造值
ConstantSource<i16, 4> intConstantSrc;

// 创建4通道的i16类型常量源，所有通道初始值为100
ConstantSource<i16, 4> intConstantSrcWithDefault(100);

// 创建2通道的f32类型常量源，所有通道初始值为3.14f
ConstantSource<f32, 2> floatConstantSrc(3.14f);

// 创建8通道的u16类型常量源
ConstantSource<u16, 8> unsignedConstantSrc(1000);
```

### 2. 设置常量值

```cpp
// 对于i16类型的常量源
ConstantSource<i16, 4> intSrc;

// 设置单个通道的值
intSrc.setValue(0, 200);  // 通道0设置为200
intSrc.setValue(1, 300);  // 通道1设置为300

// 设置所有通道为相同值
intSrc.setAllValues(500);

// 使用数组设置多个通道的值
i16 values[4] = {10, 20, 30, 40};
intSrc.setValues(values);

// 对于浮点数类型的常量源
ConstantSource<f32, 2> floatSrc;
floatSrc.setValue(0, 3.14f);
floatSrc.setValue(1, 2.71f);
floatSrc.setAllValues(1.0f);

f32 floatValues[2] = {1.23f, 4.56f};
floatSrc.setValues(floatValues);
```

### 3. 读取值

```cpp
ConstantSource<i16, 4> intSrc(100);

// 读取单个通道的值
i16 value = intSrc.getValue(0);

// 读取指定通道的常量值（与getValue相同）
i16 constValue = intSrc.getConstantValue(0);

// 获取所有通道值的指针
i16* allValues = intSrc.getValues();
for (u8 ch = 0; ch < 4; ch++) {
    printf("通道 %d: %d\n", ch, allValues[ch]);
}

// 对于不同数据类型
ConstantSource<f32, 2> floatSrc(3.14f);
f32 floatValue = floatSrc.getValue(0);
f32* allFloatValues = floatSrc.getValues();
```

### 4. 管道操作

```cpp
ConstantSource<i16, 4> intSrc;

// 更新管道（对于常量源，这个操作不执行任何操作）
intSrc.update();

// 重置所有通道为默认构造的值（通常为0）
intSrc.reset();

// 获取通道数量
constexpr u8 channelCount = ConstantSource<i16, 4>::getChannelCount();
```

## 支持的数据类型

`ConstantSource` 支持以下预实例化的数据类型：

- **i16**: 16位有符号整数 (-32768 到 32767)
- **u16**: 16位无符号整数 (0 到 65535)  
- **i32**: 32位有符号整数 (-2147483648 到 2147483647)
- **f32**: 32位单精度浮点数

支持的通道数量：1, 2, 4, 8, 16

### 使用示例

```cpp
// 16位有符号整数，4通道
ConstantSource<i16, 4> signedIntSrc(-1000);

// 16位无符号整数，2通道  
ConstantSource<u16, 2> unsignedIntSrc(65000);

// 32位整数，8通道
ConstantSource<i32, 8> bigIntSrc(1000000);

// 32位浮点数，1通道
ConstantSource<f32, 1> floatSrc(3.14159f);
```

## 应用场景

### 1. 测试和调试

在开发阶段，可以使用 `ConstantSource` 替代真实的传感器数据源进行测试：

```cpp
// 模拟4个传感器的固定输出
ConstantSource<i16, 4> mockSensors;
mockSensors.setValue(0, 1024);  // 模拟传感器1输出
mockSensors.setValue(1, 2048);  // 模拟传感器2输出
mockSensors.setValue(2, 3072);  // 模拟传感器3输出
mockSensors.setValue(3, 4096);  // 模拟传感器4输出

// 模拟浮点数传感器输出
ConstantSource<f32, 2> mockTempSensors;
mockTempSensors.setValue(0, 25.5f);  // 模拟温度传感器1: 25.5°C
mockTempSensors.setValue(1, 23.8f);  // 模拟温度传感器2: 23.8°C
```

### 2. 默认值提供

为系统提供默认的参考值：

```cpp
// 为控制系统提供默认目标值
ConstantSource<i16, 2> defaultTargets(1000);  // 默认目标值1000

// 为浮点数控制系统提供默认参考
ConstantSource<f32, 4> defaultReferences(1.0f);
```

### 3. 配置参数源

作为可配置参数的数据源：

```cpp
// 存储校准参数
ConstantSource<i16, 8> calibrationParams;
// 可以通过外部接口动态修改这些参数

// 存储浮点数配置参数
ConstantSource<f32, 4> configParams;
configParams.setValue(0, 0.5f);   // 增益系数
configParams.setValue(1, 10.0f);  // 比例系数
configParams.setValue(2, 0.1f);   // 积分系数
configParams.setValue(3, 0.01f);  // 微分系数
```

## 与其他管道组件结合

`ConstantSource` 可以作为其他管道组件的输入源：

```cpp
// 创建常量源
ConstantSource<i16, 2> constantSrc(500);

// 可以连接到滤波器、映射器等其他管道组件
// 例如：LowpassFilter<i16, 2> filter(&constantSrc, config);

// 浮点数管道示例
ConstantSource<f32, 4> floatConstantSrc(1.0f);
// 可以连接到浮点数处理管道
```

## 注意事项

1. **模板参数**: 第一个模板参数是数据类型，第二个是通道数量
2. **通道索引**: 通道索引从0开始，范围为 [0, CHANNELS-1]
3. **边界检查**: 访问无效通道索引会返回默认构造的值，设置无效通道索引会被忽略
4. **内存效率**: 每个实例只占用 `CHANNELS * sizeof(T)` 字节的内存
5. **类型安全**: 所有操作都是类型安全的，编译时检查类型匹配
6. **线程安全**: 当前实现不提供线程安全保证，如需在多线程环境使用请添加适当的同步机制

## API 参考

### 构造函数
- `ConstantSource(T defaultValue = T{})`: 创建实例，可选择设置默认值

### 管道接口
- `void update()`: 更新管道（空操作）
- `T getValue(u8 channel)`: 获取指定通道的值
- `T* getValues()`: 获取所有通道值的指针
- `void reset()`: 重置所有通道为默认构造的值

### 配置接口
- `void setValue(u8 channel, T value)`: 设置单个通道的值
- `void setAllValues(T value)`: 设置所有通道为相同值
- `void setValues(const T values[])`: 使用数组设置多个通道的值
- `T getConstantValue(u8 channel)`: 获取指定通道的常量值

### 静态接口
- `static constexpr u8 getChannelCount()`: 获取通道数量

### 模板参数
- `T`: 数据类型（如 i16, u16, i32, f32）
- `CHANNELS`: 通道数量（如 1, 2, 4, 8, 16）