# 分段线性映射器实现总结（更新版本）

## 修改概述

根据用户要求，将 `SEGMENTS` 从模板参数改为运行时配置，通过外部指针传入控制点数据。这个修改大大提高了系统的灵活性和实用性。

## 主要改动

### 1. 模板参数简化
**之前**：
```cpp
template <uint8_t CHANNELS, uint8_t SEGMENTS>
class PiecewiseLinearMapper
```

**之后**：
```cpp
template <uint8_t CHANNELS>
class PiecewiseLinearMapper
```

### 2. 配置结构重新设计
**之前**（编译时固定数组）：
```cpp
struct Config {
    float inputPoints[SEGMENTS + 1];   // 编译时确定大小
    float outputPoints[SEGMENTS + 1];
    bool  clampOutput;
    bool  enableExtrapolation;
};
```

**之后**（运行时指针配置）：
```cpp
struct Config {
    const float* inputPoints;          // 外部数组指针
    const float* outputPoints;         // 外部数组指针
    uint8_t      segmentCount;         // 运行时分段数量
    bool         clampOutput;
    bool         enableExtrapolation;
};
```

### 3. 算法适配
- 所有涉及 `SEGMENTS` 的地方改为使用 `_config.segmentCount`
- 增加了空指针和零分段的验证逻辑
- 保持了原有的分段查找和线性插值算法

## 设计优势

### ✅ 运行时灵活性
- 可以在运行时动态配置不同的分段数量
- 支持不同应用场景下的参数调整
- 同一个映射器实例可以通过 `updateConfig()` 切换不同的映射配置

### ✅ 内存效率
- 控制点数据存储在用户空间，映射器本身占用最小
- 避免了模板实例化的代码膨胀
- 支持静态数组和动态数组两种存储方式

### ✅ 易用性提升
- 用户可以方便地传入任意长度的控制点数组
- 配置更加直观，无需预先知道分段数量
- 支持从配置文件、传感器校准等场景动态加载参数

### ✅ 向后兼容
- 保持了与现有 SyncPipeline 接口的完全兼容
- API 设计风格与 LinearMapper 保持一致
- 现有代码迁移成本最小

## 使用场景对比

### 场景1：传感器校准
**之前**（编译时确定）：
```cpp
PiecewiseLinearMapper<1, 4> mapper(source, config);  // 必须预知4段
```

**之后**（运行时配置）：
```cpp
// 可以根据校准结果动态调整
static const float points[] = {/* 从校准获得 */};
config.segmentCount = calibrationPointCount - 1;
PiecewiseLinearMapper<1> mapper(source, config);
```

### 场景2：配置文件驱动
**之前**：需要为每种分段数量编译不同版本

**之后**：
```cpp
// 从配置文件读取
std::vector<float> inputPoints = loadFromConfig("input_points");
config.inputPoints = inputPoints.data();
config.segmentCount = inputPoints.size() - 1;
```

### 场景3：多设备适配
**之前**：不同设备需要不同的模板实例

**之后**：
```cpp
// 同一个类可以适配不同设备的分段需求
switch(deviceType) {
    case DEVICE_A: config.segmentCount = 2; break;
    case DEVICE_B: config.segmentCount = 5; break;
    // ...
}
```

## 性能影响

### 正面影响
- **编译时间减少**：减少了模板实例化数量
- **代码体积减小**：避免了多个 SEGMENTS 参数的代码膨胀
- **内存使用优化**：控制点数据由用户管理，更加灵活

### 需要注意的地方
- **运行时开销**：分段查找仍然是 O(segmentCount)，但现在是运行时变量
- **数据生命周期**：需要确保外部数组在映射器使用期间保持有效
- **配置验证**：运行时配置需要更严格的有效性检查

## 最佳实践

### 1. 使用静态数组（推荐）
```cpp
static const float inputPoints[] = {0.0f, 1000.0f, 2000.0f};
static const float outputPoints[] = {0.0f, 5.0f, 10.0f};

config.inputPoints = inputPoints;
config.outputPoints = outputPoints;
config.segmentCount = 2;
```

### 2. 动态数组（需要注意生命周期）
```cpp
std::vector<float> inputPoints = generatePoints();
config.inputPoints = inputPoints.data();  // 确保 vector 在使用期间不被销毁
```

### 3. 配置验证
```cpp
if (!PiecewiseLinearMapper<4>::isConfigValid(config)) {
    // 处理配置错误
    return;
}
```

## 兼容性说明

### 与现有代码的兼容性
- **SyncPipeline 接口**：完全兼容，无需修改
- **使用模式**：仅需要调整配置结构的初始化方式
- **功能行为**：映射算法和结果完全一致

### 迁移指南
1. **移除模板参数**：`PiecewiseLinearMapper<4, 3>` → `PiecewiseLinearMapper<4>`
2. **准备控制点数组**：将原本的配置数组改为外部数组
3. **设置分段数量**：在配置中指定 `segmentCount`
4. **验证配置**：使用 `isConfigValid()` 确保正确性

## 总结

这次修改成功地将分段线性映射器从编译时模板配置改为运行时指针配置，在保持性能和功能完整性的同时，大大提高了系统的灵活性和实用性。新设计更适合实际工程应用中的各种动态配置需求，是一个非常有价值的改进。

主要收益：
- ✅ **灵活性**：运行时配置，支持动态调整
- ✅ **内存效率**：用户控制数据存储，最小化映射器占用
- ✅ **易用性**：配置更直观，迁移成本低
- ✅ **兼容性**：保持接口稳定，功能一致