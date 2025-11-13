#ifndef HAL_DIGITAL_SOURCE_EXAMPLE_HPP
#define HAL_DIGITAL_SOURCE_EXAMPLE_HPP

#pragma once

#include "hal-digital-source.hpp"

namespace wibot {

/**
 * @brief HAL数字输入源使用示例
 */
class HalDigitalSourceExample {
   public:
    /**
     * @brief 运行所有示例
     */
    static void runAllExamples();

    /**
     * @brief 示例1：基本GPIO读取
     */
    static void example1_BasicGpioReading();

    /**
     * @brief 示例2：多通道GPIO配置
     */
    static void example2_MultiChannelConfiguration();

    /**
     * @brief 示例3：消抖功能演示
     */
    static void example3_DebounceDemo();

    /**
     * @brief 示例4：反转功能演示
     */
    static void example4_InverseDemo();

    /**
     * @brief 示例5：实际应用场景
     */
    static void example5_RealWorldUsage();

   private:
    /**
     * @brief 打印通道状态
     */
    template <uint8_t CHANNELS>
    static void printChannelStatus(const HalDigitalSource<CHANNELS>& source);

    /**
     * @brief 模拟延时
     */
    static void simulateDelay(uint32_t ms);
};

}  // namespace wibot

#endif  // HAL_DIGITAL_SOURCE_EXAMPLE_HPP