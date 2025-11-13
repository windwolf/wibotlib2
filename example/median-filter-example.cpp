/**
 * @file median-filter-example.cpp
 * @brief 中值滤波器使用示例
 *
 * 展示如何使用MedianFilter与其他管道组件配合使用。
 */

#include "../src/SyncPipeline/filter/median-filter.hpp"
#include <cmath>
#include <iostream>
#include <random>

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
 * @brief 演示基本中值滤波功能
 */
void demonstrateBasicMedianFiltering() {
  std::cout << "=== 基本中值滤波示例 ===" << std::endl;

  // 创建测试数据源（模拟传感器输入）
  TestDataSource<1> dataSource;

  // 配置中值滤波器
  MedianFilter<1>::Config filterConfig = {
      .windowSize = 5 // 5点窗口
  };

  // 创建中值滤波器
  MedianFilter<1> filter(dataSource, filterConfig);

  std::cout << "中值滤波器配置：" << std::endl;
  std::cout << "  窗口大小: " << static_cast<int>(filterConfig.windowSize)
            << " 点" << std::endl;
  std::cout << std::endl;

  // 模拟带脉冲噪声的信号
  float testSignal[] = {
      1.0f, 1.1f, 1.2f,  10.0f, 1.3f, // 包含一个脉冲噪声 10.0
      1.4f, 1.5f, -5.0f, 1.6f,  1.7f, // 包含一个负脉冲噪声 -5.0
      1.8f, 1.9f, 2.0f,  2.1f,  2.2f  // 正常信号
  };

  std::cout << "输入信号: 包含脉冲噪声的递增序列" << std::endl;
  std::cout << "步骤\t输入\t\t中值滤波输出" << std::endl;

  for (int i = 0; i < 15; i++) {
    // 设置数据源输入
    dataSource.setValue(0, testSignal[i]);

    // 更新滤波器
    filter.update();

    // 获取滤波结果
    float filteredOutput = filter.getValue(0);

    std::printf("%d\t%.1f\t\t%.1f\n", i + 1, testSignal[i], filteredOutput);
  }

  std::cout << std::endl;
}

/**
 * @brief 演示不同窗口大小的比较
 */
void demonstrateWindowSizeComparison() {
  std::cout << "=== 不同窗口大小比较示例 ===" << std::endl;

  // 创建测试数据源
  TestDataSource<1> dataSource;

  // 配置不同窗口大小的滤波器
  MedianFilter<1>::Config config3 = {.windowSize = 3};
  MedianFilter<1>::Config config5 = {.windowSize = 5};
  MedianFilter<1>::Config config7 = {.windowSize = 7};

  // 创建不同窗口大小的滤波器
  MedianFilter<1> filter3(dataSource, config3);
  MedianFilter<1> filter5(dataSource, config5);
  MedianFilter<1> filter7(dataSource, config7);

  std::cout << "比较不同窗口大小的滤波效果：" << std::endl;
  std::cout << "步骤\t输入\t\t窗口=3\t\t窗口=5\t\t窗口=7" << std::endl;

  // 生成带随机噪声的信号
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> noise(-0.5f, 0.5f);

  for (int i = 0; i < 20; i++) {
    // 基础信号：缓慢变化的正弦波
    float baseSignal = 2.0f + std::sin(2.0f * 3.14159f * i / 20.0f);

    // 添加随机噪声
    float noisySignal = baseSignal + static_cast<float>(noise(gen));

    // 每10个点添加一个脉冲噪声
    if (i % 10 == 5) {
      noisySignal += (i % 2 == 0) ? 3.0f : -3.0f;
    }

    // 设置数据源输入
    dataSource.setValue(0, noisySignal);

    // 更新所有滤波器
    filter3.update();
    filter5.update();
    filter7.update();

    // 获取滤波结果
    float output3 = filter3.getValue(0);
    float output5 = filter5.getValue(0);
    float output7 = filter7.getValue(0);

    std::printf("%d\t%.2f\t\t%.2f\t\t%.2f\t\t%.2f\n", i + 1, noisySignal,
                output3, output5, output7);
  }

  std::cout << std::endl;
}

/**
 * @brief 演示多通道中值滤波
 */
