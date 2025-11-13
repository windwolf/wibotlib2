/**
 * @file multichannel-mapping-example.cpp
 * @brief 多通道映射管道使用示例
 *
 * 本示例展示如何使用模板化的LinearMapper支持多通道映射
 */

#include "hal/adc-source.hpp"
#include "hal/mapping-model.hpp"

using namespace wibot;

// 示例：4通道传感器数据处理系统
void multiChannelSensorExample() {
  // 假设有ADC硬件接口
  // SomeAdcHardware adcHardware;

  // 1. 创建4通道ADC源 (12位ADC, 每通道有不同的偏移校准)
  int16_t offsets[4] = {100, -50, 200, 0}; // 校准偏移值
  // AdcSource<4> adcSource(adcHardware, 12, offsets);

  // 2. 创建4通道线性映射器 - 所有通道共享同一套配置
  // 例如：将ADC的int16_t范围映射到0-100的百分比
  LinearMapper<4>::Config sharedConfig = {
      .inputMin = -32768.0f, // int16_t最小值
      .inputMax = 32767.0f,  // int16_t最大值
      .outputMin = 0.0f,     // 输出最小值 (0%)
      .outputMax = 100.0f,   // 输出最大值 (100%)
      .clampOutput = true    // 限制输出范围
  };

  // LinearMapper<4> sensorMapper(adcSource, sharedConfig);

  // 3. 使用映射器获取各通道的百分比值
  // float channel0_percent = sensorMapper.getValue(0);  // 通道0百分比
  // float channel1_percent = sensorMapper.getValue(1);  // 通道1百分比
  // float channel2_percent = sensorMapper.getValue(2);  // 通道2百分比
  // float channel3_percent = sensorMapper.getValue(3);  // 通道3百分比

  // 4. 动态更新配置（影响所有通道）
  LinearMapper<4>::Config newConfig = {.inputMin = -30000.0f, // 调整输入范围
                                       .inputMax = 30000.0f,
                                       .outputMin = 0.0f, // 保持输出范围
                                       .outputMax = 100.0f,
                                       .clampOutput = true};
  // sensorMapper.updateConfig(newConfig);
}

// 示例：单通道温度传感器（使用便利函数）
void singleChannelTemperatureExample() {
  // 假设有ADC硬件和单通道ADC源
  // SomeAdcHardware adcHardware;
  // int16_t offset = 150;
  // AdcSource<1> tempAdcSource(adcHardware, 12, &offset);

  // 使用便利函数创建温度映射器
  // auto tempMapper = createTemperatureMapper(
  //     tempAdcSource,
  //     -32768.0f, 32767.0f,  // 输入范围
  //     -40.0f, 125.0f        // 输出温度范围
  // );

  // 获取温度值
  // float currentTemp = tempMapper.getValue(0);
}

// 示例：双通道差分传感器
void dualChannelDifferentialExample() {
  // 假设有双通道ADC源
  // SomeAdcHardware adcHardware;
  // int16_t offsets[2] = {0, 0};
  // AdcSource<2> diffAdcSource(adcHardware, 12, offsets);

  // 两个通道共享相同的映射配置（差分信号）
  LinearMapper<2>::Config diffConfig = {.inputMin = -32768.0f,
                                        .inputMax = 32767.0f,
                                        .outputMin = -2.5f, // ±2.5V差分范围
                                        .outputMax = 2.5f,
                                        .clampOutput = true};

  // LinearMapper<2> diffMapper(diffAdcSource, diffConfig);

  // 获取差分信号值
  // float signal_pos = diffMapper.getValue(0);  // 正信号
  // float signal_neg = diffMapper.getValue(1);  // 负信号
  // float differential = signal_pos - signal_neg;  // 差分值
}

// 主要特性总结：
/*
1. 模板参数：template<uint8_t CHANNELS> 编译时确定通道数
2. 共享配置：所有通道使用同一套Config，简化设计
3. 单一构造函数：LinearMapper(upstream, config) - 所有通道共享配置
4. 实时映射：getValue(channel) 直接返回映射后的float值
5. 全局配置更新：updateConfig() 更新所有通道的映射参数
6. 便利函数：createTemperatureMapper() 等快速创建常用映射器
7. 非内联实现：模板实现在.cpp文件中，减少编译时间
8. 类型安全：编译时通道数检查，运行时通道范围检查
9. 零开销：模板实例化，无虚函数调用开销

使用场景：
- 多通道相同类型传感器（温度、压力等）
- ADC原始数据到百分比的统一转换
- 差分信号处理
- 多通道数据归一化
*/