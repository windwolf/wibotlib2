#ifndef DIGITAL_SOURCE_EXAMPLE_HPP
#define DIGITAL_SOURCE_EXAMPLE_HPP

#pragma once

#include "digital-source.hpp"

namespace wibot {

/**
 * @brief DigitalSource 使用示例
 *
 * 本示例展示了如何使用DigitalSource作为Pipeline数据源：
 * 1. 基本的数字输入读取
 * 2. 配置反转和消抖
 * 3. 数字输入状态统计功能
 */
class DigitalSourceExample {
public:
  DigitalSourceExample();

  /**
   * @brief 基本数字输入读取示例
   */
  void basicUsageExample();

  /**
   * @brief 多通道配置示例
   */
  void multiChannelConfigExample();

  /**
   * @brief Pipeline集成示例
   */
  void pipelineIntegrationExample();

private:
  // 8通道数字输入源
  DigitalSource<8> _digitalSource;

  // 模拟GPIO状态更新
  void _simulateGpioUpdate();

  // 当前模拟的GPIO状态
  uint32_t _simulatedGpioState;
};

} // namespace wibot

#endif // DIGITAL_SOURCE_EXAMPLE_HPP