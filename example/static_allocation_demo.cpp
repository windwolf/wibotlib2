/**
 * @file static_allocation_demo.cpp
 * @brief 演示静态内存分配中值滤波器的优势
 */

#include "wibotlib/src/SyncPipeline/filter/median-filter.hpp"
#include <chrono>
#include <iostream>

using namespace wibot;

/**
 * @brief 简单的测试数据源
 */
template <uint8_t CHANNELS>
class StaticTestSource : public SyncPipeline<float> {
public:
  StaticTestSource() { reset(); }

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
  float _values[CHANNELS]; // 静态分配的数组
};

/**
 * @brief 演示内存使用情况
 */
void demonstrateMemoryUsage() {
  std::cout << "=== 静态内存分配演示 ===" << std::endl;

  // 显示不同配置的内存占用
  std::cout << "MedianFilter内存占用情况:" << std::endl;
  std::cout << "- MedianFilter<1>: " << sizeof(MedianFilter<1>) << " 字节"
            << std::endl;
  std::cout << "- MedianFilter<4>: " << sizeof(MedianFilter<4>) << " 字节"
            << std::endl;
  std::cout << "- MedianFilter<8>: " << sizeof(MedianFilter<8>) << " 字节"
            << std::endl;

  // 计算理论内存使用
  constexpr size_t MAX_WINDOW = MedianFilter<1>::MAX_WINDOW_SIZE;
  std::cout << "\n理论内存计算 (MAX_WINDOW_SIZE="
            << static_cast<int>(MAX_WINDOW) << "):" << std::endl;

  for (int channels : {1, 4, 8}) {
    size_t buffers_size = channels * MAX_WINDOW * sizeof(float);
    size_t state_size = channels * (2 * sizeof(uint8_t) + sizeof(float));
    size_t temp_size = MAX_WINDOW * sizeof(float);
    size_t total = buffers_size + state_size + temp_size;

    std::cout << "- " << channels << "通道: 缓冲区=" << buffers_size
              << "字节 + 状态=" << state_size << "字节 + 临时=" << temp_size
              << "字节 = 总计=" << total << "字节" << std::endl;
  }

  std::cout << std::endl;
}

/**
 * @brief 演示确定性性能
 */
void demonstrateDeterministicPerformance() {
  std::cout << "=== 确定性性能演示 ===" << std::endl;

  StaticTestSource<4> source;
  MedianFilter<4>::Config config = {.windowSize = 7};

  // 验证配置
  std::cout << "最大窗口大小: "
            << static_cast<int>(MedianFilter<4>::MAX_WINDOW_SIZE) << std::endl;
  std::cout << "当前窗口大小: " << static_cast<int>(config.windowSize)
            << std::endl;
  std::cout << "配置是否有效: "
            << (MedianFilter<4>::isConfigValid(config) ? "是" : "否")
            << std::endl;

  // 创建滤波器（注意：这里没有任何动态内存分配）
  MedianFilter<4> filter(source, config);

  std::cout << "\n性能测试:" << std::endl;
  const int iterations = 1000;

  // 预热
  for (int i = 0; i < 10; i++) {
    for (uint8_t ch = 0; ch < 4; ch++) {
      source.setValue(ch, static_cast<float>(i + ch));
    }
    filter.update();
  }

  // 测量多次执行的一致性
  std::vector<long> times;
  for (int run = 0; run < 5; run++) {
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; i++) {
      // 模拟数据输入
      for (uint8_t ch = 0; ch < 4; ch++) {
        source.setValue(ch, static_cast<float>(i * 0.01f + ch));
      }
      filter.update();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    times.push_back(duration.count());

    std::cout << "运行 " << (run + 1) << ": " << duration.count() << " μs"
              << std::endl;
  }

  // 计算稳定性
  long min_time = *std::min_element(times.begin(), times.end());
  long max_time = *std::max_element(times.begin(), times.end());
  double variance_percent = (double)(max_time - min_time) / min_time * 100.0;

  std::cout << "性能稳定性: 变化范围 " << variance_percent << "%" << std::endl;
  std::cout << "  最快: " << min_time << " μs" << std::endl;
  std::cout << "  最慢: " << max_time << " μs" << std::endl;

  std::cout << std::endl;
}

/**
 * @brief 演示配置更新的效率
 */
