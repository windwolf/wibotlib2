#include "wibotlib\src\SyncPipeline\source\memory-source.hpp"
#include <iomanip>
#include <iostream>

using namespace wibot;

// 模拟DMA缓冲区 - 带有固定偏移的ADC数据
volatile uint16_t dmaAdcBuffer[4] = {0, 0, 0, 0};

// 模拟ADC数据更新函数
void updateAdcData(int cycle) {
  // 模拟带有固定偏移的ADC读数
  // 假设理想值应该是0，但由于硬件偏移，实际读数有偏差
  uint16_t baseOffset[4] = {512, 256, 128, 64}; // 每个通道的固定偏移

  // 添加少量噪声
  int16_t noise = (cycle % 20) - 10; // -10到+10的噪声

  for (int ch = 0; ch < 4; ch++) {
    int32_t value = baseOffset[ch] + noise;
    if (value < 0)
      value = 0;
    if (value > 4095)
      value = 4095;
    dmaAdcBuffer[ch] = static_cast<uint16_t>(value);
  }
}

int main() {
  std::cout << "自动校准功能演示\n";
  std::cout << "==================\n\n";

  // ADC配置
  AdcSourceConfig config = {
      .adcResolution = 12,   // 12位ADC
      .calibrationOffset = 0 // 初始无偏移
  };

  // 自动校准配置
  AutoCalibrationConfig calibConfig = {
      .sampleIntervalMs = 10, // 每10ms采样一次
      .sampleCount = 10,      // 累计10次采样
      .enabled = true         // 启用自动校准
  };

  // 创建内存数据源
  MemorySource<4> adcSource(config, const_cast<const uint16_t *>(dmaAdcBuffer));

  std::cout << "1. 显示校准前的原始数据（带偏移）\n";
  std::cout << "-----------------------------------\n";

  // 显示校准前的数据
  for (int cycle = 0; cycle < 5; cycle++) {
    updateAdcData(cycle);
    adcSource.update();

    std::cout << "周期 " << cycle + 1 << ": ";
    for (uint8_t ch = 0; ch < 4; ch++) {
      int16_t value = adcSource.getValue(ch);
      std::cout << "CH" << (int)ch << "=" << value << " ";
    }
    std::cout << "\n";
  }

  std::cout << "\n2. 开始自动校准\n";
  std::cout << "----------------\n";

  // 开始自动校准
  adcSource.startAutoCalibration(calibConfig);

  int cycle = 0;
  while (adcSource.isCalibrating()) {
    updateAdcData(cycle);
    adcSource.update();

    float progress = adcSource.getCalibrationProgress();
    std::cout << "校准进度: " << std::fixed << std::setprecision(1) << progress
              << "% ";

    // 显示当前原始值
    std::cout << "原始值: ";
    for (uint8_t ch = 0; ch < 4; ch++) {
      std::cout << "CH" << (int)ch << "=" << dmaAdcBuffer[ch] << " ";
    }
    std::cout << "\n";

    cycle++;
  }

  std::cout << "\n校准完成！\n";
  std::cout << "当前校准偏移: " << adcSource.getCalibration() << "\n\n";

  std::cout << "3. 显示校准后的数据\n";
  std::cout << "-------------------\n";

  // 显示校准后的数据
  for (int testCycle = 0; testCycle < 5; testCycle++) {
    updateAdcData(cycle + testCycle);
    adcSource.update();

    std::cout << "周期 " << testCycle + 1 << ": ";
    for (uint8_t ch = 0; ch < 4; ch++) {
      int16_t calibratedValue = adcSource.getValue(ch);
      std::cout << "CH" << (int)ch << "=" << calibratedValue
                << " (原始:" << dmaAdcBuffer[ch] << ") ";
    }
    std::cout << "\n";
  }

  std::cout << "\n注意事项:\n";
  std::cout << "- 自动校准会以指定间隔采样原始值\n";
  std::cout << "- 累计指定次数后计算平均值作为偏移量\n";
  std::cout << "- 校准偏移会自动应用到所有后续读数\n";
  std::cout << "- 可以随时查看校准进度和状态\n";

  return 0;
}