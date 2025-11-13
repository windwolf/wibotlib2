/**
 * @file custom-mapper-example.cpp
 * @brief 自定义映射器使用示例
 *
 * 展示如何使用CustomMapper来实现各种自定义映射逻辑。
 */

#include "../src/SyncPipeline/mapper/custom-mapper.hpp"
#include <cmath>
#include <functional>
#include <iostream>

using namespace wibot;

/**
 * @brief 简单的测试数据源
 */
template <typename T, uint8_t CHANNELS>
class TestDataSource : public SyncPipeline<T> {
public:
  TestDataSource() { reset(); }

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
 * @brief 演示基本自定义映射功能
 */
void demonstrateBasicCustomMapping() {
  std::cout << "=== 基本自定义映射示例 ===" << std::endl;

  // 创建测试数据源
  TestDataSource<int16_t, 1> dataSource;

  // 定义平方映射函数
  auto squareMapping = [](int16_t input, uint8_t channel) -> float {
    return static_cast<float>(input * input);
  };

  // 配置自定义映射器
  CustomMapper<int16_t, float, 1>::Config config = {.mappingFunc =
                                                        squareMapping};

  // 创建自定义映射器
  CustomMapper<int16_t, float, 1> mapper(dataSource, config);

  std::cout << "映射函数: f(x) = x²" << std::endl;
  std::cout << "输入\t输出" << std::endl;

  for (int16_t i = -5; i <= 5; i++) {
    dataSource.setValue(0, i);
    mapper.update();
    float result = mapper.getValue(0);

    std::printf("%d\t%.1f\n", i, result);
  }

  std::cout << std::endl;
}

/**
 * @brief 演示数学函数映射
 */
void demonstrateMathMapping() {
  std::cout << "=== 数学函数映射示例 ===" << std::endl;

  TestDataSource<float, 1> dataSource;

  // 定义指数映射函数
  auto exponentialMapping = [](float input, uint8_t channel) -> float {
    return std::exp(input);
  };

  // 配置映射器
  CustomMapper<float, float, 1>::Config config = {.mappingFunc =
                                                      exponentialMapping};

  CustomMapper<float, float, 1> mapper(dataSource, config);

  std::cout << "映射函数: f(x) = e^x" << std::endl;
  std::cout << "输入\t输出" << std::endl;

  for (float i = 0.0f; i <= 3.0f; i += 0.5f) {
    dataSource.setValue(0, i);
    mapper.update();
    float result = mapper.getValue(0);

    std::printf("%.1f\t%.2f\n", i, result);
  }

  std::cout << std::endl;
}

/**
 * @brief 演示通道相关的映射
 */
void demonstrateChannelAwareMapping() {
  std::cout << "=== 通道相关映射示例 ===" << std::endl;

  constexpr uint8_t CHANNELS = 3;
  TestDataSource<int16_t, CHANNELS> dataSource;

  // 定义通道相关的映射函数
  auto channelAwareMapping = [](int16_t input, uint8_t channel) -> float {
    switch (channel) {
    case 0:
      return input * 1.0f; // CH0: 原值
    case 1:
      return input * 0.5f; // CH1: 半值
    case 2:
      return input * 2.0f; // CH2: 双值
    default:
      return 0.0f;
    }
  };

  CustomMapper<int16_t, float, CHANNELS>::Config config = {
      .mappingFunc = channelAwareMapping};

  CustomMapper<int16_t, float, CHANNELS> mapper(dataSource, config);

  std::cout << "通道映射: CH0=1x, CH1=0.5x, CH2=2x" << std::endl;
  std::cout << "输入\tCH0输出\tCH1输出\tCH2输出" << std::endl;

  for (int16_t i = 1; i <= 5; i++) {
    // 设置所有通道相同的输入
    for (uint8_t ch = 0; ch < CHANNELS; ch++) {
      dataSource.setValue(ch, i * 10);
    }

    mapper.update();

    std::printf("%d\t%.1f\t%.1f\t%.1f\n", i * 10, mapper.getValue(0),
                mapper.getValue(1), mapper.getValue(2));
  }

  std::cout << std::endl;
}

/**
 * @brief 演示复杂的数学函数映射
 */
void demonstrateComplexMathMapping() {
  std::cout << "=== 复杂数学函数映射示例 ===" << std::endl;

  TestDataSource<float, 2> dataSource;

  // 定义复杂的数学映射函数
  auto complexMapping = [](float input, uint8_t channel) -> float {
    if (channel == 0) {
      // CH0: 正弦波映射
      return std::sin(input * M_PI / 180.0f); // 度转弧度并计算sin
    } else {
      // CH1: sigmoid函数映射
      return 1.0f / (1.0f + std::exp(-input));
    }
  };

  CustomMapper<float, float, 2>::Config config = {.mappingFunc =
                                                      complexMapping};

  CustomMapper<float, float, 2> mapper(dataSource, config);

  std::cout << "CH0: sin(x°), CH1: sigmoid(x)" << std::endl;
  std::cout << "输入\tCH0(sin)\tCH1(sigmoid)" << std::endl;

  for (float angle = 0.0f; angle <= 180.0f; angle += 30.0f) {
    dataSource.setValue(0, angle);                   // CH0输入角度
    dataSource.setValue(1, (angle - 90.0f) / 30.0f); // CH1输入归一化值

    mapper.update();

    std::printf("%.0f\t%.3f\t\t%.3f\n", angle, mapper.getValue(0),
                mapper.getValue(1));
  }

  std::cout << std::endl;
}

/**
 * @brief 演示带限制的映射（在映射函数内部实现限制）
 */
void demonstrateMappingWithInternalLimiting() {
  std::cout << "=== 内部限制映射示例 ===" << std::endl;

  TestDataSource<float, 1> dataSource;

  // 在映射函数内部实现输出限制
  auto clampedExponentialMapping = [](float input, uint8_t channel) -> float {
    float result = std::exp(input);
    // 在函数内部进行限制
    const float minOutput = 0.0f;
    const float maxOutput = 10.0f;

    if (result < minOutput)
      result = minOutput;
    if (result > maxOutput)
      result = maxOutput;

    return result;
  };

  CustomMapper<float, float, 1>::Config config = {
      .mappingFunc = clampedExponentialMapping};

  CustomMapper<float, float, 1> mapper(dataSource, config);

  std::cout << "映射函数: f(x) = clamp(e^x, 0, 10)" << std::endl;
  std::cout << "输入\t原始e^x\t限制后输出" << std::endl;

  for (float i = 0.0f; i <= 4.0f; i += 0.5f) {
    dataSource.setValue(0, i);
    mapper.update();
    float result = mapper.getValue(0);
    float original = std::exp(i);

    std::printf("%.1f\t%.2f\t\t%.2f\n", i, original, result);
  }

  std::cout << std::endl;
}

/**
 * @brief 演示配置验证和错误处理
 */
void demonstrateConfigValidation() {
  std::cout << "=== 配置验证和错误处理示例 ===" << std::endl;

  TestDataSource<int, 1> dataSource;

  // 测试有效配置
  auto validMapping = [](int input, uint8_t channel) -> double {
    return static_cast<double>(input);
  };

  CustomMapper<int, double, 1>::Config validConfig = {.mappingFunc =
                                                          validMapping};

  std::cout << "有效配置验证: "
            << (CustomMapper<int, double, 1>::isConfigValid(validConfig)
                    ? "通过"
                    : "失败")
            << std::endl;

  // 测试无效配置：空映射函数
  CustomMapper<int, double, 1>::Config invalidConfig = {.mappingFunc = nullptr};

  std::cout << "无效配置（空函数）: "
            << (CustomMapper<int, double, 1>::isConfigValid(invalidConfig)
                    ? "通过"
                    : "失败")
            << std::endl;

  std::cout << std::endl;
}

/**
 * @brief 演示配置更新
 */
void demonstrateConfigUpdate() {
  std::cout << "=== 配置更新示例 ===" << std::endl;

  TestDataSource<float, 1> dataSource;

  // 初始映射函数：线性
  auto linearMapping = [](float input, uint8_t channel) -> float {
    return input * 2.0f;
  };

  CustomMapper<float, float, 1>::Config config = {.mappingFunc = linearMapping};

  CustomMapper<float, float, 1> mapper(dataSource, config);

  std::cout << "初始映射: f(x) = 2x" << std::endl;
  dataSource.setValue(0, 5.0f);
  mapper.update();
  std::cout << "输入5.0，输出: " << mapper.getValue(0) << std::endl;

  // 更新为平方根映射
  auto sqrtMapping = [](float input, uint8_t channel) -> float {
    return std::sqrt(std::abs(input));
  };

  CustomMapper<float, float, 1>::Config newConfig = {.mappingFunc =
                                                         sqrtMapping};

  std::cout << "\n更新映射: f(x) = √|x|" << std::endl;
  mapper.updateConfig(newConfig);
  mapper.update();
  std::cout << "输入5.0，输出: " << mapper.getValue(0) << std::endl;

  std::cout << std::endl;
}

/**
 * @brief 演示实用的传感器映射场景
 */
void demonstratePracticalSensorMapping() {
  std::cout << "=== 实用传感器映射示例 ===" << std::endl;

  TestDataSource<uint16_t, 3> dataSource;

  // 实用的传感器映射：ADC值转换为物理量
  auto sensorMapping = [](uint16_t adcValue, uint8_t channel) -> float {
    const float vref = 3.3f;
    const uint16_t maxAdc = 4095; // 12位ADC
    float voltage = (adcValue * vref) / maxAdc;

    switch (channel) {
    case 0:
      // CH0: 温度传感器 (假设线性关系)
      return voltage * 100.0f - 50.0f; // 转换为摄氏度
    case 1:
      // CH1: 光照传感器
      return voltage * 1000.0f; // 转换为Lux
    case 2:
      // CH2: 压力传感器
      return voltage * 200.0f; // 转换为kPa
    default:
      return voltage;
    }
  };

  CustomMapper<uint16_t, float, 3>::Config config = {.mappingFunc =
                                                         sensorMapping};

  CustomMapper<uint16_t, float, 3> mapper(dataSource, config);

  std::cout << "传感器映射: CH0=温度(°C), CH1=光照(Lux), CH2=压力(kPa)"
            << std::endl;
  std::cout << "ADC值\t温度(°C)\t光照(Lux)\t压力(kPa)" << std::endl;

  uint16_t testValues[] = {1024, 2048, 3072, 4095};

  for (auto adcValue : testValues) {
    // 设置所有通道相同的ADC值
    for (uint8_t ch = 0; ch < 3; ch++) {
      dataSource.setValue(ch, adcValue);
    }

    mapper.update();

    std::printf("%d\t%.1f\t\t%.1f\t\t%.1f\n", adcValue, mapper.getValue(0),
                mapper.getValue(1), mapper.getValue(2));
  }

  std::cout << std::endl;
}

/**
 * @brief 主函数 - 运行所有示例
 */
int main() {
  std::cout << "自定义映射器 (CustomMapper) 使用示例" << std::endl;
  std::cout << "=====================================" << std::endl;
  std::cout << std::endl;

  // 运行各个示例
  demonstrateBasicCustomMapping();
  demonstrateMathMapping();
  demonstrateChannelAwareMapping();
  demonstrateComplexMathMapping();
  demonstrateMappingWithInternalLimiting();
  demonstrateConfigValidation();
  demonstrateConfigUpdate();
  demonstratePracticalSensorMapping();

  std::cout << "所有示例运行完成！" << std::endl;

  return 0;
}