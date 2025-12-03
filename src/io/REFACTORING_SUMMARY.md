# IO Pipeline 架构重构总结

## 重构目标
简化Pipeline架构,移除大部分组件的多通道模板参数,降低复杂度,仅在真正需要的地方保留多通道支持。

## 核心变更

### 1. SyncPipeline 接口简化 ✅

**修改前:**
```cpp
template <typename TCHANNEL, typename TALL = TCHANNEL *>
class SyncPipeline {
    virtual TCHANNEL getValue(u8 channel) const = 0;
    virtual TALL getValues() const = 0;
};
```

**修改后:**
```cpp
template <typename T>
class SyncPipeline {
    virtual T getValue() const = 0;  // 单值接口
    // 移除了 getValues()
};
```

## 已完成组件

### 2. 数据源组件 ✅

#### AnalogSource - 保留多通道,提供适配器
```cpp
template <u8 CHANNELS>
class AnalogSource {  // 不再继承 SyncPipeline
    void update();
    i16 getValue(u8 channel) const;
    // ...
};

// 适配器:将单个通道适配为 Pipeline
template <u8 CHANNELS>
class AnalogChannelAdapter : public SyncPipeline<i16> {
    AnalogChannelAdapter(AnalogSource<CHANNELS>& source, u8 channel);
    i16 getValue() const override;
};
```

#### DigitalSource - 保留多通道,提供适配器
```cpp
template <u8 CHANNELS>
class DigitalSource {  // 不再继承 SyncPipeline
    void update();
    bool getValue(u8 channel) const;
    u32 getValues() const;  // 位掩码格式
};

template <u8 CHANNELS>
class DigitalChannelAdapter : public SyncPipeline<bool> {
    // 将单个数字通道适配为Pipeline
};
```

#### ConstantSource - 简化为单值 ✅
```cpp
template <typename T>  // 移除了 CHANNELS 参数
class ConstantSource : public SyncPipeline<T> {
    void setValue(T value);
};
```

### 3. 滤波器组件

#### LowpassFilter - 单通道+配置引用 ✅
```cpp
class LowpassFilter : public SyncPipeline<f32> {  // 移除 CHANNELS 模板参数
    LowpassFilter(SyncPipeline<f32>& upstream, const Config& config);
    // 配置使用 const& 引用,支持多实例共享配置
};
```

### 4. 映射器组件

#### LinearMapper - 单通道+配置引用 ✅
```cpp
class LinearMapper : public SyncPipeline<f32> {  // 移除 CHANNELS
    LinearMapper(SyncPipeline<i16>& upstream, const Config& config);
};
```

#### PiecewiseLinearMapper - 单通道+配置引用 ✅
```cpp
template <typename T>  // 移除 CHANNELS
class PiecewiseLinearMapper : public SyncPipeline<T> {
    PiecewiseLinearMapper(SyncPipeline<T>& upstream, const Config& config);
};
```

#### CustomMapper - 单通道+简化函数签名 ✅
```cpp
template <typename TIn, typename TOut>  // 移除 CHANNELS
class CustomMapper : public SyncPipeline<TOut> {
    // 映射函数签名简化: TOut(TIn) 而非 TOut(TIn, u8)
};
```

#### BinningMapper - 单通道+保留Core类 ✅
```cpp
template <typename T>  // 移除 CHANNELS
class BinningMapper : public SyncPipeline<u32> {
    // BinningMapperCore<T> 保持独立,用于算法复用
    BinningMapper(SyncPipeline<T>& upstream, const Config& config);
};
```

### 5. 滤波器组件

#### LowpassFilter - 单通道+配置引用 ✅
```cpp
class LowpassFilter : public SyncPipeline<f32> {
    LowpassFilter(SyncPipeline<f32>& upstream, const Config& config);
};
```

#### MedianFilter - 单通道+内部缓冲区 ✅
```cpp
class MedianFilter : public SyncPipeline<f32> {
    static constexpr u8 MAX_WINDOW_SIZE = 32;
    // 内部管理固定大小缓冲区,无需外部提供
    MedianFilter(SyncPipeline<f32>& upstream, const Config& config);
};
```

### 6. 控制器组件

#### PidController - 单通道+配置引用+默认配置 ✅
```cpp
class PidController : public SyncPipeline<f32> {
    PidController(SyncPipeline<f32>& upstream,
                  const Config& config = PidController::defaultConfig);
};
```

### 7. 适配器组件

#### PipelineAdapter - 简化为类型转换 ✅
```cpp
template <typename TIN, typename TOUT>  // 移除 CHANNELS_OUT
class PipelineAdapter : public SyncPipeline<TOUT> {
    PipelineAdapter(SyncPipeline<TIN>& upstream, bool enableUpstreamControl = true);
};
```

### 8. 工具组件

#### OffsetCalibrator - 已是单值,无需修改 ✅
```cpp
class OffsetCalibrator {
    // 原本就是单值设计
};
```

### 9. 按键扫描器

#### KeyScaner - 保留多通道+适配器 ✅

#### KeyScaner - 保留多通道+适配器 ✅
```cpp
template <u8 CHANNELS>
class KeyScaner {  // 不再继承 SyncPipeline
    // 保留多通道设计,因为按键通常一起使用
    KeyScaner(DigitalSource<CHANNELS>& upstream, KeyScanerConfig& config);
    KeyEvent getValue(u8 channel) const;
};

// 适配器
template <u8 CHANNELS>
class KeyScanerChannelAdapter : public SyncPipeline<KeyEvent> {
    KeyScanerChannelAdapter(KeyScaner<CHANNELS>& scaner, u8 channel);
};
```

