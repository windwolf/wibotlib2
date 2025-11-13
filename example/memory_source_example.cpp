#include "wibotlib\src\SyncPipeline\source\memory-source.hpp"
#include <iostream>

using namespace wibot;

int main() {
  // 模拟由ADC通过DMA更新的外部缓冲区数据
  uint16_t adcBuffer[4] = {1024, 2048, 3072, 4095}; // 12位ADC原始值

  // 创建配置
  AdcSourceConfig config = {
      .adcResolution = 12,   // 12位ADC
      .calibrationOffset = 0 // 无偏移
  };

  // 方法1: 使用构造函数传入缓冲区
  MemorySource<4> memorySource1(config, adcBuffer);

  // 从外部缓冲区更新数据
  memorySource1.update();

  std::cout << "方法1: 构造函数传入缓冲区\n";
  for (uint8_t ch = 0; ch < 4; ch++) {
    int16_t value = memorySource1.getValue(ch);
    std::cout << "通道 " << (int)ch << ": " << value << "\n";
  }

  // 方法2: 使用默认构造函数，后设置缓冲区
  MemorySource<4> memorySource2;
  memorySource2.setBuffer(adcBuffer);
  memorySource2.update();

  std::cout << "\n方法2: 后设置缓冲区\n";
  for (uint8_t ch = 0; ch < 4; ch++) {
    int16_t value = memorySource2.getValue(ch);
    std::cout << "通道 " << (int)ch << ": " << value << "\n";
  }

  // 模拟DMA更新外部缓冲区数据
  std::cout << "\n模拟DMA更新外部缓冲区数据:\n";
  adcBuffer[0] = 500;
  adcBuffer[1] = 1500;
  adcBuffer[2] = 2500;
  adcBuffer[3] = 3500;

  // 重新更新数据源
  memorySource1.update();

  std::cout << "更新后的值:\n";
  for (uint8_t ch = 0; ch < 4; ch++) {
    int16_t value = memorySource1.getValue(ch);
    std::cout << "通道 " << (int)ch << ": " << value << "\n";
  }

  // 测试校准功能
  std::cout << "\n测试校准功能 (偏移+1000):\n";
  memorySource1.setCalibration(1000);
  memorySource1.update();

  for (uint8_t ch = 0; ch < 4; ch++) {
    int16_t value = memorySource1.getValue(ch);
    std::cout << "通道 " << (int)ch << ": " << value << "\n";
  }

  return 0;
}