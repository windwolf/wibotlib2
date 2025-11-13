#ifndef PIPELINE_EXAMPLE_HPP
#define PIPELINE_EXAMPLE_HPP

#pragma once

#include "..\src\hal\adc-source.hpp"
#include "..\src\hal\analog-input-adapter.hpp"
#include "..\src\hal\binning-model.hpp"
#include "..\src\hal\filter-model.hpp"
#include "..\src\hal\mapping-model.hpp"
#include <iostream>
#include <memory>

namespace wibot {

/**
 * @brief 管道使用示例
 *
 * 展示如何使用新的管道架构构建复杂的数据处理链。
 */
class PipelineExample {
public:
  /**
   * @brief 示例1：基本管道链
   *
   * 演示用户要求的管道组合：
   * AdcAnalogInput -> PiecewiseLinearMapper -> LowpassFilter -> LinearBinning
   */
  static void basicPipelineExample() {
    std::cout << "=== 基本管道链示例 ===" << std::endl;

    // 1. 创建ADC源
    AdcChannel channel = {.adcInstance = nullptr, // 在实际使用中设置
                          .channelNumber = 1,
                          .rank = 1,
                          .samplingTime = 0};

    AdcAnalogInput::Config adcConfig = {.vrefVoltage = 3.3f,
                                        .resolution = 12,
                                        .oversample = 4,
                                        .calibration = {.enabled = false}};

    AdcAnalogInput adcSrc(channel, adcConfig);

    // 2. 创建分段线性映射器
    PiecewiseLinearMapper::Config mapperConfig;
    mapperConfig.points = {
        {0.0f, 0.0f},  // 0V -> 0°C
        {1.0f, 25.0f}, // 1V -> 25°C
        {2.0f, 60.0f}, // 2V -> 60°C
        {3.3f, 100.0f} // 3.3V -> 100°C
    };
    mapperConfig.clampOutput = true;

    PiecewiseLinearMapper mapper(adcSrc, mapperConfig);

    // 3. 创建低通滤波器
    LowpassFilter::Config filterConfig = {.cutoffFrequency = 5.0f,
                                          .samplingFrequency = 100.0f};

    LowpassFilter filter(mapper, filterConfig);

    // 4. 创建线性分桶器
    LinearBinning::Config binningConfig = {.binCount = 8,
                                           .minValue = 0.0f,
                                           .maxValue = 100.0f,
                                           .enableOverflow = true,
                                           .enableUnderflow = true,
                                           .clampToBounds = false};

    LinearBinning binning(filter, binningConfig);

    // 5. 模拟数据处理
    std::cout << "处理模拟数据..." << std::endl;

    // 在实际应用中，这里会从硬件读取数据
    // 现在我们模拟一些数据
    for (int i = 0; i < 10; i++) {
      // 模拟ADC读取和更新过程
      adcSrc.update();
      mapper.update();
      filter.update();
      binning.update();

      if (binning.isReady()) {
        auto result = binning.getValue();
        if (result.valid) {
          std::cout << "原始值: " << result.originalValue
                    << "°C, 分桶: " << result.binIndex << std::endl;
        }
      }
    }
  }

  /**
   * @brief 示例2：多通道传感器
   *
   * 演示多通道模拟输入适配器的使用。
   */
  static void multiChannelExample() {
    std::cout << "\n=== 多通道传感器示例 ===" << std::endl;

    // 定义多个通道
    std::vector<AdcChannel> channels = {
        {nullptr, 1, 1, 0}, // 温度传感器
        {nullptr, 2, 2, 0}, // 压力传感器
        {nullptr, 3, 3, 0}  // 光传感器
    };

    // 配置多通道输入
    AnalogInput::Config config = {};
    config.vrefVoltage = 3.3f;
    config.resolution = 12;
    config.oversample = 4;

    // 启用滤波
    config.filtering.enabled = true;
    config.filtering.windowSize = 8;

    // 创建多通道适配器
    MultiChannelAnalogInputAdapter multiChannel(channels, config);

    if (multiChannel.init()) {
      multiChannel.startConversion();

      std::cout << "通道数量: "
                << static_cast<int>(multiChannel.getChannelCount())
                << std::endl;

      // 模拟数据读取
      for (int cycle = 0; cycle < 5; cycle++) {
        multiChannel.update();

        for (uint8_t ch = 0; ch < multiChannel.getChannelCount(); ch++) {
          if (multiChannel.isReady(ch) && multiChannel.isValid(ch)) {
            std::cout << "通道 " << static_cast<int>(ch)
                      << " - 电压: " << multiChannel.getVoltage(ch) << "V"
                      << ", 滤波值: " << multiChannel.getFilteredValue(ch)
                      << ", 状态: " << multiChannel.getPipelineStatus(ch)
                      << std::endl;
          }
        }
        std::cout << "---" << std::endl;
      }
    }
  }

