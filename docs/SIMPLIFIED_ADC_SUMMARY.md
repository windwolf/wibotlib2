# AdcSource简化实现总结

## 设计目标

根据要求简化AdcSource的实现，专注于核心功能：
1. 采集原始数值
2. 校准偏移和增益  
3. 应用偏移和增益

## 主要改进

### 1. 模板化设计
- 通道数量在编译时确定：`template<uint8_t CHANNELS>`
- 避免运行时动态内存分配
- 类型安全和性能优化

### 2. 移除std标准库依赖
- 用固定大小数组替代`std::vector`
- 移除`<algorithm>`和其他std头文件
- 只保留`<cstdint>`用于标准整数类型

### 3. 简化的数据结构

#### 配置结构
```cpp
struct AdcSourceConfig {
    uint8_t adcResolution;      ///< ADC分辨率位数
    float   referenceVoltage;   ///< 参考电压 (V)
    float   calibrationOffset;  ///< 校准偏移量
    float   calibrationGain;    ///< 校准增益
};
```

移除了：
- `sampleRate` - 不是核心功能
- 复杂的时间戳支持

#### 电压值结构
```cpp
struct AdcVoltage {
    float voltage;  ///< 电压值 (V)
    bool  valid;    ///< 数据是否有效
};
```

移除了：
- `timestamp` - 简化设计
- `AdcRawValue` - 内部使用简单uint32_t

### 4. 核心类设计

```cpp
template<uint8_t CHANNELS>
class AdcAnalogInput : public SyncPipeline<AdcVoltage> {
private:
    AdcSourceConfig _config;                ///< ADC配置
    uint32_t        _rawValues[CHANNELS];   ///< 各通道原始值
    AdcVoltage      _voltages[CHANNELS];    ///< 各通道电压值
    uint32_t        _maxAdcValue;           ///< ADC最大值
};
```

### 5. 简化的接口

#### 核心功能方法
```cpp
// 设置原始值
void setRawValue(uint32_t rawValue, uint8_t channel = 0);
void setRawValues(const uint32_t* rawValues);

// 获取处理后的值
AdcVoltage getValue(uint8_t channel = 0) const override;

// 校准功能
void setCalibration(float offset, float gain);
void getCalibration(float& offset, float& gain) const;

// Pipeline接口
void update() override;
void reset() override;
```

#### 移除的复杂功能
- 异步校准过程
- 每通道独立校准参数
- 复杂的时间戳管理
- 原始值的详细结构体
- 样本统计功能

### 6. 核心算法

#### 原始值转电压
```cpp
float _rawToVoltage(uint32_t raw) const {
    return (static_cast<float>(raw) / static_cast<float>(_maxAdcValue)) * _config.referenceVoltage;
}
```

#### 应用校准
```cpp
float _applyCalibration(float voltage) const {
    return (voltage + _config.calibrationOffset) * _config.calibrationGain;
}
```

#### 更新处理
```cpp
void update() {
    for (uint8_t ch = 0; ch < CHANNELS; ch++) {
        float rawVoltage = _rawToVoltage(_rawValues[ch]);
        float calibratedVoltage = _applyCalibration(rawVoltage);
        _voltages[ch].voltage = calibratedVoltage;
        _voltages[ch].valid = true;
    }
}
```

## 类型别名

为方便使用，提供了常用的类型别名：
```cpp
using AdcInput1CH = AdcAnalogInput<1>;
using AdcInput2CH = AdcAnalogInput<2>;
using AdcInput4CH = AdcAnalogInput<4>;
using AdcInput8CH = AdcAnalogInput<8>;
```

## 使用示例

```cpp
// 创建4通道ADC
AdcSourceConfig config = {
    .adcResolution = 12,
    .referenceVoltage = 3.3f,
    .calibrationOffset = 0.0f,
    .calibrationGain = 1.0f
};

AdcInput4CH adc(config);

// 设置原始数据
uint32_t rawValues[4] = {1024, 2048, 3072, 4095};
adc.setRawValues(rawValues);

// 设置校准
adc.setCalibration(0.1f, 0.98f); // 偏移0.1V，增益0.98

// 更新处理
adc.update();

// 读取结果
for (uint8_t ch = 0; ch < 4; ch++) {
    AdcVoltage result = adc.getValue(ch);
    if (result.valid) {
        // 使用result.voltage
    }
}
```

## 性能优势

1. **编译时优化**：通道数在编译时确定，允许更好的优化
2. **零动态分配**：所有数据结构都是固定大小
3. **缓存友好**：连续的数组访问模式
4. **小内存占用**：移除了不必要的元数据

## 兼容性

- 维持了Pipeline接口的兼容性
- `getValue(channel)`方法签名保持不变
- 支持现有的mapping和filter SyncPipeline

## 编译状态

- 所有文件编译无错误
- 模板显式实例化了1、2、4、8通道版本
- 测试代码已更新并验证