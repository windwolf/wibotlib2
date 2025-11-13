#include "hal/adc-source.hpp"
#include <iostream>

using namespace wibot;

void testAdcResolution(uint8_t resolution, const char *name) {
  std::cout << "\n=== 测试 " << name << " ===\n";

  AdcSourceConfig config = {.adcResolution = resolution,
                            .calibrationOffset = 0};

  AdcInput1CH adc(config);

  uint32_t maxValue = (1U << resolution) - 1;

  // 测试几个关键值
  uint32_t testValues[] = {0, maxValue / 4, maxValue / 2, maxValue * 3 / 4,
                           maxValue};

  std::cout << "ADC分辨率: " << (int)resolution << "位, 最大值: " << maxValue
            << "\n";
  std::cout << "原始值 -> int16_t转换:\n";

  for (uint32_t rawValue : testValues) {
    adc.setRawValue(rawValue, 0);
    int16_t converted = adc.getValue(0);

    float percentage = (float)rawValue / maxValue * 100.0f;
    std::cout << "  " << rawValue << " (" << percentage << "%) -> " << converted
              << "\n";
  }
}

int main() {
  std::cout << "ADC分辨率转换测试\n";

  // 测试不同分辨率
  testAdcResolution(8, "8位ADC");
  testAdcResolution(10, "10位ADC");
  testAdcResolution(12, "12位ADC");
  testAdcResolution(16, "16位ADC");

  // 测试校准功能
  std::cout << "\n=== 测试校准功能 (12位ADC) ===\n";
  AdcSourceConfig config = {
      .adcResolution = 12,
      .calibrationOffset = 1000 // 偏移1000
  };

  AdcInput1CH adc(config);

  uint32_t testValue = 2048; // 12位ADC中点值
  adc.setRawValue(testValue, 0);
  int16_t result = adc.getValue(0);

  std::cout << "原始值: " << testValue << "\n";
  std::cout << "校准偏移: " << config.calibrationOffset << "\n";
  std::cout << "最终结果: " << result << "\n";

  return 0;
}