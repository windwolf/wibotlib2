/**
 * @file external_buffer_example.cpp
 * @brief 演示使用外部缓冲区的中值滤波器
 */

#include "wibotlib/src/SyncPipeline/filter/median-filter.hpp"
#include <array>
#include <cmath>
#include <iostream>
#include <vector>

using namespace wibot;

/**
 * @brief 简单的测试数据源
 */
template <uint8_t CHANNELS>
class ExternalBufferTestSource : public SyncPipeline<float> {
public:
  ExternalBufferTestSource() { reset(); }

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

  void update() override {}
  void reset() override {
    for (uint8_t i = 0; i < CHANNELS; i++) {
      _values[i] = 0.0f;
    }
  }

private:
  float _values[CHANNELS];
};

/**
 * @brief 演示外部缓冲区的基本使用
 */
void demonstrateExternalBuffers() {
  std::cout << "=== 外部缓冲区使用演示 ===" << std::endl;

  constexpr uint8_t CHANNELS = 2;
  constexpr uint8_t MAX_WINDOW = 16;

  // 1. 在栈上分配外部缓冲区
  float channelBuffers[CHANNELS][MAX_WINDOW];
  float tempBuffer[MAX_WINDOW];

  // 2. 创建缓冲区指针数组
  float *bufferPtrs[CHANNELS];
  for (uint8_t i = 0; i < CHANNELS; i++) {
    bufferPtrs[i] = channelBuffers[i];
  }

  // 3. 配置滤波器
  MedianFilter<CHANNELS>::Config config = {.windowSize = 5,
                                           .maxWindowSize = MAX_WINDOW};

  std::cout << "缓冲区配置:" << std::endl;
  std::cout << "- 通道数: " << static_cast<int>(CHANNELS) << std::endl;
  std::cout << "- 窗口大小: " << static_cast<int>(config.windowSize)
            << std::endl;
  std::cout << "- 最大窗口大小: " << static_cast<int>(config.maxWindowSize)
            << std::endl;
  std::cout << "- 配置有效性: "
            << (MedianFilter<CHANNELS>::isConfigValid(config) ? "有效" : "无效")
            << std::endl;

  // 4. 创建数据源和滤波器
  ExternalBufferTestSource<CHANNELS> source;
  MedianFilter<CHANNELS> filter(source, config, bufferPtrs, tempBuffer);

  // 5. 测试滤波功能
  std::cout << "\n滤波测试:" << std::endl;
  std::cout << "步骤\tCH0输入\tCH0输出\tCH1输入\tCH1输出" << std::endl;

  for (int i = 0; i < 10; i++) {
    // 为CH0创建带脉冲噪声的信号
    float ch0_input = static_cast<float>(i);
    if (i == 4)
      ch0_input = 50.0f; // 脉冲噪声

    // 为CH1创建正弦波信号
    float ch1_input = 10.0f + 3.0f * std::sin(i * 0.5f);

    source.setValue(0, ch0_input);
    source.setValue(1, ch1_input);
    filter.update();

    float ch0_output = filter.getValue(0);
    float ch1_output = filter.getValue(1);

    std::printf("%d\t%.1f\t%.1f\t%.1f\t%.1f\n", i + 1, ch0_input, ch0_output,
                ch1_input, ch1_output);
  }

  std::cout << std::endl;
}

/**
 * @brief 演示不同大小的外部缓冲区
 */
void demonstrateDifferentBufferSizes() {
  std::cout << "=== 不同缓冲区大小演示 ===" << std::endl;

  constexpr uint8_t CHANNELS = 1;

  // 测试不同的缓冲区大小
  struct TestCase {
    uint8_t maxWindow;
    uint8_t windowSize;
    const char *description;
  };

  TestCase testCases[] = {{8, 3, "小缓冲区 - 3点窗口"},
                          {16, 7, "中等缓冲区 - 7点窗口"},
                          {32, 15, "大缓冲区 - 15点窗口"}};

  for (const auto &testCase : testCases) {
    std::cout << "\n" << testCase.description << ":" << std::endl;

    // 动态分配对应大小的缓冲区
    std::vector<float> channelBuffer(testCase.maxWindow);
    std::vector<float> tempBuffer(testCase.maxWindow);

    float *bufferPtr = channelBuffer.data();
    float *bufferPtrs[CHANNELS] = {bufferPtr};

    MedianFilter<CHANNELS>::Config config = {
        .windowSize = testCase.windowSize, .maxWindowSize = testCase.maxWindow};

    std::cout << "配置: 窗口=" << static_cast<int>(config.windowSize)
              << ", 最大=" << static_cast<int>(config.maxWindowSize)
              << ", 有效="
              << (MedianFilter<CHANNELS>::isConfigValid(config) ? "是" : "否")
              << std::endl;

    if (MedianFilter<CHANNELS>::isConfigValid(config)) {
      ExternalBufferTestSource<CHANNELS> source;
      MedianFilter<CHANNELS> filter(source, config, bufferPtrs,
                                    tempBuffer.data());

      // 简单测试
      std::cout << "测试序列: ";
      for (int i = 0; i < 8; i++) {
        float input = static_cast<float>(i % 3); // 0, 1, 2, 0, 1, 2, ...
        if (i == 4)
          input = 10.0f; // 添加一个脉冲

        source.setValue(0, input);
        filter.update();
        float output = filter.getValue(0);

        std::cout << output;
        if (i < 7)
          std::cout << ", ";
      }
      std::cout << std::endl;
    }
  }

  std::cout << std::endl;
}

