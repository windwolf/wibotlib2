#ifndef ANALOG_INPUT_EXAMPLE_HPP
#define ANALOG_INPUT_EXAMPLE_HPP

#include "analog-input.hpp"

namespace wibot {

/**
 * @brief AnalogInput使用示例
 * 
 * 这个示例展示了如何使用AnalogInput类进行多通道ADC数据处理，
 * 包括滤波、映射、分桶等功能。
 */
class AnalogInputExample {
   public:
    AnalogInputExample();
    ~AnalogInputExample() = default;

    // 基本使用示例
    void basicUsageExample();

    // 高级配置示例
    void advancedConfigExample();

    // 校准示例
    void calibrationExample();

    // 滤波器比较示例
    void filterComparisonExample();

    // 实时数据处理示例
    void realTimeProcessingExample();

   private:
    // 4通道模拟输入实例
    AnalogInput<4> _analogInput;

    // 模拟ADC原始数据
    uint32_t _simulateAdcReading(uint8_t channel);

    // 打印通道信息
    void _printChannelInfo(uint8_t channel);

    // 打印统计信息
    void _printStatistics();
};

}  // namespace wibot

#endif  // ANALOG_INPUT_EXAMPLE_HPP