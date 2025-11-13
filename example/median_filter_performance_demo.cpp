/**
 * @file median_filter_performance_demo.cpp
 * @brief 演示模板化优化对性能的影响
 */

#include "wibotlib/src/SyncPipeline/filter/median-filter.hpp"
#include <chrono>
#include <iostream>

using namespace wibot;

/**
 * @brief 简单的测试数据源
 */
template <uint8_t CHANNELS> class PerfTestSource : public SyncPipeline<float> {
public:
  PerfTestSource() { reset(); }

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
  float _values[CHANNELS]; // 模板化数组，编译时确定大小
};

/**
 * @brief 性能测试函数
 */
template <uint8_t CHANNELS>
void performanceTest(const std::string &testName, int iterations) {
  std::cout << "\n=== " << testName << " ===" << std::endl;
  std::cout << "通道数: " << static_cast<int>(CHANNELS) << std::endl;
  std::cout << "迭代次数: " << iterations << std::endl;

  // 创建测试组件
  PerfTestSource<CHANNELS> source;
  MedianFilter<CHANNELS>::Config config = {.windowSize = 7};
  MedianFilter<CHANNELS> filter(source, config);

  // 预热
  for (int i = 0; i < 100; i++) {
    for (uint8_t ch = 0; ch < CHANNELS; ch++) {
      source.setValue(ch, static_cast<float>(i + ch));
    }
    filter.update();
  }

  // 性能测试
  auto start = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < iterations; i++) {
    // 模拟实际使用场景
    for (uint8_t ch = 0; ch < CHANNELS; ch++) {
      source.setValue(
          ch, static_cast<float>(i * 0.1f + ch * 0.5f + std::sin(i * 0.01f)));
    }
    filter.update();

    // 读取结果（模拟实际应用）
    float sum = 0.0f;
    for (uint8_t ch = 0; ch < CHANNELS; ch++) {
      sum += filter.getValue(ch);
    }
    // 防止编译器优化掉计算
    volatile float dummy = sum;
    (void)dummy;
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  std::cout << "总耗时: " << duration.count() << " μs" << std::endl;
  std::cout << "平均每次: "
            << static_cast<double>(duration.count()) / iterations << " μs"
            << std::endl;
  std::cout << "每秒处理: "
            << static_cast<int>(
                   1000000.0 /
                   (static_cast<double>(duration.count()) / iterations))
            << " 次/秒" << std::endl;
}

/**
 * @brief 内存布局演示
 */
void memoryLayoutDemo() {
  std::cout << "\n=== 内存布局演示 ===" << std::endl;

  // 显示不同通道数的内存占用
  std::cout << "MedianFilter<1> 大小: " << sizeof(MedianFilter<1>) << " 字节"
            << std::endl;
  std::cout << "MedianFilter<4> 大小: " << sizeof(MedianFilter<4>) << " 字节"
            << std::endl;
  std::cout << "MedianFilter<8> 大小: " << sizeof(MedianFilter<8>) << " 字节"
            << std::endl;

  std::cout << "\n基础类型大小:" << std::endl;
  std::cout << "float*: " << sizeof(float *) << " 字节" << std::endl;
  std::cout << "uint8_t: " << sizeof(uint8_t) << " 字节" << std::endl;
  std::cout << "float: " << sizeof(float) << " 字节" << std::endl;

  // 计算模板化数组的内存节省
  const int CHANNELS = 4;
  size_t template_size =
      CHANNELS * (sizeof(float *) + 2 * sizeof(uint8_t) + sizeof(float));
  size_t dynamic_size = 4 * sizeof(void *); // 4个动态分配的指针

  std::cout << "\n4通道情况下:" << std::endl;
  std::cout << "模板化数组内存: " << template_size << " 字节（栈上）"
            << std::endl;
  std::cout << "动态分配指针: " << dynamic_size << " 字节（堆上）" << std::endl;
  std::cout << "节省的间接访问: "
            << (template_size > dynamic_size ? "增加" : "减少") << " "
            << abs(static_cast<int>(template_size - dynamic_size))
            << " 字节开销，但获得直接访问性能" << std::endl;
}

int main() {
  std::cout << "中值滤波器性能演示" << std::endl;
  std::cout << "===================" << std::endl;

  // 内存布局演示
  memoryLayoutDemo();

  // 不同通道数的性能测试
  const int iterations = 10000;

  performanceTest<1>("单通道性能测试", iterations);
  performanceTest<4>("4通道性能测试", iterations);
  performanceTest<8>("8通道性能测试", iterations);

  std::cout << "\n=== 总结 ===" << std::endl;
  std::cout << "1. 模板化数组提供了编译时优化的机会" << std::endl;
  std::cout << "2. 减少了动态内存分配的开销" << std::endl;
  std::cout << "3. 提供了更好的内存局部性" << std::endl;
  std::cout << "4. 编译器能够更好地优化访问模式" << std::endl;

  return 0;
}