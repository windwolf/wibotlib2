/**
 * @file lowpass-filter-example.cpp
 * @brief 一阶低通滤波器使用示例
 *
 * 展示如何使用LowpassFilter与其他管道组件配合使用。
 */

#include "lowpass-filter.hpp"
#include <cmath>
#include <iostream>

using namespace wibot;

/**
 * @brief 简单的测试数据源（用于演示）
 */
template <uint8_t CHANNELS> class TestDataSource : public SyncPipeline<float> {
public:
  TestDataSource() { reset(); }

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
    // 测试数据源不需要更新逻辑
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
 * @brief 演示基本低通滤波功能
 */
void demonstrateBasicFiltering() {
  std::cout << "=== 基本低通滤波示例 ===" << std::endl;

  // 创建测试数据源（模拟传感器输入）
  TestDataSource<1> dataSource;

  // 配置低通滤波器
  LowpassFilter<1>::Config filterConfig = {
      .sampleTime = 0.01f, // 10ms采样间隔 (100Hz)
      .cutoffFreq = 5.0f,  // 5Hz截止频率
      .wrapValue = 0.0f,   // 不启用折叠
      .initValue = 0.0f    // 初始值为0
  };

  // 创建低通滤波器
  LowpassFilter<1> filter(dataSource, filterConfig);

  std::cout << "滤波器配置：" << std::endl;
  std::cout << "  采样频率: " << (1.0f / filterConfig.sampleTime) << " Hz"
            << std::endl;
  std::cout << "  截止频率: " << filterConfig.cutoffFreq << " Hz" << std::endl;
  std::cout << "  理论-3dB频率: " << filterConfig.cutoffFreq << " Hz"
            << std::endl;
  std::cout << std::endl;

  // 模拟带噪声的步进信号
  std::cout << "输入信号: 1.0 + 0.5*sin(20π*t) (1.0V DC + 10Hz正弦波噪声)"
            << std::endl;
  std::cout << "时间(s)\t输入\t\t滤波输出" << std::endl;

  for (int i = 0; i < 50; i++) {
    float t = i * filterConfig.sampleTime; // 当前时间

    // 生成测试信号：1.0V DC + 10Hz正弦波噪声
    float noisySignal = 1.0f + 0.5f * std::sin(2.0f * 3.14159f * 10.0f * t);

    // 设置数据源输入
    dataSource.setValue(0, noisySignal);

    // 更新滤波器
    filter.update();

    // 获取滤波结果
    float filteredOutput = filter.getValue(0);

    // 每5个采样点打印一次结果
    if (i % 5 == 0) {
      std::printf("%.3f\t\t%.3f\t\t%.3f\n", t, noisySignal, filteredOutput);
    }
  }

  std::cout << std::endl;
}

/**
 * @brief 演示角度滤波（折叠值功能）
 */
void demonstrateAngleFiltering() {
  std::cout << "=== 角度滤波示例（折叠值功能） ===" << std::endl;

  // 创建测试数据源
  TestDataSource<1> angleSource;

  // 配置低通滤波器用于角度滤波
  LowpassFilter<1>::Config angleFilterConfig = {
      .sampleTime = 0.01f,          // 10ms采样间隔
      .cutoffFreq = 2.0f,           // 2Hz截止频率
      .wrapValue = 2.0f * 3.14159f, // 2π弧度折叠（360度）
      .initValue = 0.0f             // 初始角度为0
  };

  // 创建角度滤波器
  LowpassFilter<1> angleFilter(angleSource, angleFilterConfig);

  std::cout << "角度滤波器配置：" << std::endl;
  std::cout << "  截止频率: " << angleFilterConfig.cutoffFreq << " Hz"
            << std::endl;
  std::cout << "  折叠值: 2π ("
            << (angleFilterConfig.wrapValue * 180.0f / 3.14159f) << "°)"
            << std::endl;
  std::cout << std::endl;

  // 模拟角度跳跃（从350°跳到10°）
  float angles[] = {
      6.11f, // ~350°
      6.19f, // ~355°
      0.17f, // ~10° (跳跃)
      0.26f, // ~15°
      0.35f  // ~20°
  };

  std::cout << "角度输入序列（模拟从350°跳跃到10°）：" << std::endl;
  std::cout << "步骤\t输入角度(rad)\t输入角度(°)\t滤波输出(rad)\t滤波输出(°)"
            << std::endl;

  for (int i = 0; i < 5; i++) {
    // 设置角度输入
    angleSource.setValue(0, angles[i]);

    // 更新滤波器
    angleFilter.update();

    // 获取滤波结果
    float filteredAngle = angleFilter.getValue(0);

    std::printf("%d\t%.2f\t\t\t%.1f°\t\t%.2f\t\t\t%.1f°\n", i + 1, angles[i],
                angles[i] * 180.0f / 3.14159f, filteredAngle,
                filteredAngle * 180.0f / 3.14159f);
  }

  std::cout << std::endl;
}

/**
 * @brief 演示多通道滤波
 */
void demonstrateMultiChannelFiltering() {
  std::cout << "=== 多通道滤波示例 ===" << std::endl;

  // 创建3通道测试数据源
  TestDataSource<3> multiSource;

  // 配置多通道滤波器
  LowpassFilter<3>::Config multiFilterConfig = {
      .sampleTime = 0.02f, // 20ms采样间隔 (50Hz)
      .cutoffFreq = 3.0f,  // 3Hz截止频率
      .wrapValue = 0.0f,   // 不启用折叠
      .initValue = 0.0f    // 初始值为0
  };

  // 创建3通道低通滤波器
  LowpassFilter<3> multiFilter(multiSource, multiFilterConfig);

  std::cout << "3通道滤波器配置：" << std::endl;
  std::cout << "  采样频率: " << (1.0f / multiFilterConfig.sampleTime) << " Hz"
            << std::endl;
  std::cout << "  截止频率: " << multiFilterConfig.cutoffFreq << " Hz"
            << std::endl;
  std::cout << std::endl;

  std::cout << "步骤\tCH0输入\tCH0输出\tCH1输入\tCH1输出\tCH2输入\tCH2输出"
            << std::endl;

  for (int i = 0; i < 20; i++) {
    float t = i * multiFilterConfig.sampleTime;

    // 为每个通道生成不同的测试信号
    float ch0_input =
        2.0f + 0.8f * std::sin(2.0f * 3.14159f * 8.0f * t); // 2V + 8Hz噪声
    float ch1_input =
        1.5f + 0.6f * std::sin(2.0f * 3.14159f * 12.0f * t); // 1.5V + 12Hz噪声
    float ch2_input =
        3.0f + 0.4f * std::sin(2.0f * 3.14159f * 15.0f * t); // 3V + 15Hz噪声

    // 设置多通道输入
    multiSource.setValue(0, ch0_input);
    multiSource.setValue(1, ch1_input);
    multiSource.setValue(2, ch2_input);

    // 更新滤波器
    multiFilter.update();

    // 获取滤波结果
    float ch0_output = multiFilter.getValue(0);
    float ch1_output = multiFilter.getValue(1);
    float ch2_output = multiFilter.getValue(2);

    // 每4个采样点打印一次结果
    if (i % 4 == 0) {
      std::printf("%d\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\n", i + 1, ch0_input,
                  ch0_output, ch1_input, ch1_output, ch2_input, ch2_output);
    }
  }

  std::cout << std::endl;
}

/**
 * @brief 主函数 - 运行所有示例
 */
int main() {
  std::cout << "一阶低通滤波器 (LowpassFilter) 使用示例" << std::endl;
  std::cout << "=========================================" << std::endl;
  std::cout << std::endl;

  // 运行各个示例
  demonstrateBasicFiltering();
  demonstrateAngleFiltering();
  demonstrateMultiChannelFiltering();

  std::cout << "所有示例运行完成！" << std::endl;

  return 0;
}