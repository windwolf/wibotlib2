/**
 * @file binning-mapper.test.cpp
 * @brief BinningMapper 单元测试
 *
 * 测试分桶映射器的多通道支持、区间映射、滞回功能等特性
 */

#include "../src/SyncPipeline/mapper/binning-mapper.hpp"
#include <cassert>
#include <cstdio>

namespace wibot {

/**
 * @brief 模拟上游数据源
 */
template <uint8_t CHANNELS> class MockFloatSource : public SyncPipeline<float> {
public:
  MockFloatSource() {
    for (uint8_t i = 0; i < CHANNELS; ++i) {
      _values[i] = 0.0f;
    }
  }

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
    // 模拟数据源，无需实际更新
  }

  void reset() override {
    for (uint8_t i = 0; i < CHANNELS; ++i) {
      _values[i] = 0.0f;
    }
  }

private:
  float _values[CHANNELS];
};

/**
 * @brief 测试基本分桶功能（无滞回）
 */
void test_basic_binning() {
  printf("测试基本分桶功能...\n");

  MockFloatSource<1> source;

  // 定义3个区间: [0,10), [10,20), [20,30)
  BinRange ranges[3] = {BinRange(0.0f, 10.0f), BinRange(10.0f, 20.0f),
                        BinRange(20.0f, 30.0f)};

  BinningMapper<1>::Config config;
  config.ranges = ranges;
  config.binCount = 3;
  config.hysteresisWidth = 0.0f;
  config.enableHysteresis = false;
  config.clampToRange = true;

  BinningMapper<1> mapper(source, config);

  // 测试第一个区间
  source.setValue(0, 5.0f);
  mapper.update();
  uint32_t result = mapper.getValue(0);
  assert(result == 0);
  printf("  ✓ 第一个区间测试通过 (值=5.0 -> 区间%u)\n", result);

  // 测试第二个区间
  source.setValue(0, 15.0f);
  mapper.update();
  result = mapper.getValue(0);
  assert(result == 1);
  printf("  ✓ 第二个区间测试通过 (值=15.0 -> 区间%u)\n", result);

  // 测试第三个区间
  source.setValue(0, 25.0f);
  mapper.update();
  result = mapper.getValue(0);
  assert(result == 2);
  printf("  ✓ 第三个区间测试通过 (值=25.0 -> 区间%u)\n", result);

  // 测试边界值
  source.setValue(0, 10.0f);
  mapper.update();
  result = mapper.getValue(0);
  assert(result == 1);
  printf("  ✓ 边界值测试通过 (值=10.0 -> 区间%u)\n", result);

  // 测试超出范围
  source.setValue(0, 35.0f);
  mapper.update();
  result = mapper.getValue(0);
  assert(result == 2); // clampToRange=true，应该钳制到最后一个区间
  printf("  ✓ 超出范围测试通过 (值=35.0 -> 区间%u，已钳制)\n", result);

  printf("基本分桶功能测试完成！\n\n");
}

/**
 * @brief 测试多通道功能
 */
void test_multichannel() {
  printf("测试多通道功能...\n");

  MockFloatSource<3> source;

  BinRange ranges[3] = {BinRange(0.0f, 10.0f), BinRange(10.0f, 20.0f),
                        BinRange(20.0f, 30.0f)};

  BinningMapper<3>::Config config;
  config.ranges = ranges;
  config.binCount = 3;
  config.hysteresisWidth = 0.0f;
  config.enableHysteresis = false;
  config.clampToRange = true;

  BinningMapper<3> mapper(source, config);

  // 设置不同通道的值
  source.setValue(0, 5.0f);  // 通道0 -> 区间0
  source.setValue(1, 15.0f); // 通道1 -> 区间1
  source.setValue(2, 25.0f); // 通道2 -> 区间2

  mapper.update();

  uint32_t result0 = mapper.getValue(0);
  uint32_t result1 = mapper.getValue(1);
  uint32_t result2 = mapper.getValue(2);

  assert(result0 == 0);
  assert(result1 == 1);
  assert(result2 == 2);

  printf("  ✓ 多通道映射测试通过 (通道0=%u, 通道1=%u, 通道2=%u)\n", result0,
         result1, result2);

  // 测试无效通道（应返回INVALID_BIN_INDEX）
  uint32_t invalidResult = mapper.getValue(5);
  assert(invalidResult == BinningMapper<3>::INVALID_BIN_INDEX);
  printf("  ✓ 无效通道测试通过 (通道5 -> %u)\n", invalidResult);

  printf("多通道功能测试完成！\n\n");
}

/**
 * @brief 测试滞回功能
 */
void test_hysteresis() {
  printf("测试滞回功能...\n");

  MockFloatSource<1> source;

  // 定义2个区间: [0,10), [10,20)，滞回宽度2.0
  BinRange ranges[2] = {BinRange(0.0f, 10.0f), BinRange(10.0f, 20.0f)};

  BinningMapper<1>::Config config;
  config.ranges = ranges;
  config.binCount = 2;
  config.hysteresisWidth = 2.0f; // 边界10.0±1.0的滞回区间
  config.enableHysteresis = true;
  config.clampToRange = true;

  BinningMapper<1> mapper(source, config);

  // 从区间0开始
  source.setValue(0, 5.0f);
  mapper.update();
  uint32_t result = mapper.getValue(0);
  assert(result == 0);
  printf("  ✓ 初始值测试 (值=5.0 -> 区间%u)\n", result);

  // 进入滞回区间（从下方），应保持区间0
  source.setValue(0, 9.5f);
  mapper.update();
  result = mapper.getValue(0);
  assert(result == 0);
  printf("  ✓ 滞回区间测试1 (值=9.5 -> 区间%u，保持)\n", result);

  // 超出滞回区间，跳到区间1
  source.setValue(0, 11.5f);
  mapper.update();
  result = mapper.getValue(0);
  assert(result == 1);
  printf("  ✓ 跳转测试 (值=11.5 -> 区间%u)\n", result);

  // 回到滞回区间（从上方），应保持区间1
  source.setValue(0, 10.5f);
  mapper.update();
  result = mapper.getValue(0);
  assert(result == 1);
  printf("  ✓ 滞回区间测试2 (值=10.5 -> 区间%u，保持)\n", result);

  // 超出滞回区间下界，跳回区间0
  source.setValue(0, 8.5f);
  mapper.update();
  result = mapper.getValue(0);
  assert(result == 0);
  printf("  ✓ 反向跳转测试 (值=8.5 -> 区间%u)\n", result);

  printf("滞回功能测试完成！\n\n");
}

} // namespace wibot

// 主测试函数
int main() {
  printf("=== BinningMapper 单元测试开始 ===\n\n");

  wibot::test_basic_binning();
  wibot::test_multichannel();
  wibot::test_hysteresis();

  printf("🎉 所有测试通过！\n");
  return 0;
}