  /**
   * @brief 示例3：特化传感器
   *
   * 演示使用便利函数创建特定类型的传感器。
   */
  static void specializedSensorExample() {
    std::cout << "\n=== 特化传感器示例 ===" << std::endl;

    AdcChannel tempChannel = {nullptr, 1, 1, 0};
    AdcChannel pressureChannel = {nullptr, 2, 2, 0};
    AdcChannel batteryChannel = {nullptr, 3, 3, 0};

    // 创建特化传感器
    auto tempSensor = createTemperatureSensor(tempChannel, -20.0f, 80.0f);
    auto pressureSensor = createPressureSensor(pressureChannel, 150.0f);
    auto batteryMonitor = createBatteryMonitor(batteryChannel, 12.0f);

    // 初始化所有传感器
    std::vector<SingleChannelAnalogInputAdapter *> sensors = {
        tempSensor.get(), pressureSensor.get(), batteryMonitor.get()};

    std::vector<std::string> sensorNames = {"温度传感器", "压力传感器",
                                            "电池监测"};

    for (size_t i = 0; i < sensors.size(); i++) {
      if (sensors[i]->init()) {
        sensors[i]->startConversion();
        std::cout << sensorNames[i] << " 初始化成功" << std::endl;
      }
    }

    // 模拟数据采集
    for (int cycle = 0; cycle < 3; cycle++) {
      std::cout << "\n采集周期 " << (cycle + 1) << ":" << std::endl;

      for (size_t i = 0; i < sensors.size(); i++) {
        sensors[i]->update();

        if (sensors[i]->isReady() && sensors[i]->isValid()) {
          std::cout << sensorNames[i] << ": "
                    << "原始值=" << sensors[i]->getRawValue()
                    << ", 电压=" << sensors[i]->getVoltage() << "V"
                    << ", 映射值=" << sensors[i]->getMappedValue()
                    << ", 滤波值=" << sensors[i]->getFilteredValue()
                    << std::endl;
        }
      }
    }
  }

  /**
   * @brief 示例4：高级滤波和分桶
   *
   * 演示高级滤波算法和分桶功能。
   */
  static void advancedProcessingExample() {
    std::cout << "\n=== 高级处理示例 ===" << std::endl;

    AdcChannel channel = {nullptr, 1, 1, 0};

    AdcAnalogInput::Config adcConfig = {.vrefVoltage = 3.3f,
                                        .resolution = 12,
                                        .oversample = 4,
                                        .calibration = {.enabled = false}};

    AdcAnalogInput adcSrc(channel, adcConfig);

    // 线性映射
    LinearMapper::Config mapperConfig = {.inputMin = 0.0f,
                                         .inputMax = 3.3f,
                                         .outputMin = 0.0f,
                                         .outputMax = 100.0f,
                                         .clampOutput = true};

    LinearMapper mapper(adcSrc, mapperConfig);

    // 卡尔曼滤波器
    KalmanFilter::Config kalmanConfig = {.processNoise = 0.01f,
                                         .measurementNoise = 0.1f,
                                         .initialEstimate = 0.0f,
                                         .initialErrorCovariance = 1.0f};

    KalmanFilter kalmanFilter(mapper, kalmanConfig);

    // 分位数分桶
    QuantileBinning::Config quantileConfig = {.binCount = 5,
                                              .historySize = 50,
                                              .updateInterval = 10,
                                              .enableOverflow = true,
                                              .enableUnderflow = true};

    QuantileBinning quantileBinning(kalmanFilter, quantileConfig);

    std::cout << "使用卡尔曼滤波器和分位数分桶..." << std::endl;

    // 模拟带噪声的数据
    for (int i = 0; i < 20; i++) {
      adcSrc.update();
      mapper.update();
      kalmanFilter.update();
      quantileBinning.update();

      if (quantileBinning.isReady()) {
        auto filtered = kalmanFilter.getValue();
        auto binned = quantileBinning.getValue();

        if (filtered.valid && binned.valid) {
          std::cout << "样本 " << i << ": 滤波值=" << filtered.value
                    << ", 分桶=" << binned.binIndex
                    << ", 原始值=" << binned.originalValue << std::endl;
        }
      }
    }
  }

  /**
   * @brief 运行所有示例
   */
  static void runAllExamples() {
    std::cout << "管道架构使用示例\n" << std::string(50, '=') << std::endl;

    basicPipelineExample();
    multiChannelExample();
    specializedSensorExample();
    advancedProcessingExample();

    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "所有示例完成!" << std::endl;
  }
};

} // namespace wibot

#endif // PIPELINE_EXAMPLE_HPP