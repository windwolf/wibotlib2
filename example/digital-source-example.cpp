#include "digital-source-example.hpp"
#include "system.hpp"

namespace wibot {

DigitalSourceExample::DigitalSourceExample()
    : _digitalSource(0, 50), // 无反转，50ms消抖
      _simulatedGpioState(0) {}

void DigitalSourceExample::basicUsageExample() {
  // 基本使用示例

  // 1. 配置数字输入源
  DigitalSourceConfig config;
  config.inverse = 0x03;      // 反转通道0和通道1
  config.debounceTimeMs = 30; // 30ms消抖
  _digitalSource.configure(config);

  // 2. 模拟GPIO状态更新并直接处理
  _simulateGpioUpdate();

  // 4. 读取各通道值
  for (uint8_t ch = 0; ch < 8; ch++) {
    bool value = _digitalSource.getValue(ch);
    // 使用通道值...
  }

  // 5. 批量获取所有通道值
  uint32_t allValues = _digitalSource.getAllValues();
  // 处理所有通道的位掩码...
}

void DigitalSourceExample::multiChannelConfigExample() {
  // 多通道配置示例

  // 1. 分别配置不同通道
  _digitalSource.configureChannel(0, true, 20);  // 通道0反转，20ms消抖
  _digitalSource.configureChannel(1, false, 50); // 通道1正常，50ms消抖
  _digitalSource.configureChannel(2, true, 100); // 通道2反转，100ms消抖

  // 2. 批量配置所有通道
  DigitalSourceConfig batchConfig;
  batchConfig.inverse = 0x55;      // 反转奇数通道(0b01010101)
  batchConfig.debounceTimeMs = 75; // 75ms消抖时间
  _digitalSource.configure(batchConfig);

  // 3. 更新和读取
  _simulateGpioUpdate();

  for (uint8_t ch = 0; ch < 4; ch++) {
    bool value = _digitalSource.getValue(ch);
    // 处理各通道值...
  }
}

void DigitalSourceExample::pipelineIntegrationExample() {
  // Pipeline集成使用示例

  // 1. 作为SourcePipeline使用
  SyncPipeline<bool> *source = &_digitalSource;

  // 2. Pipeline标准接口操作
  source->reset(); // 重置Pipeline状态

  // 3. 循环处理
  for (int i = 0; i < 100; i++) {
    // 更新原始输入
    _simulateGpioUpdate();

    // 读取Pipeline输出
    for (uint8_t ch = 0; ch < 4; ch++) {
      bool value = source->getValue(ch);
      // 后续Pipeline处理...
    }

    System::delayMs(50);
  }

  // 4. 直接获取处理后的值
  uint32_t processedValues = _digitalSource.getAllValues();
}

void DigitalSourceExample::_simulateGpioUpdate() {
  // 模拟GPIO状态变化
  static uint32_t counter = 0;
  counter++;

  // 简单的模拟：让某些通道周期性变化
  _simulatedGpioState = 0;

  // 通道0: 每4次循环翻转一次
  if ((counter / 4) & 1) {
    _simulatedGpioState |= 0x01;
  }

  // 通道1: 每8次循环翻转一次
  if ((counter / 8) & 1) {
    _simulatedGpioState |= 0x02;
  }

  // 通道2: 每16次循环翻转一次
  if ((counter / 16) & 1) {
    _simulatedGpioState |= 0x04;
  }

  // 通道3: 随机状态
  if ((counter * 137) & 0x100) { // 简单的伪随机
    _simulatedGpioState |= 0x08;
  }

  // 更新到DigitalSource
  _digitalSource.updateDigitalInput(_simulatedGpioState);
}

} // namespace wibot