/**
 * @file median_filter_compile_test.cpp
 * @brief 简单的编译测试，验证中值滤波器可以正常编译
 */

#include "SyncPipeline/filter/median-filter.hpp"

using namespace wibot;

// 简单的测试源
class TestSource : public SyncPipeline<float> {
public:
  TestSource() : _value(0.0f) {}

  void setValue(float value) { _value = value; }

  float getValue(uint8_t channel) const override { return _value; }

  void update() override {}
  void reset() override { _value = 0.0f; }

private:
  float _value;
};

int main() {
  // 创建测试源
  TestSource source;

  // 创建中值滤波器配置
  MedianFilter<1>::Config config = {.windowSize = 5};

  // 验证配置
  if (!MedianFilter<1>::isConfigValid(config)) {
    return 1;
  }

  // 创建中值滤波器
  MedianFilter<1> filter(source, config);

  // 简单测试
  source.setValue(1.0f);
  filter.update();

  float result = filter.getValue(0);

  // 编译成功即可
  return 0;
}