#include "wibotlib\src\SyncPipeline\source\memory-source.hpp"
#include <iostream>

using namespace wibot;

// 模拟DMA缓冲区 - 在实际应用中，这会由DMA控制器自动填充
volatile uint16_t dmaAdcBuffer[4] = {0, 0, 0, 0};

// 模拟DMA中断回调 - 在实际应用中，这会在DMA传输完成时被调用
void simulateDmaCallback() {
  // 模拟ADC转换结果被DMA写入缓冲区
  dmaAdcBuffer[0] = 1024; // 通道0: 1024/4095 ≈ 25%
  dmaAdcBuffer[1] = 2048; // 通道1: 2048/4095 ≈ 50%
  dmaAdcBuffer[2] = 3072; // 通道2: 3072/4095 ≈ 75%
  dmaAdcBuffer[3] = 4095; // 通道3: 4095/4095 = 100%
}

int main() {
  // ADC配置
  AdcSourceConfig config = {
      .adcResolution = 12,   // 12位ADC (STM32G431典型配置)
      .calibrationOffset = 0 // 无校准偏移
  };

  // 创建内存数据源，直接指向DMA缓冲区
  // 注意：这里转换为const指针，因为MemorySource只读取数据
  MemorySource<4> adcSource(config, const_cast<const uint16_t *>(dmaAdcBuffer));

  std::cout << "DMA ADC数据源示例\n";
  std::cout << "=================\n\n";

  // 初始状态（DMA缓冲区为空）
  std::cout << "初始状态（DMA缓冲区为空）:\n";
  adcSource.update();
  for (uint8_t ch = 0; ch < 4; ch++) {
    int16_t value = adcSource.getValue(ch);
    std::cout << "通道 " << (int)ch << ": " << value << "\n";
  }

  std::cout << "\n--- 模拟DMA传输完成 ---\n\n";

  // 模拟DMA传输完成，缓冲区被更新
  simulateDmaCallback();

  // 读取DMA更新后的数据
  std::cout << "DMA传输完成后的数据:\n";
  adcSource.update();
  for (uint8_t ch = 0; ch < 4; ch++) {
    int16_t value = adcSource.getValue(ch);
    float percentage = (float)dmaAdcBuffer[ch] / 4095.0f * 100.0f;
    std::cout << "通道 " << (int)ch << ": " << value
              << " (原始: " << dmaAdcBuffer[ch] << ", " << percentage << "%)\n";
  }

  // 应用校准
  std::cout << "\n--- 应用校准偏移 ---\n\n";
  adcSource.setCalibration(500); // 添加500的偏移
  adcSource.update();

  std::cout << "校准后的数据 (偏移+500):\n";
  for (uint8_t ch = 0; ch < 4; ch++) {
    int16_t value = adcSource.getValue(ch);
    std::cout << "通道 " << (int)ch << ": " << value << "\n";
  }

  std::cout << "\n注意：在实际应用中，DMA会自动将ADC转换结果写入缓冲区，\n";
  std::cout << "无需手动设置数据。只需调用update()方法读取最新数据。\n";

  return 0;
}