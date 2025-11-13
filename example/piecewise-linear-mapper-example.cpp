/**
 * @file piecewise-linear-mapper-example.cpp
 * @brief 分段线性映射器使用示例
 *
 * 本示例展示如何使用 PiecewiseLinearMapper 进行多通道分段线性映射。
 */

#include "piecewise-linear-mapper.hpp"
#include <iostream>

using namespace wibot;

// 假设的ADC数据源（为了示例简化）
template <uint8_t CHANNELS> class MockAdcSource : public SyncPipeline<int16_t> {
public:
  MockAdcSource() : _values{} {}

  void setValue(uint8_t channel, int16_t value) {
    if (channel < CHANNELS) {
      _values[channel] = value;
    }
  }

  int16_t getValue(uint8_t channel) const override {
    if (channel < CHANNELS) {
      return _values[channel];
    }
    return 0;
  }

  void update() override {
    // 模拟ADC更新
  }

  void reset() override {
    for (auto &value : _values) {
      value = 0;
    }
  }

private:
  int16_t _values[CHANNELS];
};

int main() {
  std::cout << "=== 分段线性映射器示例 ===" << std::endl;

  // 创建4通道ADC数据源
  MockAdcSource<4> adcSource;

  // 配置分段线性映射器（3段，即4个控制点）
  PiecewiseLinearMapper<4, 3>::Config config;

  // 设置输入控制点：0, 1000, 2000, 4095 (12位ADC范围)
  config.inputPoints[0] = 0.0f;
  config.inputPoints[1] = 1000.0f;
  config.inputPoints[2] = 2000.0f;
  config.inputPoints[3] = 4095.0f;

  // 设置输出控制点：实现非线性映射
  config.outputPoints[0] = 0.0f; // 0V
  config.outputPoints[1] = 1.2f; // 1.2V (非线性增长)
  config.outputPoints[2] = 2.8f; // 2.8V
  config.outputPoints[3] = 3.3f; // 3.3V

  config.clampOutput = true;          // 启用输出钳位
  config.enableExtrapolation = false; // 禁用外推

  // 验证配置
  if (!PiecewiseLinearMapper<4, 3>::isConfigValid(config)) {
    std::cout << "错误：配置无效！" << std::endl;
    return -1;
  }

  // 创建分段线性映射器
  PiecewiseLinearMapper<4, 3> mapper(adcSource, config);

  std::cout << "\n=== 测试不同输入值的映射结果 ===" << std::endl;

  // 测试数据
  int16_t testValues[] = {0,    500,  1000, 1500,
                          2000, 3000, 4095, 4500}; // 包含超出范围的值

  for (int16_t testValue : testValues) {
    // 设置所有通道为相同的测试值
    for (uint8_t ch = 0; ch < 4; ++ch) {
      adcSource.setValue(ch, testValue);
    }

    // 更新管道
    mapper.update();

    // 输出结果
    std::cout << "输入: " << testValue << " -> ";
    for (uint8_t ch = 0; ch < 4; ++ch) {
      float result = mapper.getValue(ch);
      std::cout << "CH" << (int)ch << ": " << result << "V ";
    }
    std::cout << std::endl;
  }

  std::cout << "\n=== 测试配置更新 ===" << std::endl;

  // 创建新配置：简单的2段映射
  PiecewiseLinearMapper<4, 3>::Config newConfig;
  newConfig.inputPoints[0] = 0.0f;
  newConfig.inputPoints[1] = 1365.0f; // 1/3点
  newConfig.inputPoints[2] = 2730.0f; // 2/3点
  newConfig.inputPoints[3] = 4095.0f;

  newConfig.outputPoints[0] = 0.0f;
  newConfig.outputPoints[1] = 1.1f;
  newConfig.outputPoints[2] = 2.2f;
  newConfig.outputPoints[3] = 3.3f;

  newConfig.clampOutput = true;
  newConfig.enableExtrapolation = true; // 启用外推

  // 更新配置
  mapper.updateConfig(newConfig);
  std::cout << "已更新映射配置" << std::endl;

  // 测试更新后的映射
  adcSource.setValue(0, 2000);
  mapper.update();
  std::cout << "输入: 2000 -> 输出: " << mapper.getValue(0) << "V" << std::endl;

  std::cout << "\n=== 示例完成 ===" << std::endl;

  return 0;
}

/*
预期输出示例：
=== 分段线性映射器示例 ===

=== 测试不同输入值的映射结果 ===
输入: 0 -> CH0: 0V CH1: 0V CH2: 0V CH3: 0V
输入: 500 -> CH0: 0.6V CH1: 0.6V CH2: 0.6V CH3: 0.6V
输入: 1000 -> CH0: 1.2V CH1: 1.2V CH2: 1.2V CH3: 1.2V
输入: 1500 -> CH0: 2V CH1: 2V CH2: 2V CH3: 2V
输入: 2000 -> CH0: 2.8V CH1: 2.8V CH2: 2.8V CH3: 2.8V
输入: 3000 -> CH0: 3.04V CH1: 3.04V CH2: 3.04V CH3: 3.04V
输入: 4095 -> CH0: 3.3V CH1: 3.3V CH2: 3.3V CH3: 3.3V
输入: 4500 -> CH0: 3.3V CH1: 3.3V CH2: 3.3V CH3: 3.3V

=== 测试配置更新 ===
已更新映射配置
输入: 2000 -> 输出: 1.61V

=== 示例完成 ===
*/