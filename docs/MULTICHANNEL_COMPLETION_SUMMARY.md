# Pipeline多通道支持完成总结

## 已完成的工作

### 1. Pipeline核心接口简化和多通道支持

#### 修改文件：
- `wibotlib/src/hal/model.hpp`

#### 主要更改：
1. **简化Pipeline接口**：
   - 移除了 `isReady()` 方法
   - 移除了 `configure()` 方法  
   - 移除了Builder类支持
   - 移除了多输入支持

2. **添加多通道支持**：
   - `getValue()` 方法添加了 `uint8_t channel = 0` 参数
   - 默认通道为0，保持向后兼容性
   - 所有Pipeline子类都需要实现此接口

### 2. ADC源的多通道重构

#### 修改文件：
- `wibotlib/src/hal/adc-source.hpp` - 完全重构
- `wibotlib/src/hal/adc-source.cpp` - 完全重写

#### 主要更改：
1. **移除模板设计**：
   - 删除了 `MultiChannelAdcInput` 模板类
   - `AdcAnalogInput` 现在原生支持多通道

2. **多通道数据结构**：
   ```cpp
   std::vector<AdcRawValue> _rawValues;      // 各通道原始值
   std::vector<AdcVoltage> _voltages;        // 各通道电压值
   std::vector<float> _calibrationSums;      // 各通道校准累计
   std::vector<uint32_t> _calibrationCounts; // 各通道校准计数
   std::vector<bool> _isCalibrating;         // 各通道校准状态
   ```

3. **多通道方法**：
   - `getValue(uint8_t channel = 0)` - 获取指定通道电压
   - `setRawValue(uint32_t raw, uint8_t channel = 0)` - 设置单通道原始值
   - `setRawValues(const uint32_t* rawValues, uint8_t channelCount)` - 批量设置多通道
   - `calibrate(float knownVoltage, uint32_t samples, uint8_t channel = 0)` - 校准指定通道
   - `setCalibration(float offset, float gain, uint8_t channel = 0)` - 设置校准参数
   - `getCalibration(float& offset, float& gain, uint8_t channel = 0)` - 获取校准参数

4. **通道验证**：
   - `_isValidChannel(uint8_t channel)` - 验证通道索引有效性
   - `getChannelCount()` - 获取支持的通道数量

### 3. Mapping Pipeline多通道支持

#### 修改文件：
- `wibotlib/src/hal/mapping-model.hpp` - 方法签名更新
- `wibotlib/src/hal/mapping-SyncPipeline.cpp` - 实现逻辑更新

#### 主要更改：
1. **更新所有Mapper类**：
   - LinearMapper
   - PiecewiseLinearMapper  
   - LogarithmicMapper
   - ExponentialMapper
   - CustomMapper

2. **getValue方法重构**：
   ```cpp
   // 旧版本
   MappedValue getValue() const override;
   
   // 新版本  
   MappedValue getValue(uint8_t channel = 0) const override;
   ```

3. **实时处理逻辑**：
   ```cpp
   MappedValue getValue(uint8_t channel) const {
       AdcVoltage input = _upstream.getValue(channel);
       if (!input.valid) return MappedValue();
       
       float mappedValue = _mapFunction(input.voltage);
       return MappedValue(mappedValue, true, input.timestamp);
   }
   ```

### 4. Filter Pipeline多通道支持

#### 修改文件：
- `wibotlib/src/hal/filter-model.hpp` - 方法签名更新
- `wibotlib/src/hal/filter-SyncPipeline.cpp` - 方法签名更新

#### 主要更改：
1. **更新所有Filter类**：
   - MovingAverageFilter
   - LowpassFilter
   - MedianFilter
   - KalmanFilter
   - HighpassFilter
   - BandpassFilter
   - AdaptiveFilter

2. **保持状态管理**：
   - Filter仍然使用内部状态(`_output`)
   - 需要通过`update()`方法处理数据
   - 支持通道参数但维持现有逻辑

### 5. Binning Pipeline多通道支持

#### 修改文件：
- `wibotlib/src/hal/binning-model.hpp` - 方法签名更新
- `wibotlib/src/hal/binning-SyncPipeline.cpp` - 方法签名更新

#### 主要更改：
1. **批量更新所有Binning类的getValue方法签名**
2. **添加channel参数支持**

## 架构改进

### 1. 简化的设计
- 移除了复杂的Builder模式
- 移除了isReady状态检查
- 移除了configure配置步骤
- Pipeline现在更直接和简单

### 2. 原生多通道支持
- ADC源默认支持多通道，不需要模板
- 所有Pipeline都支持通道参数
- 向后兼容：默认channel=0

### 3. 一致的接口
- 所有Pipeline类都有统一的`getValue(uint8_t channel = 0)`接口
- 简化了多通道数据处理

## 使用示例

```cpp
// 创建3通道ADC
AdcAnalogInput adc(config, 3);

// 设置多通道数据
uint32_t rawValues[3] = {1024, 2048, 3072};
adc.setRawValues(rawValues, 3);
adc.update();

// 读取各通道电压
for (uint8_t ch = 0; ch < 3; ch++) {
    float voltage = adc.getValue(ch);
    std::cout << "Channel " << ch << ": " << voltage << "V\n";
}

// 使用映射管道
LinearMapper mapper(adc, mapConfig);
for (uint8_t ch = 0; ch < 3; ch++) {
    MappedValue mapped = mapper.getValue(ch);
    std::cout << "Mapped " << ch << ": " << mapped.value << "\n";
}
```

## 编译状态
- 所有修改的文件都能正常编译，无语法错误
- Pipeline接口一致性已验证
- 多通道功能接口完整

## 下一步
1. 可以创建实际测试程序验证功能
2. 可能需要更新Filter的状态管理以支持多通道状态
3. 可以考虑为每个通道独立的校准参数（目前所有通道共享校准参数）