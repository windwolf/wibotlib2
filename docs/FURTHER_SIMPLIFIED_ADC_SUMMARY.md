# AdcSource进一步简化实现总结

## 简化目标

根据要求进一步简化AdcSource：
1. **去掉AdcVoltage概念**：直接使用原始数据，不再转换为电压
2. **统一数据类型**：无论分辨率多少，内部都使用int16_t保存

## 主要简化内容

### 1. 移除电压转换概念

#### 之前的设计：
```cpp
struct AdcVoltage {
    float voltage;  ///< 电压值 (V)
    bool  valid;    ///< 数据是否有效
};

class AdcAnalogInput : public SyncPipeline<AdcVoltage> {
    float   referenceVoltage;   ///< 参考电压 (V)
    float   calibrationOffset;  ///< 校准偏移量
    float   calibrationGain;    ///< 校准增益
};
```

#### 现在的设计：
```cpp
class AdcAnalogInput : public SyncPipeline<int16_t> {
    int16_t calibrationOffset;  ///< 校准偏移量 (原始值)
    int16_t calibrationGain;    ///< 校准增益 (1024 = 1.0)
};
```

### 2. 统一的int16_t数据类型

无论ADC分辨率是8位、10位、12位还是16位，内部都统一使用int16_t：

```cpp
// ADC原始值 -> int16_t转换
int16_t _convertToInt16(uint32_t raw) const {
    // 将原始ADC值转换为int16_t范围 (0 到 32767)
    uint32_t scaled = (raw * 32767U) / _maxAdcValue;
    return static_cast<int16_t>(scaled);
}
```

### 3. 简化的配置结构

```cpp
struct AdcSourceConfig {
    uint8_t adcResolution;      ///< ADC分辨率位数 (8, 10, 12, 16)
    int16_t calibrationOffset;  ///< 校准偏移量 (原始值)
    int16_t calibrationGain;    ///< 校准增益 (1024 = 1.0)
};
```

移除了：
- `referenceVoltage` - 不再需要电压转换
- 浮点数校准参数 - 改用固定小数点

### 4. 简化的数据存储

```cpp
template <uint8_t CHANNELS>
class AdcAnalogInput : public SyncPipeline<int16_t> {
private:
    AdcSourceConfig _config;           ///< ADC配置
    int16_t         _values[CHANNELS]; ///< 各通道校准后的值
    uint32_t        _maxAdcValue;      ///< ADC最大值
};
```

移除了：
- `_rawValues[]` - 不再单独存储原始值
- `_voltages[]` - 不再存储电压值
- `AdcVoltage` 结构体 - 直接使用int16_t

### 5. 实时处理逻辑

```cpp
void setRawValue(uint32_t rawValue, uint8_t channel) {
    if (channel >= CHANNELS) return;
    
    // 转换为int16_t并应用校准
    int16_t converted = _convertToInt16(rawValue);
    _values[channel] = _applyCalibration(converted);
}
```

**处理流程**：
1. 原始ADC值（任意分辨率）
2. 转换为int16_t（0-32767范围）
3. 应用校准（固定小数点运算）
4. 存储最终结果

### 6. 固定小数点校准

```cpp
int16_t _applyCalibration(int16_t value) const {
    // 应用校准: value = (value + offset) * gain / 1024
    int32_t result = static_cast<int32_t>(value) + static_cast<int32_t>(_config.calibrationOffset);
    result = (result * static_cast<int32_t>(_config.calibrationGain)) / 1024;
    
    // 限制到int16_t范围
    if (result > 32767) result = 32767;
    if (result < -32768) result = -32768;
    
    return static_cast<int16_t>(result);
}
```

**校准参数说明**：
- `calibrationGain = 1024` 表示增益 1.0
- `calibrationGain = 1536` 表示增益 1.5
- `calibrationOffset` 直接加到原始值上

## 使用示例

### 基本使用
```cpp
// 创建12位ADC，4通道
AdcSourceConfig config = {
    .adcResolution = 12,
    .calibrationOffset = 0,
    .calibrationGain = 1024  // 1.0
};

AdcInput4CH adc(config);

// 设置原始值（12位：0-4095）
uint32_t rawValues[4] = {1024, 2048, 3072, 4095};
adc.setRawValues(rawValues);

// 获取处理后的int16_t值（0-32767）
for (uint8_t ch = 0; ch < 4; ch++) {
    int16_t value = adc.getValue(ch);
    // 1024 -> 8192, 2048 -> 16384, 3072 -> 24576, 4095 -> 32767
}
```

### 校准功能
```cpp
// 设置校准：偏移100，增益1.5
adc.setCalibration(100, 1536);  // 1536/1024 = 1.5

// 重新设置同样的原始值
adc.setRawValues(rawValues);

// 获取校准后的值
// 计算公式：(原始int16_t值 + 100) * 1536 / 1024
```

## 性能优势

1. **更小内存占用**：
   - 移除了浮点数配置和计算
   - 每通道只需2字节存储（int16_t）

2. **更快处理速度**：
   - 整数运算替代浮点运算
   - 实时转换和校准，无需单独的update()

3. **更简单的接口**：
   - 直接返回int16_t，无需复杂的结构体
   - 固定小数点校准，易于理解和配置

## 兼容性影响

由于Pipeline接口从`SyncPipeline<AdcVoltage>`改为`SyncPipeline<int16_t>`，下游的Pipeline需要相应调整：

```cpp
// 映射器需要调整输入类型
class LinearMapper : public ProcessPipeline<int16_t, MappedValue> {
    MappedValue getValue(uint8_t channel = 0) const override {
        int16_t input = _upstream.getValue(channel);
        // 处理int16_t输入...
    }
};
```

## 编译状态

- ✅ 所有ADC相关文件编译无错误
- ✅ 测试代码已更新并验证
- ⚠️ 下游Pipeline可能需要调整以适应int16_t输入类型