void demonstrateConfigUpdate() {
  std::cout << "=== 配置更新效率演示 ===" << std::endl;

  StaticTestSource<2> source;
  MedianFilter<2>::Config config = {.windowSize = 5};
  MedianFilter<2> filter(source, config);

  std::cout << "初始配置: 窗口大小 = " << static_cast<int>(config.windowSize)
            << std::endl;

  // 输入一些数据
  for (int i = 0; i < 10; i++) {
    source.setValue(0, static_cast<float>(i));
    source.setValue(1, static_cast<float>(i * 0.5f));
    filter.update();
  }

  std::cout << "更新前输出: CH0=" << filter.getValue(0)
            << ", CH1=" << filter.getValue(1) << std::endl;

  // 测量配置更新时间（无内存分配/释放）
  auto start = std::chrono::high_resolution_clock::now();

  MedianFilter<2>::Config newConfig = {.windowSize = 15};
  filter.updateConfig(newConfig); // 这里只是重置状态，无内存操作

  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

  std::cout << "配置更新时间: " << duration.count() << " ns" << std::endl;
  std::cout << "新配置: 窗口大小 = " << static_cast<int>(newConfig.windowSize)
            << std::endl;

  // 验证配置更新后的工作
  for (int i = 0; i < 5; i++) {
    source.setValue(0, static_cast<float>(20 + i));
    source.setValue(1, static_cast<float>((20 + i) * 0.5f));
    filter.update();
  }

  std::cout << "更新后输出: CH0=" << filter.getValue(0)
            << ", CH1=" << filter.getValue(1) << std::endl;

  std::cout << std::endl;
}

/**
 * @brief 演示边界条件安全性
 */
void demonstrateBoundarySafety() {
  std::cout << "=== 边界条件安全性演示 ===" << std::endl;

  // 测试最大窗口大小
  MedianFilter<1>::Config maxConfig = {.windowSize =
                                           MedianFilter<1>::MAX_WINDOW_SIZE};
  std::cout << "最大窗口配置 (" << static_cast<int>(maxConfig.windowSize)
            << "): "
            << (MedianFilter<1>::isConfigValid(maxConfig) ? "有效" : "无效")
            << std::endl;

  // 测试超出范围的配置
  MedianFilter<1>::Config invalidConfig = {
      .windowSize = MedianFilter<1>::MAX_WINDOW_SIZE + 1};
  std::cout << "超范围配置 (" << static_cast<int>(invalidConfig.windowSize)
            << "): "
            << (MedianFilter<1>::isConfigValid(invalidConfig) ? "有效" : "无效")
            << std::endl;

  // 测试边界配置
  MedianFilter<1>::Config minConfig = {.windowSize = 1};
  std::cout << "最小窗口配置 (" << static_cast<int>(minConfig.windowSize)
            << "): "
            << (MedianFilter<1>::isConfigValid(minConfig) ? "有效" : "无效")
            << std::endl;

  MedianFilter<1>::Config zeroConfig = {.windowSize = 0};
  std::cout << "零窗口配置 (" << static_cast<int>(zeroConfig.windowSize)
            << "): "
            << (MedianFilter<1>::isConfigValid(zeroConfig) ? "有效" : "无效")
            << std::endl;

  std::cout << std::endl;
}

int main() {
  std::cout << "静态内存分配中值滤波器演示" << std::endl;
  std::cout << "===========================" << std::endl;
  std::cout << std::endl;

  // 运行所有演示
  demonstrateMemoryUsage();
  demonstrateDeterministicPerformance();
  demonstrateConfigUpdate();
  demonstrateBoundarySafety();

  std::cout << "=== 静态分配优势总结 ===" << std::endl;
  std::cout << "1. ✅ 零动态内存分配 - 无new/delete调用" << std::endl;
  std::cout << "2. ✅ 确定性性能 - 无内存分配延迟" << std::endl;
  std::cout << "3. ✅ 内存安全 - 无内存泄漏风险" << std::endl;
  std::cout << "4. ✅ 实时系统友好 - 可预测的执行时间" << std::endl;
  std::cout << "5. ✅ 编译时优化 - 更好的编译器优化" << std::endl;
  std::cout << "6. ✅ 栈上分配 - 优秀的缓存性能" << std::endl;

  return 0;
}