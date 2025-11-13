/**
 * @file test_median_filter.cpp
 * @brief 中值滤波器功能测试
 *
 * 测试MedianFilter的基本功能和边界条件。
 */

#include "wibotlib/src/SyncPipeline/filter/median-filter.hpp"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace wibot;

/**
 * @brief 简单的测试数据源
 */
template <uint8_t CHANNELS>
class SimpleTestSource : public SyncPipeline<float> {
public:
  SimpleTestSource() { reset(); }

  void setValue(uint8_t channel, float value) {
    if (channel < CHANNELS) {
      _values[channel] = value;
    }
  }

  float getValue(uint8_t channel) const override {
    if (channel < CHANNELS) {
      return _values[channel];
    }
    return 0.0f;
  }

  void update() override {
    // 不需要实际的更新逻辑
  }

  void reset() override {
    for (uint8_t i = 0; i < CHANNELS; i++) {
      _values[i] = 0.0f;
    }
  }

private:
  float _values[CHANNELS];
};

/**
 * @brief 测试基本功能
 */
bool testBasicFunctionality() {
  std::cout << "测试基本功能..." << std::endl;

  SimpleTestSource<1> source;
  MedianFilter<1>::Config config = {.windowSize = 3};
  MedianFilter<1> filter(source, config);

  // 测试序列：1, 2, 3
  source.setValue(0, 1.0f);
  filter.update();
  float result1 = filter.getValue(0);

  source.setValue(0, 2.0f);
  filter.update();
  float result2 = filter.getValue(0);

  source.setValue(0, 3.0f);
  filter.update();
  float result3 = filter.getValue(0);

  std::cout << "  输入序列: 1, 2, 3" << std::endl;
  std::cout << "  输出序列: " << result1 << ", " << result2 << ", " << result3
            << std::endl;

  // 第三个输出应该是中值 2.0
  if (std::abs(result3 - 2.0f) < 1e-6f) {
    std::cout << "  ✓ 基本功能测试通过" << std::endl;
    return true;
  } else {
    std::cout << "  ✗ 基本功能测试失败" << std::endl;
    return false;
  }
}

/**
 * @brief 测试脉冲噪声抑制
 */
bool testImpulseNoiseFiltering() {
  std::cout << "测试脉冲噪声抑制..." << std::endl;

  SimpleTestSource<1> source;
  MedianFilter<1>::Config config = {.windowSize = 5};
  MedianFilter<1> filter(source, config);

  // 测试序列：1, 1, 10, 1, 1（中间有一个脉冲噪声）
  float inputs[] = {1.0f, 1.0f, 10.0f, 1.0f, 1.0f};
  float outputs[5];

  for (int i = 0; i < 5; i++) {
    source.setValue(0, inputs[i]);
    filter.update();
    outputs[i] = filter.getValue(0);
  }

  std::cout << "  输入序列: 1, 1, 10, 1, 1" << std::endl;
  std::cout << "  输出序列: ";
  for (int i = 0; i < 5; i++) {
    std::cout << outputs[i];
    if (i < 4)
      std::cout << ", ";
  }
  std::cout << std::endl;

  // 最后的输出应该接近1.0（脉冲被抑制）
  if (std::abs(outputs[4] - 1.0f) < 1e-6f) {
    std::cout << "  ✓ 脉冲噪声抑制测试通过" << std::endl;
    return true;
  } else {
    std::cout << "  ✗ 脉冲噪声抑制测试失败，期望1.0，得到" << outputs[4]
              << std::endl;
    return false;
  }
}

/**
 * @brief 测试多通道功能
 */
bool testMultiChannel() {
  std::cout << "测试多通道功能..." << std::endl;

  SimpleTestSource<3> source;
  MedianFilter<3>::Config config = {.windowSize = 3};
  MedianFilter<3> filter(source, config);

  // 为每个通道设置不同的输入序列
  // CH0: 1, 2, 3
  // CH1: 4, 5, 6
  // CH2: 7, 8, 9

  for (int step = 0; step < 3; step++) {
    source.setValue(0, static_cast<float>(1 + step));
    source.setValue(1, static_cast<float>(4 + step));
    source.setValue(2, static_cast<float>(7 + step));
    filter.update();
  }

  float ch0_output = filter.getValue(0); // 期望: 2.0
  float ch1_output = filter.getValue(1); // 期望: 5.0
  float ch2_output = filter.getValue(2); // 期望: 8.0

  std::cout << "  CH0输出: " << ch0_output << " (期望: 2.0)" << std::endl;
  std::cout << "  CH1输出: " << ch1_output << " (期望: 5.0)" << std::endl;
  std::cout << "  CH2输出: " << ch2_output << " (期望: 8.0)" << std::endl;

  bool success = (std::abs(ch0_output - 2.0f) < 1e-6f) &&
                 (std::abs(ch1_output - 5.0f) < 1e-6f) &&
                 (std::abs(ch2_output - 8.0f) < 1e-6f);

  if (success) {
    std::cout << "  ✓ 多通道功能测试通过" << std::endl;
    return true;
  } else {
    std::cout << "  ✗ 多通道功能测试失败" << std::endl;
    return false;
  }
}