/**
 * @brief 演示配置更新
 */
void demonstrateConfigUpdate() {
  std::cout << "=== 配置更新演示 ===" << std::endl;

  constexpr uint8_t CHANNELS = 1;
  constexpr uint8_t MAX_WINDOW = 20;

  // 分配缓冲区
  float channelBuffer[MAX_WINDOW];
  float tempBuffer[MAX_WINDOW];
  float *bufferPtrs[CHANNELS] = {channelBuffer};

  // 初始配置
  MedianFilter<CHANNELS>::Config config = {.windowSize = 3,
                                           .maxWindowSize = MAX_WINDOW};

  ExternalBufferTestSource<CHANNELS> source;
  MedianFilter<CHANNELS> filter(source, config, bufferPtrs, tempBuffer);

  std::cout << "初始配置: 窗口大小 = " << static_cast<int>(config.windowSize)
            << std::endl;

  // 输入一些数据
  for (int i = 0; i < 5; i++) {
    source.setValue(0, static_cast<float>(i + 1));
    filter.update();
  }
  std::cout << "初始输出: " << filter.getValue(0) << std::endl;

  // 更新配置
  MedianFilter<CHANNELS>::Config newConfig = {.windowSize = 7,
                                              .maxWindowSize = MAX_WINDOW};

  std::cout << "\n更新配置: 窗口大小 = "
            << static_cast<int>(newConfig.windowSize) << std::endl;
  filter.updateConfig(newConfig);

  // 继续测试
  for (int i = 5; i < 10; i++) {
    source.setValue(0, static_cast<float>(i + 1));
    filter.update();
  }
  std::cout << "更新后输出: " << filter.getValue(0) << std::endl;

  std::cout << std::endl;
}

/**
 * @brief 演示错误处理
 */
void demonstrateErrorHandling() {
  std::cout << "=== 错误处理演示 ===" << std::endl;

  // 测试无效配置
  std::vector<MedianFilter<1>::Config> testConfigs = {
      {0, 10}, // 窗口大小为0
      {5, 0},  // 最大窗口大小为0
      {10, 5}, // 窗口大小超过最大窗口大小
      {5, 10}, // 有效配置
  };

  for (size_t i = 0; i < testConfigs.size(); i++) {
    const auto &config = testConfigs[i];
    bool valid = MedianFilter<1>::isConfigValid(config);

    std::cout << "配置 " << (i + 1)
              << " (窗口=" << static_cast<int>(config.windowSize)
              << ", 最大=" << static_cast<int>(config.maxWindowSize)
              << "): " << (valid ? "有效" : "无效") << std::endl;
  }

  std::cout << std::endl;
}

int main() {
  std::cout << "外部缓冲区中值滤波器演示" << std::endl;
  std::cout << "========================" << std::endl;
  std::cout << std::endl;

  // 运行所有演示
  demonstrateExternalBuffers();
  demonstrateDifferentBufferSizes();
  demonstrateConfigUpdate();
  demonstrateErrorHandling();

  std::cout << "=== 外部缓冲区优势总结 ===" << std::endl;
  std::cout << "1. ✅ 用户控制内存分配 - 可选择栈、堆或静态分配" << std::endl;
  std::cout << "2. ✅ 灵活的缓冲区大小 - 根据实际需求分配" << std::endl;
  std::cout << "3. ✅ 零内部动态分配 - 滤波器内部无new/delete" << std::endl;
  std::cout << "4. ✅ 内存复用 - 多个滤波器可共享缓冲区池" << std::endl;
  std::cout << "5. ✅ 适应性强 - 适用于各种内存受限环境" << std::endl;

  return 0;
}