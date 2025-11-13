/**
 * @file test_custom_mapper.cpp
 * @brief CustomMapper功能测试
 *
 * 测试CustomMapper的基本功能和边界条件。
 */

#include "../wibotlib/src/SyncPipeline/mapper/custom-mapper.hpp"
#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>

using namespace wibot;

/**
 * @brief 简单的测试数据源
 */
template <typename T, uint8_t CHANNELS>
class SimpleTestSource : public SyncPipeline<T> {
public:
  SimpleTestSource() { reset(); }

  void setValue(uint8_t channel, T value) {
    if (channel < CHANNELS) {
      _values[channel] = value;
    }
  }

  T getValue(uint8_t channel) const override {
    if (channel < CHANNELS) {
      return _values[channel];
    }
    return T{};
  }

  void update() override {}
  void reset() override {
    for (uint8_t i = 0; i < CHANNELS; i++) {
      _values[i] = T{};
    }
  }

private:
  T _values[CHANNELS];
};

/**
 * @brief 测试基本映射功能
 */
bool testBasicMapping() {
  std::cout << "测试基本映射功能..." << std::endl;

  SimpleTestSource<int, 1> source;

  // 创建简单的乘法映射
  auto multiplyBy2 = [](int input, uint8_t /*channel*/) -> float {
    return input * 2.0f;
  };

  CustomMapper<int, float, 1>::Config config = {.mappingFunc = multiplyBy2};

  CustomMapper<int, float, 1> mapper(source, config);

  // 测试映射
  source.setValue(0, 5);
  mapper.update();
  float result = mapper.getValue(0);

  std::cout << "  输入: 5, 期望: 10.0, 实际: " << result << std::endl;

  if (std::abs(result - 10.0f) < 1e-6f) {
    std::cout << "  ✓ 基本映射测试通过" << std::endl;
    return true;
  } else {
    std::cout << "  ✗ 基本映射测试失败" << std::endl;
    return false;
  }
}

/**
 * @brief 测试映射函数内部限制功能
 */
bool testInternalLimiting() {
  std::cout << "测试映射函数内部限制功能..." << std::endl;

  SimpleTestSource<float, 1> source;

  // 创建带内部限制的平方映射
  auto clampedSquare = [](float input, uint8_t /*channel*/) -> float {
    float result = input * input;
    // 内部限制到[0, 10]范围
    if (result > 10.0f)
      result = 10.0f;
    if (result < 0.0f)
      result = 0.0f;
    return result;
  };

  CustomMapper<float, float, 1>::Config config = {.mappingFunc = clampedSquare};

  CustomMapper<float, float, 1> mapper(source, config);

  // 测试超出上限的情况
  source.setValue(0, 5.0f); // 5² = 25，应该被限制为10
  mapper.update();
  float result1 = mapper.getValue(0);

  // 测试正常范围的情况
  source.setValue(0, 2.0f); // 2² = 4，应该保持4
  mapper.update();
  float result2 = mapper.getValue(0);

  std::cout << "  输入5.0: 期望10.0(限制), 实际: " << result1 << std::endl;
  std::cout << "  输入2.0: 期望4.0(正常), 实际: " << result2 << std::endl;

  bool success =
      (std::abs(result1 - 10.0f) < 1e-6f) && (std::abs(result2 - 4.0f) < 1e-6f);

  if (success) {
    std::cout << "  ✓ 内部限制测试通过" << std::endl;
    return true;
  } else {
    std::cout << "  ✗ 内部限制测试失败" << std::endl;
    return false;
  }
}

/**
 * @brief 测试多通道功能
 */
bool testMultiChannel() {
  std::cout << "测试多通道功能..." << std::endl;

  constexpr uint8_t CHANNELS = 3;
  SimpleTestSource<int, CHANNELS> source;

  // 创建通道相关的映射
  auto channelMapping = [](int input, uint8_t channel) -> float {
    return input * (channel + 1); // CH0: *1, CH1: *2, CH2: *3
  };

  CustomMapper<int, float, CHANNELS>::Config config = {.mappingFunc =
                                                           channelMapping};

  CustomMapper<int, float, CHANNELS> mapper(source, config);

  // 设置所有通道的输入为10
  for (uint8_t i = 0; i < CHANNELS; i++) {
    source.setValue(i, 10);
  }
  mapper.update();

  float result0 = mapper.getValue(0); // 期望: 10 * 1 = 10
  float result1 = mapper.getValue(1); // 期望: 10 * 2 = 20
  float result2 = mapper.getValue(2); // 期望: 10 * 3 = 30

  std::cout << "  CH0输出: " << result0 << " (期望: 10)" << std::endl;
  std::cout << "  CH1输出: " << result1 << " (期望: 20)" << std::endl;
  std::cout << "  CH2输出: " << result2 << " (期望: 30)" << std::endl;

  bool success = (std::abs(result0 - 10.0f) < 1e-6f) &&
                 (std::abs(result1 - 20.0f) < 1e-6f) &&
                 (std::abs(result2 - 30.0f) < 1e-6f);

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

  // 有效映射函数
  auto validFunc = [](int input, uint8_t /*channel*/) -> float {
    return static_cast<float>(input);
  };

  // 测试有效配置
  CustomMapper<int, float, 1>::Config validConfig = {.mappingFunc = validFunc};
  bool isValid1 = CustomMapper<int, float, 1>::isConfigValid(validConfig);

  // 测试无效配置：空函数
  CustomMapper<int, float, 1>::Config invalidConfig = {.mappingFunc = nullptr};
  bool isValid2 = CustomMapper<int, float, 1>::isConfigValid(invalidConfig);

  std::cout << "  有效配置: " << (isValid1 ? "有效" : "无效") << std::endl;
  std::cout << "  空函数配置: " << (isValid2 ? "有效" : "无效") << std::endl;

  if (isValid1 && !isValid2) {
    std::cout << "  ✓ 配置验证测试通过" << std::endl;
    return true;
  } else {
    std::cout << "  ✗ 配置验证测试失败" << std::endl;
    return false;
  }
}

