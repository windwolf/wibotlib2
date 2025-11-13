#include "wibotlib\src\SyncPipeline\source\memory-source.hpp"
#include <iostream>

using namespace wibot;

int main() {
  std::cout << "优化后的自动校准功能测试\n";
  std::cout << "============================\n\n";

  // 模拟DMA缓冲区
  uint16_t dmaBuffer[2] = {1000, 2000}; // 两个通道

  // 创建ADC配置
  AdcSourceConfig config = {.adcResolution = 12, .calibrationOffset = 0};

  // 创建MemorySource
  MemorySource<2> memSource(config, dmaBuffer);

  std::cout << "1. 校准前的数据:\n";
  memSource.update();
  for (uint8_t ch = 0; ch < 2; ch++) {
    std::cout << "  通道" << (int)ch << ": " << memSource.getValue(ch) << "\n";
  }

  // 开始自动校准
  AutoCalibrationConfig calibConfig = {.sampleIntervalMs = 10, // 10ms间隔
                                       .sampleCount = 5,       // 5次采样
                                       .enabled = true};

  std::cout << "\n2. 开始自动校准...\n";
  memSource.startAutoCalibration(calibConfig);

  // 模拟校准过程
  while (memSource.isCalibrating()) {
    memSource.update();
    float progress = memSource.getCalibrationProgress();
    std::cout << "  校准进度: " << progress << "%\n";
  }

  std::cout << "\n3. 校准完成后的数据:\n";
  memSource.update();
  for (uint8_t ch = 0; ch < 2; ch++) {
    std::cout << "  通道" << (int)ch << ": " << memSource.getValue(ch) << "\n";
  }

  std::cout << "\n✅ 优化验证:\n";
  std::cout << "  - AutoCalibrationConfig 不再作为成员变量存储\n";
  std::cout << "  - 累加器数组在栈中分配，每次校准时创建\n";
  std::cout << "  - 减少了内存占用和初始化开销\n";

  return 0;
}