#include "digital-source.hpp"
#include <cassert>

namespace wibot {

/**
 * @brief DigitalSource 简单测试
 */
void testDigitalSource() {
  // 测试4通道数字输入源
  DigitalSource<4> digitalSource;

  // 1. 测试基本功能
  digitalSource.updateDigitalInput(0x05); // 通道0和2为高

  assert(digitalSource.getValue(0) == true);
  assert(digitalSource.getValue(1) == false);
  assert(digitalSource.getValue(2) == true);
  assert(digitalSource.getValue(3) == false);

  // 2. 测试批量读取
  uint32_t allValues = digitalSource.getAllValues();
  assert((allValues & 0x0F) == 0x05);

  // 3. 测试反转配置
  DigitalSourceConfig config;
  config.inverse = 0x0F; // 反转所有4个通道
  config.debounceTimeMs = 50;
  digitalSource.configure(config);

  digitalSource.updateDigitalInput(0x05); // 原始：通道0和2为高

  // 反转后：通道0和2应该为低，通道1和3应该为高
  assert(digitalSource.getValue(0) == false);
  assert(digitalSource.getValue(1) == true);
  assert(digitalSource.getValue(2) == false);
  assert(digitalSource.getValue(3) == true);

  // 4. 测试重置功能
  digitalSource.reset();
  for (uint8_t ch = 0; ch < 4; ch++) {
    assert(digitalSource.getValue(ch) == false);
  }

  // 测试通过！
}

/**
 * @brief 测试不同通道数的模板实例化
 */
void testTemplateInstantiation() {
  // 测试不同通道数的实例化
  DigitalSource<1> source1;
  DigitalSource<8> source8;
  DigitalSource<16> source16;
  DigitalSource<32> source32;

  // 验证通道数
  assert(DigitalSource<1>::getChannelCount() == 1);
  assert(DigitalSource<8>::getChannelCount() == 8);
  assert(DigitalSource<16>::getChannelCount() == 16);
  assert(DigitalSource<32>::getChannelCount() == 32);

  // 基本功能测试
  source8.updateDigitalInput(0xFF);

  for (uint8_t ch = 0; ch < 8; ch++) {
    assert(source8.getValue(ch) == true);
  }
}

} // namespace wibot

/**
 * @brief 主测试函数
 */
int main() {
  wibot::testDigitalSource();
  wibot::testTemplateInstantiation();

  return 0;
}