/**
 * @brief 测试配置更新
 */
bool testConfigUpdate() {
  std::cout << "测试配置更新..." << std::endl;

  SimpleTestSource<float, 1> source;

  // 初始映射：恒等函数
  auto identity = [](float input, uint8_t /*channel*/) -> float {
    return input;
  };

  CustomMapper<float, float, 1>::Config config = {.mappingFunc = identity};

  CustomMapper<float, float, 1> mapper(source, config);

  // 测试初始配置
  source.setValue(0, 5.0f);
  mapper.update();
  float result1 = mapper.getValue(0);

  // 更新配置：平方函数
  auto square = [](float input, uint8_t /*channel*/) -> float {
    return input * input;
  };

  CustomMapper<float, float, 1>::Config newConfig = {.mappingFunc = square};

  mapper.updateConfig(newConfig);
  mapper.update();
  float result2 = mapper.getValue(0);

  std::cout << "  更新前输出: " << result1 << " (期望: 5)" << std::endl;
  std::cout << "  更新后输出: " << result2 << " (期望: 25)" << std::endl;

  if (std::abs(result1 - 5.0f) < 1e-6f && std::abs(result2 - 25.0f) < 1e-6f) {
    std::cout << "  ✓ 配置更新测试通过" << std::endl;
    return true;
  } else {
    std::cout << "  ✗ 配置更新测试失败" << std::endl;
    return false;
  }
}

/**
 * @brief 测试复杂数学函数
 */
bool testComplexMathFunctions() {
  std::cout << "测试复杂数学函数..." << std::endl;

  SimpleTestSource<float, 1> source;

  // 正弦函数映射
  auto sineMapping = [](float input, uint8_t /*channel*/) -> float {
    return std::sin(input);
  };

  CustomMapper<float, float, 1>::Config config = {.mappingFunc = sineMapping};

  CustomMapper<float, float, 1> mapper(source, config);

  // 测试sin(π/2) = 1
  source.setValue(0, M_PI / 2);
  mapper.update();
  float result = mapper.getValue(0);

  std::cout << "  sin(π/2): " << result << " (期望: ~1.0)" << std::endl;

  if (std::abs(result - 1.0f) < 1e-5f) {
    std::cout << "  ✓ 复杂数学函数测试通过" << std::endl;
    return true;
  } else {
    std::cout << "  ✗ 复杂数学函数测试失败" << std::endl;
    return false;
  }
}

/**
 * @brief 测试错误处理
 */
bool testErrorHandling() {
  std::cout << "测试错误处理..." << std::endl;

  SimpleTestSource<int, 2> source;

  auto validFunc = [](int input, uint8_t /*channel*/) -> float {
    return static_cast<float>(input);
  };

  CustomMapper<int, float, 2>::Config config = {.mappingFunc = validFunc};
  CustomMapper<int, float, 2> mapper(source, config);

  // 测试无效通道访问
  float result = mapper.getValue(5); // 通道5超出范围

  std::cout << "  无效通道访问结果: " << result << " (期望: 0)" << std::endl;

  if (std::abs(result) < 1e-6f) {
    std::cout << "  ✓ 错误处理测试通过" << std::endl;
    return true;
  } else {
    std::cout << "  ✗ 错误处理测试失败" << std::endl;
    return false;
  }
}

/**
 * @brief 测试类型转换
 */
bool testTypeConversion() {
  std::cout << "测试类型转换..." << std::endl;

  SimpleTestSource<uint16_t, 1> source;

  // ADC值转电压映射
  auto adcToVoltage = [](uint16_t adcValue, uint8_t /*channel*/) -> double {
    const double vref = 3.3;
    const uint16_t maxAdc = 4095; // 12位ADC
    return (static_cast<double>(adcValue) * vref) / maxAdc;
  };

  CustomMapper<uint16_t, double, 1>::Config config = {.mappingFunc =
                                                          adcToVoltage};

  CustomMapper<uint16_t, double, 1> mapper(source, config);

  // 测试满量程ADC值
  source.setValue(0, 4095);
  mapper.update();
  double result = mapper.getValue(0);

  std::cout << "  ADC 4095转电压: " << result << " (期望: ~3.3)" << std::endl;

  if (std::abs(result - 3.3) < 1e-6) {
    std::cout << "  ✓ 类型转换测试通过" << std::endl;
    return true;
  } else {
    std::cout << "  ✗ 类型转换测试失败" << std::endl;
    return false;
  }
}

/**
 * @brief 主函数 - 运行所有测试
 */
int main() {
  std::cout << "自定义映射器 (CustomMapper) 功能测试" << std::endl;
  std::cout << "=====================================" << std::endl;
  std::cout << std::endl;

  int passedTests = 0;
  int totalTests = 0;

  // 运行所有测试
  totalTests++;
  if (testBasicMapping())
    passedTests++;
  std::cout << std::endl;

  totalTests++;
  if (testInternalLimiting())
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
  if (testConfigUpdate())
    passedTests++;
  std::cout << std::endl;

  totalTests++;
  if (testComplexMathFunctions())
    passedTests++;
  std::cout << std::endl;

  totalTests++;
  if (testErrorHandling())
    passedTests++;
  std::cout << std::endl;

  totalTests++;
  if (testTypeConversion())
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