/**
 * @brief 测试配置验证
 */
bool testConfigValidation() {
  std::cout << "测试配置验证..." << std::endl;

  // 测试有效配置
  MedianFilter<1>::Config validConfig = {.windowSize = 5};
  bool isValid1 = MedianFilter<1>::isConfigValid(validConfig);

  // 测试无效配置（窗口大小为0）
  MedianFilter<1>::Config invalidConfig1 = {.windowSize = 0};
  bool isValid2 = MedianFilter<1>::isConfigValid(invalidConfig1);

  // 测试边界配置（窗口大小为1）
  MedianFilter<1>::Config boundaryConfig = {.windowSize = 1};
  bool isValid3 = MedianFilter<1>::isConfigValid(boundaryConfig);

  std::cout << "  窗口大小=5: " << (isValid1 ? "有效" : "无效") << std::endl;
  std::cout << "  窗口大小=0: " << (isValid2 ? "有效" : "无效") << std::endl;
  std::cout << "  窗口大小=1: " << (isValid3 ? "有效" : "无效") << std::endl;

  if (isValid1 && !isValid2 && isValid3) {
    std::cout << "  ✓ 配置验证测试通过" << std::endl;
    return true;
  } else {
    std::cout << "  ✗ 配置验证测试失败" << std::endl;
    return false;
  }
}

/**
 * @brief 测试重置功能
 */
bool testReset() {
  std::cout << "测试重置功能..." << std::endl;

  SimpleTestSource<1> source;
  MedianFilter<1>::Config config = {.windowSize = 3};
  MedianFilter<1> filter(source, config);

  // 输入一些数据
  source.setValue(0, 5.0f);
  filter.update();
  source.setValue(0, 6.0f);
  filter.update();
  source.setValue(0, 7.0f);
  filter.update();

  float outputBeforeReset = filter.getValue(0);

  // 重置滤波器
  filter.reset();

  // 输入新数据
  source.setValue(0, 1.0f);
  filter.update();

  float outputAfterReset = filter.getValue(0);

  std::cout << "  重置前输出: " << outputBeforeReset << std::endl;
  std::cout << "  重置后输出: " << outputAfterReset << std::endl;

  // 重置后的第一个输出应该等于第一个输入
  if (std::abs(outputAfterReset - 1.0f) < 1e-6f) {
    std::cout << "  ✓ 重置功能测试通过" << std::endl;
    return true;
  } else {
    std::cout << "  ✗ 重置功能测试失败" << std::endl;
    return false;
  }
}

/**
 * @brief 主函数 - 运行所有测试
 */
int main() {
  std::cout << "中值滤波器 (MedianFilter) 功能测试" << std::endl;
  std::cout << "====================================" << std::endl;
  std::cout << std::endl;

  int passedTests = 0;
  int totalTests = 0;

  // 运行所有测试
  totalTests++;
  if (testBasicFunctionality())
    passedTests++;
  std::cout << std::endl;

  totalTests++;
  if (testImpulseNoiseFiltering())
    passedTests++;
  std::cout << std::endl;

  totalTests++;
  if (testMultiChannel())
    passedTests++;
  std::cout << std::endl;

  totalTests++;
  if (testConfigValidation())
    passedTests++;
  std::cout << std::endl;

  totalTests++;
  if (testReset())
    passedTests++;
  std::cout << std::endl;

  // 测试结果汇总
  std::cout << "测试结果汇总:" << std::endl;
  std::cout << "通过测试: " << passedTests << "/" << totalTests << std::endl;

  if (passedTests == totalTests) {
    std::cout << "✓ 所有测试通过！" << std::endl;
    return 0;
  } else {
    std::cout << "✗ 部分测试失败！" << std::endl;
    return 1;
  }
}