void demonstrateMultiChannelFiltering() {
  std::cout << "=== 多通道中值滤波示例 ===" << std::endl;

  // 创建3通道测试数据源
  TestDataSource<3> multiSource;

  // 配置多通道中值滤波器
  MedianFilter<3>::Config multiFilterConfig = {
      .windowSize = 5 // 5点窗口
  };

  // 创建3通道中值滤波器
  MedianFilter<3> multiFilter(multiSource, multiFilterConfig);

  std::cout << "3通道中值滤波器配置：" << std::endl;
  std::cout << "  窗口大小: " << static_cast<int>(multiFilterConfig.windowSize)
            << " 点" << std::endl;
  std::cout << std::endl;

  std::cout << "步骤\tCH0输入\tCH0输出\tCH1输入\tCH1输出\tCH2输入\tCH2输出"
            << std::endl;

  // 随机数生成器
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> noise(-0.3f, 0.3f);

  for (int i = 0; i < 15; i++) {
    // 为每个通道生成不同的测试信号
    float ch0_base = 1.0f + 0.1f * i; // 线性增长
    float ch1_base =
        2.0f + 0.05f * std::sin(2.0f * 3.14159f * i / 8.0f); // 正弦波
    float ch2_base = 1.5f;                                   // 常数

    // 添加噪声
    float ch0_input = ch0_base + static_cast<float>(noise(gen));
    float ch1_input = ch1_base + static_cast<float>(noise(gen));
    float ch2_input = ch2_base + static_cast<float>(noise(gen));

    // 添加脉冲噪声
    if (i == 5)
      ch0_input += 2.0f; // CH0添加正脉冲
    if (i == 8)
      ch1_input -= 1.5f; // CH1添加负脉冲
    if (i == 12)
      ch2_input += 1.8f; // CH2添加正脉冲

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

    std::printf("%d\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\n", i + 1, ch0_input,
                ch0_output, ch1_input, ch1_output, ch2_input, ch2_output);
  }

  std::cout << std::endl;
}

/**
 * @brief 演示中值滤波器与低通滤波器的对比
 */
void demonstrateComparisonWithLowpass() {
  std::cout << "=== 中值滤波与低通滤波对比 ===" << std::endl;

  // 创建测试数据源
  TestDataSource<1> dataSource;

  // 配置中值滤波器
  MedianFilter<1>::Config medianConfig = {.windowSize = 5};

  // 创建中值滤波器
  MedianFilter<1> medianFilter(dataSource, medianConfig);

  std::cout << "对比分析：中值滤波器更适合去除脉冲噪声" << std::endl;
  std::cout << "步骤\t输入\t\t中值滤波\t特点说明" << std::endl;

  // 测试不同类型的信号
  struct TestCase {
    float value;
    const char *description;
  };

  TestCase testCases[] = {{1.0f, "正常信号"},
                          {1.1f, "正常信号"},
                          {1.2f, "正常信号"},
                          {8.0f, "脉冲噪声 - 中值滤波能有效抑制"},
                          {1.3f, "正常信号"},
                          {1.4f, "正常信号"},
                          {-3.0f, "负脉冲噪声 - 中值滤波能有效抑制"},
                          {1.5f, "正常信号"},
                          {1.6f, "正常信号"},
                          {1.7f, "正常信号"}};

  for (int i = 0; i < 10; i++) {
    // 设置数据源输入
    dataSource.setValue(0, testCases[i].value);

    // 更新滤波器
    medianFilter.update();

    // 获取滤波结果
    float medianOutput = medianFilter.getValue(0);

    std::printf("%d\t%.1f\t\t%.1f\t\t%s\n", i + 1, testCases[i].value,
                medianOutput, testCases[i].description);
  }

  std::cout << std::endl;
  std::cout << "总结：" << std::endl;
  std::cout << "- 中值滤波器能够有效抑制脉冲噪声，保持信号的边缘特性"
            << std::endl;
  std::cout << "- 适用于去除椒盐噪声、毛刺等突发性干扰" << std::endl;
  std::cout << "- 对连续变化的信号保真度较高" << std::endl;
  std::cout << std::endl;
}

/**
 * @brief 主函数 - 运行所有示例
 */
int main() {
  std::cout << "中值滤波器 (MedianFilter) 使用示例" << std::endl;
  std::cout << "=====================================" << std::endl;
  std::cout << std::endl;

  // 运行各个示例
  demonstrateBasicMedianFiltering();
  demonstrateWindowSizeComparison();
  demonstrateMultiChannelFiltering();
  demonstrateComparisonWithLowpass();

  std::cout << "所有示例运行完成！" << std::endl;

  return 0;
}