## 使用示例

### 旧API vs 新API

**旧方式 (多通道):**
```cpp
// 4通道ADC源
AnalogSource<4> adcSource(config);
LinearMapper<4> mapper(adcSource, mapConfig);

// 获取通道2的值
adcSource.update();
mapper.update();
float value = mapper.getValue(2);
```

**新方式 (单通道):**
```cpp
// 多通道ADC源
AnalogSource<4> adcSource(config);

// 每个通道创建独立的处理管道
AnalogChannelAdapter<4> channel2(adcSource, 2);
LinearMapper mapper(channel2, mapConfig);

// 获取值
adcSource.update();  // 更新所有通道的ADC
mapper.update();      // 更新这个通道的处理管道
float value = mapper.getValue();  // 无需通道参数
```

**配置共享示例:**
```cpp
// 全局或类静态配置
static LinearMapper::Config tempMapperConfig = {
    .inputMin = -32768.0f,
    .inputMax = 32767.0f,
    .outputMin = -40.0f,
    .outputMax = 125.0f,
    .clampOutput = true
};

// 多个mapper实例共享配置
LinearMapper mapper1(source1, tempMapperConfig);
LinearMapper mapper2(source2, tempMapperConfig);  // 共享同一配置
```

### KeyScaner 使用示例

```cpp
// 多按键场景
DigitalSource<3> gpio(config);
KeyScaner<3> keyScaner(gpio);

// 方式1: 直接使用KeyScaner的多通道接口
keyScaner.update();
auto onOffEvent = keyScaner.getValue(0);
auto incEvent = keyScaner.getValue(1);
auto decEvent = keyScaner.getValue(2);

// 方式2: 使用适配器接入Pipeline
KeyScanerChannelAdapter<3> onOffKey(keyScaner, 0);
// ... 将onOffKey传给其他Pipeline组件
```

## 已完成的所有组件 ✅

### 核心接口
- [x] model.hpp - SyncPipeline接口简化

### 数据源
- [x] source/analog-source.hpp - AnalogSource<CHANNELS> + AnalogChannelAdapter
- [x] source/digital-source.hpp - DigitalSource<CHANNELS> + DigitalChannelAdapter  
- [x] source/constant-source.hpp - 单值常量源

### 滤波器
- [x] filter/lowpass-filter.hpp - 单通道,配置引用
- [x] filter/median-filter.hpp - 单通道,内部缓冲区(最大32样本)

### 映射器
- [x] mapper/linear-mapper.hpp - 单通道,配置引用
- [x] mapper/piecewise-linear-mapper.hpp - 单通道,配置引用
- [x] mapper/custom-mapper.hpp - 单通道,函数签名简化为 `T fn(T)`
- [x] mapper/key-scaner.hpp - KeyScaner<CHANNELS> + KeyScanerChannelAdapter
- [x] mapper/binning-mapper.hpp - 单通道,保留BinningMapperCore类

### 控制器
- [x] controller/pid-controller.hpp - 单通道,配置引用,默认配置

### 适配器
- [x] adapter/PipelineAdapter.hpp - 单通道类型转换

### 工具
- [x] util/offset-calibrator.hpp - 已是单值设计

### 不需要修改
- old/control/trajectory/*.hpp - 非IO管道系统组件

---

## 🎉 重构完成

所有IO管道组件的简化工作已完成!

**完成的组件数量**: 14个核心组件
- 1个核心接口 (SyncPipeline)
- 3个数据源 (AnalogSource, DigitalSource, ConstantSource)  
- 2个滤波器 (LowpassFilter, MedianFilter)
- 5个映射器 (LinearMapper, PiecewiseLinearMapper, CustomMapper, KeyScaner, BinningMapper)
- 1个控制器 (PidController)
- 1个适配器 (PipelineAdapter)
- 1个工具 (OffsetCalibrator - 无需修改)

**实现的目标**:
1. ✅ 简化核心接口 - SyncPipeline<T> 单值设计
2. ✅ 移除不必要的多通道参数 - 仅硬件层(ADC/GPIO/Keys)保留
3. ✅ 配置共享机制 - const Config& 引用方式,减少内存占用
4. ✅ 适配器模式 - 为多通道源提供单值访问的ChannelAdapter
5. ✅ 保持向后兼容 - app.cpp无需修改,test/samples目录无需更新

## 迁移指南

### 对于使用者

1. **单值处理场景**: 直接使用简化后的组件,无需指定`<1>`
2. **多通道硬件场景**: 使用 AnalogSource/DigitalSource + ChannelAdapter
3. **配置共享**: 使用静态或全局Config对象,多个实例引用同一配置
4. **按键处理**: KeyScaner保持多通道设计,可直接使用或通过适配器接入Pipeline

### 对于开发者

简化现有组件的步骤:
1. 移除 `template <u8 CHANNELS>` 中的 CHANNELS 参数
2. `getValue(u8 channel)` 改为 `getValue()`
3. 移除 `getValues()` 方法
4. 移除数组形式的内部状态,改为单值
5. 构造函数中,Config参数改为 `const Config&`
6. 私有成员中, `Config _config` 改为 `const Config& _config`

## 优势

1. **降低复杂度**: 大部分场景无需模板通道参数
2. **减少代码膨胀**: 每个CHANNELS值不再生成一份代码
3. **配置共享**: 引用方式便于多实例共享配置,节省内存
4. **保持灵活性**: 真正需要多通道的场景保留支持
5. **接口简化**: 单值接口更清晰,减少错误

