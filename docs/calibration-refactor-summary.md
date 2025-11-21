# AnalogSource 偏移校准功能重构总结

## 重构前后对比

### 重构前的问题
- AnalogSource 类承担了数据读取和校准两个职责
- 复杂的校准策略模式增加了不必要的抽象
- 内置的定时采样逻辑限制了用户的控制灵活性
- 违反了单一职责原则

### 重构后的优势
- **简化设计**：AnalogSource 只负责数据读取和偏移应用
- **职责分离**：OffsetCalibrator 专门负责偏移量计算
- **用户控制**：完全由用户代码控制采样时机和条件
- **直接接口**：简单明了的API，易于理解和使用

## 核心组件

### 1. AnalogSource<CHANNELS>
**功能**：ADC数据源，负责读取原始数据并应用偏移校准

**核心方法**：
- `void setOffset(u8 channel, i16 offset)` - 设置单个通道偏移
- `i16 getOffset(u8 channel) const` - 获取单个通道偏移
- `void setOffsets(const i16 offsets[])` - 批量设置所有通道偏移
- `const i16* getOffsets() const` - 获取所有通道偏移
- `u16* getBuffer()` - 获取原始ADC缓冲区

**简化内容**：
- 直接在类中存储偏移量数组 `_offsets[CHANNELS]`
- 在 `update()` 方法中直接应用偏移校准
- 移除了复杂的策略模式和状态管理

### 2. OffsetCalibrator<CHANNELS>
**功能**：偏移校准器，专门负责收集样本并计算偏移量

**核心方法**：
- `void startCalibration()` - 开始新的校准周期
- `bool addSample(const u16 samples[])` - 添加样本数据（用户控制时机）
- `bool calculate()` - 计算偏移量
- `bool applyToAnalogSource(AnalogSource<CHANNELS>&)` - 直接应用到ADC源

**状态管理**：
- `State getState()` - 获取校准状态（Idle/Collecting/Ready）
- `f32 getProgress()` - 获取进度百分比
- `bool isReady()` - 检查是否完成

## 使用方式

### 基本使用流程

```cpp
// 1. 创建ADC源和校准器
AnalogSource<4>::Config adcConfig{12};  // 12位ADC
AnalogSource<4> adcSource(adcConfig);

OffsetCalibrator<4>::Config calibConfig{100};  // 100个样本
OffsetCalibrator<4> calibrator(calibConfig);

// 2. 开始校准
calibrator.startCalibration();

// 3. 用户控制采样（例如：按钮按下、定时器、特定条件等）
u16* buffer = adcSource.getBuffer();
for (int i = 0; i < 100; i++) {
    // 用户决定何时采样
    if (shouldSample()) {  // 用户自定义的采样条件
        calibrator.addSample(buffer);
    }
}

// 4. 应用校准结果
if (calibrator.isReady()) {
    calibrator.applyToAnalogSource(adcSource);
}
```

### 手动偏移设置

```cpp
AnalogSource<2> adcSource(config);

// 方式1：逐个设置
adcSource.setOffset(0, -100);
adcSource.setOffset(1, -200);

// 方式2：批量设置
i16 offsets[2] = {-100, -200};
adcSource.setOffsets(offsets);
```

### 条件触发校准

```cpp
// 用户完全控制校准时机
for (int cycle = 0; cycle < 1000; cycle++) {
    // 检查用户自定义的校准条件
    bool systemIdle = checkSystemIdle();
    bool temperatureStable = checkTemperature();
    bool userRequest = checkUserInput();
    
    if (systemIdle && temperatureStable && userRequest) {
        if (calibrator.getState() == OffsetCalibrator::State::Idle) {
            calibrator.startCalibration();
        }
        
        if (calibrator.getState() == OffsetCalibrator::State::Collecting) {
            calibrator.addSample(adcBuffer);
        }
    }
    
    if (calibrator.isReady()) {
        calibrator.applyToAnalogSource(adcSource);
        break;
    }
}
```

## 主要优势

### 1. 简化的架构
- 移除了复杂的策略模式
- 减少了抽象层级
- 代码更直观易懂

### 2. 灵活的控制
- 用户完全控制采样时机
- 支持各种触发条件：按钮、定时器、系统状态等
- 可以在校准过程中暂停或重置

### 3. 直接的API
- 简单的方法调用
- 明确的职责分工
- 易于调试和维护

### 4. 高效的实现
- 减少了运行时开销
- 减少了内存占用（移除了策略对象）
- 更好的编译优化机会

## 向后兼容性

虽然API发生了变化，但迁移很简单：

```cpp
// 旧的方式（复杂）
auto strategy = CalibrationFactory::createOffsetCalibration<4>(100);
adcSource.setCalibrationStrategy(std::move(strategy));
// ... 复杂的策略操作 ...

// 新的方式（简单）
OffsetCalibrator<4> calibrator({100});
// ... 用户控制的采样 ...
calibrator.applyToAnalogSource(adcSource);
```

## 文件结构

```
wibotlib/src/model/
├── source/
│   └── analog-source.hpp              # 简化的ADC源
├── calibration/
│   └── offset-calibrator.hpp          # 偏移校准器
├── example/
│   └── simple-calibration-example.hpp # 使用示例
└── test/
    └── calibration-test.cpp            # 测试代码
```

### 已删除的文件
- `calibration-strategy.hpp` - 复杂的策略模式，不再需要
- `analog-source-calibration-example.hpp` - 旧的复杂示例

## 总结

这次重构成功实现了：
1. **职责分离**：AnalogSource专注数据读取，OffsetCalibrator专注偏移计算
2. **用户控制**：完全由用户代码决定何时采样
3. **简化设计**：移除不必要的抽象，使用直接的API
4. **易于维护**：代码结构清晰，逻辑简单

新的设计更符合单一职责原则，提供了更大的灵活性，同时保持了简洁性和高效性。