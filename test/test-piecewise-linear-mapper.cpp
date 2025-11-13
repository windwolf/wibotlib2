/**
 * @file test-piecewise-linear-mapper.cpp
 * @brief 分段线性映射器单元测试
 */

#include "piecewise-linear-mapper.hpp"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace wibot;

// 简化的测试数据源
template <uint8_t CHANNELS>
class TestDataSource : public SyncPipeline<int16_t> {
public:
  TestDataSource() : _values{} {}

  void setValue(uint8_t channel, int16_t value) {
    if (channel < CHANNELS) {
      _values[channel] = value;
    }
  }

  int16_t getValue(uint8_t channel) const override {
    return (channel < CHANNELS) ? _values[channel] : 0;
  }

  void update() override {}
  void reset() override {
    for (auto &value : _values) {
      value = 0;
    }
  }

private:
  int16_t _values[CHANNELS];
};

// 浮点数比较函数
bool isFloatEqual(float a, float b, float epsilon = 0.001f) {
  return std::abs(a - b) < epsilon;
}

// 测试基本线性插值功能
void testBasicInterpolation() {
  std::cout << "测试基本线性插值..." << std::endl;

  TestDataSource<1> source;

  // 创建简单的2段配置
  PiecewiseLinearMapper<1, 2>::Config config;
  config.inputPoints[0] = 0.0f;
  config.inputPoints[1] = 1000.0f;
  config.inputPoints[2] = 2000.0f;

  config.outputPoints[0] = 0.0f;
  config.outputPoints[1] = 5.0f;
  config.outputPoints[2] = 10.0f;

  config.clampOutput = false;
  config.enableExtrapolation = false;

  PiecewiseLinearMapper<1, 2> mapper(source, config);

  // 测试控制点
  source.setValue(0, 0);
  mapper.update();
  assert(isFloatEqual(mapper.getValue(0), 0.0f));

  source.setValue(0, 1000);
  mapper.update();
  assert(isFloatEqual(mapper.getValue(0), 5.0f));

  source.setValue(0, 2000);
  mapper.update();
  assert(isFloatEqual(mapper.getValue(0), 10.0f));

  // 测试中间值
  source.setValue(0, 500);
  mapper.update();
  assert(isFloatEqual(mapper.getValue(0), 2.5f));

  source.setValue(0, 1500);
  mapper.update();
  assert(isFloatEqual(mapper.getValue(0), 7.5f));

  std::cout << "✓ 基本线性插值测试通过" << std::endl;
}

// 测试边界处理
void testBoundaryHandling() {
  std::cout << "测试边界处理..." << std::endl;

  TestDataSource<1> source;

  PiecewiseLinearMapper<1, 2>::Config config;
  config.inputPoints[0] = 100.0f;
  config.inputPoints[1] = 200.0f;
  config.inputPoints[2] = 300.0f;

  config.outputPoints[0] = 1.0f;
  config.outputPoints[1] = 2.0f;
  config.outputPoints[2] = 3.0f;

  config.clampOutput = true;
  config.enableExtrapolation = false;

  PiecewiseLinearMapper<1, 2> mapper(source, config);

  // 测试超出下界
  source.setValue(0, 50);
  mapper.update();
  assert(isFloatEqual(mapper.getValue(0), 1.0f));

  // 测试超出上界
  source.setValue(0, 400);
  mapper.update();
  assert(isFloatEqual(mapper.getValue(0), 3.0f));

  std::cout << "✓ 边界处理测试通过" << std::endl;
}

// 测试外推功能
void testExtrapolation() {
  std::cout << "测试外推功能..." << std::endl;

  TestDataSource<1> source;

  PiecewiseLinearMapper<1, 1>::Config config;
  config.inputPoints[0] = 0.0f;
  config.inputPoints[1] = 100.0f;

  config.outputPoints[0] = 0.0f;
  config.outputPoints[1] = 10.0f;

  config.clampOutput = false;
  config.enableExtrapolation = true;

  PiecewiseLinearMapper<1, 1> mapper(source, config);

  // 测试向下外推
  source.setValue(0, -50);
  mapper.update();
  assert(isFloatEqual(mapper.getValue(0), -5.0f));

  // 测试向上外推
  source.setValue(0, 150);
  mapper.update();
  assert(isFloatEqual(mapper.getValue(0), 15.0f));

  std::cout << "✓ 外推功能测试通过" << std::endl;
}

// 测试多通道功能
void testMultiChannel() {
  std::cout << "测试多通道功能..." << std::endl;

  TestDataSource<4> source;

  PiecewiseLinearMapper<4, 1>::Config config;
  config.inputPoints[0] = 0.0f;
  config.inputPoints[1] = 1000.0f;

  config.outputPoints[0] = 0.0f;
  config.outputPoints[1] = 5.0f;

  config.clampOutput = false;
  config.enableExtrapolation = false;

  PiecewiseLinearMapper<4, 1> mapper(source, config);

  // 设置不同通道的值
  source.setValue(0, 0);
  source.setValue(1, 250);
  source.setValue(2, 500);
  source.setValue(3, 1000);

  mapper.update();

  // 验证各通道输出
  assert(isFloatEqual(mapper.getValue(0), 0.0f));
  assert(isFloatEqual(mapper.getValue(1), 1.25f));
  assert(isFloatEqual(mapper.getValue(2), 2.5f));
  assert(isFloatEqual(mapper.getValue(3), 5.0f));

  // 测试无效通道
  assert(isFloatEqual(mapper.getValue(4), 0.0f));

  std::cout << "✓ 多通道功能测试通过" << std::endl;
}

// 测试配置验证
void testConfigValidation() {
  std::cout << "测试配置验证..." << std::endl;

  // 测试有效配置
  PiecewiseLinearMapper<1, 2>::Config validConfig;
  validConfig.inputPoints[0] = 0.0f;
  validConfig.inputPoints[1] = 100.0f;
  validConfig.inputPoints[2] = 200.0f;

  assert(PiecewiseLinearMapper<1, 2>::isConfigValid(validConfig));

  // 测试无效配置（降序）
  PiecewiseLinearMapper<1, 2>::Config invalidConfig1;
  invalidConfig1.inputPoints[0] = 200.0f;
  invalidConfig1.inputPoints[1] = 100.0f;
  invalidConfig1.inputPoints[2] = 0.0f;

  assert(!PiecewiseLinearMapper<1, 2>::isConfigValid(invalidConfig1));

  // 测试无效配置（相等点）
  PiecewiseLinearMapper<1, 2>::Config invalidConfig2;
  invalidConfig2.inputPoints[0] = 100.0f;
  invalidConfig2.inputPoints[1] = 100.0f;
  invalidConfig2.inputPoints[2] = 200.0f;

  assert(!PiecewiseLinearMapper<1, 2>::isConfigValid(invalidConfig2));

  std::cout << "✓ 配置验证测试通过" << std::endl;
}

int main() {
  std::cout << "=== 分段线性映射器单元测试 ===" << std::endl;

  try {
    testBasicInterpolation();
    testBoundaryHandling();
    testExtrapolation();
    testMultiChannel();
    testConfigValidation();

    std::cout << "\n🎉 所有测试通过！" << std::endl;
    return 0;
  } catch (const std::exception &e) {
    std::cout << "\n❌ 测试失败: " << e.what() << std::endl;
    return -1;
  } catch (...) {
    std::cout << "\n❌ 未知测试错误" << std::endl;
    return -1;
  }
}