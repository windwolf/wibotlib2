#include "analog-input-example.hpp"
#include <iostream>

/**
 * @brief AnalogInput库测试程序
 * 
 * 这个程序展示了如何使用AnalogInput库的各种功能
 */
int main() {
    std::cout << "=== WibotLib3 AnalogInput 测试程序 ===" << std::endl;
    std::cout << "参考DigitalInput设计风格的多通道ADC数据处理库\n" << std::endl;

    wibot::AnalogInputExample example;

    // 运行所有示例
    example.basicUsageExample();
    example.advancedConfigExample();
    example.calibrationExample();
    example.filterComparisonExample();
    example.realTimeProcessingExample();

    std::cout << "\n=== 测试完成 ===" << std::endl;
    std::cout << "AnalogInput库功能验证成功!" << std::endl;

    return 0;
}