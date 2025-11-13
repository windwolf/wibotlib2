#include "wibotlib\src\SyncPipeline\source\memory-source.hpp"
#include <iostream>

using namespace wibot;

int main() {
  // 设置一些测试数据
  uint32_t rawValues[4] = {1024, 2048, 3072, 4095}; // 12位ADC原始值

  // 创建一个4通道的内存数据源
  AdcSourceConfig config = {.adcResolution = 12, .calibrationOffset = 0};
  MemorySource<4> memSource(config, rawValues); // 4通道内存数据源

  // 更新数据源以从缓冲区读取数据
  memSource.update();

  // 测试多通道getValue
  std::cout << "多通道内存数据源测试结果 (int16_t原始值):\n";
  for (uint8_t ch = 0; ch < 4; ch++) {
    int16_t value = memSource.getValue(ch);
    std::cout << "通道 " << (int)ch << ": " << value << "\n";
  }

  // 测试校准功能
  std::cout << "\n测试校准功能:\n";
  memSource.setCalibration(1000); // 偏移1000

  // 更新数据源以应用新的校准
  memSource.update();

  std::cout << "校准后的值 (偏移+1000):\n";
  for (uint8_t ch = 0; ch < 4; ch++) {
    int16_t value = memSource.getValue(ch);
    std::cout << "通道 " << (int)ch << ": " << value << "\n";
  }

  // 显示当前校准偏移
  std::cout << "\n当前校准偏移: " << memSource.getCalibration() << "\n";

  return 0;
}