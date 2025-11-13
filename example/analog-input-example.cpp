#include "analog-input-example.hpp"
#include <iomanip>
#include <iostream>


namespace wibot {

AnalogInputExample::AnalogInputExample() : _analogInput() {
  // 构造函数中可以进行基本初始化
}

void AnalogInputExample::basicUsageExample() {
  std::cout << "\n=== AnalogInput基本使用示例 ===\n" << std::endl;

  // 创建一个简单的4通道模拟输入配置
  AnalogInput<4>::AnalogInputConfig config = {};
  config.activeChannels = 0x0F;   // 激活前4个通道 (0b1111)
  config.adcResolution = 12;      // 12位ADC
  config.referenceVoltage = 3.3f; // 3.3V参考电压
  config.sampleRate = 1000;       // 1kHz采样率

  // 为所有通道设置统一的基本配置
  config.channelConfig = {.filterType =
                              FilterType::kMovingAverage, // 移动平均滤波
                          .filterSize = 8,                // 8点滤波
                          .filterParameter = 0.0f,
                          .mappingMode = MappingMode::kLinear, // 线性映射
                          .inputMin = 0.0f, // 输入范围 0-3.3V
                          .inputMax = 3.3f,
                          .outputMin = 0.0f, // 输出范围 0-100%
                          .outputMax = 100.0f,
                          .enableBinning = false, // 不启用分桶
                          .binningConfig = {},
                          .calibrationOffset = 0.0f, // 无校准偏移
                          .calibrationGain = 1.0f,   // 无校准增益
                          .enableDeadband = false,   // 不启用死区
                          .deadbandCenter = 0.0f,
                          .deadbandWidth = 0.0f};

  // 应用配置
  _analogInput.configure(config);

  // 模拟10次数据更新
  std::cout << "模拟10次ADC采样和处理:\n" << std::endl;
  std::cout << std::setw(8) << "Sample" << std::setw(12) << "Ch0(V)"
            << std::setw(12) << "Ch1(V)" << std::setw(12) << "Ch2(%)"
            << std::setw(12) << "Ch3(%)" << std::endl;
  std::cout << std::string(56, '-') << std::endl;

  for (int i = 0; i < 10; i++) {
    // 模拟ADC原始数据
    uint32_t rawValues[4] = {_simulateAdcReading(0), _simulateAdcReading(1),
                             _simulateAdcReading(2), _simulateAdcReading(3)};

    // 更新模拟输入
    _analogInput.update(rawValues);

    // 显示处理后的数据
    std::cout << std::setw(8) << i + 1 << std::setw(12) << std::fixed
              << std::setprecision(3) << _analogInput.getVoltage(0)
              << std::setw(12) << std::fixed << std::setprecision(3)
              << _analogInput.getVoltage(1) << std::setw(12) << std::fixed
              << std::setprecision(1) << _analogInput.getMappedValue(2)
              << std::setw(12) << std::fixed << std::setprecision(1)
              << _analogInput.getMappedValue(3) << std::endl;
  }
}

void AnalogInputExample::advancedConfigExample() {
  std::cout << "\n=== AnalogInput高级配置示例 ===\n" << std::endl;
  std::cout << "注意: 新版本AnalogInput所有通道使用相同配置\n" << std::endl;

  // 示例1: 低通滤波配置
  std::cout << "1. 低通滤波配置示例:" << std::endl;
  AnalogInput<4> lowPassInput;
  AnalogInput<4>::AnalogInputConfig config1 = {};
  config1.activeChannels = 0x0F;
  config1.adcResolution = 12;
  config1.referenceVoltage = 3.3f;
  config1.sampleRate = 2000;

  config1.channelConfig = {
      .filterType = FilterType::kLowPass,
      .filterSize = 0,
      .filterParameter = 0.1f, // 低通滤波器系数
      .mappingMode = MappingMode::kLinear,
      .inputMin = 0.5f, // 输入范围 0.5-2.5V
      .inputMax = 2.5f,
      .outputMin = -10.0f, // 输出范围 -10到+10
      .outputMax = 10.0f,
      .enableBinning = false,
      .binningConfig = {},
      .calibrationOffset = 0.05f, // 50mV偏移校准
      .calibrationGain = 0.98f,   // 2%增益校准
      .enableDeadband = true,     // 启用死区
      .deadbandCenter = 0.0f,
      .deadbandWidth = 0.2f // ±0.1的死区
  };

  lowPassInput.configure(config1);

  // 示例2: 中值滤波配置
  std::cout << "2. 中值滤波配置示例:" << std::endl;
  AnalogInput<4> medianInput;
  AnalogInput<4>::AnalogInputConfig config2 = {};
  config2.activeChannels = 0x0F;
  config2.adcResolution = 12;
  config2.referenceVoltage = 3.3f;
  config2.sampleRate = 2000;

  config2.channelConfig = {.filterType = FilterType::kMedian,
                           .filterSize = 5, // 5点中值滤波
                           .filterParameter = 0.0f,
                           .mappingMode = MappingMode::kLogarithmic,
                           .inputMin = 0.1f,
                           .inputMax = 3.0f,
                           .outputMin = 0.0f,
                           .outputMax = 100.0f,
                           .enableBinning = false,
                           .binningConfig = {},
                           .calibrationOffset = 0.0f,
                           .calibrationGain = 1.0f,
                           .enableDeadband = false,
                           .deadbandCenter = 0.0f,
                           .deadbandWidth = 0.0f};

  medianInput.configure(config2);

  std::cout << "所有通道配置相同，简化了配置管理\n" << std::endl;

  // 运行处理
  std::cout << std::setw(8) << "Sample" << std::setw(12) << "LowPass0"
            << std::setw(12) << "LowPass1" << std::setw(12) << "Median0"
            << std::setw(12) << "Median1" << std::endl;
  std::cout << std::string(56, '-') << std::endl;

  for (int i = 0; i < 15; i++) {
    uint32_t rawValues[4] = {_simulateAdcReading(0), _simulateAdcReading(1),
                             _simulateAdcReading(2), _simulateAdcReading(3)};

    lowPassInput.update(rawValues);
    medianInput.update(rawValues);

    std::cout << std::setw(8) << i + 1 << std::setw(12) << std::fixed
              << std::setprecision(2) << lowPassInput.getMappedValue(0)
              << std::setw(12) << std::fixed << std::setprecision(2)
              << lowPassInput.getMappedValue(1) << std::setw(12) << std::fixed
              << std::setprecision(1) << medianInput.getMappedValue(0)
              << std::setw(12) << std::fixed << std::setprecision(1)
              << medianInput.getMappedValue(1) << std::endl;
  }
}

void AnalogInputExample::calibrationExample() {
  std::cout << "\n=== AnalogInput校准示例 ===\n" << std::endl;

  // 基本配置
  BasicAnalogInput<2> calibrationInput;

  std::cout << "1. 校准前测试 (假设通道0有100mV偏移误差):" << std::endl;

  // 模拟校准前的读数（有误差）
  uint32_t testValues[2] = {
      static_cast<uint32_t>((1.1f / 3.3f) * 4095), // 应该是1.0V，但读为1.1V
      static_cast<uint32_t>((2.05f / 3.3f) * 4095) // 应该是2.0V，但读为2.05V
  };

  calibrationInput.update(testValues);

  std::cout << "标准值: 1.000V, 2.000V" << std::endl;
  std::cout << "读取值: " << std::fixed << std::setprecision(3)
            << calibrationInput.getVoltage(0) << "V, "
            << calibrationInput.getVoltage(1) << "V" << std::endl;

  std::cout << "\n2. 执行校准 (使用已知1.0V标准):" << std::endl;

  // 对通道0进行校准，已知标准值为1.0V
  calibrationInput.calibrateChannel(0, 1.0f, 50); // 使用50个样本校准

  // 手动设置通道1的校准参数
  calibrationInput.setCalibration(1, -0.05f, 1.0f); // -50mV偏移

  std::cout << "校准完成!" << std::endl;

  std::cout << "\n3. 校准后测试:" << std::endl;

  // 使用相同的测试数据
  calibrationInput.update(testValues);

  std::cout << "标准值: 1.000V, 2.000V" << std::endl;
  std::cout << "读取值: " << std::fixed << std::setprecision(3)
            << calibrationInput.getVoltage(0) << "V, "
            << calibrationInput.getVoltage(1) << "V" << std::endl;

  // 显示校准参数
  float offset0, gain0, offset1, gain1;
  calibrationInput.getCalibration(0, &offset0, &gain0);
  calibrationInput.getCalibration(1, &offset1, &gain1);

  std::cout << "\n校准参数:" << std::endl;
  std::cout << "通道0: 偏移=" << offset0 << "V, 增益=" << gain0 << std::endl;
  std::cout << "通道1: 偏移=" << offset1 << "V, 增益=" << gain1 << std::endl;
}

void AnalogInputExample::filterComparisonExample() {
  std::cout << "\n=== 滤波器比较示例 ===\n" << std::endl;
  std::cout
      << "注意: 新版本中所有通道使用相同配置，这里展示不同滤波器配置的效果\n"
      << std::endl;

  // 创建4个不同配置的AnalogInput实例来比较滤波器效果
  AnalogInput<1> noFilter, movingAverage, lowPass, median;

  // 配置1: 无滤波
  AnalogInput<1>::AnalogInputConfig config1 = {};
  config1.activeChannels = 0x01;
  config1.adcResolution = 12;
  config1.referenceVoltage = 3.3f;
  config1.sampleRate = 1000;
  config1.channelConfig = {.filterType = FilterType::kNone,
                           .filterSize = 0,
                           .filterParameter = 0.0f,
                           .mappingMode = MappingMode::kLinear,
                           .inputMin = 0.0f,
                           .inputMax = 3.3f,
                           .outputMin = 0.0f,
                           .outputMax = 3.3f,
                           .enableBinning = false,
                           .binningConfig = {},
                           .calibrationOffset = 0.0f,
                           .calibrationGain = 1.0f,
                           .enableDeadband = false,
                           .deadbandCenter = 0.0f,
                           .deadbandWidth = 0.0f};

  // 配置2: 移动平均
  AnalogInput<1>::AnalogInputConfig config2 = config1;
  config2.channelConfig.filterType = FilterType::kMovingAverage;
  config2.channelConfig.filterSize = 8;

  // 配置3: 低通滤波
  AnalogInput<1>::AnalogInputConfig config3 = config1;
  config3.channelConfig.filterType = FilterType::kLowPass;
  config3.channelConfig.filterParameter = 0.2f;

  // 配置4: 中值滤波
  AnalogInput<1>::AnalogInputConfig config4 = config1;
  config4.channelConfig.filterType = FilterType::kMedian;
  config4.channelConfig.filterSize = 5;

  noFilter.configure(config1);
  movingAverage.configure(config2);
  lowPass.configure(config3);
  median.configure(config4);

  std::cout << "向所有滤波器输入相同的带噪声信号:\n" << std::endl;

  std::cout << std::setw(8) << "Sample" << std::setw(12) << "无滤波"
            << std::setw(12) << "移动平均" << std::setw(12) << "低通"
            << std::setw(12) << "中值" << std::endl;
  std::cout << std::string(56, '-') << std::endl;

  for (int i = 0; i < 20; i++) {
    // 生成1.65V基准 + 0.3V正弦波 + 随机噪声的信号
    float baseSignal = 1.65f + 0.3f * std::sin(i * 0.3f);
    float noise =
        (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.2f; // ±0.1V噪声
    float noisySignal = baseSignal + noise;

    // 限制在0-3.3V范围内
    noisySignal = std::max(0.0f, std::min(3.3f, noisySignal));

    // 转换为ADC原始值
    uint32_t rawValue = static_cast<uint32_t>((noisySignal / 3.3f) * 4095);

    uint32_t rawValues[1] = {rawValue};
    noFilter.update(rawValues);
    movingAverage.update(rawValues);
    lowPass.update(rawValues);
    median.update(rawValues);

    std::cout << std::setw(8) << i + 1 << std::setw(12) << std::fixed
              << std::setprecision(3) << noFilter.getFilteredValue(0)
              << std::setw(12) << std::fixed << std::setprecision(3)
              << movingAverage.getFilteredValue(0) << std::setw(12)
              << std::fixed << std::setprecision(3)
              << lowPass.getFilteredValue(0) << std::setw(12) << std::fixed
              << std::setprecision(3) << median.getFilteredValue(0)
              << std::endl;
  }
}

void AnalogInputExample::realTimeProcessingExample() {
  std::cout << "\n=== 实时数据处理示例 ===\n" << std::endl;

  // 配置一个用于实时处理的模拟输入
  AnalogInput<2> realtimeInput;

  AnalogInput<2>::AnalogInputConfig config = {};
  config.activeChannels = 0x03; // 激活2个通道
  config.adcResolution = 12;
  config.referenceVoltage = 3.3f;
  config.sampleRate = 1000;

  // 为所有通道配置温度传感器仿真 (需要滤波和校准)
  config.channelConfig = {
      .filterType = FilterType::kMovingAverage,
      .filterSize = 10, // 10点平均以降低噪声
      .filterParameter = 0.0f,
      .mappingMode = MappingMode::kLinear,
      .inputMin = 0.5f,    // 对应-10°C
      .inputMax = 2.5f,    // 对应+60°C
      .outputMin = -10.0f, // 温度范围-10到60°C
      .outputMax = 60.0f,
      .enableBinning = true,
      .binningConfig = {.binCount = 8, // 8个温度区间
                        .minValue = -10.0f,
                        .maxValue = 60.0f,
                        .enableOverflow = true,
                        .enableUnderflow = true},
      .calibrationOffset = -2.5f, // 传感器校准偏移
      .calibrationGain = 1.02f,   // 传感器校准增益
      .enableDeadband = true,     // 启用死区避免抖动
      .deadbandCenter = 25.0f,    // 室温附近
      .deadbandWidth = 1.0f       // ±0.5°C死区
  };

  realtimeInput.configure(config);

  std::cout << "模拟实时传感器数据处理:" << std::endl;
  std::cout << "所有通道配置: 温度传感器 (-10到60°C, 统一配置)\n" << std::endl;

  std::cout << std::setw(8) << "Time(s)" << std::setw(12) << "Temp0(°C)"
            << std::setw(12) << "T0-Bin" << std::setw(12) << "Temp1(°C)"
            << std::setw(12) << "T1-Bin" << std::endl;
  std::cout << std::string(60, '-') << std::endl;

  for (int i = 0; i < 30; i++) {
    // 模拟两个温度传感器数据 (缓慢变化 + 噪声)
    float temp0Base = 22.0f + 8.0f * std::sin(i * 0.1f);  // 通道0: 14-30°C范围
    float temp1Base = 25.0f + 5.0f * std::sin(i * 0.15f); // 通道1: 20-30°C范围
    float temp0Noise = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f;
    float temp1Noise = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f;

    float temp0Voltage = (temp0Base + temp0Noise - (-10.0f)) /
                             (60.0f - (-10.0f)) * (2.5f - 0.5f) +
                         0.5f;
    float temp1Voltage = (temp1Base + temp1Noise - (-10.0f)) /
                             (60.0f - (-10.0f)) * (2.5f - 0.5f) +
                         0.5f;

    // 转换为ADC值
    uint32_t rawValues[2] = {
        static_cast<uint32_t>((temp0Voltage / 3.3f) * 4095),
        static_cast<uint32_t>((temp1Voltage / 3.3f) * 4095)};

    realtimeInput.update(rawValues);

    std::cout << std::setw(8) << std::fixed << std::setprecision(1)
              << (i * 0.1f) << std::setw(12) << std::fixed
              << std::setprecision(1) << realtimeInput.getMappedValue(0)
              << std::setw(12) << realtimeInput.getBinIndex(0) << std::setw(12)
              << std::fixed << std::setprecision(1)
              << realtimeInput.getMappedValue(1) << std::setw(12)
              << realtimeInput.getBinIndex(1) << std::endl;
  }

  // 显示统计信息
  std::cout << "\n处理统计信息:" << std::endl;
  _printStatistics();
}

uint32_t AnalogInputExample::_simulateAdcReading(uint8_t channel) {
  // 根据通道生成不同的模拟ADC数据
  static int step = 0;
  step++;

  float voltage = 0.0f;

  switch (channel) {
  case 0: // 缓慢变化的正弦波 0.5-2.5V
    voltage = 1.5f + 1.0f * std::sin(step * 0.1f);
    break;
  case 1: // 三角波 0-3.3V
    voltage = 3.3f * (step % 20) / 20.0f;
    break;
  case 2: // 随机噪声 1-2V
    voltage = 1.0f + (static_cast<float>(rand()) / RAND_MAX);
    break;
  case 3: // 方波 0.5V/2.5V
    voltage = ((step / 5) % 2) ? 2.5f : 0.5f;
    break;
  default:
    voltage = 1.65f; // 中点电压
    break;
  }

  // 添加小量噪声
  voltage += (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.1f;

  // 限制在0-3.3V范围
  voltage = std::max(0.0f, std::min(3.3f, voltage));

  // 转换为12位ADC值
  return static_cast<uint32_t>((voltage / 3.3f) * 4095);
}

void AnalogInputExample::_printChannelInfo(uint8_t channel) {
  std::cout << "通道 " << static_cast<int>(channel) << ":" << std::endl;
  std::cout << "  原始值: " << _analogInput.getRawValue(channel) << std::endl;
  std::cout << "  电压值: " << std::fixed << std::setprecision(3)
            << _analogInput.getVoltage(channel) << " V" << std::endl;
  std::cout << "  滤波值: " << std::fixed << std::setprecision(3)
            << _analogInput.getFilteredValue(channel) << std::endl;
  std::cout << "  映射值: " << std::fixed << std::setprecision(2)
            << _analogInput.getMappedValue(channel) << std::endl;

  if (_analogInput.isChannelActive(channel)) {
    float min, max, avg, rms;
    _analogInput.getStatistics(channel, &min, &max, &avg, &rms);
    std::cout << "  统计: Min=" << min << ", Max=" << max << ", Avg=" << avg
              << ", RMS=" << rms << std::endl;
  }
}

void AnalogInputExample::_printStatistics() {
  for (uint8_t i = 0; i < 4; i++) {
    if (_analogInput.isChannelActive(i)) {
      float min, max, avg, rms;
      _analogInput.getStatistics(i, &min, &max, &avg, &rms);
      std::cout << "通道" << static_cast<int>(i) << " - Min:" << std::fixed
                << std::setprecision(2) << min << ", Max:" << max
                << ", Avg:" << avg << ", RMS:" << rms << std::endl;
    }
  }
}

} // namespace wibot